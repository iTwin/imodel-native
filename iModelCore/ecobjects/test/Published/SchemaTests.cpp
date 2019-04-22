/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

#include <iostream>
#include <fstream>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaTest : ECTestFixture {};
struct SchemaSearchTest : ECTestFixture {};
struct SchemaNameParsingTest : ECTestFixture {};
struct SchemaReferenceTest : ECTestFixture {};
struct SchemaCreationTest : ECTestFixture {};
struct SchemaLocateTest : ECTestFixture {};
struct SchemaKeyComparisonTest;
struct SchemaKeyTest : ECTestFixture {};
struct SchemaCacheTest : ECTestFixture {};
struct SchemaChecksumTest : ECTestFixture {};
struct SchemaImmutableTest : ECTestFixture {};
struct SchemaVersionTest : ECTestFixture {};
struct ECNameValidationTest;
struct SchemaElementsOrderTest : ECTestFixture {};

//=======================================================================================
//! SchemaTest
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Robert.Schili                       11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaTest, AddAndRemoveEnumerations)
    {
    ECSchemaPtr schema;
    ECEntityClassP domainClass;
    ECEnumerationP enumeration;
    ECEnumerationP enumeration2;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ASSERT_TRUE(schema.IsValid());

    //Create Enumeration
    auto status = schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_Integer);
    ASSERT_NE(nullptr, enumeration);
    EC_ASSERT_SUCCESS(status);

    status = schema->CreateEnumeration(enumeration2, "Enumeration", PrimitiveType::PRIMITIVETYPE_String);
    ASSERT_EQ(nullptr, enumeration2);
    ASSERT_EQ(ECObjectsStatus::NamedItemAlreadyExists, status);

    status = schema->CreateEntityClass(domainClass, "Enumeration");
    ASSERT_EQ(nullptr, domainClass);
    ASSERT_EQ(ECObjectsStatus::NamedItemAlreadyExists, status);

    enumeration2 = schema->GetEnumerationP("Enumeration");
    ASSERT_NE(nullptr, enumeration2);
    ASSERT_EQ(enumeration2, enumeration);

    int i = 0;
    for (auto p : schema->GetEnumerations())
        {
        i++;
        ASSERT_NE(nullptr, p);
        ASSERT_EQ(enumeration, p);
        }

    ASSERT_EQ(1, i);

    ASSERT_EQ(1, schema->GetEnumerationCount());

    EC_ASSERT_SUCCESS(schema->DeleteEnumeration(*enumeration));

    enumeration2 = nullptr;
    enumeration2 = schema->GetEnumerationP("Enumeration");
    ASSERT_EQ(nullptr, enumeration2);

    ASSERT_EQ(0, schema->GetEnumerationCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaTest, CreateDynamicSchema)
    {
    auto dynamicClass = CoreCustomAttributeHelper::GetCustomAttributeClass("DynamicSchema");
    IECInstancePtr dynamicSchemaCA = dynamicClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECSchemaCachePtr cache = ECSchemaCache::Create();
    
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 2, 0, 1);
    schema->AddReferencedSchema(*CoreCustomAttributeHelper::GetSchema());
    EC_ASSERT_SUCCESS(schema->SetCustomAttribute(*dynamicSchemaCA));
    

    ASSERT_EQ(ECObjectsStatus::Success, cache->AddSchema(*schema));
    ECSchemaP retrievedSchema = cache->GetSchema(SchemaKey("TestSchema", 2, 1), SchemaMatchType::Exact);
    ASSERT_NE(nullptr, retrievedSchema);

    ASSERT_TRUE(retrievedSchema->IsDynamicSchema());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaTest, TryRenameECClass)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    ECEntityClassP entityClass1;
    ECEntityClassP entityClass2;
    schema->CreateEntityClass(entityClass1, "ClassA");
    schema->CreateEntityClass(entityClass2, "ClassB");

    // rename classes
    EC_ASSERT_SUCCESS(schema->RenameClass(*schema->GetClassP("ClassA"), "ClassA1"));
    EC_ASSERT_SUCCESS(schema->RenameClass(*schema->GetClassP("ClassB"), "ClassB1"));

    // try to get classes with old names
    ASSERT_EQ(nullptr, schema->GetClassCP("ClassA"));
    ASSERT_EQ(nullptr, schema->GetClassCP("ClassB"));

    // Get classes with new names
    ECClassP classA1 = schema->GetClassP("ClassA1");
    ECClassP classB1 = schema->GetClassP("ClassB1");
    ASSERT_NE(nullptr, classA1);
    ASSERT_NE(nullptr, classB1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaTest, RemoveReferenceSchema)
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 5, 0, 5);

    ECRelationshipClassP relClass;
    ECEntityClassP targetClass;
    ECEntityClassP sourceClass;

    schema->CreateRelationshipClass(relClass, "RElationshipClass");
    schema->CreateEntityClass(targetClass, "Target");
    refSchema->CreateEntityClass(sourceClass, "Source");

    EXPECT_EQ(ECObjectsStatus::Success, relClass->GetTarget().AddClass(*targetClass));
    EXPECT_EQ(ECObjectsStatus::SchemaNotFound, relClass->GetSource().AddClass(*sourceClass));

    schema->AddReferencedSchema(*refSchema);
    ASSERT_FALSE(refSchema->IsStandardSchema());
    ASSERT_FALSE(refSchema->IsSupplementalSchema());
    ASSERT_FALSE(refSchema->IsSystemSchema());
    ASSERT_TRUE(ECSchema::IsSchemaReferenced(*schema, *refSchema));
    EXPECT_EQ(ECObjectsStatus::Success, relClass->GetSource().AddClass(*sourceClass));

    // try to remove refrence schema, shouldn't be successful as referenced schema's class is used a relationship constraint.
    ASSERT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(SchemaKey("RefSchema", 5, 5)));

    // remove source constraint class from relationship
    EXPECT_EQ(ECObjectsStatus::Success, relClass->GetSource().RemoveClass(*sourceClass));

    //removing reference schema should be successful now
    ASSERT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(SchemaKey("RefSchema", 5, 5)));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(SchemaTest, CannotAddMultipleSchemaItemsWithSameName)
    {
    ECSchemaPtr schema;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    ECEntityClassP testClass;
    EC_EXPECT_SUCCESS(schema->CreateEntityClass(testClass, "IdenticalName"));

    ECEntityClassP testClass2;
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, schema->CreateEntityClass(testClass2, "IdenticalName"));

    ECCustomAttributeClassP caClass;
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, schema->CreateCustomAttributeClass(caClass, "IdenticalName"));

    ECStructClassP structClass;
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, schema->CreateStructClass(structClass, "IdenticalName"));

    ECRelationshipClassP relClass;
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, schema->CreateRelationshipClass(relClass, "IdenticalName"));

    KindOfQuantityP koq;
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, schema->CreateKindOfQuantity(koq, "IdenticalName"));

    ECEnumerationP enumeration;
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, schema->CreateEnumeration(enumeration, "IdenticalName", PRIMITIVETYPE_String));

    PropertyCategoryP propCategory;
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, schema->CreatePropertyCategory(propCategory, "IdenticalName"));

    PhenomenonP phen;
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, schema->CreatePhenomenon(phen, "IdenticalName", ""));

    UnitSystemP unitSystem;
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, schema->CreateUnitSystem(unitSystem, "IdenticalName"));

    PhenomenonP realPhen;
    EC_EXPECT_SUCCESS(schema->CreatePhenomenon(realPhen, "RealPhenomenon", ""));
    UnitSystemP realUnitSystem;
    EC_EXPECT_SUCCESS(schema->CreateUnitSystem(realUnitSystem, "RealUnitSystem"));
    ECUnitP realUnit;
    EC_EXPECT_SUCCESS(schema->CreateUnit(realUnit, "RealUnit", "", *realPhen, *realUnitSystem));

    ECUnitP unit;
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, schema->CreateConstant(unit, "IdenticalName", "", *realPhen, 4));

    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, schema->CreateUnit(unit, "IdenticalName", "", *realPhen, *realUnitSystem));

    ECUnitP invertedUnit;
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, schema->CreateInvertedUnit(invertedUnit, *realUnit, "IdenticalName", *realUnitSystem));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaTest, DeleteKOQ)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    Utf8CP schemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <KindOfQuantity typeName='MyKindOfQuantity' description='My KindOfQuantity'"
        "                    displayLabel='My KindOfQuantity' persistenceUnit='CM' relativeError='10e-3'"
        "                    presentationUnits='FT;IN;YRD' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECProperty propertyName='Length' typeName='double'  kindOfQuantity='MyKindOfQuantity' />" // kindOfQuantity='s1:MyKindOfQuantity'
        "        <ECArrayProperty propertyName='AlternativeLengths' typeName='double' minOccurs='0' maxOccurs='unbounded' kindOfQuantity = 'MyKindOfQuantity'/>" // kindOfQuantity='s1:MyKindOfQuantity'
        "    </ECEntityClass>"
        "</ECSchema>";

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    std::vector<KindOfQuantityP> todelete;

    KindOfQuantityContainerCR koqContainer = schema->GetKindOfQuantities();
    for (auto koq : koqContainer)
        todelete.push_back(koq);

    for (auto koq : todelete)
        ASSERT_EQ(ECObjectsStatus::Success, schema->DeleteKindOfQuantity(*koq));

    ASSERT_EQ(0, schema->GetKindOfQuantityCount());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
TEST_F(SchemaTest, DeleteUnitSystem)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    UnitSystemP system;
    EC_ASSERT_SUCCESS(schema->CreateUnitSystem(system, "TestUnitSystem"));
    ASSERT_NE(nullptr, system);
    EXPECT_EQ(1, schema->GetUnitSystemCount());
    EXPECT_EQ(system, schema->GetUnitSystemCP("TestUnitSystem"));
    EC_EXPECT_SUCCESS(schema->DeleteUnitSystem(*system));
    EXPECT_EQ(nullptr, schema->GetUnitSystemCP("TestUnitSystem"));
    EXPECT_EQ(0, schema->GetUnitSystemCount());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
TEST_F(SchemaTest, DeletePhenomenon)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    PhenomenonP phenom;
    EC_ASSERT_SUCCESS(schema->CreatePhenomenon(phenom, "TestPhenomenon", "LENGTH"));
    ASSERT_NE(nullptr, phenom);
    EXPECT_EQ(1, schema->GetPhenomenonCount());
    EXPECT_EQ(phenom, schema->GetPhenomenonCP("TestPhenomenon"));
    EC_EXPECT_SUCCESS(schema->DeletePhenomenon(*phenom));
    EXPECT_EQ(nullptr, schema->GetPhenomenonCP("TestPhenomenon"));
    EXPECT_EQ(0, schema->GetPhenomenonCount());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(SchemaTest, DeleteUnits)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);

    PhenomenonP phenom;
    UnitSystemP unitSystem;
    EC_ASSERT_SUCCESS(schema->CreatePhenomenon(phenom, "TestPhenomenon", "LENGTH"));
    EC_ASSERT_SUCCESS(schema->CreateUnitSystem(unitSystem, "TestUnitSystem"));

    { // ECUnit
    ECUnitP unit;
    EC_ASSERT_SUCCESS(schema->CreateUnit(unit, "TestUnit", "", *phenom, *unitSystem));
    ASSERT_NE(nullptr, unit);
    EXPECT_EQ(1, schema->GetUnitCount());
    EXPECT_EQ(unit, schema->GetUnitCP("TestUnit"));
    EC_EXPECT_SUCCESS(schema->DeleteUnit(*unit));
    EXPECT_EQ(nullptr, schema->GetUnitCP("TestUnit"));
    EXPECT_EQ(0, schema->GetFormatCount());
    }
    { // Inverted Unit
    ECUnitP unit;
    EC_ASSERT_SUCCESS(schema->CreateUnit(unit, "TestUnit", "", *phenom, *unitSystem));

    ECUnitP invertedUnit;
    EC_ASSERT_SUCCESS(schema->CreateInvertedUnit(invertedUnit, *unit, "TestInvertedUnit", *unitSystem));
    ASSERT_NE(nullptr, invertedUnit);
    EXPECT_EQ(2, schema->GetUnitCount());
    EXPECT_EQ(invertedUnit, schema->GetInvertedUnitCP("TestInvertedUnit"));
    EC_EXPECT_SUCCESS(schema->DeleteUnit(*invertedUnit));
    EXPECT_EQ(nullptr, schema->GetInvertedUnitCP("TestInvertedUnit"));
    EXPECT_EQ(1, schema->GetUnitCount());

    // Cleanup
    schema->DeleteUnit(*unit);
    }
    { // Constant
    ECUnitP constant;
    EC_ASSERT_SUCCESS(schema->CreateConstant(constant, "TestConstant", "", *phenom, 5));
    ASSERT_NE(nullptr, constant);
    EXPECT_EQ(1, schema->GetUnitCount());
    EXPECT_EQ(constant, schema->GetConstantCP("TestConstant"));
    EC_EXPECT_SUCCESS(schema->DeleteUnit(*constant));
    EXPECT_EQ(nullptr, schema->GetConstantCP("TestConstant"));
    EXPECT_EQ(0, schema->GetUnitCount());
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(SchemaTest, DeleteFormat)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    ECFormatP format;
    EC_ASSERT_SUCCESS(schema->CreateFormat(format, "TestFormat"));
    ASSERT_NE(nullptr, format);
    EXPECT_EQ(1, schema->GetFormatCount());
    EXPECT_EQ(format, schema->GetFormatCP("TestFormat"));
    EC_EXPECT_SUCCESS(schema->DeleteFormat(*format));
    EXPECT_EQ(nullptr, schema->GetFormatCP("TestFormat"));
    EXPECT_EQ(0, schema->GetFormatCount());
    }

//---------------------------------------------------------------------------------**//**
// @bsimethod                                   Raimondas.Rimkus                   02/13
// +---------------+---------------+---------------+---------------+---------------+-----
TEST_F(SchemaTest, TestCircularReference)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath (L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext ();
    schemaContext->AddSchemaLocater (*schemaLocater);
    SchemaKey key("CircleSchema", 01, 00);
    testSchema = schemaContext->LocateSchema(key, SchemaMatchType::Latest);
    EXPECT_FALSE(testSchema.IsValid ());
    }

//---------------------------------------------------------------------------------**//**
// @bsimethod                                   Raimondas.Rimkus                   02/13
// +---------------+---------------+---------------+---------------+---------------+-----
TEST_F(SchemaTest, TestsLatestCompatible)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater(*schemaLocater);
    SchemaKey key("Widgets", 01, 00);
    testSchema = schemaContext->LocateSchema(key, SchemaMatchType::LatestWriteCompatible);
    EXPECT_TRUE(testSchema.IsValid());
    EXPECT_EQ(9, testSchema->GetVersionRead());
    EXPECT_EQ(6, testSchema->GetVersionMinor());
    }

//---------------------------------------------------------------------------------**//**
// @bsimethod                                   Raimondas.Rimkus                   02/13
// +---------------+---------------+---------------+---------------+---------------+-----
TEST_F(SchemaTest, TestsLatest)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath (L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext ();
    schemaContext->AddSchemaLocater (*schemaLocater);
    SchemaKey key ("Widgets", 9, 7);
    testSchema = schemaContext->LocateSchema (key, SchemaMatchType::Latest);
    EXPECT_TRUE (testSchema.IsValid ());
    EXPECT_TRUE (testSchema->GetVersionRead () == 9);
    EXPECT_TRUE (testSchema->GetVersionMinor () == 6);
    }

//---------------------------------------------------------------------------------**//**
// @bsimethod                                   Raimondas.Rimkus                   02/13
// +---------------+---------------+---------------+---------------+---------------+-----
TEST_F(SchemaTest, GetBaseClassPropertyWhenSchemaHaveDuplicatePrefixes)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath (L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext ();
    schemaContext->AddSchemaLocater (*schemaLocater);
    SchemaKey key ("DuplicatePrefixes", 01, 00);
    testSchema = schemaContext->LocateSchema (key, SchemaMatchType::Latest);
    EXPECT_TRUE (testSchema.IsValid ());
    ECClassCP circleClass = testSchema->GetClassCP ("Circle");
    EXPECT_NE(nullptr, circleClass) << "Cannot Load Ellipse Class";
    }

//---------------------------------------------------------------------------------**//**
// @bsimethod                                   Raimondas.Rimkus                   02/13
// +---------------+---------------+---------------+---------------+---------------+-----
TEST_F(SchemaTest, GetBaseClassProperty)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath (L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater (searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext ();
    schemaContext->AddSchemaLocater (*schemaLocater);
    SchemaKey key ("testschema", 01, 00);
    testSchema = schemaContext->LocateSchema (key, SchemaMatchType::Latest);
    EXPECT_TRUE (testSchema.IsValid ());
    ECClassCP wheelsChildClass = testSchema->GetClassCP ("WheelsChild");
    EXPECT_NE(nullptr, wheelsChildClass) << "Cannot Load WheelsChild Class";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaTest, HierarchyInCorrectOrder)
    {
    ECSchemaPtr schemaA;
    ECSchemaPtr schemaB;
    ECSchemaPtr schemaC;
    ECSchemaPtr schemaD;
    ECSchemaPtr schemaE;
    ECSchemaPtr schemaF;
    ECSchema::CreateSchema(schemaA, "A", "a", 1, 0, 0);
    ECSchema::CreateSchema(schemaB, "B", "b", 1, 0, 0);
    ECSchema::CreateSchema(schemaC, "C", "c", 1, 0, 0);
    ECSchema::CreateSchema(schemaD, "D", "d", 1, 0, 0);
    ECSchema::CreateSchema(schemaE, "E", "e", 1, 0, 0);
    ECSchema::CreateSchema(schemaF, "F", "f", 1, 0, 0);

    schemaA->AddReferencedSchema(*schemaB);
    schemaA->AddReferencedSchema(*schemaC);
    schemaA->AddReferencedSchema(*schemaE);
    schemaA->AddReferencedSchema(*schemaF);

    schemaB->AddReferencedSchema(*schemaC);

    schemaC->AddReferencedSchema(*schemaD);

    schemaF->AddReferencedSchema(*schemaD);

    bvector<ECSchemaP> schemas;
    schemaA->FindAllSchemasInGraph(schemas, true);

    // Expected order: E, D, C, B, A
    EXPECT_TRUE(schemas[0]->GetAlias().EqualsIAscii("d"));
    EXPECT_TRUE(schemas[1]->GetAlias().EqualsIAscii("f"));
    EXPECT_TRUE(schemas[2]->GetAlias().EqualsIAscii("e"));
    EXPECT_TRUE(schemas[3]->GetAlias().EqualsIAscii("c"));
    EXPECT_TRUE(schemas[4]->GetAlias().EqualsIAscii("b"));
    EXPECT_TRUE(schemas[5]->GetAlias().EqualsIAscii("a"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Colin.Kerr                       06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaTest, ElementIdsAreNotSetAutomatically)
    {
    ECSchemaPtr schema;
    ECEntityClassP entityClass;
    ECStructClassP structClass;
    ECCustomAttributeClassP customAttributeClass;
    ECRelationshipClassP relationshipClass;
    ECEnumerationP enumeration;
    KindOfQuantityP koq;
    PropertyCategoryP propCategory;
    uint64_t id(42);

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 5, 5);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_FALSE(schema->HasId()) << "Expected ECSchemaId to be unset when created via ECSchema::CreateSchema";
    schema->SetId(ECSchemaId(id));
    EXPECT_TRUE(schema->HasId()) << "Expected ECSchemaId to be set after calling ECSchema::SetId";
    EXPECT_EQ(id, schema->GetId().GetValue()) << "Expected ECSchemaId to be set to 42";

    //Create Domain Class
    schema->CreateEntityClass(entityClass, "EntityClass");
    ASSERT_NE(nullptr, entityClass);
    EXPECT_FALSE(entityClass->HasId()) << "Expected ECClassId to be unset when creating via ECSchema::CreateEntityClass";
    entityClass->SetId(ECClassId(id));
    EXPECT_TRUE(entityClass->HasId()) << "Expected ECClassId to be set after calling ECClass::SetId";
    EXPECT_EQ(id, entityClass->GetId().GetValue()) << "Expected ECClassId to be set to 42";

    //Create Struct
    schema->CreateStructClass(structClass, "StructClass");
    ASSERT_NE(nullptr, structClass);
    EXPECT_FALSE(structClass->HasId()) << "Expected ECClassId to be unset when creating via ECSchema::CreateStructClass";
    structClass->SetId(ECClassId(id));
    EXPECT_TRUE(structClass->HasId()) << "Expected ECClassId to be set on struct after calling ECClass::SetId";
    EXPECT_EQ(id, structClass->GetId().GetValue()) << "Expected ECClassId to be set to 42 on struct";

    //Create customAttributeClass
    schema->CreateCustomAttributeClass(customAttributeClass, "CustomAttributeClass");
    ASSERT_NE(nullptr, customAttributeClass);
    EXPECT_FALSE(customAttributeClass->HasId()) << "Expected ECClassId to be unset when creating via ECSchema::CreateCustomAttributeClass";
    customAttributeClass->SetId(ECClassId(id));
    EXPECT_TRUE(customAttributeClass->HasId()) << "Expected ECClassId to be set on custom attribute after calling ECClass::SetId";
    EXPECT_EQ(id, customAttributeClass->GetId().GetValue()) << "Expected ECClassId to be set to 42 on custom attribute";

    //Create RelationshipClass
    schema->CreateRelationshipClass(relationshipClass, "RelationshipClass");
    ASSERT_NE(nullptr, relationshipClass);
    relationshipClass->GetSource().AddClass(*entityClass);
    relationshipClass->GetTarget().AddClass(*entityClass);
    EXPECT_FALSE(relationshipClass->HasId()) << "Expected ECClassId to be unset when creating via ECSchema::CreateRelationshipClass";
    relationshipClass->SetId(ECClassId(id));
    EXPECT_TRUE(relationshipClass->HasId()) << "Expected ECClassId to be set on relationship after calling ECClass::SetId";
    EXPECT_EQ(id, relationshipClass->GetId().GetValue()) << "Expected ECClassId to be set to 42 on relationship";

    //Create Enumeration
    schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_Integer);
    ASSERT_NE(nullptr, enumeration);
    EXPECT_FALSE(enumeration->HasId()) << "Expected ECEnumerationId to be unset when creating via ECSchema::CreateEnumeration";
    enumeration->SetId(ECEnumerationId(id));
    EXPECT_TRUE(enumeration->HasId()) << "Expected ECEnumerationId to be set after calling ECEnumeration::SetId";
    EXPECT_EQ(id, enumeration->GetId().GetValue()) << "Expected ECEnumerationId to be set to 42";

    //Create KindOfQuantity
    schema->CreateKindOfQuantity(koq, "KindOfQuantity");
    ASSERT_NE(nullptr, koq);
    EXPECT_FALSE(koq->HasId()) << "Expected KindOfQuantityId to be unset when creating via ECSchema::CreateKindOfQuantity";
    koq->SetId(KindOfQuantityId(id));
    EXPECT_TRUE(koq->HasId()) << "Expected KindOfQuantityId to be set after calling KindOfQuantity::SetId";
    EXPECT_EQ(id, koq->GetId().GetValue()) << "Expected KindOfQuantityId to be set to 42";

    //Create PropertyCategory
    schema->CreatePropertyCategory(propCategory, "PropertyCategory");
    ASSERT_NE(nullptr, propCategory);
    EXPECT_FALSE(propCategory->HasId()) << "Expected PropertyCategoryId to be unset when creating via ECSchema::CreatePropertyCategory";
    propCategory->SetId(PropertyCategoryId(id));
    EXPECT_TRUE(propCategory->HasId()) << "Expected PropertyCategoryId to be set after calling PropertyCategory::SetId";
    EXPECT_EQ(id, propCategory->GetId().GetValue()) << "Expected PropertyCategoryId to be set to 42";

    //Add Property of primitive type to entity class
    PrimitiveECPropertyP primitiveProperty;
    entityClass->CreatePrimitiveProperty(primitiveProperty, "PrimitiveProperty", PrimitiveType::PRIMITIVETYPE_Boolean);
    ASSERT_NE(nullptr, primitiveProperty);
    EXPECT_FALSE(primitiveProperty->HasId()) << "Expected ECPropertyId to be unset when creating  ECClass::CreatePrimitiveProperty";
    primitiveProperty->SetId(ECPropertyId(id));
    EXPECT_TRUE(primitiveProperty->HasId()) << "Expected ECPropertyId to be set on primitive after calling ECProperty::SetId";
    EXPECT_EQ(id, primitiveProperty->GetId().GetValue()) << "Expected ECPropertyId to be set to 42 on primitive";

    //Add Property of Array type to structClass
    PrimitiveArrayECPropertyP arrProp;
    structClass->CreatePrimitiveArrayProperty (arrProp, "ArrayProperty");
    ASSERT_NE(nullptr, arrProp);
    EXPECT_FALSE(arrProp->HasId()) << "Expected ECPropertyId to be unset when creating  ECClass::CreateArrayProperty";
    arrProp->SetId(ECPropertyId(id));
    EXPECT_TRUE(arrProp->HasId()) << "Expected ECPropertyId to be set on array after calling ECProperty::SetId";
    EXPECT_EQ(id, arrProp->GetId().GetValue()) << "Expected ECPropertyId to be set to 42 on array";

    //Add Property Of Struct type to custom attribute
    StructECPropertyP propOfCustomAttribute;
    customAttributeClass->CreateStructProperty (propOfCustomAttribute, "PropertyOfCustomAttribute", *structClass);
    ASSERT_NE(nullptr, propOfCustomAttribute);
    EXPECT_FALSE(propOfCustomAttribute->HasId()) << "Expected ECPropertyId to be unset when creating  ECClass::CreateStructProperty";
    propOfCustomAttribute->SetId(ECPropertyId(id));
    EXPECT_TRUE(propOfCustomAttribute->HasId()) << "Expected ECPropertyId to be set on struct after calling ECProperty::SetId";
    EXPECT_EQ(id, propOfCustomAttribute->GetId().GetValue()) << "Expected ECPropertyId to be set to 42 on struct";

    //Add Navgation property to entity class
    NavigationECPropertyP navigationProperty;
    entityClass->CreateNavigationProperty(navigationProperty, "NavigationProperty", *relationshipClass, ECRelatedInstanceDirection::Forward);
    ASSERT_NE(nullptr, navigationProperty);
    EXPECT_FALSE(navigationProperty->HasId()) << "Expected ECPropertyId to be unset when creating  ECClass::CreateNavigationProperty";
    navigationProperty->SetId(ECPropertyId(id));
    EXPECT_TRUE(navigationProperty->HasId()) << "Expected ECPropertyId to be set on navigation property after calling ECProperty::SetId";
    EXPECT_EQ(id, navigationProperty->GetId().GetValue()) << "Expected ECPropertyId to be set to 42 on navigation property";
    }

//=======================================================================================
//! SchemaSearchTest
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaSearchTest, FindSchemaByName)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    WString seedPath(ECTestFixture::GetTestDataPath(L"").c_str());
    schemaContext->AddSchemaPath(seedPath.c_str());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"SchemaThatReferences.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    EXPECT_TRUE(nullptr != schema->FindSchema(SchemaKey("SchemaThatReferences", 1, 0), SchemaMatchType::Exact));
    EXPECT_TRUE(nullptr == schema->FindSchema(SchemaKey("SchemaThatReferencez", 1, 0), SchemaMatchType::Exact));
    EXPECT_TRUE(nullptr == schema->FindSchema(SchemaKey("SchemaThatReferences", 2, 0), SchemaMatchType::Exact));
    EXPECT_TRUE(nullptr == schema->FindSchema(SchemaKey("SchemaThatReferences", 1, 1), SchemaMatchType::Exact));

    EXPECT_TRUE(nullptr != schema->FindSchema(SchemaKey("BaseSchema", 1, 0), SchemaMatchType::Exact));
    EXPECT_TRUE(nullptr != schema->FindSchemaP(SchemaKey("SchemaThatReferences", 1, 0), SchemaMatchType::Exact));
    EXPECT_TRUE(nullptr == schema->FindSchemaP(SchemaKey("a", 123, 456), SchemaMatchType::Exact));
    }

void VerifySingleSchemaExists(ECSchemaReadContextR schemaContext, SchemaKey schemaKey)
    {
    bool found = false;
    for (ECSchemaCP schema : schemaContext.GetCache().GetSchemas())
        {
        if (schema->GetSchemaKey().Matches(schemaKey, SchemaMatchType::Exact))
            {
            if (!found)
                found = true;
            else
                {
                EXPECT_TRUE(false);
                break;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                01/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaSearchTest, FindSchemaByFileName)
    {
    // Test success when the schema can be found
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaKey testKey("Widgets", 9, 6);

    ECSchemaPtr schema = ECSchema::LocateSchema(ECTestFixture::GetTestDataPath(L"Widgets.09.06.ecschema.xml").c_str(), *schemaContext);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->GetSchemaKey().Matches(testKey, SchemaMatchType::Exact));

    // Check that if the schema is a duplicate it will be returned
    ECSchemaPtr duplicateSchema = ECSchema::LocateSchema(ECTestFixture::GetTestDataPath(L"Widgets.09.06.ecschema.xml").c_str(), *schemaContext);
    ASSERT_TRUE(duplicateSchema.IsValid());
    EXPECT_TRUE(duplicateSchema->GetSchemaKey().Matches(testKey, SchemaMatchType::Exact));
    
    EXPECT_TRUE(schema->GetSchemaKey().Matches(duplicateSchema->GetSchemaKey(), SchemaMatchType::Exact));

    // verify that there is only one in the schema cache
    VerifySingleSchemaExists(*schemaContext, testKey);
    }

    {
    SchemaKey testKey("Widgets", 9, 6);

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"Widgets.09.06.ecschema.xml").c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->GetSchemaKey().Matches(testKey, SchemaMatchType::Exact));

    status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"Widgets.09.06.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::DuplicateSchema, status);

    ECSchemaPtr dupSchema = ECSchema::LocateSchema(ECTestFixture::GetTestDataPath(L"Widgets.09.06.ecschema.xml").c_str(), *schemaContext);
    ASSERT_TRUE(dupSchema.IsValid());
    EXPECT_TRUE(dupSchema->GetSchemaKey().Matches(testKey, SchemaMatchType::Exact));
    
    VerifySingleSchemaExists(*schemaContext, testKey);
    }

    // Test failure when a non xml file is passed
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    BeTest::SetFailOnAssert(false);
    ECSchemaPtr schema = ECSchema::LocateSchema(ECTestFixture::GetTestDataPath(L"Widgets.05.10.ecschema").c_str(), *schemaContext);
    BeTest::SetFailOnAssert(true);
    ASSERT_FALSE(schema.IsValid());
    }

    // Test failure when schema xml file does not exist
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    BeTest::SetFailOnAssert(false);
    ECSchemaPtr schema = ECSchema::LocateSchema(ECTestFixture::GetTestDataPath(L"Widgets.05.10.ecschema.xml").c_str(), *schemaContext);
    BeTest::SetFailOnAssert(true);
    ASSERT_FALSE(schema.IsValid());
    }

    }

//=======================================================================================
//! SchemaNameParsingTest
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static void ValidateSchemaNameParsing(Utf8CP fullName, bool expectFailure, Utf8CP expectName, uint32_t expectRead, uint32_t expectWrite, uint32_t expectMinor)
    {
    Utf8String    shortName;
    uint32_t   versionRead;
    uint32_t   versionWrite;
    uint32_t   versionMinor;

    ECObjectsStatus status = ECSchema::ParseSchemaFullName(shortName, versionRead, versionWrite, versionMinor, fullName);

    if (expectFailure)
        {
        EXPECT_TRUE(ECObjectsStatus::Success != status);
        return;
        }

    EXPECT_TRUE(ECObjectsStatus::Success == status);

    EXPECT_STREQ(shortName.c_str(), expectName);
    EXPECT_EQ(versionRead, expectRead);
    EXPECT_EQ(versionWrite, expectWrite);
    EXPECT_EQ(versionMinor, expectMinor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Josh.Schifter                       10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaNameParsingTest, ParseFullSchemaName)
    {
    ValidateSchemaNameParsing("TestName.6.8", false, "TestName", 6, 0, 8);
    ValidateSchemaNameParsing("TestName.16.18", false, "TestName", 16, 0, 18);
    ValidateSchemaNameParsing("TestName.126.128", false, "TestName", 126, 0, 128);
    ValidateSchemaNameParsing("TestName.1267.128", false, "TestName", 1267, 0, 128);
    ValidateSchemaNameParsing("TestName.1267", true, nullptr, 0, 0, 0);
    ValidateSchemaNameParsing("TestName", true, nullptr, 0, 0, 0);
    ValidateSchemaNameParsing("", true, nullptr, 0, 0, 0);
    ValidateSchemaNameParsing("12.18", true, nullptr, 0, 0, 0);
    }

//=======================================================================================
//! SchemaReferenceTest
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod                        Carole.MacDonald                        01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, AddAndRemoveReferencedSchemas)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 5, 5);

    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 5, 5, 5);

    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*refSchema));
    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, schema->AddReferencedSchema(*refSchema));

    ECSchemaReferenceListCR refList = schema->GetReferencedSchemas();

    ECSchemaReferenceList::const_iterator schemaIterator = refList.find(refSchema->GetSchemaKey());

    EXPECT_FALSE(schemaIterator == refList.end());
    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema));

    schemaIterator = refList.find(refSchema->GetSchemaKey());

    EXPECT_TRUE(schemaIterator == refList.end());
    EXPECT_EQ(ECObjectsStatus::SchemaNotFound, schema->RemoveReferencedSchema(*refSchema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Robert.Schili                   11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaReferenceTest, CanRemoveAllUnusedSchemaReferences)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 5, 5);

    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, "RefSchema", "rs", 5, 5, 5);

    ECSchemaPtr unusedRefSchema;
    ECSchema::CreateSchema(unusedRefSchema, "UnusedRefSchema", "urs", 42, 42, 42);

    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*refSchema));
    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*unusedRefSchema));

    ECEntityClassP baseClass;
    refSchema->CreateEntityClass(baseClass, "Banana");
    ECEntityClassP derivedClass;
    schema->CreateEntityClass(derivedClass, "Apple");
    derivedClass->AddBaseClass(*baseClass);

    ECSchemaReferenceListCR refList = schema->GetReferencedSchemas();

    ECSchemaReferenceList::const_iterator schemaIterator = refList.find(refSchema->GetSchemaKey());
    EXPECT_FALSE(schemaIterator == refList.end()) << "Could not find RefSchema in reference list";
    schemaIterator = refList.find(unusedRefSchema->GetSchemaKey());
    EXPECT_FALSE(schemaIterator == refList.end()) << "Could not find UnusedRefSchema in reference list";

    EXPECT_EQ(1, schema->RemoveUnusedSchemaReferences()) << "Expected RemoveUnusedSchemaReferences to remove one schema";

    schemaIterator = refList.find(refSchema->GetSchemaKey());
    EXPECT_FALSE(schemaIterator == refList.end()) << "Could not find RefSchema in reference list after removing unused schemas";
    schemaIterator = refList.find(unusedRefSchema->GetSchemaKey());
    EXPECT_TRUE(schemaIterator == refList.end()) << "Found UnusedRefSchema in reference list after removing unused schemas";

    EXPECT_EQ(ECObjectsStatus::SchemaNotFound, schema->RemoveReferencedSchema(*unusedRefSchema)) << "Expected UnusedRefSchema to not be found after removing unused schema references";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaReferenceTest, WillNotRemoveUsedReference_MultipleCopiesOfReferencedSchema)
    {
    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, "RefSchema", "RS", 1, 2, 3);
    ECCustomAttributeClassP caClass;
    refSchema->CreateCustomAttributeClass(caClass, "CA");

    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "TS", 1, 2, 3);
    ECEntityClassP ecClass;
    schema->CreateEntityClass(ecClass, "Entity");

    ECSchemaPtr refSchemaCopy;
    refSchema->CopySchema(refSchemaCopy);
    schema->AddReferencedSchema(*refSchemaCopy);

    IECInstancePtr caInstance = caClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ecClass->SetCustomAttribute(*caInstance);

    ASSERT_TRUE(ECSchema::IsSchemaReferenced(*schema, *refSchema)) << "Failed to find the RefSchema when a copy was added as a reference";
    ASSERT_TRUE(ECSchema::IsSchemaReferenced(*schema, *refSchemaCopy)) << "Failed to find the RefSchema copy that was added as a reference";

    ASSERT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema)) << "Should have found CA which uses refSchema";
    ASSERT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchemaCopy)) << "Should have found CA which uses refSchema even though we passed in the copy";

    ASSERT_EQ(0, schema->RemoveUnusedSchemaReferences()) << "Should not have removed any referenced schemas";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Colin.Kerr                           04/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaReferenceTest, CanRemoveUnusedRefSchemaWhenSchemaUsesAnotherRefForAStructType)
    {
    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, "RefSchema", "RS", 1, 2, 3);
    ECStructClassP structClass;
    refSchema->CreateStructClass(structClass, "Struct");

    ECSchemaPtr unusedRefSchema;
    ECSchema::CreateSchema(unusedRefSchema, "UnusedRefSchema", "URS", 1, 2, 3);

    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "TS", 1, 2, 3);
    schema->AddReferencedSchema(*refSchema);
    schema->AddReferencedSchema(*unusedRefSchema);

    ECEntityClassP ecClass;
    schema->CreateEntityClass(ecClass, "Class");
    StructECPropertyP structProp;
    ecClass->CreateStructProperty(structProp, "SProp", *structClass);

    ASSERT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema)) << "Should not have been able to remove RefSchema because it is used";
    ASSERT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*unusedRefSchema)) << "Should have been able to remove UnusedRefSchema because it is not used";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Dylan.Rush                           05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, InvalidReference)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling(true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    WString seedPath(ECTestFixture::GetTestDataPath(L"").c_str());
    schemaContext->AddSchemaPath(seedPath.c_str());

    //The "InvalidReference" schema contains a reference to BaseSchema.01.01.  This schema 
    //does not exist.  1.0 exists, but the minor version numbers are incompatible.
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"InvalidReference.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_TRUE(schema.IsNull());
    EXPECT_EQ(SchemaReadStatus::ReferencedSchemaNotFound, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, ExpectErrorWhenTryRemoveSchemaInUse)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 5, 0, 5);

    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*refSchema));
    ECEntityClassP class1;
    ECEntityClassP baseClass;
    ECStructClassP structClass;
    ECRelationshipClassP relClass;
    ECRelationshipClassP navRelClass;

    refSchema->CreateEntityClass(baseClass, "BaseClass");
    refSchema->CreateStructClass(structClass, "StructClass");
    refSchema->CreateRelationshipClass(navRelClass, "NavRelClass");
    schema->CreateEntityClass(class1, "TestClass");

    schema->CreateRelationshipClass(relClass, "RelClass");

    // Test base class in referenced schema
    class1->AddBaseClass(*baseClass);
    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema));

    class1->RemoveBaseClass(*baseClass);
    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema));

    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*refSchema));

    // Test Source abstract constraint in referenced schema
    relClass->GetSource().SetAbstractConstraint(*baseClass);
    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema));
    
    relClass->GetSource().SetAbstractConstraint(*class1);
    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema));
    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*refSchema));
    
    // Test Target abstract constraint in referenced schema
    relClass->GetTarget().SetAbstractConstraint(*baseClass);
    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema));
    
    relClass->GetTarget().SetAbstractConstraint(*class1);
    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema));
    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*refSchema));

    StructECPropertyP structProp;
    StructArrayECPropertyP nestedArrayProp;
    NavigationECPropertyP navProp;

    PrimitiveArrayECPropertyP primitiveArrayProp;

    navRelClass->GetSource().AddClass(*baseClass);
    navRelClass->GetTarget().AddClass(*baseClass);

    class1->CreateStructProperty(structProp, "StructMember", *structClass);
    class1->CreateStructArrayProperty(nestedArrayProp, "NestedArray", *structClass);
    class1->CreateNavigationProperty(navProp, "NavProp", *navRelClass, ECRelatedInstanceDirection::Forward, false);

    class1->CreatePrimitiveArrayProperty(primitiveArrayProp, "PrimitiveArrayProp");
    primitiveArrayProp->SetPrimitiveElementType(PRIMITIVETYPE_Long);
    primitiveArrayProp->SetMinOccurs(1);
    primitiveArrayProp->SetMaxOccurs(10);

    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema));
    class1->RemoveProperty("StructMember");
    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema));
    class1->RemoveProperty("NestedArray");
    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema));
    class1->RemoveProperty("NavProp");
    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Caleb.Shafer                            11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, ExpectErrorWhenTryRemoveSchemaInUseWithCustomAttributes)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 5, 0, 5);
    
    ECEntityClassP entityClass;
    PrimitiveECPropertyP prop;
    ECRelationshipClassP relClass;

    schema->CreateRelationshipClass(relClass, "RelClass");
    schema->CreateEntityClass(entityClass, "EntityClass");
    entityClass->CreatePrimitiveProperty(prop, "Prop");

    ECCustomAttributeClassP custAttribute;

    refSchema->CreateCustomAttributeClass(custAttribute, "SchemaCA");
    StandaloneECEnablerPtr enabler = custAttribute->GetDefaultStandaloneEnabler();
    IECInstancePtr instance = enabler->CreateInstance().get();

    // Test a Custom Attribute at the schema level
    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*refSchema));

    EXPECT_EQ(ECObjectsStatus::Success, schema->SetCustomAttribute(*instance));
    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema));
    schema->RemoveCustomAttribute(refSchema->GetName(), custAttribute->GetName());
    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema));
    
    // Test a Custom Attribute at the class level
    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*refSchema));

    EXPECT_EQ(ECObjectsStatus::Success, entityClass->SetCustomAttribute(*instance));
    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema));
    entityClass->RemoveCustomAttribute(refSchema->GetName(), custAttribute->GetName());
    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema));

    // Test a Custom Attribute at the property level
    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*refSchema));

    EXPECT_EQ(ECObjectsStatus::Success, prop->SetCustomAttribute(*instance));
    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema));
    prop->RemoveCustomAttribute(refSchema->GetName(), custAttribute->GetName());
    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema));

    // Test a Custom Attribute on Relationship Constraints
    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*refSchema));

    EXPECT_EQ(ECObjectsStatus::Success, relClass->GetSource().SetCustomAttribute(*instance));
    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema));
    relClass->GetSource().RemoveCustomAttribute(refSchema->GetName(), custAttribute->GetName());
    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema));

    // Test a Custom Attribute on Relationship Constraints
    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*refSchema));

    EXPECT_EQ(ECObjectsStatus::Success, relClass->GetTarget().SetCustomAttribute(*instance));
    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema));
    relClass->GetTarget().RemoveCustomAttribute(refSchema->GetName(), custAttribute->GetName());
    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    01/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaReferenceTest, ExpectErrorWhenTryRemoveReferencedSchemaWithIsMixin)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, "RefSchema", "rs", 5, 0, 5);

    ECEntityClassP entityClass;
    ECEntityClassP mixinClass;

    refSchema->CreateEntityClass(entityClass, "EntityClass");
    
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateMixinClass(mixinClass, "Mixin", *entityClass)) << "Should not fail to add mixin with class in referenced schema";
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schema, *refSchema)) << "The CreateMixinClass succeeded without creating a reference to the schema the AppliesTo class is located in.";

    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema)) << "The schema containing the appliesTo class was removed when it shouldn't be because it is still in use within the Mixin CA";

    ASSERT_TRUE(mixinClass->RemoveCustomAttribute("IsMixin")) << "The IsMixin custom attribute could not be removed.";
    EXPECT_FALSE(mixinClass->IsMixin()) << "IsMixin returned true even though the custom attribute was successfully removed.";

    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema)) << "The schema containing the appliesTo class could not be removed even though all references to it have been removed.";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    06/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaReferenceTest, ExpectErrorWhenTryRemoveReferencedSchemaWithPropertyCategory)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 5, 0, 5);

    ECEntityClassP entityClass;
    PropertyCategoryP propertyCategory;
    PrimitiveECPropertyP primProp;

    schema->AddReferencedSchema(*refSchema);
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schema, *refSchema));

    refSchema->CreatePropertyCategory(propertyCategory, "PropertyCategory");
    
    schema->CreateEntityClass(entityClass, "Entity");
    entityClass->CreatePrimitiveProperty(primProp, "PrimProp");
    primProp->SetCategory(propertyCategory);

    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema)) << "The schema containing the property category was removed when it shouldn't be because it is still in use within the ECProperty";

    ASSERT_EQ(ECObjectsStatus::Success, primProp->SetCategory(nullptr));
    EXPECT_FALSE(primProp->IsCategoryDefinedLocally());

    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema)) << "The schema containing the propertyCateogry could not be removed even though all references to it have been removed.";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                      02/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaReferenceTest, ExpectErrorWhenTryRemoveReferencedSchemaWithKindOfQuantity)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 5, 0, 5);

    ECEntityClassP entityClass;
    KindOfQuantityP kindOfQuantity;
    PrimitiveECPropertyP primProp;

    schema->AddReferencedSchema(*refSchema);
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schema, *refSchema));

    refSchema->CreateKindOfQuantity(kindOfQuantity, "KindOfQuantity");

    schema->CreateEntityClass(entityClass, "Entity");
    entityClass->CreatePrimitiveProperty(primProp, "PrimProp");
    primProp->SetKindOfQuantity(kindOfQuantity);

    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema)) << 
        "The schema containing the KindOfQUantity was removed when it shouldn't be because it is still in use within the ECProperty";

    ASSERT_EQ(ECObjectsStatus::Success, primProp->SetKindOfQuantity(nullptr));
    EXPECT_FALSE(primProp->IsKindOfQuantityDefinedLocally());

    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema)) << 
        "The schema containing the KindOfQuantity could not be removed even though all references to it have been removed.";
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Colin.Kerr                      02/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaReferenceTest, ExpectErrorWhenTryRemoveReferencedSchemaWithEnumeration)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 5, 0, 5);

    ECEntityClassP entityClass;
    ECEnumerationP enumeration;
    PrimitiveECPropertyP primProp;

    schema->AddReferencedSchema(*refSchema);
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schema, *refSchema));

    refSchema->CreateEnumeration(enumeration, "Enum", PrimitiveType::PRIMITIVETYPE_Integer);

    schema->CreateEntityClass(entityClass, "Entity");
    entityClass->CreatePrimitiveProperty(primProp, "PrimProp");
    primProp->SetType(*enumeration);

    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema)) <<
        "The schema containing the ECEnumeration was removed when it shouldn't be because it is still in use within the ECProperty";

    ASSERT_EQ(ECObjectsStatus::Success, primProp->SetType(PrimitiveType::PRIMITIVETYPE_Double));
    EXPECT_EQ(nullptr, primProp->GetEnumeration());

    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema)) <<
        "The schema containing the ECEnumeration could not be removed even though all references to it have been removed.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Dylan.Rush                           05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, ExpectFailureWithCircularReferences)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    WString seedPath(ECTestFixture::GetTestDataPath(L"").c_str());
    schemaContext->AddSchemaPath(seedPath.c_str());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"CircleSchema.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_FALSE(SchemaReadStatus::Success == status);
    EXPECT_FALSE(schema.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, ExpectSuccessWithSpecialCaseOpenPlantSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    WString seedPath(ECTestFixture::GetTestDataPath(L"").c_str());
    schemaContext->AddSchemaPath(seedPath.c_str());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"OpenPlant_Supplemental_Mapping_OPPID.01.01.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECSchemaReferenceListCR refList = schema->GetReferencedSchemas();
    EXPECT_EQ(1, refList.size());
    ECSchemaPtr refSchema = refList.begin()->second;
    EXPECT_EQ(0, refSchema->GetName().CompareTo("Bentley_Standard_CustomAttributes"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, FindClassInReferenceList)
    {
    ECSchemaPtr schema, refSchema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 5, 0, 5);

    ECRelationshipClassP relClass;
    ECEntityClassP targetClass, sourceClass;
    schema->CreateRelationshipClass(relClass, "RElationshipClass");
    schema->CreateEntityClass(targetClass, "Target");
    refSchema->CreateEntityClass(sourceClass, "Source");

    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*refSchema));
    EXPECT_EQ(ECObjectsStatus::Success, relClass->GetTarget().AddClass(*targetClass));
    EXPECT_EQ(ECObjectsStatus::Success, relClass->GetSource().AddClass(*sourceClass));

    ECSchemaReferenceListCR refList = schema->GetReferencedSchemas();

    EXPECT_NE(nullptr, refList.FindClassP(SchemaNameClassNamePair("RefSchema", "Source")));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaReferenceTest, TestSchemaCannotReferenceItself)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);

    ASSERT_NE(ECObjectsStatus::Success, schema->AddReferencedSchema(*schema)) << "Schema was able to reference itself which isn't allowed.";
    }

//=======================================================================================
//! SchemaLocateTest
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                       Carole.MacDonald           08/2011
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaLocateTest, ExpectSuccessWhenLocatingStandardSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    bmap<Utf8String, Utf8CP> standardSchemaNames;
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Bentley_Standard_CustomAttributes", "01.04"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Bentley_Standard_Classes", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Bentley_ECSchemaMap", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("EditorCustomAttributes", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Bentley_Common_Classes", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Dimension_Schema", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("iip_mdb_customAttributes", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("KindOfQuantity_Schema", "01.01"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("rdl_customAttributes", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("SIUnitSystemDefaults", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Unit_Attributes", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Units_Schema", "01.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("USCustomaryUnitSystemDefaults", "01.00"));

    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("CoreCustomAttributes", "01.00.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("SchemaLocalizationCustomAttributes", "01.00.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Formats", "01.00.00"));
    standardSchemaNames.insert(bpair<Utf8String, Utf8CP>("Units", "01.00.00"));

    ECSchemaPtr schema;

    for (bmap<Utf8String, Utf8CP>::const_iterator it = standardSchemaNames.begin(); it != standardSchemaNames.end(); ++it)
        {
        bpair<Utf8String, Utf8CP>const& entry = *it;

        SchemaKey key(entry.first.c_str(), 1, 0);
        EXPECT_TRUE(ECSchema::ParseVersionString(key.m_versionRead, key.m_versionMinor, entry.second) == ECObjectsStatus::Success);
        EXPECT_EQ(key.m_versionRead, atoi(entry.second));
        EXPECT_EQ(key.m_versionMinor, atoi(strchr(entry.second, '.') + 1));
        schema = ECSchema::LocateSchema(key, *schemaContext);
        EXPECT_TRUE(schema.IsValid());
        EXPECT_TRUE(schema->IsStandardSchema());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaLocateTest, ExpectFailureWithNonStandardSchema)
    {
    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, "TestSchema", "ts", 1, 0, 2);
    EXPECT_FALSE(testSchema->IsStandardSchema());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                       Carole.MacDonald           08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaLocateTest, DetermineWhetherSchemaCanBeImported)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    SchemaKey key("Bentley_Standard_CustomAttributes", 1, 4);

    ECSchemaPtr schema = ECSchema::LocateSchema(key, *schemaContext);
    EXPECT_TRUE(schema.IsValid());
    EXPECT_FALSE(schema->ShouldNotBeStored());

    ECSchema::CreateSchema(schema, "Units_Schema", "ts", 1, 0, 4);
    EXPECT_TRUE(schema->ShouldNotBeStored());
    }

//=======================================================================================
//! SchemaCreationTest
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                 02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCreationTest, CanFullyCreateASchema)
    {
    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, "TestSchema", "ts", 1, 0, 2);
    testSchema->SetDescription("Schema for testing programmatic construction");
    testSchema->SetDisplayLabel("Test Schema");

    EXPECT_TRUE(testSchema->GetIsDisplayLabelDefined());
    EXPECT_EQ(1, testSchema->GetVersionRead());
    EXPECT_EQ(2, testSchema->GetVersionMinor());
    EXPECT_EQ(0, strcmp(testSchema->GetName().c_str(), "TestSchema"));
    EXPECT_EQ(0, strcmp(testSchema->GetAlias().c_str(), "ts"));
    EXPECT_EQ(0, strcmp(testSchema->GetDescription().c_str(), "Schema for testing programmatic construction"));
    EXPECT_EQ(0, strcmp(testSchema->GetDisplayLabel().c_str(), "Test Schema"));

    ECSchemaPtr schema2;
    ECSchema::CreateSchema(schema2, "BaseSchema", "ts", 5, 0, 5);

    testSchema->AddReferencedSchema(*schema2);

    ECEntityClassP class1;
    ECEntityClassP class2;
    ECEntityClassP baseClass;
    ECStructClassP structClass;
    ECEntityClassP relatedClass;
    ECRelationshipClassP relationshipClass;

    testSchema->CreateEntityClass(class1, "TestClass");
    testSchema->CreateEntityClass(class2, "TestClass2");
    testSchema->CreateStructClass(structClass, "StructClass");
    schema2->CreateEntityClass(baseClass, "BaseClass");
    testSchema->CreateEntityClass(relatedClass, "RelatedClass");

    class1->SetDescription("Class for testing purposes");
    class1->SetDisplayLabel("Test Class");

    EXPECT_EQ(0, strcmp(class1->GetDescription().c_str(), "Class for testing purposes"));
    EXPECT_EQ(0, strcmp(class1->GetDisplayLabel().c_str(), "Test Class"));
    EXPECT_TRUE(class1->IsEntityClass());
    EXPECT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*baseClass));
    EXPECT_TRUE(class1->HasBaseClasses());

    EXPECT_TRUE(structClass->IsStructClass());

    relatedClass->AddBaseClass(*baseClass);

    PrimitiveECPropertyP stringProp;
    StructECPropertyP structProp;
    StructArrayECPropertyP nestedArrayProp;
    PrimitiveArrayECPropertyP primitiveArrayProp;

    class1->CreatePrimitiveProperty(stringProp, "StringMember");
    class1->CreateStructProperty(structProp, "StructMember", *structClass);
    class1->CreateStructArrayProperty(nestedArrayProp, "NestedArray", *structClass);
    class1->CreatePrimitiveArrayProperty(primitiveArrayProp, "PrimitiveArray");

    primitiveArrayProp->SetPrimitiveElementType(PRIMITIVETYPE_Long);
    primitiveArrayProp->SetMinOccurs(1);
    primitiveArrayProp->SetMaxOccurs(10);

    EXPECT_TRUE(ARRAYKIND_Struct == nestedArrayProp->GetKind());
    EXPECT_TRUE(ARRAYKIND_Primitive == primitiveArrayProp->GetKind());
    EXPECT_EQ(0, nestedArrayProp->GetMinOccurs());
    EXPECT_EQ(UINT_MAX, nestedArrayProp->GetMaxOccurs());
    EXPECT_EQ(1, primitiveArrayProp->GetMinOccurs());
#if FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    EXPECT_EQ(10, primitiveArrayProp->GetMaxOccurs());
#else
    EXPECT_EQ(UINT_MAX, primitiveArrayProp->GetMaxOccurs());
#endif
    EXPECT_TRUE(stringProp->GetIsPrimitive());
    EXPECT_FALSE(stringProp->GetIsStruct());
    EXPECT_FALSE(stringProp->GetIsArray());

    EXPECT_FALSE(structProp->GetIsPrimitive());
    EXPECT_TRUE(structProp->GetIsStruct());
    EXPECT_FALSE(structProp->GetIsArray());

    EXPECT_FALSE(primitiveArrayProp->GetIsPrimitive());
    EXPECT_FALSE(primitiveArrayProp->GetIsStruct());
    EXPECT_TRUE(primitiveArrayProp->GetIsArray());

    EXPECT_FALSE(stringProp->GetIsReadOnly());

    EXPECT_EQ(0, strcmp(stringProp->GetTypeName().c_str(), "string"));
    EXPECT_TRUE(PRIMITIVETYPE_String == stringProp->GetType());
    EXPECT_EQ(0, strcmp(structProp->GetType().GetName().c_str(), "StructClass"));

    PrimitiveECPropertyP binaryProperty;
    PrimitiveECPropertyP booleanProperty;
    PrimitiveECPropertyP dateTimeProperty;
    PrimitiveECPropertyP doubleProperty;
    PrimitiveECPropertyP integerProperty;
    PrimitiveECPropertyP longProperty;
    PrimitiveECPropertyP point2DProperty;
    PrimitiveECPropertyP point3DProperty;

    class1->CreatePrimitiveProperty(binaryProperty, "BinaryProp");
    class1->CreatePrimitiveProperty(booleanProperty, "BooleanProp");
    class1->CreatePrimitiveProperty(dateTimeProperty, "DateTimeProp");
    class1->CreatePrimitiveProperty(doubleProperty, "DoubleProp");
    class1->CreatePrimitiveProperty(integerProperty, "IntProp");
    class1->CreatePrimitiveProperty(longProperty, "LongProp");
    class1->CreatePrimitiveProperty(point2DProperty, "Point2dProp");
    class1->CreatePrimitiveProperty(point3DProperty, "Point3dProp");

    EXPECT_EQ(ECObjectsStatus::ParseError, binaryProperty->SetTypeName("fake"));

    binaryProperty->SetTypeName("binary");
    booleanProperty->SetTypeName("boolean");
    dateTimeProperty->SetTypeName("dateTime");
    doubleProperty->SetTypeName("double");
    integerProperty->SetTypeName("int");
    longProperty->SetTypeName("long");
    point2DProperty->SetTypeName("point2d");
    point3DProperty->SetTypeName("point3d");

    EXPECT_TRUE(PRIMITIVETYPE_Binary == binaryProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Boolean == booleanProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_DateTime == dateTimeProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Double == doubleProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Integer == integerProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Long == longProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Point2d == point2DProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Point3d == point3DProperty->GetType());

    class1->CreatePrimitiveProperty(binaryProperty, "BinaryProp2", PRIMITIVETYPE_Binary);
    class1->CreatePrimitiveProperty(booleanProperty, "BooleanProp2", PRIMITIVETYPE_Boolean);
    class1->CreatePrimitiveProperty(dateTimeProperty, "DateTimeProp2", PRIMITIVETYPE_DateTime);
    class1->CreatePrimitiveProperty(doubleProperty, "DoubleProp2", PRIMITIVETYPE_Double);
    class1->CreatePrimitiveProperty(integerProperty, "IntProp2", PRIMITIVETYPE_Integer);
    class1->CreatePrimitiveProperty(longProperty, "LongProp2", PRIMITIVETYPE_Long);
    class1->CreatePrimitiveProperty(point2DProperty, "Point2dProp2", PRIMITIVETYPE_Point2d);
    class1->CreatePrimitiveProperty(point3DProperty, "Point3dProp2", PRIMITIVETYPE_Point3d);

    EXPECT_TRUE(PRIMITIVETYPE_Binary == binaryProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Boolean == booleanProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_DateTime == dateTimeProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Double == doubleProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Integer == integerProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Long == longProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Point2d == point2DProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Point3d == point3DProperty->GetType());

    class1->CreateStructProperty(structProp, "StructMember2", *structClass);
    class1->CreateStructArrayProperty(nestedArrayProp, "NestedArray2", *structClass);
    class1->CreatePrimitiveArrayProperty(primitiveArrayProp, "PrimitiveArray2", PRIMITIVETYPE_Integer);
    EXPECT_TRUE(ARRAYKIND_Struct == nestedArrayProp->GetKind());
    EXPECT_TRUE(ARRAYKIND_Primitive == primitiveArrayProp->GetKind());
    EXPECT_EQ(0, strcmp(structProp->GetType().GetName().c_str(), "StructClass"));
    EXPECT_EQ(0, strcmp(nestedArrayProp->GetTypeName().c_str(), "StructClass"));
    EXPECT_EQ(0, strcmp(primitiveArrayProp->GetTypeName().c_str(), "int"));

    testSchema->CreateRelationshipClass(relationshipClass, "RelationshipClass");
    EXPECT_TRUE(StrengthType::Referencing == relationshipClass->GetStrength());
    relationshipClass->SetStrength(StrengthType::Embedding);
    EXPECT_TRUE(StrengthType::Embedding == relationshipClass->GetStrength());

    EXPECT_TRUE(ECRelatedInstanceDirection::Forward == relationshipClass->GetStrengthDirection());
    relationshipClass->SetStrengthDirection(ECRelatedInstanceDirection::Backward);
    EXPECT_TRUE(ECRelatedInstanceDirection::Backward == relationshipClass->GetStrengthDirection());

    EXPECT_TRUE(relationshipClass->GetTarget().GetIsPolymorphic());
    EXPECT_TRUE(relationshipClass->GetSource().GetIsPolymorphic());
    relationshipClass->GetSource().SetIsPolymorphic(false);
    EXPECT_FALSE(relationshipClass->GetSource().GetIsPolymorphic());

    relationshipClass->SetDescription("Relates the test class to the related class");
    relationshipClass->SetDisplayLabel("TestRelationshipClass");

    EXPECT_EQ(0, relationshipClass->GetSource().GetConstraintClasses().size());
    EXPECT_EQ(0, relationshipClass->GetTarget().GetConstraintClasses().size());

    relationshipClass->GetSource().AddClass(*class1);
    EXPECT_EQ(1, relationshipClass->GetSource().GetConstraintClasses().size());

    relationshipClass->GetTarget().AddClass(*relatedClass);
    EXPECT_EQ(1, relationshipClass->GetTarget().GetConstraintClasses().size());
    relationshipClass->GetTarget().AddClass(*relatedClass);
    EXPECT_EQ(1, relationshipClass->GetTarget().GetConstraintClasses().size());

    relationshipClass->GetTarget().SetAbstractConstraint(*baseClass);
    EXPECT_EQ(baseClass->GetName().c_str(), relationshipClass->GetTarget().GetAbstractConstraint()->GetName().c_str());

    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, relationshipClass->GetTarget().AddClass(*class2)) << "Should not be able to add target constraint class because it is not compatible with the abstract constraint";
    EXPECT_EQ(1, relationshipClass->GetTarget().GetConstraintClasses().size());

    EXPECT_EQ(ECObjectsStatus::Success, class2->AddBaseClass(*baseClass)) << "Expected to be able to add base class";
    EXPECT_EQ(ECObjectsStatus::Success, relationshipClass->GetTarget().AddClass(*class2)) << "Expected to be able to add class2 to the target constraint because 'class2' derives from the abstract constraint";
    EXPECT_EQ(2, relationshipClass->GetTarget().GetConstraintClasses().size());

    EXPECT_EQ(0, relationshipClass->GetSource().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(0, relationshipClass->GetTarget().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(1, relationshipClass->GetSource().GetMultiplicity().GetUpperLimit());
    EXPECT_EQ(1, relationshipClass->GetTarget().GetMultiplicity().GetUpperLimit());

    relationshipClass->GetSource().SetMultiplicity(RelationshipMultiplicity::OneMany());
    EXPECT_EQ(1, relationshipClass->GetSource().GetMultiplicity().GetLowerLimit());
    EXPECT_TRUE(relationshipClass->GetSource().GetMultiplicity().IsUpperLimitUnbounded());

    RelationshipMultiplicity *card = new RelationshipMultiplicity(2, 5);
    relationshipClass->GetTarget().SetMultiplicity(*card);
    EXPECT_EQ(2, relationshipClass->GetTarget().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(5, relationshipClass->GetTarget().GetMultiplicity().GetUpperLimit());
    delete card;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kyle.Abramowitz   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCreationTest, CreatingASchemaWithLatestECVersionSetsOriginalXmlVersionTo31)
    {
    ECSchemaPtr testSchema;
    uint32_t    latestMajor;
    uint32_t    latestMinor;
    uint32_t    testSchemaMajor;
    uint32_t    testSchemaMinor;

    ECSchema::ParseECVersion(latestMajor, latestMinor, ECVersion::Latest);
    ECSchema::CreateSchema(testSchema, "TestSchema", "ts", 1, 0, 2, ECVersion::Latest);
    
    EXPECT_EQ(3, testSchema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(1, testSchema->GetOriginalECXmlVersionMinor());

    ECSchema::ParseECVersion(testSchemaMajor, testSchemaMinor, testSchema->GetECVersion());
    EXPECT_EQ(latestMajor, testSchemaMajor);
    EXPECT_EQ(latestMinor, testSchemaMinor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kyle.Abramowitz   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCreationTest, CreatingASchemaWithOldVersionStillSetsLatestOriginalXmlVersion)
    {
    ECSchemaPtr testSchema;
    uint32_t    latestMajor;
    uint32_t    latestMinor;
    uint32_t    testSchemaMajor;
    uint32_t    testSchemaMinor;

    ECSchema::ParseECVersion(latestMajor, latestMinor, ECVersion::V3_1);
    ECSchema::CreateSchema(testSchema, "TestSchema", "ts", 1, 0, 2, ECVersion::V3_0);

    EXPECT_EQ(latestMajor, testSchema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(latestMinor, testSchema->GetOriginalECXmlVersionMinor());

    ECSchema::ParseECVersion(testSchemaMajor, testSchemaMinor, testSchema->GetECVersion());
    EXPECT_EQ(3, testSchemaMajor);
    EXPECT_EQ(0, testSchemaMinor);
    }
//=======================================================================================
//! ECNameValidationTest
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP GetPropertyByName(ECClassCR ecClass, Utf8CP name, bool expectExists = true)
    {
    ECPropertyP prop = ecClass.GetPropertyP(name);
    EXPECT_EQ(expectExists, nullptr != prop);
    Utf8String utf8(name);
    prop = ecClass.GetPropertyP(utf8.c_str());
    EXPECT_EQ(expectExists, nullptr != prop);
    return prop;
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECNameValidationTest : ECTestFixture
    {
    struct ITester
        {
        virtual void        Preprocess(ECSchemaR schema) const = 0;
        virtual void        Postprocess(ECSchemaR schema) const = 0;
        };

    void Roundtrip(ITester const& tester)
        {
        ECSchemaPtr schema;
        ECSchema::CreateSchema(schema, "MySchema", "ts", 1, 0, 1);
        tester.Preprocess(*schema);

        Utf8String schemaXml;
        EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(schemaXml, ECVersion::V2_0));

        schema = nullptr;
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *context));

        tester.Postprocess(*schema);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DisplayLabelTester : ECNameValidationTest::ITester
    {
    Utf8String         m_name;
    Utf8String         m_encodedName;

    DisplayLabelTester(Utf8CP name, Utf8CP encodedName) : m_name(name), m_encodedName(encodedName) {}

    template<typename T> void CompareNoAutoDecode(T const& target) const
        {
        EXPECT_FALSE(target.GetIsDisplayLabelDefined());
        EXPECT_STREQ(m_encodedName.c_str(), target.GetName().c_str());
        EXPECT_STREQ(m_encodedName.c_str(), target.GetDisplayLabel().c_str());
        }
    
    template<typename T> void Compare(T const& target) const
        {
        EXPECT_EQ(!ECNameValidation::IsValidName(m_name.c_str()), target.GetIsDisplayLabelDefined());
        EXPECT_STREQ(m_encodedName.c_str(), target.GetName().c_str());
        EXPECT_STREQ(m_name.c_str(), target.GetDisplayLabel().c_str());
        }

    template<typename T> void CompareOverriddenLabel(T const& target, Utf8CP label) const
        {
        EXPECT_TRUE(target.GetIsDisplayLabelDefined());
        EXPECT_STREQ(label, target.GetDisplayLabel().c_str());
        }

    virtual void Preprocess(ECSchemaR schema) const override
        {
        // This test used to test that ECObjects would decode names into display labels, this now only happens for deserialization from EC2 schemas
        Utf8String encodedName;
        EXPECT_EQ(!ECNameValidation::IsValidName(m_name.c_str()), ECNameValidation::EncodeToValidName(encodedName, m_name));
        schema.SetName(encodedName);
        CompareNoAutoDecode(schema);

        ECEntityClassP ecclass;
        schema.CreateEntityClass(ecclass, encodedName);
        CompareNoAutoDecode(*ecclass);

        PrimitiveECPropertyP ecprop;
        ecclass->CreatePrimitiveProperty(ecprop, encodedName, PRIMITIVETYPE_String);
        CompareNoAutoDecode(*ecprop);
        }

    virtual void Postprocess(ECSchemaR schema) const override
        {
        ECClassP ecclass = schema.GetClassP(m_encodedName.c_str());
        ECPropertyP ecprop = GetPropertyByName(*ecclass, m_encodedName.c_str());

        Compare(schema);
        Compare(*ecclass);
        Compare(*ecprop);

        // Test explicitly setting display labels
        schema.SetDisplayLabel("NewDisplayLabel");
        CompareOverriddenLabel(schema, "NewDisplayLabel");
        ecclass->SetDisplayLabel("1!@$");
        CompareOverriddenLabel(*ecclass, "1!@$");                // will not be encoded
        ecprop->SetDisplayLabel("__x003E__");
        CompareOverriddenLabel(*ecprop, "__x003E__");            // will not be decoded

        // Test explicitly un-setting display labels
        ecclass->SetDisplayLabel("");
        CompareNoAutoDecode(*ecclass);
        ecprop->SetDisplayLabel("");
        CompareNoAutoDecode(*ecprop);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECNameValidationTest, DisplayLabels)
    {
    // Chinese chars...see TFS#298776...we are stuck with UTF-16 persistent encoding from long ago, should round-trip correctly with UTF-8
    static const unsigned char s_chineseUtf8[] = {0xE8, 0x88, 0xAC,
                                     0xE6, 0xA8, 0xA1,
                                     0xE5, 0x9E, 0x8B,
                                     '\0'};
    // UTF-16: 822C 6A21 578B

    static const Utf8CP s_testValues[] =
        {
        "NothingSpecial", "NothingSpecial",
        "Nothing1Special2", "Nothing1Special2",
        "1_LeadingDigitsDisallowed", "__x0031___LeadingDigitsDisallowed",
        "Special!", "Special__x0021__",
        "thing@mail.com", "thing__x0040__mail__x002E__com",
        "*", "__x002A__",
        "9&:", "__x0039____x0026____x003A__",
        "__xNotAChar__", "__xNotAChar__",
        "__xTTTT__", "__xTTTT__",
        "__x####__", "__x__x0023____x0023____x0023____x0023____",
        (Utf8CP) s_chineseUtf8, "__x822C____x6A21____x578B__",
        nullptr, nullptr
        };

    for (Utf8CP const* cur = s_testValues; *cur; cur += 2)
        {
        Utf8CP name = *cur, encoded = *(cur + 1);
        Roundtrip(DisplayLabelTester(name, encoded));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECNameValidationTest, Validate)
    {
#define EXPECT_VALIDATION_RESULT(RESULTTOEXPECT, NAMETOTEST) EXPECT_EQ (ECNameValidation::RESULT_ ## RESULTTOEXPECT, ECNameValidation::Validate (NAMETOTEST))
    EXPECT_VALIDATION_RESULT(Valid, "ThisIsAValidName");
    EXPECT_VALIDATION_RESULT(Valid, "_123");
    EXPECT_VALIDATION_RESULT(Valid, "___");
    EXPECT_VALIDATION_RESULT(Valid, "A123");

    EXPECT_VALIDATION_RESULT(NullOrEmpty, "");
    EXPECT_VALIDATION_RESULT(NullOrEmpty, nullptr);

    EXPECT_VALIDATION_RESULT(BeginsWithDigit, "1_C");

    EXPECT_VALIDATION_RESULT(IncludesInvalidCharacters, "!ABC");
    EXPECT_VALIDATION_RESULT(IncludesInvalidCharacters, "ABC@");
    }

//=======================================================================================
//! SchemaKeyComparisonTest
//=======================================================================================

struct SchemaKeyComparisonTest : ECTestFixture
{
    SchemaKey key1WithChecksum;
    SchemaKey key1_0_0;
    SchemaKey key1_0_1;
    SchemaKey key1_1_0;
    SchemaKey key2_0_0;
    SchemaKey diffChecksumKey;
    SchemaKey diffNameKey;

    void SetUp() override {
        key1WithChecksum = SchemaKey("SchemaTest", 1, 0, 0);
        key1WithChecksum.m_checksum = "aBcD";

        key1_0_0 = SchemaKey("SchemaTest", 1, 0, 0);
        // key1_0_0.m_checksum = "aBcD";
        key1_0_1 = SchemaKey("SchemaTest", 1, 0, 1);
        // key1_0_1.m_checksum = "aBcD";
        key1_1_0 = SchemaKey("SchemaTest", 1, 1, 0);
        // key1_1_0.m_checksum = "aBcD";
        key2_0_0 = SchemaKey("SchemaTest", 2, 0, 0);
        // key2_0_0.m_checksum = "aBcD";

        diffChecksumKey = SchemaKey("SchemaTest", 1, 0, 0);
        diffChecksumKey.m_checksum = "aBc";
        diffNameKey = SchemaKey("SchemaNotTest", 1, 0, 0);
        // diffNameKey.m_checksum = "aB";
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaKeyComparisonTest, VerifyMatchesOperator)
    {
    // Verifies an identical SchemaKey matches in all cases
    EXPECT_TRUE(key1_0_0 == key1WithChecksum);
    EXPECT_TRUE(key1_0_0.Matches(key1WithChecksum, SchemaMatchType::Identical));
    EXPECT_TRUE(key1_0_0.Matches(key1WithChecksum, SchemaMatchType::Exact));
    EXPECT_TRUE(key1_0_0.Matches(key1WithChecksum, SchemaMatchType::LatestWriteCompatible));
    EXPECT_TRUE(key1_0_0.Matches(key1WithChecksum, SchemaMatchType::LatestReadCompatible));
    EXPECT_TRUE(key1_0_0.Matches(key1WithChecksum, SchemaMatchType::Latest));

    //// Differs by name

    // Verifies when the left-side differs by name
    EXPECT_FALSE(diffNameKey == key1_0_0);
    EXPECT_FALSE(diffNameKey.Matches(key1_0_0, SchemaMatchType::Identical));
    EXPECT_FALSE(diffNameKey.Matches(key1_0_0, SchemaMatchType::Exact));
    EXPECT_FALSE(diffNameKey.Matches(key1_0_0, SchemaMatchType::LatestWriteCompatible));
    EXPECT_FALSE(diffNameKey.Matches(key1_0_0, SchemaMatchType::LatestReadCompatible));
    EXPECT_FALSE(diffNameKey.Matches(key1_0_0, SchemaMatchType::Latest));

    // Verifies when the right-side differs by name
    EXPECT_FALSE(key1_0_0 == diffNameKey);
    EXPECT_FALSE(key1_0_0.Matches(diffNameKey, SchemaMatchType::Identical));
    EXPECT_FALSE(key1_0_0.Matches(diffNameKey, SchemaMatchType::Exact));
    EXPECT_FALSE(key1_0_0.Matches(diffNameKey, SchemaMatchType::LatestWriteCompatible));
    EXPECT_FALSE(key1_0_0.Matches(diffNameKey, SchemaMatchType::LatestReadCompatible));
    EXPECT_FALSE(key1_0_0.Matches(diffNameKey, SchemaMatchType::Latest));

    //// Same version with different checksum

    // Verifies when the left-side only differs by checksum
    EXPECT_FALSE(diffChecksumKey == key1WithChecksum);
    EXPECT_FALSE(diffChecksumKey.Matches(key1WithChecksum, SchemaMatchType::Identical));
    EXPECT_TRUE(diffChecksumKey.Matches(key1WithChecksum, SchemaMatchType::Exact));
    EXPECT_TRUE(diffChecksumKey.Matches(key1WithChecksum, SchemaMatchType::LatestWriteCompatible));
    EXPECT_TRUE(diffChecksumKey.Matches(key1WithChecksum, SchemaMatchType::LatestReadCompatible));
    EXPECT_TRUE(diffChecksumKey.Matches(key1WithChecksum, SchemaMatchType::Latest));

    // Verifies when the right-side only differs by checksum
    EXPECT_FALSE(key1WithChecksum == diffChecksumKey);
    EXPECT_FALSE(key1WithChecksum.Matches(diffChecksumKey, SchemaMatchType::Identical));
    EXPECT_TRUE(key1WithChecksum.Matches(diffChecksumKey, SchemaMatchType::Exact));
    EXPECT_TRUE(key1WithChecksum.Matches(diffChecksumKey, SchemaMatchType::LatestWriteCompatible));
    EXPECT_TRUE(key1WithChecksum.Matches(diffChecksumKey, SchemaMatchType::LatestReadCompatible));
    EXPECT_TRUE(key1WithChecksum.Matches(diffChecksumKey, SchemaMatchType::Latest));

    //// 1.0.1 vs 1.0.0

    // Verifies when the left-side has higher minor version number
    EXPECT_FALSE(key1_0_1 == key1_0_0);
    EXPECT_FALSE(key1_0_1.Matches(key1_0_0, SchemaMatchType::Identical));
    EXPECT_FALSE(key1_0_1.Matches(key1_0_0, SchemaMatchType::Exact));
    EXPECT_TRUE(key1_0_1.Matches(key1_0_0, SchemaMatchType::LatestWriteCompatible));
    EXPECT_TRUE(key1_0_1.Matches(key1_0_0, SchemaMatchType::LatestReadCompatible));
    EXPECT_TRUE(key1_0_1.Matches(key1_0_0, SchemaMatchType::Latest));

    // Verifies when the right-side has higher minor version number
    EXPECT_FALSE(key1_0_0 == key1_0_1);
    EXPECT_FALSE(key1_0_0.Matches(key1_0_1, SchemaMatchType::Identical));
    EXPECT_FALSE(key1_0_0.Matches(key1_0_1, SchemaMatchType::Exact));
    EXPECT_FALSE(key1_0_0.Matches(key1_0_1, SchemaMatchType::LatestWriteCompatible)) << "The key that is matched against has a minor version greater than the current version so it is not read compatible with the current schema.";
    EXPECT_FALSE(key1_0_0.Matches(key1_0_1, SchemaMatchType::LatestReadCompatible)) << "The key that is matched against has a minor version greater than the current version so it is not read compatible with the current schema.";
    EXPECT_TRUE(key1_0_0.Matches(key1_0_1, SchemaMatchType::Latest));

    //// 1.1.0 vs 1.0.0

    // Verifies when the left-side has higher write version number
    EXPECT_FALSE(key1_1_0 == key1_0_0);
    EXPECT_FALSE(key1_1_0.Matches(key1_0_0, SchemaMatchType::Identical));
    EXPECT_FALSE(key1_1_0.Matches(key1_0_0, SchemaMatchType::Exact));
    EXPECT_FALSE(key1_1_0.Matches(key1_0_0, SchemaMatchType::LatestWriteCompatible));
    EXPECT_TRUE(key1_1_0.Matches(key1_0_0, SchemaMatchType::LatestReadCompatible));
    EXPECT_TRUE(key1_1_0.Matches(key1_0_0, SchemaMatchType::Latest));

    // Verifies when the right-side has higher write version number
    EXPECT_FALSE(key1_0_0 == key1_1_0);
    EXPECT_FALSE(key1_0_0.Matches(key1_1_0, SchemaMatchType::Identical));
    EXPECT_FALSE(key1_0_0.Matches(key1_1_0, SchemaMatchType::Exact));
    EXPECT_FALSE(key1_0_0.Matches(key1_1_0, SchemaMatchType::LatestWriteCompatible));
    EXPECT_FALSE(key1_0_0.Matches(key1_1_0, SchemaMatchType::LatestReadCompatible)) << "The key that is matched against has a write version greater than the current version so it is not read compatible with the current schema.";
    EXPECT_TRUE(key1_0_0.Matches(key1_1_0, SchemaMatchType::Latest));

    //// 2.0.0 vs 1.0.0

    // Verifies when the left-side has higher major version number
    EXPECT_FALSE(key2_0_0 == key1_0_0);
    EXPECT_FALSE(key2_0_0.Matches(key1_0_0, SchemaMatchType::Identical));
    EXPECT_FALSE(key2_0_0.Matches(key1_0_0, SchemaMatchType::Exact));
    EXPECT_FALSE(key2_0_0.Matches(key1_0_0, SchemaMatchType::LatestWriteCompatible));
    EXPECT_FALSE(key2_0_0.Matches(key1_0_0, SchemaMatchType::LatestReadCompatible));
    EXPECT_TRUE(key2_0_0.Matches(key1_0_0, SchemaMatchType::Latest));

    // Verifies when the right-side has higher major version number
    EXPECT_FALSE(key1_0_0 == key2_0_0);
    EXPECT_FALSE(key1_0_0.Matches(key2_0_0, SchemaMatchType::Identical));
    EXPECT_FALSE(key1_0_0.Matches(key2_0_0, SchemaMatchType::Exact));
    EXPECT_FALSE(key1_0_0.Matches(key2_0_0, SchemaMatchType::LatestWriteCompatible));
    EXPECT_FALSE(key1_0_0.Matches(key2_0_0, SchemaMatchType::LatestReadCompatible));
    EXPECT_TRUE(key1_0_0.Matches(key2_0_0, SchemaMatchType::Latest));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaKeyComparisonTest, VerifyLessThanOperator)
    {
    // Same key
    EXPECT_FALSE(key1_0_0 < key1_0_0);
    EXPECT_FALSE(key1_0_0.LessThan(key1_0_0, SchemaMatchType::Identical));
    EXPECT_FALSE(key1_0_0.LessThan(key1_0_0, SchemaMatchType::Exact));
    EXPECT_FALSE(key1_0_0.LessThan(key1_0_0, SchemaMatchType::LatestWriteCompatible));
    EXPECT_FALSE(key1_0_0.LessThan(key1_0_0, SchemaMatchType::LatestReadCompatible));
    EXPECT_FALSE(key1_0_0.LessThan(key1_0_0, SchemaMatchType::Latest));

    // Only Name is different
    SchemaKey diffName("SchemaTesa", 1, 0, 0);

    EXPECT_TRUE(diffName < key1_0_0);
    EXPECT_TRUE(diffName.LessThan(key1_0_0, SchemaMatchType::Identical));
    EXPECT_TRUE(diffName.LessThan(key1_0_0, SchemaMatchType::Exact));
    EXPECT_TRUE(diffName.LessThan(key1_0_0, SchemaMatchType::LatestWriteCompatible));
    EXPECT_TRUE(diffName.LessThan(key1_0_0, SchemaMatchType::LatestReadCompatible));
    EXPECT_TRUE(diffName.LessThan(key1_0_0, SchemaMatchType::Latest));

    // Checksum
    EXPECT_TRUE(diffChecksumKey < key1WithChecksum);
    EXPECT_TRUE(diffChecksumKey.LessThan(key1WithChecksum, SchemaMatchType::Identical));
    EXPECT_FALSE(diffChecksumKey.LessThan(key1WithChecksum, SchemaMatchType::Exact)) << "Should not detect a difference since it does not look at the checksum";
    EXPECT_FALSE(diffChecksumKey.LessThan(key1WithChecksum, SchemaMatchType::LatestWriteCompatible)) << "Should not detect a difference since it does not look at the checksum";
    EXPECT_FALSE(diffChecksumKey.LessThan(key1WithChecksum, SchemaMatchType::LatestReadCompatible)) << "Should not detect a difference since it does not look at the checksum";
    EXPECT_FALSE(diffChecksumKey.LessThan(key1WithChecksum, SchemaMatchType::Latest)) << "Should not detect a difference since it does not look at the checksum";

    // Major version
    EXPECT_TRUE(key1_0_0 < key2_0_0);
    EXPECT_TRUE(key1_0_0.LessThan(key2_0_0, SchemaMatchType::Identical));
    EXPECT_TRUE(key1_0_0.LessThan(key2_0_0, SchemaMatchType::Exact));
    EXPECT_TRUE(key1_0_0.LessThan(key2_0_0, SchemaMatchType::LatestWriteCompatible));
    EXPECT_TRUE(key1_0_0.LessThan(key2_0_0, SchemaMatchType::LatestReadCompatible));
    EXPECT_FALSE(key1_0_0.LessThan(key2_0_0, SchemaMatchType::Latest));

    // Write version
    EXPECT_FALSE(key1_1_0 < key1_0_0);
    EXPECT_FALSE(key1_1_0.LessThan(key1_0_0, SchemaMatchType::Exact));
    EXPECT_FALSE(key1_1_0.LessThan(key1_0_0, SchemaMatchType::Identical));

    EXPECT_TRUE(key1_0_0 < key1_1_0);
    EXPECT_TRUE(key1_0_0.LessThan(key1_1_0, SchemaMatchType::Identical));
    EXPECT_TRUE(key1_0_0.LessThan(key1_1_0, SchemaMatchType::Exact));
    EXPECT_FALSE(key1_0_0.LessThan(key1_1_0, SchemaMatchType::LatestReadCompatible));
    EXPECT_TRUE(key1_0_0.LessThan(key1_1_0, SchemaMatchType::LatestWriteCompatible));
    EXPECT_FALSE(key1_0_0.LessThan(key1_1_0, SchemaMatchType::Latest));

    // Minor version
    EXPECT_TRUE(key1_0_0 < key1_0_1);
    EXPECT_TRUE(key1_0_0.LessThan(key1_0_1, SchemaMatchType::Identical));
    EXPECT_TRUE(key1_0_0.LessThan(key1_0_1, SchemaMatchType::Exact));
    EXPECT_FALSE(key1_0_0.LessThan(key1_0_1, SchemaMatchType::LatestReadCompatible));
    EXPECT_FALSE(key1_0_0.LessThan(key1_0_1, SchemaMatchType::LatestWriteCompatible));
    EXPECT_FALSE(key1_0_0.LessThan(key1_0_1, SchemaMatchType::Latest));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaKeyComparisonTest, VerifyNotMatchesOperator)
    {
    EXPECT_FALSE(key1_0_0 != key1WithChecksum);
    EXPECT_TRUE(key1WithChecksum != diffChecksumKey);
    EXPECT_TRUE(key1_0_0 != diffNameKey);
    EXPECT_TRUE(key1_0_0 != key1_1_0);
    EXPECT_TRUE(key1_0_0 != key1_0_1);
    EXPECT_TRUE(key1_0_0 != key2_0_0);
    }

//=======================================================================================
//! SchemaKeyTest
//=======================================================================================

//-------------------------------------------------------------------------------------
// @bsimethod                                      Kyle.Abramowitz           05/2018
//---------------+---------------+---------------+---------------+---------------+-----
TEST_F(SchemaKeyTest, TestParseSchemaVersionString)
    {
    uint32_t r = 0;
    uint32_t w = 0;
    uint32_t m = 0;

    EXPECT_EQ(ECObjectsStatus::Success, SchemaKey::ParseVersionString(r, w, m, ""));
    EXPECT_EQ(0, r);
    EXPECT_EQ(0, w);
    EXPECT_EQ(0, m);

    EXPECT_NE(ECObjectsStatus::InvalidECVersion, SchemaKey::ParseVersionString(r, w, m, "0"));
    EXPECT_NE(ECObjectsStatus::InvalidECVersion, SchemaKey::ParseVersionString(r, w, m, "0."));
    EXPECT_NE(ECObjectsStatus::InvalidECVersion, SchemaKey::ParseVersionString(r, w, m, ".0"));
    EXPECT_NE(ECObjectsStatus::InvalidECVersion, SchemaKey::ParseVersionString(r, w, m, "."));
    EXPECT_NE(ECObjectsStatus::InvalidECVersion, SchemaKey::ParseVersionString(r, w, m, ".."));

    EC_EXPECT_SUCCESS(SchemaKey::ParseVersionString(r, w, m, "0.0"));
    EXPECT_EQ(0, r);
    EXPECT_EQ(0, w);
    EXPECT_EQ(0, m);

    EC_EXPECT_SUCCESS(SchemaKey::ParseVersionString(r, w, m, "1.2"));
    EXPECT_EQ(1, r);
    EXPECT_EQ(0, w);
    EXPECT_EQ(2, m);

    EC_EXPECT_SUCCESS(SchemaKey::ParseVersionString(r, w, m, "0.0.0"));
    EXPECT_EQ(0, r);
    EXPECT_EQ(0, w);
    EXPECT_EQ(0, m);

    EC_EXPECT_SUCCESS(SchemaKey::ParseVersionString(r, w, m, "1.2.3"));
    EXPECT_EQ(1, r);
    EXPECT_EQ(2, w);
    EXPECT_EQ(3, m);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                      Kyle.Abramowitz           05/2018
//---------------+---------------+---------------+---------------+---------------+-----
TEST_F(SchemaKeyTest, TestParseSchemaVersionStringStrict)
    {
    uint32_t r;
    uint32_t w;
    uint32_t m;

    EXPECT_NE(ECObjectsStatus::Success, SchemaKey::ParseVersionStringStrict(r, w, m, ""));
    EXPECT_NE(ECObjectsStatus::Success, SchemaKey::ParseVersionStringStrict(r, w, m, "0"));
    EXPECT_NE(ECObjectsStatus::Success, SchemaKey::ParseVersionStringStrict(r, w, m, "0."));
    EXPECT_NE(ECObjectsStatus::Success, SchemaKey::ParseVersionStringStrict(r, w, m, ".0"));
    EXPECT_NE(ECObjectsStatus::Success, SchemaKey::ParseVersionStringStrict(r, w, m, "."));
    EXPECT_NE(ECObjectsStatus::Success, SchemaKey::ParseVersionStringStrict(r, w, m, ".."));
    EXPECT_NE(ECObjectsStatus::Success, SchemaKey::ParseVersionStringStrict(r, w, m, "0.0"));
    EXPECT_EQ(ECObjectsStatus::InvalidECVersion, SchemaKey::ParseVersionStringStrict(r, w, m, "1.2"));

    EC_EXPECT_SUCCESS(SchemaKey::ParseVersionStringStrict(r, w, m, "0.0.0"));
    EXPECT_EQ(0, r);
    EXPECT_EQ(0, w);
    EXPECT_EQ(0, m);

    EC_EXPECT_SUCCESS(SchemaKey::ParseVersionStringStrict(r, w, m, "1.2.3"));
    EXPECT_EQ(1, r);
    EXPECT_EQ(2, w);
    EXPECT_EQ(3, m);
    }

//=======================================================================================
//! SchemaCacheTest
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                  Raimondas.Rimkus 02/2013
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCacheTest, LoadAndGetSchema)
    {
    ECSchemaCachePtr cache = ECSchemaCache::Create();
    ECSchemaPtr schema1;
    ECSchemaPtr schema2;
    ECSchemaPtr schema3;

    ECSchema::CreateSchema(schema1, "Widget", "ts", 5, 0, 1);
    ECSchema::CreateSchema(schema2, "BaseSchema1", "ts", 2, 0, 0);
    ECSchema::CreateSchema(schema3, "BaseSchema2", "ts", 5, 0, 5);

    EXPECT_TRUE(cache->AddSchema(*schema1) == ECObjectsStatus::Success);
    EXPECT_TRUE(cache->AddSchema(*schema2) == ECObjectsStatus::Success);
    EXPECT_TRUE(cache->AddSchema(*schema3) == ECObjectsStatus::Success);
    EXPECT_EQ(cache->GetCount(), 3);

    ECSchemaPtr fetchedSchema = cache->GetSchema(SchemaKey("BaseSchema1", 2, 0));
    ASSERT_TRUE(fetchedSchema.IsValid());
    EXPECT_TRUE(fetchedSchema->GetSchemaKey() == SchemaKey("BaseSchema1", 2, 0));

    cache->Clear();
    EXPECT_EQ(cache->GetCount(), 0);
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                  Raimondas.Rimkus 02/2013
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCacheTest, FilterSchema)
    {
    ECSchemaCachePtr cache = ECSchemaCache::Create();
    ECSchemaPtr schema1;
    ECSchemaPtr schema2;
    ECSchemaPtr schema3;

    ECSchema::CreateSchema(schema1, "Widget", "ts", 5, 0, 1);
    ECSchema::CreateSchema(schema2, "BaseSchema1", "ts", 2, 0, 0);
    ECSchema::CreateSchema(schema3, "BaseSchema2", "ts", 5, 0, 5);
    
    EC_EXPECT_SUCCESS(cache->AddSchema(*schema1));
    EC_EXPECT_SUCCESS(cache->AddSchema(*schema2));
    EC_EXPECT_SUCCESS(cache->AddSchema(*schema3));

    EXPECT_NE(nullptr, cache->GetSchema(SchemaKey("BaseSchema1", 2, 0), SchemaMatchType::Exact));
    EXPECT_EQ(nullptr, cache->GetSchema(SchemaKey("BaseZchema1", 2, 0), SchemaMatchType::Exact));
    EXPECT_EQ(nullptr, cache->GetSchema(SchemaKey("BaseSchema1", 3, 0), SchemaMatchType::Exact));
    EXPECT_EQ(nullptr, cache->GetSchema(SchemaKey("BaseSchema1", 2, 1), SchemaMatchType::Exact));

    EXPECT_NE(nullptr, cache->GetSchema(SchemaKey("BaseSchema1", 2, 0), SchemaMatchType::Identical));
    EXPECT_EQ(nullptr, cache->GetSchema(SchemaKey("BaseZchema1", 2, 0), SchemaMatchType::Identical));
    EXPECT_EQ(nullptr, cache->GetSchema(SchemaKey("BaseSchema1", 3, 0), SchemaMatchType::Identical));
    EXPECT_EQ(nullptr, cache->GetSchema(SchemaKey("BaseSchema1", 2, 1), SchemaMatchType::Identical));

    EXPECT_NE(nullptr, cache->GetSchema(SchemaKey("BaseSchema1", 2, 0), SchemaMatchType::Latest));
    EXPECT_EQ(nullptr, cache->GetSchema(SchemaKey("BaseZchema1", 2, 0), SchemaMatchType::Latest));
    EXPECT_NE(nullptr, cache->GetSchema(SchemaKey("BaseSchema1", 3, 0), SchemaMatchType::Latest));
    EXPECT_NE(nullptr, cache->GetSchema(SchemaKey("BaseSchema1", 2, 1), SchemaMatchType::Latest));

    EXPECT_NE(nullptr, cache->GetSchema(SchemaKey("BaseSchema2", 5, 5), SchemaMatchType::LatestWriteCompatible));
    EXPECT_EQ(nullptr, cache->GetSchema(SchemaKey("BaseZchema2", 5, 5), SchemaMatchType::LatestWriteCompatible));
    EXPECT_EQ(nullptr, cache->GetSchema(SchemaKey("BaseSchema2", 3, 5), SchemaMatchType::LatestWriteCompatible));
    EXPECT_NE(nullptr, cache->GetSchema(SchemaKey("BaseSchema2", 5, 3), SchemaMatchType::LatestWriteCompatible));
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                  Raimondas.Rimkus 02/2013
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCacheTest, DropSchema)
    {
    ECSchemaCachePtr cache = ECSchemaCache::Create();
    ECSchemaPtr schema1;
    ECSchemaPtr schema2;
    ECSchemaPtr schema3;

    ECSchema::CreateSchema(schema1, "Widget", "ts", 5, 0, 1);
    ECSchema::CreateSchema(schema2, "BaseSchema1", "ts", 2, 0, 0);
    ECSchema::CreateSchema(schema3, "BaseSchema2", "ts", 5, 0, 5);

    EXPECT_TRUE(cache->AddSchema(*schema1) == ECObjectsStatus::Success);
    EXPECT_TRUE(cache->AddSchema(*schema2) == ECObjectsStatus::Success);
    EXPECT_TRUE(cache->AddSchema(*schema3) == ECObjectsStatus::Success);
    EXPECT_EQ(cache->GetCount(), 3);

    EXPECT_TRUE(cache->DropSchema(SchemaKey("Widget", 5, 1)) == ECObjectsStatus::Success);
    EXPECT_EQ(cache->GetCount(), 2);
    EXPECT_TRUE(cache->DropSchema(SchemaKey("Widget", 5, 1)) != ECObjectsStatus::Success);
    EXPECT_TRUE(cache->DropSchema(SchemaKey("BaseSchema2", 5, 3)) != ECObjectsStatus::Success);
    EXPECT_TRUE(cache->DropSchema(SchemaKey("BaseSchema2", 5, 7)) != ECObjectsStatus::Success);
    EXPECT_TRUE(cache->DropSchema(SchemaKey("BaseSchema2", 1, 5)) != ECObjectsStatus::Success);
    EXPECT_EQ(cache->GetCount(), 2);
    EXPECT_TRUE(cache->DropSchema(SchemaKey("BaseSchema2", 5, 5)) == ECObjectsStatus::Success);
    EXPECT_EQ(cache->GetCount(), 1);
    EXPECT_TRUE(cache->DropSchema(SchemaKey("BaseSchema1", 2, 0)) == ECObjectsStatus::Success);
    EXPECT_EQ(cache->GetCount(), 0);
    }

//=======================================================================================
//! SchemaChecksumTest
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaChecksumTest, RawSchemaXmlStringCheckSum)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="CoreCustomAttributes" version="1.00.00" alias="CoreCA"/>
            <ECEntityClass typeName="TestClass">
            </ECEntityClass>
        </ECSchema>)xml";
    
    Utf8String checksum = ECSchema::ComputeSchemaXmlStringCheckSum(schemaXml, Utf8String(schemaXml).length());
    EXPECT_TRUE(checksum.EqualsIAscii("dadb691ddd3c751b5de803094d00c610eb142dee"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Joseph.Urbano    04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaChecksumTest, ComputeCheckSumSameAsSerializedXmlStringCheckSum)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="CoreCustomAttributes" version="1.00.00" alias="CoreCA"/>
            <ECEntityClass typeName="TestClass">
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECVersion ecXmlVersion;
    EXPECT_EQ(ECObjectsStatus::Success, ECSchema::CreateECVersion(ecXmlVersion, schema->GetOriginalECXmlVersionMajor(), schema->GetOriginalECXmlVersionMinor()));
    EXPECT_EQ(ECVersion::V3_1, ecXmlVersion);

    Utf8String serializedXml;
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedXml, ecXmlVersion));
    Utf8String serializedXmlChecksum = ECSchema::ComputeSchemaXmlStringCheckSum(serializedXml.c_str(), sizeof(Utf8Char) * serializedXml.length());
    EXPECT_STREQ("85436c33a75b65ca8f4a5acee30ec7b73f525aa3", serializedXmlChecksum.ToLower().c_str());

    Utf8String checksum = schema->ComputeCheckSum();
    EXPECT_STREQ(serializedXmlChecksum.ToLower().c_str(), checksum.ToLower().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Gintaras.Volkvicius 11/2018
//---------------------------------------------------------------------------------------
TEST_F(SchemaChecksumTest, ComputingChecksumTakingIntoAccountSchemaXmlVersion)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="TestClass">
            </ECEntityClass>
        </ECSchema>)xml";

    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext));
    EXPECT_STREQ("1de2f27499158c8747d04ab45c36d2b57dd7e9f0", schema->ComputeCheckSum().ToLower().c_str());
    
    schema->SetOriginalECXmlVersion(3, 2);
    EXPECT_STREQ("1de2f27499158c8747d04ab45c36d2b57dd7e9f0", schema->ComputeCheckSum().ToLower().c_str());

    schema->SetOriginalECXmlVersion(3, 1);
    EXPECT_STREQ("f873d1d8223498533a2567a9badaf4bcd641350c", schema->ComputeCheckSum().ToLower().c_str());
    
    schema->SetOriginalECXmlVersion(3, 0);
    EXPECT_STREQ("58baeba5e12e50c623a3fa30d876c6ae3ac7f563", schema->ComputeCheckSum().ToLower().c_str());

    schema->SetOriginalECXmlVersion(0, 0); // when no schema xml version is found, it falls back to EC3.1
    EXPECT_STREQ("f873d1d8223498533a2567a9badaf4bcd641350c", schema->ComputeCheckSum().ToLower().c_str());
    }


//=======================================================================================
//! SchemaImmutableTest
//=======================================================================================

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaImmutableTest, SetImmutable)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    schema->SetImmutable();

    ECEntityClassP class1 = nullptr;
    ECClassP class2 = schema->GetClassP("ecProject");
    ECRelationshipClassP relationshipClass;
    ECClassP base = (ECClassP) class1;
    EXPECT_EQ(schema->CreateEntityClass(class1, "TestClass"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_TRUE(class1 == nullptr);
    EXPECT_EQ(schema->CopyClass(base, *class2), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->CreateRelationshipClass(relationshipClass, "RelationshipClass"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->SetName("Some new name"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->SetAlias("Some new alias"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->SetDescription("Some new description"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->SetDisplayLabel("Some new label"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->SetVersionRead(13), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->SetVersionMinor(13), ECObjectsStatus::SchemaIsImmutable);
    }

//=======================================================================================
//! SchemaVersionTest
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                     10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaVersionTest, CreateECVersionTest)
    {
    ECVersion ecVersion;
    EXPECT_EQ(ECObjectsStatus::InvalidECVersion, ECSchema::CreateECVersion(ecVersion, 0, 0)) << "Creating an ECVersion with invalid major and minor versions should fail.";

    EXPECT_EQ(ECObjectsStatus::InvalidECVersion, ECSchema::CreateECVersion(ecVersion, 9, 9)) << "Creating an ECVersion with invalid major and minor versions should fail.";

    EXPECT_EQ(ECObjectsStatus::Success, ECSchema::CreateECVersion(ecVersion, 2, 0)) << "Creating a 2.0 ECVersion should succeed";
    EXPECT_EQ(ECVersion::V2_0, ecVersion) << "The ECVersion should have been set to 2.0.";
    EXPECT_STREQ("2.0", ECSchema::GetECVersionString(ecVersion)) << "The string should be in the major.minor for the provided ECVersion.";

    ECVersion ecVersion3;
    EXPECT_EQ(ECObjectsStatus::Success, ECSchema::CreateECVersion(ecVersion3, 3, 0)) << "Creating a 2.0 ECVersion should succeed";
    EXPECT_EQ(ECVersion::V3_0, ecVersion3) << "The ECVersion should have been set to 3.0.";
    EXPECT_STREQ("3.0", ECSchema::GetECVersionString(ecVersion3)) << "The string should be in the major.minor for the provided ECVersion.";

    ECVersion ecVersion31;
    EXPECT_EQ(ECObjectsStatus::Success, ECSchema::CreateECVersion(ecVersion31, 3, 1)) << "Creating a 3.1 ECVersion should succeed";
    EXPECT_EQ(ECVersion::V3_1, ecVersion31) << "The ECVersion should have been set to 3.1.";
    EXPECT_STREQ("3.1", ECSchema::GetECVersionString(ecVersion31)) << "The string should be in the major.minor for the provided ECVersion.";

    ECVersion ecVersion32;
    EXPECT_EQ(ECObjectsStatus::Success, ECSchema::CreateECVersion(ecVersion32, 3, 2)) << "Creating a 3.2 ECVersion should succeed";
    EXPECT_EQ(ECVersion::V3_2, ecVersion32) << "The ECVersion should have been set to 3.2.";
    EXPECT_STREQ("3.2", ECSchema::GetECVersionString(ecVersion32)) << "The string should be in the major.minor for the provided ECVersion.";

    EXPECT_EQ(ECVersion::Latest, ecVersion32) << "ECVersion Latest should be equal to 3.2, therefore the comparsion should succeed.";

    EXPECT_STREQ("3.2", ECSchema::GetECVersionString(ECVersion::Latest)) << "ECVersion Latest should be equal to 3.2, therefore the string of it should be equal to 3.2.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                     10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaVersionTest, CreateSchemaECVersionTest)
    {
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    EXPECT_TRUE(schema->IsECVersion(ECVersion::V3_2)) << "The default schema ECVersion should be EC3.2 so this should validate to true.";
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest)) << "The default schema ECVersion should be the latest so this should validate to true.";
    }
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5, ECVersion::Latest);
    EXPECT_TRUE(schema->IsECVersion(ECVersion::V3_2)) << "The schema was created as the Latest version which is EC3.2 so this should validate to true.";
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest)) << "The schema was created as the Latest version so this should validate to true.";
    }
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5, ECVersion::V3_2);
    EXPECT_TRUE(schema->IsECVersion(ECVersion::V3_2)) << "The schema was created as an EC3.2 schema.";
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest)) << "The schema was created as an EC3.2 schema so Latest should return true.";
    }
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5, ECVersion::V3_1);
    EXPECT_TRUE(schema->IsECVersion(ECVersion::V3_1)) << "The schema was created as an EC3.1 schema.";
    EXPECT_FALSE(schema->IsECVersion(ECVersion::Latest)) << "The schema was created as an EC3.1 schema so it is not the latest.";
    }
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5, ECVersion::V3_0);
    EXPECT_TRUE(schema->IsECVersion(ECVersion::V3_0)) << "The schema was created as an EC3.0 schema.";
    EXPECT_FALSE(schema->IsECVersion(ECVersion::Latest)) << "The schema was created as an EC3.0 schema so it is not the latest.";
    }
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5, ECVersion::V2_0);
    EXPECT_TRUE(schema->IsECVersion(ECVersion::V2_0)) << "The schema was created as an EC2.0 schema.";
    EXPECT_FALSE(schema->IsECVersion(ECVersion::Latest)) << "The schema was created as an EC2.0 schema so it is not the latest";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                     06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaVersionTest, ChangeOriginalECXmlVersion)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
        </ECSchema>
        )xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(2, schema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(0, schema->GetOriginalECXmlVersionMinor());
    ASSERT_EQ(ECObjectsStatus::Success, schema->SetOriginalECXmlVersion(3, 1));
    EXPECT_EQ(3, schema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(1, schema->GetOriginalECXmlVersionMinor());
    }
    {
    ECSchemaPtr schema;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0));
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(3, schema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(1, schema->GetOriginalECXmlVersionMinor());
    ASSERT_EQ(ECObjectsStatus::Success, schema->SetOriginalECXmlVersion(2, 0));
    EXPECT_EQ(2, schema->GetOriginalECXmlVersionMajor());
    EXPECT_EQ(0, schema->GetOriginalECXmlVersionMinor());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     05/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaVersionTest, ShouldFailToDeserialize32AndGreaterWithoutCorrectVersion)
    {
    ExpectSchemaDeserializationSuccess(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>
        )xml");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>
        )xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize 3.2 schema with 2 part version");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
        </ECSchema>
        )xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize 3.3 schema with 2 part version");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     05/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaVersionTest, ReferenceSchemasShoudlUse3PartVersionsForGreaterThan32Schemas)
    {
    Utf8CP refSchemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="ref" alias="r" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>
        )xml";
    ExpectSchemaDeserializationSuccess(refSchemaXml);

    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ref" version="1.00.00" alias="r"/>
    </ECSchema>
    )xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    DeserializeSchema(schema, *context, SchemaItem(refSchemaXml));
    DeserializeSchema(schema, *context, SchemaItem(schemaXml));
    }

    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
        <ECSchemaReference name="ref" version="1.00.00" alias="r"/>
    </ECSchema>
    )xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    DeserializeSchema(schema, *context, SchemaItem(refSchemaXml));
    DeserializeSchema(schema, *context, SchemaItem(schemaXml));
    }

    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ref" version="1.00" alias="r"/>
    </ECSchema>
    )xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    DeserializeSchema(schema, *context, SchemaItem(refSchemaXml));
    DeserializeSchema(schema, *context, SchemaItem(schemaXml), SchemaReadStatus::InvalidECSchemaXml);
    }

    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
        <ECSchemaReference name="ref" version="1.00" alias="r"/>
    </ECSchema>
    )xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    DeserializeSchema(schema, *context, SchemaItem(refSchemaXml));
    DeserializeSchema(schema, *context, SchemaItem(schemaXml), SchemaReadStatus::InvalidECSchemaXml);
    }

    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ref" version="a.b.c" alias="r"/>
    </ECSchema>
    )xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    DeserializeSchema(schema, *context, SchemaItem(refSchemaXml));
    DeserializeSchema(schema, *context, SchemaItem(schemaXml), SchemaReadStatus::InvalidECSchemaXml);
    }

    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
        <ECSchemaReference name="ref" version="a.b.c" alias="r"/>
    </ECSchema>
    )xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    DeserializeSchema(schema, *context, SchemaItem(refSchemaXml));
    DeserializeSchema(schema, *context, SchemaItem(schemaXml), SchemaReadStatus::InvalidECSchemaXml);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     05/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaVersionTest, CustomAttributeNameSpacesMustUseCorrectVersions)
    {

    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
    "<ECSchema schemaName='CaseInsensitive' version='01.00' displayLabel='Case Insensitive' description='Testing case sensitivity with struct names and custom attributes' alias='cs' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
    "    <ECCustomAttributeClass typeName='CustomAttrib' description='CustomAttribute' displayLabel='CA' />"
    "    <ECStructClass typeName='MyStruct'>"
    "        <ECProperty propertyName='Prop1' typeName='string' />"
    "    </ECStructClass>"
    "    <ECEntityClass typeName='Entity'>"
    "        <ECCustomAttributes>"
    "            <Customattrib xmlns='CaseInsensitive.01.00' />"
    "        </ECCustomAttributes>"
    "        <ECStructProperty propertyName='StructProp' typeName='Mystruct' />"
    "    </ECEntityClass>"
    "</ECSchema>";
    ExpectSchemaDeserializationSuccess(schemaXML, "Should successfully deserialize a 3.1 schema with 2 part versions");
    }

    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
    "<ECSchema schemaName='CaseInsensitive' version='01.00.00' displayLabel='Case Insensitive' description='Testing case sensitivity with struct names and custom attributes' alias='cs' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
    "    <ECCustomAttributeClass typeName='CustomAttrib' description='CustomAttribute' displayLabel='CA' />"
    "    <ECStructClass typeName='MyStruct'>"
    "        <ECProperty propertyName='Prop1' typeName='string' />"
    "    </ECStructClass>"
    "    <ECEntityClass typeName='Entity'>"
    "        <ECCustomAttributes>"
    "            <Customattrib xmlns='CaseInsensitive.01.00' />"
    "        </ECCustomAttributes>"
    "        <ECStructProperty propertyName='StructProp' typeName='Mystruct' />"
    "    </ECEntityClass>"
    "</ECSchema>";
    ExpectSchemaDeserializationFailure(schemaXML, SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize 3.2 schema with 2 part version in xmlns");
    }

    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
    "<ECSchema schemaName='CaseInsensitive' version='01.00.00' displayLabel='Case Insensitive' description='Testing case sensitivity with struct names and custom attributes' alias='cs' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.3'>"
    "    <ECCustomAttributeClass typeName='CustomAttrib' description='CustomAttribute' displayLabel='CA' />"
    "    <ECStructClass typeName='MyStruct'>"
    "        <ECProperty propertyName='Prop1' typeName='string' />"
    "    </ECStructClass>"
    "    <ECEntityClass typeName='Entity'>"
    "        <ECCustomAttributes>"
    "            <Customattrib xmlns='CaseInsensitive.01.00' />"
    "        </ECCustomAttributes>"
    "        <ECStructProperty propertyName='StructProp' typeName='Mystruct' />"
    "    </ECEntityClass>"
    "</ECSchema>";
    ExpectSchemaDeserializationFailure(schemaXML, SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize 3.3 schema with 2 part version in xmlns");
    }

    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
    "<ECSchema schemaName='CaseInsensitive' version='01.00.00' displayLabel='Case Insensitive' description='Testing case sensitivity with struct names and custom attributes' alias='cs' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
    "    <ECCustomAttributeClass typeName='CustomAttrib' description='CustomAttribute' displayLabel='CA' />"
    "    <ECStructClass typeName='MyStruct'>"
    "        <ECProperty propertyName='Prop1' typeName='string' />"
    "    </ECStructClass>"
    "    <ECEntityClass typeName='Entity'>"
    "        <ECCustomAttributes>"
    "            <Customattrib xmlns='CaseInsensitive.01.00.00' />"
    "        </ECCustomAttributes>"
    "        <ECStructProperty propertyName='StructProp' typeName='Mystruct' />"
    "    </ECEntityClass>"
    "</ECSchema>";
    ExpectSchemaDeserializationSuccess(schemaXML, "Should successfully deserialize a 3.2 schema with 3 part versions");
    }

    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
    "<ECSchema schemaName='CaseInsensitive' version='01.00.00' displayLabel='Case Insensitive' description='Testing case sensitivity with struct names and custom attributes' alias='cs' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.3'>"
    "    <ECCustomAttributeClass typeName='CustomAttrib' description='CustomAttribute' displayLabel='CA' />"
    "    <ECStructClass typeName='MyStruct'>"
    "        <ECProperty propertyName='Prop1' typeName='string' />"
    "    </ECStructClass>"
    "    <ECEntityClass typeName='Entity'>"
    "        <ECCustomAttributes>"
    "            <Customattrib xmlns='CaseInsensitive.01.00.00' />"
    "        </ECCustomAttributes>"
    "        <ECStructProperty propertyName='StructProp' typeName='Mystruct' />"
    "    </ECEntityClass>"
    "</ECSchema>";
    ExpectSchemaDeserializationSuccess(schemaXML, "Should successfully deserialize a 3.3 schema with 3 part versions");
    }
    }

//=======================================================================================
//! SchemaElementsOrderTest
//=======================================================================================

bvector<bpair<Utf8CP, ECSchemaElementType>> getTestElements()
    {
    // return elements in expected order if created alphabetically
    bvector<bpair<Utf8CP, ECSchemaElementType>> elements;

    elements.push_back(make_bpair<Utf8CP, ECSchemaElementType>("aEnum", ECSchemaElementType::ECEnumeration));
    elements.push_back(make_bpair<Utf8CP, ECSchemaElementType>("bEnum", ECSchemaElementType::ECEnumeration));
    elements.push_back(make_bpair<Utf8CP, ECSchemaElementType>("cEnum", ECSchemaElementType::ECEnumeration));
    elements.push_back(make_bpair<Utf8CP, ECSchemaElementType>("aClass", ECSchemaElementType::ECClass));
    elements.push_back(make_bpair<Utf8CP, ECSchemaElementType>("bClass", ECSchemaElementType::ECClass));
    elements.push_back(make_bpair<Utf8CP, ECSchemaElementType>("cClass", ECSchemaElementType::ECClass));
    elements.push_back(make_bpair<Utf8CP, ECSchemaElementType>("aKOQ", ECSchemaElementType::KindOfQuantity));
    elements.push_back(make_bpair<Utf8CP, ECSchemaElementType>("bKOQ", ECSchemaElementType::KindOfQuantity));
    elements.push_back(make_bpair<Utf8CP, ECSchemaElementType>("aPropertyCategory", ECSchemaElementType::PropertyCategory));
    elements.push_back(make_bpair<Utf8CP, ECSchemaElementType>("bPropertyCategory", ECSchemaElementType::PropertyCategory));

    return elements;
    }

ECSchemaPtr generateSchemaFromElements(const bvector<bpair<Utf8CP, ECSchemaElementType>> &elements)
    {
    ECSchemaPtr schema;

    EXPECT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestSchema", "TS", 1, 1, 1));

    for (auto element : elements)
        {
        switch (element.second)
            {
                case ECSchemaElementType::ECEnumeration:
                    ECEnumerationP e;
                    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateEnumeration(e, element.first, PrimitiveType::PRIMITIVETYPE_Integer));
                    break;
                case ECSchemaElementType::ECClass:
                    ECEntityClassP c;
                    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(c, element.first));
                    break;
                case ECSchemaElementType::KindOfQuantity:
                    KindOfQuantityP k;
                    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(k, element.first));
                    break;
                case ECSchemaElementType::PropertyCategory:
                    PropertyCategoryP p;
                    EXPECT_EQ(ECObjectsStatus::Success, schema->CreatePropertyCategory(p, element.first));
                    break;
            }
        }
    
    return schema;
    }

void generateSchemaElementsOrder(ECSchemaElementsOrder &order, const bvector<bpair<Utf8CP, ECSchemaElementType>> &elements, bool reverse = false)
    {
    if (reverse)
        {
        bvector<bpair<Utf8CP, ECSchemaElementType>> reversed;
        reversed.resize(elements.size());
        std::reverse_copy(elements.begin(), elements.end(), reversed.begin());
        
        for (const auto pair : reversed)
            {
            order.AddElement(pair.first, pair.second);
            }
        }
    else
        {
        for (const auto pair : elements)
            {
            order.AddElement(pair.first, pair.second);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Joseph.Urbano                     05/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaElementsOrderTest, PreserveElementOrder)
    {
    bvector<bpair<Utf8CP, ECSchemaElementType>> elements;
    ECSchemaElementsOrder order, orderReversed;
    ECSchemaPtr schema;
    
    elements = getTestElements();
    
    int i = 0;

    // Create schema element order with elements in order
    generateSchemaElementsOrder(order, elements, false);
    for (auto entry : order)
        {
        EXPECT_TRUE(entry.first.EqualsIAscii(elements[i++].first));
        }

    // Create schema element order with elements in reverse order
    generateSchemaElementsOrder(orderReversed, elements, true);
    for (auto entry : orderReversed)
        {
        EXPECT_TRUE(entry.first.EqualsIAscii(elements[--i].first));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Joseph.Urbano                     05/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaElementsOrderTest, CreateAlphabeticalOrder)
    {
    bvector<bpair<Utf8CP, ECSchemaElementType>> elements;
    ECSchemaElementsOrder order;
    ECSchemaPtr schema;

    elements = getTestElements();

    // Generate schema from the elements in reversed alphabetical order
    bvector<bpair<Utf8CP, ECSchemaElementType>> reversedElements;
    reversedElements.resize(elements.size());
    std::reverse_copy(elements.begin(), elements.end(), reversedElements.begin());

    schema = generateSchemaFromElements(reversedElements);

    int i = 0;
    
    // create schema element order from schema and make sure they're in alphabetical order
    order.CreateAlphabeticalOrder(*schema);
    for (auto entry : order)
        {
        EXPECT_TRUE(entry.first.EqualsIAscii(elements[i++].first));
        }
    }

END_BENTLEY_ECN_TEST_NAMESPACE
