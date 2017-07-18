/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/NonPublishedSchemaTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <ECObjects/ECInstance.h>
#include <ECObjects/StandaloneECInstance.h>
#include <ECObjects/ECValue.h>
#include <ECObjects/ECSchema.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct NonPublishedSchemaTest : ECTestFixture {};

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
    testSchema = schemaContext->LocateSchema(key, SchemaMatchType::Latest);
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
    testSchema = schemaContext->LocateSchema(key, SchemaMatchType::Latest);
    EXPECT_TRUE(testSchema.IsValid());
    EXPECT_TRUE(testSchema->GetVersionRead()==9);
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
    testSchema = schemaContext->LocateSchema(key, SchemaMatchType::Latest);
    EXPECT_TRUE(testSchema->IsSupplemented());

    }

END_BENTLEY_ECOBJECT_NAMESPACE