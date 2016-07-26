/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SchemaTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

#include <iostream>
#include <fstream>

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

// NEEDSWORK Improve strategy for seed data.  Should not be maintained in source.
#define SCHEMAS_PATH  L"" 

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

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClassTest : ECTestFixture
    {
    void TestPropertyCount(ECClassCR ecClass, size_t nPropertiesWithoutBaseClasses, size_t nPropertiesWithBaseClasses)
        {
        EXPECT_EQ(ecClass.GetPropertyCount(false), nPropertiesWithoutBaseClasses);
        EXPECT_EQ(ecClass.GetPropertyCount(true), nPropertiesWithBaseClasses);
        }
    };

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

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
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
    ArrayECPropertyP MyArrayProp;
    structClass->CreateArrayProperty(MyArrayProp, "ArrayProperty");
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

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
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

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaTest, CheckEnumerationBasicProperties)
    {
    ECSchemaPtr schema;
    ECEnumerationP enumeration;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
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

    EXPECT_TRUE(schema->FindSchema(SchemaKey("SchemaThatReferences", 1, 0), SchemaMatchType::Exact) != NULL);
    EXPECT_TRUE(schema->FindSchema(SchemaKey("SchemaThatReferencez", 1, 0), SchemaMatchType::Exact) == NULL);
    EXPECT_TRUE(schema->FindSchema(SchemaKey("SchemaThatReferences", 2, 0), SchemaMatchType::Exact) == NULL);
    EXPECT_TRUE(schema->FindSchema(SchemaKey("SchemaThatReferences", 1, 1), SchemaMatchType::Exact) == NULL);

    EXPECT_TRUE(schema->FindSchema(SchemaKey("BaseSchema", 1, 0), SchemaMatchType::Exact) != NULL);
    EXPECT_TRUE(schema->FindSchemaP(SchemaKey("SchemaThatReferences", 1, 0), SchemaMatchType::Exact) != NULL);
    EXPECT_TRUE(schema->FindSchemaP(SchemaKey("a", 123, 456), SchemaMatchType::Exact) == NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static  void    ValidateSchemaNameParsing(Utf8CP fullName, bool expectFailure, Utf8CP expectName, uint32_t expectMajor, uint32_t expectWrite, uint32_t expectMinor)
    {
    Utf8String    shortName;
    uint32_t   versionMajor;
    uint32_t   versionWrite;
    uint32_t   versionMinor;

    ECObjectsStatus status = ECSchema::ParseSchemaFullName(shortName, versionMajor, versionWrite, versionMinor, fullName);

    if (expectFailure)
        {
        EXPECT_TRUE(ECObjectsStatus::Success != status);
        return;
        }

    EXPECT_TRUE(ECObjectsStatus::Success == status);

    EXPECT_STREQ(shortName.c_str(), expectName);
    EXPECT_EQ(versionMajor, expectMajor);
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

    ECSchema::CreateSchema(schema, "Widget", 5, 5);
    ECSchema::CreateSchema(schema2, "BaseSchema", 5, 5);
    ECSchema::CreateSchema(schema3, "BaseSchema2", 5, 5);

    schema->SetNamespacePrefix("ecw");
    schema2->SetNamespacePrefix("base");
    schema3->SetNamespacePrefix("base");

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

    status2 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(L"base_ec3.xml").c_str(), 3);
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
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);

    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, "RefSchema", 5, 5);

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
    entityClass->CreatePrimitiveProperty(primitiveProperty8, "Primitive8", PrimitiveType::PRIMITIVETYPE_Point2D);
    PrimitiveECPropertyP primitiveProperty9;
    entityClass->CreatePrimitiveProperty(primitiveProperty9, "Primitive9", PrimitiveType::PRIMITIVETYPE_Point3D);
    PrimitiveECPropertyP primitiveProperty10;
    entityClass->CreatePrimitiveProperty(primitiveProperty10, "Primitive10", PrimitiveType::PRIMITIVETYPE_String);
    PrimitiveECPropertyP calculatedProperty;
    entityClass->CreatePrimitiveProperty(calculatedProperty, "Calculated", PrimitiveType::PRIMITIVETYPE_String);
    ArrayECPropertyP arrayProperty;
    entityClass->CreateArrayProperty(arrayProperty, "Array", PrimitiveType::PRIMITIVETYPE_Long);

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
    entityClass->CreateStructArrayProperty(structArrayProperty, "StructArray", structClass);

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
    relationshipClass->GetSource().SetCardinality(RelationshipCardinality::ZeroOne());
    relationshipClass->GetTarget().AddClass(*entityClass);
    relationshipClass->GetTarget().SetCardinality(RelationshipCardinality::ZeroOne());

    NavigationECPropertyP navProp;
    entityClass->CreateNavigationProperty(navProp, "NavigationProperty", *relationshipClass, ECRelatedInstanceDirection::Forward);

    KindOfQuantityP kindOfQuantity;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(kindOfQuantity, "MyKindOfQuantity"));
    kindOfQuantity->SetDescription("Kind of a Description here");
    kindOfQuantity->SetDisplayLabel("best quantity of all times");
    kindOfQuantity->SetPersistenceUnit("CENTIMETRE");
    kindOfQuantity->SetPrecision(10);
    kindOfQuantity->SetDefaultPresentationUnit("FOOT");
    auto& altPresUnits = kindOfQuantity->GetAlternativePresentationUnitListR();
    altPresUnits.push_back("INCH");
    altPresUnits.push_back("MILLIINCH");

    WString fullSchemaName;
    fullSchemaName.AssignUtf8(schema->GetFullSchemaName().c_str());
    fullSchemaName.append(L".ecschema.xml");

    WString legacyFullSchemaName;
    legacyFullSchemaName.AssignUtf8(schema->GetLegacyFullSchemaName().c_str());
    legacyFullSchemaName.append(L".ecschema.xml");

    SchemaWriteStatus status2 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(fullSchemaName.c_str()).c_str(), 3);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    SchemaWriteStatus status3 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(legacyFullSchemaName.c_str()).c_str(), 2);
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
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);

    ECSchemaPtr refSchema;
    ECSchema::CreateSchema(refSchema, "RefSchema", 5, 5);

    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*refSchema));
    ECEntityClassP class1;
    ECEntityClassP baseClass;
    ECStructClassP structClass;

    refSchema->CreateEntityClass(baseClass, "BaseClass");
    refSchema->CreateStructClass(structClass, "StructClass");
    schema->CreateEntityClass(class1, "TestClass");

    class1->AddBaseClass(*baseClass);
    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema));

    class1->RemoveBaseClass(*baseClass);
    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema));

    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*refSchema));
    StructECPropertyP structProp;
    StructArrayECPropertyP nestedArrayProp;

    ArrayECPropertyP primitiveArrayProp;

    class1->CreateStructProperty(structProp, "StructMember");
    class1->CreateStructArrayProperty(nestedArrayProp, "NestedArray", structClass);

    class1->CreateArrayProperty(primitiveArrayProp, "PrimitiveArrayProp");
    primitiveArrayProp->SetPrimitiveElementType(PRIMITIVETYPE_Long);
    primitiveArrayProp->SetMinOccurs(1);
    primitiveArrayProp->SetMaxOccurs(10);

    structProp->SetType(*structClass);

    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema));
    class1->RemoveProperty("StructMember");
    EXPECT_EQ(ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema(*refSchema));
    class1->RemoveProperty("NestedArray");
    EXPECT_EQ(ECObjectsStatus::Success, schema->RemoveReferencedSchema(*refSchema));
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
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    ECSchema::CreateSchema(refSchema, "RefSchema", 5, 5);

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
        EXPECT_TRUE(ECSchema::ParseVersionString(key.m_versionMajor, key.m_versionMinor, entry.second) == ECObjectsStatus::Success);
        EXPECT_EQ(key.m_versionMajor, atoi(entry.second));
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
    ECSchema::CreateSchema(testSchema, "TestSchema", 1, 2);
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

    ECSchema::CreateSchema(schema, "Units_Schema", 1, 4);
    EXPECT_TRUE(schema->ShouldNotBeStored());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaCreationTest, CanFullyCreateASchema)
    {
    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, "TestSchema", 1, 2);
    testSchema->SetNamespacePrefix("ts");
    testSchema->SetDescription("Schema for testing programmatic construction");
    testSchema->SetDisplayLabel("Test Schema");

    EXPECT_TRUE(testSchema->GetIsDisplayLabelDefined());
    EXPECT_EQ(1, testSchema->GetVersionMajor());
    EXPECT_EQ(2, testSchema->GetVersionMinor());
    EXPECT_EQ(0, strcmp(testSchema->GetName().c_str(), "TestSchema"));
    EXPECT_EQ(0, strcmp(testSchema->GetNamespacePrefix().c_str(), "ts"));
    EXPECT_EQ(0, strcmp(testSchema->GetDescription().c_str(), "Schema for testing programmatic construction"));
    EXPECT_EQ(0, strcmp(testSchema->GetDisplayLabel().c_str(), "Test Schema"));

    ECSchemaPtr schema2;
    ECSchema::CreateSchema(schema2, "BaseSchema", 5, 5);

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

    PrimitiveECPropertyP stringProp;
    StructECPropertyP structProp;
    StructArrayECPropertyP nestedArrayProp;
    ArrayECPropertyP primitiveArrayProp;

    class1->CreatePrimitiveProperty(stringProp, "StringMember");
    class1->CreateStructProperty(structProp, "StructMember");
    class1->CreateStructArrayProperty(nestedArrayProp, "NestedArray", structClass);
    class1->CreateArrayProperty(primitiveArrayProp, "PrimitiveArray");

    structProp->SetType(*structClass);
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
    class1->CreatePrimitiveProperty(point2DProperty, "Point2DProp");
    class1->CreatePrimitiveProperty(point3DProperty, "Point3DProp");

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
    EXPECT_TRUE(PRIMITIVETYPE_Point2D == point2DProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Point3D == point3DProperty->GetType());

    class1->CreatePrimitiveProperty(binaryProperty, "BinaryProp2", PRIMITIVETYPE_Binary);
    class1->CreatePrimitiveProperty(booleanProperty, "BooleanProp2", PRIMITIVETYPE_Boolean);
    class1->CreatePrimitiveProperty(dateTimeProperty, "DateTimeProp2", PRIMITIVETYPE_DateTime);
    class1->CreatePrimitiveProperty(doubleProperty, "DoubleProp2", PRIMITIVETYPE_Double);
    class1->CreatePrimitiveProperty(integerProperty, "IntProp2", PRIMITIVETYPE_Integer);
    class1->CreatePrimitiveProperty(longProperty, "LongProp2", PRIMITIVETYPE_Long);
    class1->CreatePrimitiveProperty(point2DProperty, "Point2DProp2", PRIMITIVETYPE_Point2D);
    class1->CreatePrimitiveProperty(point3DProperty, "Point3DProp2", PRIMITIVETYPE_Point3D);

    EXPECT_TRUE(PRIMITIVETYPE_Binary == binaryProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Boolean == booleanProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_DateTime == dateTimeProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Double == doubleProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Integer == integerProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Long == longProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Point2D == point2DProperty->GetType());
    EXPECT_TRUE(PRIMITIVETYPE_Point3D == point3DProperty->GetType());

    class1->CreateStructProperty(structProp, "StructMember2", *structClass);
    class1->CreateStructArrayProperty(nestedArrayProp, "NestedArray2", structClass);
    class1->CreateArrayProperty(primitiveArrayProp, "PrimitiveArray2", PRIMITIVETYPE_Integer);
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

    EXPECT_EQ(0, relationshipClass->GetSource().GetClasses().size());
    EXPECT_EQ(0, relationshipClass->GetTarget().GetClasses().size());

    relationshipClass->GetSource().AddClass(*class1);
    EXPECT_EQ(1, relationshipClass->GetSource().GetClasses().size());

    relationshipClass->GetTarget().AddClass(*relatedClass);
    EXPECT_EQ(1, relationshipClass->GetTarget().GetClasses().size());
    relationshipClass->GetTarget().AddClass(*relatedClass);
    EXPECT_EQ(1, relationshipClass->GetTarget().GetClasses().size());
    relationshipClass->GetTarget().AddClass(*class2);
    EXPECT_EQ(2, relationshipClass->GetTarget().GetClasses().size());

    EXPECT_EQ(0, relationshipClass->GetSource().GetCardinality().GetLowerLimit());
    EXPECT_EQ(0, relationshipClass->GetTarget().GetCardinality().GetLowerLimit());
    EXPECT_EQ(1, relationshipClass->GetSource().GetCardinality().GetUpperLimit());
    EXPECT_EQ(1, relationshipClass->GetTarget().GetCardinality().GetUpperLimit());

    relationshipClass->GetSource().SetCardinality(RelationshipCardinality::OneMany());
    EXPECT_EQ(1, relationshipClass->GetSource().GetCardinality().GetLowerLimit());
    EXPECT_TRUE(relationshipClass->GetSource().GetCardinality().IsUpperLimitUnbounded());

    RelationshipCardinality *card = new RelationshipCardinality(2, 5);
    relationshipClass->GetTarget().SetCardinality(*card);
    EXPECT_EQ(2, relationshipClass->GetTarget().GetCardinality().GetLowerLimit());
    EXPECT_EQ(5, relationshipClass->GetTarget().GetCardinality().GetUpperLimit());
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
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectErrorWithCircularBaseClasses)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    ECEntityClassP baseClass1;
    ECEntityClassP baseClass2;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateEntityClass(class1, "TestClass");
    schema->CreateEntityClass(baseClass1, "BaseClass1");
    schema->CreateEntityClass(baseClass2, "BaseClass2");

    EXPECT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*baseClass1));
    EXPECT_EQ(ECObjectsStatus::Success, baseClass1->AddBaseClass(*baseClass2));
    EXPECT_EQ(ECObjectsStatus::BaseClassUnacceptable, baseClass2->AddBaseClass(*class1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, GetPropertyCount)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", 1, 0);

    ECEntityClassP baseClass1, baseClass2, derivedClass;
    ECStructClassP structClass;

    PrimitiveECPropertyP primProp;
    StructECPropertyP structProp;

    // Struct class with 2 properties
    schema->CreateStructClass(structClass, "StructClass");
    structClass->CreatePrimitiveProperty(primProp, "StructProp1");
    structClass->CreatePrimitiveProperty(primProp, "StructProp2");

    // 1 base class with 3 primitive properties
    schema->CreateEntityClass(baseClass1, "BaseClass1");
    baseClass1->CreatePrimitiveProperty(primProp, "Base1Prop1");
    baseClass1->CreatePrimitiveProperty(primProp, "Base1Prop2");
    baseClass1->CreatePrimitiveProperty(primProp, "Base1Prop3");

    // 1 base class with 1 primitive and 2 struct properties (each struct has 2 properties
    schema->CreateEntityClass(baseClass2, "BaseClass2");
    baseClass2->CreatePrimitiveProperty(primProp, "Base2Prop1");
    baseClass2->CreateStructProperty(structProp, "Base2Prop2", *structClass);
    baseClass2->CreateStructProperty(structProp, "Base2Prop3", *structClass);

    // Derived class with 1 extra primitive property, 1 extra struct property, derived from 2 base classes
    schema->CreateEntityClass(derivedClass, "DerivedClass");
    derivedClass->CreateStructProperty(structProp, "DerivedProp1", *structClass);
    derivedClass->CreatePrimitiveProperty(primProp, "DerivedProp2");
    derivedClass->AddBaseClass(*baseClass1);
    derivedClass->AddBaseClass(*baseClass2);

    TestPropertyCount(*structClass, 2, 2);
    TestPropertyCount(*baseClass1, 3, 3);
    TestPropertyCount(*baseClass2, 3, 3);
    TestPropertyCount(*derivedClass, 2, 8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsClassInList(bvector<ECClassP> const& classList, ECClassR searchClass)
    {
    bvector<ECClassP>::const_iterator classIterator;

    for (classIterator = classList.begin(); classIterator != classList.end(); classIterator++)
        {
        if (*classIterator == &searchClass)
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, AddAndRemoveBaseClass)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    ECEntityClassP baseClass1;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateEntityClass(class1, "TestClass");
    schema->CreateEntityClass(baseClass1, "BaseClass");

    EXPECT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*baseClass1));

    EXPECT_TRUE(IsClassInList(class1->GetBaseClasses(), *baseClass1));
    EXPECT_TRUE(IsClassInList(baseClass1->GetDerivedClasses(), *class1));

    EXPECT_EQ(ECObjectsStatus::Success, class1->RemoveBaseClass(*baseClass1));

    EXPECT_FALSE(IsClassInList(class1->GetBaseClasses(), *baseClass1));
    EXPECT_FALSE(IsClassInList(baseClass1->GetDerivedClasses(), *class1));

    EXPECT_EQ(ECObjectsStatus::ClassNotFound, class1->RemoveBaseClass(*baseClass1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, AddBaseClassWithProperties)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    ECEntityClassP baseClass1;
    ECEntityClassP baseClass2;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateEntityClass(class1, "TestClass");
    schema->CreateEntityClass(baseClass1, "BaseClass");
    schema->CreateEntityClass(baseClass2, "BaseClass2");

    PrimitiveECPropertyP stringProp;
    PrimitiveECPropertyP baseStringProp;
    PrimitiveECPropertyP intProp;
    PrimitiveECPropertyP base2NonIntProp;

    class1->CreatePrimitiveProperty(stringProp, "StringProperty", PRIMITIVETYPE_String);
    baseClass1->CreatePrimitiveProperty(baseStringProp, "StringProperty", PRIMITIVETYPE_String);
    EXPECT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*baseClass1));

    class1->CreatePrimitiveProperty(intProp, "IntProperty", PRIMITIVETYPE_Integer);
    baseClass2->CreatePrimitiveProperty(base2NonIntProp, "IntProperty", PRIMITIVETYPE_String);
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->AddBaseClass(*baseClass2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, BaseClassOrder)
    {
    ECSchemaPtr schema = nullptr;
    ECEntityClassP class1 = nullptr;
    ECEntityClassP baseClass1 = nullptr;
    ECEntityClassP baseClass2 = nullptr;
    ECEntityClassP baseClass3 = nullptr;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateEntityClass(class1, "TestClass");
    schema->CreateEntityClass(baseClass1, "BaseClass");
    schema->CreateEntityClass(baseClass2, "BaseClass2");
    schema->CreateEntityClass(baseClass3, "BaseClass3");

    PrimitiveECPropertyP prop = nullptr;
    class1->CreatePrimitiveProperty(prop, "StringProperty", PRIMITIVETYPE_String);
    baseClass1->CreatePrimitiveProperty(prop, "StringProperty", PRIMITIVETYPE_String);
    baseClass2->CreatePrimitiveProperty(prop, "SstringProperty", PRIMITIVETYPE_String);
    baseClass3->CreatePrimitiveProperty(prop, "StringProperty", PRIMITIVETYPE_String);

    ASSERT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*baseClass1));
    ASSERT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*baseClass2));

    ASSERT_EQ(2, class1->GetBaseClasses().size());
    ASSERT_TRUE(baseClass1 == class1->GetBaseClasses()[0]);
    ASSERT_TRUE(baseClass2 == class1->GetBaseClasses()[1]);

    ASSERT_EQ(ECObjectsStatus::Success, class1->AddBaseClass(*baseClass3, true));
    ASSERT_EQ(3, class1->GetBaseClasses().size());
    ASSERT_TRUE(baseClass3 == class1->GetBaseClasses()[0]);
    ASSERT_TRUE(baseClass1 == class1->GetBaseClasses()[1]);
    ASSERT_TRUE(baseClass2 == class1->GetBaseClasses()[2]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, IsTests)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    ECEntityClassP baseClass1;
    ECEntityClassP baseClass2;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateEntityClass(class1, "TestClass");
    schema->CreateEntityClass(baseClass1, "BaseClass1");
    schema->CreateEntityClass(baseClass2, "BaseClass2");

    EXPECT_FALSE(class1->Is(baseClass1));
    class1->AddBaseClass(*baseClass1);
    EXPECT_TRUE(class1->Is(baseClass1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, CanOverrideBaseProperties)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    ECEntityClassP baseClass1;
    ECStructClassP structClass;
    ECStructClassP structClass2;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateEntityClass(class1, "TestClass");
    schema->CreateEntityClass(baseClass1, "BaseClass1");
    schema->CreateStructClass(structClass, "ClassForStructs");
    schema->CreateStructClass(structClass2, "ClassForStructs2");
    class1->AddBaseClass(*baseClass1);

    PrimitiveECPropertyP baseStringProp;
    PrimitiveECPropertyP baseIntProp;
    PrimitiveECPropertyP baseDoubleProp;
    StructECPropertyP baseStructProp;
    ArrayECPropertyP baseStringArrayProperty;
    StructArrayECPropertyP baseStructArrayProp;

    baseClass1->CreatePrimitiveProperty(baseStringProp, "StringProperty", PRIMITIVETYPE_String);
    baseClass1->CreatePrimitiveProperty(baseIntProp, "IntegerProperty", PRIMITIVETYPE_Integer);
    baseClass1->CreatePrimitiveProperty(baseDoubleProp, "DoubleProperty", PRIMITIVETYPE_Double);
    baseClass1->CreateStructProperty(baseStructProp, "StructProperty", *structClass);
    baseClass1->CreateArrayProperty(baseStringArrayProperty, "StringArrayProperty", PRIMITIVETYPE_String);
    baseClass1->CreateStructArrayProperty(baseStructArrayProp, "StructArrayProperty", structClass);

    PrimitiveECPropertyP longProperty = NULL;
    PrimitiveECPropertyP stringProperty = NULL;

    DISABLE_ASSERTS;
    // Primitives overriding primitives
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreatePrimitiveProperty(longProperty, "StringProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(NULL, longProperty);
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(stringProperty, "StringProperty", PRIMITIVETYPE_String));
    EXPECT_EQ(baseStringProp, stringProperty->GetBaseProperty());
    class1->RemoveProperty("StringProperty");

    {
    // Primitives overriding structs
    DISABLE_ASSERTS
        EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreatePrimitiveProperty(longProperty, "StructProperty", PRIMITIVETYPE_Long));
    }

    // Primitives overriding arrays
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreatePrimitiveProperty(longProperty, "StringArrayProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreatePrimitiveProperty(stringProperty, "StringArrayProperty", PRIMITIVETYPE_String));
    class1->RemoveProperty("StringArrayProperty");

    StructECPropertyP structProperty;

    {
    // Structs overriding primitives
    DISABLE_ASSERTS
        EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty(structProperty, "IntegerProperty"));
    }

    // Structs overriding structs
    // If we don't specify a struct type for the new property, then it should succeed
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreateStructProperty(structProperty, "StructProperty"));
    class1->RemoveProperty("StructProperty");
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty(structProperty, "StructProperty", *structClass2));

    // Structs overriding arrays
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty(structProperty, "StringArrayProperty"));
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty(structProperty, "StringArrayProperty", *structClass));
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty(structProperty, "StructArrayProperty"));
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty(structProperty, "StructArrayProperty", *structClass));
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty(structProperty, "StructArrayProperty", *structClass2));

    ArrayECPropertyP stringArrayProperty;
    ArrayECPropertyP stringArrayProperty2;
    StructArrayECPropertyP structArrayProperty;
    // Arrays overriding primitives
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreateArrayProperty(stringArrayProperty, "IntegerProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreateArrayProperty(stringArrayProperty, "StringProperty", PRIMITIVETYPE_String));
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreateArrayProperty(stringArrayProperty2, "StringProperty"));

    // Arrays overriding structs
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreateStructArrayProperty(structArrayProperty, "StructProperty", structClass2));
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreateStructArrayProperty(structArrayProperty, "StructProperty", structClass));

    ArrayECPropertyP intArrayProperty;
    // Arrays overriding arrays
    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, class1->CreateArrayProperty(intArrayProperty, "StringArrayProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreateArrayProperty(stringArrayProperty, "StringArrayProperty", PRIMITIVETYPE_String));
    class1->RemoveProperty("StringArrayProperty");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ClassTest, CanOverrideBasePropertiesInDerivedClass)
    {
    ECSchemaPtr schema;
    ECEntityClassP parent;
    ECEntityClassP derived;
    ECEntityClassP base;

    ECSchema::CreateSchema(schema, "TestSchema", 1, 0);
    schema->CreateEntityClass(base, "BaseClass");
    schema->CreateEntityClass(parent, "Parent");
    schema->CreateEntityClass(derived, "Derived");
    derived->AddBaseClass(*parent);

    PrimitiveECPropertyP derivedStringProp;
    PrimitiveECPropertyP baseIntProp;

    derived->CreatePrimitiveProperty(derivedStringProp, "Code", PRIMITIVETYPE_String);
    base->CreatePrimitiveProperty(baseIntProp, "Code", PRIMITIVETYPE_Integer);

    ECObjectsStatus status = parent->AddBaseClass(*base);
    EXPECT_NE(ECObjectsStatus::DataTypeMismatch, status);

    ECPropertyP prop = derived->GetPropertyP("Code", false);
    ASSERT_TRUE(nullptr != prop);
    PrimitiveECPropertyP primProp = prop->GetAsPrimitivePropertyP();
    EXPECT_EQ(PRIMITIVETYPE_String, primProp->GetType());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectFailureWhenStructTypeIsNotReferenced)
    {
    ECSchemaPtr schema;
    ECSchemaPtr schema2;
    ECEntityClassP class1;
    ECStructClassP structClass;
    ECStructClassP structClass2;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    ECSchema::CreateSchema(schema2, "TestSchema2", 5, 5);
    schema->CreateEntityClass(class1, "TestClass");
    schema2->CreateStructClass(structClass, "ClassForStructs");
    schema->CreateStructClass(structClass2, "ClassForStructs2");

    StructECPropertyP baseStructProp;
    StructArrayECPropertyP structArrayProperty;
    StructECPropertyP baseStructProp2;
    StructArrayECPropertyP structArrayProperty2;

    EXPECT_EQ(ECObjectsStatus::SchemaNotFound, class1->CreateStructProperty(baseStructProp, "StructProperty", *structClass));
    EXPECT_EQ(ECObjectsStatus::SchemaNotFound, class1->CreateStructArrayProperty(structArrayProperty, "StructArrayProperty", structClass));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreateStructProperty(baseStructProp2, "StructProperty2", *structClass2));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreateStructArrayProperty(structArrayProperty2, "StructArrayProperty2", structClass2));
    schema->AddReferencedSchema(*schema2);
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreateStructProperty(baseStructProp, "StructProperty", *structClass));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreateStructArrayProperty(structArrayProperty, "StructArrayProperty", structClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectPropertiesInOrder)
    {
    std::vector<Utf8CP> propertyNames;
    propertyNames.push_back("beta");
    propertyNames.push_back("gamma");
    propertyNames.push_back("delta");
    propertyNames.push_back("alpha");

    ECSchemaPtr schema;
    ECEntityClassP class1;
    PrimitiveECPropertyP property1;
    PrimitiveECPropertyP property2;
    PrimitiveECPropertyP property3;
    PrimitiveECPropertyP property4;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateEntityClass(class1, "TestClass");
    class1->CreatePrimitiveProperty(property1, "beta");
    class1->CreatePrimitiveProperty(property2, "gamma");
    class1->CreatePrimitiveProperty(property3, "delta");
    class1->CreatePrimitiveProperty(property4, "alpha");

    int i = 0;
    ECPropertyIterable  iterable = class1->GetProperties(false);
    for (ECPropertyP prop : iterable)
        {
        EXPECT_EQ(0, prop->GetName().compare(propertyNames[i]));
        i++;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectProperties)
    {
    ECSchemaPtr schema;
    ECEntityClassP ab;
    ECEntityClassP cd;
    ECEntityClassP ef;

    PrimitiveECPropertyP a;
    PrimitiveECPropertyP b;
    PrimitiveECPropertyP c;
    PrimitiveECPropertyP d;
    PrimitiveECPropertyP e;
    PrimitiveECPropertyP f;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateEntityClass(ab, "ab");
    schema->CreateEntityClass(cd, "cd");
    schema->CreateEntityClass(ef, "ef");

    ab->CreatePrimitiveProperty(a, "a");
    ab->CreatePrimitiveProperty(b, "b");

    cd->CreatePrimitiveProperty(c, "c");
    cd->CreatePrimitiveProperty(d, "d");

    ef->CreatePrimitiveProperty(e, "e");
    ef->CreatePrimitiveProperty(f, "f");

    cd->AddBaseClass(*ab);
    ef->AddBaseClass(*cd);

    EXPECT_TRUE(NULL != GetPropertyByName(*ef, "e"));
    EXPECT_TRUE(NULL != GetPropertyByName(*ef, "c"));
    EXPECT_TRUE(NULL != GetPropertyByName(*ef, "a"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectPropertiesFromBaseClass)
    {
    ECSchemaPtr schema;
    ECEntityClassP ab;
    ECEntityClassP cd;
    ECEntityClassP ef;
    ECEntityClassP gh;
    ECEntityClassP ij;
    ECEntityClassP kl;
    ECEntityClassP mn;

    PrimitiveECPropertyP a;
    PrimitiveECPropertyP b;
    PrimitiveECPropertyP c;
    PrimitiveECPropertyP d;
    PrimitiveECPropertyP e;
    PrimitiveECPropertyP f;
    PrimitiveECPropertyP g;
    PrimitiveECPropertyP h;
    PrimitiveECPropertyP i;
    PrimitiveECPropertyP j;
    PrimitiveECPropertyP k;
    PrimitiveECPropertyP l;
    PrimitiveECPropertyP m;
    PrimitiveECPropertyP n;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateEntityClass(ab, "ab");
    schema->CreateEntityClass(cd, "cd");
    schema->CreateEntityClass(ef, "ef");
    schema->CreateEntityClass(gh, "gh");
    schema->CreateEntityClass(ij, "ij");
    schema->CreateEntityClass(kl, "kl");
    schema->CreateEntityClass(mn, "mn");

    ab->CreatePrimitiveProperty(a, "a");
    ab->CreatePrimitiveProperty(b, "b");

    cd->CreatePrimitiveProperty(c, "c");
    cd->CreatePrimitiveProperty(d, "d");

    ef->CreatePrimitiveProperty(e, "e");
    ef->CreatePrimitiveProperty(f, "f");

    gh->CreatePrimitiveProperty(g, "g");
    gh->CreatePrimitiveProperty(h, "h");

    ij->CreatePrimitiveProperty(i, "i");
    ij->CreatePrimitiveProperty(j, "j");

    kl->CreatePrimitiveProperty(k, "k");
    kl->CreatePrimitiveProperty(l, "l");

    mn->CreatePrimitiveProperty(m, "m");
    mn->CreatePrimitiveProperty(n, "n");

    ef->AddBaseClass(*ab);
    ef->AddBaseClass(*cd);

    kl->AddBaseClass(*gh);
    kl->AddBaseClass(*ij);

    mn->AddBaseClass(*ef);
    mn->AddBaseClass(*kl);

    ECPropertyIterable  iterable1 = mn->GetProperties(true);
    std::vector<ECPropertyP> testVector;
    for (ECPropertyP prop : iterable1)
        testVector.push_back(prop);

    EXPECT_EQ(14, testVector.size());
    for (size_t i = 0; i < testVector.size(); i++)
        {
        Utf8Char expectedName[] = {(Utf8Char) ('a' + static_cast<Utf8Char> (i)), 0};
        EXPECT_EQ(0, testVector[i]->GetName().compare(expectedName)) << "Expected: " << expectedName << " Actual: " << testVector[i]->GetName().c_str();
        }

    // now we add some duplicate properties to mn which will "override" those from the base classes
    PrimitiveECPropertyP b2;
    PrimitiveECPropertyP d2;
    PrimitiveECPropertyP f2;
    PrimitiveECPropertyP h2;
    PrimitiveECPropertyP j2;
    PrimitiveECPropertyP k2;

    mn->CreatePrimitiveProperty(b2, "b");
    mn->CreatePrimitiveProperty(d2, "d");
    mn->CreatePrimitiveProperty(f2, "f");
    mn->CreatePrimitiveProperty(h2, "h");
    mn->CreatePrimitiveProperty(j2, "j");
    mn->CreatePrimitiveProperty(k2, "k");

    ECPropertyIterable  iterable2 = mn->GetProperties(true);
    testVector.clear();
    for (ECPropertyP prop : iterable2)
        testVector.push_back(prop);

    EXPECT_EQ(14, testVector.size());
    bvector<Utf8CP> expectedVector {"a", "c", "e", "g", "i", "l", "m", "n", "b", "d", "f", "h", "j", "k"};
    for (size_t i = 0; i < testVector.size(); i++)
        EXPECT_EQ(0, testVector[i]->GetName().compare(expectedVector[i])) << "Expected: " << expectedVector[i] << " Actual: " << testVector[i]->GetName().c_str();

    PrimitiveECPropertyP e2;
    PrimitiveECPropertyP a2;
    PrimitiveECPropertyP c2;
    PrimitiveECPropertyP g2;

    PrimitiveECPropertyP l2;
    PrimitiveECPropertyP i2;
    PrimitiveECPropertyP g3;

    PrimitiveECPropertyP a3;
    PrimitiveECPropertyP b3;
    PrimitiveECPropertyP g4;
    PrimitiveECPropertyP h3;

    kl->CreatePrimitiveProperty(e2, "e");
    kl->CreatePrimitiveProperty(a2, "a");
    kl->CreatePrimitiveProperty(c2, "c");
    kl->CreatePrimitiveProperty(g2, "g");

    ef->CreatePrimitiveProperty(l2, "l");
    gh->CreatePrimitiveProperty(i2, "i");
    ij->CreatePrimitiveProperty(g3, "g");

    gh->CreatePrimitiveProperty(a3, "a");
    gh->CreatePrimitiveProperty(b3, "b");
    ab->CreatePrimitiveProperty(g4, "g");
    ab->CreatePrimitiveProperty(h3, "h");

    ECPropertyIterable  iterable3 = mn->GetProperties(true);
    testVector.clear();
    for (ECPropertyP prop : iterable3)
        testVector.push_back(prop);

    EXPECT_EQ(14, testVector.size());
    expectedVector = {"a", "g", "c", "e", "l", "i", "m", "n", "b", "d", "f", "h", "j", "k"};
    for (size_t i = 0; i < testVector.size(); i++)
        EXPECT_EQ(0, testVector[i]->GetName().compare(expectedVector[i])) << "Expected: " << expectedVector[i] << " Actual: " << testVector[i]->GetName().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, AddAndRemoveConstraintClasses)
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    ECSchema::CreateSchema(refSchema, "RefSchema", 5, 5);

    ECRelationshipClassP relClass;
    ECEntityClassP targetClass;
    ECEntityClassP sourceClass;

    schema->CreateRelationshipClass(relClass, "RElationshipClass");
    schema->CreateEntityClass(targetClass, "Target");
    refSchema->CreateEntityClass(sourceClass, "Source");

    EXPECT_EQ(ECObjectsStatus::Success, relClass->GetTarget().AddClass(*targetClass));
    EXPECT_EQ(ECObjectsStatus::SchemaNotFound, relClass->GetSource().AddClass(*sourceClass));

    schema->AddReferencedSchema(*refSchema);
    EXPECT_EQ(ECObjectsStatus::Success, relClass->GetSource().AddClass(*sourceClass));

    EXPECT_EQ(ECObjectsStatus::Success, relClass->GetTarget().RemoveClass(*targetClass));
    EXPECT_EQ(ECObjectsStatus::ClassNotFound, relClass->GetTarget().RemoveClass(*targetClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, ExpectReadOnlyFromBaseClass)
    {
    ECSchemaPtr schema;
    ECEntityClassP child;
    ECEntityClassP base;

    PrimitiveECPropertyP readOnlyProp;

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateEntityClass(base, "BaseClass");
    schema->CreateEntityClass(child, "ChildClass");

    base->CreatePrimitiveProperty(readOnlyProp, "readOnlyProp");
    readOnlyProp->SetIsReadOnly(true);

    ASSERT_EQ(ECObjectsStatus::Success, child->AddBaseClass(*base));

    ECPropertyP ecProp = GetPropertyByName(*child, "readOnlyProp");
    ASSERT_EQ(true, ecProp->GetIsReadOnly());
    }

void TestOverriding(Utf8CP schemaName, int majorVersion, bool allowOverriding)
    {
    ECSchemaPtr schema;
    ECEntityClassP base;
    ECEntityClassP child;

    ECSchema::CreateSchema(schema, schemaName, majorVersion, 5);
    schema->CreateEntityClass(base, "base");
    schema->CreateEntityClass(child, "child");

    PrimitiveECPropertyP baseIntProp;
    ArrayECPropertyP baseIntArrayProperty;
    ArrayECPropertyP baseStringArrayProperty;
    ArrayECPropertyP baseBoolArrayProperty;

    base->CreatePrimitiveProperty(baseIntProp, "IntegerProperty", PRIMITIVETYPE_Integer);
    base->CreateArrayProperty(baseIntArrayProperty, "IntArrayProperty", PRIMITIVETYPE_Integer);
    base->CreateArrayProperty(baseStringArrayProperty, "StringArrayProperty", PRIMITIVETYPE_String);
    base->CreateArrayProperty(baseBoolArrayProperty, "BoolArrayProperty", PRIMITIVETYPE_Boolean);

    PrimitiveECPropertyP childIntProperty;
    ArrayECPropertyP childIntArrayProperty;
    ArrayECPropertyP childStringArrayProperty;
    ArrayECPropertyP childBoolArrayProperty;

    child->AddBaseClass(*base);
    // Override an integer property with an array of ints
    ECObjectsStatus status = child->CreateArrayProperty(childIntArrayProperty, "IntegerProperty", PRIMITIVETYPE_Integer);
    if (allowOverriding)
        {
        ASSERT_EQ(ECObjectsStatus::Success, status);
        child->RemoveProperty("IntegerProperty");
        }
    else
        ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, status);

    // Override an integer property with an array of strings
    status = child->CreateArrayProperty(childStringArrayProperty, "IntegerProperty", PRIMITIVETYPE_String);
    ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, status);

    // Override an integer array with an integer
    status = child->CreatePrimitiveProperty(childIntProperty, "IntArrayProperty", PRIMITIVETYPE_Integer);
    if (allowOverriding)
        {
        ASSERT_EQ(ECObjectsStatus::Success, status);
        child->RemoveProperty("IntArrayProperty");
        }
    else
        ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, status);

    // Override an array of strings with an integer
    status = child->CreatePrimitiveProperty(childIntProperty, "StringArrayProperty", PRIMITIVETYPE_Integer);
    ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, status);

    // Override an array of boolean with an array of integers
    status = child->CreateArrayProperty(childIntArrayProperty, "BoolArrayProperty", PRIMITIVETYPE_Integer);
    ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, status);

    // Override an array of integers with an array of boolean
    status = child->CreateArrayProperty(childBoolArrayProperty, "IntArrayProperty", PRIMITIVETYPE_Boolean);
    ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, status);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ClassTest, TestOverridingArrayPropertyWithNonArray)
    {
    TestOverriding("TestSchema", 5, false);
    TestOverriding("jclass", 1, true);
    TestOverriding("jclass", 2, true);
    TestOverriding("ECXA_ams", 1, true);
    TestOverriding("ECXA_ams_user", 1, true);
    TestOverriding("ams", 1, true);
    TestOverriding("ams_user", 1, true);
    TestOverriding("Bentley_JSpace_CustomAttributes", 2, true);
    TestOverriding("Bentley_Plant", 6, true);
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
        ECSchema::CreateSchema(schema, "MySchema", 1, 1);
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

    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::LatestCompatible));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaNotTest", 1, 0), SchemaMatchType::LatestCompatible));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 2, 0), SchemaMatchType::LatestCompatible));
    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 1).Matches(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::LatestCompatible));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 1), SchemaMatchType::LatestCompatible));
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

    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::LatestCompatible));
    EXPECT_TRUE(SchemaKey("SchemaTesa", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SchemaMatchType::LatestCompatible));
    EXPECT_TRUE(SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 2, 0), SchemaMatchType::LatestCompatible));
    EXPECT_FALSE(SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 9), SchemaMatchType::LatestCompatible));
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

    ECSchema::CreateSchema(schema1, "Widget", 5, 1);
    ECSchema::CreateSchema(schema2, "BaseSchema1", 2, 0);
    ECSchema::CreateSchema(schema3, "BaseSchema2", 5, 5);

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

    ECSchema::CreateSchema(schema1, "Widget", 5, 1);
    ECSchema::CreateSchema(schema2, "BaseSchema1", 2, 0);
    ECSchema::CreateSchema(schema3, "BaseSchema2", 5, 5);

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

    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseSchema2", 5, 5), SchemaMatchType::LatestCompatible) != NULL);
    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseZchema2", 5, 5), SchemaMatchType::LatestCompatible) == NULL);
    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseSchema2", 3, 5), SchemaMatchType::LatestCompatible) == NULL);
    EXPECT_TRUE(cache->GetSchema(SchemaKey("BaseSchema2", 5, 3), SchemaMatchType::LatestCompatible) != NULL);
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

    ECSchema::CreateSchema(schema1, "Widget", 5, 1);
    ECSchema::CreateSchema(schema2, "BaseSchema1", 2, 0);
    ECSchema::CreateSchema(schema3, "BaseSchema2", 5, 5);

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
    EXPECT_EQ(schema->SetNamespacePrefix("Some new prefix"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->SetDescription("Some new description"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->SetDisplayLabel("Some new label"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ(schema->SetVersionMajor(13), ECObjectsStatus::SchemaIsImmutable);
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

    SchemaWriteStatus statusW = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(L"dgn-testingonly-result.02.00.ecschema.xml").c_str(), 2, 0, false);
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

    SchemaKey schemaKey("Bentley_Standard_CustomAttributes", 1, 12);
    ECSchemaPtr standardCASchema = schemaContext->LocateSchema(schemaKey, SchemaMatchType::Latest);
    EXPECT_TRUE(standardCASchema.IsValid());

    ECSchemaCachePtr cache = ECSchemaCache::Create();
    ECSchemaPtr schema;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 2, 0, 1);
    schema->AddReferencedSchema(*standardCASchema);
    ASSERT_EQ(ECObjectsStatus::Success, schema->SetIsDynamicSchema(true));

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

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);

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

    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    ECSchema::CreateSchema(refSchema, "RefSchema", 5, 5);

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
        "                    displayLabel='My KindOfQuantity' persistenceUnit='CENTIMETRE' precision='10'"
        "                    defaultPresentationUnit='FOOT' alternativePresentationUnits='INCH;YARD' />"
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Robert.Schili                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaTest, SetGetMinMaxInt)
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Robert.Schili                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaTest, SetGetMaxLength)
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
    ASSERT_EQ(primp->IsMaximumLengthDefined(), false);

    ASSERT_EQ(primp->SetMaximumLength(42), ECObjectsStatus::Success);
    ASSERT_EQ(primp->IsMaximumLengthDefined(), true);
    ASSERT_EQ(primp->GetMaximumLength(), 42);

    primp->ResetMaximumLength();
    ASSERT_EQ(primp->IsMaximumLengthDefined(), false);
    ASSERT_EQ(primp->GetMaximumLength(), 0);
    }

END_BENTLEY_ECN_TEST_NAMESPACE
