/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECEnumerationTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ECEnumerationTest : ECTestFixture {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Caleb.Shafer                          06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECEnumerationTest, CheckEnumerationBasicProperties)
    {
    ECSchemaPtr schema;
    ECEnumerationP enumeration;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ASSERT_TRUE(schema.IsValid());

    //Create Enumeration
    auto status = schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_Integer);
    ASSERT_TRUE(enumeration != nullptr);
    ASSERT_TRUE(status == ECObjectsStatus::Success);

    EXPECT_STREQ(enumeration->GetName().c_str(), "Enumeration");

    //Type
    ASSERT_TRUE(enumeration->GetType() == PrimitiveType::PRIMITIVETYPE_Integer);

    //Description
    enumeration->SetDescription("MyDescription");
    EXPECT_STREQ(enumeration->GetDescription().c_str(), "MyDescription");

    //IsStrict
    EXPECT_TRUE(enumeration->GetIsStrict());
    enumeration->SetIsStrict(false);
    EXPECT_FALSE(enumeration->GetIsStrict());

    //DisplayLabel
    ASSERT_TRUE(enumeration->GetIsDisplayLabelDefined() == false);
    EXPECT_STREQ(enumeration->GetDisplayLabel().c_str(), "Enumeration");
    enumeration->SetDisplayLabel("Display Label");
    EXPECT_STREQ(enumeration->GetDisplayLabel().c_str(), "Display Label");
    EXPECT_STREQ(enumeration->GetInvariantDisplayLabel().c_str(), "Display Label");

    ECEnumeratorP enumerator;
    status = enumeration->CreateEnumerator(enumerator, 5);
    EXPECT_TRUE(status == ECObjectsStatus::Success);
    EXPECT_TRUE(enumerator != nullptr);
    EXPECT_STREQ(enumerator->GetInvariantDisplayLabel().c_str(), "5");
    enumerator->SetDisplayLabel("DLBL");

    EXPECT_STREQ(enumerator->GetDisplayLabel().c_str(), "DLBL");

    EXPECT_TRUE(enumerator->GetInteger() == 5);
    EXPECT_STREQ(enumerator->GetString().c_str(), "");
    EXPECT_FALSE(enumerator->IsString());
    EXPECT_TRUE(enumerator->IsInteger());

    ECEnumeratorP enumerator2;
    status = enumeration->CreateEnumerator(enumerator2, 5);
    EXPECT_TRUE(status == ECObjectsStatus::NamedItemAlreadyExists);
    EXPECT_TRUE(enumerator2 == nullptr);

    status = enumeration->CreateEnumerator(enumerator2, 1);
    EXPECT_TRUE(status == ECObjectsStatus::Success);
    EXPECT_TRUE(enumerator2 != nullptr);
    enumerator2->SetDisplayLabel("DLBL2");

    EXPECT_TRUE(enumeration->GetEnumeratorCount() == 2);

    int i = 0;
    for (auto p : enumeration->GetEnumerators())
        {
        EXPECT_TRUE(p != nullptr);
        if (i == 0)
            {
            EXPECT_TRUE(p == enumerator);
            }
        else if (i == 1)
            {
            EXPECT_TRUE(p == enumerator2);
            }

        i++;
        }

    ASSERT_TRUE(i == 2);

    EXPECT_TRUE(enumeration->DeleteEnumerator(*enumerator2) == ECObjectsStatus::Success);
    EXPECT_TRUE(enumeration->GetEnumeratorCount() == 1);
    enumeration->Clear();
    EXPECT_TRUE(enumeration->GetEnumeratorCount() == 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECEnumerationTest, TestEmptyOrMissingName)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEnumeration backingTypeName='int' description='...' displayLabel='Revision Status' isStrict='False'>
            </ECEnumeration>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEnumeration typeName='' backingTypeName='int' description='...' displayLabel='Revision Status' isStrict='False'>
            </ECEnumeration>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEnumeration typeName='1BadName' backingTypeName='int' description='...' displayLabel='Revision Status' isStrict='False'>
            </ECEnumeration>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context)) << 
        "Schema with ECEnumeration name with leading number should have failed to deserialize.";
    }
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEnumeration typeName='Testing' backingTypeName='int' description='...' displayLabel='Revision Status' isStrict='False'>
            </ECEnumeration>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECEnumerationCP ecEnum = schema->GetEnumerationCP("Testing");
    EXPECT_STREQ("Testing", ecEnum->GetName().c_str());
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Caleb.Shafer                          06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECEnumerationTest, ExpectSuccessWhenRoundtripEnumerationUsingString)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ASSERT_TRUE(schema.IsValid());

    //Create Enumeration
    ECEnumerationP enumeration;
    auto status = schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_String);
    enumeration->SetDescription("de");
    enumeration->SetDisplayLabel("dl");
    enumeration->SetIsStrict(false);
    ASSERT_TRUE(enumeration != nullptr);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    EXPECT_STREQ("dl", enumeration->GetDisplayLabel().c_str());
    EXPECT_STREQ("de", enumeration->GetDescription().c_str());
    EXPECT_STREQ("string", enumeration->GetTypeName().c_str());

    ECEnumeratorP enumerator;
    EXPECT_TRUE(enumeration->CreateEnumerator(enumerator, "First") == ECObjectsStatus::Success);
    enumerator->SetDisplayLabel("First Value");
    EXPECT_TRUE(enumeration->CreateEnumerator(enumerator, "Second") == ECObjectsStatus::Success);
    enumerator->SetDisplayLabel("Second Value");
    EXPECT_TRUE(enumeration->GetEnumeratorCount() == 2);

    ECEntityClassP entityClass;
    status = schema->CreateEntityClass(entityClass, "EntityClass");
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    PrimitiveECPropertyP property;
    status = entityClass->CreateEnumerationProperty(property, "EnumProperty", *enumeration);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    ASSERT_TRUE(property != nullptr);
    EXPECT_STREQ("Enumeration", property->GetTypeName().c_str());

    PrimitiveArrayECPropertyP arrProperty;
    status = entityClass->CreatePrimitiveArrayProperty(arrProperty, "EnumArrProperty");
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    ASSERT_TRUE(nullptr != arrProperty);
    status = arrProperty->SetType(*enumeration);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    EXPECT_STREQ("Enumeration", arrProperty->GetTypeName().c_str());

    Utf8String ecSchemaXmlString;
    SchemaWriteStatus status2 = schema->WriteToXmlString(ecSchemaXmlString, ECVersion::V3_1);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    ECSchemaPtr deserializedSchema;
    auto schemaContext = ECSchemaReadContext::CreateContext();
    auto status3 = ECSchema::ReadFromXmlString(deserializedSchema, ecSchemaXmlString.c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status3);

    ECEnumerationP deserializedEnumeration;
    deserializedEnumeration = deserializedSchema->GetEnumerationP("Enumeration");
    ASSERT_TRUE(deserializedEnumeration != nullptr);
    EXPECT_STREQ("dl", deserializedEnumeration->GetDisplayLabel().c_str());
    EXPECT_STREQ("de", deserializedEnumeration->GetDescription().c_str());
    EXPECT_STREQ("string", deserializedEnumeration->GetTypeName().c_str());

    EXPECT_TRUE(deserializedEnumeration->GetEnumeratorCount() == 2);
    enumerator = deserializedEnumeration->FindEnumerator("First");
    EXPECT_TRUE(enumerator != nullptr);
    EXPECT_STREQ("First Value", enumerator->GetInvariantDisplayLabel().c_str());
    EXPECT_FALSE(deserializedEnumeration->GetIsStrict());

    ECClassCP deserializedClass = deserializedSchema->GetClassCP("EntityClass");
    ECPropertyP deserializedProperty = deserializedClass->GetPropertyP("EnumProperty");
    EXPECT_STREQ("Enumeration", deserializedProperty->GetTypeName().c_str());
    PrimitiveECPropertyCP deserializedPrimitive = deserializedProperty->GetAsPrimitiveProperty();
    ASSERT_TRUE(nullptr != deserializedPrimitive);

    ECPropertyP deserializedArrayProperty = deserializedClass->GetPropertyP("EnumArrProperty");
    EXPECT_STREQ("Enumeration", deserializedArrayProperty->GetTypeName().c_str());
    PrimitiveArrayECPropertyCP deserializedPrimitiveArray = deserializedArrayProperty->GetAsPrimitiveArrayProperty();
    ASSERT_TRUE(nullptr != deserializedPrimitiveArray);

    ECEnumerationCP propertyEnumeration = deserializedPrimitive->GetEnumeration();
    ASSERT_TRUE(nullptr != propertyEnumeration);
    EXPECT_STREQ("string", propertyEnumeration->GetTypeName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECEnumerationTest, ExpectSuccessWithEnumerationInReferencedSchema)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='ReferencedSchema' version='01.00' displayLabel='Display Label' description='Description' nameSpacePrefix='ref' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECEnumeration typeName='RevisionStatus' backingTypeName='int' description='...' displayLabel='Revision Status' isStrict='False'>"
        "       <ECEnumerator value='0' displayLabel='Undefined' />"
        "       <ECEnumerator value='1' displayLabel='Planned' />"
        "       <ECEnumerator value='2' displayLabel='Not Approved' />"
        "       <ECEnumerator value='3' displayLabel='Approved' />"
        "       <ECEnumerator value='4' displayLabel='Previous Revision' />"
        "       <ECEnumerator value='5' displayLabel='Obsolete' />"
        "   </ECEnumeration>"
        "</ECSchema>";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr refSchema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(refSchema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Stuff' version='09.06' displayLabel='Display Label' description='Description' nameSpacePrefix='stuff' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECSchemaReference name='ReferencedSchema' version='01.00' prefix='ref' />"
        "   <ECClass typeName='Document' isStruct='false' isCustomAttributeClass='false' isDomainClass='true'>"
        "       <ECProperty propertyName='Name' typeName='string' displayLabel='Title' />"
        "       <ECProperty propertyName='DateEffective' typeName='dateTime' displayLabel='Date Effective' />"
        "       <ECProperty propertyName='DateObsolete' typeName='dateTime' displayLabel='Date Obsolete' />"
        "       <ECProperty propertyName='IsTemplate' typeName='boolean' />"
        "       <ECProperty propertyName='RevisionStatus' typeName='ref:RevisionStatus' displayLabel='Revision Status' />"
        "   </ECClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECEnumerationP enumeration;
    enumeration = refSchema->GetEnumerationP("RevisionStatus");
    ASSERT_TRUE(enumeration != nullptr);
    EXPECT_STREQ("int", enumeration->GetTypeName().c_str());
    EXPECT_FALSE(enumeration->GetIsStrict());

    ECClassCP documentClass = schema->GetClassCP("Document");
    ECPropertyP deserializedProperty = documentClass->GetPropertyP("RevisionStatus");
    EXPECT_STREQ("ref:RevisionStatus", deserializedProperty->GetTypeName().c_str());
    PrimitiveECPropertyCP deserializedPrimitive = deserializedProperty->GetAsPrimitiveProperty();
    ASSERT_TRUE(nullptr != deserializedPrimitive);

    ECEnumerationCP propertyEnumeration = deserializedPrimitive->GetEnumeration();
    ASSERT_TRUE(nullptr != propertyEnumeration);
    EXPECT_STREQ("int", propertyEnumeration->GetTypeName().c_str());
    ASSERT_TRUE(enumeration == propertyEnumeration);

    EXPECT_TRUE(enumeration->GetEnumeratorCount() == 6);
    ECEnumeratorP ecEnumerator = enumeration->FindEnumerator(3);
    EXPECT_TRUE(ecEnumerator != nullptr);
    EXPECT_TRUE(ecEnumerator->GetInteger() == 3);
    Utf8StringCR displayLabel = ecEnumerator->GetDisplayLabel();
    EXPECT_STREQ(displayLabel.c_str(), "Approved");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Caleb.Shafer                          06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECEnumerationTest, ExpectSuccessWhenDeserializingSchemaWithEnumerationInReferencedFile)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    WString seedPath(ECTestFixture::GetTestDataPath(L"").c_str());
    schemaContext->AddSchemaPath(seedPath.c_str());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"EnumInReferencedSchema.01.00.01.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ASSERT_TRUE(schema.IsValid());

    ECClassP pClass = schema->GetClassP("Entity");
    ASSERT_TRUE(nullptr != pClass);

    ECPropertyP p = pClass->GetPropertyP("EnumeratedProperty");
    ASSERT_TRUE(p != nullptr);

    PrimitiveECPropertyCP prim = p->GetAsPrimitiveProperty();
    ASSERT_TRUE(prim != nullptr);

    ECEnumerationCP ecEnum = prim->GetEnumeration();
    ASSERT_TRUE(ecEnum != nullptr);
    EXPECT_STREQ("MyEnumeration", ecEnum->GetName().c_str());

    ECPropertyP p2 = pClass->GetPropertyP("EnumeratedArray");
    ASSERT_TRUE(p2 != nullptr);

    PrimitiveArrayECPropertyCP primArr = p2->GetAsPrimitiveArrayProperty();
    ASSERT_TRUE(primArr != nullptr);

    ECEnumerationCP arrEnum = primArr->GetEnumeration();
    ASSERT_TRUE(arrEnum != nullptr);
    EXPECT_STREQ("MyEnumeration", arrEnum->GetName().c_str());

    ASSERT_TRUE(ecEnum->GetSchema().GetVersionWrite() == 12);
    }

END_BENTLEY_ECN_TEST_NAMESPACE