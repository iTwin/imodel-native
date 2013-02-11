/*--------------------------------------------------------------------------------------+
|
|     $Source: test/scenario/SchemaTests.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include <comdef.h>
#include "StopWatch.h"
#include "TestFixture.h"

#include <ECObjects\ECInstance.h>
#include <ECObjects\StandaloneECInstance.h>
#include <ECObjects\ECValue.h>
#include <ECObjects\ECSchema.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

using namespace std;

struct SchemaTest : ECTestFixture {
//IECInstance GetClassInstance(L"CustomAttribute", *schema, *schemaOwner)
//{
//    return NULL;
//}

};
struct SchemaDeserializationTest : ECTestFixture {};
struct SchemaSearchTest          : ECTestFixture {};
struct SchemaComparisonTest      : ECTestFixture {};
struct SchemaCacheTest           : ECTestFixture {};
struct SchemaReferenceTest       : ECTestFixture {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP GetPropertyByName (ECClassCR ecClass, WCharCP name, bool expectExists = true)
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
    EXPECT_STREQ (L"Widgets", schema->GetName().c_str());
    EXPECT_STREQ (L"wid", schema->GetNamespacePrefix().c_str());
    EXPECT_STREQ (L"Widgets Display Label", schema->GetDisplayLabel().c_str());
    EXPECT_TRUE (schema->GetIsDisplayLabelDefined());
    EXPECT_STREQ (L"Widgets Description", schema->GetDescription().c_str());
    EXPECT_EQ (9, schema->GetVersionMajor());
    EXPECT_EQ (6, schema->GetVersionMinor());        
 
#ifdef DEBUG_PRINT
    FOR_EACH (ECClassP pClass, schema->GetClasses())
        {
        wprintf (L"Widgets contains class: '%s' with display label '%s'\n", pClass->GetName().c_str(), pClass->GetDisplayLabel().c_str());
        }
#endif

    ECClassP pClass = schema->GetClassP(L"ClassDoesNotExistInSchema");
    EXPECT_FALSE (pClass);

    pClass = schema->GetClassP(L"ecProject");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ (L"ecProject", pClass->GetName().c_str());    
    EXPECT_STREQ (L"Project", pClass->GetDisplayLabel().c_str());
    EXPECT_TRUE (pClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ (L"Project Class", pClass->GetDescription().c_str());
    EXPECT_FALSE (pClass->GetIsStruct());
    EXPECT_FALSE (pClass->GetIsCustomAttributeClass());
    EXPECT_TRUE (pClass->GetIsDomainClass());
    EXPECT_FALSE (pClass->HasBaseClasses());
    ECPropertyP pProperty = GetPropertyByName (*pClass, L"Name");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ (L"Name", pProperty->GetName().c_str());
    EXPECT_TRUE (pProperty->GetIsPrimitive());
    EXPECT_FALSE (pProperty->GetIsStruct());
    EXPECT_FALSE (pProperty->GetIsArray());
    EXPECT_STREQ (L"string", pProperty->GetTypeName().c_str());
    EXPECT_TRUE (PRIMITIVETYPE_String == pProperty->GetAsPrimitiveProperty()->GetType());
    EXPECT_TRUE (pProperty->GetIsDisplayLabelDefined());
    EXPECT_STREQ (L"Project Name", pProperty->GetDisplayLabel().c_str());
    EXPECT_STREQ (L"", pProperty->GetDescription().c_str());
    EXPECT_EQ (pClass, &pProperty->GetClass());
    EXPECT_FALSE (pProperty->GetIsReadOnly());

    pProperty = GetPropertyByName (*pClass, L"PropertyDoesNotExistInClass", false);
    EXPECT_FALSE (pProperty);

    ECClassP customAttribClass = schema->GetClassP(L"AccessCustomAttributes");
    ASSERT_TRUE (NULL != customAttribClass);
    EXPECT_STREQ (L"AccessCustomAttributes", customAttribClass->GetName().c_str());    
    EXPECT_STREQ (L"AccessCustomAttributes", customAttribClass->GetDisplayLabel().c_str());
    EXPECT_FALSE (customAttribClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ (L"", customAttribClass->GetDescription().c_str());
    EXPECT_FALSE (customAttribClass->GetIsStruct());
    EXPECT_TRUE (customAttribClass->GetIsCustomAttributeClass());
    EXPECT_FALSE (customAttribClass->GetIsDomainClass());
    EXPECT_FALSE (customAttribClass->HasBaseClasses());

    pClass = schema->GetClassP(L"Struct1");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ (L"Struct1", pClass->GetName().c_str());    
    EXPECT_STREQ (L"Struct1", pClass->GetDisplayLabel().c_str());
    EXPECT_FALSE (pClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ (L"", pClass->GetDescription().c_str());
    EXPECT_TRUE (pClass->GetIsStruct());
    EXPECT_FALSE (pClass->GetIsCustomAttributeClass());
    EXPECT_FALSE (pClass->GetIsDomainClass());
    EXPECT_FALSE (pClass->HasBaseClasses());

    pClass = schema->GetClassP(L"Struct2");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_STREQ (L"Struct2", pClass->GetName().c_str());    
    EXPECT_STREQ (L"Struct2", pClass->GetDisplayLabel().c_str());
    EXPECT_FALSE (pClass->GetIsDisplayLabelDefined());
    EXPECT_STREQ (L"", pClass->GetDescription().c_str());
    EXPECT_TRUE (pClass->GetIsStruct());
    EXPECT_FALSE (pClass->GetIsCustomAttributeClass());
    EXPECT_TRUE (pClass->GetIsDomainClass());
    EXPECT_FALSE (pClass->HasBaseClasses());
    pProperty = GetPropertyByName (*pClass, L"NestedArray");
    EXPECT_TRUE (NULL != pProperty);
    EXPECT_STREQ (L"NestedArray", pProperty->GetName().c_str());
    EXPECT_FALSE (pProperty->GetIsPrimitive());
    EXPECT_FALSE (pProperty->GetIsStruct());
    EXPECT_TRUE (pProperty->GetIsArray());
    EXPECT_STREQ (L"Struct1", pProperty->GetTypeName().c_str());
    ArrayECPropertyP arrayProperty = pProperty->GetAsArrayPropertyP();
    EXPECT_TRUE (ARRAYKIND_Struct == arrayProperty->GetKind());
    EXPECT_EQ (schema->GetClassP(L"Struct1"), arrayProperty->GetStructElementType());
    EXPECT_EQ (0, arrayProperty->GetMinOccurs());
    EXPECT_EQ (UINT_MAX, arrayProperty->GetMaxOccurs());    
    EXPECT_FALSE (pProperty->GetIsDisplayLabelDefined());
    EXPECT_STREQ (L"NestedArray", pProperty->GetDisplayLabel().c_str());
    EXPECT_STREQ (L"", pProperty->GetDescription().c_str());
    EXPECT_EQ (pClass, &pProperty->GetClass());
    EXPECT_FALSE (pProperty->GetIsReadOnly());

    pClass = schema->GetClassP(L"TestClass");
    ASSERT_TRUE (NULL != pClass);
    EXPECT_TRUE (pClass->HasBaseClasses());
    pProperty = GetPropertyByName (*pClass, L"EmbeddedStruct");
    ASSERT_TRUE (NULL != pProperty);
    EXPECT_STREQ (L"EmbeddedStruct", pProperty->GetName().c_str());
    EXPECT_FALSE (pProperty->GetIsPrimitive());
    EXPECT_TRUE (pProperty->GetIsStruct());
    EXPECT_FALSE (pProperty->GetIsArray());
    EXPECT_STREQ (L"Struct1", pProperty->GetTypeName().c_str());
    StructECPropertyP structProperty = pProperty->GetAsStructPropertyP();    
    EXPECT_EQ (schema->GetClassP(L"Struct1"), &(structProperty->GetType()));
    EXPECT_FALSE (pProperty->GetIsDisplayLabelDefined());
    EXPECT_STREQ (L"EmbeddedStruct", pProperty->GetDisplayLabel().c_str());
    EXPECT_STREQ (L"", pProperty->GetDescription().c_str());
    EXPECT_EQ (pClass, &pProperty->GetClass());
    EXPECT_FALSE (pProperty->GetIsReadOnly());
    
    IECInstancePtr instance = pClass->GetCustomAttribute(*customAttribClass);
    EXPECT_TRUE(instance.IsValid());

    ECValue ecValue;
    EXPECT_EQ (SUCCESS, instance->GetValue (ecValue, L"AccessLevel"));
    EXPECT_EQ (4, ecValue.GetInteger());

    EXPECT_EQ (SUCCESS, instance->GetValue (ecValue, L"Writeable"));
    EXPECT_FALSE (ecValue.GetBoolean());
   
#ifdef DEBUG_PRINT
    FOR_EACH (ECPropertyP pProperty, pClass->GetProperties())
        {
        wprintf (L"TestClass contains property: %s of type %s\n", pProperty->GetName().c_str(), pProperty->GetTypeName().c_str());
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
 
    ECSchema::CreateSchema(schema, L"TestSchema", 5, 5);
    ASSERT_TRUE(schema!=NULL);
 
    //Create Domain Class
    schema->CreateClass(domainClass,L"DomainClass");
    ASSERT_TRUE(domainClass!=NULL);
    domainClass->SetIsDomainClass(true);
 
    //Create Derived Class
    schema->CreateClass(derivedClass,L"DerivedClass");
    ASSERT_TRUE(derivedClass!=NULL);
 
    //Create Struct
    schema->CreateClass(structClass,L"StructClass");
    ASSERT_TRUE(structClass!=NULL);
    structClass->SetIsStruct(true);
 
    //Add Property of Array type to structClass
    ArrayECPropertyP MyArrayProp;
    structClass->CreateArrayProperty(MyArrayProp,L"ArrayProperty");
    ASSERT_TRUE(MyArrayProp!=NULL);
 
    //Create customAttributeClass
    schema->CreateClass(customAttributeClass,L"CustomAttribute");
    ASSERT_TRUE(customAttributeClass!=NULL);
    customAttributeClass->SetIsCustomAttributeClass(true);
 
    //Add Property Of Struct type to custom attribute
    StructECPropertyP PropertyOfCustomAttribute;
    customAttributeClass->CreateStructProperty(PropertyOfCustomAttribute, L"PropertyOfCustomAttribute",*structClass);
    ASSERT_TRUE(PropertyOfCustomAttribute!=NULL);
}
TEST_F(SchemaTest, ShouldBeAbleToIterateOverECClassContainer)
    {
    ECSchemaPtr schema;
    ECClassP foo;
    ECClassP bar;
    
    ECSchema::CreateSchema(schema, L"TestSchema", 5, 5);
    schema->CreateClass(foo, L"foo");
    schema->CreateClass(bar, L"bar");

    ClassMap classMap;
    classMap.insert (bpair<WCharCP, ECClassP> (foo->GetName().c_str(), foo));
    classMap.insert (bpair<WCharCP, ECClassP> (bar->GetName().c_str(), bar));
    
    int count = 0;
    ECClassContainer container(classMap);
    for (ECClassContainer::const_iterator cit = container.begin(); cit != container.end(); ++cit)
        {
        ECClassCP ecClass = *cit;
        WString name = ecClass->GetName();
        wprintf(L"ECClass=0x%x, name=%s\n", ecClass, name.c_str());
        count++;
        }
    ASSERT_EQ(2, count);

    FOR_EACH (ECClassCP ecClass, container)
        {
        WString name = ecClass->GetName();
        wprintf(L"ECClass=0x%x, name=%s\n", ecClass, name.c_str());
        count++;
        }
    ASSERT_EQ(4, count);
    }


TEST_F(SchemaTest, TestGetClassCount)
    {
   
	ECSchemaPtr schema;
    ECClassP foo;
    ECClassP bar;
    
    ECSchema::CreateSchema(schema, L"TestSchema", 5, 5);
    schema->CreateClass(foo, L"foo");
    schema->CreateClass(bar, L"bar");

    ClassMap classMap;
    classMap.insert (bpair<WCharCP, ECClassP> (foo->GetName().c_str(), foo));
    classMap.insert (bpair<WCharCP, ECClassP> (bar->GetName().c_str(), bar));
    
    int count = 0;
    ECClassContainer container(classMap);
    for (ECClassContainer::const_iterator cit = container.begin(); cit != container.end(); ++cit)
        {
        ECClassCP ecClass = *cit;
        WString name = ecClass->GetName();
        wprintf(L"ECClass=0x%x, name=%s\n", ecClass, name.c_str());
        count++;
        }
    ASSERT_EQ(2, count);

    FOR_EACH (ECClassCP ecClass, container)
        {
        WString name = ecClass->GetName();
        wprintf(L"ECClass=0x%x, name=%s\n", ecClass, name.c_str());
        count++;
        }
    ASSERT_EQ(4, count);

	ASSERT_EQ(2,schema->GetClassCount());

    }

TEST_F (SchemaTest, DISABLED_TestCircularReference)
{
	ECSchemaPtr testSchema;
	ECSchemaReadContextPtr   schemaContext;
	SearchPathSchemaFileLocaterPtr schemaLocater;
	bvector<WString> searchPaths;
	searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
	schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
	schemaContext = ECSchemaReadContext::CreateContext();
	schemaContext->AddSchemaLocater (*schemaLocater);
	SchemaKey key(L"CircleSchema", 01, 00);
	testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
	EXPECT_FALSE(testSchema.IsValid());
}



TEST_F (SchemaTest, TestsLatestCompatible)
{
	ECSchemaPtr testSchema;
	ECSchemaReadContextPtr   schemaContext;
	SearchPathSchemaFileLocaterPtr schemaLocater;
	bvector<WString> searchPaths;
	searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
	schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
	schemaContext = ECSchemaReadContext::CreateContext();
	schemaContext->AddSchemaLocater (*schemaLocater);
	SchemaKey key(L"Widgets", 01, 00);
	testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_LatestCompatible);
	EXPECT_TRUE(testSchema.IsValid());
	EXPECT_TRUE(testSchema->GetVersionMajor()==9);
	EXPECT_TRUE(testSchema->GetVersionMinor()==6);
}

TEST_F (SchemaTest, TestsLatest)
{
	ECSchemaPtr testSchema;
	ECSchemaReadContextPtr   schemaContext;
	SearchPathSchemaFileLocaterPtr schemaLocater;
	bvector<WString> searchPaths;
	searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
	schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
	schemaContext = ECSchemaReadContext::CreateContext();
	schemaContext->AddSchemaLocater (*schemaLocater);
	SchemaKey key(L"Widgets", 9, 7);
	testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
	EXPECT_TRUE(testSchema.IsValid());
	EXPECT_TRUE(testSchema->GetVersionMajor()==9);
	EXPECT_TRUE(testSchema->GetVersionMinor()==6);
}
TEST_F (SchemaTest, GetBaseClassPropertyWhenSchemaHaveDuplicatePrefixes)
{
	ECSchemaPtr testSchema;
	ECSchemaReadContextPtr   schemaContext;
	SearchPathSchemaFileLocaterPtr schemaLocater;
	bvector<WString> searchPaths;
	searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
	schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
	schemaContext = ECSchemaReadContext::CreateContext();
	schemaContext->AddSchemaLocater (*schemaLocater);
	SchemaKey key(L"DuplicatePrefixes", 01, 00);
	testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
	EXPECT_TRUE(testSchema.IsValid());
	ECClassCP CircleClass = testSchema->GetClassCP (L"Circle");
	EXPECT_TRUE(CircleClass!=NULL)<<"Cannot Load Ellipse Class";

	IECInstancePtr CircleClassInstance = CircleClass->GetDefaultStandaloneEnabler()->CreateInstance();
	ECValue v;
    v.SetString (L"test");
	CircleClassInstance->SetValue (L"Name", v);

}

TEST_F (SchemaTest, GetBaseClassProperty)
{
	ECSchemaPtr testSchema;
	ECSchemaReadContextPtr   schemaContext;
	SearchPathSchemaFileLocaterPtr schemaLocater;
	bvector<WString> searchPaths;
	searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
	schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
	schemaContext = ECSchemaReadContext::CreateContext();
	schemaContext->AddSchemaLocater (*schemaLocater);
	SchemaKey key(L"testschema", 01, 00);
	testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
	EXPECT_TRUE(testSchema.IsValid());
	ECClassCP WheelsChildClass = testSchema->GetClassCP (L"WheelsChild");
	EXPECT_TRUE(WheelsChildClass!=NULL)<<"Cannot Load WheelsChild Class";

	IECInstancePtr WheelsChildInstance = WheelsChildClass->GetDefaultStandaloneEnabler()->CreateInstance();
	ECValue v;
    v.SetString (L"test");
	WheelsChildInstance->SetValue (L"Name", v);

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

    EXPECT_TRUE (schema->FindSchema(SchemaKey(L"SchemaThatReferences", 1, 0), SCHEMAMATCHTYPE_Exact) != NULL);
    EXPECT_TRUE (schema->FindSchema(SchemaKey(L"SchemaThatReferencez", 1, 0), SCHEMAMATCHTYPE_Exact) == NULL);
    EXPECT_TRUE (schema->FindSchema(SchemaKey(L"SchemaThatReferences", 2, 0), SCHEMAMATCHTYPE_Exact) == NULL);
    EXPECT_TRUE (schema->FindSchema(SchemaKey(L"SchemaThatReferences", 1, 1), SCHEMAMATCHTYPE_Exact) == NULL);
    
    EXPECT_TRUE (schema->FindSchema(SchemaKey(L"BaseSchema", 1, 0), SCHEMAMATCHTYPE_Exact) != NULL);
    EXPECT_TRUE (schema->FindSchemaP(SchemaKey(L"SchemaThatReferences", 1, 0), SCHEMAMATCHTYPE_Exact) != NULL);
    EXPECT_TRUE (schema->FindSchemaP(SchemaKey(L"a", 123, 456), SCHEMAMATCHTYPE_Exact) == NULL);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaReferenceTest, FindClassInReferenceList)
    {
    ECSchemaPtr schema, refSchema;
    ECSchema::CreateSchema (schema, L"TestSchema", 5, 5);
    ECSchema::CreateSchema (refSchema, L"RefSchema", 5, 5);
    
    ECRelationshipClassP relClass;
    ECClassP targetClass, sourceClass;
    schema->CreateRelationshipClass (relClass, L"RElationshipClass");
    schema->CreateClass (targetClass, L"Target");
    refSchema->CreateClass (sourceClass, L"Source");
    
    EXPECT_EQ (ECOBJECTS_STATUS_Success, schema->AddReferencedSchema (*refSchema));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, relClass->GetTarget().AddClass(*targetClass));
    EXPECT_EQ (ECOBJECTS_STATUS_Success, relClass->GetSource().AddClass(*sourceClass));
    
    ECSchemaReferenceListCR refList = schema->GetReferencedSchemas();
    
    EXPECT_TRUE (refList.FindClassP(SchemaNameClassNamePair(L"RefSchema", L"Source")) != NULL);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaComparisonTest, VerifyMatchesOperator)
    {
    EXPECT_TRUE (SchemaKey(L"SchemaTest", 1, 0) == SchemaKey(L"SchemaTest", 1, 0));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0) == SchemaKey(L"SchemaNotTest", 1, 0));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0) == SchemaKey(L"SchemaTest", 2, 0));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0) == SchemaKey(L"SchemaTest", 1, 1));
    
    EXPECT_TRUE (SchemaKey(L"SchemaTest", 1, 0).Matches(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_Exact));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).Matches(SchemaKey(L"SchemaNotTest", 1, 0), SCHEMAMATCHTYPE_Exact));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).Matches(SchemaKey(L"SchemaTest", 2, 0), SCHEMAMATCHTYPE_Exact));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).Matches(SchemaKey(L"SchemaTest", 1, 1), SCHEMAMATCHTYPE_Exact));
    
    EXPECT_TRUE (SchemaKey(L"SchemaTest", 1, 0).Matches(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_Identical));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).Matches(SchemaKey(L"SchemaNotTest", 1, 0), SCHEMAMATCHTYPE_Identical));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).Matches(SchemaKey(L"SchemaTest", 2, 0), SCHEMAMATCHTYPE_Identical));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).Matches(SchemaKey(L"SchemaTest", 1, 1), SCHEMAMATCHTYPE_Identical));
    
    EXPECT_TRUE (SchemaKey(L"SchemaTest", 1, 0).Matches(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_Latest));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).Matches(SchemaKey(L"SchemaNotTest", 1, 0), SCHEMAMATCHTYPE_Latest));
    EXPECT_TRUE (SchemaKey(L"SchemaTest", 1, 0).Matches(SchemaKey(L"SchemaTest", 2, 0), SCHEMAMATCHTYPE_Latest));
    EXPECT_TRUE (SchemaKey(L"SchemaTest", 1, 1).Matches(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_Latest));
    
    EXPECT_TRUE (SchemaKey(L"SchemaTest", 1, 0).Matches(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_LatestCompatible));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).Matches(SchemaKey(L"SchemaNotTest", 1, 0), SCHEMAMATCHTYPE_LatestCompatible));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).Matches(SchemaKey(L"SchemaTest", 2, 0), SCHEMAMATCHTYPE_LatestCompatible));
    EXPECT_TRUE (SchemaKey(L"SchemaTest", 1, 1).Matches(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_LatestCompatible));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).Matches(SchemaKey(L"SchemaTest", 1, 1), SCHEMAMATCHTYPE_LatestCompatible));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaComparisonTest, VerifyLessThanOperator)
    {
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0) < SchemaKey(L"SchemaTest", 1, 0));
    EXPECT_TRUE (SchemaKey(L"SchemaTesa", 1, 0) < SchemaKey(L"SchemaTest", 1, 0));
    EXPECT_TRUE (SchemaKey(L"SchemaTest", 1, 0) < SchemaKey(L"SchemaTest", 2, 0));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 1) < SchemaKey(L"SchemaTest", 1, 0));
    
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).LessThan(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_Exact));
    EXPECT_TRUE (SchemaKey(L"SchemaTesa", 1, 0).LessThan(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_Exact));
    EXPECT_TRUE (SchemaKey(L"SchemaTest", 1, 0).LessThan(SchemaKey(L"SchemaTest", 2, 0), SCHEMAMATCHTYPE_Exact));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 1).LessThan(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_Exact));
    
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).LessThan(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_Identical));
    EXPECT_TRUE (SchemaKey(L"SchemaTesa", 1, 0).LessThan(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_Identical));
    EXPECT_TRUE (SchemaKey(L"SchemaTest", 1, 0).LessThan(SchemaKey(L"SchemaTest", 2, 0), SCHEMAMATCHTYPE_Identical));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 1).LessThan(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_Identical));
    
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).LessThan(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_Latest));
    EXPECT_TRUE (SchemaKey(L"SchemaTesa", 1, 0).LessThan(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_Latest));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).LessThan(SchemaKey(L"SchemaTest", 2, 0), SCHEMAMATCHTYPE_Latest));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).LessThan(SchemaKey(L"SchemaTest", 1, 9), SCHEMAMATCHTYPE_Latest));
    
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).LessThan(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_LatestCompatible));
    EXPECT_TRUE (SchemaKey(L"SchemaTesa", 1, 0).LessThan(SchemaKey(L"SchemaTest", 1, 0), SCHEMAMATCHTYPE_LatestCompatible));
    EXPECT_TRUE (SchemaKey(L"SchemaTest", 1, 0).LessThan(SchemaKey(L"SchemaTest", 2, 0), SCHEMAMATCHTYPE_LatestCompatible));
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0).LessThan(SchemaKey(L"SchemaTest", 1, 9), SCHEMAMATCHTYPE_LatestCompatible));
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaComparisonTest, VerifyNotMatchesOperator)
    {
    EXPECT_FALSE (SchemaKey(L"SchemaTest", 1, 0) != SchemaKey(L"SchemaTest", 1, 0));
    EXPECT_TRUE (SchemaKey(L"SchemaTest", 1, 0) != SchemaKey(L"SchemaNotTest", 1, 0));
    EXPECT_TRUE (SchemaKey(L"SchemaTest", 1, 0) != SchemaKey(L"SchemaTest", 2, 0));
    EXPECT_TRUE (SchemaKey(L"SchemaTest", 1, 0) != SchemaKey(L"SchemaTest", 1, 1));
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
    
    ECSchema::CreateSchema(schema1, L"Widget", 5, 1);
    ECSchema::CreateSchema(schema2, L"BaseSchema1", 2, 0);
    ECSchema::CreateSchema(schema3, L"BaseSchema2", 5, 5);
    
    EXPECT_TRUE (cache->AddSchema(*schema1) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->AddSchema(*schema2) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->AddSchema(*schema3) == ECOBJECTS_STATUS_Success);
    EXPECT_EQ (cache->GetCount(), 3);
    
    ECSchemaPtr fetchedSchema = cache->GetSchema(SchemaKey(L"BaseSchema1", 2, 0));
    ASSERT_TRUE (fetchedSchema != NULL);
    EXPECT_TRUE (fetchedSchema->GetSchemaKey() == SchemaKey(L"BaseSchema1", 2, 0));
    
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
    
    ECSchema::CreateSchema (schema1, L"Widget", 5, 1);
    ECSchema::CreateSchema (schema2, L"BaseSchema1", 2, 0);
    ECSchema::CreateSchema (schema3, L"BaseSchema2", 5, 5);
    
    EXPECT_TRUE (cache->AddSchema(*schema1) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->AddSchema(*schema2) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->AddSchema(*schema3) == ECOBJECTS_STATUS_Success);
    
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseSchema1", 2, 0), SCHEMAMATCHTYPE_Exact) != NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseZchema1", 2, 0), SCHEMAMATCHTYPE_Exact) == NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseSchema1", 3, 0), SCHEMAMATCHTYPE_Exact) == NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseSchema1", 2, 1), SCHEMAMATCHTYPE_Exact) == NULL);
    
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseSchema1", 2, 0), SCHEMAMATCHTYPE_Identical) != NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseZchema1", 2, 0), SCHEMAMATCHTYPE_Identical) == NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseSchema1", 3, 0), SCHEMAMATCHTYPE_Identical) == NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseSchema1", 2, 1), SCHEMAMATCHTYPE_Identical) == NULL);
    
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseSchema1", 2, 0), SCHEMAMATCHTYPE_Latest) != NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseZchema1", 2, 0), SCHEMAMATCHTYPE_Latest) == NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseSchema1", 3, 0), SCHEMAMATCHTYPE_Latest) != NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseSchema1", 2, 1), SCHEMAMATCHTYPE_Latest) != NULL);
    
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseSchema2", 5, 5), SCHEMAMATCHTYPE_LatestCompatible) != NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseZchema2", 5, 5), SCHEMAMATCHTYPE_LatestCompatible) == NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseSchema2", 3, 5), SCHEMAMATCHTYPE_LatestCompatible) == NULL);
    EXPECT_TRUE (cache->GetSchema(SchemaKey(L"BaseSchema2", 5, 3), SCHEMAMATCHTYPE_LatestCompatible) != NULL);
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
    
    ECSchema::CreateSchema (schema1, L"Widget", 5, 1);
    ECSchema::CreateSchema (schema2, L"BaseSchema1", 2, 0);
    ECSchema::CreateSchema (schema3, L"BaseSchema2", 5, 5);
    
    EXPECT_TRUE (cache->AddSchema(*schema1) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->AddSchema(*schema2) == ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->AddSchema(*schema3) == ECOBJECTS_STATUS_Success);
    EXPECT_EQ (cache->GetCount(), 3);
    
    EXPECT_TRUE (cache->DropSchema(SchemaKey(L"Widget", 5, 1)) == ECOBJECTS_STATUS_Success);
    EXPECT_EQ (cache->GetCount(), 2);
    EXPECT_TRUE (cache->DropSchema(SchemaKey(L"Widget", 5, 1)) != ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->DropSchema(SchemaKey(L"BaseSchema2", 5, 3)) != ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->DropSchema(SchemaKey(L"BaseSchema2", 5, 7)) != ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (cache->DropSchema(SchemaKey(L"BaseSchema2", 1, 5)) != ECOBJECTS_STATUS_Success);
    EXPECT_EQ (cache->GetCount(), 2);
    EXPECT_TRUE (cache->DropSchema(SchemaKey(L"BaseSchema2", 5, 5)) == ECOBJECTS_STATUS_Success);
    EXPECT_EQ (cache->GetCount(), 1);
    EXPECT_TRUE (cache->DropSchema(SchemaKey(L"BaseSchema1", 2, 0)) == ECOBJECTS_STATUS_Success);
    EXPECT_EQ (cache->GetCount(), 0);
    }

END_BENTLEY_ECOBJECT_NAMESPACE