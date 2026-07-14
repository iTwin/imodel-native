/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct JsonDescriptionTest : ECDbTestFixture
    {
    // Schema with a plain json property without a JsonDescription constraint
    static constexpr Utf8CP baseSchema = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <ECEntityClass typeName="TestClass">
                <ECProperty propertyName="JsonProperty" typeName="json"/>
            </ECEntityClass>
        </ECSchema>)xml";

    // Schema with a JsonDescription-constrained json property alongside an unconstrained one
    static constexpr Utf8CP schemaWithJsonDescription = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
            <JsonDescription typeName="JsonSchema">
                {"type":"object","properties":{"x":{"type":"number"},"y":{"type":"number"}},"required":["x","y"]}
            </JsonDescription>

            <ECEntityClass typeName="TestClass">
                <ECProperty propertyName="JsonProperty" typeName="json" jsonDescription="JsonSchema"/>
                <ECProperty propertyName="AnyJson" typeName="json"/>
            </ECEntityClass>
        </ECSchema>)xml";
    };

// =====================================================================================-
// ECSql tests with Binders
// =====================================================================================
TEST_F(JsonDescriptionTest, InsertWithBindValidJsonObject)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("bind_valid_obj.ecdb", SchemaItem(baseSchema)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindText(1, R"({"x":1,"y":2})", IECSqlBinder::MakeCopy::No));
    ECInstanceKey key;
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }

TEST_F(JsonDescriptionTest, BindJsonTextWithOtherValidShapes)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("bind_valid_shapes.ecdb", SchemaItem(baseSchema)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES (?)"));

    // JSON array
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindText(1, R"([1,2,3])", IECSqlBinder::MakeCopy::No));
    stmt.Reset(); stmt.ClearBindings();

    // JSON number
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindText(1, "42", IECSqlBinder::MakeCopy::No));
    stmt.Reset(); stmt.ClearBindings();

    // JSON boolean
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindText(1, "true", IECSqlBinder::MakeCopy::No));
    stmt.Reset(); stmt.ClearBindings();

    // JSON null literal
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindText(1, "null", IECSqlBinder::MakeCopy::No));
    stmt.Reset(); stmt.ClearBindings();

    // JSON string
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindText(1, R"("hello")", IECSqlBinder::MakeCopy::No));
    }

TEST_F(JsonDescriptionTest, BindNull)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("bind_null.ecdb", SchemaItem(baseSchema)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindNull(1));
    ECInstanceKey key;
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }

TEST_F(JsonDescriptionTest, BindValidJsonWithAnUpdate)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("bind_valid_update.ecdb", SchemaItem(baseSchema)));

    // First insert a row
    ECSqlStatement ins;
    ASSERT_EQ(ECSqlStatus::Success, ins.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES (?)"));
    ins.BindText(1, R"({"a":1})", IECSqlBinder::MakeCopy::No);
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, ins.Step(key));

    // Now update with another valid JSON value
    ECSqlStatement upd;
    ASSERT_EQ(ECSqlStatus::Success, upd.Prepare(m_ecdb, "UPDATE ts.TestClass SET JsonProperty = ? WHERE ECInstanceId = ?"));
    EXPECT_EQ(ECSqlStatus::Success, upd.BindText(1, R"({"a":99})", IECSqlBinder::MakeCopy::No));
    EXPECT_EQ(ECSqlStatus::Success, upd.BindInt64(2, (int64_t)key.GetInstanceId().GetValueUnchecked()));
    EXPECT_EQ(BE_SQLITE_DONE, upd.Step());
    }

TEST_F(JsonDescriptionTest, BindingMalformedJsonFails)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("bind_malformed_obj.ecdb", SchemaItem(baseSchema)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Error, stmt.BindText(1, "{not valid json", IECSqlBinder::MakeCopy::No)) << "BindText must reject malformed JSON for a json-typed property";
    }

TEST_F(JsonDescriptionTest, BindBareWordJsonFails)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("bind_bareword.ecdb", SchemaItem(baseSchema)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Error, stmt.BindText(1, "hello", IECSqlBinder::MakeCopy::No))
        << "BindText must reject a bare word that is not valid JSON";
    }

TEST_F(JsonDescriptionTest, BindEmptyJsonStringFails)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("bind_empty.ecdb", SchemaItem(baseSchema)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Error, stmt.BindText(1, "", IECSqlBinder::MakeCopy::No)) << "BindText must reject empty string for a json-typed property (not valid JSON)";
    }

TEST_F(JsonDescriptionTest, BindTruncatedJsonFails)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("bind_truncated.ecdb", SchemaItem(baseSchema)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Error, stmt.BindText(1, R"({"x":1,)", IECSqlBinder::MakeCopy::No)) << "BindText must reject a truncated JSON string";
    }

TEST_F(JsonDescriptionTest, BindMalformedJsonFails)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("bind_malformed_upd.ecdb", SchemaItem(baseSchema)));

    // Insert a valid row first
    ECSqlStatement ins;
    ASSERT_EQ(ECSqlStatus::Success, ins.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES (?)"));
    ins.BindText(1, R"({"x":1})", IECSqlBinder::MakeCopy::No);
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, ins.Step(key));

    ECSqlStatement upd;
    ASSERT_EQ(ECSqlStatus::Success, upd.Prepare(m_ecdb, "UPDATE ts.TestClass SET JsonProperty = ? WHERE ECInstanceId = ?"));
    EXPECT_EQ(ECSqlStatus::Error, upd.BindText(1, "{bad json example!!", IECSqlBinder::MakeCopy::No)) << "BindText must reject malformed JSON in UPDATE";
    }

// =====================================================================================
// ECSql tests with inline literals instead of binders
// =====================================================================================
TEST_F(JsonDescriptionTest, ValidObjectPrepare)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("literal_valid.ecdb", SchemaItem(baseSchema)));

    ECSqlStatement stmt;
    // suppress log output since we just want to check status
    EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, R"(INSERT INTO ts.TestClass (JsonProperty) VALUES ('{"x":1,"y":2}'))"));
    }

TEST_F(JsonDescriptionTest, MalformedObjectPrepareFails)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("literal_malformed.ecdb", SchemaItem(baseSchema)));

    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES ('{not json}')", false));
    }

TEST_F(JsonDescriptionTest, MalformedObjectUpdatePrepareFails)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("literal_malformed_upd.ecdb", SchemaItem(baseSchema)));

    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "UPDATE ts.TestClass SET JsonProperty = '{not json}'", false));
    }

// =====================================================================================
// JsonDescription schema conformance
// =====================================================================================
TEST_F(JsonDescriptionTest, ValueConformingToJsonSchema)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("jd_conform.ecdb", SchemaItem(schemaWithJsonDescription)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindText(1, R"({"x":1.0,"y":2.0})", IECSqlBinder::MakeCopy::No)) << "A value matching the JsonSchema must be accepted";
    ECInstanceKey key;
    EXPECT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }

TEST_F(JsonDescriptionTest, UnconstrainedProperty)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("jd_unconstrained.ecdb", SchemaItem(schemaWithJsonDescription)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (AnyJson) VALUES (?)"));

    // Any valid JSON shape must be accepted for an unconstrained property
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindText(1, R"({"x":1})", IECSqlBinder::MakeCopy::No)); stmt.Reset(); stmt.ClearBindings();
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindText(1, R"([1,2,3])", IECSqlBinder::MakeCopy::No)); stmt.Reset(); stmt.ClearBindings();
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindText(1, R"("just a string")", IECSqlBinder::MakeCopy::No)); stmt.Reset(); stmt.ClearBindings();
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindText(1, "42", IECSqlBinder::MakeCopy::No));
    }

TEST_F(JsonDescriptionTest, MissingRequiredFieldFails)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("jd_missing_field.ecdb", SchemaItem(schemaWithJsonDescription)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Error,stmt.BindText(1, R"({"x":1.0})", IECSqlBinder::MakeCopy::No));   // missing required "y"
    }

TEST_F(JsonDescriptionTest, WrongFieldTypeFails)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("jd_wrong_type.ecdb", SchemaItem(schemaWithJsonDescription)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Error, stmt.BindText(1, R"({"x":"not-a-number","y":2.0})", IECSqlBinder::MakeCopy::No)); // x must be a number
    }

TEST_F(JsonDescriptionTest, WrongRootTypeFails)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("jd_wrong_root.ecdb", SchemaItem(schemaWithJsonDescription)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES (?)"));

    // Passing a JSON array where the schema expects an object
    EXPECT_EQ(ECSqlStatus::Error, stmt.BindText(1, R"([1.0, 2.0])", IECSqlBinder::MakeCopy::No));

    stmt.Reset(); stmt.ClearBindings();

    // Passing a JSON number where the schema expects an object
    EXPECT_EQ(ECSqlStatus::Error, stmt.BindText(1, "42", IECSqlBinder::MakeCopy::No));
    }

TEST_F(JsonDescriptionTest, NullBindingSucceeds)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("jd_null.ecdb", SchemaItem(schemaWithJsonDescription)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindNull(1));
    }

TEST_F(JsonDescriptionTest, ExtraFields)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("jd_extra_fields.ecdb", SchemaItem(schemaWithJsonDescription)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestClass (JsonProperty) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindText(1, R"({"x":1.0,"y":2.0,"z":3.0,"label":"origin"})", IECSqlBinder::MakeCopy::No));
    }

// =====================================================================================
// Schema upgrade scenarios
//
// Schemas used across upgrade tests. Each test creates its own file so schema names
// do not conflict across tests.
//
// "UpgradeTest": used for tests that start with a constrained json property.
//   v1.0.0: JsonSchema requires {"x","y"}; GeoCoords.Value references it.
//   v1.0.1: JsonSchema is tightened to also require {"z"}.
//   v1.0.1: JsonSchema is loosened to require only {"x"}.
//   v2.0.0: JsonSchema is removed entirely; GeoCoords.Value becomes unconstrained.
//
// "UpgradeTest2": used for the "add description" test which starts unconstrained.
//   v1.0.0: GeoCoords.Value has typeName="json" with no jsonDescription.
//   v1.0.1: JsonSchema added; GeoCoords.Value now references it.
// =====================================================================================

static constexpr Utf8CP initialSchema = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="UpgradeTest" alias="ut" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
        <JsonDescription typeName="JsonSchema">
            {"type":"object","properties":{"x":{"type":"number"},"y":{"type":"number"}},"required":["x","y"]}
        </JsonDescription>
        <ECEntityClass typeName="GeoCoords">
            <ECProperty propertyName="Value" typeName="json" jsonDescription="JsonSchema"/>
        </ECEntityClass>
    </ECSchema>)xml";

// JsonSchema tightened: now also requires "z".
static constexpr Utf8CP firstUpdate = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="UpgradeTest" alias="ut" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
        <JsonDescription typeName="JsonSchema">
            {"type":"object","properties":{"x":{"type":"number"},"y":{"type":"number"},"z":{"type":"number"}},"required":["x","y","z"]}
        </JsonDescription>
        <ECEntityClass typeName="GeoCoords">
            <ECProperty propertyName="Value" typeName="json" jsonDescription="JsonSchema"/>
        </ECEntityClass>
    </ECSchema>)xml";

// JsonSchema loosened: only "x" is required.
static constexpr Utf8CP secondUpdate = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="UpgradeTest" alias="ut" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
        <JsonDescription typeName="JsonSchema">
            {"type":"object","properties":{"x":{"type":"number"}},"required":["x"]}
        </JsonDescription>
        <ECEntityClass typeName="GeoCoords">
            <ECProperty propertyName="Value" typeName="json" jsonDescription="JsonSchema"/>
        </ECEntityClass>
    </ECSchema>)xml";

// JsonSchema removed; GeoCoords.Value becomes a plain unconstrained json property.
static constexpr Utf8CP firstMajorUpdate = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="UpgradeTest" alias="ut" version="2.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
        <ECEntityClass typeName="GeoCoords">
            <ECProperty propertyName="Value" typeName="json"/>
        </ECEntityClass>
    </ECSchema>)xml";

//---------------------------------------------------------------------------------------
// Upgrade: JsonSchema blob is tightened (adds a new required field "z").
// Existing rows that satisfied the old schema are NOT re-validated.
// New inserts after the upgrade must satisfy the new, stricter schema.
//---------------------------------------------------------------------------------------
TEST_F(JsonDescriptionTest, TighterJsonSchema_ExistingRowsNotRevalidated)
    {
    // Import v1 schema and insert a row valid under the original (x, y) constraint.
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("upgrade_tighter.ecdb", SchemaItem(initialSchema)));

    ECInstanceKey existingKey;
    {
    ECSqlStatement ins;
    ASSERT_EQ(ECSqlStatus::Success, ins.Prepare(m_ecdb, "INSERT INTO ut.GeoCoords ([Value]) VALUES (?)"));
    ASSERT_EQ(ECSqlStatus::Success, ins.BindText(1, R"({"x":1.0,"y":2.0})", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, ins.Step(existingKey));
    }

    // Upgrade to v1.0.1: JsonSchema now also requires "z".
    {
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    ECSchemaPtr v2;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(v2, firstUpdate, *ctx));
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas())) << "Schema upgrade tightening the JsonSchema blob must succeed";
    }

    // Existing row must still be readable
    {
    ECSqlStatement sel;
    ASSERT_EQ(ECSqlStatus::Success, sel.Prepare(m_ecdb, "SELECT [Value] FROM ut.GeoCoords WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, sel.BindInt64(1, (int64_t)existingKey.GetInstanceId().GetValueUnchecked()));
    ASSERT_EQ(BE_SQLITE_ROW, sel.Step()) << "Pre-upgrade row must survive a tightened JsonSchema upgrade unchanged";
    EXPECT_STREQ(R"({"x":1.0,"y":2.0})", sel.GetValueText(0)) << "Stored value must not be modified by the schema upgrade";
    }

    // New INSERT missing "z" must now be rejected.
    {
    ECSqlStatement ins;
    ASSERT_EQ(ECSqlStatus::Success, ins.Prepare(m_ecdb, "INSERT INTO ut.GeoCoords ([Value]) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Error, ins.BindText(1, R"({"x":1.0,"y":2.0})", IECSqlBinder::MakeCopy::No)) << "INSERT missing required 'z' must be rejected after tightening the schema";
    }

    // New INSERT with all three required fields must succeed.
    {
    ECSqlStatement ins;
    ASSERT_EQ(ECSqlStatus::Success, ins.Prepare(m_ecdb, "INSERT INTO ut.GeoCoords ([Value]) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Success, ins.BindText(1, R"({"x":1.0,"y":2.0,"z":3.0})", IECSqlBinder::MakeCopy::No)) << "INSERT satisfying the new tighter schema must succeed";
    }
    }

//---------------------------------------------------------------------------------------
// Upgrade: JsonSchema blob is loosened again (required set shrinks from {x,y} to {x}).
// New inserts after the upgrade use the looser schema; existing rows remain readable.
//---------------------------------------------------------------------------------------
TEST_F(JsonDescriptionTest, LooserJsonSchema_NewInsertsUseNewSchema)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("upgrade_looser.ecdb", SchemaItem(initialSchema)));

    {
    ECSqlStatement ins;
    ASSERT_EQ(ECSqlStatus::Success, ins.Prepare(m_ecdb, "INSERT INTO ut.GeoCoords ([Value]) VALUES (?)"));
    ASSERT_EQ(ECSqlStatus::Success, ins.BindText(1, R"({"x":1.0,"y":2.0})", IECSqlBinder::MakeCopy::No));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, ins.Step(key));
    }

    // Upgrade to v1.0.1: JsonSchema now only requires "x".
    {
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    ECSchemaPtr v2;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(v2, secondUpdate, *ctx));
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas())) << "Schema upgrade loosening the JsonSchema blob must succeed";
    }

    // INSERT with only "x" must now be accepted (y is no longer required).
    {
    ECSqlStatement ins;
    ASSERT_EQ(ECSqlStatus::Success, ins.Prepare(m_ecdb, "INSERT INTO ut.GeoCoords ([Value]) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Success, ins.BindText(1, R"({"x":5.0})", IECSqlBinder::MakeCopy::No)) << "INSERT satisfying the looser schema (only x required) must succeed";
    }

    // Existing row with {x, y} must still be readable.
    {
    ECSqlStatement sel;
    ASSERT_EQ(ECSqlStatus::Success, sel.Prepare(m_ecdb, "SELECT COUNT(*) FROM ut.GeoCoords"));
    ASSERT_EQ(BE_SQLITE_ROW, sel.Step());
    EXPECT_EQ(1, sel.GetValueInt(0)) << "Original row must survive a loosening schema upgrade";
    }
    }

//---------------------------------------------------------------------------------------
// Upgrade: the JsonDescription item is removed from the schema.
// The property survives with JsonDescriptionId NULLed out (ON DELETE SET NULL).
// After the upgrade any well-formed JSON must be accepted for that property.
//---------------------------------------------------------------------------------------
TEST_F(JsonDescriptionTest, PropertyPreservedWithNullReferenceAfterRemoveDescription)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("upgrade_remove_desc.ecdb", SchemaItem(initialSchema)));

    ECInstanceKey existingKey;
    {
    ECSqlStatement ins;
    ASSERT_EQ(ECSqlStatus::Success, ins.Prepare(m_ecdb, "INSERT INTO ut.GeoCoords ([Value]) VALUES (?)"));
    ASSERT_EQ(ECSqlStatus::Success, ins.BindText(1, R"({"x":1.0,"y":2.0})", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, ins.Step(existingKey));
    }

    // Upgrade to v2.0.0: JsonSchema removed, GeoCoords.Value becomes a plain json property.
    {
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    ECSchemaPtr v2;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(v2, firstMajorUpdate, *ctx));
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas())) << "Removing a JsonDescription in a schema upgrade must succeed";
    }

    // ec_Property.JsonDescriptionId must be NULL.
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT JsonDescriptionId FROM ec_Property WHERE Name='Value'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_TRUE(stmt.IsColumnNull(0)) << "JsonDescriptionId must be NULL after the JsonDescription item is removed from the schema";
    }

    // Any well-formed JSON must now be accepted.
    {
    ECSqlStatement ins;
    ASSERT_EQ(ECSqlStatus::Success, ins.Prepare(m_ecdb, "INSERT INTO ut.GeoCoords ([Value]) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Success, ins.BindText(1, R"({"x":1.0})", IECSqlBinder::MakeCopy::No)) << "INSERT missing previously-required fields must succeed after the description is removed";
    ins.Reset(); ins.ClearBindings();
    EXPECT_EQ(ECSqlStatus::Success, ins.BindText(1, R"([1,2,3])", IECSqlBinder::MakeCopy::No)) << "Any valid JSON shape must be accepted once the JsonDescription constraint is gone";
    }

    // Original row must still be readable.
    {
    ECSqlStatement sel;
    ASSERT_EQ(ECSqlStatus::Success, sel.Prepare(m_ecdb, "SELECT [Value] FROM ut.GeoCoords WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, sel.BindInt64(1, (int64_t)existingKey.GetInstanceId().GetValueUnchecked()));
    ASSERT_EQ(BE_SQLITE_ROW, sel.Step()) << "Pre-upgrade row must remain readable after the description is removed";
    }
    }

//---------------------------------------------------------------------------------------
// Deleting a JsonDescription row via SQL sets ec_Property.JsonDescriptionId to NULL.
//---------------------------------------------------------------------------------------
TEST_F(JsonDescriptionTest, DeleteJsonDescriptionRowNullsPropertyReference)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("upgrade_fk_set_null.ecdb", SchemaItem(initialSchema)));

    // Verify the property has a JsonDescriptionId set after import.
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT JsonDescriptionId FROM ec_Property WHERE Name='Value'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_FALSE(stmt.IsColumnNull(0)) << "JsonDescriptionId must be populated after the schema import";
    }

    // Delete the JsonDescription row directly.
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("DELETE FROM ec_JsonDescription WHERE Name='JsonSchema'"));

    // The property row must still exist with a NULL reference.
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT JsonDescriptionId FROM ec_Property WHERE Name='Value'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "ec_Property row must survive deletion of its referenced JsonDescription";
    EXPECT_TRUE(stmt.IsColumnNull(0));
    }
    }

//---------------------------------------------------------------------------------------
// Upgrade: a JsonDescription is added to a property that previously had none.
// Existing rows are not retroactively re-validated.
// New inserts after the upgrade must satisfy the new constraint.
//---------------------------------------------------------------------------------------
TEST_F(JsonDescriptionTest, AddDescription_NewInsertsValidated_ExistingRowsUnaffected)
    {
    // v1: plain unconstrained json property
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("upgrade_add_desc.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="UpgradeTest2" alias="ut2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
        <ECEntityClass typeName="GeoCoords">
            <ECProperty propertyName="Value" typeName="json"/>
        </ECEntityClass>
    </ECSchema>)xml")));

    ECInstanceKey existingKey;
    {
    ECSqlStatement ins;
    ASSERT_EQ(ECSqlStatus::Success, ins.Prepare(m_ecdb, "INSERT INTO ut2.GeoCoords ([Value]) VALUES (?)"));
    // Deliberately insert something that will NOT conform to the JsonSchema we are about to add.
    ASSERT_EQ(ECSqlStatus::Success, ins.BindText(1, R"({"unrelated":"stuff"})", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, ins.Step(existingKey));
    }

    // Upgrade to v1.0.1: add JsonSchema and constrain the property.
    {
    constexpr Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="UpgradeTest2" alias="ut2" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.3">
        <JsonDescription typeName="JsonSchema">
            {"type":"object","properties":{"x":{"type":"number"},"y":{"type":"number"}},"required":["x","y"]}
        </JsonDescription>
        <ECEntityClass typeName="GeoCoords">
            <ECProperty propertyName="Value" typeName="json" jsonDescription="JsonSchema"/>
        </ECEntityClass>
    </ECSchema>)xml";

    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext();
    ctx->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    ECSchemaPtr v2;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(v2, schemaXml, *ctx));
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas())) << "Adding a JsonDescription to a property in a schema upgrade must succeed";
    }

    // ec_Property.JsonDescriptionId must now be populated.
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT JsonDescriptionId FROM ec_Property WHERE Name='Value'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_FALSE(stmt.IsColumnNull(0)) << "JsonDescriptionId must be set after a JsonDescription is added to the property";
    }

    // Pre-upgrade row (which does NOT conform to the new schema) must still be readable.
    {
    ECSqlStatement sel;
    ASSERT_EQ(ECSqlStatus::Success, sel.Prepare(m_ecdb, "SELECT [Value] FROM ut2.GeoCoords WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, sel.BindInt64(1, (int64_t)existingKey.GetInstanceId().GetValueUnchecked()));
    ASSERT_EQ(BE_SQLITE_ROW, sel.Step()) << "Pre-upgrade row must remain readable even if it does not conform to the new constraint";
    EXPECT_STREQ(R"({"unrelated":"stuff"})", sel.GetValueText(0)) << "Stored value must not be altered by adding a JsonDescription constraint";
    }

    // New INSERT that violates the new JsonSchema must now be rejected.
    {
    ECSqlStatement ins;
    ASSERT_EQ(ECSqlStatus::Success, ins.Prepare(m_ecdb, "INSERT INTO ut2.GeoCoords ([Value]) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Error, ins.BindText(1, R"({"unrelated":"stuff"})", IECSqlBinder::MakeCopy::No)) << "INSERT violating the new JsonSchema must be rejected after the description is added";
    }

    // New INSERT that satisfies the new JsonSchema must succeed.
    {
    ECSqlStatement ins;
    ASSERT_EQ(ECSqlStatus::Success, ins.Prepare(m_ecdb, "INSERT INTO ut2.GeoCoords ([Value]) VALUES (?)"));
    EXPECT_EQ(ECSqlStatus::Success, ins.BindText(1, R"({"x":1.0,"y":2.0})", IECSqlBinder::MakeCopy::No)) << "INSERT satisfying the new JsonSchema must succeed";
    }
    }

// =====================================================================================
// Import-time warnings for unconstrained json properties
// =====================================================================================

//---------------------------------------------------------------------------------------
// Importing a schema that contains a json property WITHOUT a jsonDescription
// must emit a Warning.
//---------------------------------------------------------------------------------------
TEST_F(JsonDescriptionTest, ImportUnconstrainedJsonProperty_EmitsWarning)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("import_warn_unconstrained.ecdb"));

    TestIssueListener issues;
    m_ecdb.AddIssueListener(issues);

    // baseSchema has one json property (JsonProperty) with no jsonDescription.
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(baseSchema)));
    auto warnings = issues.GetIssuesBySeverity(IssueSeverity::Warning);
    ASSERT_EQ(1u, warnings.size());
    EXPECT_NE(Utf8String::npos, warnings[0].find("has no jsonDescription")) << warnings[0];
    }

TEST_F(JsonDescriptionTest, Import_MixedJsonProperties_EmitsWarning)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("import_warn_mixed.ecdb"));

    TestIssueListener issues;
    m_ecdb.AddIssueListener(issues);

    // schemaWithJsonDescription has one constrained (JsonProperty) and one unconstrained (AnyJson) json property.
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(schemaWithJsonDescription)));

    auto warnings = issues.GetIssuesBySeverity(IssueSeverity::Warning);
    ASSERT_EQ(1u, warnings.size());
    EXPECT_NE(Utf8String::npos, warnings[0].find("has no jsonDescription")) << warnings[0];
    }

END_ECDBUNITTESTS_NAMESPACE
