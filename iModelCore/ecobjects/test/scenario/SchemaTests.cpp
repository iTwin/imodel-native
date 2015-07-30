/*--------------------------------------------------------------------------------------+
|
|     $Source: test/scenario/SchemaTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaTest                : ECTestFixture {};
struct SchemaDeserializationTest : ECTestFixture {};
struct SchemaSearchTest          : ECTestFixture {};
struct SchemaComparisonTest      : ECTestFixture {};
struct SchemaCacheTest           : ECTestFixture {};
struct SchemaReferenceTest       : ECTestFixture {};
struct SchemaChecksumTest        : ECTestFixture {};
struct SchemaImmutableTest       : ECTestFixture {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP GetPropertyByName (ECClassCR ecClass, Utf8CP name, bool expectExists = true)
    {
    ECPropertyP prop = ecClass.GetPropertyP (name);
    EXPECT_EQ (expectExists, NULL != prop);
    Utf8String utf8 (name);
    prop = ecClass.GetPropertyP (utf8.c_str());
    EXPECT_EQ (expectExists, NULL != prop);
    return prop;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void VerifyWidgetsSchema
(
ECSchemaPtr const&   schema
)
    {
    EXPECT_STREQ ("Widgets", schema->GetName().c_str());
    EXPECT_STREQ ("wid", schema->GetNamespacePrefix().c_str());
    EXPECT_STREQ ("Widgets Display Label", schema->GetDisplayLabel().c_str());
    EXPECT_TRUE (schema->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("Widgets Description", schema->GetDescription().c_str());
    EXPECT_EQ (9, schema->GetVersionMajor());
    EXPECT_EQ (6, schema->GetVersionMinor());        
 
#ifdef DEBUG_PRINT
    for (ECClassP pClass: schema->GetClasses())
        {
        printf ("Widgets contains class: '%s' with display label '%s'\n", pClass->GetName().c_str(), pClass->GetDisplayLabel().c_str());
        }
#endif

    ECClassP pClass = schema->GetClassP("ClassDoesNotExistInSchema");
    EXPECT_FALSE (pClass);

    pClass = schema->GetClassP("ecProject");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ ("ecProject", pClass->GetName().c_str());    
    EXPECT_STREQ ("Project", pClass->GetDisplayLabel().c_str());
    EXPECT_TRUE (pClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("Project Class", pClass->GetDescription().c_str());
    EXPECT_FALSE (pClass->GetIsStruct());
    EXPECT_FALSE (pClass->GetIsCustomAttributeClass());
    EXPECT_TRUE (pClass->GetIsDomainClass());
    EXPECT_FALSE (pClass->HasBaseClasses());
    ECPropertyP pProperty = GetPropertyByName (*pClass, "Name");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("Name", pProperty->GetName().c_str());
    EXPECT_TRUE (pProperty->GetIsPrimitive());
    EXPECT_FALSE (pProperty->GetIsStruct());
    EXPECT_FALSE (pProperty->GetIsArray());
    EXPECT_STREQ ("string", pProperty->GetTypeName().c_str());
    EXPECT_TRUE (PRIMITIVETYPE_String == pProperty->GetAsPrimitiveProperty()->GetType());
    EXPECT_TRUE (pProperty->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("Project Name", pProperty->GetDisplayLabel().c_str());
    EXPECT_STREQ ("", pProperty->GetDescription().c_str());
    EXPECT_EQ (pClass, &pProperty->GetClass());
    EXPECT_FALSE (pProperty->GetIsReadOnly());

    pProperty = GetPropertyByName (*pClass, "PropertyDoesNotExistInClass", false);
    EXPECT_FALSE (pProperty);

    ECClassP customAttribClass = schema->GetClassP("AccessCustomAttributes");
    ASSERT_TRUE (NULL != customAttribClass);
    EXPECT_STREQ ("AccessCustomAttributes", customAttribClass->GetName().c_str());    
    EXPECT_STREQ ("AccessCustomAttributes", customAttribClass->GetDisplayLabel().c_str());
    EXPECT_FALSE (customAttribClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("", customAttribClass->GetDescription().c_str());
    EXPECT_FALSE (customAttribClass->GetIsStruct());
    EXPECT_TRUE (customAttribClass->GetIsCustomAttributeClass());
    EXPECT_FALSE (customAttribClass->GetIsDomainClass());
    EXPECT_FALSE (customAttribClass->HasBaseClasses());

    pClass = schema->GetClassP("Struct1");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ ("Struct1", pClass->GetName().c_str());    
    EXPECT_STREQ ("Struct1", pClass->GetDisplayLabel().c_str());
    EXPECT_FALSE (pClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("", pClass->GetDescription().c_str());
    EXPECT_TRUE (pClass->GetIsStruct());
    EXPECT_FALSE (pClass->GetIsCustomAttributeClass());
    EXPECT_FALSE (pClass->GetIsDomainClass());
    EXPECT_FALSE (pClass->HasBaseClasses());

    pClass = schema->GetClassP("Struct2");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ ("Struct2", pClass->GetName().c_str());    
    EXPECT_STREQ ("Struct2", pClass->GetDisplayLabel().c_str());
    EXPECT_FALSE (pClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("", pClass->GetDescription().c_str());
    EXPECT_TRUE (pClass->GetIsStruct());
    EXPECT_FALSE (pClass->GetIsCustomAttributeClass());
    EXPECT_TRUE (pClass->GetIsDomainClass());
    EXPECT_FALSE (pClass->HasBaseClasses());
    pProperty = GetPropertyByName (*pClass, "NestedArray");
    EXPECT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("NestedArray", pProperty->GetName().c_str());
    EXPECT_FALSE (pProperty->GetIsPrimitive());
    EXPECT_FALSE (pProperty->GetIsStruct());
    EXPECT_TRUE (pProperty->GetIsArray());
    EXPECT_STREQ ("Struct1", pProperty->GetTypeName().c_str());
    ArrayECPropertyP arrayProperty = pProperty->GetAsArrayPropertyP();
    EXPECT_TRUE (ARRAYKIND_Struct == arrayProperty->GetKind());
    EXPECT_EQ (schema->GetClassP("Struct1"), arrayProperty->GetStructElementType());
    EXPECT_EQ (0, arrayProperty->GetMinOccurs());
    EXPECT_EQ (UINT_MAX, arrayProperty->GetMaxOccurs());    
    EXPECT_FALSE (pProperty->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("NestedArray", pProperty->GetDisplayLabel().c_str());
    EXPECT_STREQ ("", pProperty->GetDescription().c_str());
    EXPECT_EQ (pClass, &pProperty->GetClass());
    EXPECT_FALSE (pProperty->GetIsReadOnly());

    pClass = schema->GetClassP("TestClass");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_TRUE (pClass->HasBaseClasses());
    pProperty = GetPropertyByName (*pClass, "EmbeddedStruct");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ ("EmbeddedStruct", pProperty->GetName().c_str());
    EXPECT_FALSE (pProperty->GetIsPrimitive());
    EXPECT_TRUE (pProperty->GetIsStruct());
    EXPECT_FALSE (pProperty->GetIsArray());
    EXPECT_STREQ ("Struct1", pProperty->GetTypeName().c_str());
    StructECPropertyP structProperty = pProperty->GetAsStructPropertyP();    
    EXPECT_EQ (schema->GetClassP("Struct1"), &(structProperty->GetType()));
    EXPECT_FALSE (pProperty->GetIsDisplayLabelDefined());
    EXPECT_STREQ ("EmbeddedStruct", pProperty->GetDisplayLabel().c_str());
    EXPECT_STREQ ("", pProperty->GetDescription().c_str());
    EXPECT_EQ (pClass, &pProperty->GetClass());
    EXPECT_FALSE (pProperty->GetIsReadOnly());
    
    IECInstancePtr instance = pClass->GetCustomAttribute(*customAttribClass);
    EXPECT_TRUE(instance.IsValid());

    ECValue ecValue;
    EXPECT_EQ (SUCCESS, instance->GetValue (ecValue, "AccessLevel"));
    EXPECT_EQ (4, ecValue.GetInteger());

    EXPECT_EQ (SUCCESS, instance->GetValue (ecValue, "Writeable"));
    EXPECT_FALSE (ecValue.GetBoolean());
   
#ifdef DEBUG_PRINT
    for (ECPropertyP pProperty: pClass->GetProperties())
        {
        printf ("TestClass contains property: %s of type %s\n", pProperty->GetName().c_str(), pProperty->GetTypeName().c_str());
        }
#endif   
    }

TEST_F(SchemaTest,ExpectReadOnly)
{
    ECSchemaPtr schema;   
    ECClassP domainClass;
    ECClassP derivedClass;
    ECClassP structClass;
    ECClassP customAttributeClass;
 
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    ASSERT_TRUE(schema.IsValid());
 
    //Create Domain Class
    schema->CreateClass(domainClass,"DomainClass");
    ASSERT_TRUE(domainClass!=NULL);
    domainClass->SetIsDomainClass(true);
 
    //Create Derived Class
    schema->CreateClass(derivedClass,"DerivedClass");
    ASSERT_TRUE(derivedClass!=NULL);
 
    //Create Struct
    schema->CreateClass(structClass,"StructClass");
    ASSERT_TRUE(structClass!=NULL);
    structClass->SetIsStruct(true);
 
    //Add Property of Array type to structClass
    ArrayECPropertyP MyArrayProp;
    structClass->CreateArrayProperty(MyArrayProp,"ArrayProperty");
    ASSERT_TRUE(MyArrayProp!=NULL);
 
    //Create customAttributeClass
    schema->CreateClass(customAttributeClass,"CustomAttribute");
    ASSERT_TRUE(customAttributeClass!=NULL);
    customAttributeClass->SetIsCustomAttributeClass(true);
 
    //Add Property Of Struct type to custom attribute
    StructECPropertyP PropertyOfCustomAttribute;
    customAttributeClass->CreateStructProperty(PropertyOfCustomAttribute, "PropertyOfCustomAttribute",*structClass);
    ASSERT_TRUE(PropertyOfCustomAttribute!=NULL);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaDeserializationTest, ExpectSuccessWhenRoundtripUsingFile)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath( L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);
    VerifyWidgetsSchema(schema);
    
    SchemaWriteStatus status2 = schema->WriteToXmlFile(ECTestFixture::GetTempDataPath( L"widgets.xml").c_str());
    EXPECT_EQ (SCHEMA_WRITE_STATUS_Success, status2);
    
    ECSchemaPtr deserializedSchema;
    schemaContext = ECSchemaReadContext::CreateContext();
    status = ECSchema::ReadFromXmlFile(deserializedSchema, ECTestFixture::GetTempDataPath( L"widgets.xml").c_str(), *schemaContext);
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status); 
    VerifyWidgetsSchema(deserializedSchema);
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
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath( L"SchemaThatReferences.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);    

    EXPECT_TRUE (schema->FindSchema(SchemaKey("SchemaThatReferences", 1, 0), SCHEMAMATCHTYPE_Exact) != NULL);
    EXPECT_TRUE (schema->FindSchema(SchemaKey("SchemaThatReferencez", 1, 0), SCHEMAMATCHTYPE_Exact) == NULL);
    EXPECT_TRUE (schema->FindSchema(SchemaKey("SchemaThatReferences", 2, 0), SCHEMAMATCHTYPE_Exact) == NULL);
    EXPECT_TRUE (schema->FindSchema(SchemaKey("SchemaThatReferences", 1, 1), SCHEMAMATCHTYPE_Exact) == NULL);
    
    EXPECT_TRUE (schema->FindSchema(SchemaKey("BaseSchema", 1, 0), SCHEMAMATCHTYPE_Exact) != NULL);
    EXPECT_TRUE (schema->FindSchemaP(SchemaKey("SchemaThatReferences", 1, 0), SCHEMAMATCHTYPE_Exact) != NULL);
    EXPECT_TRUE (schema->FindSchemaP(SchemaKey("a", 123, 456), SCHEMAMATCHTYPE_Exact) == NULL);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, FindClassInReferenceList)
    {
    ECSchemaPtr schema, refSchema;
    ECSchema::CreateSchema (schema, "TestSchema", 5, 5);
    ECSchema::CreateSchema (refSchema, "RefSchema", 5, 5);
    
    ECRelationshipClassP relClass;
    ECClassP targetClass, sourceClass;
    schema->CreateRelationshipClass (relClass, "RElationshipClass");
    schema->CreateClass (targetClass, "Target");
    refSchema->CreateClass (sourceClass, "Source");
    
    EXPECT_EQ (ECOBJECTS_STATUS_Success, schema->AddReferencedSchema (*refSchema));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, relClass->GetTarget().AddClass(*targetClass));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, relClass->GetSource().AddClass(*sourceClass));
    
    ECSchemaReferenceListCR refList = schema->GetReferencedSchemas();
    
    EXPECT_TRUE (refList.FindClassP(SchemaNameClassNamePair("RefSchema", "Source")) != NULL);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaComparisonTest, VerifyMatchesOperator)
    {
    EXPECT_TRUE (SchemaKey("SchemaTest", 1, 0) == SchemaKey("SchemaTest", 1, 0));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0) == SchemaKey("SchemaNotTest", 1, 0));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0) == SchemaKey("SchemaTest", 2, 0));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0) == SchemaKey("SchemaTest", 1, 1));
    
    EXPECT_TRUE (SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_Exact));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaNotTest", 1, 0), SCHEMAMATCHTYPE_Exact));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 2, 0), SCHEMAMATCHTYPE_Exact));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 1), SCHEMAMATCHTYPE_Exact));
    
    EXPECT_TRUE (SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_Identical));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaNotTest", 1, 0), SCHEMAMATCHTYPE_Identical));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 2, 0), SCHEMAMATCHTYPE_Identical));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 1), SCHEMAMATCHTYPE_Identical));
    
    EXPECT_TRUE (SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_Latest));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaNotTest", 1, 0), SCHEMAMATCHTYPE_Latest));
    EXPECT_TRUE (SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 2, 0), SCHEMAMATCHTYPE_Latest));
    EXPECT_TRUE (SchemaKey("SchemaTest", 1, 1).Matches(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_Latest));
    
    EXPECT_TRUE (SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_LatestCompatible));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaNotTest", 1, 0), SCHEMAMATCHTYPE_LatestCompatible));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 2, 0), SCHEMAMATCHTYPE_LatestCompatible));
    EXPECT_TRUE (SchemaKey("SchemaTest", 1, 1).Matches(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_LatestCompatible));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).Matches(SchemaKey("SchemaTest", 1, 1), SCHEMAMATCHTYPE_LatestCompatible));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaComparisonTest, VerifyLessThanOperator)
    {
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0) < SchemaKey("SchemaTest", 1, 0));
    EXPECT_TRUE (SchemaKey("SchemaTesa", 1, 0) < SchemaKey("SchemaTest", 1, 0));
    EXPECT_TRUE (SchemaKey("SchemaTest", 1, 0) < SchemaKey("SchemaTest", 2, 0));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 1) < SchemaKey("SchemaTest", 1, 0));
    
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_Exact));
    EXPECT_TRUE (SchemaKey("SchemaTesa", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_Exact));
    EXPECT_TRUE (SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 2, 0), SCHEMAMATCHTYPE_Exact));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 1).LessThan(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_Exact));
    
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_Identical));
    EXPECT_TRUE (SchemaKey("SchemaTesa", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_Identical));
    EXPECT_TRUE (SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 2, 0), SCHEMAMATCHTYPE_Identical));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 1).LessThan(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_Identical));
    
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_Latest));
    EXPECT_TRUE (SchemaKey("SchemaTesa", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_Latest));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 2, 0), SCHEMAMATCHTYPE_Latest));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 9), SCHEMAMATCHTYPE_Latest));
    
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_LatestCompatible));
    EXPECT_TRUE (SchemaKey("SchemaTesa", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 0), SCHEMAMATCHTYPE_LatestCompatible));
    EXPECT_TRUE (SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 2, 0), SCHEMAMATCHTYPE_LatestCompatible));
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0).LessThan(SchemaKey("SchemaTest", 1, 9), SCHEMAMATCHTYPE_LatestCompatible));
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaComparisonTest, VerifyNotMatchesOperator)
    {
    EXPECT_FALSE (SchemaKey("SchemaTest", 1, 0) != SchemaKey("SchemaTest", 1, 0));
    EXPECT_TRUE (SchemaKey("SchemaTest", 1, 0) != SchemaKey("SchemaNotTest", 1, 0));
    EXPECT_TRUE (SchemaKey("SchemaTest", 1, 0) != SchemaKey("SchemaTest", 2, 0));
    EXPECT_TRUE (SchemaKey("SchemaTest", 1, 0) != SchemaKey("SchemaTest", 1, 1));
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
    
    EXPECT_TRUE (cache->AddSchema(*schema1) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->AddSchema(*schema2) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->AddSchema(*schema3) == ECOBJECTS_STATUS_Success);
    EXPECT_EQ (cache->GetCount(), 3);
    
    ECSchemaPtr fetchedSchema = cache->GetSchema(SchemaKey("BaseSchema1", 2, 0));
    ASSERT_TRUE (fetchedSchema.IsValid());
    EXPECT_TRUE (fetchedSchema->GetSchemaKey() == SchemaKey("BaseSchema1", 2, 0));
    
    cache->Clear();
    EXPECT_EQ (cache->GetCount(), 0);
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
    
    ECSchema::CreateSchema (schema1, "Widget", 5, 1);
    ECSchema::CreateSchema (schema2, "BaseSchema1", 2, 0);
    ECSchema::CreateSchema (schema3, "BaseSchema2", 5, 5);
    
    EXPECT_TRUE (cache->AddSchema(*schema1) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->AddSchema(*schema2) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->AddSchema(*schema3) == ECOBJECTS_STATUS_Success);
    
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseSchema1", 2, 0), SCHEMAMATCHTYPE_Exact) != NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseZchema1", 2, 0), SCHEMAMATCHTYPE_Exact) == NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseSchema1", 3, 0), SCHEMAMATCHTYPE_Exact) == NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseSchema1", 2, 1), SCHEMAMATCHTYPE_Exact) == NULL);
    
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseSchema1", 2, 0), SCHEMAMATCHTYPE_Identical) != NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseZchema1", 2, 0), SCHEMAMATCHTYPE_Identical) == NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseSchema1", 3, 0), SCHEMAMATCHTYPE_Identical) == NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseSchema1", 2, 1), SCHEMAMATCHTYPE_Identical) == NULL);
    
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseSchema1", 2, 0), SCHEMAMATCHTYPE_Latest) != NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseZchema1", 2, 0), SCHEMAMATCHTYPE_Latest) == NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseSchema1", 3, 0), SCHEMAMATCHTYPE_Latest) != NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseSchema1", 2, 1), SCHEMAMATCHTYPE_Latest) != NULL);
    
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseSchema2", 5, 5), SCHEMAMATCHTYPE_LatestCompatible) != NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseZchema2", 5, 5), SCHEMAMATCHTYPE_LatestCompatible) == NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseSchema2", 3, 5), SCHEMAMATCHTYPE_LatestCompatible) == NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey("BaseSchema2", 5, 3), SCHEMAMATCHTYPE_LatestCompatible) != NULL);
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
    
    ECSchema::CreateSchema (schema1, "Widget", 5, 1);
    ECSchema::CreateSchema (schema2, "BaseSchema1", 2, 0);
    ECSchema::CreateSchema (schema3, "BaseSchema2", 5, 5);
    
    EXPECT_TRUE (cache->AddSchema(*schema1) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->AddSchema(*schema2) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->AddSchema(*schema3) == ECOBJECTS_STATUS_Success);
    EXPECT_EQ (cache->GetCount(), 3);
    
    EXPECT_TRUE (cache->DropSchema(SchemaKey("Widget", 5, 1)) == ECOBJECTS_STATUS_Success);
    EXPECT_EQ (cache->GetCount(), 2);
    EXPECT_TRUE (cache->DropSchema(SchemaKey("Widget", 5, 1)) != ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->DropSchema(SchemaKey("BaseSchema2", 5, 3)) != ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->DropSchema(SchemaKey("BaseSchema2", 5, 7)) != ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->DropSchema(SchemaKey("BaseSchema2", 1, 5)) != ECOBJECTS_STATUS_Success);
    EXPECT_EQ (cache->GetCount(), 2);
    EXPECT_TRUE (cache->DropSchema(SchemaKey("BaseSchema2", 5, 5)) == ECOBJECTS_STATUS_Success);
    EXPECT_EQ (cache->GetCount(), 1);
    EXPECT_TRUE (cache->DropSchema(SchemaKey("BaseSchema1", 2, 0)) == ECOBJECTS_STATUS_Success);
    EXPECT_EQ (cache->GetCount(), 0);
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
    
    EXPECT_EQ (ECSchema::ComputeSchemaXmlStringCheckSum(schemaXml, sizeof(schemaXml)), 682119251);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaImmutableTest, SetImmutable)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath( L"Widgets.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, status);
    
    schema->SetImmutable();
    
    ECClassP class1 = NULL;
    ECClassP class2 = schema->GetClassP("ecProject");
    ECRelationshipClassP relationshipClass;
    EXPECT_EQ (schema->CreateClass(class1, "TestClass"), ECOBJECTS_STATUS_SchemaIsImmutable);
    EXPECT_TRUE (class1 == NULL);
    EXPECT_EQ (schema->CopyClass(class1, *class2), ECOBJECTS_STATUS_SchemaIsImmutable);
    EXPECT_EQ (schema->CreateRelationshipClass(relationshipClass, "RelationshipClass"), ECOBJECTS_STATUS_SchemaIsImmutable);
    EXPECT_EQ (schema->SetName("Some new name"), ECOBJECTS_STATUS_SchemaIsImmutable);
    EXPECT_EQ (schema->SetNamespacePrefix("Some new prefix"), ECOBJECTS_STATUS_SchemaIsImmutable);
    EXPECT_EQ (schema->SetDescription("Some new description"), ECOBJECTS_STATUS_SchemaIsImmutable);
    EXPECT_EQ (schema->SetDisplayLabel("Some new label"), ECOBJECTS_STATUS_SchemaIsImmutable);
    EXPECT_EQ (schema->SetVersionMajor(13), ECOBJECTS_STATUS_SchemaIsImmutable);
    EXPECT_EQ (schema->SetVersionMinor(13), ECOBJECTS_STATUS_SchemaIsImmutable);
    }

END_BENTLEY_ECN_TEST_NAMESPACE