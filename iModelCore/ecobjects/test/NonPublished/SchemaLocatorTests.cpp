/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaLocatorTests : ECTestFixture {};

struct TestSchemaLocater1 : IECSchemaLocater, NonCopyableClass {
   protected:
    ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) override {
        ECSchemaPtr schema;
        ECSchema::CreateSchema(schema, "TestSchema1", "ts1", 1, 0, 0);
        return schema;
    }

   public:
    Utf8String GetDescription() const override {
        return Utf8PrintfString("TestSchemaLocater1");
    }
};

struct TestSchemaLocater2 : IECSchemaLocater, NonCopyableClass {
   protected:
    ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) override {
        ECSchemaPtr schema;
        ECSchema::CreateSchema(schema, "TestSchema2", "ts2", 2, 0, 0);
        return schema;
    }

   public:
    Utf8String GetDescription() const override {
        return Utf8PrintfString("TestSchemaLocater2");
    }
};

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaLocatorTests, SchemaReadContext_WithoutVersionExtension) {
    ECSchemaReadContextPtr readWithoutVersionContext = ECSchemaReadContext::CreateContext(false, true);
    readWithoutVersionContext->AddSchemaPath(ECTestFixture::GetTestDataPath(L"").c_str());

    ECSchemaReadContextPtr readWithVersionContext = ECSchemaReadContext::CreateContext(false, false);
    readWithVersionContext->AddSchemaPath(ECTestFixture::GetTestDataPath(L"").c_str());

    SchemaKey key("SchemaWithoutVersion", 1, 0, 0);

    ECSchemaPtr schema = readWithoutVersionContext->LocateSchema(key, SchemaMatchType::Exact);
    ASSERT_TRUE(schema.IsValid());

    ECSchemaPtr schema2 = readWithVersionContext->LocateSchema(key, SchemaMatchType::Exact);
    ASSERT_FALSE(schema2.IsValid());
}

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaLocatorTests, SearchSchemaFileLocater_WithoutVersionExtension) {
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
TEST_F(SchemaLocatorTests, LocatingSchemaContext_WithLocaterAddedFirst) {
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
TEST_F(SchemaLocatorTests, LocatingSchemaContext_WithPrioritizedLocater) {
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
TEST_F(SchemaLocatorTests, LocatingSchemaContext_CacheLocaterIsBeforePrioritizedLocater) {
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

TEST_F(SchemaLocatorTests, SanitizingSchemaLocater_CopySchemaUsesNewReference) {
    StringSchemaLocater locater;
    SchemaKey bisCoreKey("BisCore", 1, 0, 0);
    locater.AddSchemaString(bisCoreKey,
                            R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="BisCore" alias="bis" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Element" />
            <ECCustomAttributeClass typeName="ClassHasHandler" appliesTo="Any" />
        </ECSchema>)schema");

    SchemaKey domainSchemaKey("DomainSchema", 1, 0, 0);
    locater.AddSchemaString(domainSchemaKey,
                            R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="DomainSchema" alias="ds" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
            <ECEntityClass typeName="MyElement">
                <ECCustomAttributes>
                    <ClassHasHandler xmlns="BisCore.01.00.00" />
                </ECCustomAttributes>
                <BaseClass>bis:Element</BaseClass>
            </ECEntityClass>
        </ECSchema>)schema");

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(locater);
    auto domainSchema = context->LocateSchema(domainSchemaKey, SchemaMatchType::LatestReadCompatible);
    ASSERT_TRUE(domainSchema.IsValid());

    ECSchemaReadContextPtr context2 = ECSchemaReadContext::CreateContext();
    StringSchemaLocater locater2;
    SchemaKey bisCoreKey2("BisCore", 1, 0, 1);
    locater2.AddSchemaString(bisCoreKey2,
                             R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="BisCore" alias="bis" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Element" />
            <ECCustomAttributeClass typeName="ClassHasHandler" appliesTo="Any" />
        </ECSchema>)schema");
    SanitizingSchemaLocater sanitizingLocater(locater2);
    context2->AddSchemaLocater(sanitizingLocater);
    ECSchemaPtr domainSchema2;
    ASSERT_EQ(ECObjectsStatus::Success, domainSchema->CopySchema(domainSchema2, context2.get(), false));
    ASSERT_TRUE(domainSchema2.IsValid());
    ASSERT_TRUE(domainSchema2->GetSchemaKey().Matches(domainSchemaKey, SchemaMatchType::Exact));
    ECSchemaCP bisCore2 = nullptr;
    for (auto& refSchema : domainSchema2->GetReferencedSchemas()) {
        ASSERT_TRUE(bisCore2 == nullptr);
        bisCore2 = refSchema.second.get();
    }
    ASSERT_TRUE(bisCore2 != nullptr);
    ASSERT_TRUE(bisCore2->GetSchemaKey().Matches(bisCoreKey2, SchemaMatchType::Exact));
    auto myElement = domainSchema2->GetClassCP("MyElement");
    ASSERT_TRUE(myElement != nullptr);
    auto caInstance = myElement->GetCustomAttributeLocal("BisCore", "ClassHasHandler");
    ASSERT_TRUE(caInstance.IsValid());
    ASSERT_TRUE(caInstance->GetClass().GetSchema().GetSchemaKey().Matches(bisCoreKey2, SchemaMatchType::Exact));  // Validate CA class schema
    ECClassCP baseClass = nullptr;
    for (auto bc : myElement->GetBaseClasses()) {
        ASSERT_TRUE(baseClass == nullptr);
        baseClass = bc;
    }
    ASSERT_TRUE(baseClass != nullptr);
    ASSERT_TRUE(baseClass->GetSchema().GetSchemaKey().Matches(bisCoreKey2, SchemaMatchType::Exact));  // Validate base class schema
}

END_BENTLEY_ECN_TEST_NAMESPACE
