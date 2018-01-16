/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECEnumerationTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ECEnumerationTest : ECTestFixture {};
struct ECEnumeratorTest : ECTestFixture {};

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

    ECEnumeratorP enumerator1;
    status = enumeration->CreateEnumerator(enumerator1, "Enumerator1", 5);
    EXPECT_TRUE(status == ECObjectsStatus::Success);
    EXPECT_TRUE(enumerator1 != nullptr);
    EXPECT_STREQ(enumerator1->GetInvariantDisplayLabel().c_str(), "Enumerator1");
    enumerator1->SetDisplayLabel("DLBL");

    EXPECT_STREQ(enumerator1->GetName().c_str(), "Enumerator1");
    EXPECT_STREQ(enumerator1->GetDisplayLabel().c_str(), "DLBL");

    EXPECT_TRUE(enumerator1->GetInteger() == 5);
    EXPECT_STREQ(enumerator1->GetString().c_str(), "");
    EXPECT_FALSE(enumerator1->IsString());
    EXPECT_TRUE(enumerator1->IsInteger());

    ECEnumeratorP enumerator2;
    status = enumeration->CreateEnumerator(enumerator2, "Enumerator2", 5);
    EXPECT_TRUE(status == ECObjectsStatus::NamedItemAlreadyExists);
    EXPECT_TRUE(enumerator2 == nullptr);

    status = enumeration->CreateEnumerator(enumerator2, "Enumerator2", 1);
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
            EXPECT_TRUE(p == enumerator1);
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

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          01/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumerationTest, TestFindEnumeratorIsCaseInsensitive)
    {
    Utf8CP schemaXML = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEnumeration typeName="FoodType" backingTypeName="int" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator value="0" displayLabel="spaghetti"/>
            </ECEnumeration>
        </ECSchema>)xml";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ECEnumerationCP ecEnum = schema->GetEnumerationCP("FoodType");
    ASSERT_NE(nullptr, ecEnum);

    EXPECT_NE(nullptr, schema->GetEnumerationCP("FoodType"));
    EXPECT_NE(nullptr, schema->GetEnumerationCP("FOODTYPE"));
    EXPECT_NE(nullptr, schema->GetEnumerationCP("foodtype"));
    EXPECT_NE(nullptr, schema->GetEnumerationCP("FoOdTyPE"));
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
    EXPECT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumerator, "FirstEnumerator", "First"));
    enumerator->SetDisplayLabel("First Value");
    EXPECT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumerator, "SecondEnumerator", "Second"));
    enumerator->SetDisplayLabel("Second Value");
    EXPECT_EQ(2, enumeration->GetEnumeratorCount());

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
    SchemaWriteStatus status2 = schema->WriteToXmlString(ecSchemaXmlString, ECVersion::Latest);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    ECSchemaPtr deserializedSchema;
    auto schemaContext = ECSchemaReadContext::CreateContext();
    auto status3 = ECSchema::ReadFromXmlString(deserializedSchema, ecSchemaXmlString.c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status3);

    ECEnumerationP deserializedEnumeration;
    deserializedEnumeration = deserializedSchema->GetEnumerationP("Enumeration");
    ASSERT_NE(nullptr, deserializedEnumeration);
    EXPECT_STREQ("dl", deserializedEnumeration->GetDisplayLabel().c_str());
    EXPECT_STREQ("de", deserializedEnumeration->GetDescription().c_str());
    EXPECT_STREQ("string", deserializedEnumeration->GetTypeName().c_str());

    EXPECT_EQ(2, deserializedEnumeration->GetEnumeratorCount());
    enumerator = deserializedEnumeration->FindEnumerator("Second");
    ASSERT_NE(nullptr, enumerator);
    EXPECT_STREQ("SecondEnumerator", enumerator->GetName().c_str());
    EXPECT_STREQ("Second Value", enumerator->GetInvariantDisplayLabel().c_str());
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
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP refSchemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
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
    ECSchemaPtr refSchema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(refSchema, refSchemaXML, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);

    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
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
    ASSERT_EQ(SchemaReadStatus::Success, status);

    ECEnumerationP refEnumeration;
    refEnumeration = refSchema->GetEnumerationP("RevisionStatus");
    ASSERT_TRUE(nullptr != refEnumeration);
    EXPECT_STREQ("int", refEnumeration->GetTypeName().c_str());
    EXPECT_FALSE(refEnumeration->GetIsStrict());

    ECClassCP documentClass = schema->GetClassCP("Document");
    ECPropertyP deserializedProperty = documentClass->GetPropertyP("RevisionStatus");
    EXPECT_STREQ("ref:RevisionStatus", deserializedProperty->GetTypeName().c_str());
    PrimitiveECPropertyCP deserializedPrimitive = deserializedProperty->GetAsPrimitiveProperty();
    ASSERT_TRUE(nullptr != deserializedPrimitive);

    ECEnumerationCP deserializedPropertyEnumeration = deserializedPrimitive->GetEnumeration();
    ASSERT_TRUE(nullptr != deserializedPropertyEnumeration);
    EXPECT_STREQ("int", deserializedPropertyEnumeration->GetTypeName().c_str());
    ASSERT_TRUE(refEnumeration == deserializedPropertyEnumeration);

    EXPECT_EQ(6, refEnumeration->GetEnumeratorCount());
    ECEnumeratorP ecEnumerator = refEnumeration->FindEnumerator(3);
    ASSERT_TRUE(nullptr != ecEnumerator);
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

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumerationTest, SerializeStandaloneEnumeration)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    ECEnumerationP enumeration;
    schema->CreateEnumeration(enumeration, "ExampleEnumeration", PrimitiveType::PRIMITIVETYPE_Integer);
    enumeration->SetIsStrict(true);
    ECEnumeratorP enumeratorA;
    enumeration->CreateEnumerator(enumeratorA, "EnumeratorA", 1);
    enumeratorA->SetDisplayLabel("None");
    ECEnumeratorP enumeratorB;
    enumeration->CreateEnumerator(enumeratorB, "EnumeratorB", 2);
    enumeratorB->SetDisplayLabel("SomeVal");
    ECEnumeratorP enumeratorC;
    enumeration->CreateEnumerator(enumeratorC, "EnumeratorC", 3);
    enumeratorC->SetDisplayLabel("AnotherVal");
    Json::Value schemaJson;
    EXPECT_EQ(SchemaWriteStatus::Success, enumeration->WriteJson(schemaJson, true));

    Json::Value testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneECEnumeration.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          12/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumeratorTest, TestEnumeratorSetString)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ASSERT_TRUE(schema.IsValid());

    ECEnumerationP strEnumeration;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEnumeration(strEnumeration, "StringTestEnumeration", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_TRUE(strEnumeration != nullptr);
    ECEnumeratorP strEnumerator0;
    ASSERT_EQ(ECObjectsStatus::Success, strEnumeration->CreateEnumerator(strEnumerator0, "Enumerator0", "foo"));
    ECEnumeratorP strEnumerator1;
    ASSERT_EQ(ECObjectsStatus::Success, strEnumeration->CreateEnumerator(strEnumerator1, "Enumerator1", "bar"));

    EXPECT_EQ(ECObjectsStatus::Success, strEnumerator1->SetString("baz")) << "Setting a type-matched enumerator to a unique value should succeed.";
    EXPECT_NE(ECObjectsStatus::Success, strEnumerator1->SetString(strEnumerator0->GetString().c_str())) << "Setting a type-matched enumerator to a non-unique value should fail.";

    ECEnumerationP intEnumeration;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEnumeration(intEnumeration, "IntegerTestEnumeration", PrimitiveType::PRIMITIVETYPE_Integer));
    ASSERT_TRUE(intEnumeration != nullptr);
    ECEnumeratorP intInumerator;
    ASSERT_EQ(ECObjectsStatus::Success, intEnumeration->CreateEnumerator(intInumerator, "IntegerEnumerator", 3));

    EXPECT_NE(ECObjectsStatus::Success, intInumerator->SetString("qux")) << "Setting a type-mismatched enumerator should fail.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          12/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumeratorTest, TestEnumeratorSetInteger)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ASSERT_TRUE(schema.IsValid());

    ECEnumerationP intEnumeration;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEnumeration(intEnumeration, "IntegerTestEnumeration", PrimitiveType::PRIMITIVETYPE_Integer));
    ASSERT_TRUE(intEnumeration != nullptr);
    ECEnumeratorP intEnumerator0;
    ASSERT_EQ(ECObjectsStatus::Success, intEnumeration->CreateEnumerator(intEnumerator0, "Enumerator0", 3));
    ECEnumeratorP intEnumerator1;
    ASSERT_EQ(ECObjectsStatus::Success, intEnumeration->CreateEnumerator(intEnumerator1, "Enumerator1", 4));

    EXPECT_EQ(ECObjectsStatus::Success, intEnumerator1->SetInteger(5)) << "Setting a type-matched enumerator to a unique value should succeed.";
    EXPECT_NE(ECObjectsStatus::Success, intEnumerator1->SetInteger(intEnumerator0->GetInteger())) << "Setting a type-matched enumerator to a non-unique value should fail.";

    ECEnumerationP strEnumeration;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEnumeration(strEnumeration, "StringTestEnumeration", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_TRUE(strEnumeration != nullptr);
    ECEnumeratorP strEnumerator;
    ASSERT_EQ(ECObjectsStatus::Success, strEnumeration->CreateEnumerator(strEnumerator, "StringEnumerator", "foo"));

    EXPECT_NE(ECObjectsStatus::Success, strEnumerator->SetInteger(9)) << "Setting a type-mismatched enumerator should fail.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          01/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumeratorTest, DetermineEnumeratorName)
    {
    Utf8String enumerationName("TestEnumeration");
    // String enumerator
    {
    Utf8String enumeratorValueA("TestEnumeratorValueA");
    Utf8CP enumeratorValueB = "TestEnumeratorValueB";
    EXPECT_EQ("TestEnumeratorValueA", ECEnumerator::DetermineName(enumerationName, enumeratorValueA.c_str(), nullptr));
    EXPECT_EQ("TestEnumeratorValueB", ECEnumerator::DetermineName(enumerationName, enumeratorValueB, nullptr));
    }
    // Integer enumerator
    {
    int32_t enumeratorValue = 42;
    EXPECT_EQ("TestEnumeration42", ECEnumerator::DetermineName(enumerationName, nullptr, &enumeratorValue));
    }
    }

END_BENTLEY_ECN_TEST_NAMESPACE
