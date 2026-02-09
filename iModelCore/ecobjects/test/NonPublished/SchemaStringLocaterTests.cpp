/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaStringLocaterTests : ECTestFixture {};

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaStringLocaterTests, StringSchemaLocater_LocateSchemaWithSameKey) {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Test" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Fruit" isDomainClass="True" description="A generic description">
                <ECProperty propertyName="Color" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    StringSchemaLocater locater;
    locater.AddSchemaString(SchemaKey("Test", 1, 0, 0), schemaXml);
    context->AddSchemaLocater(locater);

    SchemaKey testKey("Test", 1, 0, 0);
    ECSchemaPtr schema = context->LocateSchema(testKey, SchemaMatchType::Latest);
    ASSERT_TRUE(schema.IsValid());

    auto fruit = schema->GetClassCP("Fruit");
    ASSERT_NE(nullptr, fruit);
}

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaStringLocaterTests, StringSchemaLocater_LocateSchemaWithDifferentKey) {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Test" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Fruit" isDomainClass="True" description="A generic description">
                <ECProperty propertyName="Color" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    StringSchemaLocater locater;
    locater.AddSchemaString(SchemaKey("Test", 1, 0, 1), schemaXml);
    context->AddSchemaLocater(locater);

    SchemaKey testKey("Test", 1, 0, 0);
    ECSchemaPtr schema = context->LocateSchema(testKey, SchemaMatchType::Latest);
    ASSERT_TRUE(schema.IsValid());

    auto fruit = schema->GetClassCP("Fruit");
    ASSERT_NE(nullptr, fruit);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaStringLocaterTests, LocatingSchemaContext_SchemaXmlIsDeleted) {
    Utf8String schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Test" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Fruit" isDomainClass="True" description="A generic description">
                <ECProperty propertyName="Color" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    StringSchemaLocater locater;
    locater.AddSchemaString(SchemaKey("Test", 1, 0, 0), schemaXml.c_str());
    context->AddSchemaLocater(locater);
    schemaXml.clear();

    SchemaKey testKey("Test", 1, 0, 0);
    ECSchemaPtr schema = context->LocateSchema(testKey, SchemaMatchType::Latest);
    ASSERT_TRUE(schema.IsValid());

    auto fruit = schema->GetClassCP("Fruit");
    ASSERT_NE(nullptr, fruit);
}

END_BENTLEY_ECN_TEST_NAMESPACE
