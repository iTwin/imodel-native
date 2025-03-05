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
