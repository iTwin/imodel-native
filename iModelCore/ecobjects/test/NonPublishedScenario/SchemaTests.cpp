/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublishedScenario/SchemaTests.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct SchemaTest : ECTestFixture {};

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
        wprintf(L"ECClass=0x%p, name=%s\n", ecClass, name.c_str());
        count++;
        }
    ASSERT_EQ(2, count);

    for (ECClassCP ecClass: container)
        {
        WString name = ecClass->GetName();
        wprintf(L"ECClass=0x%p, name=%s\n", ecClass, name.c_str());
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
        wprintf(L"ECClass=0x%p, name=%s\n", ecClass, name.c_str());
        count++;
        }
    ASSERT_EQ(2, count);

    for (ECClassCP ecClass: container)
        {
        WString name = ecClass->GetName();
        wprintf(L"ECClass=0x%p, name=%s\n", ecClass, name.c_str());
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

END_BENTLEY_ECOBJECT_NAMESPACE