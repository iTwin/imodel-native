/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ECEnumerationTest : ECTestFixture {};
struct ECEnumerationCompatibilityTests : ECTestFixture {};
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
// @bsimethod                           Victor.Cushman                          02/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumerationTest, TestFindEnumeratoByNameRetrievesProperEnumerator)
    {
    Utf8CP schemaXML = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
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
TEST_F(ECEnumerationTest, TestGetEnumerationIsCaseInsensitive)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEnumeration typeName="FoodType" backingTypeName="string" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator value="Spaghetti" displayLabel="spaghetti"/>
            </ECEnumeration>
        </ECSchema>)xml";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
    ASSERT_TRUE(schema.IsValid());
    ECEnumerationCP ecEnum = schema->GetEnumerationCP("FoodType");
    ASSERT_NE(nullptr, ecEnum);

    EXPECT_EQ(ecEnum, schema->GetEnumerationCP("FOODTYPE"));
    EXPECT_EQ(ecEnum, schema->GetEnumerationCP("foodtype"));
    EXPECT_EQ(ecEnum, schema->GetEnumerationCP("FoOdTyPE"));
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
    enumeratorA->SetDescription("AwesomeDescription");
    ECEnumeratorP enumeratorB;
    EC_ASSERT_SUCCESS(enumeration->CreateEnumerator(enumeratorB, "EnumeratorB", 2));
    enumeratorB->SetDisplayLabel("SomeVal");
    ECEnumeratorP enumeratorC;
    EC_ASSERT_SUCCESS(enumeration->CreateEnumerator(enumeratorC, "EnumeratorC", 3));
    enumeratorC->SetDisplayLabel("AnotherVal");
    Json::Value schemaJson;
    EXPECT_TRUE(enumeration->ToJson(schemaJson, true));

    Json::Value testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneECEnumeration.ecschema.json"));
    ASSERT_EQ(BentleyStatus::SUCCESS, ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile));

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                         Kyle.Abramowitz                           03/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumerationTest, ShouldFailWithBadBackingType)
    {
    ExpectSchemaDeserializationFailure( R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="FoodType" backingTypeName="foo" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator name="foo" value="Spaghetti" displayLabel="spaghetti"/>
            </ECEnumeration>
        </ECSchema>)xml", SchemaReadStatus::InvalidPrimitiveType, "Should fail to deserialize enumeration with nonexistant backing type");

    ExpectSchemaDeserializationFailure( R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="FoodType" backingTypeName="boolean" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator name="foo" value="Spaghetti" displayLabel="spaghetti"/>
            </ECEnumeration>
        </ECSchema>)xml", SchemaReadStatus::InvalidPrimitiveType, "Should fail to deserialize enumeration with invalid backingType for and Enumerator");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                         Kyle.Abramowitz                           03/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumerationTest, ShouldFailWithDuplicateName)
    {
    ExpectSchemaDeserializationFailure( R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="FoodType" backingTypeName="string" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator name="Bananas" value="Spaghetti" displayLabel="spaghetti"/>
                <ECEnumerator name="Bananas" value="Stuff" displayLabel="stuff"/>
            </ECEnumeration>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize enumeration with duplicate enumerator names");
    }


//---------------------------------------------------------------------------------------
// @bsimethod                         Kyle.Abramowitz                           03/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumerationTest, ShouldFailWithDuplicateValues)
    {
    ExpectSchemaDeserializationFailure( R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="FoodType" backingTypeName="string" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator name="Bananas" value="Spaghetti" displayLabel="spaghetti"/>
                <ECEnumerator name="Spaghetti" value="Spaghetti" displayLabel="stuff"/>
            </ECEnumeration>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize enumeration with duplicate enumerator values");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                         Kyle.Abramowitz                           03/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumerationTest, InvalidOrMissingName)
    {
    ExpectSchemaDeserializationFailure( R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="FoodType" backingTypeName="string" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator value="Spaghetti" displayLabel="spaghetti"/>
            </ECEnumeration>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize enumerator with missing name");

    ExpectSchemaDeserializationFailure( R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="FoodType" backingTypeName="string" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator name="" value="Spaghetti" displayLabel="spaghetti"/>
            </ECEnumeration>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize enumerator with empty name");

    ExpectSchemaDeserializationFailure( R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="FoodType" backingTypeName="string" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator name="@#$%" value="Spaghetti" displayLabel="spaghetti"/>
            </ECEnumeration>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize enumerator with invalid EC name");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                         Kyle.Abramowitz                           03/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumerationTest, MissingOrEmptyDisplayLabel)
    {
    {
    SchemaItem missing(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="FoodType" backingTypeName="string" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator name="Foo1" value="Spaghetti" />
            </ECEnumeration>
        </ECSchema>)xml");
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, missing.GetXmlString().c_str(), *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->Validate());
    ECEnumerationCP enumeration = schema->GetEnumerationCP("FoodType");
    ASSERT_NE(nullptr, enumeration);
    ECEnumeratorCP enumerator = enumeration->FindEnumeratorByName("Foo1");
    ASSERT_NE(nullptr, enumerator);
    ASSERT_STRCASEEQ("Foo1", enumerator->GetInvariantDisplayLabel().c_str());
    }
    {
    SchemaItem empty( R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="FoodType" backingTypeName="string" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator name="Foo2" value="Spaghetti" displayLabel=""/>
            </ECEnumeration>
        </ECSchema>)xml");
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, empty.GetXmlString().c_str(), *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->Validate());
    ECEnumerationCP enumeration = schema->GetEnumerationCP("FoodType");
    ASSERT_NE(nullptr, enumeration);
    ECEnumeratorCP enumerator = enumeration->FindEnumeratorByName("Foo2");
    ASSERT_NE(nullptr, enumerator);
    ASSERT_STRCASEEQ("Foo2", enumerator->GetInvariantDisplayLabel().c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  05/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECEnumerationTest, LookupEnumerationTest)
    {
    Utf8CP refSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="ref" version="01.00.00" alias="r" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
           <ECEnumeration typeName="FoodType" backingTypeName="string" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator name="Foo2" value="Spaghetti" displayLabel=""/>
            </ECEnumeration>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
           <ECSchemaReference name="ref" version="1.0.0" alias="r"/>
           <ECEnumeration typeName="FoodType" backingTypeName="string" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator name="Foo2" value="Spaghetti" displayLabel=""/>
            </ECEnumeration>
        </ECSchema>)xml";

    ECSchemaPtr refSchema;
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refSchemaXml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    auto shouldBeNull = schema->LookupEnumeration("");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupEnumeration("banana");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupEnumeration("banana:FoodType");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupEnumeration("ref:FoodType");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupEnumeration("testSchema:FoodType");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupEnumeration("ts:FoodType", true);
    EXPECT_EQ(nullptr, shouldBeNull);
    auto shouldNotBeNull = schema->LookupEnumeration("FoodType");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("FoodType", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupEnumeration("r:FoodType");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("FoodType", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupEnumeration("ref:FoodType", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("FoodType", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupEnumeration("testSchema:FoodType", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("FoodType", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupEnumeration("R:FoodType");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("FoodType", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupEnumeration("REF:FoodType", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("FoodType", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupEnumeration("TESTSCHEMA:FoodType", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("FoodType", shouldNotBeNull->GetName().c_str());
    ASSERT_EQ(1, schema->GetEnumerationCount());
    }

//=======================================================================================
//! ECEnumerationCompatibilityTests
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                         Kyle.Abramowitz                           03/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumerationCompatibilityTests, ShouldNotCrashWithUnsupportedBackingTypesForFutureVersions)
    {
    SchemaItem schemaXml(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECEnumeration typeName="FoodType" backingTypeName="boolean" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator name="Foo1" value="False" />
            </ECEnumeration>
        </ECSchema>)xml");
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, schemaXml.GetXmlString().c_str(), *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->Validate());
    ECEnumerationCP enumeration = schema->GetEnumerationCP("FoodType");
    ASSERT_NE(nullptr, enumeration);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                         Kyle.Abramowitz                           03/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumerationCompatibilityTests, ShouldNotCrashWithOtherNodeNameInFutureVersion)
    {
    SchemaItem schemaXml(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECEnumeration typeName="FoodType" backingTypeName="boolean" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator name="Foo1" value="False" />
                <Blah name="banana"/>
            </ECEnumeration>
        </ECSchema>)xml");
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, schemaXml.GetXmlString().c_str(), *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->Validate());
    ECEnumerationCP enumeration = schema->GetEnumerationCP("FoodType");
    ASSERT_NE(nullptr, enumeration);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                         Kyle.Abramowitz                           03/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumerationCompatibilityTests, ShouldReadEnumerationFromNewerSchemaSuccessfully)
    {
    SchemaItem missing(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FoodSchema" alias="food" version="01.00.00" displayLabel="Food Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECEnumeration typeName="FoodType" backingTypeName="string" description="Yummy yummy in my tummy" displayLabel="Food Type" isStrict="False">
                <ECEnumerator name="Foo1" value="Spaghetti" />
            </ECEnumeration>
        </ECSchema>)xml");
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, missing.GetXmlString().c_str(), *context);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->Validate());
    ECEnumerationCP enumeration = schema->GetEnumerationCP("FoodType");
    ASSERT_NE(nullptr, enumeration);
    ECEnumeratorCP enumerator = enumeration->FindEnumeratorByName("Foo1");
    ASSERT_NE(nullptr, enumerator);
    ASSERT_STRCASEEQ("Foo1", enumerator->GetInvariantDisplayLabel().c_str());
    ASSERT_STRCASEEQ("Spaghetti", enumerator->GetString().c_str()); //value
    ASSERT_STRCASEEQ("Foo1", enumerator->GetName().c_str());
    }

//=======================================================================================
//! ECEnumeratorTests
//=======================================================================================

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

//---------------------------------------------------------------------------------------
// @bsimethod                           Kyle.Abramowitz                         08/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECEnumeratorTest, CreateEnumeratorWithoutExplicitName)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ASSERT_TRUE(schema.IsValid());

    ECEnumerationP intEnumeration;
    EC_ASSERT_SUCCESS(schema->CreateEnumeration(intEnumeration, "IntegerTestEnumeration", PrimitiveType::PRIMITIVETYPE_Integer));
    ASSERT_NE(nullptr, intEnumeration);
    ECEnumeratorP intEnumeratorA;
    EC_ASSERT_SUCCESS(intEnumeration->CreateEnumerator(intEnumeratorA, 3));
    ECEnumeratorP intEnumeratorB;
    EC_ASSERT_SUCCESS(intEnumeration->CreateEnumerator(intEnumeratorB, 4));

    EC_EXPECT_SUCCESS(intEnumeratorB->SetInteger(5)) << "Setting a type-matched enumerator to a unique value should succeed.";
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, intEnumeratorB->SetInteger(intEnumeratorA->GetInteger())) << "Setting a type-matched enumerator to a non-unique value should fail.";
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, intEnumeratorA->SetString("banana")) << "Setting a type-mismatched enumerator should fail.";

    ECEnumerationP strEnumeration;
    EC_ASSERT_SUCCESS(schema->CreateEnumeration(strEnumeration, "StringTestEnumeration", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_NE(nullptr, strEnumeration);
    ECEnumeratorP strEnumerator;
    EC_ASSERT_SUCCESS(strEnumeration->CreateEnumerator(strEnumerator, "foo"));
    ECEnumeratorP strEnumeratorB;
    ASSERT_EQ(ECObjectsStatus::NamedItemAlreadyExists, strEnumeration->CreateEnumerator(strEnumeratorB, "foo"));
    EC_ASSERT_SUCCESS(strEnumeration->CreateEnumerator(strEnumeratorB, "bar"));

    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, strEnumerator->SetInteger(9)) << "Setting a type-mismatched enumerator should fail.";
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, strEnumeratorB->SetString(strEnumerator->GetString().c_str())) << "Setting a type-matched enumerator to a non-unique value should fail.";
    }


END_BENTLEY_ECN_TEST_NAMESPACE
