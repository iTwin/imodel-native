/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/TestSchemaHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TestSchemaHelper.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

//static member initialization
Utf8CP const TestSchemaHelper::TESTSCHEMA_NAME = "ComplexTestSchema";
WCharCP const TestSchemaHelper::TESTCLASS_NAME = L"PIPE_Extra";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static 
ECSchemaPtr TestSchemaHelper::CreateComplexTestSchema 
(
ECSchemaReadContextPtr& schemaReadContext
)
    {
    auto schemaXml = GetComplexTestSchemaXml ();
    return DeserializeSchema (schemaReadContext, schemaXml);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static 
ECSchemaPtr TestSchemaHelper::CreateTestSchema 
(
ECSchemaReadContextPtr& schemaReadContext,
int stringPropCount, 
int integerPropCount
)
    {
    //at least one of the two input args must be greater than 0
    EXPECT_TRUE (integerPropCount >= 0 && stringPropCount >= 0 && (integerPropCount + stringPropCount) > 0);
    auto schemaXml = GetTestSchemaXml (stringPropCount, integerPropCount);
    return DeserializeSchema (schemaReadContext, schemaXml);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
ECSchemaPtr TestSchemaHelper::DeserializeSchema
(
ECSchemaReadContextPtr& schemaReadContext,
Utf8StringCR schemaXml
)
    {
    schemaReadContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    auto ecSchemaStatus = ECSchema::ReadFromXmlString (schema, schemaXml.c_str (), *schemaReadContext);
    EXPECT_EQ (SchemaReadStatus::Success, ecSchemaStatus);
    EXPECT_TRUE (schema.IsValid ());
    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static 
Utf8String TestSchemaHelper::GetTestSchemaXml
(
int integerPropCount,
int stringPropCount
)
    {
    Utf8String schemaXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"p\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                    "    <ECClass typeName=\"PIPE_Extra\" isDomainClass=\"True\">";

    for (int i = 0; i < integerPropCount; ++i)
        {
        Utf8Char propertyName[32];
        BeStringUtilities::Snprintf (propertyName, "IntegerProperty%02d", i);
        Utf8CP propertyFormat = "        <ECProperty propertyName=\"%s\" typeName=\"int\" />";
        Utf8Char propertyXml[128];
        BeStringUtilities::Snprintf (propertyXml, propertyFormat, propertyName);
        schemaXml += propertyXml;
        } 

    for (int i = 0; i < stringPropCount; i++)
        {
        Utf8Char propertyName[32];
        BeStringUtilities::Snprintf (propertyName, "StringProperty%02d", i);
        Utf8CP propertyFormat = "        <ECProperty propertyName=\"%s\" typeName=\"string\" />";
        Utf8Char propertyXml[128];
        BeStringUtilities::Snprintf (propertyXml, propertyFormat, propertyName);
        schemaXml += propertyXml;
        }           

    schemaXml +="    </ECClass>"
                "</ECSchema>";

    return schemaXml;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                            Krischan.Eberle          09/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static 
Utf8String TestSchemaHelper::GetComplexTestSchemaXml
(
)
    {
    return Utf8String ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<ECSchema schemaName=\"ComplexTestSchema\" nameSpacePrefix=\"cp\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
"    <ECClass typeName=\"Struct1\" isStruct=\"True\" isDomainClass=\"True\">"
"        <ECProperty propertyName=\"Struct1BoolMember\" typeName=\"boolean\" />"
"        <ECProperty propertyName=\"Struct1IntMember\" typeName=\"int\" />"
"    </ECClass>"
"    <ECClass typeName=\"Struct2\" isStruct=\"True\" isDomainClass=\"True\">"
"        <ECProperty propertyName=\"Struct2StringMember\" typeName=\"string\" />"
"        <ECProperty propertyName=\"Struct2DoubleMember\" typeName=\"double\" />"
"        <ECArrayProperty propertyName=\"NestedArray\" typeName=\"Struct1\" minOccurs=\"0\" maxOccurs=\"unbounded\" isStruct=\"True\" />"
"    </ECClass>"
"    <ECClass typeName=\"Struct3\" isStruct=\"True\" isDomainClass=\"True\">"
"        <ECProperty propertyName=\"Struct3DoubleMember\" typeName=\"double\" />"
"        <ECProperty propertyName=\"Struct3IntMember\" typeName=\"int\" />"
"        <ECProperty propertyName=\"Struct3BoolMember\" typeName=\"boolean\" />"
"    </ECClass>"
"    <ECClass typeName=\"TAGGED_ITEM\" description=\"An ECClass representing Tagged Item\" isDomainClass=\"True\">"
"        <ECProperty propertyName=\"TAG\" typeName=\"string\" description=\"TAG\"/>"
"    </ECClass>"
"    <ECClass typeName=\"BaseClass\" isDomainClass=\"True\">"
"        <ECProperty propertyName=\"BaseClassMember\" typeName=\"int\" />"
"    </ECClass>"
"    <ECClass typeName=\"PIPE_Extra\" description=\"Pipe Class with extra properties\" displayLabel=\"Pipe extra\" isDomainClass=\"True\">"
"        <BaseClass>TAGGED_ITEM</BaseClass>"
"        <BaseClass>BaseClass</BaseClass>"
"        <ECProperty propertyName=\"Diameter\" typeName=\"string\" description=\"Diameter of PIPE\" />"
"        <ECProperty propertyName=\"GUID\" typeName=\"string\" />"
"        <ECArrayProperty propertyName=\"EC_BlobData\" typeName=\"string\" minOccurs=\"0\" maxOccurs=\"unbounded\" />"
"        <ECProperty propertyName=\"IntegerMember\" typeName=\"int\"/>"
"        <ECProperty propertyName=\"NegativeMember\" typeName=\"int\" />"
"        <ECProperty propertyName=\"CustomFormatInt\" typeName=\"int\"/>"
"        <ECProperty propertyName=\"LongMember\" typeName=\"long\"/>"
"        <ECProperty propertyName=\"BooleanMember\" typeName=\"boolean\" />"
"        <ECProperty propertyName=\"DoubleMember\" typeName=\"double\" readOnly=\"True\" />"
"        <ECProperty propertyName=\"DateTimeMember\" typeName=\"dateTime\" />"
"        <ECProperty propertyName=\"StringMember\" typeName=\"string\" description=\"This is the string property description\" displayLabel=\"StringDisplayLabel\"/>"
"        <ECProperty propertyName=\"StartPoint\" typeName=\"point3d\" description=\"This is the start point property description\" displayLabel=\"Start Point Label\" />"
"        <ECProperty propertyName=\"EndPoint\" typeName=\"point3d\" />"
"        <ECStructProperty propertyName=\"EmbeddedStruct\" typeName=\"Struct1\"/>"
"        <ECStructProperty propertyName=\"SecondEmbeddedStruct\" typeName=\"Struct1\" />"
"        <ECArrayProperty propertyName=\"IntArray\" typeName=\"int\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
"        <ECArrayProperty propertyName=\"SmallIntArray\" typeName=\"int\" minOccurs=\"0\" maxOccurs=\"unbounded\" />"
"        <ECArrayProperty propertyName=\"StringArray\" typeName=\"string\" minOccurs=\"0\" maxOccurs=\"unbounded\" />"
"        <ECArrayProperty propertyName=\"DateArray\" typeName=\"dateTime\" minOccurs=\"0\" maxOccurs=\"unbounded\" />"
"        <ECArrayProperty propertyName=\"StructArray\" typeName=\"Struct2\" minOccurs=\"0\" maxOccurs=\"unbounded\" isStruct=\"True\"/>"
"        <ECArrayProperty propertyName=\"EmptyIntArray\" typeName=\"int\" minOccurs=\"0\" maxOccurs=\"unbounded\" />"
"        <ECArrayProperty propertyName=\"OneMemberIntArray\" typeName=\"int\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
"        <ECStructProperty propertyName=\"FormattedStruct\" typeName=\"Struct3\"/>"
"        <ECArrayProperty propertyName=\"FormattedArray\" typeName=\"int\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
"        <ECArrayProperty propertyName=\"PointArray\" typeName=\"point3d\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
"    </ECClass>"
"</ECSchema>");
    }

END_DGNDB_UNIT_TESTS_NAMESPACE
