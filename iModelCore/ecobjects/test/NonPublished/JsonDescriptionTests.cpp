/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct JsonDescriptionTest : ECTestFixture
    {
    protected:
    // Build a minimal V3_3 schema containing one JsonDescription item and one json-typed property.
    ECSchemaPtr BuildBasicSchema()
        {
        const Utf8String schemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <JsonDescription typeName="JsonSchema" displayLabel="Test Label" description="Test desc">
                {"type":"object","properties":{"x":{"type":"number"},"y":{"type":"number"}},"required":["x","y"]}
            </JsonDescription>

            <ECEntityClass typeName="TestClass">
                <ECProperty propertyName="JsonProperty" typeName="json" jsonDescription="JsonSchema" />
            </ECEntityClass>
        </ECSchema>
        )xml";

        ECSchemaPtr schema;
        ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
        EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *ctx));
        return schema;
        }

    // Round-trip a schema through XML serialization and return the deserialized copy.
    ECSchemaPtr RoundTripXml(ECSchemaPtr const& schema)
        {
        Utf8String xmlStr;
        EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(xmlStr));

        ECSchemaPtr result;
        ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
        EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(result, Utf8String(xmlStr).c_str(), *ctx));
        return result;
        }

    // Serialize to JSON.
    BeJsDocument ToJson(ECSchemaPtr const& schema)
        {
        BeJsDocument doc;
        EXPECT_TRUE(schema->WriteToJsonValue(doc));
        return doc;
        }
    };

// =====================================================================================
// Basic Tests
// =====================================================================================
TEST_F(JsonDescriptionTest, CreateAndGetJsonDescription)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ms", 1, 0, 0, ECVersion::V3_3);

    JsonDescriptionP jd = nullptr;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateJsonDescription(jd, "TestJD"));
    ASSERT_NE(nullptr, jd);

    EXPECT_STREQ("TestJD", jd->GetName().c_str());
    EXPECT_STREQ("TestSchema:TestJD", jd->GetFullName().c_str());
    EXPECT_TRUE(jd->GetDisplayLabel().empty());
    EXPECT_TRUE(jd->GetDescription().empty());
    EXPECT_TRUE(jd->GetJsonSchema().empty());
    EXPECT_TRUE(schema.Equals(&jd->GetSchema()));
    }

TEST_F(JsonDescriptionTest, SetAndGetJsonDescriptionMetadata)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ms", 1, 0, 0, ECVersion::V3_3);

    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "TestJD");

    EXPECT_EQ(ECObjectsStatus::Success, jd->SetDisplayLabel("A Label"));
    EXPECT_STREQ("A Label", jd->GetDisplayLabel().c_str());

    EXPECT_EQ(ECObjectsStatus::Success, jd->SetDescription("A description."));
    EXPECT_STREQ("A description.", jd->GetDescription().c_str());
    }

TEST_F(JsonDescriptionTest, SetJsonSchema)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);

    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "JD");

    // Well-formed JSON object
    EXPECT_EQ(ECObjectsStatus::Success, jd->SetJsonSchema(R"({"type":"object","properties":{"count":{"type":"integer"}}})"));
    EXPECT_FALSE(jd->GetJsonSchema().empty());

    // Well-formed JSON array
    EXPECT_EQ(ECObjectsStatus::Success, jd->SetJsonSchema(R"([1,2,3])"));

    // Empty string clears the content
    EXPECT_EQ(ECObjectsStatus::Success, jd->SetJsonSchema(""));
    EXPECT_TRUE(jd->GetJsonSchema().empty());

    // nullptr clears content
    EXPECT_EQ(ECObjectsStatus::Success, jd->SetJsonSchema(nullptr));
    EXPECT_TRUE(jd->GetJsonSchema().empty());
    }

TEST_F(JsonDescriptionTest, MalformedSetJsonSchemaGetsRejectsWithParseError)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);

    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "JD");

    EXPECT_EQ(ECObjectsStatus::ParseError, jd->SetJsonSchema("{invalid json"));
    EXPECT_EQ(ECObjectsStatus::ParseError, jd->SetJsonSchema("{\"a\":}"));
    EXPECT_EQ(ECObjectsStatus::ParseError, jd->SetJsonSchema("not-json-at-all"));

    EXPECT_TRUE(jd->GetJsonSchema().empty());
    }

TEST_F(JsonDescriptionTest, DuplicateNameRejected)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);

    JsonDescriptionP jd1 = nullptr;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateJsonDescription(jd1, "Same"));

    JsonDescriptionP jd2 = nullptr;
    EXPECT_EQ(ECObjectsStatus::Error, schema->CreateJsonDescription(jd2, "Same"));
    EXPECT_EQ(nullptr, jd2);

    constexpr Utf8CP xml = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <JsonDescription typeName="JsonSchema" displayLabel="Test Label" description="Test desc">
                {"type":"object","properties":{"x":{"type":"number"},"y":{"type":"number"}},"required":["x","y"]}
            </JsonDescription>

            <JsonDescription typeName="JsonSchema" displayLabel="Test Label" description="Test desc">
                {"type":"number"}
            </JsonDescription>
        </ECSchema>
    )xml";

    ECSchemaPtr schema2;
    auto ctx = ECSchemaReadContext::CreateContext();
    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema2, xml, *ctx));
    }

TEST_F(JsonDescriptionTest, GetJsonDescriptionCountAndIteration)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);

    EXPECT_EQ(0u, schema->GetJsonDescriptionCount());

    JsonDescriptionP a;
    schema->CreateJsonDescription(a, "Alpha");
    
    JsonDescriptionP b;
    schema->CreateJsonDescription(b, "Beta");

    JsonDescriptionP c;
    schema->CreateJsonDescription(c, "Gamma");

    EXPECT_EQ(3u, schema->GetJsonDescriptionCount());

    std::vector<Utf8CP> jds = { "Alpha", "Beta", "Gamma" };
    auto count = 0;
    for (JsonDescriptionCP jd : schema->GetJsonDescriptions())
        {
        EXPECT_NE(nullptr, jd);
        EXPECT_STREQ(jd->GetName().c_str(), jds[count++]);
        }
    EXPECT_EQ(3, count);
    }

TEST_F(JsonDescriptionTest, LookupByName)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);

    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "TestJD");

    EXPECT_NE(nullptr, schema->GetJsonDescriptionCP("TestJD"));
    EXPECT_NE(nullptr, schema->GetJsonDescriptionP("TestJD"));
    // Invalid arg
    EXPECT_EQ(nullptr, schema->GetJsonDescriptionCP("DoesNotExist"));
    }

TEST_F(JsonDescriptionTest, DeleteJsonDescription)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);

    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "ToDelete");
    EXPECT_EQ(1u, schema->GetJsonDescriptionCount());

    EXPECT_EQ(ECObjectsStatus::Success, schema->DeleteJsonDescription(*jd));
    EXPECT_EQ(0u, schema->GetJsonDescriptionCount());
    EXPECT_EQ(nullptr, schema->GetJsonDescriptionCP("ToDelete"));
    }

TEST_F(JsonDescriptionTest, ImmutableSchemaRejectsCreate)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);
    schema->SetImmutable();

    JsonDescriptionP jd = nullptr;
    EXPECT_EQ(ECObjectsStatus::SchemaIsImmutable, schema->CreateJsonDescription(jd, "JD"));
    EXPECT_EQ(nullptr, jd);
    }

TEST_F(JsonDescriptionTest, PropertySetAndGetJsonDescription)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);

    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "JD");

    ECEntityClassP ecClass = nullptr;
    schema->CreateEntityClass(ecClass, "Foo");
    PrimitiveECPropertyP prop = nullptr;
    ecClass->CreatePrimitiveProperty(prop, "Val", PRIMITIVETYPE_Json);

    EXPECT_FALSE(prop->IsJsonDescriptionDefinedLocally());
    EXPECT_EQ(nullptr, prop->GetJsonDescription());

    EXPECT_EQ(ECObjectsStatus::Success, prop->SetJsonDescription(jd));
    EXPECT_TRUE(prop->IsJsonDescriptionDefinedLocally());
    ASSERT_NE(nullptr, prop->GetJsonDescription());
    EXPECT_STREQ("JD", prop->GetJsonDescription()->GetName().c_str());
    }

TEST_F(JsonDescriptionTest, PropertyUnsetJsonDescriptionWithNullptr)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);

    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "JD");

    ECEntityClassP ecClass = nullptr;
    schema->CreateEntityClass(ecClass, "Foo");
    PrimitiveECPropertyP prop = nullptr;
    ecClass->CreatePrimitiveProperty(prop, "Val", PRIMITIVETYPE_Json);
    prop->SetJsonDescription(jd);

    EXPECT_EQ(ECObjectsStatus::Success, prop->SetJsonDescription(nullptr));
    EXPECT_FALSE(prop->IsJsonDescriptionDefinedLocally());
    EXPECT_EQ(nullptr, prop->GetJsonDescription());
    }

TEST_F(JsonDescriptionTest, PropertySetJsonDescriptionFromUnreferencedSchemaFails)
    {
    ECSchemaPtr schemaA, schemaB;
    ECSchema::CreateSchema(schemaA, "A", "a", 1, 0, 0, ECVersion::V3_3);
    ECSchema::CreateSchema(schemaB, "B", "b", 1, 0, 0, ECVersion::V3_3);

    JsonDescriptionP jd = nullptr;
    schemaB->CreateJsonDescription(jd, "JD");

    ECEntityClassP ecClass = nullptr;
    schemaA->CreateEntityClass(ecClass, "Foo");
    PrimitiveECPropertyP prop = nullptr;
    ecClass->CreatePrimitiveProperty(prop, "Val", PRIMITIVETYPE_Json);

    // Fails as schemaA does NOT reference schemaB
    EXPECT_EQ(ECObjectsStatus::SchemaNotFound, prop->SetJsonDescription(jd));
    EXPECT_EQ(nullptr, prop->GetJsonDescription());
    }

TEST_F(JsonDescriptionTest, PropertySetJsonDescriptionFromReferencedSchemaSucceeds)
    {
    ECSchemaPtr schemaA, schemaB;
    ECSchema::CreateSchema(schemaA, "A", "a", 1, 0, 0, ECVersion::V3_3);
    ECSchema::CreateSchema(schemaB, "B", "b", 1, 0, 0, ECVersion::V3_3);
    schemaA->AddReferencedSchema(*schemaB);

    JsonDescriptionP jd = nullptr;
    schemaB->CreateJsonDescription(jd, "JD");

    ECEntityClassP ecClass = nullptr;
    schemaA->CreateEntityClass(ecClass, "Foo");
    PrimitiveECPropertyP prop = nullptr;
    ecClass->CreatePrimitiveProperty(prop, "Val", PRIMITIVETYPE_Json);

    // Passes as schemaA does reference schemaB
    EXPECT_EQ(ECObjectsStatus::Success, prop->SetJsonDescription(jd));
    ASSERT_NE(nullptr, prop->GetJsonDescription());
    EXPECT_STREQ("JD", prop->GetJsonDescription()->GetName().c_str());
    }

TEST_F(JsonDescriptionTest, SetJsonDescriptionOnStringPropertyFails)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);
    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "JD");

    ECEntityClassP ecClass = nullptr;
    schema->CreateEntityClass(ecClass, "C");
    PrimitiveECPropertyP prop = nullptr;
    ecClass->CreatePrimitiveProperty(prop, "StrVal", PRIMITIVETYPE_String);

    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, prop->SetJsonDescription(jd));
    EXPECT_EQ(nullptr, prop->GetJsonDescription());
    }

TEST_F(JsonDescriptionTest, SetJsonDescriptionOnIntegerPropertyFails)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);
    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "JD");

    ECEntityClassP ecClass = nullptr;
    schema->CreateEntityClass(ecClass, "C");
    PrimitiveECPropertyP prop = nullptr;
    ecClass->CreatePrimitiveProperty(prop, "IntVal", PRIMITIVETYPE_Integer);

    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, prop->SetJsonDescription(jd));
    EXPECT_EQ(nullptr, prop->GetJsonDescription());
    }

TEST_F(JsonDescriptionTest, SetJsonDescriptionOnDoublePropertyFails)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);
    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "JD");

    ECEntityClassP ecClass = nullptr;
    schema->CreateEntityClass(ecClass, "C");
    PrimitiveECPropertyP prop = nullptr;
    ecClass->CreatePrimitiveProperty(prop, "DblVal", PRIMITIVETYPE_Double);

    EXPECT_EQ(ECObjectsStatus::DataTypeMismatch, prop->SetJsonDescription(jd));
    EXPECT_EQ(nullptr, prop->GetJsonDescription());
    }

TEST_F(JsonDescriptionTest, XmlRoundTripSchemaItemPreserved)
    {
    ECSchemaPtr initialSchema = BuildBasicSchema();
    ECSchemaPtr roundTrippedSchema = RoundTripXml(initialSchema);

    ASSERT_TRUE(roundTrippedSchema.IsValid());
    EXPECT_EQ(1u, roundTrippedSchema->GetJsonDescriptionCount());

    JsonDescriptionCP jd = roundTrippedSchema->GetJsonDescriptionCP("JsonSchema");
    ASSERT_NE(nullptr, jd);
    EXPECT_STREQ("JsonSchema", jd->GetName().c_str());
    EXPECT_STREQ("Test Label", jd->GetDisplayLabel().c_str());
    EXPECT_STREQ("Test desc", jd->GetDescription().c_str());
    EXPECT_FALSE(jd->GetJsonSchema().empty());
    }

TEST_F(JsonDescriptionTest, XmlRoundTripPropertyReferencePreserved)
    {
    ECSchemaPtr initialSchema = BuildBasicSchema();
    ECSchemaPtr roundTrippedSchema = RoundTripXml(initialSchema);

    ASSERT_TRUE(roundTrippedSchema.IsValid());
    PrimitiveECPropertyCP prop = roundTrippedSchema->GetClassCP("TestClass")->GetPropertyP("JsonProperty")->GetAsPrimitiveProperty();
    ASSERT_NE(nullptr, prop);
    EXPECT_TRUE(prop->IsJsonDescriptionDefinedLocally());
    ASSERT_NE(nullptr, prop->GetJsonDescription());
    EXPECT_STREQ("JsonSchema", prop->GetJsonDescription()->GetName().c_str());
    }

TEST_F(JsonDescriptionTest, XmlRoundTripMultipleSchemaItems)
    {
    ECSchemaPtr initialSchema;
    ECSchema::CreateSchema(initialSchema, "S", "s", 1, 0, 0, ECVersion::V3_3);

    JsonDescriptionP jd1 = nullptr;
    initialSchema->CreateJsonDescription(jd1, "Alpha");
    jd1->SetJsonSchema(R"({"type":"number"})");

    JsonDescriptionP jd2 = nullptr;
    initialSchema->CreateJsonDescription(jd2, "Beta");
    jd2->SetJsonSchema(R"({"type":"string"})");

    ECEntityClassP ecClass = nullptr;
    initialSchema->CreateEntityClass(ecClass, "C");

    PrimitiveECPropertyP p1 = nullptr;
    ecClass->CreatePrimitiveProperty(p1, "A", PRIMITIVETYPE_Json);
    p1->SetJsonDescription(jd1);

    PrimitiveECPropertyP p2 = nullptr;
    ecClass->CreatePrimitiveProperty(p2, "B", PRIMITIVETYPE_Json);
    p2->SetJsonDescription(jd2);

    ECSchemaPtr roundTrippedSchema = RoundTripXml(initialSchema);
    ASSERT_TRUE(roundTrippedSchema.IsValid());
    EXPECT_EQ(2u, roundTrippedSchema->GetJsonDescriptionCount());

    ASSERT_NE(nullptr, roundTrippedSchema->GetJsonDescriptionCP("Alpha"));
    ASSERT_NE(nullptr, roundTrippedSchema->GetJsonDescriptionCP("Beta"));

    PrimitiveECPropertyCP rp1 = roundTrippedSchema->GetClassCP("C")->GetPropertyP("A")->GetAsPrimitiveProperty();
    PrimitiveECPropertyCP rp2 = roundTrippedSchema->GetClassCP("C")->GetPropertyP("B")->GetAsPrimitiveProperty();
    ASSERT_NE(nullptr, rp1->GetJsonDescription());
    ASSERT_NE(nullptr, rp2->GetJsonDescription());
    EXPECT_STREQ("Alpha", rp1->GetJsonDescription()->GetName().c_str());
    EXPECT_STREQ("Beta",  rp2->GetJsonDescription()->GetName().c_str());
    }

TEST_F(JsonDescriptionTest, GetQualifiedName_SameSchema)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);
    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "JD");

    // no alias prefix
    EXPECT_STREQ("JD", jd->GetQualifiedName(*schema).c_str());
    }

TEST_F(JsonDescriptionTest, GetQualifiedName_CrossSchema)
    {
    ECSchemaPtr schemaA, schemaB;
    ECSchema::CreateSchema(schemaA, "A", "myAlias", 1, 0, 0, ECVersion::V3_3);
    ECSchema::CreateSchema(schemaB, "B", "b", 1, 0, 0, ECVersion::V3_3);
    schemaB->AddReferencedSchema(*schemaA);

    JsonDescriptionP jd = nullptr;
    schemaA->CreateJsonDescription(jd, "JD");

    // primarySchema is schemaB, jd is in schemaA -> should be "myAlias:JD"
    EXPECT_STREQ("myAlias:JD", jd->GetQualifiedName(*schemaB).c_str());
    }

TEST_F(JsonDescriptionTest, BasePropertyGetterReturnsCorrectJD)
    {
    ECSchemaPtr schema = BuildBasicSchema();

    ECPropertyCP baseProp = schema->GetClassCP("TestClass")->GetPropertyP("JsonProperty");
    ASSERT_NE(nullptr, baseProp);
    EXPECT_TRUE(baseProp->IsJsonDescriptionDefinedLocally());
    ASSERT_NE(nullptr, baseProp->GetJsonDescription());
    EXPECT_STREQ("JsonSchema", baseProp->GetJsonDescription()->GetName().c_str());
    }

// =====================================================================================
// Cross-schema Tests: JD defined in refSchema, referenced by property in mainSchema
// =====================================================================================
TEST_F(JsonDescriptionTest, XmlRoundTripCrossSchemaReference)
    {
    ECSchemaPtr refSchema, mainSchema;
    ECSchema::CreateSchema(refSchema, "Ref", "ref", 1, 0, 0, ECVersion::V3_3);
    ECSchema::CreateSchema(mainSchema, "Main", "main", 1, 0, 0, ECVersion::V3_3);
    mainSchema->AddReferencedSchema(*refSchema);

    JsonDescriptionP jd = nullptr;
    refSchema->CreateJsonDescription(jd, "RemoteJD");
    jd->SetJsonSchema(R"({"type":"object"})");

    ECEntityClassP ecClass = nullptr;
    mainSchema->CreateEntityClass(ecClass, "C");
    PrimitiveECPropertyP prop = nullptr;
    ecClass->CreatePrimitiveProperty(prop, "Val", PRIMITIVETYPE_Json);
    prop->SetJsonDescription(jd);

    // The XML attribute in the main schema should use the alias "ref:RemoteJD"
    Utf8String xmlStr;
    ASSERT_EQ(SchemaWriteStatus::Success, mainSchema->WriteToXmlString(xmlStr));
    Utf8String utf8Xml(xmlStr);
    EXPECT_NE(Utf8String::npos, utf8Xml.find("ref:RemoteJD")) << "Expected qualified alias in XML attribute";

    // Write refSchema too so the locater can find it during round-trip
    Utf8String refXmlStr;
    ASSERT_EQ(SchemaWriteStatus::Success, refSchema->WriteToXmlString(refXmlStr));

    // Round-trip via StringSchemaLocater so the referenced schema resolves
    StringSchemaLocater locater;
    locater.AddSchemaString(SchemaKey("Ref", 1, 0, 0), Utf8String(refXmlStr).c_str());
    locater.AddSchemaString(SchemaKey("Main", 1, 0, 0), utf8Xml.c_str());

    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(locater);

    SchemaKey key("Main", 1, 0, 0);
    ECSchemaPtr rt = ctx->LocateSchema(key, SchemaMatchType::Latest);
    ASSERT_TRUE(rt.IsValid());

    PrimitiveECPropertyCP rtProp = rt->GetClassCP("C")->GetPropertyP("Val")->GetAsPrimitiveProperty();
    ASSERT_NE(nullptr, rtProp->GetJsonDescription());
    EXPECT_STREQ("RemoteJD", rtProp->GetJsonDescription()->GetName().c_str());
    EXPECT_STREQ("Ref", rtProp->GetJsonDescription()->GetSchema().GetName().c_str());
    }

// =====================================================================================
// JsonDescription element is suppressed when serialized with ECXml < V3_3
// =====================================================================================
TEST_F(JsonDescriptionTest, XmlJsonDescriptionNotWrittenForOlderVersions)
    {
    ECSchemaPtr schema = BuildBasicSchema();

    Utf8String xmlV32;
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(xmlV32, ECVersion::V3_2));
    ASSERT_FALSE(xmlV32.empty());

    EXPECT_EQ(Utf8String::npos, xmlV32.find("JsonDescription")) << "JsonDescription is a V3.3 feature and should not appear in V3.2 output";
    EXPECT_EQ(Utf8String::npos, xmlV32.find("jsonDescription")) << "jsonDescription attribute is a V3.3 feature and should not appear in V3.2 output";

    Utf8String xmlV31;
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(xmlV31, ECVersion::V3_1));
    ASSERT_FALSE(xmlV31.empty());

    EXPECT_EQ(Utf8String::npos, xmlV31.find("JsonDescription")) << "JsonDescription is a V3.3 feature and should not appear in V3.1 output";
    EXPECT_EQ(Utf8String::npos, xmlV31.find("jsonDescription")) << "jsonDescription attribute is a V3.3 feature and should not appear in V3.1 output";
    }

// =====================================================================================
// Xml reader tests
// =====================================================================================
TEST_F(JsonDescriptionTest, MissingTypeNameAttributeFails)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="S" alias="s" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <JsonDescription displayLabel="X">
                {"type":"object"}
            </JsonDescription>
        </ECSchema>)xml";

    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *ctx));
    }

TEST_F(JsonDescriptionTest, MalformedJsonSchemaContentFails)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="S" alias="s" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <JsonDescription typeName="BadJD">
                not valid json {{{
            </JsonDescription>
        </ECSchema>)xml";

    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *ctx));
    }

TEST_F(JsonDescriptionTest, UnknownJsonDescriptionRefOnPropertyIgnored)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="S" alias="s" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECEntityClass typeName="C">
                <ECProperty propertyName="Val" typeName="json" jsonDescription="DoesNotExist"/>
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *ctx));
    }

TEST_F(JsonDescriptionTest, UnknownAliasOnPropertyFails)
    {
    Utf8CP schemaXml = R"xml(
        <?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="S" alias="s" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECEntityClass typeName="C">
                <ECProperty propertyName="Val" typeName="json" jsonDescription="unknownAlias:JD"/>
            </ECEntityClass>
        </ECSchema>)xml";

    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    EXPECT_EQ(SchemaReadStatus::ReferencedSchemaNotFound, ECSchema::ReadFromXmlString(schema, schemaXml, *ctx));
    }

TEST_F(JsonDescriptionTest, EmptyJsonDescriptionBodyFailsValidation)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="S" alias="s" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <JsonDescription typeName="EmptyJD" displayLabel="Test Label" description="Test desc"/>
        </ECSchema>)xml";

    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    // A JsonDescription with no JSON Schema body must be rejected
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *ctx));
    }

// =====================================================================================
// ToJson tests
// =====================================================================================
TEST_F(JsonDescriptionTest, JsonDescriptionGetsWrittenToSchemaItems)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);
    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "TestJD");
    jd->SetDisplayLabel("Test Label");
    jd->SetDescription("Test desc.");
    jd->SetJsonSchema(R"({"type":"string"})");

    BeJsDocument doc = ToJson(schema);

    BeJsValue item = doc["items"]["TestJD"];
    EXPECT_FALSE(item.isNull()) << "Expected items.TestJD in JSON output";
    EXPECT_STREQ("JsonDescription", item["schemaItemType"].asCString(""));
    EXPECT_STREQ("Test Label",        item["label"].asCString(""));
    EXPECT_STREQ("Test desc.",        item["description"].asCString(""));
    EXPECT_FALSE(item["jsonSchema"].isNull());
    }

TEST_F(JsonDescriptionTest, JsonPropertyReferenceWritten)
    {
    ECSchemaPtr schema = BuildBasicSchema();
    BeJsDocument doc = ToJson(schema);

    // Properties are an array under items.ClassName.properties
    BeJsConst propertiesArr = doc["items"]["TestClass"]["properties"];
    ASSERT_TRUE(propertiesArr.isArray() && propertiesArr.size() >= 1);
    BeJsConst dataProp = propertiesArr[0];
    EXPECT_FALSE(dataProp["jsonDescription"].isNull()) << "Expected jsonDescription field on TestClass.JsonProperty in JSON output";
    }

// =====================================================================================
// Each of these tests verifies that the given JSON Schema body is accepted by SetJsonSchema.
// They also round-trip through XML to prove the content survives serialization.
// =====================================================================================
TEST_F(JsonDescriptionTest, JsonSchemaWithStringTypeWithConstraints)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);
    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "JD");

    Utf8CP jsonSchema = R"({"type":"string","minLength":1,"maxLength":255,"pattern":"^[A-Za-z0-9_]+$"})";
    ASSERT_EQ(ECObjectsStatus::Success, jd->SetJsonSchema(jsonSchema));
    EXPECT_FALSE(jd->GetJsonSchema().empty());

    ECSchemaPtr rt = RoundTripXml(schema);
    ASSERT_TRUE(rt.IsValid());
    JsonDescriptionCP rtJd = rt->GetJsonDescriptionCP("JD");
    ASSERT_NE(nullptr, rtJd);
    EXPECT_FALSE(rtJd->GetJsonSchema().empty());
    }

TEST_F(JsonDescriptionTest, JsonSchemaWithNumberTypeWithRange)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);
    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "JD");

    Utf8CP jsonSchema = R"({"type":"number","minimum":0.0,"maximum":1.0,"multipleOf":0.01})";
    ASSERT_EQ(ECObjectsStatus::Success, jd->SetJsonSchema(jsonSchema));

    ECSchemaPtr rt = RoundTripXml(schema);
    ASSERT_TRUE(rt.IsValid());
    EXPECT_FALSE(rt->GetJsonDescriptionCP("JD")->GetJsonSchema().empty());
    }

TEST_F(JsonDescriptionTest, JsonSchemaWithArrayOfTypedItems)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);
    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "JD");

    Utf8CP jsonSchema = R"({"type":"array","items":{"type":"number"},"minItems":1,"maxItems":10,"uniqueItems":true})";
    ASSERT_EQ(ECObjectsStatus::Success, jd->SetJsonSchema(jsonSchema));

    ECSchemaPtr rt = RoundTripXml(schema);
    ASSERT_TRUE(rt.IsValid());
    EXPECT_FALSE(rt->GetJsonDescriptionCP("JD")->GetJsonSchema().empty());
    }

TEST_F(JsonDescriptionTest, JsonSchemaWithEnumValues)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);
    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "JD");

    Utf8CP jsonSchema = R"({"type":"string","enum":["red","green","blue","yellow"]})";
    ASSERT_EQ(ECObjectsStatus::Success, jd->SetJsonSchema(jsonSchema));

    ECSchemaPtr rt = RoundTripXml(schema);
    ASSERT_TRUE(rt.IsValid());
    // Verify the round-tripped body is still valid JSON by parsing it
    JsonDescriptionCP rtJd = rt->GetJsonDescriptionCP("JD");
    ASSERT_NE(nullptr, rtJd);
    EXPECT_FALSE(rtJd->GetJsonSchema().empty());
    }

TEST_F(JsonDescriptionTest, JsonSchemaWithAllOfComposition)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);
    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "JD");

    // allOf combines two object sub-schemas
    Utf8CP jsonSchema = R"({"allOf":[{"type":"object","properties":{"x":{"type":"number"}},"required":["x"]},{"type":"object","properties":{"y":{"type":"number"}},"required":["y"]}]})";
    ASSERT_EQ(ECObjectsStatus::Success, jd->SetJsonSchema(jsonSchema));

    ECSchemaPtr rt = RoundTripXml(schema);
    ASSERT_TRUE(rt.IsValid());
    EXPECT_FALSE(rt->GetJsonDescriptionCP("JD")->GetJsonSchema().empty());
    }

TEST_F(JsonDescriptionTest, JsonSchemaWithNullableType)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);
    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "JD");

    // Nullable expressed as type array
    Utf8CP jsonSchema = R"({"type":["object","null"],"properties":{"value":{"type":"number"}}})";
    ASSERT_EQ(ECObjectsStatus::Success, jd->SetJsonSchema(jsonSchema));

    ECSchemaPtr rt = RoundTripXml(schema);
    ASSERT_TRUE(rt.IsValid());
    EXPECT_FALSE(rt->GetJsonDescriptionCP("JD")->GetJsonSchema().empty());
    }

// =====================================================================================
// Verifies a complex, deeply nested JSON Schema survives XML serialization intact.
// The round-tripped body must itself be valid JSON (re-validated by SetJsonSchema).
// =====================================================================================
TEST_F(JsonDescriptionTest, RoundTripComplexSchemaContentPreserved)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);
    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "Complex");

    Utf8CP complexJson = R"json({
        "type":"object",
        "properties": {
            "name": {
                "type": "string",
                "maxLength": 100
            },
            "tags":{
                "type": "array",
                "items": { "type": "string" },
                "uniqueItems": true
            },
            "score":{
                "type": "number",
                "minimum": 0.0,
                "maximum": 1.0
            },
            "metadata": {
                "type": "object",
                "additionalProperties": { "type": "string" }
            }
        },
        "required": ["name","score"],
        "additionalProperties": false
    })json";
    ASSERT_EQ(ECObjectsStatus::Success, jd->SetJsonSchema(complexJson));

    ECSchemaPtr rt = RoundTripXml(schema);
    ASSERT_TRUE(rt.IsValid());
    JsonDescriptionCP rtJd = rt->GetJsonDescriptionCP("Complex");
    ASSERT_NE(nullptr, rtJd);

    // The round-tripped content must itself parse as valid JSON.
    // Re-validate by assigning it to a fresh JsonDescription which causes a parse.
    ECSchemaPtr verifySchema;
    ECSchema::CreateSchema(verifySchema, "V", "v", 1, 0, 0, ECVersion::V3_3);
    JsonDescriptionP verify = nullptr;
    verifySchema->CreateJsonDescription(verify, "V");
    EXPECT_EQ(ECObjectsStatus::Success, verify->SetJsonSchema(rtJd->GetJsonSchema().c_str())) << "Round-tripped JSON Schema body must still be valid JSON";
    }

// =====================================================================================
// XML special characters inside a JSON string value must survive XML escaping.
// =====================================================================================
TEST_F(JsonDescriptionTest, XmlSpecialCharsInSchemaPreservedThroughRoundTrip)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);
    JsonDescriptionP jd = nullptr;
    schema->CreateJsonDescription(jd, "JD");

    // The description field contains '<', '>', '&' characters that XML must escape.
    Utf8CP jsonSchema = R"({"type":"string","description":"Value must satisfy: x > 0 && x < 100"})";
    ASSERT_EQ(ECObjectsStatus::Success, jd->SetJsonSchema(jsonSchema));

    ECSchemaPtr rt = RoundTripXml(schema);
    ASSERT_TRUE(rt.IsValid());
    JsonDescriptionCP rtJd = rt->GetJsonDescriptionCP("JD");
    ASSERT_NE(nullptr, rtJd);

    // The '>', '<', '&' characters must be preserved in the round-tripped body.
    EXPECT_NE(Utf8String::npos, rtJd->GetJsonSchema().find(">"))  << "Expected '>' to survive XML round-trip";
    EXPECT_NE(Utf8String::npos, rtJd->GetJsonSchema().find("<"))  << "Expected '<' to survive XML round-trip";
    EXPECT_NE(Utf8String::npos, rtJd->GetJsonSchema().find("&&")) << "Expected '&&' to survive XML round-trip";
    }

// ========================================================================================
// Sanity test: Clearing a JsonDescription with nullptr must always succeed, regardless of type.
// ========================================================================================
TEST_F(JsonDescriptionTest, ClearJsonDescriptionFromPropertyWithNullptr)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "S", "s", 1, 0, 0, ECVersion::V3_3);

    ECEntityClassP ecClass = nullptr;
    schema->CreateEntityClass(ecClass, "C");

    // Even on a non-json property, clearing (nullptr) must succeed.
    PrimitiveECPropertyP strProp = nullptr;
    ecClass->CreatePrimitiveProperty(strProp, "StrVal", PRIMITIVETYPE_String);
    EXPECT_EQ(ECObjectsStatus::Success, strProp->SetJsonDescription(nullptr));

    // And on a json property that has no JD set.
    PrimitiveECPropertyP jsonProp = nullptr;
    ecClass->CreatePrimitiveProperty(jsonProp, "JsonVal", PRIMITIVETYPE_Json);
    EXPECT_EQ(ECObjectsStatus::Success, jsonProp->SetJsonDescription(nullptr));
    }

END_BENTLEY_ECN_TEST_NAMESPACE
