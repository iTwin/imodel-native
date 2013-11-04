/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/NonPublishedSchemaTests.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"


#include <ECObjects/ECInstance.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/ECValue.h>
#include <ECObjects/ECSchema.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

using namespace std;

struct NonPublishedSchemaTest : ECTestFixture {};
TEST_F(NonPublishedSchemaTest, ShouldBeAbleToIterateOverECClassContainer)
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
        ASSERT_TRUE(ecClass != NULL);
#if PRINT_DUMP
        WString name = ecClass->GetName();
        wprintf(L"ECClass=0x%x, name=%s\n", (uintptr_t)ecClass, name.c_str());
#endif
        count++;
        }
    ASSERT_EQ(2, count);

    FOR_EACH (ECClassCP ecClass, container)
        {
        ASSERT_TRUE(ecClass != NULL);
#if PRINT_DUMP
        WString name = ecClass->GetName();
        wprintf(L"ECClass=0x%x, name=%s\n", (uintptr_t)ecClass, name.c_str());
#endif
        count++;
        }
    ASSERT_EQ(4, count);
    }

TEST_F (NonPublishedSchemaTest, TestCircularReferenceWithLocateSchema)
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
    DISABLE_ASSERTS
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
    EXPECT_FALSE(testSchema.IsValid());
    }

TEST_F (NonPublishedSchemaTest, FindLatestShouldFindSchemaWithLowerMinorVersion)
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

END_BENTLEY_ECOBJECT_NAMESPACE