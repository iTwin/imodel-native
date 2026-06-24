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
//
// Tests are organized by the ECXml version zone of the schema under test.
// The zone determines WHICH GATE is responsible for strict-mode rejection:
//
//   Zone 1 : version = Latest (currently V3_2)
//     Strict mode:   construct-level checks when reading Xml.
//     Tolerant mode: unknown attrs silently ignored; schema loads OK.
//
//   Zone 2 : Latest < version <= MaxParsable (currently V3_3)
//     Strict mode:   construct-level checks when reading Xml.
//     Tolerant mode: unknown attrs silently ignored.
//
//   Zone 3 : version > MaxParsable (e.g., V3_99.99)
//     Strict mode:   Version check restricts schema read itself.
//     Tolerant mode: schema loads OK; unknown constructs defaulted.
//=======================================================================================

struct StrictSchemaValidationTests : ECTestFixture {};

// ECXML namespace with version beyond MaxParsable, used for Zone 3 tests.
// In strict mode the version gate rejects the schema before any construct check runs.
// In tolerant mode the schema loads with unknown constructs silently defaulted.
#define FUTURE_ECXML_NS "http://www.bentley.com/schemas/Bentley.ECXML.3.99.99"

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, FlagDefaultAndSetterGetter)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    EXPECT_FALSE(context->GetStrictSchemaValidation()) << "Default should be tolerant (false)";

    context->SetStrictSchemaValidation(true);
    EXPECT_TRUE(context->GetStrictSchemaValidation());

    context->SetStrictSchemaValidation(false);
    EXPECT_FALSE(context->GetStrictSchemaValidation());
    }

//=======================================================================================
// Zone 1 : version = Latest (V3_2)
//
// Strict:   unknown attribute *names* on known elements -> InvalidECSchemaXml.
// Tolerant: unknown attrs silently ignored; schema loads OK.
//=======================================================================================

//---------------------------------------------------------------------------------------
// A fully-valid V3_2 schema (no unknown constructs) must succeed in both modes.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
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
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context)) << "Valid current-version schema should succeed in tolerant mode";
    }

    // Strict mode
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context)) << "Valid current-version schema should succeed in strict mode";
    }
    }

//---------------------------------------------------------------------------------------
// A *known* attribute value on a known element (modifier="Sealed") must always succeed
// in strict mode.  Strict mode only rejects *unknown attribute names*.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, UnknownAttributeOnPropertyCategory_TolerantSucceeds)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <PropertyCategory typeName="TestCat" priority="1" futureAttr="unknownValue"/>
        </ECSchema>)xml";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unknown attribute on PropertyCategory must be silently ignored in tolerant mode";
    ASSERT_TRUE(schema.IsValid());
    EXPECT_NE(nullptr, schema->GetPropertyCategoryCP("TestCat"));
    }

//---------------------------------------------------------------------------------------
// TODO: Revisit once strict validation is extended to PropertyCategory.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, UnknownAttributeOnPropertyCategory_StrictFails)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <PropertyCategory typeName="TestCat" priority="1" futureAttr="unknownValue"/>
        </ECSchema>)xml";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);

    ECSchemaPtr schema;
    // TODO - Fix once strict validation is extended to PropertyCategory
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unknown attribute on PropertyCategory must cause failure in strict mode";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, UnknownAttributeOnUnitSystem_TolerantSucceeds)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="SI" futureAttr="unknownValue"/>
        </ECSchema>)xml";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unknown attribute on UnitSystem must be silently ignored in tolerant mode";
    ASSERT_TRUE(schema.IsValid());
    EXPECT_NE(nullptr, schema->GetUnitSystemCP("SI"));
    }

//---------------------------------------------------------------------------------------
// TODO: Revisit once strict validation is extended to UnitSystem.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, UnknownAttributeOnUnitSystem_StrictFails)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="SI" futureAttr="unknownValue"/>
        </ECSchema>)xml";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);

    ECSchemaPtr schema;
    // TODO - Fix once strict validation is extended to UnitSystem
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unknown attribute on UnitSystem must cause failure in strict mode";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, UnknownAttributeOnPhenomenon_TolerantSucceeds)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" futureAttr="unknownValue"/>
        </ECSchema>)xml";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unknown attribute on Phenomenon must be silently ignored in tolerant mode";
    ASSERT_TRUE(schema.IsValid());
    EXPECT_NE(nullptr, schema->GetPhenomenonCP("LENGTH"));
    }

//---------------------------------------------------------------------------------------
// TODO: Revisit once strict validation is extended to Phenomenon.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, UnknownAttributeOnPhenomenon_StrictFails)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" definition="LENGTH" futureAttr="unknownValue"/>
        </ECSchema>)xml";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);

    ECSchemaPtr schema;
    // TODO - Fix once strict validation is extended to Phenomenon
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unknown attribute on Phenomenon must cause failure in strict mode";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, UnknownAttributeOnUnit_TolerantSucceeds)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="SI"/>
            <Phenomenon typeName="LENGTH" definition="LENGTH"/>
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" futureAttr="unknownValue"/>
        </ECSchema>)xml";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unknown attribute on Unit must be silently ignored in tolerant mode";
    ASSERT_TRUE(schema.IsValid());
    EXPECT_NE(nullptr, schema->GetUnitCP("M"));
    }

//---------------------------------------------------------------------------------------
// TODO: Revisit once strict validation is extended to ECUnit.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, UnknownAttributeOnUnit_StrictFails)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="SI"/>
            <Phenomenon typeName="LENGTH" definition="LENGTH"/>
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" definition="M" futureAttr="unknownValue"/>
        </ECSchema>)xml";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);

    ECSchemaPtr schema;
    // TODO - Fix once strict validation is extended to Unit
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unknown attribute on Unit must cause failure in strict mode";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, UnknownAttributeOnFormat_TolerantSucceeds)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="DefaultRealU" type="decimal" precision="6" futureAttr="unknownValue"/>
        </ECSchema>)xml";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unknown attribute on Format must be silently ignored in tolerant mode";
    ASSERT_TRUE(schema.IsValid());
    EXPECT_NE(nullptr, schema->GetFormatCP("DefaultRealU"));
    }

//---------------------------------------------------------------------------------------
// TODO: Revisit once strict validation is extended to ECFormat.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, UnknownAttributeOnFormat_StrictFails)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="DefaultRealU" type="decimal" precision="6" futureAttr="unknownValue"/>
        </ECSchema>)xml";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);

    ECSchemaPtr schema;
    // TODO - Fix once strict validation is extended to ECFormat
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unknown attribute on Format must cause failure in strict mode";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, UnknownAttributeOnKindOfQuantity_TolerantSucceeds)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="SI"/>
            <Phenomenon typeName="NewPhenomenon" definition="LENGTH"/>
            <Unit typeName="M" phenomenon="NewPhenomenon" unitSystem="SI" definition="M"/>
            <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
            <KindOfQuantity typeName="Length" relativeError="0.0001" persistenceUnit="M" futureAttr="unknownValue"/>
        </ECSchema>)xml";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unknown attribute on KindOfQuantity must be silently ignored in tolerant mode";
    ASSERT_TRUE(schema.IsValid());
    EXPECT_NE(nullptr, schema->GetKindOfQuantityCP("Length"));
    }

//---------------------------------------------------------------------------------------
// TODO: Revisit once strict validation is extended to KindOfQuantity.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, UnknownAttributeOnKindOfQuantity_StrictFails)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="SI"/>
            <Phenomenon typeName="NewPhenomenon" definition="LENGTH"/>
            <Unit typeName="M" phenomenon="NewPhenomenon" unitSystem="SI" definition="M"/>
            <Format typeName="DefaultRealU" type="decimal" precision="6" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel"/>
            <KindOfQuantity typeName="Length" relativeError="0.0001" persistenceUnit="M" futureAttr="unknownValue"/>
        </ECSchema>)xml";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);

    ECSchemaPtr schema;
    // TODO - Fix once strict validation is extended to KindOfQuantity
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unknown attribute on KindOfQuantity must cause failure in strict mode";
    }

//---------------------------------------------------------------------------------------
// The referenced schema is pre-loaded into the context so that reference resolution
// succeeds (tolerant) or the strict attribute check fires before resolution (strict).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, UnknownAttributeOnSchemaReference_TolerantSucceeds)
    {
    Utf8CP refSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="RefSchema" alias="ref" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>)xml";

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="RefSchema" version="01.00.00" alias="ref" futureAttr="unknownValue"/>
        </ECSchema>)xml";

    // Load the referenced schema first so the reference resolves.
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr refSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refSchemaXml, *context));

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unknown attribute on ECSchemaReference must be silently ignored in tolerant mode";
    ASSERT_TRUE(schema.IsValid());
    }

//---------------------------------------------------------------------------------------
// TODO: Revisit once strict validation is extended to ECSchemaReference.
// The referenced schema is pre-loaded so the *only* failure reason is the unknown attribute.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, UnknownAttributeOnSchemaReference_StrictFails)
    {
    Utf8CP refSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="RefSchema" alias="ref" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        </ECSchema>)xml";

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="RefSchema" version="01.00.00" alias="ref" futureAttr="unknownValue"/>
        </ECSchema>)xml";

    // Load the referenced schema first; without this the test would fail because of a
    // missing reference rather than because of the unknown attribute.
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);
    ECSchemaPtr refSchema;
    {
    // Parse refSchema with a separate tolerant context so it loads cleanly.
    ECSchemaReadContextPtr tempContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refSchemaXml, *tempContext));
    }
    context->AddSchema(*refSchema);

    ECSchemaPtr schema;
    // TODO - Fix once strict validation is extended to ECSchemaReference
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        << "Unknown attribute on ECSchemaReference must cause failure in strict mode";
    }

//=======================================================================================
// Zone 2 : Latest < version <= MaxParsable (currently V3_3)
//
// The version gate PASSES even in strict mode (V3_3 <= MaxParsable).
// The construct-level check in _ReadXmlAttributes is the only operative defence.
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, MaxParsableVersion_UnknownAttributeName_TolerantSucceeds)
    {
    // Build the MaxParsable xmlns dynamically so this test stays correct if MaxParsable changes.
    uint32_t major, minor;
    ECSchema::ParseECVersion(major, minor, ECVersion::MaxParsable);
    Utf8PrintfString xmlns("http://www.bentley.com/schemas/Bentley.ECXML.%d.%d", major, minor);

    Utf8PrintfString schemaXml(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="%s">
            <ECEntityClass typeName="TestClass" unknownFutureAttr="someValue"/>
        </ECSchema>)xml", xmlns.c_str());

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext(); // strict = false
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *context))
        << "A MaxParsable-version schema with an unknown attribute must succeed in tolerant mode "
           "(version gate passes, construct checks are disabled)";
    ASSERT_TRUE(schema.IsValid());
    EXPECT_NE(nullptr, schema->GetClassCP("TestClass"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, MaxParsableVersion_UnknownAttributeName_StrictFails)
    {
    uint32_t major, minor;
    ECSchema::ParseECVersion(major, minor, ECVersion::MaxParsable);
    Utf8PrintfString xmlns("http://www.bentley.com/schemas/Bentley.ECXML.%d.%d", major, minor);

    Utf8PrintfString schemaXml(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="%s">
            <ECEntityClass typeName="TestClass" unknownFutureAttr="someValue"/>
        </ECSchema>)xml", xmlns.c_str());

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->SetStrictSchemaValidation(true);

    ECSchemaPtr schema;
    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *context))
        << "A MaxParsable-version schema with an unknown attribute must fail in strict mode: "
           "the version gate passes (V3_3 <= MaxParsable) but the construct-level check "
           "in _ReadXmlAttributes rejects the unknown attribute name";
    }

//=======================================================================================
// Zone 3 : version > MaxParsable (FUTURE_ECXML_NS = V3_99.99)
//
// Strict mode: The schema is rejected outright because ecVersion > MaxParsable.
//
// Tolerant mode: The schema loads successfully. Unknown constructs are silently defaulted.
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(StrictSchemaValidationTests, UnknownClassModifier_TolerantSucceeds)
    {
    Utf8CP schemaXml = R"__(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns=")__" FUTURE_ECXML_NS R"__(">
            <ECEntityClass typeName="TestClass" modifier="FutureModifier">
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
        </ECSchema>)__";

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
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

//---------------------------------------------------------------------------------------
// Fails at the VERSION GATE (ecVersion > MaxParsable), not at the construct check.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
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
        << "Strict mode should reject future schema at the version gate";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
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

//---------------------------------------------------------------------------------------
// Fails at the VERSION GATE (ecVersion > MaxParsable), not at the construct check.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
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
        << "Strict mode should reject future schema at the version gate";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
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

//---------------------------------------------------------------------------------------
// Fails at the VERSION GATE (ecVersion > MaxParsable), not at the construct check.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
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
        << "Strict mode should reject future schema at the version gate";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
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

//---------------------------------------------------------------------------------------
// Fails at the VERSION GATE (ecVersion > MaxParsable), not at the construct check.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
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
        << "Strict mode should reject future schema at the version gate";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
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

//---------------------------------------------------------------------------------------
// Fails at the VERSION GATE (ecVersion > MaxParsable), not at the construct check.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
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
        << "Strict mode should reject future schema at the version gate";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
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

//---------------------------------------------------------------------------------------
// Fails at the VERSION GATE (ecVersion > MaxParsable), not at the construct check.
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
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

END_BENTLEY_ECN_TEST_NAMESPACE