/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Tests for strict schema validation during ECDb import.
// ECDb's SchemaWriter sets strict schema validation on its ECSchemaReadContext,
// and additionally rejects schemas with OriginalECXmlVersion > Latest.
//
// IMPORTANT: All tests using xmlns 3.99.99 hit ECDb's version gate
// (OriginalECXmlVersion > Latest) before any construct-level validation runs.
// The ecobjects parser reads tolerantly during initial parse (the test helper's
// ECSchemaReadContext defaults to strict=false), silently defaulting unknown
// constructs. SchemaWriter then rejects the schema purely because of the future
// version. The per-construct tests remain as regression coverage: if the version
// gate is ever relaxed, they would start passing and signal that construct-level
// strict validation needs its own dedicated coverage.
//=======================================================================================

struct StrictSchemaImportTests : ECDbTestFixture {};

//---------------------------------------------------------------------------------------
// A future-version schema with unknown constructs should fail to import into ECDb.
// NOTE: Rejected at the version gate (OriginalECXmlVersion > Latest), not at the
// construct level. The unknown modifier is silently defaulted during tolerant parse.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaImportTests, FutureVersionSchemaWithUnknownModifier_Rejected)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("strictimport_modifier.ecdb"));

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FutureSchema" alias="fs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.99.99">
            <ECEntityClass typeName="TestClass" modifier="FutureModifier">
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(schemaXml)))
        << "ECDb should reject a schema with future ECXml version containing unknown constructs";
    }

//---------------------------------------------------------------------------------------
// A future-version schema with an unknown relationship strength should fail to import.
// NOTE: Rejected at the version gate, not at the construct level.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaImportTests, FutureVersionSchemaWithUnknownStrength_Rejected)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("strictimport_strength.ecdb"));

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FutureSchema" alias="fs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.99.99">
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
        </ECSchema>)xml";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(schemaXml)))
        << "ECDb should reject a schema with future ECXml version containing unknown strength";
    }

//---------------------------------------------------------------------------------------
// A future-version schema with an unknown enum backing type should fail to import.
// NOTE: Rejected at the version gate, not at the construct level.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaImportTests, FutureVersionSchemaWithUnknownEnumType_Rejected)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("strictimport_enum.ecdb"));

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FutureSchema" alias="fs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.99.99">
            <ECEnumeration typeName="TestEnum" backingTypeName="FutureType" isStrict="true">
                <ECEnumerator name="Val1" value="1" displayLabel="Value One" />
            </ECEnumeration>
        </ECSchema>)xml";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(schemaXml)))
        << "ECDb should reject a schema with future ECXml version containing unknown enum backing type";
    }

//---------------------------------------------------------------------------------------
// A future-version schema with an unknown property type should fail to import.
// NOTE: Rejected at the version gate, not at the construct level.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaImportTests, FutureVersionSchemaWithUnknownPropertyType_Rejected)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("strictimport_proptype.ecdb"));

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FutureSchema" alias="fs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.99.99">
            <ECEntityClass typeName="TestClass" modifier="None">
                <ECProperty propertyName="FutureProp" typeName="FuturePrimitiveType" />
            </ECEntityClass>
        </ECSchema>)xml";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(schemaXml)))
        << "ECDb should reject a schema with future ECXml version containing unknown property type";
    }

//---------------------------------------------------------------------------------------
// A valid current-version schema should succeed import regardless of strict mode.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaImportTests, ValidCurrentVersionSchema_Succeeds)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("strictimport_valid.ecdb"));

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="ValidSchema" alias="vs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="TestEnum" backingTypeName="int" isStrict="true">
                <ECEnumerator name="Val1" value="1" displayLabel="Value One" />
            </ECEnumeration>
            <ECEntityClass typeName="SourceClass" modifier="None">
                <ECProperty propertyName="Prop1" typeName="string" />
                <ECProperty propertyName="Prop2" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="TargetClass" modifier="Sealed">
                <ECProperty propertyName="Prop1" typeName="double" />
            </ECEntityClass>
            <ECRelationshipClass typeName="TestRel" strength="embedding" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..*)" roleLabel="source" polymorphic="true">
                    <Class class="SourceClass" />
                </Source>
                <Target multiplicity="(0..*)" roleLabel="target" polymorphic="true">
                    <Class class="TargetClass" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    EXPECT_EQ(SUCCESS, ImportSchema(SchemaItem(schemaXml)))
        << "A valid current-version schema should import successfully";
    }

//---------------------------------------------------------------------------------------
// A future-version schema that only has known constructs should still be rejected
// by ECDb because of the version check (OriginalECXmlVersion > Latest).
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaImportTests, FutureVersionSchemaWithOnlyKnownConstructs_StillRejected)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("strictimport_futureknown.ecdb"));

    // This schema has a future xmlns but only known constructs.
    // ECDb should still reject it because of the OriginalECXmlVersion > Latest check.
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FutureButValid" alias="fv" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.99.99">
            <ECEntityClass typeName="TestClass" modifier="None">
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(schemaXml)))
        << "ECDb should reject any schema with future ECXml version, even if all constructs are known";
    }

//=======================================================================================
// Current-version (3.2) schemas with unknown constructs.
// For 3.2, OriginalECXmlVersionGreaterThan(Latest) is false, so no tolerance path
// is entered. Unknown constructs are rejected during XML parse by the ecobjects layer,
// before SchemaWriter is even reached. These tests verify construct-level rejection.
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaImportTests, CurrentVersionSchemaWithUnknownModifier_Rejected)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("strictcurrent_modifier.ecdb"));

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BadModifier" alias="bm" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="TestClass" modifier="FutureModifier">
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(schemaXml)))
        << "Should reject unknown modifier in a 3.2 schema at the construct level";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaImportTests, CurrentVersionSchemaWithUnknownPropertyType_Rejected)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("strictcurrent_proptype.ecdb"));

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BadPropType" alias="bp" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="TestClass" modifier="None">
                <ECProperty propertyName="FutureProp" typeName="FuturePrimitiveType" />
            </ECEntityClass>
        </ECSchema>)xml";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(schemaXml)))
        << "Should reject unknown property type in a 3.2 schema at the construct level";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaImportTests, CurrentVersionSchemaWithUnknownEnumBackingType_Rejected)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("strictcurrent_enum.ecdb"));

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BadEnum" alias="be" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="TestEnum" backingTypeName="FutureType" isStrict="true">
                <ECEnumerator name="Val1" value="1" displayLabel="Value One" />
            </ECEnumeration>
        </ECSchema>)xml";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(schemaXml)))
        << "Should reject unknown enum backing type in a 3.2 schema at the construct level";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaImportTests, CurrentVersionSchemaWithUnknownStrength_Rejected)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("strictcurrent_strength.ecdb"));

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BadStrength" alias="bs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
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
        </ECSchema>)xml";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(schemaXml)))
        << "Should reject unknown relationship strength in a 3.2 schema at the construct level";
    }

END_ECDBUNITTESTS_NAMESPACE
