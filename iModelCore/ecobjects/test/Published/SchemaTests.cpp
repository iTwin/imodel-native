/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SchemaTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
struct SchemaSerializationTest : ECTestFixture {};
struct SchemaReferenceTest : ECTestFixture {};
struct SchemaCreationTest : ECTestFixture {};
struct SchemaCopyTest : ECTestFixture {};
struct SchemaLocateTest : ECTestFixture {};
struct SchemaComparisonTest : ECTestFixture {};
struct SchemaCacheTest : ECTestFixture {};
struct SchemaChecksumTest : ECTestFixture {};
struct SchemaImmutableTest : ECTestFixture {};
struct SchemaVersionTest : ECTestFixture {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP GetPropertyByName(ECClassCR ecClass, Utf8CP name, bool expectExists = true)
    {
    ECPropertyP prop = ecClass.GetPropertyP(name);
    EXPECT_EQ(expectExists, NULL != prop);
    Utf8String utf8(name);
    prop = ecClass.GetPropertyP(utf8.c_str());
    EXPECT_EQ(expectExists, NULL != prop);
    return prop;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaTest, ExpectReadOnly)
    {
    ECSchemaPtr schema;
    ECEntityClassP domainClass;
    ECEntityClassP derivedClass;
    ECStructClassP structClass;
    ECCustomAttributeClassP customAttributeClass;
    ECEnumerationP enumeration;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    ASSERT_TRUE(schema.IsValid());

    //Create Domain Class
    schema->CreateEntityClass(domainClass, "DomainClass");
    ASSERT_TRUE(domainClass != NULL);

    //Create Derived Class
    schema->CreateEntityClass(derivedClass, "DerivedClass");
    ASSERT_TRUE(derivedClass != NULL);

    //Create Struct
    schema->CreateStructClass(structClass, "StructClass");
    ASSERT_TRUE(structClass != NULL);

    //Create Enumeration
    schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_Integer);
    ASSERT_TRUE(enumeration != nullptr);

    //Add Property of Array type to structClass
    PrimitiveArrayECPropertyP MyArrayProp;
    structClass->CreatePrimitiveArrayProperty(MyArrayProp, "ArrayProperty");
    ASSERT_TRUE(MyArrayProp != NULL);

    //Create customAttributeClass
    schema->CreateCustomAttributeClass(customAttributeClass, "CustomAttribute");
    ASSERT_TRUE(customAttributeClass != NULL);

    //Add Property Of Struct type to custom attribute
    StructECPropertyP PropertyOfCustomAttribute;
    customAttributeClass->CreateStructProperty(PropertyOfCustomAttribute, "PropertyOfCustomAttribute", *structClass);
    ASSERT_TRUE(PropertyOfCustomAttribute != NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
    ASSERT_TRUE(enumeration != nullptr);
    ASSERT_TRUE(status == ECObjectsStatus::Success);

    status = schema->CreateEnumeration(enumeration2, "Enumeration", PrimitiveType::PRIMITIVETYPE_String);
    ASSERT_TRUE(enumeration2 == nullptr);
    ASSERT_TRUE(status == ECObjectsStatus::NamedItemAlreadyExists);

    status = schema->CreateEntityClass(domainClass, "Enumeration");
    ASSERT_TRUE(domainClass == nullptr);
    ASSERT_TRUE(status == ECObjectsStatus::NamedItemAlreadyExists);

    enumeration2 = schema->GetEnumerationP("Enumeration");
    ASSERT_TRUE(enumeration2 != nullptr);
    ASSERT_TRUE(enumeration2 == enumeration);

    int i = 0;
    for (auto p : schema->GetEnumerations())
        {
        i++;
        ASSERT_TRUE(p != nullptr);
        ASSERT_TRUE(p == enumeration);
        }

    ASSERT_TRUE(i == 1);

    ASSERT_TRUE(schema->GetEnumerationCount() == 1);

    ASSERT_TRUE(schema->DeleteEnumeration(*enumeration) == ECObjectsStatus::Success);

    enumeration2 = nullptr;
    enumeration2 = schema->GetEnumerationP("Enumeration");
    ASSERT_TRUE(enumeration2 == nullptr);

    ASSERT_TRUE(schema->GetEnumerationCount() == 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaTest, TestPrimitiveEnumerationProperty)
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
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaTest, RemoveBaseClassFromGrandChild)
    {
    Utf8CP schemaXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"TestSchema\" version=\"01.00\" displayLabel=\"TestSchema\" description=\"Test Schema\" nameSpacePrefix=\"ts\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" >"
        "    <ECClass typeName=\"GrandParent\" description=\"The base class\" displayLabel=\"GrandParent\" isDomainClass=\"True\">"
        "       <ECProperty propertyName=\"PropA\" typeName=\"string\" displayLabel=\"PropA\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"Parent\" isDomainClass=\"True\">"
        "        <BaseClass>GrandParent</BaseClass>"
        "       <ECProperty propertyName=\"PropA\" typeName=\"string\" displayLabel=\"PropA\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"Child\" isDomainClass=\"True\">"
        "        <BaseClass>Parent</BaseClass>"
        "       <ECProperty propertyName=\"PropA\" typeName=\"string\" displayLabel=\"PropA\" />"
        "    </ECClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECClassP child = schema->GetClassP("Child");
    ECPropertyP prop = child->GetPropertyP("PropA", false);
    ASSERT_TRUE(nullptr != prop);
    prop = child->GetPropertyP("PropA", true);
    ASSERT_TRUE(nullptr != prop);

    ECClassP parent = schema->GetClassP("Parent");
    child->RemoveBaseClass(*parent);
    ECBaseClassesList baseClasses = child->GetBaseClasses();
    ASSERT_TRUE(baseClasses.empty());
    prop = child->GetPropertyP("PropA", false);
    ASSERT_TRUE(nullptr != prop);
    ASSERT_TRUE(nullptr == prop->GetBaseProperty());  // This shouldn't fail!

    }

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
    ECSchemaPtr schema = ECSchema::LocateSchema(ECTestFixture::GetTestDataPath(L"Widgets.05.10.ecschema").c_str(), *schemaContext);
    ASSERT_FALSE(schema.IsValid());
    }

    // Test failure when schema xml file does not exist
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema = ECSchema::LocateSchema(ECTestFixture::GetTestDataPath(L"Widgets.05.10.ecschema.xml").c_str(), *schemaContext);
    ASSERT_FALSE(schema.IsValid());
    }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static  void    ValidateSchemaNameParsing(Utf8CP fullName, bool expectFailure, Utf8CP expectName, uint32_t expectRead, uint32_t expectWrite, uint32_t expectMinor)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaNameParsingTest, ParseFullSchemaName)
    {
    ValidateSchemaNameParsing("TestName.6.8", false, "TestName", 6, 0, 8);
    ValidateSchemaNameParsing("TestName.16.18", false, "TestName", 16, 0, 18);
    ValidateSchemaNameParsing("TestName.126.128", false, "TestName", 126, 0, 128);
    ValidateSchemaNameParsing("TestName.1267.128", false, "TestName", 1267, 0, 128);
    ValidateSchemaNameParsing("TestName.1267", true, NULL, 0, 0, 0);
    ValidateSchemaNameParsing("TestName", true, NULL, 0, 0, 0);
    ValidateSchemaNameParsing("", true, NULL, 0, 0, 0);
    ValidateSchemaNameParsing("12.18", true, NULL, 0, 0, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaSerializationTest, ExpectSuccessWithSerializingBaseClasses)
    {
    ECSchemaPtr schema;
    ECSchemaPtr schema2;
    ECSchemaPtr schema3;

    ECSchema::CreateSchema(schema, "Widget", "ecw", 5, 5, 5);
    ECSchema::CreateSchema(schema2, "BaseSchema", "base", 5, 5, 5);
    ECSchema::CreateSchema(schema3, "BaseSchema2", "base", 5, 5, 5);

    ECEntityClassP class1;
    ECEntityClassP baseClass;
    ECEntityClassP anotherBase;
    ECEntityClassP gadget;
    ECEntityClassP bolt;
    ECEnumerationP enumeration;
    schema->CreateEntityClass(class1, "TestClass");
    schema->CreateEntityClass(gadget, "Gadget");
    schema->CreateEntityClass(bolt, "Bolt");
    schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_Integer);
    enumeration->SetDisplayLabel("This is a display label.");
    enumeration->SetDescription("This is a description.");
    ECEnumeratorP enumerator;
    EXPECT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumerator, 1));
    enumerator->SetDisplayLabel("First");
    EXPECT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumerator, 2));
    enumerator->SetDisplayLabel("Second");
    EXPECT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumerator, 3));
    enumerator->SetDisplayLabel("Third");

    PrimitiveECPropertyP prop;
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreateEnumerationProperty(prop, "EnumeratedProperty", *enumeration));

    schema2->CreateEntityClass(baseClass, "BaseClass");
    schema3->CreateEntityClass(anotherBase, "AnotherBase");

    EXPECT_EQ(ECObjectsStatus::SchemaNotFound, class1->AddBaseClass(*baseClass));
    schema->AddReferencedSchema(*schema2);
    schema->AddReferencedSchema(*schema3);
    EXPECT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*baseClass));
    EXPECT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*anotherBase));
    EXPECT_EQ(ECObjectsStatus::Success, gadget->AddBaseClass(*class1));

    SchemaWriteStatus status2 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(L"base.xml").c_str());
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    status2 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(L"base_ec3.xml").c_str(), ECVersion::V3_1);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    WString ecSchemaXmlString;
    SchemaWriteStatus status3 = schema->WriteToXmlString(ecSchemaXmlString);
    EXPECT_EQ(SchemaWriteStatus::Success, status3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
// @bsimethod                                   
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaSerializationTest, SerializeComprehensiveSchema)
    {
    //Load Bentley_Standard_CustomAttributes
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back(ECTestFixture::GetTestDataPath(L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
    schemaContext->AddSchemaLocater(*schemaLocater);

    SchemaKey schemaKey("Bentley_Standard_CustomAttributes", 1, 12);
    ECSchemaPtr standardCASchema = schemaContext->LocateSchema(schemaKey, SchemaMatchType::Latest);
    EXPECT_TRUE(standardCASchema.IsValid());

    //Compose our new schema
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ComprehensiveSchema", "cmpr", 1, 5, 2);
    schema->SetDescription("Comprehensive Schema to demonstrate use of all ECSchema concepts.");
    schema->SetDisplayLabel("Comprehensive Schema");
    schema->AddReferencedSchema(*standardCASchema);

    ECEntityClassP baseEntityClass;
    ECEntityClassP entityClass;
    ECStructClassP structClass;
    ECCustomAttributeClassP classCustomAttributeClass;
    ECCustomAttributeClassP generalCustomAttributeClass;
    ECEnumerationP enumeration;

    schema->CreateEntityClass(baseEntityClass, "BaseEntity");
    PrimitiveECPropertyP inheritedPrimitiveProperty;
    baseEntityClass->CreatePrimitiveProperty(inheritedPrimitiveProperty, "InheritedProperty", PrimitiveType::PRIMITIVETYPE_String);
    baseEntityClass->SetClassModifier(ECClassModifier::Abstract);
    baseEntityClass->SetDisplayLabel("Base Entity");
    baseEntityClass->SetDescription("Base Entity Description");

    schema->CreateEntityClass(entityClass, "Entity");
    entityClass->SetClassModifier(ECClassModifier::Sealed);
    entityClass->AddBaseClass(*baseEntityClass);
    PrimitiveECPropertyP primitiveProperty1;
    entityClass->CreatePrimitiveProperty(primitiveProperty1, "Primitive1", PrimitiveType::PRIMITIVETYPE_Binary);
    primitiveProperty1->SetDisplayLabel("Property Display Label");
    PrimitiveECPropertyP primitiveProperty2;
    entityClass->CreatePrimitiveProperty(primitiveProperty2, "Primitive2", PrimitiveType::PRIMITIVETYPE_Boolean);
    primitiveProperty2->SetDescription("Property Description");
    PrimitiveECPropertyP primitiveProperty3;
    entityClass->CreatePrimitiveProperty(primitiveProperty3, "Primitive3", PrimitiveType::PRIMITIVETYPE_DateTime);
    primitiveProperty3->SetIsReadOnly(true);
    PrimitiveECPropertyP primitiveProperty4;
    entityClass->CreatePrimitiveProperty(primitiveProperty4, "Primitive4", PrimitiveType::PRIMITIVETYPE_Double);
    PrimitiveECPropertyP primitiveProperty5;
    entityClass->CreatePrimitiveProperty(primitiveProperty5, "Primitive5", PrimitiveType::PRIMITIVETYPE_IGeometry);
    PrimitiveECPropertyP primitiveProperty6;
    entityClass->CreatePrimitiveProperty(primitiveProperty6, "Primitive6", PrimitiveType::PRIMITIVETYPE_Integer);
    PrimitiveECPropertyP primitiveProperty7;
    entityClass->CreatePrimitiveProperty(primitiveProperty7, "Primitive7", PrimitiveType::PRIMITIVETYPE_Long);
    PrimitiveECPropertyP primitiveProperty8;
    entityClass->CreatePrimitiveProperty(primitiveProperty8, "Primitive8", PrimitiveType::PRIMITIVETYPE_Point2d);
    PrimitiveECPropertyP primitiveProperty9;
    entityClass->CreatePrimitiveProperty(primitiveProperty9, "Primitive9", PrimitiveType::PRIMITIVETYPE_Point3d);
    PrimitiveECPropertyP primitiveProperty10;
    entityClass->CreatePrimitiveProperty(primitiveProperty10, "Primitive10", PrimitiveType::PRIMITIVETYPE_String);
    PrimitiveECPropertyP calculatedProperty;
    entityClass->CreatePrimitiveProperty(calculatedProperty, "Calculated", PrimitiveType::PRIMITIVETYPE_String);
    PrimitiveArrayECPropertyP arrayProperty;
    entityClass->CreatePrimitiveArrayProperty(arrayProperty, "Array", PrimitiveType::PRIMITIVETYPE_Long);

    ECClassCP calcSpecClass = standardCASchema->GetClassCP("CalculatedECPropertySpecification");
    IECInstancePtr calcSpecAttr = calcSpecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue v;
    v.SetUtf8CP("\"Primitve 10=\" & this.Primitive10");
    calcSpecAttr->SetValue("ECExpression", v);
    calculatedProperty->SetCalculatedPropertySpecification(calcSpecAttr.get());

    schema->CreateStructClass(structClass, "Struct");
    structClass->SetDisplayLabel("Struct Class");
    PrimitiveECPropertyP structPrimitive1;
    structClass->CreatePrimitiveProperty(structPrimitive1, "Primitive1", PrimitiveType::PRIMITIVETYPE_Integer);
    StructECPropertyP structProperty;
    entityClass->CreateStructProperty(structProperty, "Struct1", *structClass);

    StructArrayECPropertyP structArrayProperty;
    entityClass->CreateStructArrayProperty(structArrayProperty, "StructArray", *structClass);

    schema->CreateCustomAttributeClass(classCustomAttributeClass, "ClassCustomAttribute");
    classCustomAttributeClass->SetDescription("Custom Attribute that can only be applied to classes.");
    classCustomAttributeClass->SetContainerType(CustomAttributeContainerType::AnyClass);
    PrimitiveECPropertyP classCustomAttributeProperty;
    classCustomAttributeClass->CreatePrimitiveProperty(classCustomAttributeProperty, "Primitive", PrimitiveType::PRIMITIVETYPE_String);
    IECInstancePtr classCA = classCustomAttributeClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue cV;
    cV.SetUtf8CP("General Value on Class");
    classCA->SetValue("Primitive", cV);
    entityClass->SetCustomAttribute(*classCA);

    schema->CreateCustomAttributeClass(generalCustomAttributeClass, "GeneralCustomAttribute");
    generalCustomAttributeClass->SetDescription("Custom Attribute that can be applied to anything.");
    PrimitiveECPropertyP generalCustomAttributeProperty;
    generalCustomAttributeClass->CreatePrimitiveProperty(generalCustomAttributeProperty, "Primitive", PrimitiveType::PRIMITIVETYPE_String);
    IECInstancePtr generalCA = generalCustomAttributeClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue gV;
    gV.SetUtf8CP("General Value");
    generalCA->SetValue("Primitive", gV);
    schema->SetCustomAttribute(*generalCA);

    schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_Integer);
    enumeration->SetDisplayLabel("This is a display label.");
    enumeration->SetDescription("This is a description.");
    ECEnumeratorP enumerator;
    EXPECT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumerator, 1));
    enumerator->SetDisplayLabel("First");
    EXPECT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumerator, 2));
    enumerator->SetDisplayLabel("Second");
    EXPECT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumerator, 3));
    enumerator->SetDisplayLabel("Third");

    PrimitiveECPropertyP prop;
    EXPECT_EQ(ECObjectsStatus::Success, entityClass->CreateEnumerationProperty(prop, "Enumerated", *enumeration));

    ECRelationshipClassP relationshipClass;
    schema->CreateRelationshipClass(relationshipClass, "RelationshipClass");
    PrimitiveECPropertyP relationshipProperty;
    relationshipClass->CreatePrimitiveProperty(relationshipProperty, "RelationshipProperty");
    relationshipClass->SetStrength(StrengthType::Referencing);
    relationshipClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationshipClass->GetSource().AddClass(*entityClass);
    relationshipClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relationshipClass->GetTarget().AddClass(*entityClass);
    relationshipClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroOne());

    NavigationECPropertyP navProp;
    entityClass->CreateNavigationProperty(navProp, "NavigationProperty", *relationshipClass, ECRelatedInstanceDirection::Forward);

    KindOfQuantityP kindOfQuantity;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(kindOfQuantity, "MyKindOfQuantity"));
    kindOfQuantity->SetDescription("Kind of a Description here");
    kindOfQuantity->SetDisplayLabel("best quantity of all times");
    kindOfQuantity->SetPersistenceUnit("CM");
    kindOfQuantity->SetRelativeError(10e-3);
    kindOfQuantity->SetDefaultPresentationUnit("FT");
    kindOfQuantity->AddPresentationUnit("IN");
    kindOfQuantity->AddPresentationUnit("MILLIINCH");

    WString fullSchemaName;
    fullSchemaName.AssignUtf8(schema->GetFullSchemaName().c_str());
    fullSchemaName.append(L".ecschema.xml");

    WString legacyFullSchemaName;
    legacyFullSchemaName.AssignUtf8(schema->GetLegacyFullSchemaName().c_str());
    legacyFullSchemaName.append(L".ecschema.xml");

    SchemaWriteStatus status2 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(fullSchemaName.c_str()).c_str(), ECVersion::V3_1);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    SchemaWriteStatus status3 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(legacyFullSchemaName.c_str()).c_str(), ECVersion::V2_0);
    EXPECT_EQ(SchemaWriteStatus::Success, status3);
    }

//This test ensures we support any unknown element or attribute put into existing ECSchema XML. Important for backwards compatibility of future EC versions.
TEST_F(SchemaSerializationTest, DeserializeComprehensiveSchemaWithUnknowns)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"ComprehensiveSchemaWithUnknowns.01.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "Failed to load ComprehensiveSchemaWithUnknowns for test";

    EXPECT_EQ(6, schema->GetClassCount());
    EXPECT_EQ(1, schema->GetEnumerationCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaSerializationTest, ExpectSuccessWithInheritedKindOfQuantities)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "testSchema", "ts", 1, 0, 0);
    schema->SetDescription("Schema to test Kind of Quantity Inheritance serialization.");
    schema->SetDisplayLabel("KOQ Inheritance Test Schema");

    ECEntityClassP parentEntityClass;
    ECEntityClassP derivedEntityClass1;
    ECEntityClassP derivedEntityClass2;
    ECEntityClassP derivedEntityClass3;
    KindOfQuantityP kindOfQuantity;
    KindOfQuantityP kindOfQuantity2;
    
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(kindOfQuantity, "MyKindOfQuantity"));
    kindOfQuantity->SetDescription("Kind of a Description here");
    kindOfQuantity->SetDisplayLabel("best quantity of all times");
    kindOfQuantity->SetPersistenceUnit("CM");
    kindOfQuantity->SetRelativeError(10e-3);
    kindOfQuantity->SetDefaultPresentationUnit("FT");
    kindOfQuantity->AddPresentationUnit("IN");
    kindOfQuantity->AddPresentationUnit("MILLIINCH");

    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(kindOfQuantity2, "OverrideKindOfQuantity"));
    kindOfQuantity2->SetDescription("Kind of a Description here");
    kindOfQuantity2->SetDisplayLabel("best quantity of all times");
    kindOfQuantity2->SetPersistenceUnit("CM");
    kindOfQuantity2->SetRelativeError(10e-4);
    kindOfQuantity2->SetDefaultPresentationUnit("FT");
    kindOfQuantity2->AddPresentationUnit("IN");
    kindOfQuantity2->AddPresentationUnit("MILLIINCH");

    schema->CreateEntityClass(parentEntityClass, "ParentEntity");
    parentEntityClass->SetClassModifier(ECClassModifier::Abstract);
    parentEntityClass->SetDisplayLabel("Parent Entity");
    parentEntityClass->SetDescription("Parent Entity Description");
    PrimitiveECPropertyP parentPrimitiveProperty;
    parentEntityClass->CreatePrimitiveProperty(parentPrimitiveProperty, "InheritedProperty", PrimitiveType::PRIMITIVETYPE_String);
    parentPrimitiveProperty->SetKindOfQuantity(kindOfQuantity);

    schema->CreateEntityClass(derivedEntityClass1, "DerivedEntity1");
    derivedEntityClass1->AddBaseClass(*parentEntityClass);
    derivedEntityClass1->SetClassModifier(ECClassModifier::Abstract);
    derivedEntityClass1->SetDisplayLabel("Derived Entity 1");
    derivedEntityClass1->SetDescription("Derived Entity Description");
    PrimitiveECPropertyP derivedPrimitiveProperty1;
    derivedEntityClass1->CreatePrimitiveProperty(derivedPrimitiveProperty1, "InheritedProperty", PrimitiveType::PRIMITIVETYPE_String);
    derivedPrimitiveProperty1->SetBaseProperty(parentPrimitiveProperty);

    schema->CreateEntityClass(derivedEntityClass2, "DerivedEntity2");
    derivedEntityClass2->AddBaseClass(*derivedEntityClass1);
    derivedEntityClass2->SetClassModifier(ECClassModifier::Sealed);
    derivedEntityClass2->SetDisplayLabel("Derived Entity 2");
    derivedEntityClass2->SetDescription("Derived Entity Description");
    PrimitiveECPropertyP derivedPrimitiveProperty2;
    derivedEntityClass2->CreatePrimitiveProperty(derivedPrimitiveProperty2, "InheritedProperty", PrimitiveType::PRIMITIVETYPE_String);
    derivedPrimitiveProperty2->SetBaseProperty(derivedPrimitiveProperty1);

    schema->CreateEntityClass(derivedEntityClass3, "DerivedEntity3");
    derivedEntityClass3->AddBaseClass(*derivedEntityClass1);
    derivedEntityClass3->SetClassModifier(ECClassModifier::Sealed);
    derivedEntityClass3->SetDisplayLabel("Derived Entity 3");
    derivedEntityClass3->SetDescription("Derived Entity Description");
    PrimitiveECPropertyP derivedPrimitiveProperty3;
    derivedEntityClass3->CreatePrimitiveProperty(derivedPrimitiveProperty3, "InheritedProperty", PrimitiveType::PRIMITIVETYPE_String);
    derivedPrimitiveProperty3->SetBaseProperty(derivedPrimitiveProperty1);
    derivedPrimitiveProperty3->SetKindOfQuantity(kindOfQuantity2);

    SchemaWriteStatus writeStatus = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(L"InheritedKOQ.01.00.00.ecschema.xml").c_str());
    ASSERT_EQ(SchemaWriteStatus::Success, writeStatus);
    
    ECSchemaPtr readSchema; 
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus readStatus = ECSchema::ReadFromXmlFile(readSchema, ECTestFixture::GetTempDataPath(L"InheritedKOQ.01.00.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, readStatus);
    ASSERT_TRUE(readSchema.IsValid());

    PrimitiveECPropertyCP parentProp = readSchema->GetClassCP("ParentEntity")->GetPropertyP("InheritedProperty", false)->GetAsPrimitiveProperty();
    ASSERT_TRUE(parentProp != nullptr);
    ASSERT_TRUE(parentProp->IsKindOfQuantityDefinedLocally());
    ASSERT_STREQ("MyKindOfQuantity", parentProp->GetKindOfQuantity()->GetName().c_str());
    
    PrimitiveECPropertyCP derivedProp1 = readSchema->GetClassCP("DerivedEntity1")->GetPropertyP("InheritedProperty", false)->GetAsPrimitiveProperty();
    ASSERT_TRUE(derivedProp1 != nullptr);
    ASSERT_FALSE(derivedProp1->IsKindOfQuantityDefinedLocally());
    ASSERT_STREQ("MyKindOfQuantity", derivedProp1->GetKindOfQuantity()->GetName().c_str());

    PrimitiveECPropertyCP derivedProp2 = readSchema->GetClassCP("DerivedEntity2")->GetPropertyP("InheritedProperty", false)->GetAsPrimitiveProperty();
    ASSERT_TRUE(derivedProp2 != nullptr);
    ASSERT_FALSE(derivedProp2->IsKindOfQuantityDefinedLocally());
    ASSERT_STREQ("MyKindOfQuantity", derivedProp2->GetKindOfQuantity()->GetName().c_str());

    PrimitiveECPropertyCP derivedProp3 = readSchema->GetClassCP("DerivedEntity3")->GetPropertyP("InheritedProperty", false)->GetAsPrimitiveProperty();
    ASSERT_TRUE(derivedProp3 != nullptr);
    ASSERT_TRUE(derivedProp3->IsKindOfQuantityDefinedLocally());
    ASSERT_STREQ("OverrideKindOfQuantity", derivedProp3->GetKindOfQuantity()->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
    class1->CreateNavigationProperty(navProp, "NavProp", *navRelClass, ECRelatedInstanceDirection::Forward, PrimitiveType::PRIMITIVETYPE_Long, false);

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
* @bsimethod
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
    ECSchema::CreateSchema(refSchema, "RefSchema", "ts", 5, 0, 5);

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

    EXPECT_TRUE(refList.FindClassP(SchemaNameClassNamePair("RefSchema", "Source")) != NULL);
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
* @bsimethod
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCopyTest, ExpectSuccessWhenCopyingStructs)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECSchemaPtr copiedSchema;
    ECObjectsStatus status2 = schema->CopySchema(copiedSchema);
    EXPECT_EQ(ECObjectsStatus::Success, status2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCopyTest, CopySchemaWithEnumeration)
    {
    ECSchemaPtr schema;
    ECEnumerationP enumeration;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    //Create Enumeration
    auto status = schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_Integer);
    ASSERT_TRUE(enumeration != nullptr);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    EXPECT_TRUE(schema.IsValid());
    enumeration->SetDisplayLabel("My Display Label");

    ECSchemaPtr copiedSchema = NULL;
    schema->CopySchema(copiedSchema);
    EXPECT_TRUE(copiedSchema.IsValid());
    ECEnumerationP enumeration2 = copiedSchema->GetEnumerationP("Enumeration");
    ASSERT_TRUE(enumeration2 != nullptr);
    EXPECT_TRUE(enumeration2->GetType() == enumeration->GetType());
    EXPECT_TRUE(enumeration2 != enumeration); //ensure the object was copied and not just referenced
    EXPECT_STREQ(enumeration2->GetDisplayLabel().c_str(), enumeration->GetDisplayLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaCopyTest, CopySchemaWithPropertyCategory)
    {
    ECSchemaPtr schema;
    PropertyCategoryP propertyCategory;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    auto status = schema->CreatePropertyCategory(propertyCategory, "PropertyCategory");
    ASSERT_TRUE(propertyCategory != nullptr);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    EXPECT_TRUE(schema.IsValid());
    propertyCategory->SetDisplayLabel("My Display Label");
    propertyCategory->SetDescription("My Description");
    propertyCategory->SetPriority(3);

    ECSchemaPtr copiedSchema = nullptr;
    status = schema->CopySchema(copiedSchema);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    EXPECT_TRUE(copiedSchema.IsValid());

    PropertyCategoryCP copiedPropertyCategory = copiedSchema->GetPropertyCategoryCP("PropertyCategory");
    ASSERT_TRUE(copiedPropertyCategory != nullptr);
    EXPECT_TRUE(copiedPropertyCategory != propertyCategory); //ensure the object was copied and not just referenced
    EXPECT_TRUE(copiedPropertyCategory->GetPriority() == propertyCategory->GetPriority());
    EXPECT_STREQ(copiedPropertyCategory->GetDisplayLabel().c_str(), propertyCategory->GetDisplayLabel().c_str());
    EXPECT_STREQ(copiedPropertyCategory->GetDescription().c_str(), propertyCategory->GetDescription().c_str());
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
        EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(schemaXml));

        schema = NULL;
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

    template<typename T> void Compare(T const& target) const
        {
        EXPECT_FALSE(target.GetIsDisplayLabelDefined());
        EXPECT_TRUE(target.GetName().Equals(m_encodedName)) << "Name: Expected " << m_encodedName.c_str() << " Actual " << target.GetName().c_str();
        EXPECT_TRUE(target.GetDisplayLabel().Equals(m_name)) << "Label: Expected " << m_name.c_str() << " Actual " << target.GetDisplayLabel().c_str();
        }

    template<typename T> void CompareOverriddenLabel(T const& target, Utf8CP label) const
        {
        EXPECT_TRUE(target.GetIsDisplayLabelDefined());
        EXPECT_TRUE(target.GetDisplayLabel().Equals(label));
        }

    virtual void Preprocess(ECSchemaR schema) const override
        {
        // This test used to rely on SetName() automatically encoding a non-EC name.
        // We removed that behavior because it diverges from managed EC (which throws an "invalid name" exception instead)
        // So now we must explicitly encode the name first.
        Utf8String encodedName;
        EXPECT_EQ(!ECNameValidation::IsValidName(m_name.c_str()), ECNameValidation::EncodeToValidName(encodedName, m_name));
        schema.SetName(encodedName);
        Compare(schema);

        ECEntityClassP ecclass;
        schema.CreateEntityClass(ecclass, encodedName);
        Compare(*ecclass);

        PrimitiveECPropertyP ecprop;
        ecclass->CreatePrimitiveProperty(ecprop, encodedName, PRIMITIVETYPE_String);
        Compare(*ecprop);
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
        Compare(*ecclass);
        ecprop->SetDisplayLabel("");
        Compare(*ecprop);
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
        NULL, NULL
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
    EXPECT_VALIDATION_RESULT(NullOrEmpty, NULL);

    EXPECT_VALIDATION_RESULT(BeginsWithDigit, "1_C");

    EXPECT_VALIDATION_RESULT(IncludesInvalidCharacters, "!ABC");
    EXPECT_VALIDATION_RESULT(IncludesInvalidCharacters, "ABC@");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaComparisonTest, VerifyMatchesOperator)
    {
    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 0) == SchemaKey("SchemaTest", 1, 0));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0) == SchemaKey("SchemaNotTest", 1, 0));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0) == SchemaKey("SchemaTest", 2, 0));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0) == SchemaKey("SchemaTest", 1, 1));

    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::Exact));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaNotTest", 1, 0), SchemaMatchType::Exact));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 2, 0), SchemaMatchType::Exact));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 1), SchemaMatchType::Exact));

    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::Identical));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaNotTest", 1, 0), SchemaMatchType::Identical));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 2, 0), SchemaMatchType::Identical));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 1), SchemaMatchType::Identical));

    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::Latest));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaNotTest", 1, 0), SchemaMatchType::Latest));
    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 2, 0), SchemaMatchType::Latest));
    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 1).Matches(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::Latest));

    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::LatestWriteCompatible));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaNotTest", 1, 0), SchemaMatchType::LatestWriteCompatible));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 2, 0), SchemaMatchType::LatestWriteCompatible));
    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 1).Matches(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::LatestWriteCompatible));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 1), SchemaMatchType::LatestWriteCompatible));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaComparisonTest, VerifyLessThanOperator)
    {
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0) < SchemaKey("SchemaTest", 1, 0));
    EXPECT_TRUE(SchemaKey("SchemaTesa", 1, 0) < SchemaKey("SchemaTest", 1, 0));
    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 0) < SchemaKey("SchemaTest", 2, 0));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 1) < SchemaKey("SchemaTest", 1, 0));

    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::Exact));
    EXPECT_TRUE(SchemaKey("SchemaTesa", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::Exact));
    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 2, 0), SchemaMatchType::Exact));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 1).LessThan(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::Exact));

    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::Identical));
    EXPECT_TRUE(SchemaKey("SchemaTesa", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::Identical));
    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 2, 0), SchemaMatchType::Identical));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 1).LessThan(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::Identical));

    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::Latest));
    EXPECT_TRUE(SchemaKey("SchemaTesa", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::Latest));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 2, 0), SchemaMatchType::Latest));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 9), SchemaMatchType::Latest));

    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::LatestWriteCompatible));
    EXPECT_TRUE(SchemaKey("SchemaTesa", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::LatestWriteCompatible));
    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 2, 0), SchemaMatchType::LatestWriteCompatible));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 9), SchemaMatchType::LatestWriteCompatible));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaComparisonTest, VerifyNotMatchesOperator)
    {
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0) != SchemaKey("SchemaTest", 1, 0));
    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 0) != SchemaKey("SchemaNotTest", 1, 0));
    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 0) != SchemaKey("SchemaTest", 2, 0));
    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 0) != SchemaKey("SchemaTest", 1, 1));
    }

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

    EXPECT_TRUE(cache->AddSchema(*schema1) == ECObjectsStatus::Success);
    EXPECT_TRUE(cache->AddSchema(*schema2) == ECObjectsStatus::Success);
    EXPECT_TRUE(cache->AddSchema(*schema3) == ECObjectsStatus::Success);

    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseSchema1", 2, 0), SchemaMatchType::Exact) != NULL);
    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseZchema1", 2, 0), SchemaMatchType::Exact) == NULL);
    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseSchema1", 3, 0), SchemaMatchType::Exact) == NULL);
    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseSchema1", 2, 1), SchemaMatchType::Exact) == NULL);

    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseSchema1", 2, 0), SchemaMatchType::Identical) != NULL);
    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseZchema1", 2, 0), SchemaMatchType::Identical) == NULL);
    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseSchema1", 3, 0), SchemaMatchType::Identical) == NULL);
    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseSchema1", 2, 1), SchemaMatchType::Identical) == NULL);

    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseSchema1", 2, 0), SchemaMatchType::Latest) != NULL);
    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseZchema1", 2, 0), SchemaMatchType::Latest) == NULL);
    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseSchema1", 3, 0), SchemaMatchType::Latest) != NULL);
    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseSchema1", 2, 1), SchemaMatchType::Latest) != NULL);

    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseSchema2", 5, 5), SchemaMatchType::LatestWriteCompatible) != NULL);
    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseZchema2", 5, 5), SchemaMatchType::LatestWriteCompatible) == NULL);
    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseSchema2", 3, 5), SchemaMatchType::LatestWriteCompatible) == NULL);
    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseSchema2", 5, 3), SchemaMatchType::LatestWriteCompatible) != NULL);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaChecksumTest, ComputeSchemaXmlStringCheckSum)
    {
    Utf8Char schemaXml[] =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"Widgets\" version=\"09.06\" displayLabel=\"Widgets Display Label\" description=\"Widgets Description\" nameSpacePrefix=\"wid\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ods=\"Bentley_ODS.01.02\">"
        "    <ECClass typeName=\"ecProject\" description=\"Project ECClass\" displayLabel=\"Project\" isDomainClass=\"True\">"
        "       <ECProperty propertyName=\"Name\" typename=\"string\" displayLabel=\"Project Name\" />"
        "    </ECClass>"
        "</ECSchema>";

    EXPECT_EQ(ECSchema::ComputeSchemaXmlStringCheckSum(schemaXml, sizeof(schemaXml)), 682119251);
    }

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

    ECEntityClassP class1 = NULL;
    ECClassP class2 = schema->GetClassP("ecProject");
    ECRelationshipClassP relationshipClass;
    ECClassP base = (ECClassP) class1;
    EXPECT_EQ(schema->CreateEntityClass(class1, "TestClass"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_TRUE(class1 == NULL);
    EXPECT_EQ(schema->CopyClass(base, *class2), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->CreateRelationshipClass(relationshipClass, "RelationshipClass"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->SetName("Some new name"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->SetAlias("Some new alias"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->SetDescription("Some new description"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->SetDisplayLabel("Some new label"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->SetVersionRead(13), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->SetVersionMinor(13), ECObjectsStatus::SchemaIsImmutable);
    }

bool CompareFiles(Utf8StringCP lFileName, Utf8StringCP rFileName)
    {
    BeFile lFile;
    BeFileStatus lStatus = lFile.Open(*lFileName, BeFileAccess::Read);

    EXPECT_EQ(BeFileStatus::Success, lStatus) << "Could not open " << *lFileName << " for verification";

    BeFile rFile;
    BeFileStatus rStatus = rFile.Open(*rFileName, BeFileAccess::Read);
    EXPECT_EQ(BeFileStatus::Success, lStatus) << "Could not open " << *rFileName << " for verification";

    ByteStream lStream;
    ByteStream rStream;
    lStatus = lFile.ReadEntireFile(lStream);
    rStatus = rFile.ReadEntireFile(rStream);

    if (lStream.GetSize() != rStream.GetSize())
        return false;

    const uint8_t *lBuffer = lStream.GetData();
    const uint8_t *rBuffer = rStream.GetData();
    for (uint32_t i = 0; i < lStream.GetSize(); i++)
        {
        if (lBuffer[i] != rBuffer[i])
            return false;
        }
    lFile.Close();
    rFile.Close();
    return true;
    }

TEST_F(SchemaTest, RoundtripSchemaXmlCommentsTest)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->SetPreserveXmlComments(true);
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"dgn-testingonly.02.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    SchemaWriteStatus statusW = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(L"dgn-testingonly-result.02.00.ecschema.xml").c_str(), ECVersion::V2_0, false);
    EXPECT_EQ(SchemaWriteStatus::Success, statusW);

    Utf8String serializedSchemaFile(ECTestFixture::GetTempDataPath(L"dgn-testingonly-result.02.00.ecschema.xml"));
    Utf8String expectedSchemaFile(ECTestFixture::GetTestDataPath(L"dgn-testingonly-ExpectedResult.02.00.ecschema.xml"));

    // Deactivated because it might fail randomly because of varying order of <schemareference> in schema
    //EXPECT_TRUE(CompareFiles(&serializedSchemaFile, &expectedSchemaFile)) << "Serialized schema differs from expected schema";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaTest, CreateDynamicSchema)
    {
    //Load Bentley_Standard_CustomAttributes
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back(ECTestFixture::GetTestDataPath(L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
    schemaContext->AddSchemaLocater(*schemaLocater);

    SchemaKey schemaKey("CoreCustomAttributes", 1, 0);
    ECSchemaPtr standardCASchema = schemaContext->LocateSchema(schemaKey, SchemaMatchType::Latest);
    EXPECT_TRUE(standardCASchema.IsValid());

    IECInstancePtr dynamicSchemaCA = standardCASchema->GetClassCP("DynamicSchema")->GetDefaultStandaloneEnabler()->CreateInstance();
    ECSchemaCachePtr cache = ECSchemaCache::Create();
    ECSchemaPtr schema;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 2, 0, 1);
    schema->AddReferencedSchema(*standardCASchema);
    ASSERT_EQ(ECObjectsStatus::Success, schema->SetCustomAttribute(*dynamicSchemaCA));
    

    ASSERT_EQ(ECObjectsStatus::Success, cache->AddSchema(*schema));
    ECSchemaP retrievedSchema = cache->GetSchema(SchemaKey("TestSchema", 2, 1), SchemaMatchType::Exact);
    ASSERT_TRUE(retrievedSchema != NULL);

    ASSERT_TRUE(retrievedSchema->IsDynamicSchema());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaTest, TryRenameECClass)
    {
    ECSchemaCachePtr cache = ECSchemaCache::Create();
    ECSchemaPtr schema;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    ECEntityClassP entityClass1;
    ECEntityClassP entityClass2;
    schema->CreateEntityClass(entityClass1, "ClassA");
    schema->CreateEntityClass(entityClass2, "ClassB");

    ASSERT_EQ(ECObjectsStatus::Success, cache->AddSchema(*schema));
    ECSchemaP retrievedSchema = cache->GetSchema(SchemaKey("TestSchema", 5, 5), SchemaMatchType::Exact);
    ASSERT_TRUE(retrievedSchema != NULL);

    // rename classes
    ASSERT_EQ(ECObjectsStatus::Success, retrievedSchema->RenameClass(*retrievedSchema->GetClassP("ClassA"), "ClassA1"));
    ASSERT_EQ(ECObjectsStatus::Success, retrievedSchema->RenameClass(*retrievedSchema->GetClassP("ClassB"), "ClassB1"));

    // try to get classes with old names
    ASSERT_TRUE(nullptr == retrievedSchema->GetClassCP("ClassA"));
    ASSERT_TRUE(nullptr == retrievedSchema->GetClassCP("ClassB"));

    // Get classes with new names
    ECClassP classA1 = retrievedSchema->GetClassP("ClassA1");
    ECClassP classB1 = retrievedSchema->GetClassP("ClassB1");
    ASSERT_TRUE(nullptr != classA1);
    ASSERT_TRUE(nullptr != classB1);

    // Delete Classes
    ASSERT_EQ(ECObjectsStatus::Success, retrievedSchema->DeleteClass(*classA1));
    ASSERT_EQ(ECObjectsStatus::Success, retrievedSchema->DeleteClass(*classB1));
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
        "                    presentationUnits='FT;INCH;YARD' />"
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
        {
        ASSERT_EQ(ECObjectsStatus::Success, schema->DeleteKindOfQuantity(*koq));
        }

    schema->DebugDump();
    }

// This test was to illustrate a problem with the ECDiff tool.  However, we decided to not to make the fix on this branch.  The tool has been rewritten on bim0200.
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
//TEST(SchemaDiffTests, RightNotLeft)
//    {
//    Utf8CP leftXml = 
//        "<?xml version='1.0' encoding='utf-8'?>"
//
//        "<ECSchema schemaName=\"OpenPlant\" nameSpacePrefix=\"op\" version=\"1.4\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
//        "    <ECClass typeName=\"NAMED_ITEM\" displayLabel=\"Named Item\" isDomainClass=\"True\">"
//        "        <ECProperty propertyName=\"NAME\" typeName=\"string\" description=\"name of area.\" displayLabel=\"Name\" />"
//        "    </ECClass>"
//        "    <ECClass typeName=\"SPECIALTY_ITEM\" displayLabel=\"Specialty Item\" isDomainClass=\"True\">"
//        "        <BaseClass>NAMED_ITEM</BaseClass>"
//        "    </ECClass>"
//        "</ECSchema>";
//
//    Utf8CP rightXml = 
//        "<?xml version='1.0' encoding='utf-8'?>"
//        "<ECSchema schemaName=\"OpenPlant\" nameSpacePrefix=\"op\" version=\"1.4\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
//        "    <ECClass typeName=\"NAMED_ITEM\" displayLabel=\"Named Item\" isDomainClass=\"True\">"
//        "        <ECProperty propertyName=\"NAME\" typeName=\"string\" description=\"name of area.\" displayLabel=\"Name\" />"
//        "    </ECClass>"
//        "    <ECClass typeName=\"SPECIALTY_ITEM\" displayLabel=\"Specialty Item\" isDomainClass=\"True\">"
//        "        <BaseClass>NAMED_ITEM</BaseClass>"
//        "        <ECProperty propertyName=\"NAME\" typeName=\"string\" description=\"name of area.\" displayLabel=\"Tag Number\" />"
//        "    </ECClass>"
//        "</ECSchema>";
//
//    ECSchemaReadContextPtr leftSchemaContext = ECSchemaReadContext::CreateContext();
//    ECSchemaReadContextPtr rightSchemaContext = ECSchemaReadContext::CreateContext();
//    ECSchemaPtr leftSchema, rightSchema;
//    ECSchema::ReadFromXmlString(leftSchema, leftXml, *leftSchemaContext);
//    ECSchema::ReadFromXmlString(rightSchema, rightXml, *rightSchemaContext);
//    ECDiffPtr diff = ECDiff::Diff(*leftSchema, *rightSchema);
//    ASSERT_TRUE(diff.IsValid());
//
//    ECSchemaPtr mergedSchema;
//    MergeStatus status = diff->Merge(mergedSchema, CONFLICTRULE_TakeLeft);
//    ASSERT_EQ(status, MergeStatus::Success);
//    ASSERT_TRUE(mergedSchema.IsValid());
//
//    }

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

    EXPECT_EQ(ECVersion::Latest, ecVersion31) << "ECVersion Latest should be equal to 3.1, therefore the comparsion should succeed.";

    EXPECT_STREQ("3.1", ECSchema::GetECVersionString(ECVersion::Latest)) << "ECVersion Latest should be equal to 3.1, therefore the string of it should be equal to 3.1.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                     10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaVersionTest, CreateSchemaECVersionTest)
    {
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);
    EXPECT_TRUE(schema->IsECVersion(ECVersion::V3_1)) << "The default schema ECVersion should be EC3.1 so this should validate to true.";
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest)) << "The default schema ECVersion should be the latest so this should validate to true.";
    }
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5, ECVersion::Latest);
    EXPECT_TRUE(schema->IsECVersion(ECVersion::V3_1)) << "The schema was created as the Latest version which is EC3.1 so this should validate to true.";
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest)) << "The schema was created as the Latest version so this should validate to true.";
    }
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5, ECVersion::V3_1);
    EXPECT_TRUE(schema->IsECVersion(ECVersion::V3_1)) << "The schema was created as an EC3.1 schema.";
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest)) << "The schema was created as an EC3.1 schema so Latest should return true.";
    }
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5, ECVersion::V3_0);
    EXPECT_TRUE(schema->IsECVersion(ECVersion::V3_0)) << "The schema was created as an EC3.0 schema.";
    EXPECT_FALSE(schema->IsECVersion(ECVersion::Latest)) << "The schema was created as an EC3.0 schema so it is not the latest";
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

END_BENTLEY_ECN_TEST_NAMESPACE
