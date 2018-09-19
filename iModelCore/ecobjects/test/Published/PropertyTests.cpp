/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/PropertyTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct PropertyTest : ECTestFixture {};
struct PropertySerializationTest : ECTestFixture {};
struct PropertyDeserializationTest : ECTestFixture 
    {
    void VerifyDeserializedProperty(ECSchemaPtr deserializedSchema, ECPropertyCP expectedProp, Utf8StringCR className, Utf8StringCR propName);

    void RoundTripSchema(ECSchemaCP schema, bvector<ECPropertyCP> expectedProperties, ECVersion versionToSerialize = ECVersion::Latest);
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Muhammad Hassan                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
struct PropertyOverrideTests : ECTestFixture
    {
    ECSchemaPtr RoundTripSchema(ECSchemaPtr testSchema);
    void VerifyPropertyInheritance(ECClassCP ab, ECClassCP cd, ECClassCP ef, ECClassCP gh, ECClassCP ij, ECClassCP kl, ECClassCP mn);
    ECSchemaPtr SetUpBaseSchema();
    void ExpectBasePropertyFromClass(Utf8CP propName, ECClassCP derived, ECClassCP base);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Robert.Schili                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTest, SetGetMinMaxInt)
    {
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    Utf8CP schemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='Foo' >"
        "    </ECEntityClass>"
        "</ECSchema>";

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECClassP cp = schema->GetClassP("Foo");
    ASSERT_NE(cp, nullptr);

    PrimitiveECPropertyP primp;
    ASSERT_EQ(cp->CreatePrimitiveProperty(primp, "Foo"), ECObjectsStatus::Success);
    ASSERT_EQ(primp->SetType(PrimitiveType::PRIMITIVETYPE_Integer), ECObjectsStatus::Success);
    ASSERT_EQ(primp->IsMinimumLengthDefined(), false);
    ASSERT_EQ(primp->IsMaximumLengthDefined(), false);
    ASSERT_EQ(primp->IsMaximumValueDefined(), false);
    ASSERT_EQ(primp->IsMinimumValueDefined(), false);

    ECValue val;
    val.SetUtf8CP("bar");
    ASSERT_EQ(primp->SetMaximumValue(val), ECObjectsStatus::DataTypeNotSupported);
    val.SetInteger(42);
    ASSERT_EQ(primp->SetMaximumValue(val), ECObjectsStatus::Success);
    ASSERT_EQ(primp->IsMaximumValueDefined(), true);
    val.SetToNull(); //ensure val has been copied
    ASSERT_EQ(primp->IsMaximumValueDefined(), true);

    ASSERT_EQ(primp->GetMaximumValue(val), ECObjectsStatus::Success);
    ASSERT_EQ(val.GetInteger(), 42);

    primp->ResetMaximumValue();
    ASSERT_EQ(primp->IsMaximumValueDefined(), false);
    ASSERT_EQ(primp->GetMaximumValue(val), ECObjectsStatus::Error);
    }
    {
    ECSchemaPtr schema;
    ECEntityClassP entity;
    ECStructClassP structClass;
    PrimitiveECPropertyP primProp;
    PrimitiveECPropertyP structProp;
    PrimitiveArrayECPropertyP arrPrimProp;
    
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    schema->CreateEntityClass(entity, "Entity");
    entity->CreatePrimitiveProperty(primProp, "PrimProp");
    primProp->SetType(PrimitiveType::PRIMITIVETYPE_Integer);
    entity->CreatePrimitiveArrayProperty(arrPrimProp, "ArrPrimProp");
    arrPrimProp->SetPrimitiveElementType(PrimitiveType::PRIMITIVETYPE_Integer);

    ECValue arrValue;
    arrValue.SetPrimitiveArrayInfo(PrimitiveType::PRIMITIVETYPE_Integer, 5, true);
    EXPECT_EQ(ECObjectsStatus::DataTypeNotSupported, primProp->SetMaximumValue(arrValue));
    EXPECT_EQ(ECObjectsStatus::DataTypeNotSupported, primProp->SetMinimumValue(arrValue));
    EXPECT_EQ(ECObjectsStatus::DataTypeNotSupported, arrPrimProp->SetMaximumValue(arrValue));
    EXPECT_EQ(ECObjectsStatus::DataTypeNotSupported, arrPrimProp->SetMinimumValue(arrValue));

    schema->CreateStructClass(structClass, "Struct");
    structClass->CreatePrimitiveProperty(structProp, "StructProp");

    IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ECValue structValue;
    structValue.SetStruct(structInstance.get());
    EXPECT_EQ(ECObjectsStatus::DataTypeNotSupported, primProp->SetMaximumValue(structValue));
    EXPECT_EQ(ECObjectsStatus::DataTypeNotSupported, primProp->SetMinimumValue(structValue));
    EXPECT_EQ(ECObjectsStatus::DataTypeNotSupported, arrPrimProp->SetMaximumValue(structValue));
    EXPECT_EQ(ECObjectsStatus::DataTypeNotSupported, arrPrimProp->SetMinimumValue(structValue));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Robert.Schili                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTest, GetSetPropertyMinMaxLength)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    Utf8CP schemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='Foo' >"
        "    </ECEntityClass>"
        "</ECSchema>";

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECClassP cp = schema->GetClassP("Foo");
    ASSERT_NE(cp, nullptr);

    PrimitiveECPropertyP primp;
    ASSERT_EQ(cp->CreatePrimitiveProperty(primp, "Foo"), ECObjectsStatus::Success);
    ASSERT_EQ(primp->SetType(PrimitiveType::PRIMITIVETYPE_String), ECObjectsStatus::Success);
    ASSERT_EQ(primp->IsMinimumLengthDefined(), false);
    ASSERT_EQ(primp->IsMaximumLengthDefined(), false);

    ASSERT_EQ(primp->SetMaximumLength(42), ECObjectsStatus::Success);
    ASSERT_EQ(primp->IsMaximumLengthDefined(), true);
    ASSERT_EQ(primp->GetMaximumLength(), 42);

    primp->ResetMaximumLength();
    ASSERT_EQ(primp->IsMaximumLengthDefined(), false);
    ASSERT_EQ(primp->GetMaximumLength(), 0);

    ASSERT_EQ(primp->SetMinimumLength(10), ECObjectsStatus::Success);
    ASSERT_EQ(primp->IsMinimumLengthDefined(), true);
    ASSERT_EQ(primp->GetMinimumLength(), 10);

    primp->ResetMinimumLength();
    ASSERT_EQ(primp->IsMinimumLengthDefined(), false);
    ASSERT_EQ(primp->GetMinimumLength(), 0);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTest, GetSetCategory)
    {
    ECSchemaPtr schema;
    ECEntityClassP entityClass;
    PropertyCategoryP propertyCategory;
    PrimitiveECPropertyP prop;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0));

    schema->CreatePropertyCategory(propertyCategory, "TestPropertyCategory");
    schema->CreateEntityClass(entityClass, "TestClass");

    entityClass->CreatePrimitiveProperty(prop, "TestProp");
    EXPECT_EQ(ECObjectsStatus::Success, prop->SetCategory(propertyCategory));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTest, GetSetInheritedCategory)
    {
    ECSchemaPtr schema;
    ECEntityClassP entityClass;
    ECEntityClassP derivedEntityClass;
    PropertyCategoryP propertyCategory;
    PrimitiveECPropertyP prop;
    PrimitiveECPropertyP derivedProp;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0));

    schema->CreateEntityClass(entityClass, "TestClass");
    schema->CreateEntityClass(derivedEntityClass, "DerivedClass");
    schema->CreatePropertyCategory(propertyCategory, "TestPropertyCategory");

    entityClass->CreatePrimitiveProperty(prop, "TestProp");
    EXPECT_EQ(ECObjectsStatus::Success, prop->SetCategory(propertyCategory));

    derivedEntityClass->CreatePrimitiveProperty(derivedProp, "TestProp");

    derivedEntityClass->AddBaseClass(*entityClass);
    EXPECT_FALSE(derivedProp->IsCategoryDefinedLocally());
    EXPECT_EQ(derivedProp->GetCategory(), prop->GetCategory());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    09/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTest, SetCategoryFromReferencedSchema)
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECEntityClassP entityClass;
    PropertyCategoryP propertyCategory;
    PrimitiveECPropertyP prop;

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0));
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 1, 0, 0));
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*refSchema));

    schema->CreateEntityClass(entityClass, "TestClass");
    refSchema->CreatePropertyCategory(propertyCategory, "TestPropertyCategory");

    entityClass->CreatePrimitiveProperty(prop, "TestProp");
    EC_EXPECT_SUCCESS(prop->SetCategory(propertyCategory));

    EXPECT_STREQ("RefSchema", prop->GetCategory()->GetSchema().GetName().c_str());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    09/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTest, SetCategoryFromNonReferencedSchema)
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECEntityClassP entityClass;
    PropertyCategoryP propertyCategory;
    PrimitiveECPropertyP prop;

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0));
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 1, 0, 0));

    schema->CreateEntityClass(entityClass, "TestClass");
    refSchema->CreatePropertyCategory(propertyCategory, "TestPropertyCategory");

    entityClass->CreatePrimitiveProperty(prop, "TestProp");
    ASSERT_EQ(ECObjectsStatus::SchemaNotFound, prop->SetCategory(propertyCategory));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    09/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTest, SetKindOfQuantityFromReferencedSchema)
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECEntityClassP entityClass;
    KindOfQuantityP koq;
    PrimitiveECPropertyP prop;

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0));
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 1, 0, 0));
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*refSchema));

    schema->CreateEntityClass(entityClass, "TestClass");
    refSchema->CreateKindOfQuantity(koq, "TestKoQ");

    entityClass->CreatePrimitiveProperty(prop, "TestProp");
    EC_EXPECT_SUCCESS(prop->SetKindOfQuantity(koq));

    EXPECT_STREQ("RefSchema", prop->GetKindOfQuantity()->GetSchema().GetName().c_str());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    09/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTest, SetKindOfQuantityFromNonReferencedSchema)
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECEntityClassP entityClass;
    KindOfQuantityP koq;
    PrimitiveECPropertyP prop;

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0));
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 1, 0, 0));

    schema->CreateEntityClass(entityClass, "TestClass");
    refSchema->CreateKindOfQuantity(koq, "TestKoQ");

    entityClass->CreatePrimitiveProperty(prop, "TestProp");
    ASSERT_EQ(ECObjectsStatus::SchemaNotFound, prop->SetKindOfQuantity(koq));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                Robert.Schili                      11/2015
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTest, TestPrimitiveEnumerationProperty)
    {
    ECSchemaPtr schema;
    ECEntityClassP domainClass;
    ECEnumerationP enumeration;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ASSERT_TRUE(schema.IsValid());

    //Create Enumeration
    auto status = schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_Integer);
    ASSERT_TRUE(enumeration != nullptr);
    ASSERT_TRUE(status == ECObjectsStatus::Success);

    status = schema->CreateEntityClass(domainClass, "Class");
    ASSERT_TRUE(domainClass != nullptr);
    ASSERT_TRUE(status == ECObjectsStatus::Success);

    PrimitiveECPropertyP prop;
    status = domainClass->CreateEnumerationProperty(prop, "MyProperty", *enumeration);
    ASSERT_TRUE(prop != nullptr);
    ASSERT_TRUE(status == ECObjectsStatus::Success);

    ASSERT_TRUE(prop->GetType() == PrimitiveType::PRIMITIVETYPE_Integer);
    ASSERT_TRUE(prop->GetEnumeration() == enumeration);

    prop->SetType(PrimitiveType::PRIMITIVETYPE_Double);
    ASSERT_TRUE(prop->GetEnumeration() == nullptr);

    prop->SetType(*enumeration);
    ASSERT_TRUE(prop->GetType() == PrimitiveType::PRIMITIVETYPE_Integer);
    ASSERT_TRUE(prop->GetEnumeration() == enumeration);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    09/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTest, SetEnumerationFromReferencedSchema)
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECEntityClassP entityClass;
    ECEnumerationP enumeration;
    PrimitiveECPropertyP prop;
    PrimitiveArrayECPropertyP arrProp;

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0));
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 1, 0, 0));
    EC_EXPECT_SUCCESS(schema->AddReferencedSchema(*refSchema));

    schema->CreateEntityClass(entityClass, "TestClass");
    refSchema->CreateEnumeration(enumeration, "TestEnum", PRIMITIVETYPE_Integer);

    entityClass->CreatePrimitiveProperty(prop, "TestProp");
    entityClass->CreatePrimitiveArrayProperty(arrProp, "TestArrProp");
    EC_EXPECT_SUCCESS(prop->SetType(*enumeration));
    EC_EXPECT_SUCCESS(arrProp->SetType(*enumeration));

    EXPECT_STREQ("RefSchema", prop->GetEnumeration()->GetSchema().GetName().c_str());
    EXPECT_STREQ("RefSchema", arrProp->GetEnumeration()->GetSchema().GetName().c_str());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    09/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTest, SetEnumerationFromNonReferencedSchema)
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECEntityClassP entityClass;
    ECEnumerationP enumeration;
    PrimitiveECPropertyP prop;
    PrimitiveArrayECPropertyP arrProp;

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0));
    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 1, 0, 0));

    schema->CreateEntityClass(entityClass, "TestClass");
    refSchema->CreateEnumeration(enumeration, "TestEnum", PRIMITIVETYPE_Integer);

    entityClass->CreatePrimitiveProperty(prop, "TestProp");
    entityClass->CreatePrimitiveArrayProperty(arrProp, "TestArrProp");
    EXPECT_EQ(ECObjectsStatus::SchemaNotFound, prop->SetType(*enumeration));
    EXPECT_EQ(ECObjectsStatus::SchemaNotFound, arrProp->SetType(*enumeration));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTest, GetSetPriority)
    {
    ECSchemaPtr schema;
    ECEntityClassP entityClass;
    PrimitiveECPropertyP prop;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0));

    schema->CreateEntityClass(entityClass, "TestClass");
    entityClass->CreatePrimitiveProperty(prop, "TestProp");
    EXPECT_EQ(ECObjectsStatus::Success, prop->SetPriority(3));

    EXPECT_EQ(3, prop->GetPriority());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyTest, InheritedPriority)
    {
    ECSchemaPtr schema;
    ECEntityClassP entityClass;
    ECEntityClassP derivedEntityClass;
    PrimitiveECPropertyP prop;
    PrimitiveECPropertyP derivedProp;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0));

    int32_t testPriority = -3;

    schema->CreateEntityClass(entityClass, "TestClass");
    schema->CreateEntityClass(derivedEntityClass, "DerivedClass");
    EXPECT_EQ(ECObjectsStatus::Success, derivedEntityClass->AddBaseClass(*entityClass));

    entityClass->CreatePrimitiveProperty(prop, "TestProp");
    EXPECT_EQ(ECObjectsStatus::Success, prop->SetPriority(testPriority));

    derivedEntityClass->CreatePrimitiveProperty(derivedProp, "TestProp");
    EXPECT_EQ(testPriority, derivedProp->GetPriority());
    }

void createECProperty(PrimitiveECPropertyP& prop, ECEntityClassP class1, Utf8CP name, PrimitiveType primitiveType = PRIMITIVETYPE_Integer)
    {
    class1->CreatePrimitiveProperty(prop, name, primitiveType);
    prop->SetDisplayLabel("TestProp Display Label");
    prop->SetDescription("TestProp Description");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyTest, CompareProperties)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    PrimitiveECPropertyP prop1;
    PrimitiveECPropertyP stringProp;

    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0));
    schema->CreateEntityClass(class1, "TestClass");
    createECProperty(prop1, class1, "TestProp");
    createECProperty(stringProp, class1, "StringProp", PRIMITIVETYPE_String);

    // Properties have different name
        {
        ECEntityClassP testClass;
        PrimitiveECPropertyP testProp;
        schema->CreateEntityClass(testClass, "TestClass2");
        createECProperty(testProp, testClass, "TestProp2");
        EXPECT_FALSE(testProp->IsSame(*prop1));
        }

    // Properties have different type
        {
        ECEntityClassP testClass;
        PrimitiveECPropertyP testProp;
        schema->CreateEntityClass(testClass, "TestClass3");
        createECProperty(testProp, testClass, "TestProp", PRIMITIVETYPE_String);
        EXPECT_FALSE(testProp->IsSame(*prop1));
        }

    // Properties have different display labels
        {
        ECEntityClassP testClass;
        PrimitiveECPropertyP testProp;
        schema->CreateEntityClass(testClass, "TestClass4");
        createECProperty(testProp, testClass, "TestProp");
        testProp->SetDisplayLabel("TestProp Display Label2");
        EXPECT_FALSE(testProp->IsSame(*prop1));
        }

    // Properties have different descriptions
        {
        ECEntityClassP testClass;
        PrimitiveECPropertyP testProp;
        schema->CreateEntityClass(testClass, "TestClass5");
        createECProperty(testProp, testClass, "TestProp");
        testProp->SetDescription("TestProp Description2");
        EXPECT_FALSE(testProp->IsSame(*prop1));
        }

    // Minimum Value
        {
        ECEntityClassP testClass;
        PrimitiveECPropertyP testProp;
        schema->CreateEntityClass(testClass, "TestClass6");
        createECProperty(testProp, testClass, "TestProp");
        ECValue min1(1);
        prop1->SetMinimumValue(min1);
        EXPECT_FALSE(testProp->IsSame(*prop1));
        ECValue min2(2);
        testProp->SetMinimumValue(min2);
        EXPECT_FALSE(testProp->IsSame(*prop1));
        prop1->SetMinimumValue(min2);
        EXPECT_TRUE(testProp->IsSame(*prop1));
        prop1->ResetMinimumValue();
        EXPECT_FALSE(testProp->IsSame(*prop1));
        }

    // Maximum Value
        {
        ECEntityClassP testClass;
        PrimitiveECPropertyP testProp;
        schema->CreateEntityClass(testClass, "TestClass7");
        createECProperty(testProp, testClass, "TestProp");
        ECValue max1(100);
        prop1->SetMaximumValue(max1);
        EXPECT_FALSE(testProp->IsSame(*prop1));
        ECValue max2(200);
        testProp->SetMaximumValue(max2);
        EXPECT_FALSE(testProp->IsSame(*prop1));
        prop1->SetMaximumValue(max2);
        EXPECT_TRUE(testProp->IsSame(*prop1));
        prop1->ResetMaximumValue();
        EXPECT_FALSE(testProp->IsSame(*prop1));
        }

    // Minimum Length
        {
        ECEntityClassP testClass;
        PrimitiveECPropertyP testProp;
        schema->CreateEntityClass(testClass, "TestClass8");
        createECProperty(testProp, testClass, "StringProp", PRIMITIVETYPE_String);
        testProp->SetMinimumLength(2);
        EXPECT_FALSE(testProp->IsSame(*stringProp));
        stringProp->SetMinimumLength(3);
        EXPECT_FALSE(testProp->IsSame(*stringProp));
        testProp->SetMinimumLength(3);
        EXPECT_TRUE(testProp->IsSame(*stringProp));
        testProp->ResetMinimumLength();
        EXPECT_FALSE(testProp->IsSame(*stringProp));
        stringProp->ResetMinimumLength();
        EXPECT_TRUE(testProp->IsSame(*stringProp));
        }

    // Maximum Length
        {
        ECEntityClassP testClass;
        PrimitiveECPropertyP testProp;
        schema->CreateEntityClass(testClass, "TestClass9");
        createECProperty(testProp, testClass, "StringProp", PRIMITIVETYPE_String);
        testProp->SetMaximumLength(200);
        EXPECT_FALSE(testProp->IsSame(*stringProp));
        stringProp->SetMaximumLength(300);
        EXPECT_FALSE(testProp->IsSame(*stringProp));
        testProp->SetMaximumLength(300);
        EXPECT_TRUE(testProp->IsSame(*stringProp));
        testProp->ResetMaximumLength();
        EXPECT_FALSE(testProp->IsSame(*stringProp));
        stringProp->ResetMaximumLength();
        EXPECT_TRUE(testProp->IsSame(*stringProp));
        }

    // IsReadOnly
        {
        ECEntityClassP testClass;
        PrimitiveECPropertyP testProp;
        schema->CreateEntityClass(testClass, "TestClass10");
        createECProperty(testProp, testClass, "TestProp");
        prop1->SetIsReadOnly(true);
        EXPECT_FALSE(testProp->IsSame(*prop1));
        testProp->SetIsReadOnly(true);
        EXPECT_TRUE(testProp->IsSame(*prop1));
        testProp->SetIsReadOnly(false);
        EXPECT_FALSE(testProp->IsSame(*prop1));
        prop1->SetIsReadOnly(false);
        EXPECT_TRUE(testProp->IsSame(*prop1));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                       06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyDeserializationTest::VerifyDeserializedProperty(ECSchemaPtr deserializedSchema, ECPropertyCP expectedProp, Utf8StringCR className, Utf8StringCR propName)
    {
    ECClassCP deserializedClass = deserializedSchema->GetClassCP(className.c_str());
    ASSERT_TRUE(nullptr != deserializedClass) << "Class '" << className << "' containing property not found in deserialized schema";

    ECPropertyP ecProp = deserializedClass->GetPropertyP(propName);
    ASSERT_TRUE(nullptr != ecProp) << "Property '" << propName << "' not found in deserialized schema";
    
    ASSERT_EQ(expectedProp->GetName(), ecProp->GetName()) << "Property '" << propName << "' does not have correct name in deserialized schema";
    ASSERT_EQ(expectedProp->GetPriority(), ecProp->GetPriority()) << "Property '" << propName << "' does not have correct priority in deserialized schema";
    ASSERT_EQ(expectedProp->IsPriorityLocallyDefined(), ecProp->IsPriorityLocallyDefined());
    ASSERT_EQ(expectedProp->IsMaximumValueDefined(), ecProp->IsMaximumValueDefined());
    ASSERT_EQ(expectedProp->IsMinimumValueDefined(), ecProp->IsMinimumValueDefined());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                       06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyDeserializationTest::RoundTripSchema(ECSchemaCP schema, bvector<ECPropertyCP> expectedProperties, ECVersion versionToRoundTrip)
    {
    Utf8String schemaString;
    SchemaWriteStatus writeStatus = schema->WriteToXmlString(schemaString, versionToRoundTrip);
    ASSERT_EQ(SchemaWriteStatus::Success, writeStatus) << "Failed to serialize schema";

    ECSchemaPtr deserializedSchema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus readStatus = ECSchema::ReadFromXmlString(deserializedSchema, schemaString.c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, readStatus) << "Failed to deserialize schema";
    for (auto const& prop : expectedProperties)
        VerifyDeserializedProperty(deserializedSchema, prop, prop->GetClass().GetName(), prop->GetName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Robert.Schili                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyDeserializationTest, MinMaxValue)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    Utf8CP schemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='Foo'>"
        "       <ECProperty propertyName='DoubleProp' typeName='double' minimumValue='3.0' maximumValue='42'/>"
        "    </ECEntityClass>"
        "</ECSchema>";

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECClassP cp = schema->GetClassP("Foo");
    ASSERT_NE(cp, nullptr);

    ECPropertyP pp = cp->GetPropertyP("DoubleProp");
    ASSERT_NE(pp, nullptr);

    ECValue minVal;
    ECValue maxVal;
    ASSERT_EQ(pp->GetMinimumValue(minVal), ECObjectsStatus::Success);
    ASSERT_EQ(pp->GetMaximumValue(maxVal), ECObjectsStatus::Success);

    ASSERT_EQ(minVal.IsNull(), false);
    ASSERT_EQ(maxVal.IsNull(), false);

    ASSERT_EQ(minVal.GetPrimitiveType(), PrimitiveType::PRIMITIVETYPE_Double);
    ASSERT_EQ(maxVal.GetPrimitiveType(), PrimitiveType::PRIMITIVETYPE_Double);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                      06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyDeserializationTest, MinMaxValue_EC2)
    {
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='utf-8'?>
        <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>
            <ECClass typeName='Foo' isDomainClass="true">
               <ECProperty propertyName='DoubleProp' typeName='double' MinimumValue='3.0' MaximumValue='42'/>
            </ECClass>
        </ECSchema>)xml";

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECClassP cp = schema->GetClassP("Foo");
    ASSERT_NE(cp, nullptr);

    ECPropertyP pp = cp->GetPropertyP("DoubleProp");
    ASSERT_NE(pp, nullptr);

    ECValue minVal;
    ECValue maxVal;
    ASSERT_EQ(pp->GetMinimumValue(minVal), ECObjectsStatus::Success);
    ASSERT_EQ(pp->GetMaximumValue(maxVal), ECObjectsStatus::Success);

    ASSERT_EQ(minVal.IsNull(), false);
    ASSERT_EQ(maxVal.IsNull(), false);

    ASSERT_EQ(minVal.GetPrimitiveType(), PrimitiveType::PRIMITIVETYPE_Double);
    ASSERT_EQ(maxVal.GetPrimitiveType(), PrimitiveType::PRIMITIVETYPE_Double);

    bvector<ECPropertyCP> expectedProps;
    expectedProps.push_back(pp);
    RoundTripSchema(schema.get(), expectedProps, ECVersion::V2_0);
    }
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='utf-8'?>
        <ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>
            <ECClass typeName='Foo' isDomainClass="true">
               <ECProperty propertyName='DoubleProp' typeName='double' minimumValue='3.0' maximumValue='42'/>
            </ECClass>
        </ECSchema>)xml";

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECClassP cp = schema->GetClassP("Foo");
    ASSERT_NE(cp, nullptr);
    
    ECPropertyP pp = cp->GetPropertyP("DoubleProp");
    ASSERT_NE(pp, nullptr);

    ASSERT_FALSE(pp->IsMinimumValueDefined());
    ASSERT_FALSE(pp->IsMaximumValueDefined());

    bvector<ECPropertyCP> expectedProps;
    expectedProps.push_back(pp);
    RoundTripSchema(schema.get(), expectedProps, ECVersion::V2_0);
    }
    }

void TestMinMaxValueTypeEnforcement(Utf8CP primitiveType, bool shouldFail)
    {
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='utf-8'?>
        <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
            <ECEntityClass typeName='Foo'>
               <ECProperty propertyName='DoubleProp' typeName='%s' minimumValue='3.0' maximumValue='42'/>
            </ECEntityClass>
        </ECSchema>)xml";

    Utf8String formattedSchemaXml;
    formattedSchemaXml.Sprintf(schemaXml, primitiveType);

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus readStatus = ECSchema::ReadFromXmlString(schema, formattedSchemaXml.c_str(), *context);
    if (shouldFail)
        {
        ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, readStatus) << "A property with primitive type " << primitiveType << " should fail to deserialize.";
        return;
        }

    ASSERT_EQ(SchemaReadStatus::Success, readStatus) << "A property with primitive type " << primitiveType << " should deserialize successfully.";
    ASSERT_TRUE(schema.IsValid()) << "The schema is invalid even though deserialization was successful";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                      06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyDeserializationTest, MinMaxValueTypeEnforcement)
    {
    TestMinMaxValueTypeEnforcement("double", false);
    TestMinMaxValueTypeEnforcement("int", false);
    TestMinMaxValueTypeEnforcement("long", false);
    TestMinMaxValueTypeEnforcement("binary", true);
    TestMinMaxValueTypeEnforcement("string", true);
    TestMinMaxValueTypeEnforcement("bool", true);
    TestMinMaxValueTypeEnforcement("dateTime", true);
    TestMinMaxValueTypeEnforcement("Bentley.Geometry.Common.IGeometry", true);
    TestMinMaxValueTypeEnforcement("point2d", true);
    TestMinMaxValueTypeEnforcement("point3d", true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                      01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyDeserializationTest, MinMaxLength)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    Utf8CP schemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='Foo'>"
        "       <ECProperty propertyName='StringProp' typeName='string' minimumLength='3' maximumLength='42'/>"
        "    </ECEntityClass>"
        "</ECSchema>";

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECClassP cp = schema->GetClassP("Foo");
    ASSERT_NE(cp, nullptr);

    ECPropertyP pp = cp->GetPropertyP("StringProp");
    ASSERT_NE(pp, nullptr);

    EXPECT_TRUE(pp->IsMinimumLengthDefined());
    EXPECT_TRUE(pp->IsMaximumLengthDefined());

    EXPECT_EQ(3, pp->GetMinimumLength());
    EXPECT_EQ(42, pp->GetMaximumLength());
    }

void TestMinMaxLengthTypeEnforcement(Utf8CP primitiveType, bool shouldFail)
    {
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='utf-8'?>
        <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
            <ECEntityClass typeName='Foo'>
               <ECProperty propertyName='DoubleProp' typeName='%s' minimumLength='3.0' maximumLength='42'/>
            </ECEntityClass>
        </ECSchema>)xml";

    Utf8String formattedSchemaXml;
    formattedSchemaXml.Sprintf(schemaXml, primitiveType);

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus readStatus = ECSchema::ReadFromXmlString(schema, formattedSchemaXml.c_str(), *context);
    if (shouldFail)
        {
        ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, readStatus) << "A property with primitive type " << primitiveType << " should fail to deserialize.";
        return;
        }

    ASSERT_EQ(SchemaReadStatus::Success, readStatus) << "A property with primitive type " << primitiveType << " should deserialize successfully.";
    ASSERT_TRUE(schema.IsValid()) << "The schema is invalid even though deserialization was successful";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                      06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyDeserializationTest, MinMaxLengthTypeEnforcement)
    {
    TestMinMaxLengthTypeEnforcement("binary", false);
    TestMinMaxLengthTypeEnforcement("string", false);
    TestMinMaxLengthTypeEnforcement("double", true);
    TestMinMaxLengthTypeEnforcement("int", true);
    TestMinMaxLengthTypeEnforcement("long", true);
    TestMinMaxLengthTypeEnforcement("bool", true);
    TestMinMaxLengthTypeEnforcement("dateTime", true);
    TestMinMaxLengthTypeEnforcement("Bentley.Geometry.Common.IGeometry", true);
    TestMinMaxLengthTypeEnforcement("point2d", true);
    TestMinMaxLengthTypeEnforcement("point3d", true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                      06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyDeserializationTest, Priority)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='utf-8'?>
        <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
            <ECEntityClass typeName='Foo'>"
               <ECProperty propertyName='StringProp' typeName='string' priority="2"/>
            </ECEntityClass>
        </ECSchema>)xml";

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECClassP cp = schema->GetClassP("Foo");
    ASSERT_NE(cp, nullptr);

    ECPropertyP pp = cp->GetPropertyP("StringProp");
    ASSERT_NE(pp, nullptr);

    EXPECT_EQ(2, pp->GetPriority());

    bvector<ECPropertyCP> expectedProps;
    expectedProps.push_back(pp);
    RoundTripSchema(schema.get(), expectedProps);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                      06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyDeserializationTest, InheritedPriority)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding='utf-8'?>
        <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
            <ECEntityClass typeName='Foo'>
                <ECProperty propertyName='StringProp' typeName='string' priority="2"/>
            </ECEntityClass>
            <ECEntityClass typeName='Foo2'>
                <BaseClass>Foo</BaseClass>
                <ECProperty propertyName='StringProp' typeName='string' priority="-4"/>
            </ECEntityClass>
        </ECSchema>)xml";

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECClassP foo = schema->GetClassP("Foo");
    ASSERT_NE(foo, nullptr);

    ECPropertyP pp = foo->GetPropertyP("StringProp");
    ASSERT_NE(pp, nullptr);

    EXPECT_EQ(2, pp->GetPriority());

    ECClassP foo2 = schema->GetClassP("Foo2");
    ASSERT_NE(foo2, nullptr);

    ECPropertyP pp2 = foo2->GetPropertyP("StringProp");
    ASSERT_NE(pp2, nullptr);

    EXPECT_EQ(-4, pp2->GetPriority());

    bvector<ECPropertyCP> expectedProps;
    expectedProps.push_back(pp);
    expectedProps.push_back(pp2);
    RoundTripSchema(schema.get(), expectedProps);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                      06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyDeserializationTest, CategoryAttribute)
    {
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding="utf-8"?>
        <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
            <ECEntityClass typeName='Foo'>
               <ECProperty propertyName='StringProp' typeName='string' category="testPropCategory"/>
            </ECEntityClass>
            <PropertyCategory typeName="testPropCategory" displayLabel="PropertyCategory" description="This is an awesome new Property Category" priority="0"/>
        </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECClassCP ecClass = schema->GetClassCP("Foo");
    ASSERT_NE(nullptr, ecClass);

    ECPropertyP ecProp = ecClass->GetPropertyP("StringProp");
    ASSERT_NE(nullptr, ecProp);

    PropertyCategoryCP propCategory = ecProp->GetCategory();
    EXPECT_NE(nullptr, propCategory);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                      06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyDeserializationTest, CategoryFromReferencedSchema)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();

    {
    Utf8CP refSchemaXml = R"xml(<?xml version='1.0' encoding="utf-8"?>
        <ECSchema schemaName='RefSchemaWithPropertyCategory' alias='rswpc' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
            <PropertyCategory typeName="testPropCategory" displayLabel="PropertyCategory" description="This is an awesome new Property Category" priority="0"/>
        </ECSchema>)xml";

    ECSchemaPtr refSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refSchemaXml, *context));
    }

    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding="utf-8"?>
        <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
            <ECSchemaReference name='RefSchemaWithPropertyCategory' version='01.00' alias='rswpc' />
            <ECEntityClass typeName='Foo'>
                <ECProperty propertyName='StringProp' typeName='string' category="rswpc:testPropCategory"/>
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECClassCP ecClass = schema->GetClassCP("Foo");
    ASSERT_NE(nullptr, ecClass);

    ECPropertyP ecProp = ecClass->GetPropertyP("StringProp");
    ASSERT_NE(nullptr, ecProp);

    PropertyCategoryCP propCategory = ecProp->GetCategory();
    EXPECT_NE(nullptr, propCategory);
    EXPECT_STREQ("testPropCategory", propCategory->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                      06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyDeserializationTest, CategoryFromReferencedSchema_MissingSchemaReference)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding="utf-8"?>
        <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
            <ECSchemaReference name='RefSchemaWithPropertyCategory' version='01.00' alias='rswpc' />
            <ECEntityClass typeName='Foo'>
                <ECProperty propertyName='StringProp' typeName='string' category="rswpc:testPropCategory"/>
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::ReferencedSchemaNotFound, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    {

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    Utf8CP refSchemaXml = R"xml(<?xml version='1.0' encoding="utf-8"?>
        <ECSchema schemaName='RefSchemaWithPropertyCategory' alias='rswpc' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
        </ECSchema>)xml";

    ECSchemaPtr refSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refSchemaXml, *context));

    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding="utf-8"?>
        <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
            <ECSchemaReference name='RefSchemaWithPropertyCategory' version='01.00' alias='rswpc' />
            <ECEntityClass typeName='Foo'>
                <ECProperty propertyName='StringProp' typeName='string' category="rswpc:testPropCategory"/>
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                      06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyDeserializationTest, CategoryAttributeInherited)
    {
    Utf8CP schemaXml = R"xml(<?xml version='1.0' encoding="utf-8"?>
        <ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
            <ECEntityClass typeName='Foo'>
                <ECProperty propertyName='StringProp' typeName='string' category="testPropCategory"/>
            </ECEntityClass>
            <ECEntityClass typeName='Foo2'>
                <BaseClass>Foo</BaseClass>
                <ECProperty propertyName='StringProp' typeName='string'/>
            </ECEntityClass>
            <PropertyCategory typeName="testPropCategory" displayLabel="PropertyCategory" description="This is an awesome new Property Category" priority="0"/>
        </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ECClassCP ecClass = schema->GetClassCP("Foo");
    ASSERT_NE(nullptr, ecClass);

    ECPropertyP ecProp = ecClass->GetPropertyP("StringProp");
    ASSERT_NE(nullptr, ecProp);

    PropertyCategoryCP propCategory = ecProp->GetCategory();
    EXPECT_NE(nullptr, propCategory);

    ECPropertyP derivedProp = ecClass->GetPropertyP("StringProp");
    ASSERT_NE(nullptr, derivedProp);
    
    PropertyCategoryCP derivedPropCategory = derivedProp->GetCategory();
    EXPECT_NE(nullptr, derivedPropCategory);

    EXPECT_EQ(derivedPropCategory, propCategory);
    EXPECT_STREQ(derivedPropCategory->GetFullName().c_str(), propCategory->GetFullName().c_str());
    }

//---------------------------------------------------------------------------------------
// This is only needed for Primitive and PrimitiveArray properties
// @bsimethod                                   Caleb.Shafer                      06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertySerializationTest, KindOfQuantityAndExtendedTypeNameRoundtrip)
    {
    Utf8String schemaXml;

    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    ECSchemaR unitsSchema = *ECTestFixture::GetUnitsSchema();
    schema->AddReferencedSchema(unitsSchema);

    ECEntityClassP entity;
    EC_EXPECT_SUCCESS(schema->CreateEntityClass(entity, "TestEntity"));

    KindOfQuantityP koq;
    EC_EXPECT_SUCCESS(schema->CreateKindOfQuantity(koq, "KoQ"));
    koq->SetRelativeError(5);
    
    ECUnitCP mmUnit = unitsSchema.GetUnitCP("MM");
    EXPECT_NE(nullptr, mmUnit);
    EXPECT_EQ(ECObjectsStatus::Success, koq->SetPersistenceUnit(*mmUnit));

    PrimitiveECPropertyP primProp;
    EC_EXPECT_SUCCESS(entity->CreatePrimitiveProperty(primProp, "TestPrimProp"));
    EC_EXPECT_SUCCESS(primProp->SetExtendedTypeName("Json"));
    EC_EXPECT_SUCCESS(primProp->SetKindOfQuantity(koq));

    PrimitiveArrayECPropertyP primArr;
    EC_EXPECT_SUCCESS(entity->CreatePrimitiveArrayProperty(primArr, "TestPrimArrProp"));
    EC_EXPECT_SUCCESS(primArr->SetExtendedTypeName("Json"));
    EC_EXPECT_SUCCESS(primArr->SetKindOfQuantity(koq));
    
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(schemaXml));
    }

    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *context);

    ECEntityClassCP entity = schema->GetClassCP("TestEntity")->GetEntityClassCP();

    PrimitiveECPropertyCP primProp = entity->GetPropertyP("TestPrimProp")->GetAsPrimitiveProperty();
    EXPECT_STREQ("Json", primProp->GetExtendedTypeName().c_str());

    EXPECT_TRUE(primProp->IsKindOfQuantityDefinedLocally());
    KindOfQuantityCP primKoQ = primProp->GetKindOfQuantity();
    EXPECT_STREQ("KoQ", primKoQ->GetName().c_str());

    PrimitiveArrayECPropertyCP primArrProp = entity->GetPropertyP("TestPrimArrProp")->GetAsPrimitiveArrayProperty();
    EXPECT_STREQ("Json", primArrProp->GetExtendedTypeName().c_str());

    EXPECT_TRUE(primArrProp->IsKindOfQuantityDefinedLocally());
    KindOfQuantityCP primArrKoQ = primArrProp->GetKindOfQuantity();
    EXPECT_STREQ("KoQ", primArrKoQ->GetName().c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyOverrideTests::ExpectBasePropertyFromClass (Utf8CP propName, ECClassCP derived, ECClassCP base)
    {
    auto baseProp = derived->GetPropertyP (propName)->GetBaseProperty();
    EXPECT_TRUE (baseProp->GetClass().GetName().Equals (base->GetName()))
        << "Base property " << propName
        << " expected from class " << base->GetName().c_str()
        << " actually from class " << baseProp->GetClass().GetName().c_str();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaPtr PropertyOverrideTests::RoundTripSchema(ECSchemaPtr testSchema)
    {
    ECSchemaPtr tempSchema;
    Utf8String schemaXml;
    EXPECT_EQ(SchemaWriteStatus::Success, testSchema->WriteToXmlString(schemaXml));
    ECSchemaReadContextPtr deserializedSchemaContext = ECSchemaReadContext::CreateContext();
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(tempSchema, schemaXml.c_str(), *deserializedSchemaContext));
    return tempSchema;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyOverrideTests::VerifyPropertyInheritance(ECClassCP ab, ECClassCP cd, ECClassCP ef, ECClassCP gh, ECClassCP ij, ECClassCP kl, ECClassCP mn)
    {
    Utf8CP propName = "a";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ab->GetName()));
    propName = "b";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ExpectBasePropertyFromClass (propName, mn, ab);
    propName = "c";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(cd->GetName()));
    propName = "d";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ExpectBasePropertyFromClass (propName, mn, cd);
    propName = "e";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ef->GetName()));
    propName = "f";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ExpectBasePropertyFromClass (propName, mn, ef);
    propName = "g";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ab->GetName()));
    propName = "h";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ExpectBasePropertyFromClass (propName, mn, ab);
    propName = "i";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(gh->GetName()));
    propName = "j";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ExpectBasePropertyFromClass (propName, mn, ij);
    propName = "k";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ExpectBasePropertyFromClass (propName, mn, kl);
    propName = "l";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ef->GetName()));
    propName = "m";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    propName = "n";
    EXPECT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaPtr PropertyOverrideTests::SetUpBaseSchema()
    {
    ECSchemaPtr testSchema;
    Utf8String schemaName = "testSchema";
    Utf8String alias = "ts";
    EXPECT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(testSchema, schemaName, alias, 1, 0, 0));

    ECEntityClassP root = NULL;
    EXPECT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(root, "Root"));
    ECEntityClassP b1 = NULL;
    EXPECT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(b1, "B1"));
    ECEntityClassP b2 = NULL;
    EXPECT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(b2, "B2"));
    ECEntityClassP foo = NULL;
    EXPECT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(foo, "Foo"));

    return testSchema;
    }
/*---------------------------------------------------------------------------------------
 <summary>Creates a class chain and add properties and then verifies if they
 come in the expected sequence.</summary>
 <Scenario>
 This is the class hierarchy used in this test. The numbers indicate override priority,
 and the letters indicate ECClass name and their inital properties, e.g.
 "4cd" represents the ECClass named "cd" which has ECProperties named "c" and "d"
 and which is 4th overall in override priority... it can override properties from ECClass
 "kl", but not properties from "ab".

 //     3ab 4cd 6gh 7ij
 //       \/      \/
 //       2ef    5kl
 //          \  /
 //          1mn
 </scenario>
 @bsimethod                              Muhammad Hassan                         07/15
 -------------+---------------+---------------+---------------+---------------+---------*/
TEST_F(PropertyOverrideTests, PropertyOverrideInMultiInheritance)
    {
    //create Schema
    ECSchemaPtr testSchema;
    Utf8String schemaName = "testSchema";
    Utf8String alias = "ts";
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(testSchema, schemaName, alias, 1, 0, 0));

    ECEntityClassP ab = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(ab, "ab"));
    ECEntityClassP cd = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(cd, "cd"));
    ECEntityClassP ef = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(ef, "ef"));
    ECEntityClassP gh = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(gh, "gh"));
    ECEntityClassP ij = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(ij, "ij"));
    ECEntityClassP kl = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(kl, "kl"));
    ECEntityClassP mn = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(mn, "mn"));

    //add base classes of ef
    ef->AddBaseClass(*ab);
    ef->AddBaseClass(*cd);

    //add base classes of kl
    kl->AddBaseClass(*gh);
    kl->AddBaseClass(*ij);

    //add base classes of mn
    mn->AddBaseClass(*ef);
    mn->AddBaseClass(*kl);

    PrimitiveECPropertyP primitiveProperty;
    ASSERT_EQ(ECObjectsStatus::Success, ab->CreatePrimitiveProperty(primitiveProperty, "a", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, ab->CreatePrimitiveProperty(primitiveProperty, "b", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, cd->CreatePrimitiveProperty(primitiveProperty, "c", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, cd->CreatePrimitiveProperty(primitiveProperty, "d", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, ef->CreatePrimitiveProperty(primitiveProperty, "e", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, ef->CreatePrimitiveProperty(primitiveProperty, "f", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, gh->CreatePrimitiveProperty(primitiveProperty, "g", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, gh->CreatePrimitiveProperty(primitiveProperty, "h", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, ij->CreatePrimitiveProperty(primitiveProperty, "i", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, ij->CreatePrimitiveProperty(primitiveProperty, "j", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, kl->CreatePrimitiveProperty(primitiveProperty, "k", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, kl->CreatePrimitiveProperty(primitiveProperty, "l", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, mn->CreatePrimitiveProperty(primitiveProperty, "m", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, mn->CreatePrimitiveProperty(primitiveProperty, "n", PrimitiveType::PRIMITIVETYPE_String));

    //verify that properties successfully added for classes
    Utf8CP propName = "a";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ab->GetName()));
    propName = "b";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ab->GetName()));
    propName = "c";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(cd->GetName()));
    propName = "d";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(cd->GetName()));
    propName = "e";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ef->GetName()));
    propName = "f";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ef->GetName()));
    propName = "g";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(gh->GetName()));
    propName = "h";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(gh->GetName()));
    propName = "i";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ij->GetName()));
    propName = "j";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ij->GetName()));
    propName = "k";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(kl->GetName()));
    propName = "l";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(kl->GetName()));
    propName = "m";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    propName = "n";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));

    //now we add some duplicate properties to mn, which will "override" those from the base classes 
    mn->CreatePrimitiveProperty(primitiveProperty, "b", PrimitiveType::PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(primitiveProperty, "d", PrimitiveType::PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(primitiveProperty, "f", PrimitiveType::PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(primitiveProperty, "h", PrimitiveType::PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(primitiveProperty, "j", PrimitiveType::PRIMITIVETYPE_String);
    mn->CreatePrimitiveProperty(primitiveProperty, "k", PrimitiveType::PRIMITIVETYPE_String);

    propName = "a";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ab->GetName()));
    propName = "b";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetBaseProperty()->GetClass()).GetName().Equals(ab->GetName()));
    propName = "c";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(cd->GetName()));
    propName = "d";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetBaseProperty()->GetClass()).GetName().Equals(cd->GetName()));
    propName = "e";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ef->GetName()));
    propName = "f";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetBaseProperty()->GetClass()).GetName().Equals(ef->GetName()));
    propName = "g";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(gh->GetName()));
    propName = "h";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetBaseProperty()->GetClass()).GetName().Equals(gh->GetName()));
    propName = "i";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(ij->GetName()));
    propName = "j";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetBaseProperty()->GetClass()).GetName().Equals(ij->GetName()));
    propName = "k";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetBaseProperty()->GetClass()).GetName().Equals(kl->GetName()));
    propName = "l";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(kl->GetName()));
    propName = "m";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));
    propName = "n";
    ASSERT_TRUE((mn->GetPropertyP(propName)->GetClass()).GetName().Equals(mn->GetName()));

    //Override more properties of base classes (Add eacg to kl, iab to gh, l to ef, g to ij and gh to ab) and verify property inheritance
    ASSERT_EQ(ECObjectsStatus::Success, kl->CreatePrimitiveProperty(primitiveProperty, "e", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, kl->CreatePrimitiveProperty(primitiveProperty, "a", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, kl->CreatePrimitiveProperty(primitiveProperty, "c", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, kl->CreatePrimitiveProperty(primitiveProperty, "g", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(ECObjectsStatus::Success, gh->CreatePrimitiveProperty(primitiveProperty, "a", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, gh->CreatePrimitiveProperty(primitiveProperty, "b", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, gh->CreatePrimitiveProperty(primitiveProperty, "i", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(ECObjectsStatus::Success, ef->CreatePrimitiveProperty(primitiveProperty, "l", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(ECObjectsStatus::Success, ij->CreatePrimitiveProperty(primitiveProperty, "g", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(ECObjectsStatus::Success, ab->CreatePrimitiveProperty(primitiveProperty, "g", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, ab->CreatePrimitiveProperty(primitiveProperty, "h", PrimitiveType::PRIMITIVETYPE_String));

    VerifyPropertyInheritance(ab, cd, ef, gh, ij, kl, mn);

    //Roundtrip Schema and test that order is still the same
    ECSchemaPtr testSchemaCopy1 = RoundTripSchema(testSchema);
    ECSchemaPtr testSchemaCopy2 = RoundTripSchema(testSchemaCopy1);

    ECClassCP ab1 = testSchemaCopy1->GetClassCP("ab");
    ASSERT_TRUE(ab1 != nullptr);
    ECClassCP cd1 = testSchemaCopy1->GetClassCP("cd");
    ASSERT_TRUE(cd1 != nullptr);
    ECClassCP ef1 = testSchemaCopy1->GetClassCP("ef");
    ASSERT_TRUE(ef1 != nullptr);
    ECClassCP gh1 = testSchemaCopy1->GetClassCP("gh");
    ASSERT_TRUE(gh1 != nullptr);
    ECClassCP ij1 = testSchemaCopy1->GetClassCP("ij");
    ASSERT_TRUE(ij1 != nullptr);
    ECClassCP kl1 = testSchemaCopy1->GetClassCP("kl");
    ASSERT_TRUE(kl1 != nullptr);
    ECClassCP mn1 = testSchemaCopy1->GetClassCP("mn");
    ASSERT_TRUE(mn1 != nullptr);
    //Verify Property Inheritance for the 1st copy of Schema (after xml Deserialization)
    VerifyPropertyInheritance(ab1, cd1, ef1, gh1, ij1, kl1, mn1);

    ECClassCP ab2 = testSchemaCopy2->GetClassCP("ab");
    ASSERT_TRUE(ab2 != nullptr);
    ECClassCP cd2 = testSchemaCopy2->GetClassCP("cd");
    ASSERT_TRUE(cd2 != nullptr);
    ECClassCP ef2 = testSchemaCopy2->GetClassCP("ef");
    ASSERT_TRUE(ef2 != nullptr);
    ECClassCP gh2 = testSchemaCopy2->GetClassCP("gh");
    ASSERT_TRUE(gh2 != nullptr);
    ECClassCP ij2 = testSchemaCopy2->GetClassCP("ij");
    ASSERT_TRUE(ij2 != nullptr);
    ECClassCP kl2 = testSchemaCopy2->GetClassCP("kl");
    ASSERT_TRUE(kl2 != nullptr);
    ECClassCP mn2 = testSchemaCopy2->GetClassCP("mn");
    ASSERT_TRUE(mn2 != nullptr);
    //Verify Property Inheritance for the 2st copy of Schema (after xml Deserialization)
    VerifyPropertyInheritance(ab2, cd2, ef2, gh2, ij2, kl2, mn2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PropertyOverrideTests, AddingBasePropertyOverrideChangesPropertyInDerivedClass)
    {
    ECSchemaPtr testSchema;
    Utf8String schemaName = "testSchema";
    Utf8String alias = "ts";
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(testSchema, schemaName, alias, 1, 0, 0));
    ECEntityClassP baseClass = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(baseClass, "BaseClass"));
    PrimitiveECPropertyP primitiveProperty;
    ASSERT_EQ(ECObjectsStatus::Success, baseClass->CreatePrimitiveProperty(primitiveProperty, "testProp", PrimitiveType::PRIMITIVETYPE_String));
    ECEntityClassP derivedClass = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(derivedClass, "DerivedClass"));
    derivedClass->AddBaseClass(*baseClass);
    ECEntityClassP doubleDerivedClass = NULL;
    ASSERT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(doubleDerivedClass, "DoubleDerivedClass"));
    doubleDerivedClass->AddBaseClass(*derivedClass);

    ASSERT_EQ(doubleDerivedClass->GetPropertyP("testProp"), baseClass->GetPropertyP("testProp")) << "Expected base and double derived classes to return the base property";

    ASSERT_EQ(ECObjectsStatus::Success, derivedClass->CreatePrimitiveProperty(primitiveProperty, "testProp", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(doubleDerivedClass->GetPropertyP("testProp"), baseClass->GetPropertyP("testProp")) << "Expected base and double derived classes to return the base property and Derived Property respectively";
    ASSERT_EQ(doubleDerivedClass->GetPropertyP("testProp"), (derivedClass->GetPropertyP("testProp"))) << "Expected derived and double derived classes to return the derived Propertyy";

    ASSERT_EQ(ECObjectsStatus::Success, doubleDerivedClass->CreatePrimitiveProperty(primitiveProperty, "testProp", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(ECObjectsStatus::Success, derivedClass->RemoveProperty("testProp"));

    ASSERT_EQ(doubleDerivedClass->GetPropertyP("testProp")->GetBaseProperty(), baseClass->GetPropertyP("testProp")) << "doubleDerivedClass.testProp.GetBaseProperty to have BaseClass.testProp because derivedClass Property has been removed";
    }

/*---------------------------------------------------------------------------------------
 <summary>Creates a class chain and add properties and then verifies if they
 come in the expected sequence depending on Traversal Order of ECClasses.</summary>
 <Scenario>
 <ECClass typeName = "Root" / > //Defines a property “A”
 <ECClass typeName = "B1">
 <BaseClass>Root< / BaseClass >
 <ECClass typeName = "B2" / >  // Define’s a property “A”
 <ECClass typeName = "Foo">
 <BaseClass>B1< / BaseClass >
 <BaseClass>B2< / BaseClass >
 //     3Root
 //    /                   //digits show overall override priority
 //   2B1    4B2
 //     \  /
 //      1Foo
 The traversal order will be: Foo, B1, Root, B2, and Root’s definition of property “A” will “win”,
 </scenario>
 @bsimethod                              Muhammad Hassan                         07/15
 -------------+---------------+---------------+---------------+---------------+---------*/
TEST_F(PropertyOverrideTests, VerifyClassTraversalOrderAfterPropertyOverride)
    {
    ECSchemaPtr testSchema = SetUpBaseSchema();
    ECClassP root = testSchema->GetClassP("Root");
    ECClassP b1 = testSchema->GetClassP("B1");
    ECClassP b2 = testSchema->GetClassP("B2");
    ECClassP foo = testSchema->GetClassP("Foo");

    b1->AddBaseClass(*root);

    foo->AddBaseClass(*b1);
    foo->AddBaseClass(*b2);

    PrimitiveECPropertyP primitiveProp;
    ASSERT_EQ(ECObjectsStatus::Success, root->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(foo->GetPropertyP("A"), root->GetPropertyP("A")) << "Expected Foo and Root to return the Root Property";

    ASSERT_EQ(ECObjectsStatus::Success, foo->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(foo->GetPropertyP("A"), root->GetPropertyP("A")) << "Expected Foo and Root to return the Foo Property and Root property Respectively";
    ASSERT_EQ(foo->GetPropertyP("A")->GetBaseProperty(), root->GetPropertyP("A")) << "Expected Foo and Root both to return Root  property";

    //now add same property A to class B2, as B2 has Higher Override Priority so traversal Order should not get changed.
    ASSERT_EQ(ECObjectsStatus::Success, b2->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(foo->GetPropertyP("A"), root->GetPropertyP("A")) << "Expected Foo and Root to return the Foo Property and Root property Respectively";
    ASSERT_EQ(foo->GetPropertyP("A")->GetBaseProperty(), root->GetPropertyP("A")) << "Expected Foo and Root both to return Root property";
    }

/*---------------------------------------------------------------------------------------
 <summary>Creates a class chain and add properties and then verifies if they
 come in the expected sequence depending on Traversal Order of ECClasses.</summary>
 <Scenario>
 <ECClass typeName = "Root" / > //Defines a property “A”
 <ECClass typeName = "B1">
 <BaseClass>Root< / BaseClass >
 <ECClass typeName = "B2" / >  // Define’s a property “A”
 <ECClass typeName = "Foo">
 <BaseClass>B2< / BaseClass >
 <BaseClass>B1< / BaseClass >
 //     4Root
 //    /                   //digits show overall override priority
 //   3B1    2B2
 //     \  /
 //      1Foo
 The traversal order will be: Foo, B2, B1, Root and B2’s definition of property “A” will “win”.
 </scenario>
 @bsimethod                              Muhammad Hassan                         07/15
 -------------+---------------+---------------+---------------+---------------+---------*/
TEST_F(PropertyOverrideTests, VerifyClassTraversalOrderAfterPropertyOverride1)
    {
    ECSchemaPtr testSchema = SetUpBaseSchema();
    ECClassP root = testSchema->GetClassP("Root");
    ECClassP b1 = testSchema->GetClassP("B1");
    ECClassP b2 = testSchema->GetClassP("B2");
    ECClassP foo = testSchema->GetClassP("Foo");

    b1->AddBaseClass(*root);

    foo->AddBaseClass(*b2);
    foo->AddBaseClass(*b1);

    PrimitiveECPropertyP primitiveProp;
    ASSERT_EQ(ECObjectsStatus::Success, b2->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_EQ(foo->GetPropertyP("A"), b2->GetPropertyP("A")) << "Expected Foo and B2 to return the B2 Property";

    ASSERT_EQ(ECObjectsStatus::Success, foo->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(foo->GetPropertyP("A"), b2->GetPropertyP("A")) << "Expected Foo and B2 to return the Foo Property and B2 property Respectively";
    ASSERT_EQ(foo->GetPropertyP("A")->GetBaseProperty(), b2->GetPropertyP("A")) << "Expected Foo and B2 both to return B2  property";

    //now add same property A to class B1, as B1 has Lower Override Priority so traversal Order should not get changed.
    ASSERT_EQ(ECObjectsStatus::Success, b1->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(foo->GetPropertyP("A"), b2->GetPropertyP("A")) << "Expected Foo and B2 to return the Foo Property and B2 property Respectively";
    ASSERT_EQ(foo->GetPropertyP("A")->GetBaseProperty(), b2->GetPropertyP("A")) << "Expected Foo and B2 both to return B2 property";
    }

/*---------------------------------------------------------------------------------------
 <summary>Creates a class chain and add properties and then verifies if they
 come in the expected sequence depending on Traversal Order of ECClasses.</summary>
 <Scenario>
 <ECClass typeName = "Root" / > //Defines a property “A”
 <ECClass typeName = "B1">
 <BaseClass>Root< / BaseClass >
 <ECClass typeName = "B2" / >  // Define’s a property “A”
 <ECClass typeName = "Foo">
 <BaseClass>B1< / BaseClass >
 <BaseClass>B2< / BaseClass >
 //     3Root
 //    /                   //digits show overall override priority
 //   2B1    4B2
 //     \  /
 //      1Foo
 The traversal order will be: Foo, B1, Root, B2, and Root’s definition of property “A” will “win”,
 </scenario>
 @bsimethod                              Muhammad Hassan                         07/15
 -------------+---------------+---------------+---------------+---------------+---------*/
TEST_F(PropertyOverrideTests, VerifyTraversalOrderAfterSerializingDeserializingSchemaToXmlString)
    {
    ECSchemaPtr testSchema = SetUpBaseSchema();
    ECClassP root = testSchema->GetClassP("Root");
    ECClassP b1 = testSchema->GetClassP("B1");
    ECClassP b2 = testSchema->GetClassP("B2");
    ECClassP foo = testSchema->GetClassP("Foo");

    b1->AddBaseClass(*root);

    foo->AddBaseClass(*b1);
    foo->AddBaseClass(*b2);

    PrimitiveECPropertyP primitiveProp;
    ASSERT_EQ(ECObjectsStatus::Success, root->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, foo->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));
    ASSERT_EQ(ECObjectsStatus::Success, b2->CreatePrimitiveProperty(primitiveProp, "A", PrimitiveType::PRIMITIVETYPE_String));

    ASSERT_NE(foo->GetPropertyP("A"), root->GetPropertyP("A")) << "Expected Foo and Root to return the Foo Property and Root property Respectively";
    ASSERT_EQ(foo->GetPropertyP("A")->GetBaseProperty(), root->GetPropertyP("A")) << "Expected Foo and Root both to return Root property even after adding property A to class B2";

    ECSchemaPtr testSchemaCopy1 = RoundTripSchema(testSchema);
    ECClassCP root1 = testSchemaCopy1->GetClassCP("Root");
    ASSERT_TRUE(root1 != nullptr);
    ECClassCP b11 = testSchemaCopy1->GetClassCP("B1");
    ASSERT_TRUE(b11 != nullptr);
    ECClassCP b21 = testSchemaCopy1->GetClassCP("B2");
    ASSERT_TRUE(b21 != nullptr);
    ECClassCP foo1 = testSchemaCopy1->GetClassCP("Foo");
    ASSERT_TRUE(foo1 != nullptr);

    ASSERT_NE(foo1->GetPropertyP("A"), root1->GetPropertyP("A")) << "Expected Foo and Root to return the Foo Property and Root property Respectively";
    ASSERT_EQ(foo1->GetPropertyP("A")->GetBaseProperty(), root1->GetPropertyP("A")) << "Expected Foo and Root both to return Root property even after adding property A to class B2";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    01/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyOverrideTests, TestKOQOverride)
    {
    ECSchemaPtr ecSchema;
    ECSchema::CreateSchema(ecSchema, "TestSchema", "ts", 1, 0, 0);
    ecSchema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    ecSchema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());

    ECEntityClassP a;
    ECEntityClassP b;
    ECEntityClassP c;
    KindOfQuantityP feet;
    KindOfQuantityP inch;
    KindOfQuantityP temperature;
    PrimitiveECPropertyP primProp;
    PrimitiveECPropertyP primPropOverride;
    PrimitiveECPropertyP primPropOverride2;
    PrimitiveArrayECPropertyP primArrProp;
    PrimitiveArrayECPropertyP primArrPropOverride;
    PrimitiveArrayECPropertyP primArrPropOverride2;

    ecSchema->CreateEntityClass(a, "A");
    ecSchema->CreateEntityClass(b, "B");
    ecSchema->CreateEntityClass(c, "C");
    ecSchema->CreateKindOfQuantity(feet, "Feet");
    ecSchema->CreateKindOfQuantity(inch, "Inches");
    ecSchema->CreateKindOfQuantity(temperature, "Temperature");

    // Phenomenon Length
    feet->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("MM"));
    feet->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI"));
    feet->SetRelativeError(10e-3);

    // Phenomenon Length
    inch->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("M"));
    inch->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFI"));
    inch->SetRelativeError(10e-4);
    
    // Phenomenon Temperature
    temperature->SetPersistenceUnit(*ECTestFixture::GetUnitsSchema()->GetUnitCP("CELSIUS"));
    temperature->SetDefaultPresentationFormat(*ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"));
    temperature->SetRelativeError(10e-3);

    // Test PrimitiveECProperty
    a->CreatePrimitiveProperty(primProp, "PrimProp");
    primProp->SetKindOfQuantity(feet);

    b->CreatePrimitiveProperty(primPropOverride, "PrimProp");
    primPropOverride->SetKindOfQuantity(temperature);

    EXPECT_NE(ECObjectsStatus::Success, b->AddBaseClass(*a)) << "Should have failed to add since the property A:PrimProp has a kind of quantity with a different Phenomenon then B:PrimProp";
    primPropOverride->SetKindOfQuantity(feet);
    EXPECT_EQ(ECObjectsStatus::Success, b->AddBaseClass(*a)) << "Failed even though the property Kind of Quantities are the same.";

    c->AddBaseClass(*a);
    c->CreatePrimitiveProperty(primPropOverride2, "PrimProp");
    EXPECT_EQ(ECObjectsStatus::KindOfQuantityNotCompatible, primPropOverride2->SetKindOfQuantity(temperature));
    EXPECT_EQ(ECObjectsStatus::KindOfQuantityNotCompatible, primPropOverride2->SetKindOfQuantity(inch));
    EXPECT_EQ(ECObjectsStatus::Success, primPropOverride2->SetKindOfQuantity(feet));

    a->RemoveProperty("PrimProp");
    EXPECT_EQ(0, a->GetPropertyCount()) << "All properties were not successfully removed from class 'A'";
    b->RemoveProperty("PrimProp");
    EXPECT_EQ(0, b->GetPropertyCount()) << "All properties were not successfully removed from class 'B'";
    EXPECT_EQ(ECObjectsStatus::Success, b->RemoveBaseClass(*a));
    c->RemoveProperty("PrimProp");
    EXPECT_EQ(0, c->GetPropertyCount()) << "All properties were not successfully removed from class 'C'";
    EXPECT_EQ(ECObjectsStatus::Success, c->RemoveBaseClass(*a));

    // Test PrimitiveArrayECProperty
    a->CreatePrimitiveArrayProperty(primArrProp, "PrimArrProp");
    primArrProp->SetKindOfQuantity(feet);

    b->CreatePrimitiveArrayProperty(primArrPropOverride, "PrimArrProp");
    primArrPropOverride->SetKindOfQuantity(temperature);

    EXPECT_NE(ECObjectsStatus::Success, b->AddBaseClass(*a)) << "Should have failed to add since the property A:PrimArrProp has a kind of quantity with a different Phenomenon then B:PrimArrProp";
    primArrPropOverride->SetKindOfQuantity(feet);
    EXPECT_EQ(ECObjectsStatus::Success, b->AddBaseClass(*a)) << "Failed even though the property Kind of Quantities are the same.";

    c->AddBaseClass(*a);
    c->CreatePrimitiveArrayProperty(primArrPropOverride2, "PrimArrProp");
    EXPECT_EQ(ECObjectsStatus::KindOfQuantityNotCompatible, primArrPropOverride2->SetKindOfQuantity(temperature));
    EXPECT_EQ(ECObjectsStatus::KindOfQuantityNotCompatible, primArrPropOverride2->SetKindOfQuantity(inch));
    EXPECT_EQ(ECObjectsStatus::Success, primArrPropOverride2->SetKindOfQuantity(feet));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyOverrideTests, CategoryOverride)
    {
    ECSchemaPtr ecSchema;
    ECSchema::CreateSchema(ecSchema, "TestSchema", "ts", 1, 0, 0);

    ECEntityClassP a;
    ECEntityClassP b;

    PropertyCategoryP propertyCategoryA;
    PropertyCategoryP propertyCategoryB;

    PrimitiveECPropertyP primProp;
    PrimitiveECPropertyP primPropOverride;

    ecSchema->CreateEntityClass(a, "A");
    ecSchema->CreateEntityClass(b, "B");

    ecSchema->CreatePropertyCategory(propertyCategoryA, "CategoryA");
    ecSchema->CreatePropertyCategory(propertyCategoryB, "CategoryB");

    a->CreatePrimitiveProperty(primProp, "PrimProp");
    b->CreatePrimitiveProperty(primPropOverride, "PrimProp");

    EXPECT_EQ(ECObjectsStatus::Success, primProp->SetCategory(propertyCategoryA));
    EXPECT_EQ(ECObjectsStatus::Success, primPropOverride->SetCategory(propertyCategoryB));

    EXPECT_EQ(ECObjectsStatus::Success, b->AddBaseClass(*a));

    EXPECT_EQ(propertyCategoryB, primPropOverride->GetCategory());
    EXPECT_STREQ(propertyCategoryB->GetFullName().c_str(), primPropOverride->GetCategory()->GetFullName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(PropertyOverrideTests, PriorityOverride)
    {
    {
    ECSchemaPtr ecSchema;
    ECSchema::CreateSchema(ecSchema, "TestSchema", "ts", 1, 0, 0);

    ECEntityClassP a;
    ECEntityClassP b;

    int32_t priorityA = 99;
    int32_t priorityB = -99;

    PrimitiveECPropertyP primProp;
    PrimitiveECPropertyP primPropOverride;

    ecSchema->CreateEntityClass(a, "A");
    ecSchema->CreateEntityClass(b, "B");

    a->CreatePrimitiveProperty(primProp, "PrimProp");
    b->CreatePrimitiveProperty(primPropOverride, "PrimProp");

    EXPECT_EQ(ECObjectsStatus::Success, primProp->SetPriority(priorityA));
    EXPECT_EQ(ECObjectsStatus::Success, primPropOverride->SetPriority(priorityB));

    EXPECT_EQ(ECObjectsStatus::Success, b->AddBaseClass(*a));

    EXPECT_EQ(priorityA, primProp->GetPriority());
    EXPECT_EQ(priorityB, primPropOverride->GetPriority());
    }
    {
    ECSchemaPtr ecSchema;
    ECSchema::CreateSchema(ecSchema, "TestSchema", "ts", 1, 0, 0);

    ECEntityClassP a;
    ECEntityClassP b;

    int32_t priorityA = 99;
    int32_t priorityB = 0;

    PrimitiveECPropertyP primProp;
    PrimitiveECPropertyP primPropOverride;

    ecSchema->CreateEntityClass(a, "A");
    ecSchema->CreateEntityClass(b, "B");

    a->CreatePrimitiveProperty(primProp, "PrimProp");
    b->CreatePrimitiveProperty(primPropOverride, "PrimProp");

    EXPECT_EQ(ECObjectsStatus::Success, primProp->SetPriority(priorityA));
    EXPECT_EQ(ECObjectsStatus::Success, primPropOverride->SetPriority(priorityB));

    EXPECT_EQ(ECObjectsStatus::Success, b->AddBaseClass(*a));

    EXPECT_EQ(priorityA, primProp->GetPriority());
    EXPECT_EQ(priorityB, primPropOverride->GetPriority());
    }
    }

END_BENTLEY_ECN_TEST_NAMESPACE