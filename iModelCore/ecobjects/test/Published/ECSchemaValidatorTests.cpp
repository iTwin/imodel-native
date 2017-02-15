/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECSchemaValidatorTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaValidatorTests : ECTestFixture {};

TEST_F(SchemaValidatorTests, TestBaseECValidation)
    {
    // Test failure if not latest EC Version schema
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" isDomainClass="true"/>
            <ECRelationshipClass typeName="TestRelationship">
                <Source cardinality="(1,1)" polymorphic="true">
                    <Class class="TestClass"/>
                </Source>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_FALSE(schema->IsECVersion(ECVersion::Latest));

    EXPECT_FALSE(ECSchemaValidator::Validate(*schema)) << "TestSchema validated successfully even though it is not a valid EC3.1 schema";
    }
    // Test successfully validates EC2.0 schema
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" namespacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" isDomainClass="true"/>
            <ECClass typeName="A" isDomainClass="true"/>
            <ECRelationshipClass typeName="ARelB">
                <Source cardinality="(1,1)" polymorphic="true">
                    <Class class="TestClass"/>
                </Source>
                <Target cardinality="(1,1)" polymorphic="true">
                    <Class class="A"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));

    EXPECT_TRUE(ECSchemaValidator::Validate(*schema)) << "TestSchema failed to validate successfully even though it a valid EC3.1 schema";
    }
    // Test successfully validates EC3.1 schema
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="TestClass"/>
            <ECEntityClass typeName="A"/>
            <ECRelationshipClass typeName="ARelB" modifier="None">
                <Source multiplicity="(1..1)" polymorphic="true" roleLabel="Source">
                    <Class class="TestClass"/>
                </Source>
                <Target multiplicity="(1..1)" polymorphic="true" roleLabel="Target">
                    <Class class="A"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_TRUE(schema.IsValid());
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest));

    EXPECT_TRUE(ECSchemaValidator::Validate(*schema)) << "TestSchema failed to validate successfully even though it is a valid EC3.1 schema";
    }
    }

END_BENTLEY_ECN_TEST_NAMESPACE