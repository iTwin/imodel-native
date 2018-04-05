/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/DgnECPresentationSerializerTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "../NonPublished/RulesEngine/ECDbTestProject.h"
#include "../NonPublished/RulesEngine/TestHelpers.h"
#include <UnitTests/BackDoor/ECPresentation/TestECPresentationManager.h>

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define TEST_SCHEMA R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="test" version="9.3" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECEnumeration typeName="TestEnum" backingTypeName="int" isStrict="True" description="" displayLabel="IntegerEnum">
            <ECEnumerator value="1" displayLabel="A" />
            <ECEnumerator value="2" displayLabel="B" />
            <ECEnumerator value="3" displayLabel="C" />
        </ECEnumeration>
        <ECEnumeration typeName="TestStringEnum" backingTypeName="string" isStrict="True" description="" displayLabel="StringEnum">
            <ECEnumerator value="D" displayLabel="A" />
            <ECEnumerator value="E" displayLabel="B" />
            <ECEnumerator value="F" displayLabel="C" />
        </ECEnumeration>
        <KindOfQuantity typeName="TestKOQ" displayLabel="Test" persistenceUnit="MM(real)" relativeError="0"/>
        <ECEntityClass typeName="PropertyTestClassB">
            <ECProperty propertyName="String" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName="PropertyTestClassA">
            <ECProperty propertyName="String" typeName="string" />
            <ECProperty propertyName="Int" typeName="int" />
            <ECProperty propertyName="EnumProperty" typeName="TestEnum" />
            <ECProperty propertyName="EnumStringProperty" typeName="TestStringEnum" />
            <ECProperty propertyName="KOQProperty" typeName="int" kindOfQuantity="TestKOQ" />
        </ECEntityClass>
        <ECRelationshipClass typeName="TestClassAHasTestClassB" modifier="None" strength="embedding">
            <Source multiplicity="(1..1)" roleLabel="owns" polymorphic="true">
                <Class class="PropertyTestClassA"/>
            </Source>
            <Target multiplicity="(1..1)" roleLabel="is owned by" polymorphic="true">
                <Class class="PropertyTestClassB"/>
            </Target>
        </ECRelationshipClass>
        <ECStructClass typeName="TestStruct">
            <ECProperty propertyName="IntProperty" typeName="int" />
            <ECProperty propertyName="StringProperty" typeName="string" />
        </ECStructClass>
    </ECSchema>)xml"

/*=================================================================================**//**
* @bsiclass                                     Mantas.Kontrimas                03/2018
+===============+===============+===============+===============+===============+======*/
struct DgnECPresentationSerializerTests : ECPresentationTest
{
    static ECDbTestProject* s_project;

    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("DgnECPresentationSerializerTests");

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

    ECClassCP GetClass(Utf8CP className)
        {
        return s_project->GetECDb().Schemas().GetClass("TestSchema", className);
        }

    virtual void SetUp() override
        {
        ECPresentationTest::SetUp();
        Localization::Init();
        }
};
ECDbTestProject* DgnECPresentationSerializerTests::s_project = nullptr;

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ConnectionEventSerializationWithOpenedConnectionEventType)
    {
    IConnectionPtr connection = new TestConnection(s_project->GetECDb());
    ConnectionEvent evt(*connection, true, ConnectionEventType::Opened);
    rapidjson::Document actual = evt.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "EventType": 0,
        "Connection": {
            "ConnectionId" : "",
            "ConnectionGuid" : "",
            "IsProjectPrimary": true
            }
        })");
    expected["Connection"]["ConnectionId"].SetString(connection->GetId().c_str(), expected.GetAllocator());
    expected["Connection"]["ConnectionGuid"].SetString(connection->GetECDb().GetDbGuid().ToString().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ConnectionEventSerializationWithClosedConnectionEventType)
    {
    IConnectionPtr connection = new TestConnection(s_project->GetECDb());
    ConnectionEvent evt(*connection, true, ConnectionEventType::Closed);
    rapidjson::Document actual = evt.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "EventType": 1,
        "Connection": {
            "ConnectionId" : "",
            "ConnectionGuid" : "",
            "IsProjectPrimary": false
            }
        })");
    expected["Connection"]["ConnectionId"].SetString(connection->GetId().c_str(), expected.GetAllocator());
    expected["Connection"]["ConnectionGuid"].SetString(connection->GetECDb().GetDbGuid().ToString().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ContentFieldEditorSerializationNoParams)
    {
    ContentFieldEditor contentFieldEditor("contentFieldEditor");
    rapidjson::Document actual = contentFieldEditor.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Name": "contentFieldEditor",
        "Params": {}
        })");

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
    int _CompareTo(Params const& other) const override 
        {
        if (0 != Params::_CompareTo(other))
            return Params::_CompareTo(other);
        return strcmp(m_name.c_str(), static_cast<TestEditorParams const&>(other).m_name.c_str());
        }

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
TEST_F(DgnECPresentationSerializerTests, ContentFieldEditorsSerializationWithOneParam)
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

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, CategorySerialization)
    {
    ContentDescriptor::Category category("categoryName", "categoryLabel", "categoryDescription", 10, true);
    rapidjson::Document actual = category.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Name": "categoryName",
        "DisplayLabel": "categoryLabel",
        "Description": "categoryDescription",
        "Expand": true,
        "Priority": 10
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, PropertySerializationSimplePrimitiveProperty)
    {
    ECClassCP testClass = GetClass("PropertyTestClassA");
    ContentDescriptor::Property property("this", *testClass, *testClass->GetPropertyP("String"));
    rapidjson::Document actual = property.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Property": { 
            "BaseClassInfo": {
                "Id": "",
                "Name": "TestSchema:PropertyTestClassA",
                "Label": "PropertyTestClassA"
                },
            "ActualClassInfo": {
                "Id": "",
                "Name": "TestSchema:PropertyTestClassA",
                "Label": "PropertyTestClassA"
                },
            "Name": "String",
            "Type": "string"
        },
        "RelatedClassPath": []
        })");
    expected["Property"]["BaseClassInfo"]["Id"].SetString(testClass->GetId().ToString().c_str(), expected.GetAllocator());
    expected["Property"]["ActualClassInfo"]["Id"].SetString(testClass->GetId().ToString().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, PropertySerializationSimplePrimitivePropertyIsRelated)
    {
    ECClassCP testClassA = GetClass("PropertyTestClassA");
    ECClassCP testClassB = GetClass("PropertyTestClassB");
    ECRelationshipClassCP relClassAClassB = GetClass("TestClassAHasTestClassB")->GetRelationshipClassCP();
    ContentDescriptor::Property property("this", *testClassA, *testClassA->GetPropertyP("String"));
    property.SetIsRelated(RelatedClass(*testClassA, *testClassB, *relClassAClassB, false), RelationshipMeaning::RelatedInstance);
    rapidjson::Document actual = property.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Property": { 
            "BaseClassInfo": {
                "Id": "",
                "Name": "TestSchema:PropertyTestClassA",
                "Label": "PropertyTestClassA"
                },
            "ActualClassInfo": {
                "Id": "",
                "Name": "TestSchema:PropertyTestClassA",
                "Label": "PropertyTestClassA"
                },
            "Name": "String",
            "Type": "string"
        },
        "RelatedClassPath": [
           {
            "SourceClassInfo": {
                "Id": "",
                "Name": "TestSchema:PropertyTestClassA",
                "Label": "PropertyTestClassA"
            },
            "TargetClassInfo": {
                "Id": "",
                "Name": "TestSchema:PropertyTestClassB",
                "Label": "PropertyTestClassB"
            },
            "RelationshipInfo": {
                "Id": "",
                "Name": "TestSchema:TestClassAHasTestClassB",
                "Label": "TestClassAHasTestClassB"
            },
            "IsForwardRelationship": false,
            "IsPolymorphicRelationship": true
            } 
        ]
        })");
    expected["Property"]["BaseClassInfo"]["Id"].SetString(testClassA->GetId().ToString().c_str(), expected.GetAllocator());
    expected["Property"]["ActualClassInfo"]["Id"].SetString(testClassA->GetId().ToString().c_str(), expected.GetAllocator());
    expected["RelatedClassPath"][0]["SourceClassInfo"]["Id"].SetString(testClassA->GetId().ToString().c_str(), expected.GetAllocator());
    expected["RelatedClassPath"][0]["TargetClassInfo"]["Id"].SetString(testClassB->GetId().ToString().c_str(), expected.GetAllocator());
    expected["RelatedClassPath"][0]["RelationshipInfo"]["Id"].SetString(relClassAClassB->GetId().ToString().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, PropertySerializationEnumInt)
    {
    ECClassCP testClass = GetClass("PropertyTestClassA");
    ContentDescriptor::Property property("this", *testClass, *testClass->GetPropertyP("EnumProperty"));
    rapidjson::Document actual = property.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Property": { 
            "BaseClassInfo": {
                "Id": "",
                "Name": "TestSchema:PropertyTestClassA",
                "Label": "PropertyTestClassA"
                },
            "ActualClassInfo": {
                "Id": "",
                "Name": "TestSchema:PropertyTestClassA",
                "Label": "PropertyTestClassA"
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
        })");
    expected["Property"]["BaseClassInfo"]["Id"].SetString(testClass->GetId().ToString().c_str(), expected.GetAllocator());
    expected["Property"]["ActualClassInfo"]["Id"].SetString(testClass->GetId().ToString().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, PropertySerializationEnumString)
    {
    ECClassCP testClass = GetClass("PropertyTestClassA");
    ContentDescriptor::Property property("this", *testClass, *testClass->GetPropertyP("EnumStringProperty"));
    rapidjson::Document actual = property.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Property": { 
            "BaseClassInfo": {
                "Id": "",
                "Name": "TestSchema:PropertyTestClassA",
                "Label": "PropertyTestClassA"
                },
            "ActualClassInfo": {
                "Id": "",
                "Name": "TestSchema:PropertyTestClassA",
                "Label": "PropertyTestClassA"
                },
            "Name": "EnumStringProperty",
            "Type": "enum",
            "IsStrict": true,
            "Choices": [{
                "Label": "A",
                "Value": "D"
                },{
                "Label": "B",
                "Value": "E"
                },{
                "Label": "C",
                "Value": "F"
                }]
            },
        "RelatedClassPath": []
        })");
    expected["Property"]["BaseClassInfo"]["Id"].SetString(testClass->GetId().ToString().c_str(), expected.GetAllocator());
    expected["Property"]["ActualClassInfo"]["Id"].SetString(testClass->GetId().ToString().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, PropertySerializationWithKOQ)
    {
    ECClassCP testClass = GetClass("PropertyTestClassA");
    ContentDescriptor::Property property("this", *testClass, *testClass->GetPropertyP("KOQProperty"));
    rapidjson::Document actual = property.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"*({
        "Property": { 
            "BaseClassInfo": {
                "Id": "",
                "Name": "TestSchema:PropertyTestClassA",
                "Label": "PropertyTestClassA"
                },
            "ActualClassInfo": {
                "Id": "",
                "Name": "TestSchema:PropertyTestClassA",
                "Label": "PropertyTestClassA"
                },
            "Name": "KOQProperty",
            "Type": "int",
            "KindOfQuantity": {
                "Name": "TestSchema:TestKOQ",
                "DisplayLabel": "Test",
                "PersistenceUnit": "MM(real)",
                "CurrentFusId": "MM(real)"
                }
            },
        "RelatedClassPath": []
        })*");
    expected["Property"]["BaseClassInfo"]["Id"].SetString(testClass->GetId().ToString().c_str(), expected.GetAllocator());
    expected["Property"]["ActualClassInfo"]["Id"].SetString(testClass->GetId().ToString().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, UpdateRecordCSerializationhangeTypeDelete)
    {
    TestNavNodePtr node = TestNavNode::Create();
    UpdateRecord updateRecord(*node);
    rapidjson::Document actual = updateRecord.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Type": "Delete",
        "Node": {
            "NodeId": "0",
            "ParentNodeId": "0",
            "Key": {
                "Type": "TestType",
                "PathFromRoot": []
                },
            "Label": "TestLabel",
            "Description": "",
            "ExpandedImageId": "",
            "CollapsedImageId": "",
            "ForeColor": "",
            "BackColor": "",
            "FontStyle": "Regular",
            "Type": "TestType",
            "HasChildren": false,
            "IsSelectable": true,
            "IsEditable": false,
            "IsChecked": false,
            "IsCheckboxVisible": false,
            "IsCheckboxEnabled": false,
            "IsExpanded": false,
            "ECInstanceId": "0"
            },
        "RulesetId": "Invalid ruleset ID"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, UpdateRecordSerializationChangeTypeInsert)
    {
    TestNavNodePtr node = TestNavNode::Create();
    UpdateRecord updateRecord(*node, 10);
    rapidjson::Document actual = updateRecord.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Type": "Insert",
        "Node": {
            "NodeId": "0",
            "ParentNodeId": "0",
            "Key": {
                "Type": "TestType",
                "PathFromRoot": []
                },
            "Label": "TestLabel",
            "Description": "",
            "ExpandedImageId": "",
            "CollapsedImageId": "",
            "ForeColor": "",
            "BackColor": "",
            "FontStyle": "Regular",
            "Type": "TestType",
            "HasChildren": false,
            "IsSelectable": true,
            "IsEditable": false,
            "IsChecked": false,
            "IsCheckboxVisible": false,
            "IsCheckboxEnabled": false,
            "IsExpanded": false,
            "ECInstanceId": "0"
            },
        "Position": 10,
        "RulesetId": "Invalid ruleset ID"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, UpdateRecordSerializationChangeTypeUpdate)
    {
    TestNavNodePtr node = TestNavNode::Create();
    bvector<JsonChange> changes;
    changes.push_back(JsonChange("PropertyName", rapidjson::Value("oldValue"), rapidjson::Value("newValue")));
    UpdateRecord updateRecord(*node, std::move(changes));
    rapidjson::Document actual = updateRecord.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Type": "Update",
        "Node": {
            "NodeId": "0",
            "ParentNodeId": "0",
            "Key": {
                "Type": "TestType",
                "PathFromRoot": []
                },
            "Label": "TestLabel",
            "Description": "",
            "ExpandedImageId": "",
            "CollapsedImageId": "",
            "ForeColor": "",
            "BackColor": "",
            "FontStyle": "Regular",
            "Type": "TestType",
            "HasChildren": false,
            "IsSelectable": true,
            "IsEditable": false,
            "IsChecked": false,
            "IsCheckboxVisible": false,
            "IsCheckboxEnabled": false,
            "IsExpanded": false,
            "ECInstanceId": "0"
            },
        "Changes": [
            {
            "Name": "PropertyName",
            "OldValue": "oldValue",
            "NewValue": "newValue"
            }
            ],
        "RulesetId": "Invalid ruleset ID"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, UpdateRecordSerializationChangeTypeUpdateMoreThanOneChange)
    {
    TestNavNodePtr node = TestNavNode::Create();
    bvector<JsonChange> changes;
    changes.push_back(JsonChange("PropertyName", rapidjson::Value("oldValue"), rapidjson::Value("newValue")));
    changes.push_back(JsonChange("PropertyName2", rapidjson::Value("oldValue2"), rapidjson::Value("newValue2")));
    UpdateRecord updateRecord(*node, std::move(changes));
    rapidjson::Document actual = updateRecord.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Type": "Update",
        "Node": {
            "NodeId": "0",
            "ParentNodeId": "0",
            "Key": {
                "Type": "TestType",
                "PathFromRoot": []
                },
            "Label": "TestLabel",
            "Description": "",
            "ExpandedImageId": "",
            "CollapsedImageId": "",
            "ForeColor": "",
            "BackColor": "",
            "FontStyle": "Regular",
            "Type": "TestType",
            "HasChildren": false,
            "IsSelectable": true,
            "IsEditable": false,
            "IsChecked": false,
            "IsCheckboxVisible": false,
            "IsCheckboxEnabled": false,
            "IsExpanded": false,
            "ECInstanceId": "0"
            },
        "Changes": [
            {
            "Name": "PropertyName",
            "OldValue": "oldValue",
            "NewValue": "newValue"
            },
            {
            "Name": "PropertyName2",
            "OldValue": "oldValue2",
            "NewValue": "newValue2"
            }
            ],
        "RulesetId": "Invalid ruleset ID"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, UpdateRecordSerializationChangeTypeDeleteNoRulesetId)
    {
    TestNavNodePtr node = TestNavNode::Create();
    node->GetExtendedDataR().RemoveAllMembers();
    UpdateRecord updateRecord(*node);
    rapidjson::Document actual = updateRecord.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Type": "Delete",
        "Node": {
            "NodeId": "0",
            "ParentNodeId": "0",
            "Key": {
                "Type": "TestType",
                "PathFromRoot": []
                },
            "Label": "TestLabel",
            "Description": "",
            "ExpandedImageId": "",
            "CollapsedImageId": "",
            "ForeColor": "",
            "BackColor": "",
            "FontStyle": "Regular",
            "Type": "TestType",
            "HasChildren": false,
            "IsSelectable": true,
            "IsEditable": false,
            "IsChecked": false,
            "IsCheckboxVisible": false,
            "IsCheckboxEnabled": false,
            "IsExpanded": false,
            "ECInstanceId": "0"
            }
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, DisplayLabelFieldSerialization)
    {
    ContentDescriptor::DisplayLabelField field("Label", 10);
    rapidjson::Document actual = field.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Category": {
            "Name": "Miscellaneous",
            "DisplayLabel": "Miscellaneous",
            "Description": "",
            "Expand": false,
            "Priority": 1000
            },
        "Name": "DisplayLabel",
        "DisplayLabel": "Label",
        "Type": {
            "TypeName": "string",
            "ValueFormat": "Primitive"
            },
        "IsReadOnly": true,
        "Priority": 10
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, CalculatedPropertyFieldSerialization)
    {
    ECClassCP testClass = GetClass("PropertyTestClassA");
    ContentDescriptor::CalculatedPropertyField field("10", "FieldName", "ValueExpression", testClass, 10);
    rapidjson::Document actual = field.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Category": {
            "Name": "Miscellaneous",
            "DisplayLabel": "Miscellaneous",
            "Description": "",
            "Expand": false,
            "Priority": 1000
            },
        "Name": "FieldName",
        "DisplayLabel": "10",
        "Type": {
            "TypeName": "string",
            "ValueFormat": "Primitive"
            },
        "IsReadOnly": true,
        "Priority": 10
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ECPropertiesFieldSerialization)
    {
    ECClassCP testClass = GetClass("PropertyTestClassA");
    ContentDescriptor::ECPropertiesField field(*testClass, ContentDescriptor::Property("this", *testClass, *testClass->GetPropertyP("String")));
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
        "Name": "PropertyTestClassA_String",
        "DisplayLabel": "String",
        "Type": {
            "TypeName": "string",
            "ValueFormat": "Primitive"
            },
        "IsReadOnly": false,
        "Priority": 0,
        "Properties": [
            {
            "Property": {
                "BaseClassInfo": {
                    "Id": "",
                    "Name": "TestSchema:PropertyTestClassA",
                    "Label": "PropertyTestClassA"
                    },
                "ActualClassInfo": {
                    "Id": "",
                    "Name": "TestSchema:PropertyTestClassA",
                    "Label": "PropertyTestClassA"
                    },
                "Name": "String",
                "Type": "string"
            },
            "RelatedClassPath": []
            }
            ]
        })");
    expected["Properties"][0]["Property"]["BaseClassInfo"]["Id"].SetString(testClass->GetId().ToString().c_str(), expected.GetAllocator());
    expected["Properties"][0]["Property"]["ActualClassInfo"]["Id"].SetString(testClass->GetId().ToString().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ECPropertiesFieldSerializationWithMoreThanOneProperty)
    {
    ECClassCP testClass = GetClass("PropertyTestClassA");
    ContentDescriptor::ECPropertiesField field(*testClass, ContentDescriptor::Property("this", *testClass, *testClass->GetPropertyP("String")));
    field.AddProperty(ContentDescriptor::Property("this", *testClass, *testClass->GetPropertyP("Int")));
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
        "Name": "PropertyTestClassA_String",
        "DisplayLabel": "String",
        "Type": {
            "TypeName": "string",
            "ValueFormat": "Primitive"
            },
        "IsReadOnly": false,
        "Priority": 0,
        "Properties": [
            {
            "Property": {
                "BaseClassInfo": {
                    "Id": "",
                    "Name": "TestSchema:PropertyTestClassA",
                    "Label": "PropertyTestClassA"
                    },
                "ActualClassInfo": {
                    "Id": "",
                    "Name": "TestSchema:PropertyTestClassA",
                    "Label": "PropertyTestClassA"
                    },
                "Name": "String",
                "Type": "string"
                },
            "RelatedClassPath": []
            },
            {
            "Property": {
                "BaseClassInfo": {
                    "Id": "",
                    "Name": "TestSchema:PropertyTestClassA",
                    "Label": "PropertyTestClassA"
                    },
                "ActualClassInfo": {
                    "Id": "",
                    "Name": "TestSchema:PropertyTestClassA",
                    "Label": "PropertyTestClassA"
                    },
                "Name": "Int",
                "Type": "int"
                },
            "RelatedClassPath": []
            }
            ]
        })");
    expected["Properties"][0]["Property"]["BaseClassInfo"]["Id"].SetString(testClass->GetId().ToString().c_str(), expected.GetAllocator());
    expected["Properties"][1]["Property"]["BaseClassInfo"]["Id"].SetString(testClass->GetId().ToString().c_str(), expected.GetAllocator());
    expected["Properties"][0]["Property"]["ActualClassInfo"]["Id"].SetString(testClass->GetId().ToString().c_str(), expected.GetAllocator());
    expected["Properties"][1]["Property"]["ActualClassInfo"]["Id"].SetString(testClass->GetId().ToString().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ECInstanceKeyFieldSerialization)
    {
    ContentDescriptor::ECInstanceKeyField field;
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
        "Name": "key",
        "DisplayLabel": "",
        "Type": {
            "TypeName": "ECInstanceKey",
            "ValueFormat": "Primitive"
            },
        "IsReadOnly": true,
        "Priority": 0
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ECNavigationInstanceIdFieldSerialization)
    {
    ECClassCP testClass = GetClass("PropertyTestClassA");
    ContentDescriptor::ECNavigationInstanceIdField field(ContentDescriptor::ECPropertiesField(*testClass, ContentDescriptor::Property("this", *testClass, *testClass->GetPropertyP("String"))));
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
        "Name": "",
        "DisplayLabel": "",
        "Type": {
            "TypeName": "ECInstanceId",
            "ValueFormat": "Primitive"
            },
        "IsReadOnly": true,
        "Priority": 0
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, NestedContentFieldSerialization)
    {
    ECClassCP testClassA = GetClass("PropertyTestClassA");
    ECClassCP testClassB = GetClass("PropertyTestClassB");
    ECRelationshipClassCP relClassAClassB = GetClass("TestClassAHasTestClassB")->GetRelationshipClassCP();
    ContentDescriptor::Category category("name", "label", "", 1);
    ContentDescriptor::NestedContentField field(category, "field_name", "field_label", *testClassA, "PropertyTestClassA",
        {RelatedClass(*testClassA, *testClassB, *relClassAClassB, false, "primary_instance", "rel")},
        {
        new ContentDescriptor::DisplayLabelField("NestedLabel", 10),
        new ContentDescriptor::ECPropertiesField(*testClassB, ContentDescriptor::Property("rel_RET_PropertyTestClassA_0", *testClassA, *testClassA->GetPropertyP("String")))
        });
    rapidjson::Document actual = field.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Category": {
            "Name": "name",
            "DisplayLabel": "label",
            "Description": "",
            "Expand": false,
            "Priority": 1
            },
        "Name": "field_name",
        "DisplayLabel": "field_label",
        "Type": {
            "TypeName": "PropertyTestClassA",
            "ValueFormat": "Struct",
            "Members": [
                {
                "Name": "DisplayLabel",
                "Label": "NestedLabel",
                "Type": {
                    "TypeName": "string",
                    "ValueFormat": "Primitive"
                    }
                },
                {
                "Name": "PropertyTestClassA_String",
                "Label": "String",
                "Type": {
                    "TypeName": "string",
                    "ValueFormat": "Primitive"
                    }
                }
                ]
            },
        "IsReadOnly": true,
        "Priority": 0,
        "ContentClassInfo": {
            "Id": "",
            "Name": "TestSchema:PropertyTestClassA",
            "Label": "PropertyTestClassA"
            },
        "PathToPrimary": [
            {
            "SourceClassInfo": {
                "Id": "",
                "Name": "TestSchema:PropertyTestClassA",
                "Label": "PropertyTestClassA"
                },
            "TargetClassInfo": {
                "Id": "",
                "Name": "TestSchema:PropertyTestClassB",
                "Label": "PropertyTestClassB"
                },
            "RelationshipInfo": {
                "Id": "",
                "Name": "TestSchema:TestClassAHasTestClassB",
                "Label": "TestClassAHasTestClassB"
                },
            "IsForwardRelationship": false,
            "IsPolymorphicRelationship": true
            }
            ],
        "NestedFields": [
            {
            "Category": {
                "Name": "Miscellaneous",
                "DisplayLabel": "Miscellaneous",
                "Description": "",
                "Expand": false,
                "Priority": 1000
                },
            "Name": "DisplayLabel",
            "DisplayLabel": "NestedLabel",
            "Type": {
                "TypeName": "string",
                "ValueFormat": "Primitive"
                },
            "IsReadOnly": true,
            "Priority": 10
            },
            {
            "Category": {
                "Name": "",
                "DisplayLabel": "",
                "Description": "",
                "Expand": false,
                "Priority": 0
                },
            "Name": "PropertyTestClassA_String",
            "DisplayLabel": "String",
            "Type": {
                "TypeName": "string",
                "ValueFormat": "Primitive"
                },
            "IsReadOnly": false,
            "Priority": 0,
            "Properties": [
                {
                "Property": {
                    "BaseClassInfo": {
                        "Id": "",
                        "Name": "TestSchema:PropertyTestClassA",
                        "Label": "PropertyTestClassA"
                        },
                    "ActualClassInfo": {
                        "Id": "",
                        "Name": "TestSchema:PropertyTestClassA",
                        "Label": "PropertyTestClassA"
                        },
                    "Name": "String",
                    "Type": "string"
                    },
                "RelatedClassPath": []
                }
                ]
            }
            ]
        })");
    expected["ContentClassInfo"]["Id"].SetString(testClassA->GetId().ToString().c_str(), expected.GetAllocator());
    expected["PathToPrimary"][0]["SourceClassInfo"]["Id"].SetString(testClassA->GetId().ToString().c_str(), expected.GetAllocator());
    expected["PathToPrimary"][0]["TargetClassInfo"]["Id"].SetString(testClassB->GetId().ToString().c_str(), expected.GetAllocator());
    expected["PathToPrimary"][0]["RelationshipInfo"]["Id"].SetString(relClassAClassB->GetId().ToString().c_str(), expected.GetAllocator());
    expected["NestedFields"][1]["Properties"][0]["Property"]["BaseClassInfo"]["Id"].SetString(testClassA->GetId().ToString().c_str(), expected.GetAllocator());
    expected["NestedFields"][1]["Properties"][0]["Property"]["ActualClassInfo"]["Id"].SetString(testClassA->GetId().ToString().c_str(), expected.GetAllocator());


    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, DisplayLabelFieldSerializationHasContentFieldEditor)
    {
    ContentFieldEditor contentFieldEditor("ContentFieldEditorName");
    ContentDescriptor::DisplayLabelField field("Label", 10);
    field.SetEditor(&contentFieldEditor);
    rapidjson::Document actual = field.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Category": {
            "Name": "Miscellaneous",
            "DisplayLabel": "Miscellaneous",
            "Description": "",
            "Expand": false,
            "Priority": 1000
            },
        "Name": "DisplayLabel",
        "DisplayLabel": "Label",
        "Type": {
            "TypeName": "string",
            "ValueFormat": "Primitive"
            },
        "IsReadOnly": true,
        "Priority": 10,
        "Editor": {
            "Name": "ContentFieldEditorName",
            "Params": {}
            }
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnECPresentationSerializerTests, FieldEditorJsonParamsSerialization)
    {
    Json::Value json;
    json["Test1"] = 1;
    json["Test2"][0] = 2;
    json["Test2"][1] = 3;
    PropertyEditorJsonParameters spec(json);
    FieldEditorJsonParams params(spec);
    rapidjson::Document actual = params.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Test1": 1,
        "Test2": [2, 3]
        })");
    
    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnECPresentationSerializerTests, FieldEditorMultilineParamsSerialization)
    {
    PropertyEditorMultilineParameters spec(999);
    FieldEditorMultilineParams params(spec);
    rapidjson::Document actual = params.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "HeightInRows": 999
        })");
    
    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnECPresentationSerializerTests, FieldEditorRangeParamsSerialization)
    {
    PropertyEditorRangeParameters spec(123.33, 456.66);
    FieldEditorRangeParams params(spec);
    rapidjson::Document actual = params.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Minimum": 123.33,
        "Maximum": 456.66
        })");
    
    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, FieldEditorRangeParamsNoMinOrMaxSetSerialization)
    {
    PropertyEditorRangeParameters spec;
    FieldEditorRangeParams params(spec);
    rapidjson::Document actual = params.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Minimum": null,
        "Maximum": null
        })");
    
    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnECPresentationSerializerTests, FieldEditorSliderParamsSerialization)
    {
    PropertyEditorSliderParameters spec(123.33, 456.66, 5, 100, true);
    FieldEditorSliderParams params(spec);
    rapidjson::Document actual = params.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Minimum": 123.33,
        "Maximum": 456.66,
        "IntervalsCount": 5,
        "ValueFactor": 100,
        "IsVertical": true
        })");
    
    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, TypeDescriptionSerialization)
    {
    ContentDescriptor::Field::TypeDescriptionPtr typeDescription = new ContentDescriptor::Field::TypeDescription("TypeDescriptionName");
    rapidjson::Document actual = typeDescription->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "TypeName": "TypeDescriptionName"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, PrimitiveTypeDescriptionSerialization)
    {
    ContentDescriptor::Field::TypeDescriptionPtr typeDescription = new ContentDescriptor::Field::PrimitiveTypeDescription("TypeDescriptionName");
    rapidjson::Document actual = typeDescription->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "TypeName": "TypeDescriptionName",
        "ValueFormat": "Primitive"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ArrayTypeDescriptionSerialization)
    {
    ContentDescriptor::Field::TypeDescriptionPtr primitiveTypeDescription = new ContentDescriptor::Field::PrimitiveTypeDescription("PrimitiveTypeDescriptionName");
    ContentDescriptor::Field::TypeDescriptionPtr typeDescription = new ContentDescriptor::Field::ArrayTypeDescription(*primitiveTypeDescription);
    rapidjson::Document actual = typeDescription->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "TypeName": "PrimitiveTypeDescriptionName[]",
        "ValueFormat": "Array",
        "MemberType": {
            "TypeName": "PrimitiveTypeDescriptionName",
            "ValueFormat": "Primitive"
            }
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, StructTypeDescriptionSerialization)
    {
    ECStructClassCP testStruct = GetClass("TestStruct")->GetStructClassCP();
    ContentDescriptor::Field::TypeDescriptionPtr typeDescription = new ContentDescriptor::Field::StructTypeDescription(*testStruct);
    rapidjson::Document actual = typeDescription->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "TypeName": "TestStruct",
        "ValueFormat": "Struct",
        "Members": [
            {
            "Name": "IntProperty",
            "Label": "IntProperty",
            "Type": {
                "TypeName": "int",
                "ValueFormat": "Primitive"
                }
            },
            {
            "Name": "StringProperty",
            "Label": "StringProperty",
            "Type": {
                "TypeName": "string",
                "ValueFormat": "Primitive"
                }
            }
            ]
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, NestedContentTypeDescriptionSerialization)
    {
    ECClassCP testClassA = GetClass("PropertyTestClassA");
    ECClassCP testClassB = GetClass("PropertyTestClassB");
    ECRelationshipClassCP relClassAClassB = GetClass("TestClassAHasTestClassB")->GetRelationshipClassCP();
    ContentDescriptor::Category category("name", "label", "", 1);
    ContentDescriptor::NestedContentField field(category, "field_name", "field_label", *testClassA, "PropertyTestClassA",
        {RelatedClass(*testClassA, *testClassB, *relClassAClassB, false, "primary_instance", "rel")},
        {
        new ContentDescriptor::DisplayLabelField("NestedLabel", 10),
        new ContentDescriptor::ECPropertiesField(*testClassB, ContentDescriptor::Property("rel_RET_PropertyTestClassA_0", *testClassA, *testClassA->GetPropertyP("String")))
        });
    ContentDescriptor::Field::TypeDescriptionPtr typeDescription = new ContentDescriptor::Field::NestedContentTypeDescription(field);
    rapidjson::Document actual = typeDescription->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "TypeName": "PropertyTestClassA",
        "ValueFormat": "Struct",
        "Members": [
            {
            "Name": "DisplayLabel",
            "Label": "NestedLabel",
            "Type": {
                "TypeName": "string",
                "ValueFormat": "Primitive"
                }
            },
            {
            "Name": "PropertyTestClassA_String",
            "Label": "String",
            "Type": {
                "TypeName": "string",
                "ValueFormat": "Primitive"
                }
            }
            ]
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ECInstanceChangeResultSerializationSuccess)
    {
    ECInstanceChangeResult changeResult = ECInstanceChangeResult::Success(ECValue(456));
    rapidjson::Document actual = changeResult.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Status": 0,
        "Value": 456
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ECInstanceChangeResultSerializationSuccessValueUninitialized)
    {
    ECInstanceChangeResult changeResult = ECInstanceChangeResult::Success(ECValue());
    rapidjson::Document actual = changeResult.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Status": 1,
        "IgnoreReason": ""
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ECInstanceChangeResultSerializationIgnore)
    {
    ECInstanceChangeResult changeResult = ECInstanceChangeResult::Ignore("IgnoreMessage");
    rapidjson::Document actual = changeResult.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Status": 1,
        "IgnoreReason": "IgnoreMessage"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, NavNodeKeySerialization)
    {
    NavNodeKeyPtr key = NavNodeKey::Create("TypeName", {"123", "abc"});
    // Serialize
    rapidjson::Document actual = key->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Type": "TypeName",
        "PathFromRoot": [
            "123",
            "abc"
            ]
        })");

    ASSERT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);

    // Deserialize RapidJson
    NavNodeKeyPtr deserializedKey = NavNodeKey::FromJson(actual);

    EXPECT_EQ(key->GetType(), deserializedKey->GetType());
    EXPECT_EQ(key->GetPathFromRoot(), deserializedKey->GetPathFromRoot());

    // Deserialize JsonValue
    Json::Value expectedJsonValue;
    Json::Reader::Parse(BeRapidJsonUtilities::ToString(actual), expectedJsonValue);
    NavNodeKeyPtr deserializedKey2 = NavNodeKey::FromJson(expectedJsonValue);

    EXPECT_EQ(key->GetType(), deserializedKey2->GetType());
    EXPECT_EQ(key->GetPathFromRoot(), deserializedKey2->GetPathFromRoot());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ECInstanceNodeKeySerialization)
    {
    ECInstanceNodeKeyPtr key = ECInstanceNodeKey::Create(ECClassId((uint64_t)456), ECInstanceId(BeInt64Id(123)), {"123", "abc"});
    // Serialize
    rapidjson::Document actual = key->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Type": "ECInstanceNode",
        "PathFromRoot": [
            "123",
            "abc"
            ],
        "ECClassId": "456",
        "ECInstanceId": "123"
        })");

    ASSERT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);

    // Deserialize RapidJson
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(actual);
    ASSERT_TRUE(navNodeKey.IsValid());
    ECInstanceNodeKey const* deserializedKey = navNodeKey->AsECInstanceNodeKey();
    ASSERT_NE(nullptr, deserializedKey);

    EXPECT_EQ(key->GetType(), deserializedKey->GetType());
    EXPECT_EQ(key->GetPathFromRoot(), deserializedKey->GetPathFromRoot());
    EXPECT_EQ(key->GetInstanceKey(), deserializedKey->GetInstanceKey());

    // Deserialize JsonValue
    Json::Value expectedJsonValue;
    Json::Reader::Parse(BeRapidJsonUtilities::ToString(actual), expectedJsonValue);
    NavNodeKeyPtr navNodeKey2 = NavNodeKey::FromJson(expectedJsonValue);
    ASSERT_TRUE(navNodeKey2.IsValid());
    ECInstanceNodeKey const* deserializedKey2 = navNodeKey2->AsECInstanceNodeKey();
    ASSERT_NE(nullptr, deserializedKey2);

    EXPECT_EQ(key->GetType(), deserializedKey2->GetType());
    EXPECT_EQ(key->GetPathFromRoot(), deserializedKey2->GetPathFromRoot());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ECClassGroupingNodeKeySerialization)
    {
    ECClassGroupingNodeKeyPtr key = ECClassGroupingNodeKey::Create(ECClassId((uint64_t) 456), {"123", "abc"});
    //Serialize
    rapidjson::Document actual = key->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Type": "ECClassGroupingNode",
        "PathFromRoot": [
            "123",
            "abc"
            ],
        "ECClassId": "456"
        })");

    ASSERT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);

    // Deserialiaze RapidJson
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(actual);
    ASSERT_TRUE(navNodeKey.IsValid());
    ECClassGroupingNodeKey const* deserializedKey = navNodeKey->AsECClassGroupingNodeKey();
    ASSERT_NE(nullptr, deserializedKey);

    EXPECT_EQ(key->GetType(), deserializedKey->GetType());
    EXPECT_EQ(key->GetPathFromRoot(), deserializedKey->GetPathFromRoot());
    EXPECT_EQ(key->GetECClassId(), deserializedKey->GetECClassId());

    // Deserialize JsonValue
    Json::Value expectedJsonValue;
    Json::Reader::Parse(BeRapidJsonUtilities::ToString(actual), expectedJsonValue);
    NavNodeKeyPtr navNodeKey2 = NavNodeKey::FromJson(expectedJsonValue);
    ASSERT_TRUE(navNodeKey2.IsValid());
    ECClassGroupingNodeKey const* deserializedKey2 = navNodeKey2->AsECClassGroupingNodeKey();
    ASSERT_NE(nullptr, deserializedKey2);

    EXPECT_EQ(key->GetType(), deserializedKey2->GetType());
    EXPECT_EQ(key->GetPathFromRoot(), deserializedKey2->GetPathFromRoot());
    EXPECT_EQ(key->GetECClassId(), deserializedKey2->GetECClassId());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ECPropertyGroupingNodeKeySerializationNoGroupingValue)
    {
    ECPropertyGroupingNodeKeyPtr key = ECPropertyGroupingNodeKey::Create(ECClassId((uint64_t) 456), "PropertyNameText", nullptr, {"123", "abc"});
    // Serialize
    rapidjson::Document actual = key->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Type": "ECPropertyGroupingNode",
        "PathFromRoot": [
            "123",
            "abc"
            ],
        "ECClassId": "456",
        "PropertyName": "PropertyNameText"
        })");

    ASSERT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);

    // Deserialiaze RapidJson
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(actual);
    ASSERT_TRUE(navNodeKey.IsValid());
    ECPropertyGroupingNodeKey const* deserializedKey = navNodeKey->AsECPropertyGroupingNodeKey();
    ASSERT_NE(nullptr, deserializedKey);

    EXPECT_EQ(key->GetType(), deserializedKey->GetType());
    EXPECT_EQ(key->GetPathFromRoot(), deserializedKey->GetPathFromRoot());
    EXPECT_EQ(key->GetECClassId(), deserializedKey->GetECClassId());
    EXPECT_EQ(key->GetPropertyName(), deserializedKey->GetPropertyName());
    EXPECT_EQ(key->GetGroupingValue(), deserializedKey->GetGroupingValue());

    // Deserialize JsonValue
    Json::Value expectedJsonValue;
    Json::Reader::Parse(BeRapidJsonUtilities::ToString(actual), expectedJsonValue);
    NavNodeKeyPtr navNodeKey2 = NavNodeKey::FromJson(expectedJsonValue);
    ASSERT_TRUE(navNodeKey2.IsValid());
    ECPropertyGroupingNodeKey const* deserializedKey2 = navNodeKey2->AsECPropertyGroupingNodeKey();
    ASSERT_NE(nullptr, deserializedKey2);

    EXPECT_EQ(key->GetType(), deserializedKey2->GetType());
    EXPECT_EQ(key->GetPathFromRoot(), deserializedKey2->GetPathFromRoot());
    EXPECT_EQ(key->GetECClassId(), deserializedKey2->GetECClassId());
    EXPECT_EQ(key->GetPropertyName(), deserializedKey2->GetPropertyName());
    EXPECT_EQ(key->GetGroupingValue(), deserializedKey2->GetGroupingValue());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ECPropertyGroupingNodeKeySerializationWithGroupingValue)
    {
    rapidjson::Document groupingValue;
    groupingValue.SetString("GroupingValueText");
    ECPropertyGroupingNodeKeyPtr key = ECPropertyGroupingNodeKey::Create(ECClassId((uint64_t) 456), "PropertyNameText", &groupingValue, {"123", "abc"});
    // Serialize
    rapidjson::Document actual = key->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Type": "ECPropertyGroupingNode",
        "PathFromRoot": [
            "123",
            "abc"
            ],
        "ECClassId": "456",
        "PropertyName": "PropertyNameText",
        "GroupingValue": "GroupingValueText"
        })");

    ASSERT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);

    // Deserialiaze
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(actual);
    ASSERT_TRUE(navNodeKey.IsValid());
    ECPropertyGroupingNodeKey const* deserializedKey = navNodeKey->AsECPropertyGroupingNodeKey();
    ASSERT_NE(nullptr, deserializedKey);

    EXPECT_EQ(key->GetType(), deserializedKey->GetType());
    EXPECT_EQ(key->GetPathFromRoot(), deserializedKey->GetPathFromRoot());
    EXPECT_EQ(key->GetECClassId(), deserializedKey->GetECClassId());
    EXPECT_EQ(key->GetPropertyName(), deserializedKey->GetPropertyName());
    EXPECT_EQ(*key->GetGroupingValue(), *deserializedKey->GetGroupingValue());

    // Deserialize JsonValue
    Json::Value expectedJsonValue;
    Json::Reader::Parse(BeRapidJsonUtilities::ToString(actual), expectedJsonValue);
    NavNodeKeyPtr navNodeKey2 = NavNodeKey::FromJson(expectedJsonValue);
    ASSERT_TRUE(navNodeKey2.IsValid());
    ECPropertyGroupingNodeKey const* deserializedKey2 = navNodeKey2->AsECPropertyGroupingNodeKey();
    ASSERT_NE(nullptr, deserializedKey2);

    EXPECT_EQ(key->GetType(), deserializedKey2->GetType());
    EXPECT_EQ(key->GetPathFromRoot(), deserializedKey2->GetPathFromRoot());
    EXPECT_EQ(key->GetECClassId(), deserializedKey2->GetECClassId());
    EXPECT_EQ(key->GetPropertyName(), deserializedKey2->GetPropertyName());
    EXPECT_EQ(*key->GetGroupingValue(), *deserializedKey2->GetGroupingValue());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, LabelGroupingNodeKeySerialization)
    {
    LabelGroupingNodeKeyPtr key = LabelGroupingNodeKey::Create("LabelText", {"123", "abc"});
    // Serialize
    rapidjson::Document actual = key->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Type": "DisplayLabelGroupingNode",
        "PathFromRoot": [
            "123",
            "abc"
            ],
        "Label": "LabelText"
        })");

    ASSERT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);

    // Deserialiaze
    NavNodeKeyPtr navNodeKey = NavNodeKey::FromJson(actual);
    ASSERT_TRUE(navNodeKey.IsValid());
    LabelGroupingNodeKey const* deserializedKey = navNodeKey->AsLabelGroupingNodeKey();
    ASSERT_NE(nullptr, deserializedKey);

    EXPECT_EQ(key->GetType(), deserializedKey->GetType());
    EXPECT_EQ(key->GetPathFromRoot(), deserializedKey->GetPathFromRoot());
    EXPECT_EQ(key->GetLabel(), deserializedKey->GetLabel());

    // Deserialize JsonValue
    Json::Value expectedJsonValue;
    Json::Reader::Parse(BeRapidJsonUtilities::ToString(actual), expectedJsonValue);
    NavNodeKeyPtr navNodeKey2 = NavNodeKey::FromJson(expectedJsonValue);
    ASSERT_TRUE(navNodeKey2.IsValid());
    LabelGroupingNodeKey const* deserializedKey2 = navNodeKey2->AsLabelGroupingNodeKey();
    ASSERT_NE(nullptr, deserializedKey2);

    EXPECT_EQ(key->GetType(), deserializedKey2->GetType());
    EXPECT_EQ(key->GetPathFromRoot(), deserializedKey2->GetPathFromRoot());
    EXPECT_EQ(key->GetLabel(), deserializedKey2->GetLabel());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, NodesPathElementSerializationNoNavNode)
    {
    NodesPathElement nodesPathElement;
    rapidjson::Document actual = nodesPathElement.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"(null)");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, NodesPathElementSerializationWithNode)
    {
    TestNavNodePtr node = TestNavNode::Create();
    NodesPathElement nodesPathElement(*node, 10);
    TestNavNodePtr nodeChild = TestNavNode::Create();
    nodeChild->SetLabel("ChildTestLabeld");
    NodesPathElement nodesPathElementChild(*nodeChild, 9);
    nodesPathElementChild.SetIsMarked(true);
    nodesPathElement.GetChildren().push_back(nodesPathElementChild);
    rapidjson::Document actual = nodesPathElement.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Node": {
            "NodeId": "0",
            "ParentNodeId": "0",
            "Key": {
                "Type": "TestType",
                "PathFromRoot": []
                },
            "Label": "TestLabel",
            "Description": "",
            "ExpandedImageId": "",
            "CollapsedImageId": "",
            "ForeColor": "",
            "BackColor": "",
            "FontStyle": "Regular",
            "Type": "TestType",
            "HasChildren": false,
            "IsSelectable": true,
            "IsEditable": false,
            "IsChecked": false,
            "IsCheckboxVisible": false,
            "IsCheckboxEnabled": false,
            "IsExpanded": false,
            "ECInstanceId": "0"
            },
        "Index": 10,
        "IsMarked": false,
        "Children": [
            {
            "Node": {
                "NodeId": "0",
                "ParentNodeId": "0",
                "Key": {
                    "Type": "TestType",
                    "PathFromRoot": []
                    },
                "Label": "ChildTestLabeld",
                "Description": "",
                "ExpandedImageId": "",
                "CollapsedImageId": "",
                "ForeColor": "",
                "BackColor": "",
                "FontStyle": "Regular",
                "Type": "TestType",
                "HasChildren": false,
                "IsSelectable": true,
                "IsEditable": false,
                "IsChecked": false,
                "IsCheckboxVisible": false,
                "IsCheckboxEnabled": false,
                "IsExpanded": false,
                "ECInstanceId": "0"
                },
            "Index": 9,
            "IsMarked": true,
            "Children": []
            }
        ]
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, KeySetSerialization)
    {
    KeySetPtr keySet = KeySet::Create(bvector<ECInstanceKey>{
        ECInstanceKey(ECClassId((uint64_t) 55), ECInstanceId((uint64_t) 1)),
        ECInstanceKey(ECClassId((uint64_t) 56), ECInstanceId((uint64_t) 2))
        });
    keySet->Add(*NavNodeKey::Create("TypeName", {"123", "abc"}));

    // Serialize
    rapidjson::Document actual = keySet->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "InstanceKeys": {
            "55": [
                1
                ],
            "56": [
                2
                ]
            },
        "NodeKeys": [
            {
            "Type": "TypeName",
            "PathFromRoot": [
                "123",
                "abc"
                ]
            }
            ]
        })");

    ASSERT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);

    //Deserialize
    Json::Value expectedJson;
    Json::Reader::Parse(BeRapidJsonUtilities::ToString(actual), expectedJson);
    KeySetPtr deserializedKeySet = KeySet::FromJson(expectedJson);

    EXPECT_EQ(keySet->GetInstanceKeys(), deserializedKeySet->GetInstanceKeys());
    EXPECT_EQ(keySet->GetNavNodeKeys().size(), deserializedKeySet->GetNavNodeKeys().size());
    for (NavNodeKeyCPtr navNodeKey : deserializedKeySet->GetNavNodeKeys())
        EXPECT_TRUE(keySet->GetNavNodeKeys().end() != keySet->GetNavNodeKeys().find(navNodeKey));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, SelectionChangedEventSerialization)
    {
    RefCountedPtr<TestConnection> connection = new TestConnection(s_project->GetECDb());
    KeySetPtr keySet = KeySet::Create(bvector<ECInstanceKey>{ECInstanceKey(ECClassId((uint64_t) 55), ECInstanceId((uint64_t) 1))});
    SelectionChangedEventPtr selectionChangedEvent = SelectionChangedEvent::Create(*connection, "SourceNameText", SelectionChangeType::Add, false, *keySet);
    selectionChangedEvent->GetExtendedDataR().AddMember("ExtendedDataPropertyName", "ExtendedDataProperty", selectionChangedEvent->GetExtendedDataAllocator());
    // Serialize
    rapidjson::Document actual = selectionChangedEvent->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "ConnectionId": "",
        "Source": "SourceNameText",
        "IsSubSelection": false,
        "ChangeType": 0,
        "Keys": {
            "InstanceKeys": {
                "55": [
                    1
                    ]
                },
            "NodeKeys": []
            },
        "ExtendedData": {
            "ExtendedDataPropertyName": "ExtendedDataProperty"
            }
        })");
    expected["ConnectionId"].SetString(connection->GetId().c_str(), expected.GetAllocator());

    ASSERT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);

    //Deserialize
    TestConnectionCache cache;
    cache.m_connections.Insert(connection->GetId(), connection);
    Json::Value expectedJson;
    Json::Reader::Parse(BeRapidJsonUtilities::ToString(actual), expectedJson);
    SelectionChangedEventPtr deserializedChangedEvent = SelectionChangedEvent::FromJson(cache, expectedJson);

    EXPECT_EQ(selectionChangedEvent->GetConnection().GetId(), deserializedChangedEvent->GetConnection().GetId());
    EXPECT_EQ(selectionChangedEvent->GetSourceName(), deserializedChangedEvent->GetSourceName());
    EXPECT_EQ(selectionChangedEvent->GetChangeType(), deserializedChangedEvent->GetChangeType());
    EXPECT_EQ(selectionChangedEvent->IsSubSelection(), deserializedChangedEvent->IsSubSelection());
    EXPECT_EQ(selectionChangedEvent->GetSelectedKeys(), deserializedChangedEvent->GetSelectedKeys());
    EXPECT_EQ(selectionChangedEvent->GetExtendedData(), deserializedChangedEvent->GetExtendedData());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, SelectionChangedEventDeserializationNoExtendedData)
    {
    RefCountedPtr<TestConnection> connection = new TestConnection(s_project->GetECDb());
    rapidjson::Document expected;
    expected.Parse(R"({
        "ConnectionId": "",
        "Source": "SourceNameText",
        "IsSubSelection": false,
        "ChangeType": 0,
        "Keys": {
            "InstanceKeys": {
                "55": [
                    1
                    ]
                },
            "NodeKeys": []
            }
        })");
    expected["ConnectionId"].SetString(connection->GetId().c_str(), expected.GetAllocator());

    //Deserialize
    TestConnectionCache cache;
    cache.m_connections.Insert(connection->GetId(), connection);
    Json::Value expectedJson;
    Json::Reader::Parse(BeRapidJsonUtilities::ToString(expected), expectedJson);
    SelectionChangedEventPtr deserializedChangedEvent = SelectionChangedEvent::FromJson(cache, expectedJson);

    EXPECT_EQ(connection->GetId(), deserializedChangedEvent->GetConnection().GetId());
    EXPECT_EQ("SourceNameText", deserializedChangedEvent->GetSourceName());
    EXPECT_EQ(SelectionChangeType::Add, deserializedChangedEvent->GetChangeType());
    EXPECT_EQ(false, deserializedChangedEvent->IsSubSelection());
    EXPECT_FALSE(deserializedChangedEvent->GetSelectedKeys().empty());
    EXPECT_TRUE(deserializedChangedEvent->GetExtendedData().ObjectEmpty());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, SelectionChangedEventDeserializationExtendedDataIsArray)
    {
    RefCountedPtr<TestConnection> connection = new TestConnection(s_project->GetECDb());
    rapidjson::Document expected;
    expected.Parse(R"({
        "ConnectionId": "",
        "Source": "SourceNameText",
        "IsSubSelection": false,
        "ChangeType": 0,
        "Keys": {
            "InstanceKeys": {
                "55": [
                    1
                    ]
                },
            "NodeKeys": []
            },
        "ExtendedData": []
        })");
    expected["ConnectionId"].SetString(connection->GetId().c_str(), expected.GetAllocator());

    //Deserialize
    TestConnectionCache cache;
    cache.m_connections.Insert(connection->GetId(), connection);
    Json::Value expectedJson;
    Json::Reader::Parse(BeRapidJsonUtilities::ToString(expected), expectedJson);
    SelectionChangedEventPtr deserializedChangedEvent = SelectionChangedEvent::FromJson(cache, expectedJson);

    EXPECT_EQ(connection->GetId(), deserializedChangedEvent->GetConnection().GetId());
    EXPECT_EQ("SourceNameText", deserializedChangedEvent->GetSourceName());
    EXPECT_EQ(SelectionChangeType::Add, deserializedChangedEvent->GetChangeType());
    EXPECT_EQ(false, deserializedChangedEvent->IsSubSelection());
    EXPECT_FALSE(deserializedChangedEvent->GetSelectedKeys().empty());
    EXPECT_TRUE(deserializedChangedEvent->GetExtendedData().ObjectEmpty());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, SelectionChangedEventDeserializationExtendedDataIsEmptyObject)
    {
    RefCountedPtr<TestConnection> connection = new TestConnection(s_project->GetECDb());
    rapidjson::Document expected;
    expected.Parse(R"({
        "ConnectionId": "",
        "Source": "SourceNameText",
        "IsSubSelection": false,
        "ChangeType": 0,
        "Keys": {
            "InstanceKeys": {
                "55": [
                    1
                    ]
                },
            "NodeKeys": []
            },
        "ExtendedData": {}
        })");
    expected["ConnectionId"].SetString(connection->GetId().c_str(), expected.GetAllocator());

    //Deserialize
    TestConnectionCache cache;
    cache.m_connections.Insert(connection->GetId(), connection);
    Json::Value expectedJson;
    Json::Reader::Parse(BeRapidJsonUtilities::ToString(expected), expectedJson);
    SelectionChangedEventPtr deserializedChangedEvent = SelectionChangedEvent::FromJson(cache, expectedJson);

    EXPECT_EQ(connection->GetId(), deserializedChangedEvent->GetConnection().GetId());
    EXPECT_EQ("SourceNameText", deserializedChangedEvent->GetSourceName());
    EXPECT_EQ(SelectionChangeType::Add, deserializedChangedEvent->GetChangeType());
    EXPECT_EQ(false, deserializedChangedEvent->IsSubSelection());
    EXPECT_FALSE(deserializedChangedEvent->GetSelectedKeys().empty());
    EXPECT_TRUE(deserializedChangedEvent->GetExtendedData().ObjectEmpty());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ContentDescriptorSerializationNoSelectionInfo)
    {
    ECClassCP testClassA = GetClass("PropertyTestClassA");
    ECClassCP testClassB = GetClass("PropertyTestClassB");
    ECRelationshipClassCP relClassAClassB = GetClass("TestClassAHasTestClassB")->GetRelationshipClassCP();
    IConnectionPtr connection = new TestConnection(s_project->GetECDb());
    RulesDrivenECPresentationManager::ContentOptions options("rulesetIdText");
    INavNodeKeysContainerCPtr container = NavNodeKeyListContainer::Create(bvector<NavNodeKeyCPtr>{
        NavNodeKey::Create("TypeName", {"123", "abc"}),
        NavNodeKey::Create("TypeName2", {"456", "def"})
        });
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*connection, options.GetJson(), *container, "DisplayTypeText");
    SelectClassInfo selectClassInfo(*testClassA, false);
    selectClassInfo.SetPathToPrimaryClass({RelatedClass(*testClassA, *testClassB, *relClassAClassB, false)});
    selectClassInfo.SetRelatedPropertyPaths({{RelatedClass(*testClassA, *testClassB, *relClassAClassB, false)}});
    descriptor->GetSelectClasses().push_back(selectClassInfo);
    descriptor->AddField(new ContentDescriptor::DisplayLabelField("Label", 10));
    descriptor->SetSortingField(0);
    descriptor->SetSortDirection(SortDirection::Descending);
    descriptor->SetContentFlags((int)ContentFlags::ShowLabels);
    descriptor->SetFilterExpression("ExpressionText");
    rapidjson::Document actual = descriptor->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "PreferredDisplayType": "DisplayTypeText",
        "SelectClasses": [
            {
            "SelectClassInfo": {
                "Id": "",
                "Name": "TestSchema:PropertyTestClassA",
                "Label": "PropertyTestClassA"
                },
            "IsPolymorphic": false,
            "PathToPrimaryClass": [
                {
                "SourceClassInfo": {
                    "Id": "",
                    "Name": "TestSchema:PropertyTestClassA",
                    "Label": "PropertyTestClassA"
                    },
                "TargetClassInfo": {
                    "Id": "",
                    "Name": "TestSchema:PropertyTestClassB",
                    "Label": "PropertyTestClassB"
                    },
                "RelationshipInfo": {
                    "Id": "",
                    "Name": "TestSchema:TestClassAHasTestClassB",
                    "Label": "TestClassAHasTestClassB"
                    },
                "IsForwardRelationship": false,
                "IsPolymorphicRelationship": true
                }
                ],
            "RelatedPropertyPaths": [
                    [
                    {
                    "SourceClassInfo": {
                        "Id": "",
                        "Name": "TestSchema:PropertyTestClassA",
                        "Label": "PropertyTestClassA"
                        },
                    "TargetClassInfo": {
                        "Id": "",
                        "Name": "TestSchema:PropertyTestClassB",
                        "Label": "PropertyTestClassB"
                        },
                    "RelationshipInfo": {
                        "Id": "",
                        "Name": "TestSchema:TestClassAHasTestClassB",
                        "Label": "TestClassAHasTestClassB"
                        },
                    "IsForwardRelationship": false,
                    "IsPolymorphicRelationship": true
                    }
                    ]
                ]
            }
            ],
        "Fields": [
            {
            "Category": {
                "Name": "Miscellaneous",
                "DisplayLabel": "Miscellaneous",
                "Description": "",
                "Expand": false,
                "Priority": 1000
                },
            "Name": "DisplayLabel",
            "DisplayLabel": "Label",
            "Type": {
                "TypeName": "string",
                "ValueFormat": "Primitive"
                },
            "IsReadOnly": true,
            "Priority": 10
            }
            ],
        "SortingFieldIndex": 0,
        "SortDirection": 1,
        "ContentFlags": 4,
        "ConnectionId": "",
        "FilterExpression": "ExpressionText",
        "InputKeysHash": "",
        "ContentOptions": {
            "RulesetId": "rulesetIdText"
            }
        })");

    expected["ConnectionId"].SetString(connection->GetId().c_str(), expected.GetAllocator());
    expected["InputKeysHash"].SetString(container->GetHash().c_str(), expected.GetAllocator());
    expected["SelectClasses"][0]["SelectClassInfo"]["Id"].SetString(testClassA->GetId().ToString().c_str(), expected.GetAllocator());
    expected["SelectClasses"][0]["PathToPrimaryClass"][0]["SourceClassInfo"]["Id"].SetString(testClassA->GetId().ToString().c_str(), expected.GetAllocator());
    expected["SelectClasses"][0]["PathToPrimaryClass"][0]["TargetClassInfo"]["Id"].SetString(testClassB->GetId().ToString().c_str(), expected.GetAllocator());
    expected["SelectClasses"][0]["PathToPrimaryClass"][0]["RelationshipInfo"]["Id"].SetString(relClassAClassB->GetId().ToString().c_str(), expected.GetAllocator());
    expected["SelectClasses"][0]["RelatedPropertyPaths"][0][0]["SourceClassInfo"]["Id"].SetString(testClassA->GetId().ToString().c_str(), expected.GetAllocator());
    expected["SelectClasses"][0]["RelatedPropertyPaths"][0][0]["TargetClassInfo"]["Id"].SetString(testClassB->GetId().ToString().c_str(), expected.GetAllocator());
    expected["SelectClasses"][0]["RelatedPropertyPaths"][0][0]["RelationshipInfo"]["Id"].SetString(relClassAClassB->GetId().ToString().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ContentDescriptorSerializationWithSelectionInfo)
    {
    IConnectionPtr connection = new TestConnection(s_project->GetECDb());
    RulesDrivenECPresentationManager::ContentOptions options("rulesetIdText");
    INavNodeKeysContainerCPtr container = NavNodeKeyListContainer::Create(bvector<NavNodeKeyCPtr>{
        NavNodeKey::Create("TypeName", {"123", "abc"}),
        NavNodeKey::Create("TypeName2", {"456", "def"})
        });
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*connection, options.GetJson(), *container, "DisplayTypeText");
    descriptor->SetSelectionInfo(*(new SelectionInfo("ProviderNameText", true)));
    rapidjson::Document actual = descriptor->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "PreferredDisplayType": "DisplayTypeText",
        "SelectClasses": [],
        "Fields": [],
        "SortingFieldIndex": -1,
        "SortDirection": 0,
        "ContentFlags": 0,
        "ConnectionId": "",
        "FilterExpression": "",
        "InputKeysHash": "",
        "ContentOptions": {
            "RulesetId": "rulesetIdText"
            },
        "SelectionInfo": {
            "SelectionProvider": "ProviderNameText",
            "IsSubSelection": true
            }
        })");
    expected["ConnectionId"].SetString(connection->GetId().c_str(), expected.GetAllocator());
    expected["InputKeysHash"].SetString(container->GetHash().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ContentSetSerializationItemWithClassSet)
    {
    ECClassCP testClass = GetClass("PropertyTestClassA");
    ContentDescriptor::ECPropertiesField field(*testClass, ContentDescriptor::Property("this", *testClass, *testClass->GetPropertyP("String")));
    bvector<ECInstanceKey> keys {
        ECInstanceKey(ECClassId((uint64_t) 55), ECInstanceId((uint64_t) 1)),
        ECInstanceKey(ECClassId((uint64_t) 56), ECInstanceId((uint64_t) 2))
        };
    rapidjson::Document values(rapidjson::kObjectType);
    values.AddMember("FieldName", "FieldValue", values.GetAllocator());
    rapidjson::Document displayValues(rapidjson::kObjectType);
    displayValues.AddMember("FieldName", "FieldDisplayValue", values.GetAllocator());
    bvector<Utf8String> mergedFieldNames {"MergedField1", "MergedField2"};
    ContentDescriptor::ECPropertiesField ecPropertiesField(*testClass, ContentDescriptor::Property("this", *testClass, *testClass->GetPropertyP("String")));
    ContentDescriptor::ECPropertiesField ecPropertiesField1(*testClass, ContentDescriptor::Property("this", *testClass, *testClass->GetPropertyP("Int")));
    ContentSetItem::FieldPropertyInstanceKeyMap fieldPropertyInstanceKeys;
    fieldPropertyInstanceKeys.Insert(ContentSetItem::FieldProperty(ecPropertiesField, 0), keys);
    fieldPropertyInstanceKeys.Insert(ContentSetItem::FieldProperty(ecPropertiesField1, 1), keys);
    ContentSetItemPtr contentSetItem = ContentSetItem::Create(keys, "DisplayLabelText", "ImageIdText", std::move(values),
                                                              std::move(displayValues), mergedFieldNames, std::move(fieldPropertyInstanceKeys));
    contentSetItem->SetClass(testClass);
    rapidjson::Document actual = contentSetItem->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "DisplayLabel": "DisplayLabelText",
        "ImageId": "ImageIdText",
        "Values": {
            "FieldName": "FieldValue"
            },
        "DisplayValues": {
            "FieldName": "FieldDisplayValue"
            },
        "ClassInfo": {
            "Id": "",
            "Name": "TestSchema:PropertyTestClassA",
            "Label": "PropertyTestClassA"
            },
        "PrimaryKeys": [
            {
            "ECClassId": "55",
            "ECInstanceId": "1"
            },
            {
            "ECClassId": "56",
            "ECInstanceId": "2"
            }
            ],
        "MergedFieldNames": [
            "MergedField1",
            "MergedField2"
            ],
        "FieldValueKeys": {
            "PropertyTestClassA_String": [
                {
                "PropertyIndex": 0,
                "Keys": [
                    {
                    "ECClassId": "55",
                    "ECInstanceId": "1"
                    },
                    {
                    "ECClassId": "56",
                    "ECInstanceId": "2"
                    }
                    ]
                }
                ],
            "PropertyTestClassA_Int": [
                {
                "PropertyIndex": 1,
                "Keys": [
                    {
                    "ECClassId": "55",
                    "ECInstanceId": "1"
                    },
                    {
                    "ECClassId": "56",
                    "ECInstanceId": "2"
                    }
                    ]
                }
                ]
            }
        })");
    expected["ClassInfo"]["Id"].SetString(testClass->GetId().ToString().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ContentSetItemSerializationNoClassSet)
    {
    rapidjson::Document values(rapidjson::kObjectType);
    rapidjson::Document displayValues(rapidjson::kObjectType);
    ContentSetItemPtr contentSetItem = ContentSetItem::Create({ECInstanceKey()}, "", "", std::move(values), std::move(displayValues),
                                                              bvector<Utf8String>(), bmap<ContentSetItem::FieldProperty, bvector<ECInstanceKey>>());
    rapidjson::Document actual = contentSetItem->AsJson(ContentSetItem::SerializationFlags::SERIALIZE_ClassInfo);

    rapidjson::Document expected;
    expected.Parse(R"({
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ContentSerialization)
    {
    IConnectionPtr connection = new TestConnection(s_project->GetECDb());
    RulesDrivenECPresentationManager::ContentOptions options("rulesetIdText");
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*connection, options.GetJson(),
                                                                *NavNodeKeyListContainer::Create(bvector<NavNodeKeyCPtr>{}), "DisplayTypeText");

    rapidjson::Document values(rapidjson::kObjectType);
    rapidjson::Document displayValues(rapidjson::kObjectType);
    ContentSetItemPtr contentSetItem = ContentSetItem::Create({ECInstanceKey()}, "", "", std::move(values), std::move(displayValues),
                                                              bvector<Utf8String>(), bmap<ContentSetItem::FieldProperty, bvector<ECInstanceKey>>());
    RefCountedPtr<TestDataSource> dataSource = TestDataSource::Create();
    dataSource->AddContentSetItem(contentSetItem);

    ContentPtr content = Content::Create(*descriptor, *dataSource);
    rapidjson::Document actual = content->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Descriptor": {
            "PreferredDisplayType": "DisplayTypeText",
            "SelectClasses": [],
            "Fields": [],
            "SortingFieldIndex": -1,
            "SortDirection": 0,
            "ContentFlags": 0,
            "ConnectionId": "",
            "FilterExpression": "",
            "InputKeysHash": "d41d8cd98f00b204e9800998ecf8427e",
            "ContentOptions": {
                "RulesetId": "rulesetIdText"
                }
            },
        "ContentSet": [
            {
            "DisplayLabel": "",
            "ImageId": "",
            "Values": {},
            "DisplayValues": {},
            "PrimaryKeys": [
                {
                "ECClassId": "0",
                "ECInstanceId": "0"
                }
            ],
            "MergedFieldNames": [],
            "FieldValueKeys": {}
            }
            ]
        })");
    expected["Descriptor"]["ConnectionId"].SetString(connection->GetId().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, ContenteSerializationContentSetItemNotValid)
    {
    IConnectionPtr connection = new TestConnection(s_project->GetECDb());
    RulesDrivenECPresentationManager::ContentOptions options("rulesetIdText");
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*connection, options.GetJson(),
                                                                *NavNodeKeyListContainer::Create(bvector<NavNodeKeyCPtr>{}), "DisplayTypeText");

    ContentSetItemPtr contentSetItem = nullptr;
    RefCountedPtr<TestDataSource> dataSource = TestDataSource::Create();
    dataSource->AddContentSetItem(contentSetItem);

    ContentPtr content = Content::Create(*descriptor, *dataSource);
    rapidjson::Document actual = content->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "Descriptor": {
            "PreferredDisplayType": "DisplayTypeText",
            "SelectClasses": [],
            "Fields": [],
            "SortingFieldIndex": -1,
            "SortDirection": 0,
            "ContentFlags": 0,
            "ConnectionId": "",
            "FilterExpression": "",
            "InputKeysHash": "d41d8cd98f00b204e9800998ecf8427e",
            "ContentOptions": {
                "RulesetId": "rulesetIdText"
                }
            },
        "ContentSet": []
        })");
    expected["Descriptor"]["ConnectionId"].SetString(connection->GetId().c_str(), expected.GetAllocator());

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Mantas.Kontrimas                03/2018
//---------------------------------------------------------------------------------------
TEST_F(DgnECPresentationSerializerTests, NavNodeSerialization)
    {
    IConnectionPtr connection = new TestConnection(s_project->GetECDb());
    TestNavNodePtr navNode = TestNavNode::Create(connection.get());
    navNode->SetNodeId(10);
    navNode->SetParentNodeId(12);
    navNode->SetNodeKey(*NavNodeKey::Create("TypeName", {"123", "abc"}));
    navNode->SetLabel("TestNodeLabel");
    navNode->SetDescription("TestNodeDescription");
    navNode->SetExpandedImageId("EImageId");
    navNode->SetCollapsedImageId("CImageId");
    navNode->SetForeColor("FColor");
    navNode->SetBackColor("BColor");
    navNode->SetFontStyle("FStyle");
    navNode->SetType("CustomTestNode");
    navNode->SetHasChildren(false);
    navNode->SetIsSelectable(true);
    navNode->SetIsEditable(false);
    navNode->SetIsChecked(false);
    navNode->SetIsCheckboxVisible(false);
    navNode->SetIsCheckboxEnabled(false);
    navNode->SetIsExpanded(true);
    navNode->SetInstanceId(132);
    // Serialize
    rapidjson::Document actual = navNode->AsJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "NodeId": "10",
        "ParentNodeId": "12",
        "Key": {
            "Type": "TypeName",
            "PathFromRoot": [
                "123",
                "abc"
            ]
            },
        "Label": "TestNodeLabel",
        "Description": "TestNodeDescription",
        "ExpandedImageId": "EImageId",
        "CollapsedImageId": "CImageId",
        "ForeColor": "FColor",
        "BackColor": "BColor",
        "FontStyle": "FStyle",
        "Type": "CustomTestNode",
        "HasChildren": false,
        "IsSelectable": true,
        "IsEditable": false,
        "IsChecked": false,
        "IsCheckboxVisible": false,
        "IsCheckboxEnabled": false,
        "IsExpanded": true,
        "ECInstanceId": "132"
        })");

    ASSERT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);

    //Deserialize
    NavNodePtr deserializedNavNode = NavNode::FromJson(actual);

    EXPECT_EQ(navNode->GetInstanceId(), deserializedNavNode->GetInstanceId());
    EXPECT_EQ(navNode->GetNodeId(), deserializedNavNode->GetNodeId());
    EXPECT_EQ(navNode->GetParentNodeId(), deserializedNavNode->GetParentNodeId());
    EXPECT_EQ(*navNode->GetKey(), *deserializedNavNode->GetKey());
    EXPECT_EQ(navNode->GetLabel(), deserializedNavNode->GetLabel());
    EXPECT_EQ(navNode->GetDescription(), deserializedNavNode->GetDescription());
    EXPECT_EQ(navNode->GetExpandedImageId(), deserializedNavNode->GetExpandedImageId());
    EXPECT_EQ(navNode->GetCollapsedImageId(), deserializedNavNode->GetCollapsedImageId());
    EXPECT_EQ(navNode->GetForeColor(), deserializedNavNode->GetForeColor());
    EXPECT_EQ(navNode->GetBackColor(), deserializedNavNode->GetBackColor());
    EXPECT_EQ(navNode->GetFontStyle(), deserializedNavNode->GetFontStyle());
    EXPECT_EQ(navNode->GetType(), deserializedNavNode->GetType());
    EXPECT_EQ(navNode->HasChildren(), deserializedNavNode->HasChildren());
    EXPECT_EQ(navNode->IsSelectable(), deserializedNavNode->IsSelectable());
    EXPECT_EQ(navNode->IsEditable(), deserializedNavNode->IsEditable());
    EXPECT_EQ(navNode->IsChecked(), deserializedNavNode->IsChecked());
    EXPECT_EQ(navNode->IsCheckboxEnabled(), deserializedNavNode->IsCheckboxEnabled());
    EXPECT_EQ(navNode->IsExpanded(), deserializedNavNode->IsExpanded());
    EXPECT_EQ(navNode->GetExtendedData().ObjectEmpty(), deserializedNavNode->GetExtendedData().ObjectEmpty());
    }