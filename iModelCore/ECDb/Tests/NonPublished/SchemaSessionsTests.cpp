/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct SchemaSessionsTestFixture : public ECDbTestFixture
    {
    };

//---------------------------------------------------------------------------------------
// @bsimethod
// Test that sessions are disabled by default and can be enabled
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSessionsTestFixture, EnableDisable)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaSessions_EnableDisable.ecdb"));

    auto& sessions = m_ecdb.Schemas().GetSessions();
    ASSERT_FALSE(sessions.IsEnabled());
    ASSERT_EQ(0, sessions.GetCount());

    sessions.SetEnabled(true);
    ASSERT_TRUE(sessions.IsEnabled());

    sessions.SetEnabled(false);
    ASSERT_FALSE(sessions.IsEnabled());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test recording a single schema import session
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSessionsTestFixture, RecordSingleSession)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaSessions_RecordSingle.ecdb"));
    auto& sessions = m_ecdb.Schemas().GetSessions();
    sessions.SetEnabled(true);

    SchemaItem schema(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Foo">
            <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));
    m_ecdb.SaveChanges();

    // Should have recorded 1 session
    ASSERT_EQ(1, sessions.GetCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test recording multiple schema import sessions
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSessionsTestFixture, RecordMultipleSessions)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaSessions_RecordMultiple.ecdb"));
    auto& sessions = m_ecdb.Schemas().GetSessions();
    sessions.SetEnabled(true);

    SchemaItem schema1(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="ClassA">
            <ECProperty propertyName="PropA" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    SchemaItem schema2(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="Schema2" alias="s2" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="ClassB">
            <ECProperty propertyName="PropB" typeName="int" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema1));
    m_ecdb.SaveChanges();
    ASSERT_EQ(1, sessions.GetCount());

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema2));
    m_ecdb.SaveChanges();
    ASSERT_EQ(2, sessions.GetCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test that no recording happens when sessions are disabled
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSessionsTestFixture, NoRecordWhenDisabled)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaSessions_NoRecord.ecdb"));
    auto& sessions = m_ecdb.Schemas().GetSessions();
    // sessions are disabled by default

    SchemaItem schema(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Foo">
            <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));
    m_ecdb.SaveChanges();

    ASSERT_EQ(0, sessions.GetCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test clearing all sessions
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSessionsTestFixture, ClearSessions)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaSessions_Clear.ecdb"));
    auto& sessions = m_ecdb.Schemas().GetSessions();
    sessions.SetEnabled(true);

    SchemaItem schema(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Foo">
            <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema));
    m_ecdb.SaveChanges();
    ASSERT_EQ(1, sessions.GetCount());

    ASSERT_EQ(BentleyStatus::SUCCESS, sessions.Clear());
    ASSERT_EQ(0, sessions.GetCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test replaying sessions into a fresh ECDb
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSessionsTestFixture, ReplaySessions)
    {
    // First, set up a source ECDb with sessions recorded
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaSessions_Replay_Source.ecdb"));
    auto& sessions = m_ecdb.Schemas().GetSessions();
    sessions.SetEnabled(true);

    SchemaItem schema1(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Foo">
            <ECProperty propertyName="Name" typeName="string" />
            <ECProperty propertyName="Age" typeName="int" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema1));
    m_ecdb.SaveChanges();
    ASSERT_EQ(1, sessions.GetCount());

    // Now open a fresh target ECDb and replay into it
    ECDb targetDb;
    BeFileName targetPath;
    {
    BeTest::GetHost().GetOutputRoot(targetPath);
    targetPath.AppendToPath(L"SchemaSessions_Replay_Target.ecdb");
    if (targetPath.DoesPathExist())
        BeFileName::BeDeleteFile(targetPath);
    ASSERT_EQ(BE_SQLITE_OK, targetDb.CreateNewDb(targetPath));
    }

    SchemaImportResult replayResult = sessions.Replay(targetDb);
    ASSERT_TRUE(replayResult.IsOk());

    // Verify the schema was imported into target
    ASSERT_NE(nullptr, targetDb.Schemas().GetClass("TestSchema", "Foo"));

    targetDb.SaveChanges();
    targetDb.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test replaying multiple sessions in order
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSessionsTestFixture, ReplayMultipleSessions)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaSessions_ReplayMulti_Source.ecdb"));
    auto& sessions = m_ecdb.Schemas().GetSessions();
    sessions.SetEnabled(true);

    SchemaItem schema1(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Foo">
            <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    SchemaItem schema2(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="Foo">
            <ECProperty propertyName="Name" typeName="string" />
            <ECProperty propertyName="Age" typeName="int" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema1));
    m_ecdb.SaveChanges();

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema2));
    m_ecdb.SaveChanges();
    ASSERT_EQ(2, sessions.GetCount());

    // Replay into fresh target
    ECDb targetDb;
    BeFileName targetPath;
    {
    BeTest::GetHost().GetOutputRoot(targetPath);
    targetPath.AppendToPath(L"SchemaSessions_ReplayMulti_Target.ecdb");
    if (targetPath.DoesPathExist())
        BeFileName::BeDeleteFile(targetPath);
    ASSERT_EQ(BE_SQLITE_OK, targetDb.CreateNewDb(targetPath));
    }

    SchemaImportResult replayResult = sessions.Replay(targetDb);
    ASSERT_TRUE(replayResult.IsOk());

    // Verify schema v1.0.1 is present (upgraded via second session)
    auto ecClass = targetDb.Schemas().GetClass("TestSchema", "Foo");
    ASSERT_NE(nullptr, ecClass);
    ASSERT_NE(nullptr, ecClass->GetPropertyP("Age"));

    targetDb.SaveChanges();
    targetDb.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test dynamic schema handling - XML stored per session even at same version
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSessionsTestFixture, DynamicSchemaPerSession)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaSessions_Dynamic.ecdb"));
    auto& sessions = m_ecdb.Schemas().GetSessions();
    sessions.SetEnabled(true);

    // Import a dynamic schema
    SchemaItem dynamicSchema1(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="DynSchema" alias="dyn" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECCustomAttributes>
            <DynamicSchema xmlns="CoreCustomAttributes.01.00.00"/>
          </ECCustomAttributes>
          <ECEntityClass typeName="DynClass">
            <ECProperty propertyName="PropA" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(dynamicSchema1));
    m_ecdb.SaveChanges();
    ASSERT_EQ(1, sessions.GetCount());

    // Import same version with different content (dynamic schemas allow this)
    SchemaItem dynamicSchema2(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="DynSchema" alias="dyn" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECCustomAttributes>
            <DynamicSchema xmlns="CoreCustomAttributes.01.00.00"/>
          </ECCustomAttributes>
          <ECEntityClass typeName="DynClass">
            <ECProperty propertyName="PropA" typeName="string" />
            <ECProperty propertyName="PropB" typeName="int" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(dynamicSchema2,
        SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications));
    m_ecdb.SaveChanges();
    ASSERT_EQ(2, sessions.GetCount());

    // Both sessions should have distinct XML stored (verified by checking be_Local has both keys)
    Utf8String xmlKey1("ec_schemaXml_DynSchema.01.00.00_1");
    Utf8String xmlKey2("ec_schemaXml_DynSchema.01.00.00_2");
    Utf8String xml1, xml2;
    ASSERT_EQ(BE_SQLITE_ROW, m_ecdb.QueryBriefcaseLocalValue(xml1, xmlKey1.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, m_ecdb.QueryBriefcaseLocalValue(xml2, xmlKey2.c_str()));
    ASSERT_NE(xml1, xml2); // Different content since PropB was added
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test GetCount with no sessions stored
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSessionsTestFixture, GetCountEmpty)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaSessions_GetCountEmpty.ecdb"));
    auto& sessions = m_ecdb.Schemas().GetSessions();
    sessions.SetEnabled(true);

    ASSERT_EQ(0, sessions.GetCount());
    }

END_ECDBUNITTESTS_NAMESPACE
