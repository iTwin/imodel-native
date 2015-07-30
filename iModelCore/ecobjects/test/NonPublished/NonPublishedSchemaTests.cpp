/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/NonPublishedSchemaTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <ECObjects/ECInstance.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/ECValue.h>
#include <ECObjects/ECSchema.h>

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

using namespace std;
using namespace BentleyApi::ECN;

struct NonPublishedSchemaTest : ECTestFixture {};
TEST_F(NonPublishedSchemaTest, ShouldBeAbleToIterateOverECClassContainer)
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
        ASSERT_TRUE(ecClass != NULL);
#if PRINT_DUMP
        Utf8String name = ecClass->GetName();
        printf("ECClass=0x%x, name=%s\n", (uintptr_t)ecClass, name.c_str());
#endif
        count++;
        }
    ASSERT_EQ(2, count);

    for (ECClassCP ecClass: container)
        {
        ASSERT_TRUE(ecClass != NULL);
#if PRINT_DUMP
        Utf8String name = ecClass->GetName();
        printf("ECClass=0x%x, name=%s\n", (uintptr_t)ecClass, name.c_str());
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
    SchemaKey key("CircleSchema", 01, 00);
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
    SchemaKey key("Widgets", 9, 7);
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE(testSchema.IsValid());
    EXPECT_TRUE(testSchema->GetVersionMajor()==9);
    EXPECT_TRUE(testSchema->GetVersionMinor()==6);
    }

TEST(SupplementalDeserializationTests, VerifyDeserializedSchemaIsSupplemented)
    {
    ECSchemaPtr testSchema;
    ECSchemaReadContextPtr   schemaContext;
    SearchPathSchemaFileLocaterPtr schemaLocater;
    bvector<WString> searchPaths;
    searchPaths.push_back (ECTestFixture::GetTestDataPath(L""));
    schemaLocater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths);
    schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater (*schemaLocater);
    SchemaKey key("MasterSchema", 1, 0);
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE(testSchema->IsSupplemented());

    }

END_BENTLEY_ECOBJECT_NAMESPACE