/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/NonPublishedSchemaTests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include <comdef.h>
#include "TestFixture.h"

#include <ECObjects\ECInstance.h>
#include <ECObjects\StandaloneECInstance.h>
#include <ECObjects\ECValue.h>
#include <ECObjects\ECSchema.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

using namespace std;

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

TEST_F (SchemaTest, TestCircularReferenceWithLocateSchema)
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

TEST_F (SchemaTest, FindLatestShouldFindSchemaWithLowerMinorVersion)
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
    SchemaKey key(L"MasterSchema", 1, 0);
    testSchema = schemaContext->LocateSchema(key, SCHEMAMATCHTYPE_Latest);
    EXPECT_TRUE(testSchema->IsSupplemented());

    }

END_BENTLEY_ECOBJECT_NAMESPACE
