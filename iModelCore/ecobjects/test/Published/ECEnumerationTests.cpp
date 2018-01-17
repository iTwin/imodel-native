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

    // Create Enumeration
    EC_ASSERT_SUCCESS(schema->CreateEnumeration(enumeration, "TestEnumeration", PrimitiveType::PRIMITIVETYPE_Integer));
    ASSERT_NE(nullptr, enumeration);

    EXPECT_STREQ("TestEnumeration", enumeration->GetName().c_str());

    // Type
    ASSERT_EQ(PrimitiveType::PRIMITIVETYPE_Integer, enumeration->GetType());

    // Description
    enumeration->SetDescription("TestDescription");
    EXPECT_STREQ("TestDescription", enumeration->GetDescription().c_str());

    // IsStrict
    EXPECT_TRUE(enumeration->GetIsStrict());
    enumeration->SetIsStrict(false);
    EXPECT_FALSE(enumeration->GetIsStrict());

    // DisplayLabel
    ASSERT_FALSE(enumeration->GetIsDisplayLabelDefined());
    EXPECT_STREQ("TestEnumeration", enumeration->GetDisplayLabel().c_str());
    enumeration->SetDisplayLabel("Test Display Label");
    EXPECT_TRUE(enumeration->GetIsDisplayLabelDefined());
    EXPECT_STREQ("Test Display Label", enumeration->GetDisplayLabel().c_str());
    EXPECT_STREQ("Test Display Label", enumeration->GetInvariantDisplayLabel().c_str());

    // Enumerators
    ECEnumeratorP enumeratorA;
    EC_ASSERT_SUCCESS(enumeration->CreateEnumerator(enumeratorA, "TestEnumeratorA", 5));
    ASSERT_NE(nullptr, enumeratorA);
    EXPECT_STREQ("TestEnumeratorA", enumeratorA->GetInvariantDisplayLabel().c_str());
    enumeratorA->SetDisplayLabel("Display Label A");
    EXPECT_STREQ("TestEnumeratorA", enumeratorA->GetName().c_str());
    EXPECT_STREQ("Display Label A", enumeratorA->GetDisplayLabel().c_str());
    EXPECT_EQ(5, enumeratorA->GetInteger());
    EXPECT_STREQ("", enumeratorA->GetString().c_str());
    EXPECT_TRUE(enumeratorA->IsInteger());
    EXPECT_FALSE(enumeratorA->IsString());

    ECEnumeratorP enumeratorB;
    ASSERT_EQ(ECObjectsStatus::NamedItemAlreadyExists, enumeration->CreateEnumerator(enumeratorB, "TestEnumeratorA", 1));
    ASSERT_EQ(nullptr, enumeratorB);
    EC_ASSERT_SUCCESS(enumeration->CreateEnumerator(enumeratorB, "TestEnumeratorB", 1));
    ASSERT_NE(nullptr, enumeratorB);
    ASSERT_EQ(2, enumeration->GetEnumeratorCount());
    enumeratorB->SetDisplayLabel("Display Label B");
    EXPECT_STREQ("TestEnumeratorB", enumeratorB->GetName().c_str());
    EXPECT_STREQ("Display Label B", enumeratorB->GetDisplayLabel().c_str());
    EXPECT_EQ(1, enumeratorB->GetInteger());
    EXPECT_STREQ("", enumeratorB->GetString().c_str());
    EXPECT_TRUE(enumeratorB->IsInteger());
    EXPECT_FALSE(enumeratorB->IsString());

    // Convert the iterator to a vector, preserving order.
    bvector<ECEnumeratorCP> enumerators;
    for (auto const e : enumeration->GetEnumerators())
        enumerators.push_back(e);
    EXPECT_EQ(2, enumerators.size());
    EXPECT_EQ(enumeratorA, enumerators[0]);
    EXPECT_EQ(enumeratorB, enumerators[1]);

    EC_EXPECT_SUCCESS(enumeration->DeleteEnumerator(*enumeratorB));
    EXPECT_EQ(1, enumeration->GetEnumeratorCount());
    enumeration->Clear();
    EXPECT_EQ(0, enumeration->GetEnumeratorCount());
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
TEST_F(ECEnumerationTest, TestFindEnumeratoByNameRetrievesProperEnumerator)
    {
    Utf8CP schemaXML = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="FoodType" backingTypeName="int" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator name="spaghetti" value="13" displayLabel="Spaghetti"/>
                <ECEnumerator name="cannoli"   value="42" displayLabel="Cannoli"/>
            </ECEnumeration>
        </ECSchema>)xml";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);

    // Lookup using the exact name casing should succeed.
    {
    ECEnumerationCP ecEnum = schema->GetEnumerationCP("FoodType");
    ASSERT_NE(nullptr, ecEnum);

    auto enumerator = ecEnum->FindEnumeratorByName("spaghetti");
    ASSERT_NE(nullptr, enumerator);
    EXPECT_EQ(13, enumerator->GetInteger());
    enumerator = ecEnum->FindEnumeratorByName("cannoli");
    ASSERT_NE(nullptr, enumerator);
    EXPECT_EQ(42, enumerator->GetInteger());
    }
    // Lookup using case insensitive name should succeed.
    {
    ECEnumerationCP ecEnum = schema->GetEnumerationCP("FoodType");
    ASSERT_NE(nullptr, ecEnum);

    ASSERT_NE(nullptr, ecEnum->FindEnumeratorByName("sPaGhEtTi"));
    ASSERT_NE(nullptr, ecEnum->FindEnumeratorByName("cAnNoLi"));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          01/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumerationTest, TestFindEnumeratorRetrievesProperEnumerator)
    {
    // Integer backed
    {
    Utf8CP schemaXML = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEnumeration typeName="FoodType" backingTypeName="int" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator value="13" displayLabel="Spaghetti"/>
                <ECEnumerator value="42" displayLabel="Cannoli"/>
            </ECEnumeration>
        </ECSchema>)xml";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ECEnumerationCP ecEnum = schema->GetEnumerationCP("FoodType");
    ASSERT_NE(nullptr, ecEnum);

    auto enumerator = ecEnum->FindEnumerator(13);
    ASSERT_NE(nullptr, enumerator);
    EXPECT_EQ(13, enumerator->GetInteger());
    enumerator = ecEnum->FindEnumerator(42);
    ASSERT_NE(nullptr, enumerator);
    EXPECT_EQ(42, enumerator->GetInteger());
    }
    // String backed
    {
    Utf8CP schemaXML = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEnumeration typeName="FoodType" backingTypeName="string" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator value="spaghetti" displayLabel="Spaghetti"/>
                <ECEnumerator value="cannoli"   displayLabel="Cannoli"/>
            </ECEnumeration>
        </ECSchema>)xml";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ECEnumerationCP ecEnum = schema->GetEnumerationCP("FoodType");
    ASSERT_NE(nullptr, ecEnum);

    auto enumerator = ecEnum->FindEnumerator("spaghetti");
    ASSERT_NE(nullptr, enumerator);
    EXPECT_EQ("spaghetti", enumerator->GetString());
    enumerator = ecEnum->FindEnumerator("cannoli");
    ASSERT_NE(nullptr, enumerator);
    EXPECT_EQ("cannoli", enumerator->GetString());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          01/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumerationTest, TestFindEnumeratorIsCaseInsensitiveForStringTypes)
    {
    Utf8CP schemaXML = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEnumeration typeName="FoodType" backingTypeName="string" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator value="spaghetti" displayLabel="Spaghetti"/>
            </ECEnumeration>
        </ECSchema>)xml";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ECEnumerationCP ecEnum = schema->GetEnumerationCP("FoodType");
    ASSERT_NE(nullptr, ecEnum);

    EXPECT_NE(nullptr, ecEnum->FindEnumerator("spaghetti"));
    EXPECT_NE(nullptr, ecEnum->FindEnumerator("Spaghetti"));
    EXPECT_NE(nullptr, ecEnum->FindEnumerator("SPAGHETTI"));
    EXPECT_NE(nullptr, ecEnum->FindEnumerator("SpAgHeTtI"));
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
    EC_ASSERT_SUCCESS(schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_NE(nullptr, enumeration);
    enumeration->SetDescription("de");
    enumeration->SetDisplayLabel("dl");
    enumeration->SetIsStrict(false);
    EXPECT_STREQ("dl", enumeration->GetDisplayLabel().c_str());
    EXPECT_STREQ("de", enumeration->GetDescription().c_str());
    EXPECT_STREQ("string", enumeration->GetTypeName().c_str());

    ECEnumeratorP enumerator;
    EC_EXPECT_SUCCESS(enumeration->CreateEnumerator(enumerator, "FirstEnumerator", "First"));
    enumerator->SetDisplayLabel("First Value");
    EC_EXPECT_SUCCESS(enumeration->CreateEnumerator(enumerator, "SecondEnumerator", "Second"));
    enumerator->SetDisplayLabel("Second Value");
    EXPECT_EQ(2, enumeration->GetEnumeratorCount());

    ECEntityClassP entityClass;
    EC_ASSERT_SUCCESS(schema->CreateEntityClass(entityClass, "EntityClass"));
    PrimitiveECPropertyP property;
    EC_ASSERT_SUCCESS(entityClass->CreateEnumerationProperty(property, "EnumProperty", *enumeration));
    ASSERT_NE(nullptr, property);
    EXPECT_STREQ("Enumeration", property->GetTypeName().c_str());

    PrimitiveArrayECPropertyP arrProperty;
    EC_ASSERT_SUCCESS(entityClass->CreatePrimitiveArrayProperty(arrProperty, "EnumArrProperty"));
    ASSERT_NE(nullptr, arrProperty);
    EC_ASSERT_SUCCESS(arrProperty->SetType(*enumeration));
    EXPECT_STREQ("Enumeration", arrProperty->GetTypeName().c_str());

    Utf8String ecSchemaXmlString;
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(ecSchemaXmlString, ECVersion::Latest));

    ECSchemaPtr deserializedSchema;
    auto schemaContext = ECSchemaReadContext::CreateContext();
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(deserializedSchema, ecSchemaXmlString.c_str(), *schemaContext));

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
    ASSERT_NE(nullptr, deserializedPrimitive);

    ECPropertyP deserializedArrayProperty = deserializedClass->GetPropertyP("EnumArrProperty");
    EXPECT_STREQ("Enumeration", deserializedArrayProperty->GetTypeName().c_str());
    PrimitiveArrayECPropertyCP deserializedPrimitiveArray = deserializedArrayProperty->GetAsPrimitiveArrayProperty();
    ASSERT_NE(nullptr, deserializedPrimitiveArray);

    ECEnumerationCP propertyEnumeration = deserializedPrimitive->GetEnumeration();
    ASSERT_NE(nullptr, propertyEnumeration);
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
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refSchemaXML, *schemaContext));

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
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext));

    ECEnumerationP refEnumeration;
    refEnumeration = refSchema->GetEnumerationP("RevisionStatus");
    ASSERT_NE(nullptr, refEnumeration);
    EXPECT_STREQ("int", refEnumeration->GetTypeName().c_str());
    EXPECT_FALSE(refEnumeration->GetIsStrict());

    ECClassCP documentClass = schema->GetClassCP("Document");
    ECPropertyP deserializedProperty = documentClass->GetPropertyP("RevisionStatus");
    EXPECT_STREQ("ref:RevisionStatus", deserializedProperty->GetTypeName().c_str());
    PrimitiveECPropertyCP deserializedPrimitive = deserializedProperty->GetAsPrimitiveProperty();
    ASSERT_NE(nullptr, deserializedPrimitive);

    ECEnumerationCP deserializedPropertyEnumeration = deserializedPrimitive->GetEnumeration();
    ASSERT_NE(nullptr, deserializedPropertyEnumeration);
    EXPECT_STREQ("int", deserializedPropertyEnumeration->GetTypeName().c_str());
    ASSERT_EQ(refEnumeration, deserializedPropertyEnumeration);

    EXPECT_EQ(6, refEnumeration->GetEnumeratorCount());
    ECEnumeratorP ecEnumerator = refEnumeration->FindEnumerator(3);
    ASSERT_NE(nullptr, ecEnumerator);
    EXPECT_EQ(3, ecEnumerator->GetInteger());
    Utf8StringCR displayLabel = ecEnumerator->GetDisplayLabel();
    EXPECT_STREQ("Approved", displayLabel.c_str());
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
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"EnumInReferencedSchema.01.00.01.ecschema.xml").c_str(), *schemaContext));

    ASSERT_TRUE(schema.IsValid());

    ECClassP pClass = schema->GetClassP("Entity");
    ASSERT_NE(nullptr, pClass);

    ECPropertyP p = pClass->GetPropertyP("EnumeratedProperty");
    ASSERT_NE(nullptr, p);

    PrimitiveECPropertyCP prim = p->GetAsPrimitiveProperty();
    ASSERT_NE(nullptr, prim);

    ECEnumerationCP ecEnum = prim->GetEnumeration();
    ASSERT_NE(nullptr, ecEnum);
    EXPECT_STREQ("MyEnumeration", ecEnum->GetName().c_str());

    ECPropertyP p2 = pClass->GetPropertyP("EnumeratedArray");
    ASSERT_NE(nullptr, p2);

    PrimitiveArrayECPropertyCP primArr = p2->GetAsPrimitiveArrayProperty();
    ASSERT_NE(nullptr, primArr);

    ECEnumerationCP arrEnum = primArr->GetEnumeration();
    ASSERT_NE(nullptr, arrEnum);
    EXPECT_STREQ("MyEnumeration", arrEnum->GetName().c_str());

    ASSERT_EQ(12, ecEnum->GetSchema().GetVersionWrite());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumerationTest, SerializeStandaloneEnumeration)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    ECEnumerationP enumeration;
    EC_ASSERT_SUCCESS(schema->CreateEnumeration(enumeration, "ExampleEnumeration", PrimitiveType::PRIMITIVETYPE_Integer));
    enumeration->SetIsStrict(true);
    ECEnumeratorP enumeratorA;
    EC_ASSERT_SUCCESS(enumeration->CreateEnumerator(enumeratorA, "EnumeratorA", 1));
    enumeratorA->SetDisplayLabel("None");
    ECEnumeratorP enumeratorB;
    EC_ASSERT_SUCCESS(enumeration->CreateEnumerator(enumeratorB, "EnumeratorB", 2));
    enumeratorB->SetDisplayLabel("SomeVal");
    ECEnumeratorP enumeratorC;
    EC_ASSERT_SUCCESS(enumeration->CreateEnumerator(enumeratorC, "EnumeratorC", 3));
    enumeratorC->SetDisplayLabel("AnotherVal");
    Json::Value schemaJson;
    EXPECT_EQ(SchemaWriteStatus::Success, enumeration->WriteJson(schemaJson, true));

    Json::Value testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneECEnumeration.ecschema.json"));
    ASSERT_EQ(BentleyStatus::SUCCESS, ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile));

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
    EC_ASSERT_SUCCESS(schema->CreateEnumeration(strEnumeration, "StringTestEnumeration", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_NE(nullptr, strEnumeration);
    ECEnumeratorP strEnumeratorA;
    EC_ASSERT_SUCCESS(strEnumeration->CreateEnumerator(strEnumeratorA, "EnumeratorA", "foo"));
    ECEnumeratorP strEnumeratorB;
    EC_ASSERT_SUCCESS(strEnumeration->CreateEnumerator(strEnumeratorB, "EnumeratorB", "bar"));

    EC_EXPECT_SUCCESS(strEnumeratorB->SetString("baz")) << "Setting a type-matched enumerator to a unique value should succeed.";
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, strEnumeratorB->SetString(strEnumeratorA->GetString().c_str())) << "Setting a type-matched enumerator to a non-unique value should fail.";

    ECEnumerationP intEnumeration;
    EC_ASSERT_SUCCESS(schema->CreateEnumeration(intEnumeration, "IntegerTestEnumeration", PrimitiveType::PRIMITIVETYPE_Integer));
    ASSERT_NE(nullptr, intEnumeration);
    ECEnumeratorP intInumerator;
    EC_ASSERT_SUCCESS(intEnumeration->CreateEnumerator(intInumerator, "IntegerEnumerator", 3));

    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, intInumerator->SetString("qux")) << "Setting a type-mismatched enumerator should fail.";
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
    EC_ASSERT_SUCCESS(schema->CreateEnumeration(intEnumeration, "IntegerTestEnumeration", PrimitiveType::PRIMITIVETYPE_Integer));
    ASSERT_NE(nullptr, intEnumeration);
    ECEnumeratorP intEnumeratorA;
    EC_ASSERT_SUCCESS(intEnumeration->CreateEnumerator(intEnumeratorA, "EnumeratorA", 3));
    ECEnumeratorP intEnumeratorB;
    EC_ASSERT_SUCCESS(intEnumeration->CreateEnumerator(intEnumeratorB, "EnumeratorB", 4));

    EC_EXPECT_SUCCESS(intEnumeratorB->SetInteger(5)) << "Setting a type-matched enumerator to a unique value should succeed.";
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, intEnumeratorB->SetInteger(intEnumeratorA->GetInteger())) << "Setting a type-matched enumerator to a non-unique value should fail.";

    ECEnumerationP strEnumeration;
    EC_ASSERT_SUCCESS(schema->CreateEnumeration(strEnumeration, "StringTestEnumeration", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_NE(nullptr, strEnumeration);
    ECEnumeratorP strEnumerator;
    EC_ASSERT_SUCCESS(strEnumeration->CreateEnumerator(strEnumerator, "StringEnumerator", "foo"));

    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, strEnumerator->SetInteger(9)) << "Setting a type-mismatched enumerator should fail.";
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
