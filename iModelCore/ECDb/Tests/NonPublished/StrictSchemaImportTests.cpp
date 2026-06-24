/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Tests for ECDb schema import version-gate behavior.
//
// Tests are organized by the ECXml version zone of the schema under test:
//
//   Zone 1 : version = Latest (V3_2)
//     ECDb import gate:   PASSES (V3_2 <= Latest).
//     Construct checks:   operative - the ecobjects parser rejects unknown construct
//                         values even in tolerant mode for V3_2 range schemas.
//
//   Zone 2 : Latest < version <= MaxParsable (V3_3)
//     ECDb import gate:   Fails (V3_3 > Latest = V3_2).
//
//   Zone 3 : version > MaxParsable (V3_99.99)
//     ECDb import gate:   Fails (V3_99.99 > Latest = V3_2).
//=======================================================================================

struct StrictSchemaImportTests : ECDbTestFixture {};

//=======================================================================================
// Zone 1 : version = Latest (V3_2)
//=======================================================================================

//---------------------------------------------------------------------------------------
// A fully-valid V3_2 schema must import successfully regardless of what follows.
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

//=======================================================================================
// Zone 2 : Latest < version <= MaxParsable (V3_3)
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaImportTests, MaxParsableVersion_ValidSchema_ImportRejected)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("strictimport_maxparsable.ecdb"));

    // Build the MaxParsable xmlns dynamically so this test stays correct if MaxParsable changes.
    uint32_t major, minor;
    ECSchema::ParseECVersion(major, minor, ECVersion::MaxParsable);
    Utf8PrintfString xmlns("http://www.bentley.com/schemas/Bentley.ECXML.%d.%d", major, minor);

    Utf8PrintfString schemaXml(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="MaxParsableSchema" alias="mp" version="01.00.00" xmlns="%s">
            <ECEntityClass typeName="TestClass" modifier="None">
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", xmlns.c_str());

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(schemaXml.c_str())))
        << "ECDb should reject a V3_3 schema at the import gate (V3_3 > Latest=V3_2); "
           "this gate will be relaxed to MaxParsable when Milestone 3 lands";
    }

//=======================================================================================
// Zone 3 : version > MaxParsable (V3_99.99)
//=======================================================================================

//---------------------------------------------------------------------------------------
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
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(StrictSchemaImportTests, FutureVersionSchemaWithOnlyKnownConstructs_StillRejected)
    {
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDb("strictimport_futureknown.ecdb"));

    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="FutureButValid" alias="fv" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.99.99">
            <ECEntityClass typeName="TestClass" modifier="None">
                <ECProperty propertyName="Prop1" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml";

    EXPECT_EQ(ERROR, ImportSchema(SchemaItem(schemaXml)))
        << "ECDb should reject any schema with future ECXml version, even if all constructs are known";
    }

END_ECDBUNITTESTS_NAMESPACE