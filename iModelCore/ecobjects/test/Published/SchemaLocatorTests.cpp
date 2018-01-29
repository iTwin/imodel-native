/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SchemaLocatorTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaLocatorTests : ECTestFixture {};

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    12/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaLocatorTests, SchemaReadContext_WithoutVersionExtension)
    {
    ECSchemaReadContextPtr readWithoutVersionContext = ECSchemaReadContext::CreateContext(false, true);
    readWithoutVersionContext->AddSchemaPath(ECTestFixture::GetTestDataPath(L"").c_str());

    ECSchemaReadContextPtr readWithVersionContext = ECSchemaReadContext::CreateContext(false, false);
    readWithVersionContext->AddSchemaPath(ECTestFixture::GetTestDataPath(L"").c_str());

    SchemaKey key ("SchemaWithoutVersion", 1, 0, 0);

    ECSchemaPtr schema = readWithoutVersionContext->LocateSchema(key, SchemaMatchType::Exact);
    ASSERT_TRUE(schema.IsValid());

    ECSchemaPtr schema2 = readWithVersionContext->LocateSchema(key, SchemaMatchType::Exact);
    ASSERT_FALSE(schema2.IsValid());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Caleb.Shafer                    12/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaLocatorTests, SearchSchemaFileLocater_WithoutVersionExtension)
    {
    bvector<WString> searchPaths;
    searchPaths.push_back(ECTestFixture::GetTestDataPath(L"\\").c_str());

    SchemaKey key("SchemaWithoutVersion", 1, 0, 0);

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    auto locater = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths, true);
    ECSchemaPtr schema = locater->LocateSchema(key, SchemaMatchType::Identical, *context);
    ASSERT_TRUE(schema.IsValid());

    ECSchemaReadContextPtr context2 = ECSchemaReadContext::CreateContext();
    auto locater2 = SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(searchPaths, false);
    ECSchemaPtr schema2 = locater2->LocateSchema(key, SchemaMatchType::Identical, *context2);
    ASSERT_FALSE(schema2.IsValid());
    }

END_BENTLEY_ECN_TEST_NAMESPACE
