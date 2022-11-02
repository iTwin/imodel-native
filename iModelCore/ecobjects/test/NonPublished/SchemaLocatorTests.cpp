/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaLocatorTests : ECTestFixture {};

struct TestSchemaLocater1 : IECSchemaLocater, NonCopyableClass
    {
protected:
    ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) override
        {
        ECSchemaPtr schema;
        ECSchema::CreateSchema(schema, "TestSchema1", "ts1", 1, 0, 0);
        return schema;
        }
    };

struct TestSchemaLocater2 : IECSchemaLocater, NonCopyableClass
    {
protected:
    ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) override
        {
        ECSchemaPtr schema;
        ECSchema::CreateSchema(schema, "TestSchema2", "ts2", 2, 0, 0);
        return schema;
        }
    };

//---------------------------------------------------------------------------------------
//@bsimethod
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
//@bsimethod
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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaLocatorTests, LocatingSchemaContext_WithLocaterAddedFirst)
    {
    auto const context = ECSchemaReadContext::CreateContext();
    TestSchemaLocater1 locator1;
    TestSchemaLocater2 locator2;
    SchemaKey key;

    context->AddSchemaLocater(locator1);
    context->AddSchemaLocater(locator2);

    auto const schema = context->LocateSchema(key, SchemaMatchType::Latest);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_STREQ("TestSchema1", schema->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaLocatorTests, LocatingSchemaContext_WithPrioritizedLocater)
    {
    auto const context = ECSchemaReadContext::CreateContext();
    TestSchemaLocater1 locator1;
    TestSchemaLocater2 locator2;
    SchemaKey key;

    context->AddSchemaLocater(locator1);
    context->AddFirstSchemaLocater(locator2);

    auto const schema = context->LocateSchema(key, SchemaMatchType::Latest);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_STREQ("TestSchema2", schema->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaLocatorTests, LocatingSchemaContext_CacheLocaterIsBeforePrioritizedLocater)
    {
    auto const context = ECSchemaReadContext::CreateContext();
    TestSchemaLocater2 locator;
    SchemaKey key;

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema1" alias="ts1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="TestClass">
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    EXPECT_STREQ("TestSchema1", schema->GetName().c_str());

    context->AddFirstSchemaLocater(locator);

    // Getting TestSchema2 because cache locater doesn't contain schema with empty key and there is TestSchemaLocater2 after it
    auto const locatedSchema1 = context->LocateSchema(key, SchemaMatchType::Latest);
    ASSERT_TRUE(locatedSchema1.IsValid());
    EXPECT_STREQ("TestSchema2", locatedSchema1->GetName().c_str());

    // Getting TestSchema1 because cache locater is in front of TestSchemaLocater2 and key matches cached schema key
    SchemaKey matchingKey("TestSchema1", 1, 0, 0);
    auto const locatedSchema2 = context->LocateSchema(matchingKey, SchemaMatchType::Latest);
    ASSERT_TRUE(locatedSchema2.IsValid());
    EXPECT_STREQ("TestSchema1", locatedSchema2->GetName().c_str());
    }

END_BENTLEY_ECN_TEST_NAMESPACE
