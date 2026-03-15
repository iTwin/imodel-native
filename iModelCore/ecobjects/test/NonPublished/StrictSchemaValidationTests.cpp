/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

//=======================================================================================
// Tests for strict schema validation mode on ECSchemaReadContext.
// In tolerant mode (default), unknown constructs from schemas with ECXml version
// greater than Latest are silently accepted with defaults.
// In strict mode, these cause parse failures.
//
// All test schemas use xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.99.99"
// which is greater than the current Latest (3.2).
//
// Note: Raw string delimiter is __ (not xml) so that )__" terminates correctly
// at the xmlns concatenation point. Using xml as delimiter would cause )xml" to
// only match at </ECSchema>)xml", swallowing the macro as literal text.
//=======================================================================================

struct StrictSchemaValidationTests : ECTestFixture {};

// ECXML namespace that is newer than Latest (3.2)
#define FUTURE_ECXML_NS "http://www.bentley.com/schemas/Bentley.ECXML.3.99.99"

//---------------------------------------------------------------------------------------
// Unknown class modifier: tolerant mode should accept and default to None;
// strict mode should reject.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaValidationTests, UnknownClassModifier_TolerantSucceeds)
    {
    Utf8CP schemaXml = R"__(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns=")__" FUTURE_ECXML_NS R"__(">
            <ECEntityClass typeName="TestClass" modifier="FutureModifier">
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
        </ECSchema>)__";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    // Default is tolerant (strict = false)
    ASSERT_FALSE(context->GetStrictSchemaValidation());

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Tolerant mode should accept unknown class modifier from future schema";
    ASSERT_TRUE(schema.IsValid());

    ECClassCP testClass = schema->GetClassCP("TestClass");
    ASSERT_NE(nullptr, testClass);
    EXPECT_EQ(ECClassModifier::None, testClass->GetClassModifier())
        << "Unknown modifier should default to None in tolerant mode";
    }

TEST_F(StrictSchemaValidationTests, UnknownClassModifier_StrictFails)
    {
    Utf8CP schemaXml = R"__(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns=")__" FUTURE_ECXML_NS R"__(">
            <ECEntityClass typeName="TestClass" modifier="FutureModifier">
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
        </ECSchema>)__";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);

    ECSchemaPtr schema;
    EXPECT_NE(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Strict mode should reject unknown class modifier from future schema";
    }

//---------------------------------------------------------------------------------------
// Known modifier on a current-version schema should always succeed regardless of mode.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaValidationTests, KnownClassModifier_StrictSucceeds)
    {
    Utf8CP schemaXml = R"__(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="TestClass" modifier="Sealed">
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
        </ECSchema>)__";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Known modifier should always succeed even in strict mode";
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(ECClassModifier::Sealed, schema->GetClassCP("TestClass")->GetClassModifier());
    }

//---------------------------------------------------------------------------------------
// Unknown relationship strength: tolerant mode should accept and default to Referencing;
// strict mode should reject.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaValidationTests, UnknownRelationshipStrength_TolerantSucceeds)
    {
    Utf8CP schemaXml = R"__(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns=")__" FUTURE_ECXML_NS R"__(">
            <ECEntityClass typeName="SourceClass" modifier="None">
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="TargetClass" modifier="None">
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
            <ECRelationshipClass typeName="TestRel" strength="FutureStrength" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..*)" roleLabel="source" polymorphic="true">
                    <Class class="SourceClass" />
                </Source>
                <Target multiplicity="(0..*)" roleLabel="target" polymorphic="true">
                    <Class class="TargetClass" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)__";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Tolerant mode should accept unknown relationship strength from future schema";
    ASSERT_TRUE(schema.IsValid());

    ECClassCP relClass = schema->GetClassCP("TestRel");
    ASSERT_NE(nullptr, relClass);
    ASSERT_TRUE(relClass->IsRelationshipClass());
    EXPECT_EQ(StrengthType::Referencing, relClass->GetRelationshipClassCP()->GetStrength())
        << "Unknown strength should default to Referencing in tolerant mode";
    }

TEST_F(StrictSchemaValidationTests, UnknownRelationshipStrength_StrictFails)
    {
    Utf8CP schemaXml = R"__(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns=")__" FUTURE_ECXML_NS R"__(">
            <ECEntityClass typeName="SourceClass" modifier="None">
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="TargetClass" modifier="None">
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
            <ECRelationshipClass typeName="TestRel" strength="FutureStrength" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..*)" roleLabel="source" polymorphic="true">
                    <Class class="SourceClass" />
                </Source>
                <Target multiplicity="(0..*)" roleLabel="target" polymorphic="true">
                    <Class class="TargetClass" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)__";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);

    ECSchemaPtr schema;
    EXPECT_NE(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Strict mode should reject unknown relationship strength from future schema";
    }

//---------------------------------------------------------------------------------------
// Unknown enumeration backing type: tolerant mode should accept and default to String;
// strict mode should reject.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaValidationTests, UnknownEnumBackingType_TolerantSucceeds)
    {
    Utf8CP schemaXml = R"__(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns=")__" FUTURE_ECXML_NS R"__(">
            <ECEnumeration typeName="TestEnum" backingTypeName="FutureType" isStrict="true">
                <ECEnumerator name="Val1" value="1" displayLabel="Value One" />
            </ECEnumeration>
        </ECSchema>)__";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Tolerant mode should accept unknown enum backing type from future schema";
    ASSERT_TRUE(schema.IsValid());

    ECEnumerationCP testEnum = schema->GetEnumerationCP("TestEnum");
    ASSERT_NE(nullptr, testEnum);
    EXPECT_EQ(PRIMITIVETYPE_String, testEnum->GetType())
        << "Unknown enum backing type should default to String in tolerant mode";
    }

TEST_F(StrictSchemaValidationTests, UnknownEnumBackingType_StrictFails)
    {
    Utf8CP schemaXml = R"__(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns=")__" FUTURE_ECXML_NS R"__(">
            <ECEnumeration typeName="TestEnum" backingTypeName="FutureType" isStrict="true">
                <ECEnumerator name="Val1" value="1" displayLabel="Value One" />
            </ECEnumeration>
        </ECSchema>)__";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);

    ECSchemaPtr schema;
    EXPECT_NE(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Strict mode should reject unknown enum backing type from future schema";
    }

//---------------------------------------------------------------------------------------
// Unknown child element in enumeration: tolerant mode should skip it;
// strict mode should reject.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaValidationTests, UnknownEnumChildElement_TolerantSucceeds)
    {
    Utf8CP schemaXml = R"__(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns=")__" FUTURE_ECXML_NS R"__(">
            <ECEnumeration typeName="TestEnum" backingTypeName="int" isStrict="true">
                <FutureElement name="Foo" value="99" />
                <ECEnumerator name="Val1" value="1" displayLabel="Value One" />
            </ECEnumeration>
        </ECSchema>)__";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Tolerant mode should skip unknown child elements in enumerations from future schema";
    ASSERT_TRUE(schema.IsValid());

    ECEnumerationCP testEnum = schema->GetEnumerationCP("TestEnum");
    ASSERT_NE(nullptr, testEnum);
    EXPECT_EQ(1, testEnum->GetEnumeratorCount())
        << "Only the known ECEnumerator should be parsed, unknown child elements skipped";
    }

TEST_F(StrictSchemaValidationTests, UnknownEnumChildElement_StrictFails)
    {
    Utf8CP schemaXml = R"__(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns=")__" FUTURE_ECXML_NS R"__(">
            <ECEnumeration typeName="TestEnum" backingTypeName="int" isStrict="true">
                <FutureElement name="Foo" value="99" />
                <ECEnumerator name="Val1" value="1" displayLabel="Value One" />
            </ECEnumeration>
        </ECSchema>)__";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);

    ECSchemaPtr schema;
    EXPECT_NE(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Strict mode should reject unknown child elements in enumerations from future schema";
    }

//---------------------------------------------------------------------------------------
// Unknown property type: tolerant mode should default to string;
// strict mode should reject.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaValidationTests, UnknownPropertyType_TolerantSucceeds)
    {
    Utf8CP schemaXml = R"__(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns=")__" FUTURE_ECXML_NS R"__(">
            <ECEntityClass typeName="TestClass" modifier="None">
                <ECProperty propertyName="FutureProp" typeName="FuturePrimitiveType" />
            </ECEntityClass>
        </ECSchema>)__";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Tolerant mode should accept unknown property type from future schema";
    ASSERT_TRUE(schema.IsValid());

    ECClassCP testClass = schema->GetClassCP("TestClass");
    ASSERT_NE(nullptr, testClass);
    ECPropertyP prop = testClass->GetPropertyP("FutureProp");
    ASSERT_NE(nullptr, prop);
    ASSERT_TRUE(prop->GetIsPrimitive());
    EXPECT_EQ(PRIMITIVETYPE_String, prop->GetAsPrimitiveProperty()->GetType())
        << "Unknown property type should default to string in tolerant mode";
    }

TEST_F(StrictSchemaValidationTests, UnknownPropertyType_StrictFails)
    {
    Utf8CP schemaXml = R"__(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns=")__" FUTURE_ECXML_NS R"__(">
            <ECEntityClass typeName="TestClass" modifier="None">
                <ECProperty propertyName="FutureProp" typeName="FuturePrimitiveType" />
            </ECEntityClass>
        </ECSchema>)__";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);

    ECSchemaPtr schema;
    EXPECT_NE(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Strict mode should reject unknown property type from future schema";
    }

//---------------------------------------------------------------------------------------
// Comprehensive test: a schema with multiple unknown constructs.
// Tolerant mode should succeed; strict mode should fail on first unknown.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaValidationTests, MultipleUnknownConstructs_TolerantSucceeds)
    {
    Utf8CP schemaXml = R"__(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns=")__" FUTURE_ECXML_NS R"__(">
            <ECEnumeration typeName="TestEnum" backingTypeName="FutureType" isStrict="true">
                <FutureElement name="Foo" value="99" />
                <ECEnumerator name="Val1" value="1" displayLabel="Value One" />
            </ECEnumeration>
            <ECEntityClass typeName="TestClass" modifier="FutureModifier">
                <ECProperty propertyName="KnownProp" typeName="string" />
                <ECProperty propertyName="FutureProp" typeName="FuturePrimitiveType" />
            </ECEntityClass>
        </ECSchema>)__";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Tolerant mode should accept a schema with multiple unknown constructs from future schema";
    ASSERT_TRUE(schema.IsValid());
    EXPECT_EQ(1, schema->GetClassCount());
    EXPECT_EQ(1, schema->GetEnumerationCount());
    }

TEST_F(StrictSchemaValidationTests, MultipleUnknownConstructs_StrictFails)
    {
    Utf8CP schemaXml = R"__(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns=")__" FUTURE_ECXML_NS R"__(">
            <ECEnumeration typeName="TestEnum" backingTypeName="FutureType" isStrict="true">
                <FutureElement name="Foo" value="99" />
                <ECEnumerator name="Val1" value="1" displayLabel="Value One" />
            </ECEnumeration>
            <ECEntityClass typeName="TestClass" modifier="FutureModifier">
                <ECProperty propertyName="KnownProp" typeName="string" />
                <ECProperty propertyName="FutureProp" typeName="FuturePrimitiveType" />
            </ECEntityClass>
        </ECSchema>)__";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);

    ECSchemaPtr schema;
    EXPECT_NE(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Strict mode should reject a schema with unknown constructs from future schema";
    }

//---------------------------------------------------------------------------------------
// Current-version schema with all valid constructs should pass in both modes.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaValidationTests, ValidCurrentVersionSchema_BothModesSucceed)
    {
    Utf8CP schemaXml = R"__(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="TestEnum" backingTypeName="int" isStrict="true">
                <ECEnumerator name="Val1" value="1" displayLabel="Value One" />
                <ECEnumerator name="Val2" value="2" displayLabel="Value Two" />
            </ECEnumeration>
            <ECEntityClass typeName="SourceClass" modifier="None">
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="TargetClass" modifier="Sealed">
                <ECProperty propertyName="Prop1" typeName="int" />
            </ECEntityClass>
            <ECRelationshipClass typeName="TestRel" strength="embedding" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..*)" roleLabel="source" polymorphic="true">
                    <Class class="SourceClass" />
                </Source>
                <Target multiplicity="(0..*)" roleLabel="target" polymorphic="true">
                    <Class class="TargetClass" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)__";

    // Tolerant mode
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Valid current-version schema should succeed in tolerant mode";
    }

    // Strict mode
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Valid current-version schema should succeed in strict mode";
    }
    }

//---------------------------------------------------------------------------------------
// StrictSchemaValidation flag default value and setter/getter.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaValidationTests, FlagDefaultAndSetterGetter)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    EXPECT_FALSE(context->GetStrictSchemaValidation())
        << "Default should be tolerant (false)";

    context->SetStrictSchemaValidation(true);
    EXPECT_TRUE(context->GetStrictSchemaValidation());

    context->SetStrictSchemaValidation(false);
    EXPECT_FALSE(context->GetStrictSchemaValidation());
    }

END_BENTLEY_ECN_TEST_NAMESPACE
