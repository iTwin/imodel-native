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
    // The property Name is defined in the base entity class "A" and gets overridden in the child "B".
    const auto schemaV1_0_0 = R"xml(<?xml version="1.0" encoding="utf-8" ?>
      <ECSchema schemaName="TestSchema%d" alias="ts%d" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="BisCore" version="1.0.12" alias="bis"/>
        <ECSchemaReference name="CoreCustomAttributes" version="1.0.3" alias="CoreCA"/>

        <ECCustomAttributes>
          <DynamicSchema xmlns="CoreCustomAttributes.1.0.3"/>
        </ECCustomAttributes>

        <ECEntityClass typeName="A">
          <BaseClass>bis:PhysicalElement</BaseClass>
          <ECProperty propertyName="Name" typeName="string" priority="0" />
        </ECEntityClass>

        <ECEntityClass typeName="B">
          <BaseClass>A</BaseClass>
          <ECProperty propertyName="Name" typeName="string" priority="1003" />
        </ECEntityClass>

        <ECEntityClass typeName="C">
          <BaseClass>B</BaseClass>
          <ECProperty propertyName="TestPropertyC" typeName="string" />
        </ECEntityClass>
      </ECSchema>)xml";

    // Minor schema update:
    // Override the property "Name" in the child class "C" and also add a derived class "D" which will inherit the property and add a property map row for itself.
    const auto schemaV1_0_1 = R"xml(<?xml version="1.0" encoding="utf-8" ?>
      <ECSchema schemaName="TestSchema%d" alias="ts%d" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="BisCore" version="1.0.12" alias="bis"/>
        <ECSchemaReference name="CoreCustomAttributes" version="1.0.3" alias="CoreCA"/>

        <ECCustomAttributes>
          <DynamicSchema xmlns="CoreCustomAttributes.1.0.3"/>
        </ECCustomAttributes>

        <ECEntityClass typeName="A">
          <BaseClass>bis:PhysicalElement</BaseClass>
          <ECProperty propertyName="Name" typeName="string" priority="0" />
        </ECEntityClass>

        <ECEntityClass typeName="B">
          <BaseClass>A</BaseClass>
          <ECProperty propertyName="Name" typeName="string" priority="1003" />
        </ECEntityClass>

        <ECEntityClass typeName="C">
          <BaseClass>B</BaseClass>
          <ECProperty propertyName="TestPropertyC" typeName="string" />
          <ECProperty propertyName="Name" typeName="string" priority="0" />
        </ECEntityClass>
          
        <ECEntityClass typeName="D">
          <BaseClass>C</BaseClass>
          <ECProperty propertyName="TestPropertyD" typeName="double" />
        </ECEntityClass>
      </ECSchema>)xml";

    // Major schema upgrade:
    // Delete the overridden property "Name" from class "C".
    // A property map for class "D" is created for the inherited property "Name".
    // Also delete the leaf class "D".
    const auto schemaV2_0_0 = R"xml(<?xml version="1.0" encoding="utf-8" ?>
      <ECSchema schemaName="TestSchema%d" alias="ts%d" version="2.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="BisCore" version="1.0.12" alias="bis"/>
        <ECSchemaReference name="CoreCustomAttributes" version="1.0.3" alias="CoreCA"/>

        <ECCustomAttributes>
          <DynamicSchema xmlns="CoreCustomAttributes.1.0.3"/>
        </ECCustomAttributes>

        <ECEntityClass typeName="A">
          <BaseClass>bis:PhysicalElement</BaseClass>
          <ECProperty propertyName="Name" typeName="string" priority="0" />
        </ECEntityClass>

        <ECEntityClass typeName="B">
          <BaseClass>A</BaseClass>
          <ECProperty propertyName="Name" typeName="string" priority="1003" />
        </ECEntityClass>

        <ECEntityClass typeName="C">
          <BaseClass>B</BaseClass>
          <ECProperty propertyName="TestPropertyC" typeName="string" />
        </ECEntityClass>
      </ECSchema>)xml";

    auto verifyClassExists = [&](Utf8StringCR schemaName, Utf8StringCR className) -> bool
      {
      Statement stmt;
      stmt.Prepare(*m_db, "SELECT 1 FROM ec_Class c JOIN ec_Schema s ON c.SchemaId = s.Id WHERE s.Name = ? AND c.Name = ?");
      stmt.BindText(1, schemaName, Statement::MakeCopy::No);
      stmt.BindText(2, className, Statement::MakeCopy::No);
      return stmt.Step() == BE_SQLITE_ROW;
      };

    auto getPropertyMapCount = [&](Utf8StringCR schemaName, Utf8StringCR className, Utf8StringCR propertyName) -> int
      {
      Statement stmt;
      stmt.Prepare(*m_db, R"sql(
        SELECT COUNT(*) FROM ec_PropertyMap pm
          JOIN ec_Class c ON pm.ClassId = c.Id
          JOIN ec_Schema s ON c.SchemaId = s.Id
          JOIN ec_PropertyPath pp ON pm.PropertyPathId = pp.Id
          JOIN ec_Property p ON pp.RootPropertyId = p.Id
        WHERE s.Name = ? AND c.Name = ? AND p.Name = ?
      )sql");
      stmt.BindText(1, schemaName, Statement::MakeCopy::No);
      stmt.BindText(2, className, Statement::MakeCopy::No);
      stmt.BindText(3, propertyName, Statement::MakeCopy::No);

      return (stmt.Step() == BE_SQLITE_ROW) ? stmt.GetValueInt(0) : 0;
      };

    auto getOrphanPropertyPathCount = [&]() -> int
      {
      Statement stmt;
      stmt.Prepare(*m_db, R"sql(
        SELECT COUNT(*) FROM ec_PropertyPath pp WHERE NOT EXISTS (SELECT 1 FROM ec_PropertyMap pm WHERE pm.PropertyPathId = pp.Id)
      )sql");

      return (stmt.Step() == BE_SQLITE_ROW) ? stmt.GetValueInt(0) : 0;
      };

    for (const auto& [legacyImport, schemaNumber] : { 
      std::make_pair(false, 1), // Test the regular import logic
      std::make_pair(true, 2) // Test the legacy V8 import logic with all the same verifications.
    })
      {
      const auto schemaName = Utf8PrintfString("TestSchema%d", schemaNumber);
      // Import v1.0.0 - baseline schema
        {
        ECSchemaPtr schema;
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, Utf8PrintfString(schemaV1_0_0, schemaNumber, schemaNumber).c_str(), *context));
        ASSERT_EQ(SchemaStatus::Success, legacyImport ? m_db->ImportV8LegacySchemas({ schema.get() }) : m_db->ImportSchemas({ schema.get() }, true));
        m_db->SaveChanges();

        ASSERT_TRUE(verifyClassExists(schemaName, "A"));
        ASSERT_TRUE(verifyClassExists(schemaName, "B"));
        ASSERT_TRUE(verifyClassExists(schemaName, "C"));
        ASSERT_FALSE(verifyClassExists(schemaName, "D"));

        // Verify property mappings exist
        ASSERT_EQ(getPropertyMapCount(schemaName, "A", "Name"), 1);
        ASSERT_EQ(getPropertyMapCount(schemaName, "B", "Name"), 1);
        ASSERT_EQ(getPropertyMapCount(schemaName, "C", "Name"), 1);
        ASSERT_EQ(getPropertyMapCount(schemaName, "C", "TestPropertyC"), 1);
        }

        // Import v1.0.1 - add D class and override Name in C
        {
        ECSchemaPtr schema;
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, Utf8PrintfString(schemaV1_0_1, schemaNumber, schemaNumber).c_str(), *context));
        ASSERT_EQ(SchemaStatus::Success, legacyImport ? m_db->ImportV8LegacySchemas({ schema.get() }) : m_db->ImportSchemas({ schema.get() }, true));
        m_db->SaveChanges();

        ASSERT_TRUE(verifyClassExists(schemaName, "D"));

        // Verify Name property mappings exist for B, C (override), and D (inherited from C)
        ASSERT_EQ(getPropertyMapCount(schemaName, "A", "Name"), 1);
        ASSERT_EQ(getPropertyMapCount(schemaName, "B", "Name"), 1);
        ASSERT_EQ(getPropertyMapCount(schemaName, "C", "Name"), 1);
        ASSERT_EQ(getPropertyMapCount(schemaName, "C", "TestPropertyC"), 1);
        ASSERT_EQ(getPropertyMapCount(schemaName, "D", "Name"), 1);
        ASSERT_EQ(getPropertyMapCount(schemaName, "D", "TestPropertyD"), 1);
        }

        // Import v2.0.0 - delete D class and remove Name override from C
        {
        ECSchemaPtr schema;
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, Utf8PrintfString(schemaV2_0_0, schemaNumber, schemaNumber).c_str(), *context));
        ASSERT_EQ(SchemaStatus::Success, legacyImport ? m_db->ImportV8LegacySchemas({ schema.get() }) : m_db->ImportSchemas({ schema.get() }, true));
        m_db->SaveChanges();

        // Verify D class no longer exists
        ASSERT_FALSE(verifyClassExists(schemaName, "D"));

        // Verify property maps for D are cleaned up (CASCADE DELETE on ClassId)
        ASSERT_EQ(getPropertyMapCount(schemaName, "D", "Name"), 0);
        ASSERT_EQ(getPropertyMapCount(schemaName, "D", "TestPropertyD"), 0);

        // Verify C still has property mappings for B's Name (no longer overridden)
        ASSERT_EQ(getPropertyMapCount(schemaName, "B", "Name"), 1);
        ASSERT_EQ(getPropertyMapCount(schemaName, "C", "Name"), 1);
        ASSERT_EQ(getPropertyMapCount(schemaName, "C", "TestPropertyC"), 1);

        // Verify no orphan property paths remain
        ASSERT_EQ(getOrphanPropertyPathCount(), 0) << "Orphan property paths should be cleaned up during schema remapping";
        }
      }
    }
