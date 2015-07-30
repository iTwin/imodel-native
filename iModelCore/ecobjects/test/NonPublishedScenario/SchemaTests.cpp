/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublishedScenario/SchemaTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct NonPublishedScenarioSchemaTest : ECTestFixture {};

TEST_F(NonPublishedScenarioSchemaTest, ShouldBeAbleToIterateOverECClassContainer)
    {
    ECSchemaPtr schema;
    ECClassP foo;
    ECClassP bar;
    
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateClass(foo, "foo");
    schema->CreateClass(bar, "bar");

    ClassMap classMap;
    classMap.insert (bpair<Utf8CP, ECClassP> (foo->GetName().c_str(), foo));
    classMap.insert (bpair<Utf8CP, ECClassP> (bar->GetName().c_str(), bar));
    
    int count = 0;
    ECClassContainer container(classMap);
    for (ECClassContainer::const_iterator cit = container.begin(); cit != container.end(); ++cit)
        {
        ECClassCP ecClass = *cit;
        Utf8String name = ecClass->GetName();
        printf("ECClass=0x%x, name=%s\n", (uintptr_t)ecClass, name.c_str());
        count++;
        }
    ASSERT_EQ(2, count);

    for (ECClassCP ecClass: container)
        {
        Utf8String name = ecClass->GetName();
        printf("ECClass=0x%x, name=%s\n", (uintptr_t)ecClass, name.c_str());
        count++;
        }
    ASSERT_EQ(4, count);
    }

TEST_F(NonPublishedScenarioSchemaTest, TestGetClassCount)
    {
   
    ECSchemaPtr schema;
    ECClassP foo;
    ECClassP bar;
    
    ECSchema::CreateSchema(schema, "TestSchema", 5, 5);
    schema->CreateClass(foo, "foo");
    schema->CreateClass(bar, "bar");

    ClassMap classMap;
    classMap.insert (bpair<Utf8CP, ECClassP> (foo->GetName().c_str(), foo));
    classMap.insert (bpair<Utf8CP, ECClassP> (bar->GetName().c_str(), bar));
    
    int count = 0;
    ECClassContainer container(classMap);
    for (ECClassContainer::const_iterator cit = container.begin(); cit != container.end(); ++cit)
        {
        ECClassCP ecClass = *cit;
        Utf8String name = ecClass->GetName();
        printf("ECClass=0x%x, name=%s\n", (uintptr_t)ecClass, name.c_str());
        count++;
        }
    ASSERT_EQ(2, count);

    for (ECClassCP ecClass: container)
        {
        Utf8String name = ecClass->GetName();
        printf("ECClass=0x%x, name=%s\n", (uintptr_t)ecClass, name.c_str());
        count++;
        }
    ASSERT_EQ(4, count);

    ASSERT_EQ(2,schema->GetClassCount());
    }

TEST_F (NonPublishedScenarioSchemaTest, DISABLED_TestCircularReference)
{
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater (*schemaLocater);
    SchemaKey key("CircleSchema", 01, 00);
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
    EXPECT_FALSE(testSchema.IsValid());
}

TEST_F (NonPublishedScenarioSchemaTest, TestsLatestCompatible)
{
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater (*schemaLocater);
    SchemaKey key("Widgets", 01, 00);
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_LatestCompatible);
    EXPECT_TRUE(testSchema.IsValid());
    EXPECT_TRUE(testSchema->GetVersionMajor()==9);
    EXPECT_TRUE(testSchema->GetVersionMinor()==6);
}

TEST_F (NonPublishedScenarioSchemaTest, TestsLatest)
{
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater (*schemaLocater);
    SchemaKey key("Widgets", 9, 7);
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE(testSchema.IsValid());
    EXPECT_TRUE(testSchema->GetVersionMajor()==9);
    EXPECT_TRUE(testSchema->GetVersionMinor()==6);
}

TEST_F (NonPublishedScenarioSchemaTest, GetBaseClassPropertyWhenSchemaHaveDuplicatePrefixes)
{
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater (*schemaLocater);
    SchemaKey key("DuplicatePrefixes", 01, 00);
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE(testSchema.IsValid());
    ECClassCP CircleClass = testSchema->GetClassCP ("Circle");
    EXPECT_TRUE(CircleClass!=NULL)<<"Cannot Load Ellipse Class";

    IECInstancePtr CircleClassInstance = CircleClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue v;
    v.SetUtf8CP ("test");
    CircleClassInstance->SetValue ("Name", v);
}

TEST_F (NonPublishedScenarioSchemaTest, GetBaseClassProperty)
{
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater (*schemaLocater);
    SchemaKey key("testschema", 01, 00);
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE(testSchema.IsValid());
    ECClassCP WheelsChildClass = testSchema->GetClassCP ("WheelsChild");
    EXPECT_TRUE(WheelsChildClass!=NULL)<<"Cannot Load WheelsChild Class";

    IECInstancePtr WheelsChildInstance = WheelsChildClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue v;
    v.SetUtf8CP ("test");
    WheelsChildInstance->SetValue ("Name", v);
}

END_BENTLEY_ECN_TEST_NAMESPACE