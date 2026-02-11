/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_EC

struct SchemaRemapTest : DgnDbTestFixture
    {
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTest, SchemaRemapChangeClassHierarchyWithV8LegacyImport)
    {
    SetupSeedProject();
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(m_db->GetSchemaLocater());

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, R"schema(<?xml version='1.0' encoding='utf-8' ?>
      <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" displayLabel="TestSchema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="BisCore" version="01.00.16" alias="bis"/>
        <ECEntityClass typeName="A">
          <BaseClass>bis:PhysicalElement</BaseClass>
          <ECProperty propertyName="PropA" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName="C">
          <BaseClass>A</BaseClass>
          <ECProperty propertyName="PropC" typeName="string" />
        </ECEntityClass>
      </ECSchema>)schema", *context));
    ASSERT_EQ(SchemaStatus::Success, m_db->ImportV8LegacySchemas({ schema.get() }));

    ECSchemaPtr schemaRemapped;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaRemapped, R"schema(<?xml version='1.0' encoding='utf-8' ?>
      <ECSchema schemaName="TestSchema" alias="ts" version="01.01.00" displayLabel="TestSchema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="BisCore" version="01.00.16" alias="bis"/>
        <ECEntityClass typeName="A">
          <BaseClass>bis:PhysicalElement</BaseClass>
          <ECProperty propertyName="PropA" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName="B">
          <BaseClass>A</BaseClass>
          <ECProperty propertyName="PropB" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName="C">
          <BaseClass>B</BaseClass>
          <ECProperty propertyName="PropC" typeName="string" />
        </ECEntityClass>
      </ECSchema>)schema", *context));
    ASSERT_EQ(SchemaStatus::Success, m_db->ImportV8LegacySchemas({ schemaRemapped.get() }));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTest, SchemaRemapMovePropertyWithV8LegacyImport)
    {
    SetupSeedProject();
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(m_db->GetSchemaLocater());

    const auto schemaXml = R"schema(<?xml version='1.0' encoding='utf-8' ?>
      <ECSchema schemaName="TestSchema" alias="ts" version="01.00.%d" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="BisCore" version="01.00.16" alias="bis"/>
        <ECEntityClass typeName="TestClass">
          <BaseClass>bis:PhysicalElement</BaseClass>
          %s
        </ECEntityClass>
        <ECEntityClass typeName="A">
          <BaseClass>TestClass</BaseClass>
          <ECProperty propertyName="MovingProperty" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName="B">
          <BaseClass>TestClass</BaseClass>
          <ECProperty propertyName="PropC1" typeName="string" />
        </ECEntityClass>
      </ECSchema>)schema";

    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, Utf8PrintfString(schemaXml, 0, "").c_str(), *context));
    ASSERT_EQ(SchemaStatus::Success, m_db->ImportV8LegacySchemas({ schema.get() }));

    const auto testSchema = m_db->Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(testSchema);
    const auto testClass = testSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClass);
    ASSERT_FALSE(testClass->GetPropertyP("MovingProperty", false));

    ECSchemaPtr schemaRemapped;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaRemapped, Utf8PrintfString(schemaXml, 1, R"xml(<ECProperty propertyName="MovingProperty" typeName="string"/>)xml").c_str(), *context));
    ASSERT_EQ(SchemaStatus::Success, m_db->ImportV8LegacySchemas({ schemaRemapped.get() }));

    const auto remappedSchema = m_db->Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(remappedSchema);
    const auto testClassUpdated = remappedSchema->GetClassCP("TestClass");
    ASSERT_TRUE(testClassUpdated);
    ASSERT_TRUE(testClassUpdated->GetPropertyP("MovingProperty", false));
    }

TEST_F(SchemaRemapTest, DeletePropertyOverrideAndDerivedClassSimultaneously)
    {
    SetupSeedProject();
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(m_db->GetSchemaLocater());

    // Setup the base schema with necessary classes.
    // The property Alias is defined in the base entity class "Named_Item" and gets overriden in the child "Device".
    const auto schemaV1_0_0 = R"xml(<?xml version="1.0" encoding="utf-8" ?>
      <ECSchema schemaName="TestSchema%d" alias="ts%d" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="BisCore" version="1.0.12" alias="bis"/>
        <ECSchemaReference name="CoreCustomAttributes" version="1.0.3" alias="CoreCA"/>

        <ECCustomAttributes>
          <DynamicSchema xmlns="CoreCustomAttributes.1.0.3"/>
        </ECCustomAttributes>

        <ECEntityClass typeName="Named_Item">
          <BaseClass>bis:PhysicalElement</BaseClass>
          <ECProperty propertyName="Alias" typeName="string" priority="0" />
        </ECEntityClass>

        <ECEntityClass typeName="Device">
          <BaseClass>Named_Item</BaseClass>
          <ECProperty propertyName="Alias" typeName="string" priority="1003" />
        </ECEntityClass>

        <ECEntityClass typeName="Equipment">
          <BaseClass>Device</BaseClass>
          <ECProperty propertyName="SerialNumber" typeName="string" />
        </ECEntityClass>
      </ECSchema>)xml";

    // Minor schema update:
    // Override the property "Alias" in the child class "Equipment" and also add a derived class "SILO" which will inherit the property and add a property map row.
    const auto schemaV1_0_1 = R"xml(<?xml version="1.0" encoding="utf-8" ?>
      <ECSchema schemaName="TestSchema%d" alias="ts%d" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="BisCore" version="1.0.12" alias="bis"/>
        <ECSchemaReference name="CoreCustomAttributes" version="1.0.3" alias="CoreCA"/>

        <ECCustomAttributes>
          <DynamicSchema xmlns="CoreCustomAttributes.1.0.3"/>
        </ECCustomAttributes>

        <ECEntityClass typeName="Named_Item">
          <BaseClass>bis:PhysicalElement</BaseClass>
          <ECProperty propertyName="Alias" typeName="string" priority="0" />
        </ECEntityClass>

        <ECEntityClass typeName="Device">
          <BaseClass>Named_Item</BaseClass>
          <ECProperty propertyName="Alias" typeName="string" priority="1003" />
        </ECEntityClass>

        <ECEntityClass typeName="Equipment">
          <BaseClass>Device</BaseClass>
          <ECProperty propertyName="SerialNumber" typeName="string" />
          <ECProperty propertyName="Alias" typeName="string" priority="0" />
        </ECEntityClass>
          
        <ECEntityClass typeName="SILO">
          <BaseClass>Equipment</BaseClass>
          <ECProperty propertyName="Height" typeName="double" />
        </ECEntityClass>
      </ECSchema>)xml";

    // Major schema upgrade that deletes the overriden property "Alias" from class "Equipment" and also deletes the leaf class "Silo".
    // Since the property map for alias contains a property path to "Silo", it is persisted.
    // However, since the class "Silo" itself gets deleted, we end up with an error "Failed to find class SILO for ensuring persisted property maps." when remapping the persisted property maps.
    const auto schemaV2_0_0 = R"xml(<?xml version="1.0" encoding="utf-8" ?>
      <ECSchema schemaName="TestSchema%d" alias="ts%d" version="2.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="BisCore" version="1.0.12" alias="bis"/>
        <ECSchemaReference name="CoreCustomAttributes" version="1.0.3" alias="CoreCA"/>

        <ECCustomAttributes>
          <DynamicSchema xmlns="CoreCustomAttributes.1.0.3"/>
        </ECCustomAttributes>

        <ECEntityClass typeName="Named_Item">
          <BaseClass>bis:PhysicalElement</BaseClass>
          <ECProperty propertyName="Alias" typeName="string" priority="0" />
        </ECEntityClass>

        <ECEntityClass typeName="Device">
          <BaseClass>Named_Item</BaseClass>
          <ECProperty propertyName="Alias" typeName="string" priority="1003" />
        </ECEntityClass>

        <ECEntityClass typeName="Equipment">
          <BaseClass>Device</BaseClass>
          <ECProperty propertyName="SerialNumber" typeName="string" />
        </ECEntityClass>
      </ECSchema>)xml";

    for (const auto& schemaToImport : { schemaV1_0_0, schemaV1_0_1, schemaV2_0_0 })
      {
      ECSchemaPtr schema;
      ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, Utf8PrintfString(schemaToImport, 1, 1).c_str(), *context));
      ASSERT_EQ(SchemaStatus::Success, m_db->ImportSchemas({ schema.get() }, true));
      m_db->SaveChanges();

      // Test the legacy V8 import logic
      ECSchemaPtr legacyImportSchema;
      ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(legacyImportSchema, Utf8PrintfString(schemaToImport, 2, 2).c_str(), *context));
      ASSERT_EQ(SchemaStatus::Success, m_db->ImportV8LegacySchemas({ legacyImportSchema.get() }));
      m_db->SaveChanges();
      }
    }