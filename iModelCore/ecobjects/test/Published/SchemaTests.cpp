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

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

// NEEDSWORK Improve strategy for seed data.  Should not be maintained in source.
#define SCHEMAS_PATH  L"" 

struct SchemaTest : ECTestFixture {};
struct SchemaSearchTest : ECTestFixture {};
struct SchemaNameParsingTest : ECTestFixture {};
struct SchemaDeserializationTest : ECTestFixture {};
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
    void TestPropertyCount (ECClassCR ecClass, size_t nPropertiesWithoutBaseClasses, size_t nPropertiesWithBaseClasses)
        {
        EXPECT_EQ (ecClass.GetPropertyCount (false), nPropertiesWithoutBaseClasses);
        EXPECT_EQ (ecClass.GetPropertyCount (true), nPropertiesWithBaseClasses);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP GetPropertyByName (ECClassCR ecClass, Utf8CP name, bool expectExists = true)
    {
    ECPropertyP prop = ecClass.GetPropertyP (name);
    EXPECT_EQ (expectExists, NULL != prop);
    Utf8String utf8 (name);
    prop = ecClass.GetPropertyP (utf8.c_str ());
    EXPECT_EQ (expectExists, NULL != prop);
    return prop;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void VerifyWidgetsSchema (ECSchemaPtr const&   schema)
    {
    EXPECT_STREQ ("Widgets", schema->GetName ().c_str ());
    EXPECT_STREQ ("wid", schema->GetNamespacePrefix ().c_str ());
    EXPECT_STREQ ("Widgets Display Label", schema->GetDisplayLabel ().c_str ());
    EXPECT_TRUE (schema->GetIsDisplayLabelDefined ());
    EXPECT_STREQ ("Widgets Description", schema->GetDescription ().c_str ());
    EXPECT_EQ (9, schema->GetVersionMajor ());
    EXPECT_EQ (6, schema->GetVersionMinor ());

#ifdef DEBUG_PRINT
    for (ECClassP pClass: schema->GetClasses())
        {
        printf ("Widgets contains class: '%s' with display label '%s'\n", pClass->GetName().c_str(), pClass->GetDisplayLabel().c_str());
        }
#endif

    ECClassP pClass = schema->GetClassP ("ClassDoesNotExistInSchema");
    EXPECT_FALSE (pClass);

    pClass = schema->GetClassP ("ecProject");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ ("ecProject", pClass->GetName ().c_str ());
    EXPECT_STREQ ("Project", pClass->GetDisplayLabel ().c_str ());
    EXPECT_TRUE (pClass->GetIsDisplayLabelDefined ());
    EXPECT_STREQ ("Project Class", pClass->GetDescription ().c_str ());
    EXPECT_TRUE (pClass->IsEntityClass());
    EXPECT_FALSE (pClass->HasBaseClasses ());
    ECPropertyP pProperty = GetPropertyByName (*pClass, "Name");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("Name", pProperty->GetName ().c_str ());
    EXPECT_TRUE (pProperty->GetIsPrimitive ());
    EXPECT_FALSE (pProperty->GetIsStruct ());
    EXPECT_FALSE (pProperty->GetIsArray ());
    EXPECT_STREQ ("string", pProperty->GetTypeName ().c_str ());
    EXPECT_TRUE (PRIMITIVETYPE_String == pProperty->GetAsPrimitiveProperty ()->GetType ());
    EXPECT_TRUE (pProperty->GetIsDisplayLabelDefined ());
    EXPECT_STREQ ("Project Name", pProperty->GetDisplayLabel ().c_str ());
    EXPECT_STREQ ("", pProperty->GetDescription ().c_str ());
    EXPECT_EQ (pClass, &pProperty->GetClass ());
    EXPECT_FALSE (pProperty->GetIsReadOnly ());

    pProperty = GetPropertyByName (*pClass, "PropertyDoesNotExistInClass", false);
    EXPECT_FALSE (pProperty);

    ECClassP customAttribClass = schema->GetClassP ("AccessCustomAttributes");
    ASSERT_TRUE (NULL != customAttribClass);
    EXPECT_STREQ ("AccessCustomAttributes", customAttribClass->GetName ().c_str ());
    EXPECT_STREQ ("AccessCustomAttributes", customAttribClass->GetDisplayLabel ().c_str ());
    EXPECT_FALSE (customAttribClass->GetIsDisplayLabelDefined ());
    EXPECT_STREQ ("", customAttribClass->GetDescription ().c_str ());
    EXPECT_TRUE (customAttribClass->IsCustomAttributeClass());
    EXPECT_FALSE (customAttribClass->HasBaseClasses ());

    pClass = schema->GetClassP ("Struct1");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ ("Struct1", pClass->GetName ().c_str ());
    EXPECT_STREQ ("Struct1", pClass->GetDisplayLabel ().c_str ());
    EXPECT_FALSE (pClass->GetIsDisplayLabelDefined ());
    EXPECT_STREQ ("", pClass->GetDescription ().c_str ());
    EXPECT_TRUE (pClass->IsStructClass());
    EXPECT_FALSE (pClass->HasBaseClasses ());

    pClass = schema->GetClassP ("Struct2");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ ("Struct2", pClass->GetName ().c_str ());
    EXPECT_STREQ ("Struct2", pClass->GetDisplayLabel ().c_str ());
    EXPECT_FALSE (pClass->GetIsDisplayLabelDefined ());
    EXPECT_STREQ ("", pClass->GetDescription ().c_str ());
    EXPECT_TRUE (pClass->IsStructClass());
    EXPECT_FALSE (pClass->HasBaseClasses ());
    pProperty = GetPropertyByName (*pClass, "NestedArray");
    EXPECT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("NestedArray", pProperty->GetName ().c_str ());
    EXPECT_FALSE (pProperty->GetIsPrimitive ());
    EXPECT_FALSE (pProperty->GetIsStruct ());
    EXPECT_TRUE (pProperty->GetIsArray ());
    EXPECT_STREQ ("Struct1", pProperty->GetTypeName ().c_str ());
    ArrayECPropertyP arrayProperty = pProperty->GetAsArrayPropertyP ();
    EXPECT_TRUE (ARRAYKIND_Struct == arrayProperty->GetKind ());
    EXPECT_EQ (schema->GetClassP ("Struct1"), arrayProperty->GetAsStructArrayPropertyP()->GetStructElementType ());
    EXPECT_EQ (0, arrayProperty->GetMinOccurs ());
    EXPECT_EQ (UINT_MAX, arrayProperty->GetMaxOccurs ());
    EXPECT_FALSE (pProperty->GetIsDisplayLabelDefined ());
    EXPECT_STREQ ("NestedArray", pProperty->GetDisplayLabel ().c_str ());
    EXPECT_STREQ ("", pProperty->GetDescription ().c_str ());
    EXPECT_EQ (pClass, &pProperty->GetClass ());
    EXPECT_FALSE (pProperty->GetIsReadOnly ());

    pClass = schema->GetClassP ("TestClass");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_TRUE (pClass->HasBaseClasses ());
    pProperty = GetPropertyByName (*pClass, "EmbeddedStruct");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("EmbeddedStruct", pProperty->GetName ().c_str ());
    EXPECT_FALSE (pProperty->GetIsPrimitive ());
    EXPECT_TRUE (pProperty->GetIsStruct ());
    EXPECT_FALSE (pProperty->GetIsArray ());
    EXPECT_STREQ ("Struct1", pProperty->GetTypeName ().c_str ());
    StructECPropertyP structProperty = pProperty->GetAsStructPropertyP ();
    EXPECT_EQ (schema->GetClassP ("Struct1"), &(structProperty->GetType ()));
    EXPECT_FALSE (pProperty->GetIsDisplayLabelDefined ());
    EXPECT_STREQ ("EmbeddedStruct", pProperty->GetDisplayLabel ().c_str ());
    EXPECT_STREQ ("", pProperty->GetDescription ().c_str ());
    EXPECT_EQ (pClass, &pProperty->GetClass ());
    EXPECT_FALSE (pProperty->GetIsReadOnly ());

    IECInstancePtr instance = pClass->GetCustomAttribute (*customAttribClass);
    EXPECT_TRUE (instance.IsValid ());

    ECValue ecValue;
    EXPECT_EQ (ECObjectsStatus::Success, instance->GetValue (ecValue, "AccessLevel"));
    EXPECT_EQ (4, ecValue.GetInteger ());

    EXPECT_EQ (ECObjectsStatus::Success, instance->GetValue (ecValue, "Writeable"));
    EXPECT_FALSE (ecValue.GetBoolean ());

#ifdef DEBUG_PRINT
    for (ECPropertyP pProperty: pClass->GetProperties())
        {
        printf ("TestClass contains property: %s of type %s\n", pProperty->GetName().c_str(), pProperty->GetTypeName().c_str());
        }
#endif   
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaTest, ExpectReadOnly)
    {
    ECSchemaPtr schema;
    ECEntityClassP domainClass;
    ECEntityClassP derivedClass;
    ECStructClassP structClass;
    ECCustomAttributeClassP customAttributeClass;
    ECEnumerationP enumeration;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    ASSERT_TRUE (schema.IsValid ());

    //Create Domain Class
    schema->CreateEntityClass (domainClass, "DomainClass");
    ASSERT_TRUE (domainClass != NULL);

    //Create Derived Class
    schema->CreateEntityClass (derivedClass, "DerivedClass");
    ASSERT_TRUE (derivedClass != NULL);

    //Create Struct
    schema->CreateStructClass (structClass, "StructClass");
    ASSERT_TRUE (structClass != NULL);

    //Create Enumeration
    schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_Integer);
    ASSERT_TRUE(enumeration != nullptr);

    //Add Property of Array type to structClass
    ArrayECPropertyP MyArrayProp;
    structClass->CreateArrayProperty (MyArrayProp, "ArrayProperty");
    ASSERT_TRUE (MyArrayProp != NULL);

    //Create customAttributeClass
    schema->CreateCustomAttributeClass (customAttributeClass, "CustomAttribute");
    ASSERT_TRUE (customAttributeClass != NULL);

    //Add Property Of Struct type to custom attribute
    StructECPropertyP PropertyOfCustomAttribute;
    customAttributeClass->CreateStructProperty (PropertyOfCustomAttribute, "PropertyOfCustomAttribute", *structClass);
    ASSERT_TRUE (PropertyOfCustomAttribute != NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaTest, AddAndRemoveEnumerations)
    {
    ECSchemaPtr schema;
    ECEntityClassP domainClass;
    ECEnumerationP enumeration;
    ECEnumerationP enumeration2;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    ASSERT_TRUE (schema.IsValid ());

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
TEST_F (SchemaTest, TestPrimitiveEnumerationProperty)
    {
    ECSchemaPtr schema;
    ECEntityClassP domainClass;
    ECEnumerationP enumeration;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    ASSERT_TRUE (schema.IsValid ());

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
TEST_F (SchemaTest, CheckEnumerationBasicProperties)
    {
    ECSchemaPtr schema;
    ECEnumerationP enumeration;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    ASSERT_TRUE (schema.IsValid ());

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
        else if(i == 1)
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
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext ();
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, schemaXML, *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);

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
TEST_F (SchemaSearchTest, FindSchemaByName)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext ();
    WString seedPath (ECTestFixture::GetTestDataPath (L"").c_str ());
    schemaContext->AddSchemaPath (seedPath.c_str ());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"SchemaThatReferences.01.00.ecschema.xml").c_str (), *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);

    EXPECT_TRUE (schema->FindSchema (SchemaKey ("SchemaThatReferences", 1, 0), SchemaMatchType::Exact) != NULL);
    EXPECT_TRUE (schema->FindSchema (SchemaKey ("SchemaThatReferencez", 1, 0), SchemaMatchType::Exact) == NULL);
    EXPECT_TRUE (schema->FindSchema (SchemaKey ("SchemaThatReferences", 2, 0), SchemaMatchType::Exact) == NULL);
    EXPECT_TRUE (schema->FindSchema (SchemaKey ("SchemaThatReferences", 1, 1), SchemaMatchType::Exact) == NULL);

    EXPECT_TRUE (schema->FindSchema (SchemaKey ("BaseSchema", 1, 0), SchemaMatchType::Exact) != NULL);
    EXPECT_TRUE (schema->FindSchemaP (SchemaKey ("SchemaThatReferences", 1, 0), SchemaMatchType::Exact) != NULL);
    EXPECT_TRUE (schema->FindSchemaP (SchemaKey ("a", 123, 456), SchemaMatchType::Exact) == NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static  void    ValidateSchemaNameParsing (Utf8CP fullName, bool expectFailure, Utf8CP expectName, uint32_t expectMajor, uint32_t expectWrite, uint32_t expectMinor)
    {
    Utf8String    shortName;
    uint32_t   versionMajor;
    uint32_t   versionWrite;
    uint32_t   versionMinor;

    ECObjectsStatus status = ECSchema::ParseSchemaFullName (shortName, versionMajor, versionWrite, versionMinor, fullName);

    if (expectFailure)
        {
        EXPECT_TRUE (ECObjectsStatus::Success != status);
        return;
        }

    EXPECT_TRUE (ECObjectsStatus::Success == status);

    EXPECT_STREQ (shortName.c_str (), expectName);
    EXPECT_EQ (versionMajor, expectMajor);
    EXPECT_EQ(versionWrite, expectWrite);
    EXPECT_EQ (versionMinor, expectMinor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaNameParsingTest, ParseFullSchemaName)
    {
    ValidateSchemaNameParsing ("TestName.6.8", false, "TestName", 6, 0, 8);
    ValidateSchemaNameParsing ("TestName.16.18", false, "TestName", 16, 0, 18);
    ValidateSchemaNameParsing ("TestName.126.128", false, "TestName", 126, 0, 128);
    ValidateSchemaNameParsing ("TestName.1267.128", false, "TestName", 1267, 0, 128);
    ValidateSchemaNameParsing ("TestName.1267", true, NULL, 0, 0, 0);
    ValidateSchemaNameParsing ("TestName", true, NULL, 0, 0, 0);
    ValidateSchemaNameParsing ("", true, NULL, 0, 0, 0);
    ValidateSchemaNameParsing ("12.18", true, NULL, 0, 0, 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F (SchemaDeserializationTest, TestAbstractness)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Abstract' version='01.00' displayLabel='AbstractClasses' description='Schema with abstract class' nameSpacePrefix='ab' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='Abstract1' description='Abstract ECClass' displayLabel='Abstract' isDomainClass='false' isCustomAttributeClass='false' isStruct='false'>"
        "       <ECProperty propertyName='Name' typeName='string' displayLabel='Project Name' />"
        "    </ECClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECClassCP absClass = schema->GetClassCP("Abstract1");
    ASSERT_TRUE(absClass->IsEntityClass());
    ASSERT_TRUE(ECClassModifier::Abstract == absClass->GetClassModifier());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2015
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, InvalidStructArrayPropertySpecification)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='StructArray' version='01.00' displayLabel='StructArrays' description='Schema with invalid XML for a struct array property' nameSpacePrefix='sa' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='Ent1' description='Entity ECClass' displayLabel='Entity' isDomainClass='true' isCustomAttributeClass='false' isStruct='false'>"
        "        <ECArrayProperty propertyName = 'TypeReferences' typeName = 'string' minOccurs = '0' maxOccurs = 'unbounded' isStruct = 'True' />" 
        "    </ECClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    ECClassP ent = schema->GetClassP("Ent1");
    ASSERT_TRUE(ent->IsEntityClass());
    ECPropertyP prop = ent->GetPropertyP("TypeReferences");
    ASSERT_TRUE(nullptr != prop);
    StructArrayECPropertyCP typeReferences1 = prop->GetAsStructArrayProperty();
    ASSERT_TRUE(nullptr == typeReferences1);

    ArrayECPropertyCP typeReferences2 = prop->GetAsArrayProperty();
    ASSERT_TRUE(nullptr != typeReferences2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, CaseSensitivity)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='CaseInsensitive' version='01.00' displayLabel='Case Insensitive' description='Testing case sensitivity with struct names and custom attributes' nameSpacePrefix='cs' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
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
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    ECClassP ent = schema->GetClassP("Entity");
    ASSERT_TRUE(nullptr != ent);
    ECPropertyP prop = ent->GetPropertyP("StructProp");
    ASSERT_TRUE(nullptr != prop);
    StructECPropertyP structProp = prop->GetAsStructPropertyP();
    ASSERT_TRUE(nullptr != structProp);
    EXPECT_TRUE(ent->IsDefined("CustomAttrib"));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectSuccessWhenRoundtripUsingFile)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"Widgets.01.00.ecschema.xml").c_str (), *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);
    VerifyWidgetsSchema (schema);

    SchemaWriteStatus status2 = schema->WriteToXmlFile (ECTestFixture::GetTempDataPath (L"widgets.xml").c_str ());
    EXPECT_EQ (SchemaWriteStatus::Success, status2);

    ECSchemaPtr deserializedSchema;
    schemaContext = ECSchemaReadContext::CreateContext ();
    status = ECSchema::ReadFromXmlFile (deserializedSchema, ECTestFixture::GetTempDataPath (L"widgets.xml").c_str (), *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);
    VerifyWidgetsSchema (deserializedSchema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectErrorWhenXmlFileDoesNotExist)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"ThisFileIsntReal.xml").c_str (), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::FailedToParseXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectErrorWhenXmlFileIsMissingNodes)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);
    DISABLE_ASSERTS
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"MissingNodes.01.00.ecschema.xml").c_str (), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::FailedToParseXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectErrorWhenXmlFileIsIllFormed)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);
    DISABLE_ASSERTS
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"IllFormedXml.01.00.ecschema.xml").c_str (), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::FailedToParseXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectErrorWhenXmlFileIsMissingECSchemaNode)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"MissingECSchemaNode.01.00.ecschema.xml").c_str (), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::InvalidECSchemaXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectErrorWhenXmlFileIsMissingNamespace)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"MissingNamespace.01.00.ecschema.xml").c_str (), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::InvalidECSchemaXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectSuccessWhenCustomAttributeIsMissingNamespace)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"MissingNamespaceOnCustomAttribute.01.00.ecschema.xml").c_str (), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::Success, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectErrorWhenXmlFileHasUnsupportedNamespace)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"UnsupportedECXmlNamespace.01.00.ecschema.xml").c_str (), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::InvalidECSchemaXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectErrorWhenRelationshipEndpointNotFound)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"BadRelationship.01.00.ecschema.xml").c_str (), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::InvalidECSchemaXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectSuccessWithDuplicateNamespacePrefixes)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"DuplicatePrefixes.01.00.ecschema.xml").c_str (), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::Success, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectErrorWhenXmlFileHasMissingSchemaNameAttribute)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"MissingSchemaName.01.00.ecschema.xml").c_str (), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::InvalidECSchemaXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectErrorWhenXmlFileHasMissingClassNameAttribute)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"MissingClassName.01.00.ecschema.xml").c_str (), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::InvalidECSchemaXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectSuccessWhenXmlFileHasInvalidVersionString)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"InvalidVersionString.01.00.ecschema.xml").c_str (), *schemaContext);
    EXPECT_EQ (1, schema->GetVersionMajor ());
    EXPECT_EQ (0, schema->GetVersionMinor ());

    EXPECT_EQ (SchemaReadStatus::Success, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectFailureWhenMissingTypeNameInProperty)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Widgets' version='09.06' displayLabel='Widgets Display Label' description='Widgets Description' nameSpacePrefix='wid' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='ecProject' description='Project ECClass' displayLabel='Project' isDomainClass='True'>"
        "       <ECProperty propertyName='Name' typename='string' displayLabel='Project Name' />" // typename is mis-capitalized
        "    </ECClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, schemaXML, *schemaContext);

    EXPECT_EQ (SchemaReadStatus::InvalidECSchemaXml, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectUnrecognizedTypeNamesToSurviveRoundtrip)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='a' version='23.42' nameSpacePrefix='a' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='c'>"
        "       <ECProperty      propertyName='p' typeName='foobar'  />"
        "       <ECArrayProperty propertyName='q' typeName='barfood' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, schemaXML, *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);

    Utf8String ecSchemaXml;
    SchemaWriteStatus writeStatus = schema->WriteToXmlString (ecSchemaXml);
    EXPECT_EQ (SchemaWriteStatus::Success, writeStatus);

    EXPECT_NE (Utf8String::npos, ecSchemaXml.find ("typeName=\"foobar\""));
    EXPECT_NE (Utf8String::npos, ecSchemaXml.find ("typeName=\"barfood\""));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectSuccessWithInvalidTypeNameInPrimitiveProperty)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Widgets' version='09.06' displayLabel='Widgets Display Label' description='Widgets Description' nameSpacePrefix='wid' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECClass typeName='ecProject' description='Project ECClass' displayLabel='Project' isDomainClass='True'>"
        "       <ECProperty propertyName='Name' typeName='strng' displayLabel='Project Name' />"
        "    </ECClass>"
        "</ECSchema>";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, schemaXML, *schemaContext);

    EXPECT_EQ (SchemaReadStatus::Success, status);

    ECClassP pClass = schema->GetClassP ("ecProject");
    ECPropertyP pProperty = GetPropertyByName (*pClass, "Name");
    EXPECT_TRUE (PRIMITIVETYPE_String == pProperty->GetAsPrimitiveProperty ()->GetType ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectSuccessWithEmptyCustomAttribute)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    //schemaContext->AddSchemaPath(L"C:\\temp\\data\\ECXA\\SchemasAndDgn\');
    //SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, L"C:\\temp\\data\\ECXA\\SchemasAndDgn\\Bentley_Plant.06.00.ecschema.xml", *schemaContext);
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"EmptyCustomAttribute.01.00.ecschema.xml").c_str (), *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);

    Utf8String ecSchemaXmlString;
    SchemaWriteStatus status2 = schema->WriteToXmlString (ecSchemaXmlString);
    EXPECT_EQ (SchemaWriteStatus::Success, status2);
    EXPECT_NE (Utf8String::npos, ecSchemaXmlString.find ("<Relationship/>"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectSuccessWhenDeserializingSchemaWithBaseClassInReferencedFile)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();
    WString seedPath (ECTestFixture::GetTestDataPath (L"").c_str ());
    schemaContext->AddSchemaPath (seedPath.c_str ());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"SchemaThatReferences.01.00.ecschema.xml").c_str (), *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);

    ECClassP pClass = schema->GetClassP ("circle");
    ASSERT_TRUE (NULL != pClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectSuccessWhenDeserializingSchemaWithEnumerationInReferencedFile)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();
    WString seedPath (ECTestFixture::GetTestDataPath (L"").c_str ());
    schemaContext->AddSchemaPath (seedPath.c_str ());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"EnumInReferencedSchema.01.00.01.ecschema.xml").c_str (), *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);

    ASSERT_TRUE(schema.IsValid());

    ECClassP pClass = schema->GetClassP ("Entity");
    ASSERT_TRUE (nullptr != pClass);

    ECPropertyP p = pClass->GetPropertyP("EnumeratedProperty");
    ASSERT_TRUE(p != nullptr);

    PrimitiveECPropertyCP prim = p->GetAsPrimitiveProperty();
    ASSERT_TRUE(prim != nullptr);

    ECEnumerationCP ecEnum = prim->GetEnumeration();
    ASSERT_TRUE(ecEnum != nullptr);

    ASSERT_TRUE(ecEnum->GetSchema().GetVersionWrite() == 12);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectSuccessWhenECSchemaContainsOnlyRequiredAttributes)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"OnlyRequiredECSchemaAttributes.01.00.ecschema.xml").c_str (), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::Success, status);
    EXPECT_STREQ ("OnlyRequiredECSchemaAttributes", schema->GetName ().c_str ());
    EXPECT_STREQ ("", schema->GetNamespacePrefix ().c_str ());
    EXPECT_STREQ ("OnlyRequiredECSchemaAttributes", schema->GetDisplayLabel ().c_str ());
    EXPECT_FALSE (schema->GetIsDisplayLabelDefined ());
    EXPECT_STREQ ("", schema->GetDescription ().c_str ());
    EXPECT_EQ (1, schema->GetVersionMajor ());
    EXPECT_EQ (0, schema->GetVersionMinor ());

    ECClassP pClass = schema->GetClassP ("OnlyRequiredECClassAttributes");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ ("OnlyRequiredECClassAttributes", pClass->GetName ().c_str ());
    EXPECT_STREQ ("OnlyRequiredECClassAttributes", pClass->GetDisplayLabel ().c_str ());
    EXPECT_FALSE (pClass->GetIsDisplayLabelDefined ());
    EXPECT_STREQ ("", pClass->GetDescription ().c_str ());
    EXPECT_TRUE (pClass->IsEntityClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectSuccessWhenDeserializingWidgetsECSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"Widgets.01.00.ecschema.xml").c_str (), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::Success, status);
    VerifyWidgetsSchema (schema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectSuccessWhenDeserializingECSchemaFromString)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Widgets' version='09.06' displayLabel='Widgets Display Label' description='Widgets Description' nameSpacePrefix='wid' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='ecProject' description='Project ECClass' displayLabel='Project' isDomainClass='True'>"
        "       <ECProperty propertyName='Name' typeName='string' displayLabel='Project Name' />"
        "       <ECProperty propertyName='Geometry' typeName='Bentley.Geometry.Common.IGeometry' displayLabel='Geometry' />"
        "       <ECProperty propertyName='LineSegment' typeName='Bentley.Geometry.Common.ILineSegment' displayLabel='Line Segment' />"
        "    </ECClass>"
        "    <ECEnumeration typeName='Enumeration' backingTypeName='int' description='desc' displayLabel='dl'/>"
        "</ECSchema>";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, schemaXML, *schemaContext);

    EXPECT_EQ (SchemaReadStatus::Success, status);
    EXPECT_STREQ ("Widgets", schema->GetName ().c_str ());
    EXPECT_STREQ ("wid", schema->GetNamespacePrefix ().c_str ());
    EXPECT_STREQ ("Widgets Display Label", schema->GetDisplayLabel ().c_str ());
    EXPECT_TRUE (schema->GetIsDisplayLabelDefined ());
    EXPECT_STREQ ("Widgets Description", schema->GetDescription ().c_str ());
    EXPECT_EQ (9, schema->GetVersionMajor ());
    EXPECT_EQ (6, schema->GetVersionMinor ());

    ECClassP pClass = schema->GetClassP ("ClassDoesNotExistInSchema");
    EXPECT_FALSE (pClass);

    pClass = schema->GetClassP ("ecProject");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ ("ecProject", pClass->GetName ().c_str ());
    EXPECT_STREQ ("Project", pClass->GetDisplayLabel ().c_str ());
    EXPECT_TRUE (pClass->GetIsDisplayLabelDefined ());
    EXPECT_STREQ ("Project ECClass", pClass->GetDescription ().c_str ());
    EXPECT_TRUE (pClass->IsEntityClass());
    EXPECT_FALSE (pClass->HasBaseClasses ());
    ECPropertyP pProperty = GetPropertyByName (*pClass, "Name");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("Name", pProperty->GetName ().c_str ());
    EXPECT_TRUE (pProperty->GetIsPrimitive ());
    EXPECT_FALSE (pProperty->GetIsStruct ());
    EXPECT_FALSE (pProperty->GetIsArray ());
    EXPECT_STREQ ("string", pProperty->GetTypeName ().c_str ());
    EXPECT_TRUE (PRIMITIVETYPE_String == pProperty->GetAsPrimitiveProperty ()->GetType ());
    EXPECT_TRUE (pProperty->GetIsDisplayLabelDefined ());
    EXPECT_STREQ ("Project Name", pProperty->GetDisplayLabel ().c_str ());
    EXPECT_STREQ ("", pProperty->GetDescription ().c_str ());
    EXPECT_EQ (pClass, &pProperty->GetClass ());
    EXPECT_FALSE (pProperty->GetIsReadOnly ());

    pProperty = pClass->GetPropertyP ("Geometry");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("Geometry", pProperty->GetName ().c_str ());
    EXPECT_TRUE (pProperty->GetIsPrimitive ());
    EXPECT_FALSE (pProperty->GetIsStruct ());
    EXPECT_FALSE (pProperty->GetIsArray ());
    EXPECT_STREQ ("Bentley.Geometry.Common.IGeometry", pProperty->GetTypeName ().c_str ());
    EXPECT_TRUE (PRIMITIVETYPE_IGeometry == pProperty->GetAsPrimitiveProperty ()->GetType ());
    EXPECT_TRUE (pProperty->GetIsDisplayLabelDefined ());
    EXPECT_STREQ ("Geometry", pProperty->GetDisplayLabel ().c_str ());
    EXPECT_STREQ ("", pProperty->GetDescription ().c_str ());
    EXPECT_EQ (pClass, &pProperty->GetClass ());
    EXPECT_FALSE (pProperty->GetIsReadOnly ());

    pProperty = pClass->GetPropertyP ("LineSegment");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("LineSegment", pProperty->GetName ().c_str ());
    EXPECT_TRUE (pProperty->GetIsPrimitive ());
    EXPECT_FALSE (pProperty->GetIsStruct ());
    EXPECT_FALSE (pProperty->GetIsArray ());
    EXPECT_STREQ ("Bentley.Geometry.Common.IGeometry", pProperty->GetTypeName ().c_str ());
    EXPECT_TRUE (PRIMITIVETYPE_IGeometry == pProperty->GetAsPrimitiveProperty ()->GetType ());
    EXPECT_TRUE (pProperty->GetIsDisplayLabelDefined ());
    EXPECT_STREQ ("Line Segment", pProperty->GetDisplayLabel ().c_str ());
    EXPECT_STREQ ("", pProperty->GetDescription ().c_str ());
    EXPECT_EQ (pClass, &pProperty->GetClass ());
    EXPECT_FALSE (pProperty->GetIsReadOnly ());

    pProperty = GetPropertyByName (*pClass, "PropertyDoesNotExistInClass", false);
    EXPECT_FALSE (pProperty);

    ECEnumerationP pEnumeration = schema->GetEnumerationP("Enumeration");
    EXPECT_TRUE(pEnumeration != nullptr);
    EXPECT_STREQ("int", pEnumeration->GetTypeName().c_str());
    EXPECT_STREQ("desc", pEnumeration->GetDescription().c_str());
    EXPECT_STREQ("dl", pEnumeration->GetDisplayLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectSuccessWhenRoundtripUsingString)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"Widgets.01.00.ecschema.xml").c_str (), *schemaContext);
    VerifyWidgetsSchema (schema);

    EXPECT_EQ (SchemaReadStatus::Success, status);

    Utf8String ecSchemaXmlString;

    SchemaWriteStatus status2 = schema->WriteToXmlString (ecSchemaXmlString);
    EXPECT_EQ (SchemaWriteStatus::Success, status2);

    ECSchemaPtr deserializedSchema;
    schemaContext = ECSchemaReadContext::CreateContext ();
    status = ECSchema::ReadFromXmlString (deserializedSchema, ecSchemaXmlString.c_str (), *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);

    VerifyWidgetsSchema (deserializedSchema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectSuccessWhenRoundtripEnumerationUsingString)
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

    Utf8String ecSchemaXmlString;
    SchemaWriteStatus status2 = schema->WriteToXmlString (ecSchemaXmlString, 3);
    EXPECT_EQ (SchemaWriteStatus::Success, status2);

    ECSchemaPtr deserializedSchema;
    auto schemaContext = ECSchemaReadContext::CreateContext ();
    auto status3 = ECSchema::ReadFromXmlString (deserializedSchema, ecSchemaXmlString.c_str (), *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status3);

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

    ECEnumerationCP propertyEnumeration = deserializedPrimitive->GetEnumeration();
    ASSERT_TRUE(nullptr != propertyEnumeration);
    EXPECT_STREQ("string", propertyEnumeration->GetTypeName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectFailureWithDuplicateClassesInXml)
    {
    Utf8CP widgets_schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Widgets' version='09.06' displayLabel='Widgets Display Label' description='Widgets Description' nameSpacePrefix='wid' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='DifferentClass' isDomainClass='True'>"
        "    </ECClass>"
        "    <ECClass typeName='ecProject' description='Project ECClass' displayLabel='Project' isDomainClass='True'>"
        "       <ECProperty propertyName='Name' typeName='string' displayLabel='Project Name' />"
        "    </ECClass>"
        "    <ECClass typeName='ecProject' isDomainClass='True'>"
        "    </ECClass>"
        "</ECSchema>";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, widgets_schemaXML, *schemaContext);

    EXPECT_EQ (SchemaReadStatus::DuplicateTypeName, status);

    ECSchemaPtr schema2;
    ECSchemaReadContextPtr   schemaContext2 = ECSchemaReadContext::CreateContext ();

    Utf8CP widgets2_schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Widgets2' version='09.06' displayLabel='Widgets Display Label' description='Widgets Description' nameSpacePrefix='wid' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='ecProject' description='Project ECClass' displayLabel='Project' isDomainClass='True'>"
        "       <ECProperty propertyName='Name' typeName='string' displayLabel='Project Name' />"
        "    </ECClass>"
        "    <ECClass typeName='ecProject' description='New Project ECClass' isDomainClass='True'>"
        "       <ECProperty propertyName='Author' typeName='string' displayLabel='Project Name' />"
        "    </ECClass>"
        "</ECSchema>";
    status = ECSchema::ReadFromXmlString (schema2, widgets2_schemaXML, *schemaContext2);

    EXPECT_EQ (SchemaReadStatus::DuplicateTypeName, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, EnsureSupplementalSchemaCannotHaveBaseClasses)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0' schemaName='SupplementalSchemaWithBaseClasses_Supplemental_Mapping' nameSpacePrefix='ss' version='01.00' description='Test Supplemental Mapping Schema' displayLabel='Electrical Extended Supplemental Mapping' xmlns:ec='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECClass typeName='MAPPING' displayLabel='Mapping' isStruct='false' isDomainClass='true' isCustomAttributeClass='false'/>"
        "<ECClass typeName='ELECTRICAL_PROPERTY_MAPPING' displayLabel='Electrical Property Mapping' isStruct='false' isDomainClass='false' isCustomAttributeClass='true'>"
        "<BaseClass>MAPPING</BaseClass>"
        "<ECProperty propertyName='APPLICATION_PROPERTY_NAME' typeName='string' displayLabel='Application Property Name' readOnly='false'/>"
        "</ECClass>"
        "<ECClass typeName='ELECTRICAL_ITEM' displayLabel='Electrical Item' isStruct='false' isDomainClass='true' isCustomAttributeClass='false'>"
        "<BaseClass>bentley:BENTLEY_BASE_OBJECT</BaseClass>"
        "<ECProperty propertyName='ID' typeName='string' description='Business ID for an electrical item.' readOnly='false'>"
        "<ECCustomAttributes>"
        "<ELECTRICAL_PROPERTY_MAPPING xmlns='ElectricalExtended_Supplemental_Mapping.01.00'>"
        "<APPLICATION_PROPERTY_NAME>DeviceID</APPLICATION_PROPERTY_NAME>"
        "</ELECTRICAL_PROPERTY_MAPPING>"
        "</ECCustomAttributes>"
        "</ECProperty>"
        "</ECClass>"
        "</ECSchema>";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, schemaXML, *schemaContext);

    EXPECT_EQ (SchemaReadStatus::Success, status);
    Utf8CP className = "ELECTRICAL_ITEM";
    ECClassCP ecClass = schema->GetClassCP (className);

    const ECBaseClassesList& baseClassList = ecClass->GetBaseClasses ();
    EXPECT_EQ (0, baseClassList.size ()) << "Class " << className << " should not have any base classes since it is in a supplemental schema.";

    className = "ELECTRICAL_PROPERTY_MAPPING";
    ecClass = schema->GetClassCP (className);

    const ECBaseClassesList& baseClassList2 = ecClass->GetBaseClasses ();
    EXPECT_EQ (0, baseClassList2.size ()) << "Class " << className << " should not have any base classes since it is in a supplemental schema.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectSuccessWhenSerializingToFile)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"Widgets.01.00.ecschema.xml").c_str (), *schemaContext);
    EXPECT_EQ (status, SchemaReadStatus::Success);
    VerifyWidgetsSchema (schema);

    SchemaWriteStatus status2 = schema->WriteToXmlFile (ECTestFixture::GetTempDataPath (L"test.xml").c_str ());
    EXPECT_EQ (SchemaWriteStatus::Success, status2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenReferencingNonExistingUnitAttributesSchema)
    {
    Utf8CP schemaXML = "<ECSchema schemaName=\"ReferencesUnits\" nameSpacePrefix=\"ru\" Description=\"Schema that references the unavailable U_A.1.1 schema\" "
        "version=\"1.1\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECSchemaReference name=\"Unit_Attributes\" version=\"01.01\" prefix=\"ua\" />"
        "</ECSchema>";

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, schemaXML, *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);

    }

#if defined (NEEDSWORK_LIBXML)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenRoundtripUsingStream)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
#ifdef DEBUG_PRINT
    printf("Verifying original schema from file.\n");
#endif
    VerifyWidgetsSchema(schema);

    EXPECT_EQ (SchemaReadStatus::Success, status);    
    LPSTREAM stream = NULL;
    //HRESULT res = ::CreateStreamOnHGlobal(NULL,TRUE,&stream);
    ::CreateStreamOnHGlobal(NULL,TRUE,&stream);

    SchemaWriteStatus status2 = schema->WriteToXmlStream(stream);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    LARGE_INTEGER liPos = {0};
    stream->Seek(liPos, STREAM_SEEK_SET, NULL);

    ECSchemaP deserializedSchema;
    schemaOwner = ECSchemaCache::Create(); // We need a new cache... we don't want to read the ECSchema into the cache that already has a copy of this ECSchema
    schemaContext = ECSchemaReadContext::CreateContext(*schemaOwner);
    status = ECSchema::ReadFromXmlStream(deserializedSchema, stream, *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status); 
#ifdef DEBUG_PRINT
    printf("Verifying schema deserialized from stream.\n");
#endif
    VerifyWidgetsSchema(deserializedSchema);
    }
#endif

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                    Paul.Connelly   11/12
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectErrorWhenBaseClassNotFound)
    {
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='ReferencedSchema' version='01.00' displayLabel='Display Label' description='Description' nameSpacePrefix='ref' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECClass typeName='BaseClass' description='Project ECClass' displayLabel='Project' isDomainClass='True' />"
        "</ECSchema>";
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr refSchema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (refSchema, schemaXML, *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);

    schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='Stuff' version='09.06' displayLabel='Display Label' description='Description' nameSpacePrefix='stuff' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECSchemaReference name='ReferencedSchema' version='01.00' prefix='ref' />"
        "    <ECClass typeName='ecProject' description='Project ECClass' displayLabel='Project' isDomainClass='True'>"
        "       <BaseClass>BaseClassDOESNOTEXIST</BaseClass>"
        "       <BaseClass>ref:BaseClass</BaseClass>"
        "    </ECClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    status = ECSchema::ReadFromXmlString (schema, schemaXML, *schemaContext);

    EXPECT_NE (SchemaReadStatus::Success, status);
    EXPECT_TRUE (schema.IsNull ());
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                    Paul.Connelly   11/12
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaDeserializationTest, ExpectSuccessWithEnumerationInReferencedSchema)
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
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr refSchema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString (refSchema, schemaXML, *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);

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
    status = ECSchema::ReadFromXmlString (schema, schemaXML, *schemaContext);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   06/15
//---------------+---------------+---------------+---------------+---------------+------
TEST_F (SchemaDeserializationTest, TestMultipleConstraintClasses)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0' schemaName='ReferencedSchema' nameSpacePrefix='ref' version='01.00' description='Description' displayLabel='Display Label' xmlns:ec='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "  <ECClass typeName = 'Class' isDomainClass = 'True'>"
        "      <ECProperty propertyName = 'Property' typeName = 'string' />"
        "  </ECClass>"
        "  <ECClass typeName = 'Class1' isDomainClass = 'True'>"
        "      <ECProperty propertyName = 'Property1' typeName = 'string' />"
        "      <ECProperty propertyName = 'Property2' typeName = 'string' />"
        "  </ECClass>"
        "  <ECClass typeName = 'Class2' isDomainClass = 'True'>"
        "      <ECProperty propertyName = 'Property3' typeName = 'string' />"
        "      <ECProperty propertyName = 'Property4' typeName = 'string' />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName = 'ClassHasClass1Or2' isDomainClass = 'True' strength = 'referencing' strengthDirection = 'forward'>"
        "      <Source cardinality = '(0, 1)' polymorphic = 'True'>"
        "          <Class class = 'Class' />"
        "      </Source>"
        "      <Target cardinality = '(0, 1)' polymorphic = 'True'>"
        "          <Class class = 'Class1' />"
        "          <Class class = 'Class2' />"
        "      </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext ();
    ECSchemaPtr ecSchema = nullptr;
    ASSERT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString (ecSchema, schemaXml, *schemaContext));
    ECRelationshipClassCP relClass = ecSchema->GetClassCP ("ClassHasClass1Or2")->GetRelationshipClassCP ();
    ASSERT_TRUE (relClass != nullptr);
    ASSERT_EQ (1, relClass->GetSource ().GetConstraintClasses ().size ());
    ASSERT_STREQ ("Class", relClass->GetSource ().GetClasses ()[0]->GetName ().c_str ());

    ASSERT_EQ (2, relClass->GetTarget ().GetConstraintClasses ().size ());
    ASSERT_STREQ ("Class1", relClass->GetTarget ().GetClasses ()[0]->GetName ().c_str ());
    ASSERT_STREQ ("Class2", relClass->GetTarget ().GetClasses ()[1]->GetName ().c_str ());
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                    Paul.Connelly   11/12
//---------------+---------------+---------------+---------------+---------------+------
TEST_F (SchemaDeserializationTest, TestRelationshipKeys)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr ecSchema;
    Utf8String schemaString ("<?xml version='1.0' encoding='UTF-8'?>"
                             "<ECSchema xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0' schemaName='ReferencedSchema' nameSpacePrefix='ref' version='01.00' description='Description' displayLabel='Display Label' xmlns:ec='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                             "  <ECClass typeName = 'Class' isDomainClass = 'True'>"
                             "      <ECProperty propertyName = 'Property' typeName = 'string' />"
                             "  </ECClass>"
                             "  <ECClass typeName = 'Class1' isDomainClass = 'True'>"
                             "      <ECProperty propertyName = 'Property1' typeName = 'string' />"
                             "      <ECProperty propertyName = 'Property2' typeName = 'string' />"
                             "  </ECClass>"
                             "  <ECRelationshipClass typeName = 'RelationshipClass' isDomainClass = 'True' strength = 'referencing' strengthDirection = 'forward'>"
                             "      <Source cardinality = '(0, 1)' polymorphic = 'True'>"
                             "          <Class class = 'Class'>"
                             "              <Key>"
                             "                  <Property name = 'Property' />"
                             "              </Key>"
                             "          </Class>"
                             "      </Source>"
                             "      <Target cardinality = '(0, 1)' polymorphic = 'True'>"
                             "          <Class class = 'Class1'>"
                             "              <Key>"
                             "                  <Property name = 'Property1' />"
                             "                  <Property name = 'Property2' />"
                             "              </Key>"
                             "          </Class>"
                             "      </Target>"
                             "  </ECRelationshipClass>"
                             "</ECSchema>");

    ASSERT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString (ecSchema, schemaString.c_str (), *schemaContext));
    ECRelationshipClassP relClass = ecSchema->GetClassP ("RelationshipClass")->GetRelationshipClassP ();
    for (auto constrainClass : relClass->GetSource ().GetConstraintClasses ())
        {
        Utf8String key = constrainClass->GetKeys ().at (0);
        ASSERT_TRUE (key.Equals ("Property"));
        }

    for (auto constrainClass : relClass->GetTarget ().GetConstraintClasses ())
        {
        auto keys = constrainClass->GetKeys ();
        ASSERT_EQ (2, keys.size ());
        ASSERT_TRUE (((Utf8String)keys[0]).Equals ("Property1"));
        ASSERT_TRUE (((Utf8String)keys[1]).Equals ("Property2"));
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   06/15
//---------------+---------------+---------------+---------------+---------------+------
TEST_F (SchemaDeserializationTest, TestMultipleConstraintClassesWithKeyProperties)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0' schemaName='ReferencedSchema' nameSpacePrefix='ref' version='01.00' description='Description' displayLabel='Display Label' xmlns:ec='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "  <ECClass typeName = 'Class' isDomainClass = 'True'>"
        "      <ECProperty propertyName = 'Property' typeName = 'string' />"
        "  </ECClass>"
        "  <ECClass typeName = 'Class1' isDomainClass = 'True'>"
        "      <ECProperty propertyName = 'Property1' typeName = 'string' />"
        "      <ECProperty propertyName = 'Property2' typeName = 'string' />"
        "  </ECClass>"
        "  <ECClass typeName = 'Class2' isDomainClass = 'True'>"
        "      <ECProperty propertyName = 'Property3' typeName = 'string' />"
        "      <ECProperty propertyName = 'Property4' typeName = 'string' />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName = 'ClassHasClass1Or2' isDomainClass = 'True' strength = 'referencing' strengthDirection = 'forward'>"
        "      <Source cardinality = '(0, 1)' polymorphic = 'True'>"
        "          <Class class = 'Class' />"
        "      </Source>"
        "      <Target cardinality = '(0, 1)' polymorphic = 'True'>"
        "          <Class class = 'Class1'>"
        "              <Key>"
        "                  <Property name = 'Property1' />"
        "                  <Property name = 'Property2' />"
        "              </Key>"
        "          </Class>"
        "          <Class class = 'Class2' />"
        "      </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext ();
    ECSchemaPtr ecSchema = nullptr;
    ASSERT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString (ecSchema, schemaXml, *schemaContext));
    ECRelationshipClassCP relClass = ecSchema->GetClassCP ("ClassHasClass1Or2")->GetRelationshipClassCP ();
    ASSERT_TRUE (relClass != nullptr);
    ASSERT_EQ (1, relClass->GetSource ().GetConstraintClasses ().size ());
    ASSERT_STREQ ("Class", relClass->GetSource ().GetClasses ()[0]->GetName ().c_str ());

    ASSERT_EQ (2, relClass->GetTarget ().GetConstraintClasses ().size ());
    ECRelationshipConstraintClassCP constraintClass1 = relClass->GetTarget ().GetConstraintClasses ()[0];
    ASSERT_STREQ ("Class1", constraintClass1->GetClass ().GetName ().c_str ());

    ASSERT_EQ (2, constraintClass1->GetKeys ().size ());
    ASSERT_STREQ ("Property1", constraintClass1->GetKeys ()[0].c_str ());
    ASSERT_STREQ ("Property2", constraintClass1->GetKeys ()[1].c_str ());

    ECRelationshipConstraintClassCP constraintClass2 = relClass->GetTarget ().GetConstraintClasses ()[1];
    ASSERT_STREQ ("Class2", constraintClass2->GetClass ().GetName ().c_str ());
    ASSERT_EQ (0, constraintClass2->GetKeys ().size ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Basanta.Kharel   12/2015
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaDeserializationTest, KindOfQuantityTest)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "KindOfQuantitySchema", "koq", 5, 0, 6);
    ASSERT_TRUE(schema.IsValid());

    ECEntityClassP entityClass;
    schema->CreateEntityClass(entityClass, "Class");
    ASSERT_NE(entityClass, nullptr);

    PrimitiveECPropertyP prop;
    entityClass->CreatePrimitiveProperty(prop, "Property");
    ASSERT_NE(prop, nullptr);

    EXPECT_EQ(ECObjectsStatus::Success, prop->SetKindOfQuantity("koq"));

    Utf8String schemaXML;
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(schemaXML, 3));

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr refSchema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(refSchema, schemaXML.c_str(), *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);

    ECClassCP entityClassDup = refSchema->GetClassCP("Class");
    ASSERT_NE(entityClassDup, nullptr);
    PrimitiveECPropertyCP property = entityClassDup->GetPropertyP("Property")->GetAsPrimitiveProperty();
    ASSERT_NE(property, nullptr);

    EXPECT_EQ("koq", property->GetKindOfQuantity());
    }


void ValidateElementOrder(bvector<Utf8String> expectedTypeNames, BeXmlNodeP root)
    {
    BeXmlNodeP currentNode = root->GetFirstChild();
    for (auto expectedTypeName : expectedTypeNames)
        {
        if (currentNode == nullptr)
            {
            FAIL() << "Expected end of document, Node '" << expectedTypeName << "' expected.";
            }

        Utf8String nodeTypeName;
        EXPECT_EQ(BeXmlStatus::BEXML_Success, currentNode->GetAttributeStringValue(nodeTypeName, "typeName"));
        EXPECT_EQ(expectedTypeName, nodeTypeName);

        currentNode = currentNode->GetNextSibling();
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Stefan.Apfel    02/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, TestPreservingElementOrder)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->SetPreserveElementOrder(true);

    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TestSchema' version='01.00' nameSpacePrefix='ab' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName=\"GHI\" description=\"Project ECClass\" displayLabel=\"Class GHI\"></ECEntityClass>"
        "    <ECEntityClass typeName=\"ABC\" description=\"Project ECClass\" displayLabel=\"Class ABC\"></ECEntityClass>"
        "    <ECEnumeration typeName=\"DEF\" displayLabel=\"Enumeration DEF\" backingTypeName=\"int\" />"
        "</ECSchema>";
    ECSchemaPtr schema;
    SchemaReadStatus schemaReadingStatus = ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, schemaReadingStatus);

    Utf8String ecSchemaXmlString;
    SchemaWriteStatus schemaWritingStatus = schema->WriteToXmlString(ecSchemaXmlString,3,0);
    EXPECT_EQ(SchemaWriteStatus::Success, schemaWritingStatus);

    size_t stringByteCount = ecSchemaXmlString.length() * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, ecSchemaXmlString.c_str(), stringByteCount);
    EXPECT_EQ(BEXML_Success, xmlStatus);

    bvector<Utf8String> typeNames = {"GHI", "ABC", "DEF"};
    ValidateElementOrder(typeNames, xmlDom.get()->GetRootElement());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Stefan.Apfel    02/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, TestDefaultElementOrder)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TestSchema' version='01.00' nameSpacePrefix='ab' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName=\"GHI\" description=\"Project ECClass\" displayLabel=\"Class GHI\"></ECEntityClass>"
        "    <ECEntityClass typeName=\"ABC\" description=\"Project ECClass\" displayLabel=\"Class ABC\"></ECEntityClass>"
        "    <ECEnumeration typeName=\"DEF\" displayLabel=\"Enumeration DEF\" backingTypeName=\"int\" />"
        "</ECSchema>";
    ECSchemaPtr schema;
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext));

    WString ecSchemaXmlString;
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(ecSchemaXmlString,3,0));

    size_t stringByteCount = ecSchemaXmlString.length() * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, ecSchemaXmlString.c_str(), stringByteCount);
    EXPECT_EQ(BEXML_Success, xmlStatus);

    // Enumerations (DEF) are serialized first, then classes (ABC, GHI)
    bvector<Utf8String> typeNames = {"DEF", "ABC", "GHI"};
    ValidateElementOrder(typeNames, xmlDom.get()->GetRootElement());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Stefan.Apfel    02/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, TestPreserveElementOrderWithBaseClassAndRelationships)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->SetPreserveElementOrder(true);

    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TestSchema' version='01.00' nameSpacePrefix='ab' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName=\"GHI\" description=\"Project ECClass\" displayLabel=\"Class GHI\"></ECEntityClass>"
        "    <ECEntityClass typeName=\"ABC\" description=\"Project ECClass\" displayLabel=\"Class ABC\">"
        "      <BaseClass>MNO</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName=\"PQR\" isDomainClass=\"True\" strength=\"referencing\" strengthDirection=\"forward\">"
        "      <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "          <Class class = \"MNO\" />"
        "      </Source>"
        "      <Target cardinality = '(0, 1)' polymorphic = 'True'>"
        "          <Class class = 'JKL'>"
        "              <Key>"
        "                  <Property name = 'Property1' />"
        "                  <Property name = 'Property2' />"
        "              </Key>"
        "          </Class>"
        "      </Target>"
        "    </ECRelationshipClass>"
        "    <ECEntityClass typeName = \"MNO\" description=\"Project ECClass\" displayLabel=\"Class MNO\"></ECEntityClass>"
        "    <ECEntityClass typeName = \"JKL\" description=\"Project ECClass\" displayLabel=\"Class JKL\">"
        "      <ECProperty propertyName=\"Property1\" typeName=\"string\" />"
        "      <ECProperty propertyName=\"Property2\" typeName=\"string\" />"
        "    </ECEntityClass>"
        "    <ECEnumeration typeName=\"DEF\" displayLabel=\"Enumeration PQR\" backingTypeName=\"int\" />"
        "</ECSchema>";
    ECSchemaPtr schema;
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext));

    WString ecSchemaXmlString;
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(ecSchemaXmlString, 3, 0));

    size_t stringByteCount = ecSchemaXmlString.length() * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, ecSchemaXmlString.c_str(), stringByteCount);
    EXPECT_EQ(BEXML_Success, xmlStatus);

    // Expecting the same order as specified in the SchemaXML Document.
    bvector<Utf8String> typeNames = {"GHI","ABC","PQR", "MNO", "JKL", "DEF", };
    ValidateElementOrder(typeNames, xmlDom.get()->GetRootElement());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Stefan.Apfel    02/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationTest, TestDefaultElementOrderWithBaseClassAndRelationships)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    Utf8CP schemaXML = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TestSchema' version='01.00' nameSpacePrefix='ab' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName=\"GHI\" description=\"Project ECClass\" displayLabel=\"Class GHI\"></ECEntityClass>"
        "    <ECEntityClass typeName=\"ABC\" description=\"Project ECClass\" displayLabel=\"Class ABC\">"
        "      <BaseClass>MNO</BaseClass>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName=\"DEF\" isDomainClass=\"True\" strength=\"referencing\" strengthDirection=\"forward\">"
        "      <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "          <Class class = \"MNO\" />"
        "      </Source>"
        "      <Target cardinality = '(0, 1)' polymorphic = 'True'>"
        "          <Class class = 'JKL'>"
        "              <Key>"
        "                  <Property name = 'Property1' />"
        "                  <Property name = 'Property2' />"
        "              </Key>"
        "          </Class>"
        "      </Target>"
        "    </ECRelationshipClass>"
        "    <ECEntityClass typeName = \"MNO\" description=\"Project ECClass\" displayLabel=\"Class MNO\"></ECEntityClass>"
        "    <ECEntityClass typeName = \"JKL\" description=\"Project ECClass\" displayLabel=\"Class JKL\">"
        "      <ECProperty propertyName=\"Property1\" typeName=\"string\" />"
        "      <ECProperty propertyName=\"Property2\" typeName=\"string\" />"
        "    </ECEntityClass>"
        "    <ECEnumeration typeName=\"PQR\" displayLabel=\"Enumeration PQR\" backingTypeName=\"int\" />"
        "</ECSchema>";
    ECSchemaPtr schema;
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXML, *schemaContext));

    WString ecSchemaXmlString;
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(ecSchemaXmlString, 3, 0));

    size_t stringByteCount = ecSchemaXmlString.length() * sizeof(Utf8Char);
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString(xmlStatus, ecSchemaXmlString.c_str(), stringByteCount);
    EXPECT_EQ(BEXML_Success, xmlStatus);

    // First Enumeration (PQR), then classes alphabetically (ABC, DEF, GHI). As MNO is the base class of ABC and
    // JKL has a constraint in DEF, those two classes are written before the class they depend in.
    bvector<Utf8String> typeNames = {"PQR", "MNO", "ABC", "JKL", "DEF", "GHI"};
    ValidateElementOrder(typeNames, xmlDom.get()->GetRootElement());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaSerializationTest, ExpectSuccessWithSerializingBaseClasses)
    {
    ECSchemaPtr schema;
    ECSchemaPtr schema2;
    ECSchemaPtr schema3;

    ECSchema::CreateSchema (schema, "Widget", 5, 5);
    ECSchema::CreateSchema (schema2, "BaseSchema", 5, 5);
    ECSchema::CreateSchema (schema3, "BaseSchema2", 5, 5);

    schema->SetNamespacePrefix ("ecw");
    schema2->SetNamespacePrefix ("base");
    schema3->SetNamespacePrefix ("base");

    ECEntityClassP class1;
    ECEntityClassP baseClass;
    ECEntityClassP anotherBase;
    ECEntityClassP gadget;
    ECEntityClassP bolt;
    ECEnumerationP enumeration;
    schema->CreateEntityClass (class1, "TestClass");
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

    EXPECT_EQ (ECObjectsStatus::SchemaNotFound, class1->AddBaseClass (*baseClass));
    schema->AddReferencedSchema (*schema2);
    schema->AddReferencedSchema (*schema3);
    EXPECT_EQ (ECObjectsStatus::Success, class1->AddBaseClass (*baseClass));
    EXPECT_EQ (ECObjectsStatus::Success, class1->AddBaseClass (*anotherBase));
    EXPECT_EQ (ECObjectsStatus::Success, gadget->AddBaseClass (*class1));

    SchemaWriteStatus status2 = schema->WriteToXmlFile (ECTestFixture::GetTempDataPath (L"base.xml").c_str ());
    EXPECT_EQ (SchemaWriteStatus::Success, status2);

    status2 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath(L"base_ec3.xml").c_str(), 3);
    EXPECT_EQ(SchemaWriteStatus::Success, status2);

    WString ecSchemaXmlString;
    SchemaWriteStatus status3 = schema->WriteToXmlString (ecSchemaXmlString);
    EXPECT_EQ (SchemaWriteStatus::Success, status3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaReferenceTest, AddAndRemoveReferencedSchemas)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);

    ECSchemaPtr refSchema;
    ECSchema::CreateSchema (refSchema, "RefSchema", 5, 5);

    EXPECT_EQ (ECObjectsStatus::Success, schema->AddReferencedSchema (*refSchema));
    EXPECT_EQ (ECObjectsStatus::NamedItemAlreadyExists, schema->AddReferencedSchema (*refSchema));

    ECSchemaReferenceListCR refList = schema->GetReferencedSchemas ();

    ECSchemaReferenceList::const_iterator schemaIterator = refList.find (refSchema->GetSchemaKey ());

    EXPECT_FALSE (schemaIterator == refList.end ());
    EXPECT_EQ (ECObjectsStatus::Success, schema->RemoveReferencedSchema (*refSchema));

    schemaIterator = refList.find (refSchema->GetSchemaKey ());

    EXPECT_TRUE (schemaIterator == refList.end ());
    EXPECT_EQ (ECObjectsStatus::SchemaNotFound, schema->RemoveReferencedSchema (*refSchema));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaSerializationTest, SerializeComprehensiveSchema)
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
    ECSchema::CreateSchema (schema, "ComprehensiveSchema", "cmpr", 1, 5, 2);
    schema->SetDescription("Comprehensive Schema to demonstrate use of all ECSchema concepts.");
    schema->SetDisplayLabel("Comprehensive Schema");
    schema->AddReferencedSchema(*standardCASchema);

    ECEntityClassP baseEntityClass;
    ECEntityClassP entityClass;
    ECStructClassP structClass;
    ECCustomAttributeClassP classCustomAttributeClass;
    ECCustomAttributeClassP generalCustomAttributeClass;
    ECEnumerationP enumeration;
    
    schema->CreateEntityClass (baseEntityClass, "BaseEntity");
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

    WString fullSchemaName;
    fullSchemaName.AssignUtf8(schema->GetFullSchemaName().c_str());
    fullSchemaName.append(L".ecschema.xml");

    WString legacyFullSchemaName;
    legacyFullSchemaName.AssignUtf8(schema->GetLegacyFullSchemaName().c_str());
    legacyFullSchemaName.append(L".ecschema.xml");
    
    SchemaWriteStatus status2 = schema->WriteToXmlFile (ECTestFixture::GetTempDataPath (fullSchemaName.c_str()).c_str (), 3);
    EXPECT_EQ (SchemaWriteStatus::Success, status2);

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
TEST_F (SchemaReferenceTest, InvalidReference)
    {
    // show error messages but do not assert.
    ECSchema::SetErrorHandling (true, false);

    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();
    WString seedPath (ECTestFixture::GetTestDataPath (L"").c_str ());
    schemaContext->AddSchemaPath (seedPath.c_str ());

    //The "InvalidReference" schema contains a reference to BaseSchema.01.01.  This schema 
    //does not exist.  1.0 exists, but the minor version numbers are incompatible.
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"InvalidReference.01.00.ecschema.xml").c_str (), *schemaContext);

    EXPECT_TRUE (schema.IsNull ());
    EXPECT_EQ (SchemaReadStatus::ReferencedSchemaNotFound, status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaReferenceTest, ExpectErrorWhenTryRemoveSchemaInUse)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);

    ECSchemaPtr refSchema;
    ECSchema::CreateSchema (refSchema, "RefSchema", 5, 5);

    EXPECT_EQ (ECObjectsStatus::Success, schema->AddReferencedSchema (*refSchema));
    ECEntityClassP class1;
    ECEntityClassP baseClass;
    ECStructClassP structClass;

    refSchema->CreateEntityClass(baseClass, "BaseClass");
    refSchema->CreateStructClass(structClass, "StructClass");
    schema->CreateEntityClass(class1, "TestClass");

    class1->AddBaseClass (*baseClass);
    EXPECT_EQ (ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema (*refSchema));

    class1->RemoveBaseClass (*baseClass);
    EXPECT_EQ (ECObjectsStatus::Success, schema->RemoveReferencedSchema (*refSchema));

    EXPECT_EQ (ECObjectsStatus::Success, schema->AddReferencedSchema (*refSchema));
    StructECPropertyP structProp;
    StructArrayECPropertyP nestedArrayProp;

    ArrayECPropertyP primitiveArrayProp;

    class1->CreateStructProperty (structProp, "StructMember");
    class1->CreateStructArrayProperty (nestedArrayProp, "NestedArray", structClass);

    class1->CreateArrayProperty (primitiveArrayProp, "PrimitiveArrayProp");
    primitiveArrayProp->SetPrimitiveElementType (PRIMITIVETYPE_Long);
    primitiveArrayProp->SetMinOccurs (1);
    primitiveArrayProp->SetMaxOccurs (10);

    structProp->SetType (*structClass);

    EXPECT_EQ (ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema (*refSchema));
    class1->RemoveProperty ("StructMember");
    EXPECT_EQ (ECObjectsStatus::SchemaInUse, schema->RemoveReferencedSchema (*refSchema));
    class1->RemoveProperty ("NestedArray");
    EXPECT_EQ (ECObjectsStatus::Success, schema->RemoveReferencedSchema (*refSchema));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaReferenceTest, ExpectFailureWithCircularReferences)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();
    WString seedPath (ECTestFixture::GetTestDataPath (L"").c_str ());
    schemaContext->AddSchemaPath (seedPath.c_str ());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"CircleSchema.01.00.ecschema.xml").c_str (), *schemaContext);
    EXPECT_FALSE (SchemaReadStatus::Success == status);
    EXPECT_FALSE (schema.IsValid ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaReferenceTest, ExpectSuccessWithSpecialCaseOpenPlantSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();
    WString seedPath (ECTestFixture::GetTestDataPath (L"").c_str ());
    schemaContext->AddSchemaPath (seedPath.c_str ());

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"OpenPlant_Supplemental_Mapping_OPPID.01.01.ecschema.xml").c_str (), *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);

    ECSchemaReferenceListCR refList = schema->GetReferencedSchemas ();
    EXPECT_EQ (1, refList.size ());
    ECSchemaPtr refSchema = refList.begin ()->second;
    EXPECT_EQ (0, refSchema->GetName ().CompareTo ("Bentley_Standard_CustomAttributes"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaReferenceTest, FindClassInReferenceList)
    {
    ECSchemaPtr schema, refSchema;
    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    ECSchema::CreateSchema (refSchema, "RefSchema", 5, 5);

    ECRelationshipClassP relClass;
    ECEntityClassP targetClass, sourceClass;
    schema->CreateRelationshipClass (relClass, "RElationshipClass");
    schema->CreateEntityClass (targetClass, "Target");
    refSchema->CreateEntityClass (sourceClass, "Source");

    EXPECT_EQ (ECObjectsStatus::Success, schema->AddReferencedSchema (*refSchema));
    EXPECT_EQ (ECObjectsStatus::Success, relClass->GetTarget ().AddClass (*targetClass));
    EXPECT_EQ (ECObjectsStatus::Success, relClass->GetSource ().AddClass (*sourceClass));

    ECSchemaReferenceListCR refList = schema->GetReferencedSchemas ();

    EXPECT_TRUE (refList.FindClassP (SchemaNameClassNamePair ("RefSchema", "Source")) != NULL);
    }

//TEST_F(SchemaReferenceTest, ExpectSchemaGraphInCorrectOrder)
//    {
//    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
//    WString seedPath(ECTestFixture::GetTestDataPath(L"").c_str());
//    schemaContext->AddSchemaPath(seedPath.c_str());
//
//    ECSchemaPtr schema;
//    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath( L"Bentley_Plant.06.00.ecschema.xml").c_str(), *schemaContext);
//    ASSERT_EQ (SchemaReadStatus::Success, status);
//    ASSERT_TRUE( schema.IsValid() );
//    bvector<ECSchemaP> schemasToImport;
//    schema->FindAllSchemasInGraph (schemasToImport, true);
//    bvector<WCharCP> expectedPrefixes;
//    expectedPrefixes.push_back(L"iipmdbca");
//    expectedPrefixes.push_back(L"rdlca");
//    expectedPrefixes.push_back(L"beca");
//    expectedPrefixes.push_back(L"bsca");
//    expectedPrefixes.push_back(L"bsm");
//    expectedPrefixes.push_back(L"bjca");
//    expectedPrefixes.push_back(L"jclass");
//    expectedPrefixes.push_back(L"dmd");
//    expectedPrefixes.push_back(L"stepmd");
//    expectedPrefixes.push_back(L"bjeca");
//    expectedPrefixes.push_back(L"bpca");
//    expectedPrefixes.push_back(L"bp");
//
//    EXPECT_EQ(schemasToImport.size(), expectedPrefixes.size());
//    int counter = 0;
//    for (ECSchemaP testSchema: schemasToImport)
//        {
//        WString prefix = testSchema->GetNamespacePrefix();
//        EXPECT_EQ(0, wcscmp(prefix.c_str(), expectedPrefixes[counter]));
//        counter++;
//        }
//    }

TEST_F (SchemaLocateTest, ExpectSuccessWhenLocatingStandardSchema)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    bmap<Utf8String, Utf8CP> standardSchemaNames;
    standardSchemaNames.insert (bpair<Utf8String, Utf8CP> ("Bentley_Standard_CustomAttributes", "01.04"));
    standardSchemaNames.insert (bpair<Utf8String, Utf8CP> ("Bentley_Standard_Classes", "01.00"));
    standardSchemaNames.insert (bpair<Utf8String, Utf8CP> ("Bentley_ECSchemaMap", "01.00"));
    standardSchemaNames.insert (bpair<Utf8String, Utf8CP> ("EditorCustomAttributes", "01.00"));
    standardSchemaNames.insert (bpair<Utf8String, Utf8CP> ("Bentley_Common_Classes", "01.00"));
    standardSchemaNames.insert (bpair<Utf8String, Utf8CP> ("Dimension_Schema", "01.00"));
    standardSchemaNames.insert (bpair<Utf8String, Utf8CP> ("iip_mdb_customAttributes", "01.00"));
    standardSchemaNames.insert (bpair<Utf8String, Utf8CP> ("KindOfQuantity_Schema", "01.01"));
    standardSchemaNames.insert (bpair<Utf8String, Utf8CP> ("rdl_customAttributes", "01.00"));
    standardSchemaNames.insert (bpair<Utf8String, Utf8CP> ("SIUnitSystemDefaults", "01.00"));
    standardSchemaNames.insert (bpair<Utf8String, Utf8CP> ("Unit_Attributes", "01.00"));
    standardSchemaNames.insert (bpair<Utf8String, Utf8CP> ("Units_Schema", "01.00"));
    standardSchemaNames.insert (bpair<Utf8String, Utf8CP> ("USCustomaryUnitSystemDefaults", "01.00"));

    ECSchemaPtr schema;

    for (bmap<Utf8String, Utf8CP>::const_iterator it = standardSchemaNames.begin (); it != standardSchemaNames.end (); ++it)
        {
        bpair<Utf8String, Utf8CP>const& entry = *it;

        SchemaKey key (entry.first.c_str (), 1, 0);
        EXPECT_TRUE (ECSchema::ParseVersionString (key.m_versionMajor, key.m_versionMinor, entry.second) == ECObjectsStatus::Success);
        EXPECT_EQ (key.m_versionMajor, atoi (entry.second));
        EXPECT_EQ (key.m_versionMinor, atoi (strchr (entry.second, '.') + 1));
        schema = ECSchema::LocateSchema (key, *schemaContext);
        EXPECT_TRUE (schema.IsValid ());
        EXPECT_TRUE (schema->IsStandardSchema ());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaLocateTest, ExpectFailureWithNonStandardSchema)
    {
    ECSchemaPtr testSchema;
    ECSchema::CreateSchema (testSchema, "TestSchema", 1, 2);
    EXPECT_FALSE (testSchema->IsStandardSchema ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaLocateTest, DetermineWhetherSchemaCanBeImported)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    SchemaKey key ("Bentley_Standard_CustomAttributes", 1, 4);

    ECSchemaPtr schema = ECSchema::LocateSchema (key, *schemaContext);
    EXPECT_TRUE (schema.IsValid ());
    EXPECT_FALSE (schema->ShouldNotBeStored ());

    ECSchema::CreateSchema (schema, "Units_Schema", 1, 4);
    EXPECT_TRUE (schema->ShouldNotBeStored ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaCreationTest, CanFullyCreateASchema)
    {
    ECSchemaPtr testSchema;
    ECSchema::CreateSchema (testSchema, "TestSchema", 1, 2);
    testSchema->SetNamespacePrefix ("ts");
    testSchema->SetDescription ("Schema for testing programmatic construction");
    testSchema->SetDisplayLabel ("Test Schema");

    EXPECT_TRUE (testSchema->GetIsDisplayLabelDefined ());
    EXPECT_EQ (1, testSchema->GetVersionMajor ());
    EXPECT_EQ (2, testSchema->GetVersionMinor ());
    EXPECT_EQ (0, strcmp (testSchema->GetName ().c_str (), "TestSchema"));
    EXPECT_EQ (0, strcmp (testSchema->GetNamespacePrefix ().c_str (), "ts"));
    EXPECT_EQ (0, strcmp (testSchema->GetDescription ().c_str (), "Schema for testing programmatic construction"));
    EXPECT_EQ (0, strcmp (testSchema->GetDisplayLabel ().c_str (), "Test Schema"));

    ECSchemaPtr schema2;
    ECSchema::CreateSchema (schema2, "BaseSchema", 5, 5);

    testSchema->AddReferencedSchema (*schema2);

    ECEntityClassP class1;
    ECEntityClassP class2;
    ECEntityClassP baseClass;
    ECStructClassP structClass;
    ECEntityClassP relatedClass;
    ECRelationshipClassP relationshipClass;

    testSchema->CreateEntityClass(class1, "TestClass");
    testSchema->CreateEntityClass(class2, "TestClass2");
    testSchema->CreateStructClass (structClass, "StructClass");
    schema2->CreateEntityClass (baseClass, "BaseClass");
    testSchema->CreateEntityClass (relatedClass, "RelatedClass");

    class1->SetDescription ("Class for testing purposes");
    class1->SetDisplayLabel ("Test Class");

    EXPECT_EQ (0, strcmp (class1->GetDescription ().c_str (), "Class for testing purposes"));
    EXPECT_EQ (0, strcmp (class1->GetDisplayLabel ().c_str (), "Test Class"));
    EXPECT_TRUE (class1->IsEntityClass());
    EXPECT_EQ (ECObjectsStatus::Success, class1->AddBaseClass (*baseClass));
    EXPECT_TRUE (class1->HasBaseClasses ());

    EXPECT_TRUE (structClass->IsStructClass());

    PrimitiveECPropertyP stringProp;
    StructECPropertyP structProp;
    StructArrayECPropertyP nestedArrayProp;
    ArrayECPropertyP primitiveArrayProp;

    class1->CreatePrimitiveProperty (stringProp, "StringMember");
    class1->CreateStructProperty (structProp, "StructMember");
    class1->CreateStructArrayProperty (nestedArrayProp, "NestedArray", structClass);
    class1->CreateArrayProperty (primitiveArrayProp, "PrimitiveArray");

    structProp->SetType (*structClass);
    primitiveArrayProp->SetPrimitiveElementType (PRIMITIVETYPE_Long);
    primitiveArrayProp->SetMinOccurs (1);
    primitiveArrayProp->SetMaxOccurs (10);

    EXPECT_TRUE (ARRAYKIND_Struct == nestedArrayProp->GetKind ());
    EXPECT_TRUE (ARRAYKIND_Primitive == primitiveArrayProp->GetKind ());
    EXPECT_EQ (0, nestedArrayProp->GetMinOccurs ());
    EXPECT_EQ (UINT_MAX, nestedArrayProp->GetMaxOccurs ());
    EXPECT_EQ (1, primitiveArrayProp->GetMinOccurs ());
#if FIXED_COUNT_ARRAYS_ARE_SUPPORTED
    EXPECT_EQ(10, primitiveArrayProp->GetMaxOccurs());
#else
    EXPECT_EQ (UINT_MAX, primitiveArrayProp->GetMaxOccurs ());
#endif
    EXPECT_TRUE (stringProp->GetIsPrimitive ());
    EXPECT_FALSE (stringProp->GetIsStruct ());
    EXPECT_FALSE (stringProp->GetIsArray ());

    EXPECT_FALSE (structProp->GetIsPrimitive ());
    EXPECT_TRUE (structProp->GetIsStruct ());
    EXPECT_FALSE (structProp->GetIsArray ());

    EXPECT_FALSE (primitiveArrayProp->GetIsPrimitive ());
    EXPECT_FALSE (primitiveArrayProp->GetIsStruct ());
    EXPECT_TRUE (primitiveArrayProp->GetIsArray ());

    EXPECT_FALSE (stringProp->GetIsReadOnly ());

    EXPECT_EQ (0, strcmp (stringProp->GetTypeName ().c_str (), "string"));
    EXPECT_TRUE (PRIMITIVETYPE_String == stringProp->GetType ());
    EXPECT_EQ (0, strcmp (structProp->GetType ().GetName ().c_str (), "StructClass"));

    PrimitiveECPropertyP binaryProperty;
    PrimitiveECPropertyP booleanProperty;
    PrimitiveECPropertyP dateTimeProperty;
    PrimitiveECPropertyP doubleProperty;
    PrimitiveECPropertyP integerProperty;
    PrimitiveECPropertyP longProperty;
    PrimitiveECPropertyP point2DProperty;
    PrimitiveECPropertyP point3DProperty;

    class1->CreatePrimitiveProperty (binaryProperty, "BinaryProp");
    class1->CreatePrimitiveProperty (booleanProperty, "BooleanProp");
    class1->CreatePrimitiveProperty (dateTimeProperty, "DateTimeProp");
    class1->CreatePrimitiveProperty (doubleProperty, "DoubleProp");
    class1->CreatePrimitiveProperty (integerProperty, "IntProp");
    class1->CreatePrimitiveProperty (longProperty, "LongProp");
    class1->CreatePrimitiveProperty (point2DProperty, "Point2DProp");
    class1->CreatePrimitiveProperty (point3DProperty, "Point3DProp");

    EXPECT_EQ (ECObjectsStatus::ParseError, binaryProperty->SetTypeName ("fake"));

    binaryProperty->SetTypeName ("binary");
    booleanProperty->SetTypeName ("boolean");
    dateTimeProperty->SetTypeName ("dateTime");
    doubleProperty->SetTypeName ("double");
    integerProperty->SetTypeName ("int");
    longProperty->SetTypeName ("long");
    point2DProperty->SetTypeName ("point2d");
    point3DProperty->SetTypeName ("point3d");

    EXPECT_TRUE (PRIMITIVETYPE_Binary == binaryProperty->GetType ());
    EXPECT_TRUE (PRIMITIVETYPE_Boolean == booleanProperty->GetType ());
    EXPECT_TRUE (PRIMITIVETYPE_DateTime == dateTimeProperty->GetType ());
    EXPECT_TRUE (PRIMITIVETYPE_Double == doubleProperty->GetType ());
    EXPECT_TRUE (PRIMITIVETYPE_Integer == integerProperty->GetType ());
    EXPECT_TRUE (PRIMITIVETYPE_Long == longProperty->GetType ());
    EXPECT_TRUE (PRIMITIVETYPE_Point2D == point2DProperty->GetType ());
    EXPECT_TRUE (PRIMITIVETYPE_Point3D == point3DProperty->GetType ());

    class1->CreatePrimitiveProperty (binaryProperty, "BinaryProp2", PRIMITIVETYPE_Binary);
    class1->CreatePrimitiveProperty (booleanProperty, "BooleanProp2", PRIMITIVETYPE_Boolean);
    class1->CreatePrimitiveProperty (dateTimeProperty, "DateTimeProp2", PRIMITIVETYPE_DateTime);
    class1->CreatePrimitiveProperty (doubleProperty, "DoubleProp2", PRIMITIVETYPE_Double);
    class1->CreatePrimitiveProperty (integerProperty, "IntProp2", PRIMITIVETYPE_Integer);
    class1->CreatePrimitiveProperty (longProperty, "LongProp2", PRIMITIVETYPE_Long);
    class1->CreatePrimitiveProperty (point2DProperty, "Point2DProp2", PRIMITIVETYPE_Point2D);
    class1->CreatePrimitiveProperty (point3DProperty, "Point3DProp2", PRIMITIVETYPE_Point3D);

    EXPECT_TRUE (PRIMITIVETYPE_Binary == binaryProperty->GetType ());
    EXPECT_TRUE (PRIMITIVETYPE_Boolean == booleanProperty->GetType ());
    EXPECT_TRUE (PRIMITIVETYPE_DateTime == dateTimeProperty->GetType ());
    EXPECT_TRUE (PRIMITIVETYPE_Double == doubleProperty->GetType ());
    EXPECT_TRUE (PRIMITIVETYPE_Integer == integerProperty->GetType ());
    EXPECT_TRUE (PRIMITIVETYPE_Long == longProperty->GetType ());
    EXPECT_TRUE (PRIMITIVETYPE_Point2D == point2DProperty->GetType ());
    EXPECT_TRUE (PRIMITIVETYPE_Point3D == point3DProperty->GetType ());

    class1->CreateStructProperty (structProp, "StructMember2", *structClass);
    class1->CreateStructArrayProperty (nestedArrayProp, "NestedArray2", structClass);
    class1->CreateArrayProperty (primitiveArrayProp, "PrimitiveArray2", PRIMITIVETYPE_Integer);
    EXPECT_TRUE (ARRAYKIND_Struct == nestedArrayProp->GetKind ());
    EXPECT_TRUE (ARRAYKIND_Primitive == primitiveArrayProp->GetKind ());
    EXPECT_EQ (0, strcmp (structProp->GetType ().GetName ().c_str (), "StructClass"));
    EXPECT_EQ (0, strcmp (nestedArrayProp->GetTypeName ().c_str (), "StructClass"));
    EXPECT_EQ (0, strcmp (primitiveArrayProp->GetTypeName ().c_str (), "int"));

    testSchema->CreateRelationshipClass (relationshipClass, "RelationshipClass");
    EXPECT_TRUE (StrengthType::Referencing == relationshipClass->GetStrength ());
    relationshipClass->SetStrength (StrengthType::Embedding);
    EXPECT_TRUE (StrengthType::Embedding == relationshipClass->GetStrength ());

    EXPECT_TRUE (ECRelatedInstanceDirection::Forward == relationshipClass->GetStrengthDirection ());
    relationshipClass->SetStrengthDirection (ECRelatedInstanceDirection::Backward);
    EXPECT_TRUE (ECRelatedInstanceDirection::Backward == relationshipClass->GetStrengthDirection ());

    EXPECT_TRUE (relationshipClass->GetTarget ().GetIsPolymorphic ());
    EXPECT_TRUE (relationshipClass->GetSource ().GetIsPolymorphic ());
    relationshipClass->GetSource ().SetIsPolymorphic (false);
    EXPECT_FALSE (relationshipClass->GetSource ().GetIsPolymorphic ());

    relationshipClass->SetDescription ("Relates the test class to the related class");
    relationshipClass->SetDisplayLabel ("TestRelationshipClass");

    EXPECT_EQ (0, relationshipClass->GetSource ().GetClasses ().size ());
    EXPECT_EQ (0, relationshipClass->GetTarget ().GetClasses ().size ());

    relationshipClass->GetSource ().AddClass (*class1);
    EXPECT_EQ (1, relationshipClass->GetSource ().GetClasses ().size ());

    relationshipClass->GetTarget ().AddClass (*relatedClass);
    EXPECT_EQ (1, relationshipClass->GetTarget ().GetClasses ().size ());
    relationshipClass->GetTarget ().AddClass (*relatedClass);
    EXPECT_EQ (1, relationshipClass->GetTarget ().GetClasses ().size ());
    relationshipClass->GetTarget().AddClass(*class2);
    EXPECT_EQ(2, relationshipClass->GetTarget().GetClasses().size());

    EXPECT_EQ (0, relationshipClass->GetSource ().GetCardinality ().GetLowerLimit ());
    EXPECT_EQ (0, relationshipClass->GetTarget ().GetCardinality ().GetLowerLimit ());
    EXPECT_EQ (1, relationshipClass->GetSource ().GetCardinality ().GetUpperLimit ());
    EXPECT_EQ (1, relationshipClass->GetTarget ().GetCardinality ().GetUpperLimit ());

    relationshipClass->GetSource ().SetCardinality (RelationshipCardinality::OneMany ());
    EXPECT_EQ (1, relationshipClass->GetSource ().GetCardinality ().GetLowerLimit ());
    EXPECT_TRUE (relationshipClass->GetSource ().GetCardinality ().IsUpperLimitUnbounded ());

    RelationshipCardinality *card = new RelationshipCardinality (2, 5);
    relationshipClass->GetTarget ().SetCardinality (*card);
    EXPECT_EQ (2, relationshipClass->GetTarget ().GetCardinality ().GetLowerLimit ());
    EXPECT_EQ (5, relationshipClass->GetTarget ().GetCardinality ().GetUpperLimit ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaCopyTest, ExpectSuccessWhenCopyingStructs)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"Widgets.01.00.ecschema.xml").c_str (), *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);

    ECSchemaPtr copiedSchema;
    ECObjectsStatus status2 = schema->CopySchema (copiedSchema);
    EXPECT_EQ (ECObjectsStatus::Success, status2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaCopyTest, CopySchemaWithEnumeration)
    {
    ECSchemaPtr schema;
    ECEnumerationP enumeration;
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);

    //Create Enumeration
    auto status = schema->CreateEnumeration(enumeration, "Enumeration", PrimitiveType::PRIMITIVETYPE_Integer);
    ASSERT_TRUE(enumeration != nullptr);
    ASSERT_TRUE(status == ECObjectsStatus::Success);
    EXPECT_TRUE (schema.IsValid ());
    enumeration->SetDisplayLabel("My Display Label");

    ECSchemaPtr copiedSchema = NULL;
    schema->CopySchema (copiedSchema);
    EXPECT_TRUE (copiedSchema.IsValid ());
    ECEnumerationP enumeration2 = copiedSchema->GetEnumerationP("Enumeration");
    ASSERT_TRUE(enumeration2 != nullptr);
    EXPECT_TRUE(enumeration2->GetType() == enumeration->GetType());
    EXPECT_TRUE(enumeration2 != enumeration); //ensure the object was copied and not just referenced
    EXPECT_STREQ(enumeration2->GetDisplayLabel().c_str(), enumeration->GetDisplayLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ClassTest, ExpectErrorWithCircularBaseClasses)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    ECEntityClassP baseClass1;
    ECEntityClassP baseClass2;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    schema->CreateEntityClass (class1, "TestClass");
    schema->CreateEntityClass (baseClass1, "BaseClass1");
    schema->CreateEntityClass (baseClass2, "BaseClass2");

    EXPECT_EQ (ECObjectsStatus::Success, class1->AddBaseClass (*baseClass1));
    EXPECT_EQ (ECObjectsStatus::Success, baseClass1->AddBaseClass (*baseClass2));
    EXPECT_EQ (ECObjectsStatus::BaseClassUnacceptable, baseClass2->AddBaseClass (*class1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ClassTest, GetPropertyCount)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema (schema, "TestSchema", 1, 0);

    ECEntityClassP baseClass1, baseClass2, derivedClass;
    ECStructClassP structClass;

    PrimitiveECPropertyP primProp;
    StructECPropertyP structProp;

    // Struct class with 2 properties
    schema->CreateStructClass (structClass, "StructClass");
    structClass->CreatePrimitiveProperty (primProp, "StructProp1");
    structClass->CreatePrimitiveProperty (primProp, "StructProp2");

    // 1 base class with 3 primitive properties
    schema->CreateEntityClass (baseClass1, "BaseClass1");
    baseClass1->CreatePrimitiveProperty (primProp, "Base1Prop1");
    baseClass1->CreatePrimitiveProperty (primProp, "Base1Prop2");
    baseClass1->CreatePrimitiveProperty (primProp, "Base1Prop3");

    // 1 base class with 1 primitive and 2 struct properties (each struct has 2 properties
    schema->CreateEntityClass (baseClass2, "BaseClass2");
    baseClass2->CreatePrimitiveProperty (primProp, "Base2Prop1");
    baseClass2->CreateStructProperty (structProp, "Base2Prop2", *structClass);
    baseClass2->CreateStructProperty (structProp, "Base2Prop3", *structClass);

    // Derived class with 1 extra primitive property, 1 extra struct property, derived from 2 base classes
    schema->CreateEntityClass (derivedClass, "DerivedClass");
    derivedClass->CreateStructProperty (structProp, "DerivedProp1", *structClass);
    derivedClass->CreatePrimitiveProperty (primProp, "DerivedProp2");
    derivedClass->AddBaseClass (*baseClass1);
    derivedClass->AddBaseClass (*baseClass2);

    TestPropertyCount (*structClass, 2, 2);
    TestPropertyCount (*baseClass1, 3, 3);
    TestPropertyCount (*baseClass2, 3, 3);
    TestPropertyCount (*derivedClass, 2, 8);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsClassInList (bvector<ECClassP> const& classList, ECClassR searchClass)
    {
    bvector<ECClassP>::const_iterator classIterator;

    for (classIterator = classList.begin (); classIterator != classList.end (); classIterator++)
        {
        if (*classIterator == &searchClass)
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ClassTest, AddAndRemoveBaseClass)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    ECEntityClassP baseClass1;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    schema->CreateEntityClass (class1, "TestClass");
    schema->CreateEntityClass (baseClass1, "BaseClass");

    EXPECT_EQ (ECObjectsStatus::Success, class1->AddBaseClass (*baseClass1));

    EXPECT_TRUE (IsClassInList (class1->GetBaseClasses (), *baseClass1));
    EXPECT_TRUE (IsClassInList (baseClass1->GetDerivedClasses (), *class1));

    EXPECT_EQ (ECObjectsStatus::Success, class1->RemoveBaseClass (*baseClass1));

    EXPECT_FALSE (IsClassInList (class1->GetBaseClasses (), *baseClass1));
    EXPECT_FALSE (IsClassInList (baseClass1->GetDerivedClasses (), *class1));

    EXPECT_EQ (ECObjectsStatus::ClassNotFound, class1->RemoveBaseClass (*baseClass1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ClassTest, AddBaseClassWithProperties)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    ECEntityClassP baseClass1;
    ECEntityClassP baseClass2;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    schema->CreateEntityClass (class1, "TestClass");
    schema->CreateEntityClass (baseClass1, "BaseClass");
    schema->CreateEntityClass (baseClass2, "BaseClass2");

    PrimitiveECPropertyP stringProp;
    PrimitiveECPropertyP baseStringProp;
    PrimitiveECPropertyP intProp;
    PrimitiveECPropertyP base2NonIntProp;

    class1->CreatePrimitiveProperty (stringProp, "StringProperty", PRIMITIVETYPE_String);
    baseClass1->CreatePrimitiveProperty (baseStringProp, "StringProperty", PRIMITIVETYPE_String);
    EXPECT_EQ (ECObjectsStatus::Success, class1->AddBaseClass (*baseClass1));

    class1->CreatePrimitiveProperty (intProp, "IntProperty", PRIMITIVETYPE_Integer);
    baseClass2->CreatePrimitiveProperty (base2NonIntProp, "IntProperty", PRIMITIVETYPE_String);
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->AddBaseClass (*baseClass2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ClassTest, BaseClassOrder)
    {
    ECSchemaPtr schema = nullptr;
    ECEntityClassP class1 = nullptr;
    ECEntityClassP baseClass1 = nullptr;
    ECEntityClassP baseClass2 = nullptr;
    ECEntityClassP baseClass3 = nullptr;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    schema->CreateEntityClass (class1, "TestClass");
    schema->CreateEntityClass (baseClass1, "BaseClass");
    schema->CreateEntityClass (baseClass2, "BaseClass2");
    schema->CreateEntityClass (baseClass3, "BaseClass3");

    PrimitiveECPropertyP prop = nullptr;
    class1->CreatePrimitiveProperty (prop, "StringProperty", PRIMITIVETYPE_String);
    baseClass1->CreatePrimitiveProperty (prop, "StringProperty", PRIMITIVETYPE_String);
    baseClass2->CreatePrimitiveProperty (prop, "SstringProperty", PRIMITIVETYPE_String);
    baseClass3->CreatePrimitiveProperty (prop, "StringProperty", PRIMITIVETYPE_String);

    ASSERT_EQ (ECObjectsStatus::Success, class1->AddBaseClass (*baseClass1));
    ASSERT_EQ (ECObjectsStatus::Success, class1->AddBaseClass (*baseClass2));

    ASSERT_EQ (2, class1->GetBaseClasses ().size ());
    ASSERT_TRUE (baseClass1 == class1->GetBaseClasses ()[0]);
    ASSERT_TRUE (baseClass2 == class1->GetBaseClasses ()[1]);

    ASSERT_EQ (ECObjectsStatus::Success, class1->AddBaseClass (*baseClass3, true));
    ASSERT_EQ (3, class1->GetBaseClasses ().size ());
    ASSERT_TRUE (baseClass3 == class1->GetBaseClasses ()[0]);
    ASSERT_TRUE (baseClass1 == class1->GetBaseClasses ()[1]);
    ASSERT_TRUE (baseClass2 == class1->GetBaseClasses ()[2]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ClassTest, IsTests)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    ECEntityClassP baseClass1;
    ECEntityClassP baseClass2;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    schema->CreateEntityClass (class1, "TestClass");
    schema->CreateEntityClass (baseClass1, "BaseClass1");
    schema->CreateEntityClass (baseClass2, "BaseClass2");

    EXPECT_FALSE (class1->Is (baseClass1));
    class1->AddBaseClass (*baseClass1);
    EXPECT_TRUE (class1->Is (baseClass1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ClassTest, CanOverrideBaseProperties)
    {
    ECSchemaPtr schema;
    ECEntityClassP class1;
    ECEntityClassP baseClass1;
    ECStructClassP structClass;
    ECStructClassP structClass2;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    schema->CreateEntityClass (class1, "TestClass");
    schema->CreateEntityClass (baseClass1, "BaseClass1");
    schema->CreateStructClass (structClass, "ClassForStructs");
    schema->CreateStructClass (structClass2, "ClassForStructs2");
    class1->AddBaseClass (*baseClass1);

    PrimitiveECPropertyP baseStringProp;
    PrimitiveECPropertyP baseIntProp;
    PrimitiveECPropertyP baseDoubleProp;
    StructECPropertyP baseStructProp;
    ArrayECPropertyP baseStringArrayProperty;
    StructArrayECPropertyP baseStructArrayProp;

    baseClass1->CreatePrimitiveProperty (baseStringProp, "StringProperty", PRIMITIVETYPE_String);
    baseClass1->CreatePrimitiveProperty (baseIntProp, "IntegerProperty", PRIMITIVETYPE_Integer);
    baseClass1->CreatePrimitiveProperty (baseDoubleProp, "DoubleProperty", PRIMITIVETYPE_Double);
    baseClass1->CreateStructProperty (baseStructProp, "StructProperty", *structClass);
    baseClass1->CreateArrayProperty (baseStringArrayProperty, "StringArrayProperty", PRIMITIVETYPE_String);
    baseClass1->CreateStructArrayProperty (baseStructArrayProp, "StructArrayProperty", structClass);

    PrimitiveECPropertyP longProperty = NULL;
    PrimitiveECPropertyP stringProperty = NULL;

    DISABLE_ASSERTS;
    // Primitives overriding primitives
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreatePrimitiveProperty (longProperty, "StringProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ (NULL, longProperty);
    EXPECT_EQ (ECObjectsStatus::Success, class1->CreatePrimitiveProperty (stringProperty, "StringProperty", PRIMITIVETYPE_String));
    EXPECT_EQ (baseStringProp, stringProperty->GetBaseProperty ());
    class1->RemoveProperty ("StringProperty");

        {
        // Primitives overriding structs
        DISABLE_ASSERTS
            EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreatePrimitiveProperty (longProperty, "StructProperty", PRIMITIVETYPE_Long));
        }

    // Primitives overriding arrays
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreatePrimitiveProperty (longProperty, "StringArrayProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreatePrimitiveProperty (stringProperty, "StringArrayProperty", PRIMITIVETYPE_String));
    class1->RemoveProperty ("StringArrayProperty");

    StructECPropertyP structProperty;

        {
        // Structs overriding primitives
        DISABLE_ASSERTS
            EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty (structProperty, "IntegerProperty"));
        }

    // Structs overriding structs
    // If we don't specify a struct type for the new property, then it should succeed
    EXPECT_EQ (ECObjectsStatus::Success, class1->CreateStructProperty (structProperty, "StructProperty"));
    class1->RemoveProperty ("StructProperty");
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty (structProperty, "StructProperty", *structClass2));

    // Structs overriding arrays
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty (structProperty, "StringArrayProperty"));
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty (structProperty, "StringArrayProperty", *structClass));
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty (structProperty, "StructArrayProperty"));
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty (structProperty, "StructArrayProperty", *structClass));
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreateStructProperty (structProperty, "StructArrayProperty", *structClass2));

    ArrayECPropertyP stringArrayProperty;
    ArrayECPropertyP stringArrayProperty2;
    StructArrayECPropertyP structArrayProperty;
    // Arrays overriding primitives
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreateArrayProperty (stringArrayProperty, "IntegerProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreateArrayProperty (stringArrayProperty, "StringProperty", PRIMITIVETYPE_String));
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreateArrayProperty (stringArrayProperty2, "StringProperty"));

    // Arrays overriding structs
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreateStructArrayProperty (structArrayProperty, "StructProperty", structClass2));
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreateStructArrayProperty (structArrayProperty, "StructProperty", structClass));

    ArrayECPropertyP intArrayProperty;
    // Arrays overriding arrays
    EXPECT_EQ (ECObjectsStatus::DataTypeMismatch, class1->CreateArrayProperty (intArrayProperty, "StringArrayProperty", PRIMITIVETYPE_Long));
    EXPECT_EQ (ECObjectsStatus::Success, class1->CreateArrayProperty (stringArrayProperty, "StringArrayProperty", PRIMITIVETYPE_String));
    class1->RemoveProperty ("StringArrayProperty");

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
TEST_F (ClassTest, ExpectFailureWhenStructTypeIsNotReferenced)
    {
    ECSchemaPtr schema;
    ECSchemaPtr schema2;
    ECEntityClassP class1;
    ECStructClassP structClass;
    ECStructClassP structClass2;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    ECSchema::CreateSchema (schema2, "TestSchema2", 5, 5);
    schema->CreateEntityClass (class1, "TestClass");
    schema2->CreateStructClass (structClass, "ClassForStructs");
    schema->CreateStructClass (structClass2, "ClassForStructs2");

    StructECPropertyP baseStructProp;
    StructArrayECPropertyP structArrayProperty;
    StructECPropertyP baseStructProp2;
    StructArrayECPropertyP structArrayProperty2;

    EXPECT_EQ (ECObjectsStatus::SchemaNotFound, class1->CreateStructProperty (baseStructProp, "StructProperty", *structClass));
    EXPECT_EQ (ECObjectsStatus::SchemaNotFound, class1->CreateStructArrayProperty (structArrayProperty, "StructArrayProperty", structClass));
    EXPECT_EQ (ECObjectsStatus::Success, class1->CreateStructProperty (baseStructProp2, "StructProperty2", *structClass2));
    EXPECT_EQ (ECObjectsStatus::Success, class1->CreateStructArrayProperty (structArrayProperty2, "StructArrayProperty2", structClass2));
    schema->AddReferencedSchema (*schema2);
    EXPECT_EQ (ECObjectsStatus::Success, class1->CreateStructProperty (baseStructProp, "StructProperty", *structClass));
    EXPECT_EQ (ECObjectsStatus::Success, class1->CreateStructArrayProperty (structArrayProperty, "StructArrayProperty", structClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ClassTest, ExpectPropertiesInOrder)
    {
    std::vector<Utf8CP> propertyNames;
    propertyNames.push_back ("beta");
    propertyNames.push_back ("gamma");
    propertyNames.push_back ("delta");
    propertyNames.push_back ("alpha");

    ECSchemaPtr schema;
    ECEntityClassP class1;
    PrimitiveECPropertyP property1;
    PrimitiveECPropertyP property2;
    PrimitiveECPropertyP property3;
    PrimitiveECPropertyP property4;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    schema->CreateEntityClass (class1, "TestClass");
    class1->CreatePrimitiveProperty (property1, "beta");
    class1->CreatePrimitiveProperty (property2, "gamma");
    class1->CreatePrimitiveProperty (property3, "delta");
    class1->CreatePrimitiveProperty (property4, "alpha");

    int i = 0;
    ECPropertyIterable  iterable = class1->GetProperties (false);
    for (ECPropertyP prop : iterable)
        {
        EXPECT_EQ (0, prop->GetName ().compare (propertyNames[i]));
        i++;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ClassTest, ExpectProperties)
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

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    schema->CreateEntityClass (ab, "ab");
    schema->CreateEntityClass (cd, "cd");
    schema->CreateEntityClass (ef, "ef");

    ab->CreatePrimitiveProperty (a, "a");
    ab->CreatePrimitiveProperty (b, "b");

    cd->CreatePrimitiveProperty (c, "c");
    cd->CreatePrimitiveProperty (d, "d");

    ef->CreatePrimitiveProperty (e, "e");
    ef->CreatePrimitiveProperty (f, "f");

    cd->AddBaseClass (*ab);
    ef->AddBaseClass (*cd);

    EXPECT_TRUE (NULL != GetPropertyByName (*ef, "e"));
    EXPECT_TRUE (NULL != GetPropertyByName (*ef, "c"));
    EXPECT_TRUE (NULL != GetPropertyByName (*ef, "a"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ClassTest, ExpectPropertiesFromBaseClass)
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

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    schema->CreateEntityClass (ab, "ab");
    schema->CreateEntityClass (cd, "cd");
    schema->CreateEntityClass (ef, "ef");
    schema->CreateEntityClass (gh, "gh");
    schema->CreateEntityClass (ij, "ij");
    schema->CreateEntityClass (kl, "kl");
    schema->CreateEntityClass (mn, "mn");

    ab->CreatePrimitiveProperty (a, "a");
    ab->CreatePrimitiveProperty (b, "b");

    cd->CreatePrimitiveProperty (c, "c");
    cd->CreatePrimitiveProperty (d, "d");

    ef->CreatePrimitiveProperty (e, "e");
    ef->CreatePrimitiveProperty (f, "f");

    gh->CreatePrimitiveProperty (g, "g");
    gh->CreatePrimitiveProperty (h, "h");

    ij->CreatePrimitiveProperty (i, "i");
    ij->CreatePrimitiveProperty (j, "j");

    kl->CreatePrimitiveProperty (k, "k");
    kl->CreatePrimitiveProperty (l, "l");

    mn->CreatePrimitiveProperty (m, "m");
    mn->CreatePrimitiveProperty (n, "n");

    ef->AddBaseClass (*ab);
    ef->AddBaseClass (*cd);

    kl->AddBaseClass (*gh);
    kl->AddBaseClass (*ij);

    mn->AddBaseClass (*ef);
    mn->AddBaseClass (*kl);

    ECPropertyIterable  iterable1 = mn->GetProperties (true);
    std::vector<ECPropertyP> testVector;
    for (ECPropertyP prop : iterable1)
        testVector.push_back (prop);

    EXPECT_EQ (14, testVector.size ());
    for (size_t i = 0; i < testVector.size (); i++)
        {
        Utf8Char expectedName[] = { (Utf8Char)('a' + static_cast<Utf8Char> (i)), 0 };
        EXPECT_EQ (0, testVector[i]->GetName ().compare (expectedName)) << "Expected: " << expectedName << " Actual: " << testVector[i]->GetName ().c_str ();
        }

    // now we add some duplicate properties to mn which will "override" those from the base classes
    PrimitiveECPropertyP b2;
    PrimitiveECPropertyP d2;
    PrimitiveECPropertyP f2;
    PrimitiveECPropertyP h2;
    PrimitiveECPropertyP j2;
    PrimitiveECPropertyP k2;

    mn->CreatePrimitiveProperty (b2, "b");
    mn->CreatePrimitiveProperty (d2, "d");
    mn->CreatePrimitiveProperty (f2, "f");
    mn->CreatePrimitiveProperty (h2, "h");
    mn->CreatePrimitiveProperty (j2, "j");
    mn->CreatePrimitiveProperty (k2, "k");

    ECPropertyIterable  iterable2 = mn->GetProperties (true);
    testVector.clear ();
    for (ECPropertyP prop : iterable2)
        testVector.push_back (prop);

    EXPECT_EQ (14, testVector.size ());
    bvector<Utf8CP> expectedVector{ "a", "c", "e", "g", "i", "l", "m", "n", "b", "d", "f", "h", "j", "k" };
    for (size_t i = 0; i < testVector.size (); i++)
        EXPECT_EQ (0, testVector[i]->GetName ().compare (expectedVector[i])) << "Expected: " << expectedVector[i] << " Actual: " << testVector[i]->GetName ().c_str ();

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

    kl->CreatePrimitiveProperty (e2, "e");
    kl->CreatePrimitiveProperty (a2, "a");
    kl->CreatePrimitiveProperty (c2, "c");
    kl->CreatePrimitiveProperty (g2, "g");

    ef->CreatePrimitiveProperty (l2, "l");
    gh->CreatePrimitiveProperty (i2, "i");
    ij->CreatePrimitiveProperty (g3, "g");

    gh->CreatePrimitiveProperty (a3, "a");
    gh->CreatePrimitiveProperty (b3, "b");
    ab->CreatePrimitiveProperty (g4, "g");
    ab->CreatePrimitiveProperty (h3, "h");

    ECPropertyIterable  iterable3 = mn->GetProperties (true);
    testVector.clear ();
    for (ECPropertyP prop : iterable3)
        testVector.push_back (prop);

    EXPECT_EQ (14, testVector.size ());
    expectedVector = { "a", "g", "c", "e", "l", "i", "m", "n", "b", "d", "f", "h", "j", "k" };
    for (size_t i = 0; i < testVector.size (); i++)
        EXPECT_EQ (0, testVector[i]->GetName ().compare (expectedVector[i])) << "Expected: " << expectedVector[i] << " Actual: " << testVector[i]->GetName ().c_str ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ClassTest, AddAndRemoveConstraintClasses)
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    ECSchema::CreateSchema (refSchema, "RefSchema", 5, 5);

    ECRelationshipClassP relClass;
    ECEntityClassP targetClass;
    ECEntityClassP sourceClass;

    schema->CreateRelationshipClass (relClass, "RElationshipClass");
    schema->CreateEntityClass (targetClass, "Target");
    refSchema->CreateEntityClass (sourceClass, "Source");

    EXPECT_EQ (ECObjectsStatus::Success, relClass->GetTarget ().AddClass (*targetClass));
    EXPECT_EQ (ECObjectsStatus::SchemaNotFound, relClass->GetSource ().AddClass (*sourceClass));

    schema->AddReferencedSchema (*refSchema);
    EXPECT_EQ (ECObjectsStatus::Success, relClass->GetSource ().AddClass (*sourceClass));

    EXPECT_EQ (ECObjectsStatus::Success, relClass->GetTarget ().RemoveClass (*targetClass));
    EXPECT_EQ (ECObjectsStatus::ClassNotFound, relClass->GetTarget ().RemoveClass (*targetClass));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ClassTest, ExpectReadOnlyFromBaseClass)
    {
    ECSchemaPtr schema;
    ECEntityClassP child;
    ECEntityClassP base;

    PrimitiveECPropertyP readOnlyProp;

    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    schema->CreateEntityClass (base, "BaseClass");
    schema->CreateEntityClass (child, "ChildClass");

    base->CreatePrimitiveProperty (readOnlyProp, "readOnlyProp");
    readOnlyProp->SetIsReadOnly (true);

    ASSERT_EQ (ECObjectsStatus::Success, child->AddBaseClass (*base));

    ECPropertyP ecProp = GetPropertyByName (*child, "readOnlyProp");
    ASSERT_EQ (true, ecProp->GetIsReadOnly ());
    }

void TestOverriding (Utf8CP schemaName, int majorVersion, bool allowOverriding)
    {
    ECSchemaPtr schema;
    ECEntityClassP base;
    ECEntityClassP child;

    ECSchema::CreateSchema (schema, schemaName, majorVersion, 5);
    schema->CreateEntityClass (base, "base");
    schema->CreateEntityClass (child, "child");

    PrimitiveECPropertyP baseIntProp;
    ArrayECPropertyP baseIntArrayProperty;
    ArrayECPropertyP baseStringArrayProperty;
    ArrayECPropertyP baseBoolArrayProperty;

    base->CreatePrimitiveProperty (baseIntProp, "IntegerProperty", PRIMITIVETYPE_Integer);
    base->CreateArrayProperty (baseIntArrayProperty, "IntArrayProperty", PRIMITIVETYPE_Integer);
    base->CreateArrayProperty (baseStringArrayProperty, "StringArrayProperty", PRIMITIVETYPE_String);
    base->CreateArrayProperty (baseBoolArrayProperty, "BoolArrayProperty", PRIMITIVETYPE_Boolean);

    PrimitiveECPropertyP childIntProperty;
    ArrayECPropertyP childIntArrayProperty;
    ArrayECPropertyP childStringArrayProperty;
    ArrayECPropertyP childBoolArrayProperty;

    child->AddBaseClass (*base);
    // Override an integer property with an array of ints
    ECObjectsStatus status = child->CreateArrayProperty (childIntArrayProperty, "IntegerProperty", PRIMITIVETYPE_Integer);
    if (allowOverriding)
        {
        ASSERT_EQ (ECObjectsStatus::Success, status);
        child->RemoveProperty ("IntegerProperty");
        }
    else
        ASSERT_EQ (ECObjectsStatus::DataTypeMismatch, status);

    // Override an integer property with an array of strings
    status = child->CreateArrayProperty (childStringArrayProperty, "IntegerProperty", PRIMITIVETYPE_String);
    ASSERT_EQ (ECObjectsStatus::DataTypeMismatch, status);

    // Override an integer array with an integer
    status = child->CreatePrimitiveProperty (childIntProperty, "IntArrayProperty", PRIMITIVETYPE_Integer);
    if (allowOverriding)
        {
        ASSERT_EQ (ECObjectsStatus::Success, status);
        child->RemoveProperty ("IntArrayProperty");
        }
    else
        ASSERT_EQ (ECObjectsStatus::DataTypeMismatch, status);

    // Override an array of strings with an integer
    status = child->CreatePrimitiveProperty (childIntProperty, "StringArrayProperty", PRIMITIVETYPE_Integer);
    ASSERT_EQ (ECObjectsStatus::DataTypeMismatch, status);

    // Override an array of boolean with an array of integers
    status = child->CreateArrayProperty (childIntArrayProperty, "BoolArrayProperty", PRIMITIVETYPE_Integer);
    ASSERT_EQ (ECObjectsStatus::DataTypeMismatch, status);

    // Override an array of integers with an array of boolean
    status = child->CreateArrayProperty (childBoolArrayProperty, "IntArrayProperty", PRIMITIVETYPE_Boolean);
    ASSERT_EQ (ECObjectsStatus::DataTypeMismatch, status);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ClassTest, TestOverridingArrayPropertyWithNonArray)
    {
    TestOverriding ("TestSchema", 5, false);
    TestOverriding ("jclass", 1, true);
    TestOverriding ("jclass", 2, true);
    TestOverriding ("ECXA_ams", 1, true);
    TestOverriding ("ECXA_ams_user", 1, true);
    TestOverriding ("ams", 1, true);
    TestOverriding ("ams_user", 1, true);
    TestOverriding ("Bentley_JSpace_CustomAttributes", 2, true);
    TestOverriding ("Bentley_Plant", 6, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECNameValidationTest : ECTestFixture
    {
    struct ITester
        {
        virtual void        Preprocess (ECSchemaR schema) const = 0;
        virtual void        Postprocess (ECSchemaR schema) const = 0;
        };

    void Roundtrip (ITester const& tester)
        {
        ECSchemaPtr schema;
        ECSchema::CreateSchema (schema, "MySchema", 1, 1);
        tester.Preprocess (*schema);

        Utf8String schemaXml;
        EXPECT_EQ (SchemaWriteStatus::Success, schema->WriteToXmlString (schemaXml));

        schema = NULL;
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext ();
        EXPECT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString (schema, schemaXml.c_str (), *context));

        tester.Postprocess (*schema);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct DisplayLabelTester : ECNameValidationTest::ITester
    {
    Utf8String         m_name;
    Utf8String         m_encodedName;

    DisplayLabelTester (Utf8CP name, Utf8CP encodedName) : m_name (name), m_encodedName (encodedName) {}

    template<typename T> void Compare (T const& target) const
        {
        EXPECT_FALSE (target.GetIsDisplayLabelDefined ());
        EXPECT_TRUE (target.GetName ().Equals (m_encodedName)) << "Name: Expected " << m_encodedName.c_str () << " Actual " << target.GetName ().c_str ();
        EXPECT_TRUE (target.GetDisplayLabel ().Equals (m_name)) << "Label: Expected " << m_name.c_str () << " Actual " << target.GetDisplayLabel ().c_str ();
        }

    template<typename T> void CompareOverriddenLabel (T const& target, Utf8CP label) const
        {
        EXPECT_TRUE (target.GetIsDisplayLabelDefined ());
        EXPECT_TRUE (target.GetDisplayLabel ().Equals (label));
        }

    virtual void Preprocess (ECSchemaR schema) const override
        {
        // This test used to rely on SetName() automatically encoding a non-EC name.
        // We removed that behavior because it diverges from managed EC (which throws an "invalid name" exception instead)
        // So now we must explicitly encode the name first.
        Utf8String encodedName;
        EXPECT_EQ (!ECNameValidation::IsValidName (m_name.c_str()), ECNameValidation::EncodeToValidName (encodedName, m_name));
        schema.SetName (encodedName);
        Compare (schema);

        ECEntityClassP ecclass;
        schema.CreateEntityClass (ecclass, encodedName);
        Compare (*ecclass);

        PrimitiveECPropertyP ecprop;
        ecclass->CreatePrimitiveProperty (ecprop, encodedName, PRIMITIVETYPE_String);
        Compare (*ecprop);
        }

    virtual void Postprocess (ECSchemaR schema) const override
        {
        ECClassP ecclass = schema.GetClassP (m_encodedName.c_str ());
        ECPropertyP ecprop = GetPropertyByName (*ecclass, m_encodedName.c_str ());

        Compare (schema);
        Compare (*ecclass);
        Compare (*ecprop);

        // Test explicitly setting display labels
        schema.SetDisplayLabel ("NewDisplayLabel");
        CompareOverriddenLabel (schema, "NewDisplayLabel");
        ecclass->SetDisplayLabel ("1!@$");
        CompareOverriddenLabel (*ecclass, "1!@$");                // will not be encoded
        ecprop->SetDisplayLabel ("__x003E__");
        CompareOverriddenLabel (*ecprop, "__x003E__");            // will not be decoded

        // Test explicitly un-setting display labels
        ecclass->SetDisplayLabel ("");
        Compare (*ecclass);
        ecprop->SetDisplayLabel ("");
        Compare (*ecprop);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECNameValidationTest, DisplayLabels)
    {
    // Chinese chars...see TFS#298776...we are stuck with UTF-16 persistent encoding from long ago, should round-trip correctly with UTF-8
    static const unsigned char s_chineseUtf8[] = { 0xE8, 0x88, 0xAC, 
                                     0xE6, 0xA8, 0xA1,
                                     0xE5, 0x9E, 0x8B,
                                     '\0' };
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
        (Utf8CP)s_chineseUtf8, "__x822C____x6A21____x578B__",
        NULL, NULL
        };

    for (Utf8CP const* cur = s_testValues; *cur; cur += 2)
        {
        Utf8CP name = *cur, encoded = *(cur + 1);
        Roundtrip (DisplayLabelTester (name, encoded));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECNameValidationTest, Validate)
    {
#define EXPECT_VALIDATION_RESULT(RESULTTOEXPECT, NAMETOTEST) EXPECT_EQ (ECNameValidation::RESULT_ ## RESULTTOEXPECT, ECNameValidation::Validate (NAMETOTEST))
    EXPECT_VALIDATION_RESULT (Valid, "ThisIsAValidName");
    EXPECT_VALIDATION_RESULT (Valid, "_123");
    EXPECT_VALIDATION_RESULT (Valid, "___");
    EXPECT_VALIDATION_RESULT (Valid, "A123");

    EXPECT_VALIDATION_RESULT (NullOrEmpty, "");
    EXPECT_VALIDATION_RESULT (NullOrEmpty, NULL);

    EXPECT_VALIDATION_RESULT (BeginsWithDigit, "1_C");

    EXPECT_VALIDATION_RESULT (IncludesInvalidCharacters, "!ABC");
    EXPECT_VALIDATION_RESULT (IncludesInvalidCharacters, "ABC@");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaComparisonTest, VerifyMatchesOperator)
    {
    EXPECT_TRUE (SchemaKey ("SchemaTest", 1, 0) == SchemaKey ("SchemaTest", 1, 0));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0) == SchemaKey ("SchemaNotTest", 1, 0));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0) == SchemaKey ("SchemaTest", 2, 0));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0) == SchemaKey ("SchemaTest", 1, 1));

    EXPECT_TRUE (SchemaKey ("SchemaTest", 1, 0).Matches (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::Exact));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).Matches (SchemaKey ("SchemaNotTest", 1, 0), SchemaMatchType::Exact));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).Matches (SchemaKey ("SchemaTest", 2, 0), SchemaMatchType::Exact));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).Matches (SchemaKey ("SchemaTest", 1, 1), SchemaMatchType::Exact));

    EXPECT_TRUE (SchemaKey ("SchemaTest", 1, 0).Matches (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::Identical));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).Matches (SchemaKey ("SchemaNotTest", 1, 0), SchemaMatchType::Identical));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).Matches (SchemaKey ("SchemaTest", 2, 0), SchemaMatchType::Identical));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).Matches (SchemaKey ("SchemaTest", 1, 1), SchemaMatchType::Identical));

    EXPECT_TRUE (SchemaKey ("SchemaTest", 1, 0).Matches (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::Latest));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).Matches (SchemaKey ("SchemaNotTest", 1, 0), SchemaMatchType::Latest));
    EXPECT_TRUE (SchemaKey ("SchemaTest", 1, 0).Matches (SchemaKey ("SchemaTest", 2, 0), SchemaMatchType::Latest));
    EXPECT_TRUE (SchemaKey ("SchemaTest", 1, 1).Matches (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::Latest));

    EXPECT_TRUE (SchemaKey ("SchemaTest", 1, 0).Matches (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::LatestCompatible));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).Matches (SchemaKey ("SchemaNotTest", 1, 0), SchemaMatchType::LatestCompatible));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).Matches (SchemaKey ("SchemaTest", 2, 0), SchemaMatchType::LatestCompatible));
    EXPECT_TRUE (SchemaKey ("SchemaTest", 1, 1).Matches (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::LatestCompatible));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).Matches (SchemaKey ("SchemaTest", 1, 1), SchemaMatchType::LatestCompatible));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaComparisonTest, VerifyLessThanOperator)
    {
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0) < SchemaKey ("SchemaTest", 1, 0));
    EXPECT_TRUE (SchemaKey ("SchemaTesa", 1, 0) < SchemaKey ("SchemaTest", 1, 0));
    EXPECT_TRUE (SchemaKey ("SchemaTest", 1, 0) < SchemaKey ("SchemaTest", 2, 0));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 1) < SchemaKey ("SchemaTest", 1, 0));

    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).LessThan (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::Exact));
    EXPECT_TRUE (SchemaKey ("SchemaTesa", 1, 0).LessThan (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::Exact));
    EXPECT_TRUE (SchemaKey ("SchemaTest", 1, 0).LessThan (SchemaKey ("SchemaTest", 2, 0), SchemaMatchType::Exact));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 1).LessThan (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::Exact));

    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).LessThan (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::Identical));
    EXPECT_TRUE (SchemaKey ("SchemaTesa", 1, 0).LessThan (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::Identical));
    EXPECT_TRUE (SchemaKey ("SchemaTest", 1, 0).LessThan (SchemaKey ("SchemaTest", 2, 0), SchemaMatchType::Identical));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 1).LessThan (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::Identical));

    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).LessThan (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::Latest));
    EXPECT_TRUE (SchemaKey ("SchemaTesa", 1, 0).LessThan (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::Latest));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).LessThan (SchemaKey ("SchemaTest", 2, 0), SchemaMatchType::Latest));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).LessThan (SchemaKey ("SchemaTest", 1, 9), SchemaMatchType::Latest));

    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).LessThan (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::LatestCompatible));
    EXPECT_TRUE (SchemaKey ("SchemaTesa", 1, 0).LessThan (SchemaKey ("SchemaTest", 1, 0), SchemaMatchType::LatestCompatible));
    EXPECT_TRUE (SchemaKey ("SchemaTest", 1, 0).LessThan (SchemaKey ("SchemaTest", 2, 0), SchemaMatchType::LatestCompatible));
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0).LessThan (SchemaKey ("SchemaTest", 1, 9), SchemaMatchType::LatestCompatible));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaComparisonTest, VerifyNotMatchesOperator)
    {
    EXPECT_FALSE (SchemaKey ("SchemaTest", 1, 0) != SchemaKey ("SchemaTest", 1, 0));
    EXPECT_TRUE (SchemaKey ("SchemaTest", 1, 0) != SchemaKey ("SchemaNotTest", 1, 0));
    EXPECT_TRUE (SchemaKey ("SchemaTest", 1, 0) != SchemaKey ("SchemaTest", 2, 0));
    EXPECT_TRUE (SchemaKey ("SchemaTest", 1, 0) != SchemaKey ("SchemaTest", 1, 1));
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                  Raimondas.Rimkus 02/2013
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaCacheTest, LoadAndGetSchema)
    {
    ECSchemaCachePtr cache = ECSchemaCache::Create ();
    ECSchemaPtr schema1;
    ECSchemaPtr schema2;
    ECSchemaPtr schema3;

    ECSchema::CreateSchema (schema1, "Widget", 5, 1);
    ECSchema::CreateSchema (schema2, "BaseSchema1", 2, 0);
    ECSchema::CreateSchema (schema3, "BaseSchema2", 5, 5);

    EXPECT_TRUE (cache->AddSchema (*schema1) == ECObjectsStatus::Success);
    EXPECT_TRUE (cache->AddSchema (*schema2) == ECObjectsStatus::Success);
    EXPECT_TRUE (cache->AddSchema (*schema3) == ECObjectsStatus::Success);
    EXPECT_EQ (cache->GetCount (), 3);

    ECSchemaPtr fetchedSchema = cache->GetSchema (SchemaKey ("BaseSchema1", 2, 0));
    ASSERT_TRUE (fetchedSchema.IsValid ());
    EXPECT_TRUE (fetchedSchema->GetSchemaKey () == SchemaKey ("BaseSchema1", 2, 0));

    cache->Clear ();
    EXPECT_EQ (cache->GetCount (), 0);
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                  Raimondas.Rimkus 02/2013
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaCacheTest, FilterSchema)
    {
    ECSchemaCachePtr cache = ECSchemaCache::Create ();
    ECSchemaPtr schema1;
    ECSchemaPtr schema2;
    ECSchemaPtr schema3;

    ECSchema::CreateSchema (schema1, "Widget", 5, 1);
    ECSchema::CreateSchema (schema2, "BaseSchema1", 2, 0);
    ECSchema::CreateSchema (schema3, "BaseSchema2", 5, 5);

    EXPECT_TRUE (cache->AddSchema (*schema1) == ECObjectsStatus::Success);
    EXPECT_TRUE (cache->AddSchema (*schema2) == ECObjectsStatus::Success);
    EXPECT_TRUE (cache->AddSchema (*schema3) == ECObjectsStatus::Success);

    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseSchema1", 2, 0), SchemaMatchType::Exact) != NULL);
    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseZchema1", 2, 0), SchemaMatchType::Exact) == NULL);
    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseSchema1", 3, 0), SchemaMatchType::Exact) == NULL);
    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseSchema1", 2, 1), SchemaMatchType::Exact) == NULL);

    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseSchema1", 2, 0), SchemaMatchType::Identical) != NULL);
    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseZchema1", 2, 0), SchemaMatchType::Identical) == NULL);
    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseSchema1", 3, 0), SchemaMatchType::Identical) == NULL);
    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseSchema1", 2, 1), SchemaMatchType::Identical) == NULL);

    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseSchema1", 2, 0), SchemaMatchType::Latest) != NULL);
    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseZchema1", 2, 0), SchemaMatchType::Latest) == NULL);
    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseSchema1", 3, 0), SchemaMatchType::Latest) != NULL);
    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseSchema1", 2, 1), SchemaMatchType::Latest) != NULL);

    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseSchema2", 5, 5), SchemaMatchType::LatestCompatible) != NULL);
    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseZchema2", 5, 5), SchemaMatchType::LatestCompatible) == NULL);
    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseSchema2", 3, 5), SchemaMatchType::LatestCompatible) == NULL);
    EXPECT_TRUE (cache->GetSchema (SchemaKey ("BaseSchema2", 5, 3), SchemaMatchType::LatestCompatible) != NULL);
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                  Raimondas.Rimkus 02/2013
 +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaCacheTest, DropSchema)
    {
    ECSchemaCachePtr cache = ECSchemaCache::Create ();
    ECSchemaPtr schema1;
    ECSchemaPtr schema2;
    ECSchemaPtr schema3;

    ECSchema::CreateSchema (schema1, "Widget", 5, 1);
    ECSchema::CreateSchema (schema2, "BaseSchema1", 2, 0);
    ECSchema::CreateSchema (schema3, "BaseSchema2", 5, 5);

    EXPECT_TRUE (cache->AddSchema (*schema1) == ECObjectsStatus::Success);
    EXPECT_TRUE (cache->AddSchema (*schema2) == ECObjectsStatus::Success);
    EXPECT_TRUE (cache->AddSchema (*schema3) == ECObjectsStatus::Success);
    EXPECT_EQ (cache->GetCount (), 3);

    EXPECT_TRUE (cache->DropSchema (SchemaKey ("Widget", 5, 1)) == ECObjectsStatus::Success);
    EXPECT_EQ (cache->GetCount (), 2);
    EXPECT_TRUE (cache->DropSchema (SchemaKey ("Widget", 5, 1)) != ECObjectsStatus::Success);
    EXPECT_TRUE (cache->DropSchema (SchemaKey ("BaseSchema2", 5, 3)) != ECObjectsStatus::Success);
    EXPECT_TRUE (cache->DropSchema (SchemaKey ("BaseSchema2", 5, 7)) != ECObjectsStatus::Success);
    EXPECT_TRUE (cache->DropSchema (SchemaKey ("BaseSchema2", 1, 5)) != ECObjectsStatus::Success);
    EXPECT_EQ (cache->GetCount (), 2);
    EXPECT_TRUE (cache->DropSchema (SchemaKey ("BaseSchema2", 5, 5)) == ECObjectsStatus::Success);
    EXPECT_EQ (cache->GetCount (), 1);
    EXPECT_TRUE (cache->DropSchema (SchemaKey ("BaseSchema1", 2, 0)) == ECObjectsStatus::Success);
    EXPECT_EQ (cache->GetCount (), 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaChecksumTest, ComputeSchemaXmlStringCheckSum)
    {
    Utf8Char schemaXml[] =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"Widgets\" version=\"09.06\" displayLabel=\"Widgets Display Label\" description=\"Widgets Description\" nameSpacePrefix=\"wid\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ec=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" xmlns:ods=\"Bentley_ODS.01.02\">"
        "    <ECClass typeName=\"ecProject\" description=\"Project ECClass\" displayLabel=\"Project\" isDomainClass=\"True\">"
        "       <ECProperty propertyName=\"Name\" typename=\"string\" displayLabel=\"Project Name\" />"
        "    </ECClass>"
        "</ECSchema>";

    EXPECT_EQ (ECSchema::ComputeSchemaXmlStringCheckSum (schemaXml, sizeof(schemaXml)), 682119251);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchemaImmutableTest, SetImmutable)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext ();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"Widgets.01.00.ecschema.xml").c_str (), *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);

    schema->SetImmutable ();

    ECEntityClassP class1 = NULL;
    ECClassP class2 = schema->GetClassP ("ecProject");
    ECRelationshipClassP relationshipClass;
    ECClassP base = (ECClassP) class1;
    EXPECT_EQ (schema->CreateEntityClass (class1, "TestClass"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_TRUE (class1 == NULL);
    EXPECT_EQ (schema->CopyClass (base, *class2), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ (schema->CreateRelationshipClass (relationshipClass, "RelationshipClass"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ (schema->SetName ("Some new name"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ (schema->SetNamespacePrefix ("Some new prefix"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ (schema->SetDescription ("Some new description"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ (schema->SetDisplayLabel ("Some new label"), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ (schema->SetVersionMajor (13), ECObjectsStatus::SchemaIsImmutable);
    EXPECT_EQ (schema->SetVersionMinor (13), ECObjectsStatus::SchemaIsImmutable);
    }
END_BENTLEY_ECN_TEST_NAMESPACE
