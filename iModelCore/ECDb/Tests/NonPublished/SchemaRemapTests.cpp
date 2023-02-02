/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <set>
#include <ECObjects/SchemaComparer.h>
#include <Bentley/BeDirectoryIterator.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct SchemaRemapTestFixture : public ECDbTestFixture
    {
    std::vector<Utf8String> m_updatedDbs;
    protected:

        //---------------------------------------------------------------------------------------
        // @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        void CloseReOpenECDb()
            {
            Utf8CP dbFileName = m_ecdb.GetDbFileName();
            BeFileName dbPath(dbFileName);
            m_ecdb.CloseDb();
            ASSERT_FALSE(m_ecdb.IsDbOpen());
            ASSERT_EQ(BE_SQLITE_OK, m_ecdb.OpenBeSQLiteDb(dbPath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));
            ASSERT_TRUE(m_ecdb.IsDbOpen());
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        DbResult OpenBesqliteDb(Utf8CP dbPath) { return m_ecdb.OpenBeSQLiteDb(dbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)); }
        BentleyStatus ImportSchemasFromFolder(BeFileName const& schemaFolder);
        BentleyStatus ImportSchemaFromFile(BeFileName const& fileName);
    };

#define ASSERT_ECSQL(ECDB_OBJ, PREPARESTATUS, STEPSTATUS, ECSQL)   {\
                                                                    ECSqlStatement stmt;\
                                                                    ASSERT_EQ(PREPARESTATUS, stmt.Prepare(ECDB_OBJ, ECSQL));\
                                                                    if (PREPARESTATUS == ECSqlStatus::Success)\
                                                                        ASSERT_EQ(STEPSTATUS, stmt.Step());\
                                                                   }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, ChangeBaseClassDownInExistingHierarchy)
    {
    // start: C -> A   end: C -> B -> A
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="NewBaseClass" alias="nbc" version="01.00.00" displayLabel="InsertNewBaseClassInMiddleOfExistingHierarchy" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
            <ECProperty propertyName="PropA" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropC" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("modifyBaseClassDownInExistingHierarchy.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO NewBaseClass.C (PropA,PropC) VALUES ('FIRSTA', 'FIRSTC')");
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="NewBaseClass" alias="nbc" version="01.01.00" displayLabel="InsertNewBaseClassInMiddleOfExistingHierarchy" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
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
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    {
    ECClassCP c = m_ecdb.Schemas().GetClass("NewBaseClass", "C");
    ASSERT_NE(c, nullptr);
    ASSERT_EQ(1, c->GetBaseClasses().size());
    ASSERT_STREQ(c->GetBaseClasses().at(0)->GetFullName(), "NewBaseClass:B");
    ASSERT_EQ(3, c->GetPropertyCount());
    }

    // Verify we can insert and select
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO NewBaseClass.C (PropA,PropB,PropC) VALUES ('SECONDA', 'SECONDB', 'SECONDC')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA, PropB, PropC FROM NewBaseClass.C");
    ASSERT_EQ(JsonValue(R"json([{"PropA":"FIRSTA", "PropC":"FIRSTC"},{"PropA":"SECONDA", "PropB":"SECONDB", "PropC":"SECONDC"}])json"),result) << "Verify inserted instances";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, AddTwoClassesInMiddleOfHierarchy)
    {
    // start: D -> A   end: D -> C -> B -> A
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="NewBaseClass" alias="nbc" version="01.00.00" displayLabel="InsertNewBaseClassInMiddleOfExistingHierarchy" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
            <ECProperty propertyName="PropA" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="D">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropD" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("addTwoClassesInMiddleOfHierarchy.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO NewBaseClass.D (PropA,PropD) VALUES ('FIRSTA', 'FIRSTD')");
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="NewBaseClass" alias="nbc" version="01.01.00" displayLabel="InsertNewBaseClassInMiddleOfExistingHierarchy" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
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
          <ECEntityClass typeName="D">
            <BaseClass>C</BaseClass>
            <ECProperty propertyName="PropD" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    {
    ECClassCP d = m_ecdb.Schemas().GetClass("NewBaseClass", "D");
    ASSERT_NE(d, nullptr);
    ASSERT_EQ(1, d->GetBaseClasses().size());
    ASSERT_STREQ(d->GetBaseClasses().at(0)->GetFullName(), "NewBaseClass:C");
    ASSERT_EQ(4, d->GetPropertyCount());
    }

    // Verify we can insert and select
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO NewBaseClass.D (PropA,PropB,PropC,PropD) VALUES ('SECONDA', 'SECONDB', 'SECONDC', 'SECONDD')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA, PropB, PropC, PropD FROM NewBaseClass.D");
    ASSERT_EQ(JsonValue(R"json([{"PropA":"FIRSTA", "PropD":"FIRSTD"},{"PropA":"SECONDA", "PropB":"SECONDB", "PropC":"SECONDC", "PropD":"SECONDD"}])json"),result) << "Verify inserted instances";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, RemoveClassFromMiddleOfHiearchy)
    {
    // start: C -> B -> A end: C -> A
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="NewBaseClass" alias="nbc" version="01.00.00" displayLabel="InsertNewBaseClassInMiddleOfExistingHierarchy" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
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
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("removeClassFromMiddleOfHiearchy.ecdb", schemaItem));

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="NewBaseClass" alias="nbc" version="01.01.00" displayLabel="InsertNewBaseClassInMiddleOfExistingHierarchy" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
            <ECProperty propertyName="PropA" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropB" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropC" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Moving up the base class hierarchy should not be supported";
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MoveClassToDifferentHierarchy)
    {
    // start: C -> A end: C -> B
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="NewBaseClass" alias="nbc" version="01.00.00" displayLabel="InsertNewBaseClassInMiddleOfExistingHierarchy" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
            <ECProperty propertyName="PropA" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <ECProperty propertyName="PropB" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropC" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("moveClassToDifferentHierarchy.ecdb", schemaItem));

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="NewBaseClass" alias="nbc" version="01.01.00" displayLabel="InsertNewBaseClassInMiddleOfExistingHierarchy" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
            <ECProperty propertyName="PropA" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <ECProperty propertyName="PropB" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="PropC" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Changing the base class should not be supported";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MovePropertyUpInHierarchyUsingOverflowTable)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="PseudoBisElement">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                  <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="PropElement1" typeName="string" />
            <ECProperty propertyName="PropElement2" typeName="string" />
            <ECProperty propertyName="PropElement3" typeName="string" />
            <ECProperty propertyName="PropElement4" typeName="string" />
            <ECProperty propertyName="PropElement5" typeName="string" />
            <ECProperty propertyName="PropElement6" typeName="string" />
            <ECProperty propertyName="PropElement7" typeName="string" />
            <ECProperty propertyName="PropElement8" typeName="string" />
            <ECProperty propertyName="PropElement9" typeName="string" />
            <ECProperty propertyName="PropElement10" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="HVAC_COMPONENT">
            <BaseClass>PseudoBisElement</BaseClass>
            <ECProperty propertyName="PropA1" typeName="string" />
            <ECProperty propertyName="PropA2" typeName="string" />
            <ECProperty propertyName="PropA3" typeName="string" />
            <ECProperty propertyName="PropA4" typeName="string" />
            <ECProperty propertyName="PropA5" typeName="string" />
            <ECProperty propertyName="PropA6" typeName="string" />
            <ECProperty propertyName="PropA7" typeName="string" />
            <ECProperty propertyName="PropA8" typeName="string" />
            <ECProperty propertyName="PropA9" typeName="string" />
            <ECProperty propertyName="PropA10" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="HVAC_EQUIPMENTS">
            <BaseClass>HVAC_COMPONENT</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="BAFFLE_SILENCERS">
            <BaseClass>HVAC_EQUIPMENTS</BaseClass>
            <ECProperty propertyName="PropB1" typeName="string" />
            <ECProperty propertyName="PropB2" typeName="string" />
            <ECProperty propertyName="PropB3" typeName="string" />
            <ECProperty propertyName="PropB4" typeName="string" />
            <ECProperty propertyName="PropB5" typeName="string" />
            <ECProperty propertyName="PropB6" typeName="string" />
            <ECProperty propertyName="PropB7" typeName="string" />
            <ECProperty propertyName="PropB8" typeName="string" />
            <ECProperty propertyName="PropB9" typeName="string" />
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="HVAC_AIR_FILTER">
            <BaseClass>HVAC_EQUIPMENTS</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
            <ECProperty propertyName="PropC2" typeName="string" />
            <ECProperty propertyName="PropC3" typeName="string" />
            <ECProperty propertyName="PropC4" typeName="string" />
            <ECProperty propertyName="PropC5" typeName="string" />
            <ECProperty propertyName="PropC6" typeName="string" />
            <ECProperty propertyName="PropC7" typeName="string" />
            <ECProperty propertyName="PropC8" typeName="string" />
            <ECProperty propertyName="PropC9" typeName="string" />
            <ECProperty propertyName="PropC10" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertyUpInHierarchyUsingOverflowTable.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.BAFFLE_SILENCERS (MovingProperty) VALUES ('FIRST')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.BAFFLE_SILENCERS");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="PseudoBisElement">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                  <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="PropElement1" typeName="string" />
            <ECProperty propertyName="PropElement2" typeName="string" />
            <ECProperty propertyName="PropElement3" typeName="string" />
            <ECProperty propertyName="PropElement4" typeName="string" />
            <ECProperty propertyName="PropElement5" typeName="string" />
            <ECProperty propertyName="PropElement6" typeName="string" />
            <ECProperty propertyName="PropElement7" typeName="string" />
            <ECProperty propertyName="PropElement8" typeName="string" />
            <ECProperty propertyName="PropElement9" typeName="string" />
            <ECProperty propertyName="PropElement10" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="HVAC_COMPONENT">
            <BaseClass>PseudoBisElement</BaseClass>
            <ECProperty propertyName="PropA1" typeName="string" />
            <ECProperty propertyName="PropA2" typeName="string" />
            <ECProperty propertyName="PropA3" typeName="string" />
            <ECProperty propertyName="PropA4" typeName="string" />
            <ECProperty propertyName="PropA5" typeName="string" />
            <ECProperty propertyName="PropA6" typeName="string" />
            <ECProperty propertyName="PropA7" typeName="string" />
            <ECProperty propertyName="PropA8" typeName="string" />
            <ECProperty propertyName="PropA9" typeName="string" />
            <ECProperty propertyName="PropA10" typeName="string" />
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="HVAC_EQUIPMENTS">
            <BaseClass>HVAC_COMPONENT</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="BAFFLE_SILENCERS">
            <BaseClass>HVAC_EQUIPMENTS</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
            <ECProperty propertyName="PropB1" typeName="string" />
            <ECProperty propertyName="PropB2" typeName="string" />
            <ECProperty propertyName="PropB3" typeName="string" />
            <ECProperty propertyName="PropB4" typeName="string" />
            <ECProperty propertyName="PropB5" typeName="string" />
            <ECProperty propertyName="PropB6" typeName="string" />
            <ECProperty propertyName="PropB7" typeName="string" />
            <ECProperty propertyName="PropB8" typeName="string" />
            <ECProperty propertyName="PropB9" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="HVAC_AIR_FILTER">
            <BaseClass>HVAC_EQUIPMENTS</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
            <ECProperty propertyName="PropC2" typeName="string" />
            <ECProperty propertyName="PropC3" typeName="string" />
            <ECProperty propertyName="PropC4" typeName="string" />
            <ECProperty propertyName="PropC5" typeName="string" />
            <ECProperty propertyName="PropC6" typeName="string" />
            <ECProperty propertyName="PropC7" typeName="string" />
            <ECProperty propertyName="PropC8" typeName="string" />
            <ECProperty propertyName="PropC9" typeName="string" />
            <ECProperty propertyName="PropC10" typeName="string" />
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Verify we can insert and select
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.BAFFLE_SILENCERS (MovingProperty) VALUES ('SECOND')");
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.BAFFLE_SILENCERS");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}, {"MovingProperty":"SECOND"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MovePropertyUpInHierarchySimplified)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertyUpInHierarchySimple.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('FIRST')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Verify we can insert and select
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('SECOND')");
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}, {"MovingProperty":"SECOND"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MovePropertyUpInHierarchyRemoveOriginal)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertyUpInHierarchyRemoveOriginal.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('FIRST')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Verify the instance is still intact
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.MyBaseClass");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MovePropertyUpInHierarchyDeleteBeforeAddInSchema)
    {
    // like previous test, but the base class comes later in the schema, so the change to "delete" the property is detected first

    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertyUpInHierarchyDeleteBeforeAdd.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('FIRST')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Verify the instance is still intact
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.MyBaseClass");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, RemoveOverwrittenProperty)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("removeOverwrittenProperty.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('FIRST')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    // Verify the instance is still intact
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.MyBaseClass");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MoveMultiplePropertiesUpInHierarchy)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
            <ECProperty propertyName="MovingProperty2" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("moveMultiplePropertyUpInHierarchy.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('FIRST')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.B (MovingProperty2) VALUES ('FIRST2')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty2 FROM TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty2":"FIRST2"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="MovingProperty" typeName="string" />
            <ECProperty propertyName="MovingProperty2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Verify the instance is still intact
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty, MovingProperty2 FROM TestSchema.MyBaseClass");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}, {"MovingProperty2":"FIRST2"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, AddOverwrittenPropertyInOneStep)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("addOverwittenProperty.ecdb", schemaItem));

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="OverwrittenProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="OverwrittenProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, AddNewBaseClassInMiddleMovePropertyUp)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("newBaseClassMoveProperty.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('FIRST')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="MiddleBaseClass">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MiddleBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MiddleBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Verify the instance is still intact
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, AddNewBaseClassInMiddleMovePropertyUpRemoveOriginal)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("newBaseClassMovePropertyRemoveOrig.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('FIRST')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="MiddleBaseClass">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MiddleBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MiddleBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>A</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Verify the instance is still intact
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, AddNewBaseClassInMiddleMovePropertyUpReversed)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("newBaseClassMovePropertyRev.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('FIRST')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <BaseClass>MiddleBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MiddleBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="MiddleBaseClass">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Verify the instance is still intact
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, AddNewBaseClassInMiddleMovePropertyUpRemoveOriginalReversed)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("newBaseClassMovePropertyRemoveOrigRev.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('FIRST')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <BaseClass>MiddleBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MiddleBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="MiddleBaseClass">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Verify the instance is still intact
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MovePropertyToNonSharedColumn)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyNonSharedBase">
          </ECEntityClass>
          <ECEntityClass typeName="MyBaseClass">
            <BaseClass>MyNonSharedBase</BaseClass>
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertyToNonSharedColumn.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('FIRST')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyNonSharedBase">
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="MyBaseClass">
            <BaseClass>MyNonSharedBase</BaseClass>
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    // Verify the instance is still intact
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MoveMultiColumnPropertyUp)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="blue" typeName="int" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="int" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="int" displayLabel="Red"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECStructProperty propertyName="MovingProperty" typeName="ColorType"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("moveMultiColumnPropertyUpInHierarchySimple.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty.blue, MovingProperty.green, MovingProperty.red) VALUES (1,2,3)");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":{"blue":1,"green":2,"red":3}}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="blue" typeName="int" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="int" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="int" displayLabel="Red"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECStructProperty propertyName="MovingProperty" typeName="ColorType"/>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECStructProperty propertyName="MovingProperty" typeName="ColorType"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty.blue, MovingProperty.green, MovingProperty.red) VALUES (4,5,6)");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":{"blue":1,"green":2,"red":3}},{"MovingProperty":{"blue":4,"green":5,"red":6}}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MoveMultiColumnPropertiesUp)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="blue" typeName="string" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="string" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="string" displayLabel="Red"/>
          </ECStructClass>
          <ECStructClass typeName="Coordinates">
            <ECProperty propertyName="x" typeName="string" displayLabel="Blue"/>
            <ECProperty propertyName="y" typeName="string" displayLabel="Green"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECStructProperty propertyName="Color" typeName="ColorType"/>
            <ECProperty propertyName="SimpleProp" typeName="string" />
            <ECStructProperty propertyName="Coords" typeName="Coordinates"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="UnrelatedProp" typeName="string" />
            <ECStructProperty propertyName="Coords" typeName="Coordinates"/>
            <ECProperty propertyName="SimpleProp" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECStructProperty propertyName="Color" typeName="ColorType"/>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertiesUpInHierarchy.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (Color.blue, Color.green, Color.red, SimpleProp, Coords.x, Coords.y) VALUES ('A.blue','A.green','A.red', 'A simple', 'A.x', 'A.y')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT Color, SimpleProp, Coords FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"Color":{"blue":"A.blue","green":"A.green","red":"A.red"},"SimpleProp":"A simple","Coords":{"x":"A.x","y":"A.y"}}])json"), result);
    }
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.B (UnrelatedProp, SimpleProp, Coords.x, Coords.y) VALUES ('B unrelated', 'B simple', 'B.x', 'B.y')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT UnrelatedProp, SimpleProp, Coords FROM ONLY TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"UnrelatedProp":"B unrelated","SimpleProp":"B simple","Coords":{"x":"B.x","y":"B.y"}}])json"), result);
    }
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.C (Color.blue, Color.green, Color.red, UnrelatedProp, SimpleProp, Coords.x, Coords.y) VALUES ('C.blue','C.green','C.red', 'C unrelated', 'C simple', 'C.x', 'C.y')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT Color, UnrelatedProp, SimpleProp, Coords FROM ONLY TestSchema.C");
    ASSERT_EQ(JsonValue(R"json([{"Color":{"blue":"C.blue","green":"C.green","red":"C.red"},"UnrelatedProp":"C unrelated","SimpleProp":"C simple","Coords":{"x":"C.x","y":"C.y"}}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="blue" typeName="string" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="string" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="string" displayLabel="Red"/>
          </ECStructClass>
          <ECStructClass typeName="Coordinates">
            <ECProperty propertyName="x" typeName="string" displayLabel="Blue"/>
            <ECProperty propertyName="y" typeName="string" displayLabel="Green"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECStructProperty propertyName="Color" typeName="ColorType"/>
            <ECProperty propertyName="SimpleProp" typeName="string" />
            <ECStructProperty propertyName="Coords" typeName="Coordinates"/>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="UnrelatedProp" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT Color, SimpleProp, Coords FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"Color":{"blue":"A.blue","green":"A.green","red":"A.red"},"SimpleProp":"A simple","Coords":{"x":"A.x","y":"A.y"}}])json"), result);
    }
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT UnrelatedProp, SimpleProp, Coords FROM ONLY TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"UnrelatedProp":"B unrelated","SimpleProp":"B simple","Coords":{"x":"B.x","y":"B.y"}}])json"), result);
    }
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT Color, UnrelatedProp, SimpleProp, Coords FROM ONLY TestSchema.C");
    ASSERT_EQ(JsonValue(R"json([{"Color":{"blue":"C.blue","green":"C.green","red":"C.red"},"UnrelatedProp":"C unrelated","SimpleProp":"C simple","Coords":{"x":"C.x","y":"C.y"}}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MovePropertyToMixin)
    {
    SchemaItem schemaItem(R"schema(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="IMyMixin" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                    <AppliesToEntityClass>MyBaseClass</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <BaseClass>IMyMixin</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertyToMixin.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('FIRST')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="IMyMixin" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                    <AppliesToEntityClass>MyBaseClass</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <BaseClass>IMyMixin</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    // Verify old and new instances
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('SECOND')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"},{"MovingProperty":"SECOND"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MovePropertiesInNonSharedColumns)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="blue" typeName="string" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="string" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="string" displayLabel="Red"/>
          </ECStructClass>
          <ECStructClass typeName="Coordinates">
            <ECProperty propertyName="x" typeName="string" displayLabel="Blue"/>
            <ECProperty propertyName="y" typeName="string" displayLabel="Green"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECStructProperty propertyName="Color" typeName="ColorType"/>
            <ECProperty propertyName="SimpleProp" typeName="string" />
            <ECStructProperty propertyName="Coords" typeName="Coordinates"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="UnrelatedProp" typeName="string" />
            <ECStructProperty propertyName="Coords" typeName="Coordinates"/>
            <ECProperty propertyName="SimpleProp" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECStructProperty propertyName="Color" typeName="ColorType"/>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertiesInNonSharedColumns.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (Color.blue, Color.green, Color.red, SimpleProp, Coords.x, Coords.y) VALUES ('A.blue','A.green','A.red', 'A simple', 'A.x', 'A.y')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT Color, SimpleProp, Coords FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"Color":{"blue":"A.blue","green":"A.green","red":"A.red"},"SimpleProp":"A simple","Coords":{"x":"A.x","y":"A.y"}}])json"), result);
    }
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.B (UnrelatedProp, SimpleProp, Coords.x, Coords.y) VALUES ('B unrelated', 'B simple', 'B.x', 'B.y')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT UnrelatedProp, SimpleProp, Coords FROM ONLY TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"UnrelatedProp":"B unrelated","SimpleProp":"B simple","Coords":{"x":"B.x","y":"B.y"}}])json"), result);
    }
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.C (Color.blue, Color.green, Color.red, UnrelatedProp, SimpleProp, Coords.x, Coords.y) VALUES ('C.blue','C.green','C.red', 'C unrelated', 'C simple', 'C.x', 'C.y')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT Color, UnrelatedProp, SimpleProp, Coords FROM ONLY TestSchema.C");
    ASSERT_EQ(JsonValue(R"json([{"Color":{"blue":"C.blue","green":"C.green","red":"C.red"},"UnrelatedProp":"C unrelated","SimpleProp":"C simple","Coords":{"x":"C.x","y":"C.y"}}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="blue" typeName="string" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="string" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="string" displayLabel="Red"/>
          </ECStructClass>
          <ECStructClass typeName="Coordinates">
            <ECProperty propertyName="x" typeName="string" displayLabel="Blue"/>
            <ECProperty propertyName="y" typeName="string" displayLabel="Green"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
            </ECCustomAttributes>
            <ECStructProperty propertyName="Color" typeName="ColorType"/>
            <ECProperty propertyName="SimpleProp" typeName="string" />
            <ECStructProperty propertyName="Coords" typeName="Coordinates"/>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="UnrelatedProp" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT Color, SimpleProp, Coords FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"Color":{"blue":"A.blue","green":"A.green","red":"A.red"},"SimpleProp":"A simple","Coords":{"x":"A.x","y":"A.y"}}])json"), result);
    }
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT UnrelatedProp, SimpleProp, Coords FROM ONLY TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"UnrelatedProp":"B unrelated","SimpleProp":"B simple","Coords":{"x":"B.x","y":"B.y"}}])json"), result);
    }
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT Color, UnrelatedProp, SimpleProp, Coords FROM ONLY TestSchema.C");
    ASSERT_EQ(JsonValue(R"json([{"Color":{"blue":"C.blue","green":"C.green","red":"C.red"},"UnrelatedProp":"C unrelated","SimpleProp":"C simple","Coords":{"x":"C.x","y":"C.y"}}])json"), result);
    }
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MovePropertiesInDefaultTables)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="blue" typeName="string" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="string" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="string" displayLabel="Red"/>
          </ECStructClass>
          <ECStructClass typeName="Coordinates">
            <ECProperty propertyName="x" typeName="string" displayLabel="Blue"/>
            <ECProperty propertyName="y" typeName="string" displayLabel="Green"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECStructProperty propertyName="Color" typeName="ColorType"/>
            <ECProperty propertyName="SimpleProp" typeName="string" />
            <ECStructProperty propertyName="Coords" typeName="Coordinates"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="UnrelatedProp" typeName="string" />
            <ECStructProperty propertyName="Coords" typeName="Coordinates"/>
            <ECProperty propertyName="SimpleProp" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECStructProperty propertyName="Color" typeName="ColorType"/>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertiesInDefaultTables.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (Color.blue, Color.green, Color.red, SimpleProp, Coords.x, Coords.y) VALUES ('A.blue','A.green','A.red', 'A simple', 'A.x', 'A.y')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT Color, SimpleProp, Coords FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"Color":{"blue":"A.blue","green":"A.green","red":"A.red"},"SimpleProp":"A simple","Coords":{"x":"A.x","y":"A.y"}}])json"), result);
    }
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.B (UnrelatedProp, SimpleProp, Coords.x, Coords.y) VALUES ('B unrelated', 'B simple', 'B.x', 'B.y')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT UnrelatedProp, SimpleProp, Coords FROM ONLY TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"UnrelatedProp":"B unrelated","SimpleProp":"B simple","Coords":{"x":"B.x","y":"B.y"}}])json"), result);
    }
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.C (Color.blue, Color.green, Color.red, UnrelatedProp, SimpleProp, Coords.x, Coords.y) VALUES ('C.blue','C.green','C.red', 'C unrelated', 'C simple', 'C.x', 'C.y')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT Color, UnrelatedProp, SimpleProp, Coords FROM ONLY TestSchema.C");
    ASSERT_EQ(JsonValue(R"json([{"Color":{"blue":"C.blue","green":"C.green","red":"C.red"},"UnrelatedProp":"C unrelated","SimpleProp":"C simple","Coords":{"x":"C.x","y":"C.y"}}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="blue" typeName="string" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="string" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="string" displayLabel="Red"/>
          </ECStructClass>
          <ECStructClass typeName="Coordinates">
            <ECProperty propertyName="x" typeName="string" displayLabel="Blue"/>
            <ECProperty propertyName="y" typeName="string" displayLabel="Green"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECStructProperty propertyName="Color" typeName="ColorType"/>
            <ECProperty propertyName="SimpleProp" typeName="string" />
            <ECStructProperty propertyName="Coords" typeName="Coordinates"/>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="UnrelatedProp" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT Color, SimpleProp, Coords FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"Color":{"blue":"A.blue","green":"A.green","red":"A.red"},"SimpleProp":"A simple","Coords":{"x":"A.x","y":"A.y"}}])json"), result);
    }
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT UnrelatedProp, SimpleProp, Coords FROM ONLY TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"UnrelatedProp":"B unrelated","SimpleProp":"B simple","Coords":{"x":"B.x","y":"B.y"}}])json"), result);
    }
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT Color, UnrelatedProp, SimpleProp, Coords FROM ONLY TestSchema.C");
    ASSERT_EQ(JsonValue(R"json([{"Color":{"blue":"C.blue","green":"C.green","red":"C.red"},"UnrelatedProp":"C unrelated","SimpleProp":"C simple","Coords":{"x":"C.x","y":"C.y"}}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, ModifyAndMoveStruct)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="blue" typeName="int" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="int" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="int" displayLabel="Red"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECStructProperty propertyName="MovingProperty" typeName="ColorType"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("modifyAndMoveStruct.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty.blue, MovingProperty.green, MovingProperty.red) VALUES (1,2,3)");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":{"blue":1,"green":2,"red":3}}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="black" typeName="int" displayLabel="Black"/>
            <ECProperty propertyName="blue" typeName="int" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="int" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="int" displayLabel="Red"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECStructProperty propertyName="MovingProperty" typeName="ColorType"/>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECStructProperty propertyName="MovingProperty" typeName="ColorType"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropC1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty.blue, MovingProperty.green, MovingProperty.red) VALUES (4,5,6)");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":{"blue":1,"green":2,"red":3}},{"MovingProperty":{"blue":4,"green":5,"red":6}}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MovePropertyToMixinAndRemoveOriginal)
    {
    SchemaItem schemaItem(R"schema(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="IMyMixin" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                    <AppliesToEntityClass>MyBaseClass</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <BaseClass>IMyMixin</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertyToMixinAndRemoveOriginal.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('FIRST')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="IMyMixin" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                    <AppliesToEntityClass>MyBaseClass</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <BaseClass>IMyMixin</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    // Verify old and new instances
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('SECOND')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"},{"MovingProperty":"SECOND"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MovePropertyOnRelationshipClass)
    {
    SchemaItem schemaItem(R"schema(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>

          <ECEntityClass typeName="Element" modifier="Abstract">
              <ECCustomAttributes>
                  <ClassMap xmlns="ECDbMap.2.0.0">
                      <MapStrategy>TablePerHierarchy</MapStrategy>
                  </ClassMap>
                  <NotSubclassableInReferencingSchemas xmlns="CoreCustomAttributes.1.0.0"/>
              </ECCustomAttributes>
          </ECEntityClass>

          <ECRelationshipClass typeName="ElementRefersToElements" strength="referencing" modifier="Abstract">
            <ECCustomAttributes>
                <LinkTableRelationshipMap xmlns="ECDbMap.2.0.0">
                    <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                </LinkTableRelationshipMap>
                <ClassMap xmlns="ECDbMap.2.0.0">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.2.0.0">
                    <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                    <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                </ShareColumns>
            </ECCustomAttributes>
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Element"/>
            </Target>
          </ECRelationshipClass>

          <ECRelationshipClass typeName="CategoryRefersToCategories" strength="referencing" modifier="Sealed">
            <BaseClass>ElementRefersToElements</BaseClass>
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="Category"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Category"/>
            </Target>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECRelationshipClass>

          <ECEntityClass typeName="Category">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="Description" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertyOnRelationshipClass.ecdb", schemaItem));
    {
    ECClassId categoryClassId = m_ecdb.Schemas().GetSchema("TestSchema")->GetClassCP("Category")->GetId();
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Category (Description) VALUES ('Category 1')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Category (Description) VALUES ('Category 2')");

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO TestSchema.CategoryRefersToCategories(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, MovingProperty) VALUES(1 , %llu , 2 , %llu, 'FIRST')", categoryClassId.GetValue(), categoryClassId.GetValue());
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.CategoryRefersToCategories");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>

          <ECEntityClass typeName="Element" modifier="Abstract">
              <ECCustomAttributes>
                  <ClassMap xmlns="ECDbMap.2.0.0">
                      <MapStrategy>TablePerHierarchy</MapStrategy>
                  </ClassMap>
                  <NotSubclassableInReferencingSchemas xmlns="CoreCustomAttributes.1.0.0"/>
              </ECCustomAttributes>
          </ECEntityClass>

          <ECRelationshipClass typeName="ElementRefersToElements" strength="referencing" modifier="Abstract">
            <ECCustomAttributes>
                <LinkTableRelationshipMap xmlns="ECDbMap.2.0.0">
                    <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                </LinkTableRelationshipMap>
                <ClassMap xmlns="ECDbMap.2.0.0">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.2.0.0">
                    <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                    <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                </ShareColumns>
            </ECCustomAttributes>
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="Element"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Element"/>
            </Target>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECRelationshipClass>

          <ECRelationshipClass typeName="CategoryRefersToCategories" strength="referencing" modifier="Sealed">
            <BaseClass>ElementRefersToElements</BaseClass>
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="true">
                <Class class="Category"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Category"/>
            </Target>
          </ECRelationshipClass>

          <ECEntityClass typeName="Category">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="Description" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Verify old and new instances
    {
    ECClassId categoryClassId = m_ecdb.Schemas().GetSchema("TestSchema")->GetClassCP("Category")->GetId();
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Category (Description) VALUES ('Category 3')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Category (Description) VALUES ('Category 4')");

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO TestSchema.CategoryRefersToCategories(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, MovingProperty) VALUES(4 , %llu , 5 , %llu, 'SECOND')", categoryClassId.GetValue(), categoryClassId.GetValue());
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, ecsql.c_str());

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.ElementRefersToElements where SourceECInstanceId=4");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"SECOND"}])json"), result);

    result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.ElementRefersToElements");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"},{"MovingProperty":"SECOND"}])json"), result);
    m_ecdb.SaveChanges();
    m_ecdb.CloseDb();
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, SpatialCompositionNewBaseScenario)
    {
    // Test that attempts the reflect the exact conditions that we hit before implementing moving properties
    // a new base class with a property is introduced in schema SpatialComposition
    SchemaItem schemaItem(R"schema(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>

<ECEntityClass typeName="bis_Element" modifier="Abstract" description="A bis:Element is the smallest individually identifiable building block for modeling the real world. Each bis:Element represents an Entity in the real world. Sets of bis:Elements (contained in bis:Models) are used to sub-model other bis:Elements that represent larger scale real world Entities. Using this recursive modeling strategy, bis:Elements can represent Entities at any scale. Elements can represent physical things, abstract concepts or simply be information records.">
        <!-- NOTE: Only platform can directly subclass from Element. Everyone else should pick the most appropriate superclass. -->
        <!-- NOTE: Element subclasses should drop the "Element" suffix from their name once the concept becomes "user facing" -->
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <NotSubclassableInReferencingSchemas xmlns="CoreCustomAttributes.1.0.0"/>
            <DbIndexList xmlns="ECDbMap.2.0.0">
                <Indexes>
                    <DbIndex>
                        <Name>ix_bis_Element_FederationGuid</Name>
                        <IsUnique>True</IsUnique>
                        <Properties>
                            <string>FederationGuid</string>
                        </Properties>
                        <Where>IndexedColumnsAreNotNull</Where>
                    </DbIndex>
                    <DbIndex>
                        <Name>ix_bis_Element_UserLabel</Name>
                        <IsUnique>False</IsUnique>
                        <Properties>
                            <string>UserLabel</string>
                        </Properties>
                        <Where>IndexedColumnsAreNotNull</Where>
                    </DbIndex>
                </Indexes>
            </DbIndexList>
            <ClassHasCurrentTimeStampProperty xmlns="CoreCustomAttributes.1.0.0">
                <PropertyName>LastMod</PropertyName>
            </ClassHasCurrentTimeStampProperty>
        </ECCustomAttributes>
        <ECProperty propertyName="LastMod" typeName="dateTime" readOnly="True" displayLabel="Last Modified" description="The last modified time of the bis:Element. This is maintained by the core framework and should not be set directly by applications.">
            <ECCustomAttributes>
                <DateTimeInfo xmlns="CoreCustomAttributes.1.0.0">
                    <DateTimeKind>Utc</DateTimeKind>
                </DateTimeInfo>
                <HiddenProperty xmlns="CoreCustomAttributes.1.0.0"/>
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="CodeValue" typeName="string" displayLabel="Code" description="The CodeValue property stores the formal name (business key) for a bis:Element. The combination of CodeSpec, CodeScope, and CodeValue properties must be unique for each bis:Element instance.">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.2.0.0">
                    <IsNullable>True</IsNullable>
                    <Collation>NoCase</Collation>
                </PropertyMap>
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="UserLabel" typeName="string" displayLabel="User Label" description="An optional friendly name given by the user (as opposed to the formal name stored in the CodeValue property).">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.2.0.0">
                    <IsNullable>True</IsNullable>
                    <Collation>NoCase</Collation>
                    <IsUnique>False</IsUnique>
                </PropertyMap>
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="FederationGuid" typeName="binary" extendedTypeName="BeGuid" displayLabel="Federation GUID" description="The GUID used to federate this bis:Element across repositories.">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.2.0.0">
                    <IsNullable>True</IsNullable>
                    <IsUnique>True</IsUnique>
                </PropertyMap>
                <HiddenProperty xmlns="CoreCustomAttributes.1.0.0"/>
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="JsonProperties" typeName="string" extendedTypeName="Json" displayLabel="JSON Properties" description="A string property that users and/or applications can use to persist ad hoc JSON values.">
            <ECCustomAttributes>
                <HiddenProperty xmlns="CoreCustomAttributes.1.0.0"/>
            </ECCustomAttributes>
        </ECProperty>
    </ECEntityClass>

    <ECEntityClass typeName="bis_GeometricElement" modifier="Abstract" displayLabel="Geometric Element" description="bis:GeometricElement is an abstract base class used to model real world Entities that intrinsically have geometry.">
        <BaseClass>bis_Element</BaseClass>
        <ECCustomAttributes>
            <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0"/>
            <NotSubclassableInReferencingSchemas xmlns="CoreCustomAttributes.1.0.0"/>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="bis_GeometricElement3d" modifier="Abstract" displayLabel="3D Geometric Element" description="bis:GeometricElement3d is an abstract base class used to model real world Entities that intrinsically have 3D geometry.">
        <!-- Base class for elements with 3d geometry -->
        <!-- GeometricElement3d elements are not inherently spatially located, but can be spatially located. -->
        <BaseClass>bis_GeometricElement</BaseClass>
        <ECCustomAttributes>
            <ShareColumns xmlns="ECDbMap.2.0.0">
                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
        <ECProperty propertyName="InSpatialIndex" typeName="boolean" displayLabel="In Spatial Index" description="If true, this element will have an entry in the Spatial Index.">
        </ECProperty>
        <ECProperty propertyName="Origin" typeName="point3d" description="The placement origin of this bis:Element.">
        </ECProperty>
        <ECProperty propertyName="Yaw" typeName="double" description="The Yaw angle (in degrees) of the orientation of this bis:Element.">
        </ECProperty>
        <ECProperty propertyName="Pitch" typeName="double" description="The Pitch angle (in degrees) of the orientation of this bis:Element.">
        </ECProperty>
        <ECProperty propertyName="Roll" typeName="double" description="The Roll angle (in degrees) of the orientation of this bis:Element.">
        </ECProperty>
        <ECProperty propertyName="BBoxLow" typeName="point3d" displayLabel="Bounding Box Low" description="The 'low' point of the element-aligned bounding box of this bis:Element.">
        </ECProperty>
        <ECProperty propertyName="BBoxHigh" typeName="point3d" displayLabel="Bounding Box High" description="The 'high' point of the element-aligned bounding box of this bis:Element.">
        </ECProperty>
        <ECProperty propertyName="GeometryStream" typeName="binary" extendedTypeName="GeometryStream" displayLabel="Geometry Stream" description="Binary stream used to persist the geometry of this bis:Element.">
        </ECProperty>
    </ECEntityClass>
    <ECEntityClass typeName="bis_SpatialElement" modifier="Abstract" displayLabel="Spatial Element" description="A bis:SpatialElement is a bis:GeometricElement3d that occupies real world space.">
        <BaseClass>bis_GeometricElement3d</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="bis_SpatialLocationElement" modifier="Abstract" displayLabel="Spatial Location Element" description="A bis:SpatialLocationElement identifies a 'tracked' real world location but has no mass and cannot be 'touched'.">
        <BaseClass>bis_SpatialElement</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="spcomp_CompositeElement" modifier="Abstract" description="a spatial element that may be Composite of other CompositeElements" >
        <BaseClass>bis_SpatialLocationElement</BaseClass>
        <ECProperty propertyName="FootprintArea" displayLabel="FootprintArea" typeName="double" readOnly="true"/>
    </ECEntityClass>
    <ECEntityClass typeName="spcomp_ICompositeVolume" modifier="Abstract" description="An interface that indicates that the CompositeElement is delimited by a volume">
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                <AppliesToEntityClass>spcomp_CompositeElement</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="buildingSpatial_Building" modifier="None" description="an element modeling the spatial perspective of a building">
        <BaseClass>spcomp_CompositeElement</BaseClass>
        <BaseClass>spcomp_ICompositeVolume</BaseClass>
    </ECEntityClass>)schema"
    R"schema(
    <ECEntityClass typeName="Building" description="Converted to EC3 from DataGroup : Building" displayLabel="Building">
        <BaseClass>buildingSpatial_Building</BaseClass>
        <ECProperty propertyName="Identity_PART" typeName="string" description="Open Buildings Designer part" displayLabel="Part"/>
        <ECProperty propertyName="Identity_FAMILY" typeName="string" description="Open Buildings Designer part family" displayLabel="Family"/>
        <ECProperty propertyName="ArchBuilding_BuildingName" typeName="string" description="Translated from datagroup xpath : ArchBuilding/@BuildingName" displayLabel="Building Name"/>
        <ECProperty propertyName="ArchBuilding_YearConstructed" typeName="string" description="Translated from datagroup xpath : ArchBuilding/@YearConstructed" displayLabel="Year Constructed"/>
        <ECProperty propertyName="ObjectPostalAddress_Address1" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@Address1" displayLabel="Postal | Address Line 1"/>
        <ECProperty propertyName="ObjectPostalAddress_Address2" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@Address2" displayLabel="Postal | Address Line 2"/>
        <ECProperty propertyName="ObjectPostalAddress_Address3" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@Address3" displayLabel="Postal | Address Line 3"/>
        <ECProperty propertyName="ObjectPostalAddress_Address4" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@Address4" displayLabel="Postal | Address Line 4"/>
        <ECProperty propertyName="ObjectPostalAddress_PostalBox" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@PostalBox" displayLabel="Postal | Box" />
        <ECProperty propertyName="ObjectPostalAddress_City" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@City" displayLabel="Postal | City"/>
        <ECProperty propertyName="ObjectPostalAddress_Region" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@Region" displayLabel="Postal | Region"/>
        <ECProperty propertyName="ObjectPostalAddress_PostalCode" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@PostalCode" displayLabel="Postal | Code"/>
        <ECProperty propertyName="ObjectPostalAddress_Country" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@Country" displayLabel="Postal | Country"/>
        <ECProperty propertyName="ObjectIdentity_Mark" typeName="string" description="Translated from datagroup xpath : ObjectIdentity/@Mark" displayLabel="ID | Type ID"/>
        <ECProperty propertyName="ObjectIdentity_InstanceMark" typeName="string" description="Translated from datagroup xpath : ObjectIdentity/@InstanceMark" displayLabel="ID | Item ID"/>
        <ECProperty propertyName="ObjectIdentity_NameAlt" typeName="string" description="Translated from datagroup xpath : ObjectIdentity/@NameAlt" displayLabel="ID | Name (Alternate)"/>
        <ECProperty propertyName="ObjectIdentity_Description" typeName="string" description="Translated from datagroup xpath : ObjectIdentity/@Description" displayLabel="ID | Description"/>
        <ECProperty propertyName="ObjectIdentity_Keynote" typeName="string" description="Translated from datagroup xpath : ObjectIdentity/@Keynote" displayLabel="ID | Keynote"/>
        <ECProperty propertyName="ObjectIdentity_Tag" typeName="string" description="Translated from datagroup xpath : ObjectIdentity/@Tag" displayLabel="ID | Asset Tag"/>
        <ECProperty propertyName="ObjectIdentity_Notes" typeName="string" description="Translated from datagroup xpath : ObjectIdentity/@Notes" displayLabel="ID | Notes"/>
        <ECProperty propertyName="ObjectClassification_MasterFormat" typeName="string" description="Translated from datagroup xpath : ObjectClassification/@MasterFormat" displayLabel="MasterFormat"/>
        <ECProperty propertyName="ObjectClassification_OmniClass" typeName="string" description="Translated from datagroup xpath : ObjectClassification/@OmniClass" displayLabel="OmniClass"/>
        <ECProperty propertyName="ObjectClassification_UniFormat" typeName="string" description="Translated from datagroup xpath : ObjectClassification/@UniFormat" displayLabel="UniFormat"/>

    </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("spatialCompositionNewBaseScenario.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Building (Identity_PART) VALUES ('TEST1')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT Identity_PART FROM TestSchema.Building");
    ASSERT_EQ(JsonValue(R"json([{"Identity_PART":"TEST1"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>

<ECEntityClass typeName="bis_Element" modifier="Abstract" description="A bis:Element is the smallest individually identifiable building block for modeling the real world. Each bis:Element represents an Entity in the real world. Sets of bis:Elements (contained in bis:Models) are used to sub-model other bis:Elements that represent larger scale real world Entities. Using this recursive modeling strategy, bis:Elements can represent Entities at any scale. Elements can represent physical things, abstract concepts or simply be information records.">
        <!-- NOTE: Only platform can directly subclass from Element. Everyone else should pick the most appropriate superclass. -->
        <!-- NOTE: Element subclasses should drop the "Element" suffix from their name once the concept becomes "user facing" -->
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <NotSubclassableInReferencingSchemas xmlns="CoreCustomAttributes.1.0.0"/>
            <DbIndexList xmlns="ECDbMap.2.0.0">
                <Indexes>
                    <DbIndex>
                        <Name>ix_bis_Element_FederationGuid</Name>
                        <IsUnique>True</IsUnique>
                        <Properties>
                            <string>FederationGuid</string>
                        </Properties>
                        <Where>IndexedColumnsAreNotNull</Where>
                    </DbIndex>
                    <DbIndex>
                        <Name>ix_bis_Element_UserLabel</Name>
                        <IsUnique>False</IsUnique>
                        <Properties>
                            <string>UserLabel</string>
                        </Properties>
                        <Where>IndexedColumnsAreNotNull</Where>
                    </DbIndex>
                </Indexes>
            </DbIndexList>
            <ClassHasCurrentTimeStampProperty xmlns="CoreCustomAttributes.1.0.0">
                <PropertyName>LastMod</PropertyName>
            </ClassHasCurrentTimeStampProperty>
        </ECCustomAttributes>
        <ECProperty propertyName="LastMod" typeName="dateTime" readOnly="True" displayLabel="Last Modified" description="The last modified time of the bis:Element. This is maintained by the core framework and should not be set directly by applications.">
            <ECCustomAttributes>
                <DateTimeInfo xmlns="CoreCustomAttributes.1.0.0">
                    <DateTimeKind>Utc</DateTimeKind>
                </DateTimeInfo>
                <HiddenProperty xmlns="CoreCustomAttributes.1.0.0"/>
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="CodeValue" typeName="string" displayLabel="Code" description="The CodeValue property stores the formal name (business key) for a bis:Element. The combination of CodeSpec, CodeScope, and CodeValue properties must be unique for each bis:Element instance.">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.2.0.0">
                    <IsNullable>True</IsNullable>
                    <Collation>NoCase</Collation>
                </PropertyMap>
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="UserLabel" typeName="string" displayLabel="User Label" description="An optional friendly name given by the user (as opposed to the formal name stored in the CodeValue property).">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.2.0.0">
                    <IsNullable>True</IsNullable>
                    <Collation>NoCase</Collation>
                    <IsUnique>False</IsUnique>
                </PropertyMap>
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="FederationGuid" typeName="binary" extendedTypeName="BeGuid" displayLabel="Federation GUID" description="The GUID used to federate this bis:Element across repositories.">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.2.0.0">
                    <IsNullable>True</IsNullable>
                    <IsUnique>True</IsUnique>
                </PropertyMap>
                <HiddenProperty xmlns="CoreCustomAttributes.1.0.0"/>
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="JsonProperties" typeName="string" extendedTypeName="Json" displayLabel="JSON Properties" description="A string property that users and/or applications can use to persist ad hoc JSON values.">
            <ECCustomAttributes>
                <HiddenProperty xmlns="CoreCustomAttributes.1.0.0"/>
            </ECCustomAttributes>
        </ECProperty>
    </ECEntityClass>

    <ECEntityClass typeName="bis_GeometricElement" modifier="Abstract" displayLabel="Geometric Element" description="bis:GeometricElement is an abstract base class used to model real world Entities that intrinsically have geometry.">
        <BaseClass>bis_Element</BaseClass>
        <ECCustomAttributes>
            <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0"/>
            <NotSubclassableInReferencingSchemas xmlns="CoreCustomAttributes.1.0.0"/>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="bis_GeometricElement3d" modifier="Abstract" displayLabel="3D Geometric Element" description="bis:GeometricElement3d is an abstract base class used to model real world Entities that intrinsically have 3D geometry.">
        <!-- Base class for elements with 3d geometry -->
        <!-- GeometricElement3d elements are not inherently spatially located, but can be spatially located. -->
        <BaseClass>bis_GeometricElement</BaseClass>
        <ECCustomAttributes>
            <ShareColumns xmlns="ECDbMap.2.0.0">
                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
        <ECProperty propertyName="InSpatialIndex" typeName="boolean" displayLabel="In Spatial Index" description="If true, this element will have an entry in the Spatial Index.">
        </ECProperty>
        <ECProperty propertyName="Origin" typeName="point3d" description="The placement origin of this bis:Element.">
        </ECProperty>
        <ECProperty propertyName="Yaw" typeName="double" description="The Yaw angle (in degrees) of the orientation of this bis:Element.">
        </ECProperty>
        <ECProperty propertyName="Pitch" typeName="double" description="The Pitch angle (in degrees) of the orientation of this bis:Element.">
        </ECProperty>
        <ECProperty propertyName="Roll" typeName="double" description="The Roll angle (in degrees) of the orientation of this bis:Element.">
        </ECProperty>
        <ECProperty propertyName="BBoxLow" typeName="point3d" displayLabel="Bounding Box Low" description="The 'low' point of the element-aligned bounding box of this bis:Element.">
        </ECProperty>
        <ECProperty propertyName="BBoxHigh" typeName="point3d" displayLabel="Bounding Box High" description="The 'high' point of the element-aligned bounding box of this bis:Element.">
        </ECProperty>
        <ECProperty propertyName="GeometryStream" typeName="binary" extendedTypeName="GeometryStream" displayLabel="Geometry Stream" description="Binary stream used to persist the geometry of this bis:Element.">
        </ECProperty>
    </ECEntityClass>
    <ECEntityClass typeName="bis_SpatialElement" modifier="Abstract" displayLabel="Spatial Element" description="A bis:SpatialElement is a bis:GeometricElement3d that occupies real world space.">
        <BaseClass>bis_GeometricElement3d</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="bis_SpatialLocationElement" modifier="Abstract" displayLabel="Spatial Location Element" description="A bis:SpatialLocationElement identifies a 'tracked' real world location but has no mass and cannot be 'touched'.">
        <BaseClass>bis_SpatialElement</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="spcomp_SpatialStructureElement" modifier="Abstract" displayLabel="Spatial Structure Element" description="An Element used to form a spatial breakdown structure. As an ISpatialOrganizer, it may explicitly 'hold' or 'reference' SpatialElements.">
        <BaseClass>spcomp_CompositeElement</BaseClass>
        <BaseClass>spcomp_ISpatialOrganizer</BaseClass>
        <ECProperty propertyName="Description" typeName="string" description="A human-readable description of this Spatial Structure Element"/>
    </ECEntityClass>
    <ECEntityClass typeName="spcomp_ISpatialOrganizer" modifier="Abstract" displayLabel="Spatial Organizer" description="An bis:SpatialLocation that organizes bis:SpatialElements using 'SpatialOrganizerHoldsSpatialElements' and 'SpatialOrganizerReferencesSpatialElements'">
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                <AppliesToEntityClass>bis_SpatialLocationElement</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
    </ECEntityClass>

    <ECEntityClass typeName="spcomp_Facility" modifier="Abstract" description="A volume occupied by a built facility, such as a building, bridge, road, factory/plant, railway, tunnel, etc.">
        <BaseClass>spcomp_SpatialStructureElement</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="spcomp_CompositeElement" modifier="Abstract" description="a spatial element that may be Composite of other CompositeElements" >
        <BaseClass>bis_SpatialLocationElement</BaseClass>
        <ECProperty propertyName="FootprintArea" displayLabel="FootprintArea" typeName="double" readOnly="true"/>
    </ECEntityClass>
    <ECEntityClass typeName="spcomp_ICompositeVolume" modifier="Abstract" description="An interface that indicates that the CompositeElement is delimited by a volume">
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                <AppliesToEntityClass>spcomp_CompositeElement</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="buildingSpatial_Building" modifier="None" description="an element modeling the spatial perspective of a building">
        <BaseClass>spcomp_Facility</BaseClass>
        <BaseClass>spcomp_ICompositeVolume</BaseClass>
    </ECEntityClass>)schema"
    R"schema(
    <ECEntityClass typeName="Building" description="Converted to EC3 from DataGroup : Building" displayLabel="Building">
        <BaseClass>buildingSpatial_Building</BaseClass>
        <ECProperty propertyName="Identity_PART" typeName="string" description="Open Buildings Designer part" displayLabel="Part"/>
        <ECProperty propertyName="Identity_FAMILY" typeName="string" description="Open Buildings Designer part family" displayLabel="Family"/>
        <ECProperty propertyName="ArchBuilding_BuildingName" typeName="string" description="Translated from datagroup xpath : ArchBuilding/@BuildingName" displayLabel="Building Name"/>
        <ECProperty propertyName="ArchBuilding_YearConstructed" typeName="string" description="Translated from datagroup xpath : ArchBuilding/@YearConstructed" displayLabel="Year Constructed"/>
        <ECProperty propertyName="ObjectPostalAddress_Address1" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@Address1" displayLabel="Postal | Address Line 1"/>
        <ECProperty propertyName="ObjectPostalAddress_Address2" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@Address2" displayLabel="Postal | Address Line 2"/>
        <ECProperty propertyName="ObjectPostalAddress_Address3" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@Address3" displayLabel="Postal | Address Line 3"/>
        <ECProperty propertyName="ObjectPostalAddress_Address4" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@Address4" displayLabel="Postal | Address Line 4"/>
        <ECProperty propertyName="ObjectPostalAddress_PostalBox" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@PostalBox" displayLabel="Postal | Box" />
        <ECProperty propertyName="ObjectPostalAddress_City" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@City" displayLabel="Postal | City"/>
        <ECProperty propertyName="ObjectPostalAddress_Region" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@Region" displayLabel="Postal | Region"/>
        <ECProperty propertyName="ObjectPostalAddress_PostalCode" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@PostalCode" displayLabel="Postal | Code"/>
        <ECProperty propertyName="ObjectPostalAddress_Country" typeName="string" description="Translated from datagroup xpath : ObjectPostalAddress/@Country" displayLabel="Postal | Country"/>
        <ECProperty propertyName="ObjectIdentity_Mark" typeName="string" description="Translated from datagroup xpath : ObjectIdentity/@Mark" displayLabel="ID | Type ID"/>
        <ECProperty propertyName="ObjectIdentity_InstanceMark" typeName="string" description="Translated from datagroup xpath : ObjectIdentity/@InstanceMark" displayLabel="ID | Item ID"/>
        <ECProperty propertyName="ObjectIdentity_NameAlt" typeName="string" description="Translated from datagroup xpath : ObjectIdentity/@NameAlt" displayLabel="ID | Name (Alternate)"/>
        <ECProperty propertyName="ObjectIdentity_Description" typeName="string" description="Translated from datagroup xpath : ObjectIdentity/@Description" displayLabel="ID | Description"/>
        <ECProperty propertyName="ObjectIdentity_Keynote" typeName="string" description="Translated from datagroup xpath : ObjectIdentity/@Keynote" displayLabel="ID | Keynote"/>
        <ECProperty propertyName="ObjectIdentity_Tag" typeName="string" description="Translated from datagroup xpath : ObjectIdentity/@Tag" displayLabel="ID | Asset Tag"/>
        <ECProperty propertyName="ObjectIdentity_Notes" typeName="string" description="Translated from datagroup xpath : ObjectIdentity/@Notes" displayLabel="ID | Notes"/>
        <ECProperty propertyName="ObjectClassification_MasterFormat" typeName="string" description="Translated from datagroup xpath : ObjectClassification/@MasterFormat" displayLabel="MasterFormat"/>
        <ECProperty propertyName="ObjectClassification_OmniClass" typeName="string" description="Translated from datagroup xpath : ObjectClassification/@OmniClass" displayLabel="OmniClass"/>
        <ECProperty propertyName="ObjectClassification_UniFormat" typeName="string" description="Translated from datagroup xpath : ObjectClassification/@UniFormat" displayLabel="UniFormat"/>

    </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Verify old and new instances
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Building (Identity_PART) VALUES ('TEST2')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT Identity_PART FROM TestSchema.Building");
    ASSERT_EQ(JsonValue(R"json([{"Identity_PART":"TEST1"},{"Identity_PART":"TEST2"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, InsertBaseClassHierarchy)
    {
    SchemaItem schemaItem(R"schema(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A" modifier="Abstract" description="A">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.2.0.0">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0"/>
            </ECCustomAttributes>
            <ECProperty propertyName="A1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B" modifier="Abstract">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="B1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="F">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="F1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("insertBaseClassHierarchy.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.F (A1,B1,F1) VALUES ('A11','B11','F11')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT A1,B1,F1 FROM TestSchema.F");
    ASSERT_EQ(JsonValue(R"json([{"A1":"A11","B1":"B11","F1":"F11"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A" modifier="Abstract" description="A">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.2.0.0">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0"/>
            </ECCustomAttributes>
            <ECProperty propertyName="A1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B" modifier="Abstract">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="B1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="C1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="D">
            <BaseClass>C</BaseClass>
            <ECProperty propertyName="D1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="E">
            <BaseClass>D</BaseClass>
            <ECProperty propertyName="E1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="F">
            <BaseClass>E</BaseClass>
            <ECProperty propertyName="F1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    // Verify old and new instances
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.F (A1,B1,C1,D1,E1,F1) VALUES ('A12','B12','C12','D12','E12','F12')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT A1,B1,C1,D1,E1,F1 FROM TestSchema.F");
    ASSERT_EQ(JsonValue(R"json([{"A1":"A11","B1":"B11","F1":"F11"},{"A1":"A12","B1":"B12","C1":"C12","D1":"D12","E1":"E12","F1":"F12"}])json"), result);
    }
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, InsertBaseClassHierarchyAndMovePropertyUp)
    {
    SchemaItem schemaItem(R"schema(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A" modifier="Abstract" description="A">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.2.0.0">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0"/>
            </ECCustomAttributes>
            <ECProperty propertyName="A1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B" modifier="Abstract">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="B1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="F">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="F1" typeName="string" />
            <ECProperty propertyName="F2" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("insertBaseClassHierarchyAndMovePropertyUp.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.F (A1,B1,F1,F2) VALUES ('A11','B11','F11','F21')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT A1,B1,F1,F2 FROM TestSchema.F");
    ASSERT_EQ(JsonValue(R"json([{"A1":"A11","B1":"B11","F1":"F11","F2":"F21"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A" modifier="Abstract" description="A">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.2.0.0">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0"/>
            </ECCustomAttributes>
            <ECProperty propertyName="A1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B" modifier="Abstract">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="B1" typeName="string" />
            <ECProperty propertyName="F2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="C1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="D">
            <BaseClass>C</BaseClass>
            <ECProperty propertyName="D1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="E">
            <BaseClass>D</BaseClass>
            <ECProperty propertyName="E1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="F">
            <BaseClass>E</BaseClass>
            <ECProperty propertyName="F1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    // Verify old and new instances
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.F (A1,B1,C1,D1,E1,F1,F2) VALUES ('A12','B12','C12','D12','E12','F12','F22')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT A1,B1,C1,D1,E1,F1,F2 FROM TestSchema.F");
    ASSERT_EQ(JsonValue(R"json([{"A1":"A11","B1":"B11","F1":"F11","F2":"F21"},{"A1":"A12","B1":"B12","C1":"C12","D1":"D12","E1":"E12","F1":"F12","F2":"F22"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MovePropertyFromMixin)
    {
    SchemaItem schemaItem(R"schema(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
          </ECEntityClass>
          <ECEntityClass typeName="IMyMixin" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                    <AppliesToEntityClass>MyBaseClass</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <BaseClass>IMyMixin</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertyFromMixin.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('FIRST')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="IMyMixin" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                    <AppliesToEntityClass>MyBaseClass</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <BaseClass>IMyMixin</BaseClass>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));

    // Verify old and new instances
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty) VALUES ('SECOND')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST"},{"MovingProperty":"SECOND"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, InvalidRootPropertyId)
    {
    //Copied from another test which produced assertions in PropertyMap::GetRootPropertyId() in debug builds. Behavior should be fixed now,
    // the other test has been altered and no longer reflects the original scenario
    // The problem is that when cleaning up overridden properties, we access PropertyMapping.RootPropertyId, which in this case raised an Assertion as
    // We insert both, a new base class AND an override at the same time. Has been fixed by adding a new TryGetRootPropertyId method.
    SchemaItem schemaItem(R"schema(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECEntityClass typeName="AB">
              <ECProperty propertyName="a" typeName="double" />
              <ECProperty propertyName="b" typeName="double" />
          </ECEntityClass>
          <ECEntityClass typeName="ICD" modifier="Abstract">
              <ECCustomAttributes>
                  <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                      <AppliesToEntityClass>AB</AppliesToEntityClass>
                  </IsMixin>
              </ECCustomAttributes>
              <ECProperty propertyName="c" typeName="double" />
              <ECProperty propertyName="d" typeName="double" />
          </ECEntityClass>
          <ECEntityClass typeName="EF">
              <BaseClass>AB</BaseClass>
              <BaseClass>ICD</BaseClass>
              <ECProperty propertyName="e" typeName="double" />
              <ECProperty propertyName="f" typeName="double" />
          </ECEntityClass>
          <ECEntityClass typeName="IGH" modifier="Abstract">
              <ECCustomAttributes>
                  <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                      <AppliesToEntityClass>AB</AppliesToEntityClass>
                  </IsMixin>
              </ECCustomAttributes>
              <ECProperty propertyName="g" typeName="double" />
              <ECProperty propertyName="h" typeName="double" />
          </ECEntityClass>
          <ECEntityClass typeName="IIJ" modifier="Abstract">
              <ECCustomAttributes>
                  <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                      <AppliesToEntityClass>AB</AppliesToEntityClass>
                  </IsMixin>
              </ECCustomAttributes>
              <ECProperty propertyName="i" typeName="double" />
              <ECProperty propertyName="j" typeName="double" />
          </ECEntityClass>
          <ECEntityClass typeName="IKL" modifier="Abstract">
              <ECCustomAttributes>
                  <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                      <AppliesToEntityClass>AB</AppliesToEntityClass>
                  </IsMixin>
              </ECCustomAttributes>
              <BaseClass>IGH</BaseClass>
              <ECProperty propertyName="k" typeName="double" />
              <ECProperty propertyName="l" typeName="double" />
          </ECEntityClass>
          <ECEntityClass typeName="MN">
              <BaseClass>EF</BaseClass>
              <BaseClass>IIJ</BaseClass>
              <BaseClass>IKL</BaseClass>
              <ECProperty propertyName="m" typeName="double" />
              <ECProperty propertyName="n" typeName="double" />
              <ECProperty propertyName="b" typeName="double" />
              <ECProperty propertyName="d" typeName="double" />
              <ECProperty propertyName="f" typeName="double" />
              <ECProperty propertyName="h" typeName="double" />
              <ECProperty propertyName="j" typeName="double" />
              <ECProperty propertyName="k" typeName="double" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("invalidRootPropertyId.ecdb", schemaItem));

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECEntityClass typeName="AB">
              <ECProperty propertyName="a" typeName="double" />
              <ECProperty propertyName="b" typeName="double" />
              <ECProperty propertyName="g" typeName="double" />
              <ECProperty propertyName="h" typeName="double" />
          </ECEntityClass>
          <ECEntityClass typeName="ICD" modifier="Abstract">
                <ECCustomAttributes>
                  <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                      <AppliesToEntityClass>AB</AppliesToEntityClass>
                  </IsMixin>
              </ECCustomAttributes>
              <ECProperty propertyName="c" typeName="double" />
              <ECProperty propertyName="d" typeName="double" />
          </ECEntityClass>
          <ECEntityClass typeName="EF">
              <BaseClass>AB</BaseClass>
              <BaseClass>ICD</BaseClass>
              <ECProperty propertyName="e" typeName="double" />
              <ECProperty propertyName="f" typeName="double" />
              <ECProperty propertyName="l" typeName="double" />
          </ECEntityClass>
          <ECEntityClass typeName="IGH" modifier="Abstract">
                <ECCustomAttributes>
                  <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                      <AppliesToEntityClass>AB</AppliesToEntityClass>
                  </IsMixin>
              </ECCustomAttributes>
              <ECProperty propertyName="g" typeName="double" />
              <ECProperty propertyName="h" typeName="double" />
              <ECProperty propertyName="a" typeName="double" />
              <ECProperty propertyName="b" typeName="double" />
              <ECProperty propertyName="i" typeName="double" />
          </ECEntityClass>
          <ECEntityClass typeName="IIJ" modifier="Abstract">
              <ECCustomAttributes>
                  <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                      <AppliesToEntityClass>AB</AppliesToEntityClass>
                  </IsMixin>
              </ECCustomAttributes>
              <ECProperty propertyName="i" typeName="double" />
              <ECProperty propertyName="j" typeName="double" />
              <ECProperty propertyName="g" typeName="double" />
          </ECEntityClass>
          <ECEntityClass typeName="IKL" modifier="Abstract">
              <ECCustomAttributes>
                  <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                      <AppliesToEntityClass>AB</AppliesToEntityClass>
                  </IsMixin>
              </ECCustomAttributes>
              <BaseClass>IGH</BaseClass>
              <ECProperty propertyName="k" typeName="double" />
              <ECProperty propertyName="l" typeName="double" />
              <ECProperty propertyName="e" typeName="double" />
              <ECProperty propertyName="a" typeName="double" />
              <ECProperty propertyName="c" typeName="double" />
              <ECProperty propertyName="g" typeName="double" />
          </ECEntityClass>
          <ECEntityClass typeName="MN">
              <BaseClass>EF</BaseClass>
              <BaseClass>IIJ</BaseClass>
              <BaseClass>IKL</BaseClass>
              <ECProperty propertyName="m" typeName="double" />
              <ECProperty propertyName="n" typeName="double" />
              <ECProperty propertyName="b" typeName="double" />
              <ECProperty propertyName="d" typeName="double" />
              <ECProperty propertyName="f" typeName="double" />
              <ECProperty propertyName="h" typeName="double" />
              <ECProperty propertyName="j" typeName="double" />
              <ECProperty propertyName="k" typeName="double" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem));
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MoveMultiplePropertiesUp)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="BisElement">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                  <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="BisElement1" typeName="string" />
            <ECProperty propertyName="BisElement2" typeName="string" />
            <ECProperty propertyName="BisElement3" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="DomainElement">
            <BaseClass>BisElement</BaseClass>
            <ECProperty propertyName="DomainElement1" typeName="string" />
            <ECProperty propertyName="DomainElement2" typeName="string" />
            <ECProperty propertyName="DomainElement3" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="SubElement1">
            <BaseClass>DomainElement</BaseClass>
            <ECProperty propertyName="Sub11" typeName="string" />
            <ECProperty propertyName="Sub12" typeName="string" />
            <ECProperty propertyName="Sub13" typeName="string" />
            <ECProperty propertyName="SubShared" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="SubElement2">
            <BaseClass>DomainElement</BaseClass>
            <ECProperty propertyName="Sub21" typeName="string" />
            <ECProperty propertyName="Sub22" typeName="string" />
            <ECProperty propertyName="Sub23" typeName="string" />
            <ECProperty propertyName="SubShared" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="SubElement3">
            <BaseClass>DomainElement</BaseClass>
            <ECProperty propertyName="Sub31" typeName="string" />
            <ECProperty propertyName="Sub32" typeName="string" />
            <ECProperty propertyName="Sub33" typeName="string" />
            <ECProperty propertyName="SubShared" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="SubElement4">
            <BaseClass>DomainElement</BaseClass>
            <ECProperty propertyName="Sub41" typeName="string" />
            <ECProperty propertyName="Sub42" typeName="string" />
            <ECProperty propertyName="Sub43" typeName="string" />
            <ECProperty propertyName="Sub44" typeName="string" />
            <ECProperty propertyName="Sub45" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("moveMultiplePropertiesUp.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.SubElement1 (Sub11,Sub12,Sub13,SubShared) VALUES ('Sub11A', 'Sub12A','Sub13A','SubSharedA')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.SubElement4 (Sub41,Sub42,Sub43,Sub44,Sub45) VALUES ('Sub41B', 'Sub42B','Sub43B','Sub44B','Sub45B')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT Sub11,Sub12,Sub13,SubShared FROM TestSchema.SubElement1");
    ASSERT_EQ(JsonValue(R"json([{"Sub11":"Sub11A","Sub12":"Sub12A","Sub13":"Sub13A","SubShared":"SubSharedA"}])json"), result);

    result = GetHelper().ExecuteSelectECSql("SELECT Sub41,Sub42,Sub43,Sub44,Sub45 FROM TestSchema.SubElement4");
    ASSERT_EQ(JsonValue(R"json([{"Sub41":"Sub41B","Sub42":"Sub42B","Sub43":"Sub43B","Sub44":"Sub44B","Sub45":"Sub45B"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="BisElement">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                  <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="BisElement1" typeName="string" />
            <ECProperty propertyName="BisElement2" typeName="string" />
            <ECProperty propertyName="BisElement3" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="DomainElement">
            <BaseClass>BisElement</BaseClass>
            <ECProperty propertyName="DomainElement1" typeName="string" />
            <ECProperty propertyName="DomainElement2" typeName="string" />
            <ECProperty propertyName="DomainElement3" typeName="string" />
            <ECProperty propertyName="SubShared" typeName="string" />
            <ECProperty propertyName="Sub45" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="SubElement1">
            <BaseClass>DomainElement</BaseClass>
            <ECProperty propertyName="Sub11" typeName="string" />
            <ECProperty propertyName="Sub12" typeName="string" />
            <ECProperty propertyName="Sub13" typeName="string" />
            <ECProperty propertyName="SubShared" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="SubElement2">
            <BaseClass>DomainElement</BaseClass>
            <ECProperty propertyName="Sub21" typeName="string" />
            <ECProperty propertyName="Sub22" typeName="string" />
            <ECProperty propertyName="Sub23" typeName="string" />
            <ECProperty propertyName="SubShared" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="SubElement3">
            <BaseClass>DomainElement</BaseClass>
            <ECProperty propertyName="Sub31" typeName="string" />
            <ECProperty propertyName="Sub32" typeName="string" />
            <ECProperty propertyName="Sub33" typeName="string" />
            <ECProperty propertyName="Sub45" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="SubElement4">
            <BaseClass>DomainElement</BaseClass>
            <ECProperty propertyName="Sub41" typeName="string" />
            <ECProperty propertyName="Sub42" typeName="string" />
            <ECProperty propertyName="Sub43" typeName="string" />
            <ECProperty propertyName="Sub44" typeName="string" />
            <ECProperty propertyName="Sub45" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Verify instances are still intact
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT Sub11,Sub12,Sub13,SubShared FROM TestSchema.SubElement1");
    ASSERT_EQ(JsonValue(R"json([{"Sub11":"Sub11A","Sub12":"Sub12A","Sub13":"Sub13A","SubShared":"SubSharedA"}])json"), result);

    result = GetHelper().ExecuteSelectECSql("SELECT Sub41,Sub42,Sub43,Sub44,Sub45 FROM TestSchema.SubElement4");
    ASSERT_EQ(JsonValue(R"json([{"Sub41":"Sub41B","Sub42":"Sub42B","Sub43":"Sub43B","Sub44":"Sub44B","Sub45":"Sub45B"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MovePropertyToOverflow)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Base1" typeName="string" />
            <ECProperty propertyName="Base2" typeName="string" />
            <ECProperty propertyName="Base3" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="A1" typeName="string" />
            <ECProperty propertyName="A2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="B1" typeName="string" />
            <ECProperty propertyName="B2" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertyToOverflow.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (Base1,Base2,Base3,A1,A2) VALUES ('Base1','Base2','Base3','A1','A2')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT Base1,Base2,Base3,A1,A2 FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"Base1":"Base1","Base2":"Base2","Base3":"Base3","A1":"A1","A2":"A2"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Base1" typeName="string" />
            <ECProperty propertyName="Base2" typeName="string" />
            <ECProperty propertyName="Base3" typeName="string" />
            <ECProperty propertyName="A1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="A2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="B1" typeName="string" />
            <ECProperty propertyName="B2" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT Base1,Base2,Base3,A1,A2 FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"Base1":"Base1","Base2":"Base2","Base3":"Base3","A1":"A1","A2":"A2"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MovePropertyFromOverflow)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
                  <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Base1" typeName="string" />
            <ECProperty propertyName="Base2" typeName="string" />
            <ECProperty propertyName="Base3" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="A1" typeName="string" />
            <ECProperty propertyName="A2" typeName="string" />
            <ECProperty propertyName="A3" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="B1" typeName="string" />
            <ECProperty propertyName="B2" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertyFromOverflow.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (Base1,Base2,Base3,A1,A2,A3) VALUES ('Base1','Base2','Base3','A1','A2','A3')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT Base1,Base2,Base3,A1,A2,A3 FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"Base1":"Base1","Base2":"Base2","Base3":"Base3","A1":"A1","A2":"A2","A3":"A3"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
                  <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Base1" typeName="string" />
            <ECProperty propertyName="Base2" typeName="string" />
            <ECProperty propertyName="Base3" typeName="string" />
            <ECProperty propertyName="A2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="A1" typeName="string" />
            <ECProperty propertyName="A3" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="B1" typeName="string" />
            <ECProperty propertyName="B2" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT Base1,Base2,Base3,A1,A2,A3 FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"Base1":"Base1","Base2":"Base2","Base3":"Base3","A1":"A1","A2":"A2","A3":"A3"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, SwapColumnsWithOverflow)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>
                  <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Element">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="Peanut">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="A" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="C" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Potato">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="C" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="A" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("swapColumnsWithOverflow.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Peanut (A,B,C) VALUES ('PA','PB','PC')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT A,B,C FROM TestSchema.Peanut");
    ASSERT_EQ(JsonValue(R"json([{"A":"PA","B":"PB","C":"PC"}])json"), result);
    }

    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Potato (A,B,C) VALUES ('PoA','PoB','PoC')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT A,B,C FROM TestSchema.Potato");
    ASSERT_EQ(JsonValue(R"json([{"A":"PoA","B":"PoB","C":"PoC"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>
                  <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Element">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="A" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="C" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Peanut">
            <BaseClass>Element</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="Potato">
            <BaseClass>Element</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem));

    //The below part is the expected result, in case we start supporting this scenario:
    /*{
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Peanut (A,B,C) VALUES ('PA','PB','PC')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT A,B,C FROM TestSchema.Peanut");
    ASSERT_EQ(JsonValue(R"json([{"A":"PA","B":"PB","C":"PC"}])json"), result);
    }

    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Potato (A,B,C) VALUES ('PoA','PoB','PoC')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT A,B,C FROM TestSchema.Potato");
    ASSERT_EQ(JsonValue(R"json([{"A":"PoA","B":"PoB","C":"PoC"}])json"), result);
    }*/
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MovePropertyFromOverflowDropOverflowTable)
    {
    //This is currently unsupported. The schema update will return an error and say that there is an overflow table with no data in it.
    //This is a very rare scenario that should be supported with a future update.
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
                  <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Base1" typeName="string" />
            <ECProperty propertyName="Base2" typeName="string" />
            <ECProperty propertyName="Base3" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="A1" typeName="string" />
            <ECProperty propertyName="A2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="B1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertyFromOverflowDropOverflowTable.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (Base1,Base2,Base3,A1,A2) VALUES ('Base1','Base2','Base3','A1','A2')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT Base1,Base2,Base3,A1,A2 FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"Base1":"Base1","Base2":"Base2","Base3":"Base3","A1":"A1","A2":"A2"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
                  <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Base1" typeName="string" />
            <ECProperty propertyName="Base2" typeName="string" />
            <ECProperty propertyName="Base3" typeName="string" />
            <ECProperty propertyName="A2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="A1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="B1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(ERROR, ImportSchema(editedSchemaItem)) << "Import should fail because it leaves an overflow table with no data in it.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, SwapColumnsForProperty)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Base1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="Sub1" typeName="string" />
            <ECProperty propertyName="Sub2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="Sub2" typeName="string" />
            <ECProperty propertyName="Sub1" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("swapPropertyColumns.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (Base1,Sub1,Sub2) VALUES ('ABase1','ASub1','ASub2')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT Base1,Sub1,Sub2 FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"Base1":"ABase1","Sub1":"ASub1","Sub2":"ASub2"}])json"), result);
    }
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.B (Base1,Sub1,Sub2) VALUES ('BBase1','BSub1','BSub2')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT Base1,Sub1,Sub2 FROM TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"Base1":"BBase1","Sub1":"BSub1","Sub2":"BSub2"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Base1" typeName="string" />
            <ECProperty propertyName="Sub2" typeName="string" />
            <ECProperty propertyName="Sub1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT Base1,Sub1,Sub2 FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"Base1":"ABase1","Sub1":"ASub1","Sub2":"ASub2"}])json"), result);
    }
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT Base1,Sub1,Sub2 FROM TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"Base1":"BBase1","Sub1":"BSub1","Sub2":"BSub2"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MoveMultiplePropertiesInCircle)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Class1">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="A" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="C" typeName="string" />
            <ECProperty propertyName="D" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Class2">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="D" typeName="string" />
            <ECProperty propertyName="C" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="A" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Class3">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="C" typeName="string" />
            <ECProperty propertyName="A" typeName="string" />
            <ECProperty propertyName="D" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("moveMultiplePropertiesInCircle.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Class1 (A,B,C,D) VALUES ('1A','1B','1C','1D')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class1");
    ASSERT_EQ(JsonValue(R"json([{"A":"1A","B":"1B","C":"1C","D":"1D"}])json"), result);
    }
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Class2 (A,B,C,D) VALUES ('2A','2B','2C','2D')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class2");
    ASSERT_EQ(JsonValue(R"json([{"A":"2A","B":"2B","C":"2C","D":"2D"}])json"), result);
    }
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Class3 (A,B,C,D) VALUES ('3A','3B','3C','3D')");
    auto result = GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class3");
    ASSERT_EQ(JsonValue(R"json([{"A":"3A","B":"3B","C":"3C","D":"3D"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="A" typeName="string" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="C" typeName="string" />
            <ECProperty propertyName="D" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Class1">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="Class2">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="Class3">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class1");
    ASSERT_EQ(JsonValue(R"json([{"A":"1A","B":"1B","C":"1C","D":"1D"}])json"), result);
    }
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class2");
    ASSERT_EQ(JsonValue(R"json([{"A":"2A","B":"2B","C":"2C","D":"2D"}])json"), result);
    }
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT A,B,C,D FROM TestSchema.Class3");
    ASSERT_EQ(JsonValue(R"json([{"A":"3A","B":"3B","C":"3C","D":"3D"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MovePropertyToOverflowUsingDifferentIdColumn)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
                <ECInstanceIdColumn>CustomId</ECInstanceIdColumn>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Base1" typeName="string" />
            <ECProperty propertyName="Base2" typeName="string" />
            <ECProperty propertyName="Base3" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="A1" typeName="string" />
            <ECProperty propertyName="A2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="B1" typeName="string" />
            <ECProperty propertyName="B2" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("movePropertyToOverflowUsingDifferentIdColumn.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (Base1,Base2,Base3,A1,A2) VALUES ('Base1','Base2','Base3','A1','A2')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT Base1,Base2,Base3,A1,A2 FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"Base1":"Base1","Base2":"Base2","Base3":"Base3","A1":"A1","A2":"A2"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
                <ECInstanceIdColumn>CustomId</ECInstanceIdColumn>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Base1" typeName="string" />
            <ECProperty propertyName="Base2" typeName="string" />
            <ECProperty propertyName="Base3" typeName="string" />
            <ECProperty propertyName="A1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="A2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="B1" typeName="string" />
            <ECProperty propertyName="B2" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT Base1,Base2,Base3,A1,A2 FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"Base1":"Base1","Base2":"Base2","Base3":"Base3","A1":"A1","A2":"A2"}])json"), result);
    }
    }


//-------------------------------------------------------------------rt e--------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, CivilProblemMay21)
    { //this is a replica to reproduce issue FID-01064. https://connect-projectforms.bentley.com/#/d55308d4-8f5b-47ff-805c-d319b98b8406/type/Field%20Data/form/5c146214-ebc0-455f-9fe1-43fe11b4387f?discipline=issue
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>

          <ECEntityClass typeName="GeometricElement" modifier="Abstract" displayLabel="Geometric Element" description="Geometric Element is an abstract base class used to model real world entities that intrinsically have geometry.">
              <BaseClass>Element</BaseClass>
              <ECCustomAttributes>
                <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                <NotSubclassableInReferencingSchemas xmlns="CoreCustomAttributes.01.00.00"/>
              </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="GeometricElement3d" modifier="Abstract" displayLabel="3D Geometric Element" description="3D Geometric Element is an abstract base class used to model real world entities that intrinsically have 3D geometry.">
              <!-- Base class for elements with 3d geometry -->
              <!-- GeometricElement3d elements are not inherently spatially located, but can be spatially located. -->
              <BaseClass>GeometricElement</BaseClass>
              <ECCustomAttributes>
                  <ShareColumns xmlns="ECDbMap.02.00.00">
                      <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                      <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                  </ShareColumns>
              </ECCustomAttributes>
              <ECProperty propertyName="InSpatialIndex" typeName="boolean" displayLabel="In Spatial Index" description="If true, this element will have an entry in the Spatial Index."/>
              <ECProperty propertyName="Origin" typeName="point3d" description="The placement origin of the Element."/>
              <ECProperty propertyName="Yaw" typeName="double" description="The Yaw angle (in degrees) of the orientation of the Element."/>
              <ECProperty propertyName="Pitch" typeName="double" description="The Pitch angle (in degrees) of the orientation of the Element."/>
              <ECProperty propertyName="Roll" typeName="double" description="The Roll angle (in degrees) of the orientation of the Element."/>
              <ECProperty propertyName="BBoxLow" typeName="point3d" displayLabel="Bounding Box Low" description="The 'low' point of the element-aligned bounding box of the Element."/>
              <ECProperty propertyName="BBoxHigh" typeName="point3d" displayLabel="Bounding Box High" description="The 'high' point of the element-aligned bounding box of the Element."/>
              <ECProperty propertyName="GeometryStream" typeName="binary" extendedTypeName="GeometryStream" displayLabel="Geometry Stream" description="Binary stream used to persist the geometry of this Element."/>
          </ECEntityClass>
          <ECEntityClass typeName="GraphicalElement3d" modifier="Abstract" displayLabel="3D Graphical Element">
              <!-- 3D geometric element that is used to convey information within 3D graphical presentations. -->
              <BaseClass>GeometricElement3d</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="SpatialElement" modifier="Abstract" displayLabel="Spatial Element" description="A Spatial Element occupies real world space.">
              <BaseClass>GeometricElement3d</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="PhysicalElement" modifier="Abstract" displayLabel="Physical Element" description="A Physical Element is spatially located, has mass, and can be 'touched'.">
              <BaseClass>SpatialElement</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="PhysicalPortion" modifier="Abstract" displayLabel="Physical Portion" description="A Physical Portion represents an arbitrary portion of a larger Physical Element that will be broken down in more detail in a separate (sub) Physical Model.">
              <BaseClass>PhysicalElement</BaseClass>
              <BaseClass>ISubModeledElement</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="ISubModeledElement" modifier="Abstract" displayLabel="Modellable Element" description="An interface which indicates that an Element can be broken down or described by a (sub) Model.  This interface is mutually exclusive with IParentElement.">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                        <!-- Only subclasses of bis:Element can implement the ISubModeledElement interface -->
                        <AppliesToEntityClass>Element</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="SpatialLocationElement" modifier="Abstract" displayLabel="Spatial Location Element" description="A Spatial Location Element identifies a 'tracked' real world location but has no mass and cannot be 'touched'.">
              <BaseClass>SpatialElement</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="SpatialLocationPortion" modifier="Abstract" displayLabel="Spatial Location Portion" description="A Spatial Location Portion represents an arbitrary portion of a larger Spatial Location Element that will be broken down in more detail in a separate (sub) Spatial Location Model.">
              <BaseClass>SpatialLocationElement</BaseClass>
              <BaseClass>ISubModeledElement</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="VolumeElement" displayLabel="Volume Element" description="A Volume Element is a Spatial Location Element that is restricted to defining a volume.">
              <BaseClass>SpatialLocationElement</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="Element" modifier="Abstract" >
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
            </ECCustomAttributes>
            <ECProperty propertyName="BisElement1" typeName="string" />
            <ECProperty propertyName="BisElement2" typeName="string" />
            <ECProperty propertyName="BisElement3" typeName="string" />
            <ECProperty propertyName="BisElement4" typeName="string" />
            <ECProperty propertyName="BisElement5" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="CivilObject">
              <BaseClass>PhysicalElement</BaseClass>
              <ECProperty propertyName="SyncId" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="Entity">
              <BaseClass>CivilObject</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="NamedEntity" displayLabel="Named Entity">
              <BaseClass>Entity</BaseClass>
              <ECProperty propertyName="Name" typeName="string"/>
              <ECProperty propertyName="ShortName" typeName="string" displayLabel="Short Name"/>
          </ECEntityClass>
          <ECEntityClass typeName="FeaturizedEntity" displayLabel="Featurized Entity">
              <BaseClass>NamedEntity</BaseClass>
              <ECProperty propertyName="FeatureName" typeName="string" displayLabel="Feature Name"/>
              <ECProperty propertyName="DoNotAnnotate" typeName="boolean" displayLabel="Do Not Annotate"/>
              <ECProperty propertyName="FeatureDescription" typeName="string" displayLabel="Feature Description"/>
          </ECEntityClass>
          <ECEntityClass typeName="Alignment">
              <BaseClass>FeaturizedEntity</BaseClass>
              <ECProperty propertyName="Geometry" typeName="Bentley.Geometry.Common.IGeometry"/>
              <ECProperty propertyName="GeometryClass" typeName="int" displayLabel="Geometry Type"/>
              <ECProperty propertyName="State" typeName="int" displayLabel="Is Valid"/>
          </ECEntityClass>
          <ECEntityClass typeName="Corridor">
              <BaseClass>FeaturizedEntity</BaseClass>
              <ECProperty propertyName="Geometry" typeName="Bentley.Geometry.Common.IGeometry" displayLabel="Exterior Boundary"/>
              <ECProperty propertyName="MaximumInterval" typeName="double" displayLabel="Maximum Interval"/>
              <ECProperty propertyName="XmlFragment" typeName="string" displayLabel="XmlFragment"/>
          </ECEntityClass>
          <ECEntityClass typeName="Profile">
              <BaseClass>FeaturizedEntity</BaseClass>
              <ECProperty propertyName="Geometry" typeName="Bentley.Geometry.Common.IGeometry"/>
              <ECProperty propertyName="GeometryClass" typeName="int" displayLabel="Geometry Type"/>
              <ECProperty propertyName="State" typeName="int" displayLabel="Is Valid"/>
          </ECEntityClass>
          <ECEntityClass typeName="AlignmentProxy">
              <BaseClass>Alignment</BaseClass>
              <ECProperty propertyName="IsReversed" typeName="boolean"/>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("civilProblemMay21.ecdb", schemaItem));

    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Alignment (Name,GeometryClass) VALUES ('First Alignment', 1)");

    auto result = GetHelper().ExecuteSelectECSql("SELECT Name, GeometryClass FROM ONLY TestSchema.Alignment");
    ASSERT_EQ(JsonValue(R"json([{"Name":"First Alignment","GeometryClass":1}])json"), result);
    }

    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.AlignmentProxy (Name,GeometryClass) VALUES ('First AlignmentProxy', 2)");

    auto result = GetHelper().ExecuteSelectECSql("SELECT Name, GeometryClass FROM ONLY TestSchema.AlignmentProxy");
    ASSERT_EQ(JsonValue(R"json([{"Name":"First AlignmentProxy","GeometryClass":2}])json"), result);
    }

    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Corridor (Name) VALUES ('First Corridor')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT Name FROM ONLY TestSchema.Corridor");
    ASSERT_EQ(JsonValue(R"json([{"Name":"First Corridor"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>

          <ECEntityClass typeName="GeometricElement" modifier="Abstract" displayLabel="Geometric Element" description="Geometric Element is an abstract base class used to model real world entities that intrinsically have geometry.">
              <BaseClass>Element</BaseClass>
              <ECCustomAttributes>
                <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                <NotSubclassableInReferencingSchemas xmlns="CoreCustomAttributes.01.00.00"/>
              </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="GeometricElement3d" modifier="Abstract" displayLabel="3D Geometric Element" description="3D Geometric Element is an abstract base class used to model real world entities that intrinsically have 3D geometry.">
              <!-- Base class for elements with 3d geometry -->
              <!-- GeometricElement3d elements are not inherently spatially located, but can be spatially located. -->
              <BaseClass>GeometricElement</BaseClass>
              <ECCustomAttributes>
                  <ShareColumns xmlns="ECDbMap.02.00.00">
                      <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                      <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                  </ShareColumns>
              </ECCustomAttributes>
              <ECProperty propertyName="InSpatialIndex" typeName="boolean" displayLabel="In Spatial Index" description="If true, this element will have an entry in the Spatial Index."/>
              <ECProperty propertyName="Origin" typeName="point3d" description="The placement origin of the Element."/>
              <ECProperty propertyName="Yaw" typeName="double" description="The Yaw angle (in degrees) of the orientation of the Element."/>
              <ECProperty propertyName="Pitch" typeName="double" description="The Pitch angle (in degrees) of the orientation of the Element."/>
              <ECProperty propertyName="Roll" typeName="double" description="The Roll angle (in degrees) of the orientation of the Element."/>
              <ECProperty propertyName="BBoxLow" typeName="point3d" displayLabel="Bounding Box Low" description="The 'low' point of the element-aligned bounding box of the Element."/>
              <ECProperty propertyName="BBoxHigh" typeName="point3d" displayLabel="Bounding Box High" description="The 'high' point of the element-aligned bounding box of the Element."/>
              <ECProperty propertyName="GeometryStream" typeName="binary" extendedTypeName="GeometryStream" displayLabel="Geometry Stream" description="Binary stream used to persist the geometry of this Element."/>
          </ECEntityClass>
          <ECEntityClass typeName="GraphicalElement3d" modifier="Abstract" displayLabel="3D Graphical Element">
              <!-- 3D geometric element that is used to convey information within 3D graphical presentations. -->
              <BaseClass>GeometricElement3d</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="SpatialElement" modifier="Abstract" displayLabel="Spatial Element" description="A Spatial Element occupies real world space.">
              <BaseClass>GeometricElement3d</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="PhysicalElement" modifier="Abstract" displayLabel="Physical Element" description="A Physical Element is spatially located, has mass, and can be 'touched'.">
              <BaseClass>SpatialElement</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="PhysicalPortion" modifier="Abstract" displayLabel="Physical Portion" description="A Physical Portion represents an arbitrary portion of a larger Physical Element that will be broken down in more detail in a separate (sub) Physical Model.">
              <BaseClass>PhysicalElement</BaseClass>
              <BaseClass>ISubModeledElement</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="ISubModeledElement" modifier="Abstract" displayLabel="Modellable Element" description="An interface which indicates that an Element can be broken down or described by a (sub) Model.  This interface is mutually exclusive with IParentElement.">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                        <!-- Only subclasses of bis:Element can implement the ISubModeledElement interface -->
                        <AppliesToEntityClass>Element</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="SpatialLocationElement" modifier="Abstract" displayLabel="Spatial Location Element" description="A Spatial Location Element identifies a 'tracked' real world location but has no mass and cannot be 'touched'.">
              <BaseClass>SpatialElement</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="SpatialLocationPortion" modifier="Abstract" displayLabel="Spatial Location Portion" description="A Spatial Location Portion represents an arbitrary portion of a larger Spatial Location Element that will be broken down in more detail in a separate (sub) Spatial Location Model.">
              <BaseClass>SpatialLocationElement</BaseClass>
              <BaseClass>ISubModeledElement</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="VolumeElement" displayLabel="Volume Element" description="A Volume Element is a Spatial Location Element that is restricted to defining a volume.">
              <BaseClass>SpatialLocationElement</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="Element" modifier="Abstract" >
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
            </ECCustomAttributes>
            <ECProperty propertyName="BisElement1" typeName="string" />
            <ECProperty propertyName="BisElement2" typeName="string" />
            <ECProperty propertyName="BisElement3" typeName="string" />
            <ECProperty propertyName="BisElement4" typeName="string" />
            <ECProperty propertyName="BisElement5" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="FeaturizedEntity" displayLabel="Featurized Entity">
              <BaseClass>NamedEntity</BaseClass>
              <ECProperty propertyName="FeatureName" typeName="string" displayLabel="Feature Name"/>
              <ECProperty propertyName="DoNotAnnotate" typeName="boolean" displayLabel="Do Not Annotate"/>
              <ECProperty propertyName="FeatureDescription" typeName="string" displayLabel="Feature Description"/>
              <ECProperty propertyName="GeometryClass" typeName="int" displayLabel="Geometry Type"/>
              <ECProperty propertyName="State" typeName="int" displayLabel="Is Valid"/>
          </ECEntityClass>
          <ECEntityClass typeName="NamedEntity" displayLabel="Named Entity">
              <BaseClass>Entity</BaseClass>
              <ECProperty propertyName="Name" typeName="string">
              </ECProperty>
              <ECProperty propertyName="ShortName" typeName="string" displayLabel="Short Name"/>
          </ECEntityClass>
          <ECEntityClass typeName="Entity">
              <BaseClass>CivilObject</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="CivilObject">
              <BaseClass>PhysicalElement</BaseClass>
              <ECProperty propertyName="SyncId" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="Alignment">
              <BaseClass>FeaturizedEntity</BaseClass>
              <ECProperty propertyName="Geometry" typeName="Bentley.Geometry.Common.IGeometry"/>
              <ECProperty propertyName="GeometryClass" typeName="int" displayLabel="Geometry Type"/>
              <ECProperty propertyName="State" typeName="int" displayLabel="Is Valid"/>
          </ECEntityClass>
          <ECEntityClass typeName="Corridor">
              <BaseClass>FeaturizedEntity</BaseClass>
              <ECProperty propertyName="Geometry" typeName="Bentley.Geometry.Common.IGeometry" displayLabel="Exterior Boundary"/>
              <ECProperty propertyName="MaximumInterval" typeName="double" displayLabel="Maximum Interval"/>
              <ECProperty propertyName="XmlFragment" typeName="string" displayLabel="XmlFragment"/>
              <ECProperty propertyName="UseActiveDesignProfile" typeName="boolean" displayLabel="Use Active Design Profile"/>
          </ECEntityClass>
          <ECEntityClass typeName="Profile">
              <BaseClass>FeaturizedEntity</BaseClass>
              <ECProperty propertyName="Geometry" typeName="Bentley.Geometry.Common.IGeometry"/>
              <ECProperty propertyName="GeometryClass" typeName="int" displayLabel="Geometry Type"/>
              <ECProperty propertyName="State" typeName="int" displayLabel="Is Valid"/>
          </ECEntityClass>
          <ECEntityClass typeName="AlignmentProxy">
              <BaseClass>Alignment</BaseClass>
              <ECProperty propertyName="IsReversed" typeName="boolean"/>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    //Verify our instances are still intact
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT Name, GeometryClass FROM ONLY TestSchema.Alignment");
    ASSERT_EQ(JsonValue(R"json([{"Name":"First Alignment","GeometryClass":1}])json"), result);
    }

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT Name, GeometryClass FROM ONLY TestSchema.AlignmentProxy");
    ASSERT_EQ(JsonValue(R"json([{"Name":"First AlignmentProxy","GeometryClass":2}])json"), result);
    }

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT Name FROM ONLY TestSchema.Corridor");
    ASSERT_EQ(JsonValue(R"json([{"Name":"First Corridor"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, IfcProblemJune21)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
<ECSchema schemaName="IFCDynamic" alias="IFC" version="100.03.21" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
    <ECEntityClass typeName="bisElementAspect" modifier="Abstract"  description="A bis:ElementAspect is a class that defines a set of properties that are related to (and owned by) a single bis:Element. Semantically, a bis:ElementAspect can be considered part of the bis:Element. Thus, a bis:ElementAspect is deleted if its owning bis:Element is deleted.">
    </ECEntityClass>
    <ECEntityClass typeName="bisElementMultiAspect" modifier="Abstract"  description="A bis:ElementMultiAspect is a bis:ElementAspect where there can be N instances of the bis:ElementAspect class per bis:Element.">
        <BaseClass>bisElementAspect</BaseClass>
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00.00">
                <!-- All subclasses of ElementUniqueAspect will share the same table -->
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <ShareColumns xmlns="ECDbMap.02.00.00">
                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECStructClass typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_ifc5888_10" >
        <ECProperty propertyName="ifccalculated_runon_subcatchment" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3" typeName="double" />
    </ECStructClass>)schema" R"schema(
    <ECStructClass typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_ifc5889_10" >
        <ECProperty propertyName="ifccalculated_runon_subcatchment" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3" typeName="double" />
    </ECStructClass>)schema" R"schema(
    <ECStructClass typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_ifc5890_10" >
        <ECProperty propertyName="ifccalculated_runoff_depth_subcatchment" typeName="double" />)schema" R"schema(
        <ECProperty propertyName="ifccalculated_runoff_depth_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_depth_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_catchment" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3" typeName="double" />
    </ECStructClass>)schema" R"schema(
    <ECStructClass typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_10" >
        <ECStructProperty propertyName="ifc5888" typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_ifc5888_10" />
        <ECStructProperty propertyName="ifc5889" typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_ifc5889_10" />
        <ECStructProperty propertyName="ifc5890" typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_ifc5890_10" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_lower_fence" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_lower_quartile" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_max" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_median" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_min" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_std_dev" typeName="double" />)schema" R"schema(
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_trimmed_mean" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_upper_fence" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_upper_quartile" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_lower_fence" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_lower_quartile" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_max" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_median" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_min" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_std_dev" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_trimmed_mean" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_upper_fence" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_upper_quartile" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment2_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment2_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3_critical_storm_id" typeName="string" />
    </ECStructClass>)schema" R"schema(
    <ECStructClass typeName="ifcStruct_12d_Model_ifcdynamic_10" >
        <ECStructProperty propertyName="ifc25" typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_10" />
        <ECProperty propertyName="ifccalculated_evaporation_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_evaporation_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_evaporation_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_impervious_tc3" typeName="double" />
        <ECProperty propertyName="ifccalculated_impervious_tc3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_impervious_tc3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_infiltration_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_infiltration_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_infiltration_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_max_impervious_tc3" typeName="double" />
        <ECProperty propertyName="ifccalculated_max_impervious_tc3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_max_impervious_tc3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_max_pervious_tc3" typeName="double" />
        <ECProperty propertyName="ifccalculated_max_pervious_tc3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_max_pervious_tc3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_min_impervious_tc3" typeName="double" />
        <ECProperty propertyName="ifccalculated_min_impervious_tc3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_min_impervious_tc3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_min_pervious_tc3" typeName="double" />
        <ECProperty propertyName="ifccalculated_min_pervious_tc3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_min_pervious_tc3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_pervious_tc3" typeName="double" />
        <ECProperty propertyName="ifccalculated_pervious_tc3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_pervious_tc3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_rainfall_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_rainfall_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_rainfall_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_c_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_c_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_c_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_depth_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_depth_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_depth_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3_critical_storm_id" typeName="string" />
    </ECStructClass>
    <ECStructClass typeName="ifcStruct_12d_Model_ifc12dField_ifcInst_Stat_Setup_ifcHelmert_Details_10" >
        <ECProperty propertyName="ifchelm_pos_error_2" typeName="double" />
        <ECProperty propertyName="ifchelm_str_ref_3" typeName="string" />
    </ECStructClass>
    <ECStructClass typeName="ifcStruct_12d_Model_ifc12dField_ifcInst_Stat_Setup_10" >
        <ECStructProperty propertyName="ifcHelmert_Details" typeName="ifcStruct_12d_Model_ifc12dField_ifcInst_Stat_Setup_ifcHelmert_Details_10" />
    </ECStructClass>
    <ECStructClass typeName="ifcStruct_12d_Model_ifc12dField_ifcMeasurement_10" >
        <ECProperty propertyName="ifcpu_comment_line" typeName="string" />
    </ECStructClass>
    <ECStructClass typeName="ifcStruct_12d_Model_ifc12dField_10" >
        <ECStructProperty propertyName="ifcInst_Stat_Setup" typeName="ifcStruct_12d_Model_ifc12dField_ifcInst_Stat_Setup_10" />
        <ECStructProperty propertyName="ifcMeasurement" typeName="ifcStruct_12d_Model_ifc12dField_ifcMeasurement_10" />
    </ECStructClass>
    <ECEntityClass typeName="ifcAspect_12d_Model_9" >
        <BaseClass>bisElementMultiAspect</BaseClass>
        <ECStructProperty propertyName="ifcdynamic" typeName="ifcStruct_12d_Model_ifcdynamic_10" />
        <ECProperty propertyName="ifcGuidance" typeName="string" />
        <ECProperty propertyName="ifcnumber_of_pipes" typeName="int" />
        <ECProperty propertyName="ifcservice_clearance5" typeName="double" />
        <ECProperty propertyName="ifcservice_drainage_invert5" typeName="double" />
        <ECProperty propertyName="ifcservice_height5" typeName="double" />
        <ECProperty propertyName="ifcservice_invert5" typeName="double" />
        <ECProperty propertyName="ifcservice_location5" typeName="double" />
        <ECProperty propertyName="ifcservice_min_clearance5" typeName="double" />
        <ECProperty propertyName="ifcservice_name5" typeName="string" />
        <ECProperty propertyName="ifcservice_width" typeName="double" />
        <ECProperty propertyName="ifcservice_width4" typeName="double" />
        <ECProperty propertyName="ifcwidth" typeName="double" />
        <ECStructProperty propertyName="ifc12dField" typeName="ifcStruct_12d_Model_ifc12dField_10" />
    </ECEntityClass>
</ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("IfcProblemJune21.ecdb", schemaItem));

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="IFCDynamic" alias="IFC" version="100.03.22" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
    <ECEntityClass typeName="bisElementAspect" modifier="Abstract"  description="A bis:ElementAspect is a class that defines a set of properties that are related to (and owned by) a single bis:Element. Semantically, a bis:ElementAspect can be considered part of the bis:Element. Thus, a bis:ElementAspect is deleted if its owning bis:Element is deleted.">
    </ECEntityClass>
    <ECEntityClass typeName="bisElementMultiAspect" modifier="Abstract"  description="A bis:ElementMultiAspect is a bis:ElementAspect where there can be N instances of the bis:ElementAspect class per bis:Element.">
        <BaseClass>bisElementAspect</BaseClass>
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00.00">
                <!-- All subclasses of ElementUniqueAspect will share the same table -->
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <ShareColumns xmlns="ECDbMap.02.00.00">
                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
    </ECEntityClass>)schema" R"schema(
    <ECStructClass typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_ifc5888_10" >
        <ECProperty propertyName="ifccalculated_runon_subcatchment" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3" typeName="double" />
    </ECStructClass>
    <ECStructClass typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_ifc5889_10" >
        <ECProperty propertyName="ifccalculated_runon_subcatchment" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3" typeName="double" />
    </ECStructClass>)schema" R"schema(
    <ECStructClass typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_ifc5890_10" >
        <ECProperty propertyName="ifccalculated_runoff_depth_subcatchment" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_depth_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_depth_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_catchment" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3" typeName="double" />
    </ECStructClass>)schema" R"schema(
    <ECStructClass typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_10" >
        <ECStructProperty propertyName="ifc5888" typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_ifc5888_10" />
        <ECStructProperty propertyName="ifc5889" typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_ifc5889_10" />
        <ECStructProperty propertyName="ifc5890" typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_ifc5890_10" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_lower_fence" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_lower_quartile" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_max" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_median" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_min" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_std_dev" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_trimmed_mean" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_upper_fence" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment2_ensemble_upper_quartile" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_lower_fence" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_lower_quartile" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_max" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_median" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_min" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_std_dev" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_trimmed_mean" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_upper_fence" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_ensemble_upper_quartile" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment2_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment2" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment2_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3_critical_storm_id" typeName="string" />
    </ECStructClass>)schema" R"schema(
    <ECStructClass typeName="ifcStruct_12d_Model_ifcdynamic_10" >
        <ECStructProperty propertyName="ifc25" typeName="ifcStruct_12d_Model_ifcdynamic_ifc25_10" />
        <ECProperty propertyName="ifccalculated_evaporation_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_evaporation_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_evaporation_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_impervious_tc3" typeName="double" />
        <ECProperty propertyName="ifccalculated_impervious_tc3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_impervious_tc3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_infiltration_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_infiltration_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_infiltration_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_max_impervious_tc3" typeName="double" />
        <ECProperty propertyName="ifccalculated_max_impervious_tc3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_max_impervious_tc3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_max_pervious_tc3" typeName="double" />
        <ECProperty propertyName="ifccalculated_max_pervious_tc3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_max_pervious_tc3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_min_impervious_tc3" typeName="double" />
        <ECProperty propertyName="ifccalculated_min_impervious_tc3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_min_impervious_tc3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_min_pervious_tc3" typeName="double" />
        <ECProperty propertyName="ifccalculated_min_pervious_tc3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_min_pervious_tc3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_pervious_tc3" typeName="double" />
        <ECProperty propertyName="ifccalculated_pervious_tc3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_pervious_tc3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_rainfall_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_rainfall_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_rainfall_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_c_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_c_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_c_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_depth_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_depth_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_depth_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_max_flow_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_runoff_volume_subcatchment3_critical_storm_id" typeName="string" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3_critical_storm" typeName="double" />
        <ECProperty propertyName="ifccalculated_runon_subcatchment3_critical_storm_id" typeName="string" />
    </ECStructClass>
    <ECStructClass typeName="ifcStruct_12d_Model_ifc12dField_ifcInst_Stat_Setup_ifcHelmert_Details_10" >
        <ECProperty propertyName="ifchelm_pos_error_2" typeName="double" />
        <ECProperty propertyName="ifchelm_str_ref_3" typeName="string" />
    </ECStructClass>
    <ECStructClass typeName="ifcStruct_12d_Model_ifc12dField_ifcInst_Stat_Setup_10" >
        <ECStructProperty propertyName="ifcHelmert_Details" typeName="ifcStruct_12d_Model_ifc12dField_ifcInst_Stat_Setup_ifcHelmert_Details_10" />
    </ECStructClass>
    <ECStructClass typeName="ifcStruct_12d_Model_ifc12dField_ifcMeasurement_10" >
        <ECProperty propertyName="ifcpu_comment_line" typeName="string" />
    </ECStructClass>)schema" R"schema(
    <ECStructClass typeName="ifcStruct_12d_Model_ifc12dField_ifcSatellite_Data_10" >
        <ECProperty propertyName="ifcgeoid_adjust" typeName="double" />
        <ECProperty propertyName="ifcgln_fix_type" typeName="int" />
        <ECProperty propertyName="ifcgln_fix_type_text" typeName="string" />
        <ECProperty propertyName="ifcgln_satellite_count" typeName="int" />
        <ECProperty propertyName="ifcgps_base_projection_data" typeName="string" />
        <ECProperty propertyName="ifcgps_base_projection_name" typeName="string" />
        <ECProperty propertyName="ifcgps_base_projection_type" typeName="string" />
        <ECProperty propertyName="ifcgps_coordinate_quality" typeName="double" />
        <ECProperty propertyName="ifcgps_fix_quality_text" typeName="string" />
        <ECProperty propertyName="ifcgps_fix_type" typeName="int" />
        <ECProperty propertyName="ifcgps_fix_type_text" typeName="string" />
        <ECProperty propertyName="ifcgps_glggq_sentence" typeName="string" />
        <ECProperty propertyName="ifcgps_gnggq_sentence" typeName="string" />
        <ECProperty propertyName="ifcgps_gpggq_sentence" typeName="string" />
        <ECProperty propertyName="ifcgps_helmert_params_data" typeName="string" />
        <ECProperty propertyName="ifcgps_helmert_params_file" typeName="string" />
        <ECProperty propertyName="ifcgps_helmert_params_stations" typeName="string" />
        <ECProperty propertyName="ifcgps_satellite_count" typeName="int" />
        <ECProperty propertyName="ifclocal_adjust" typeName="double" />
        <ECProperty propertyName="ifclocal_tin_ref" typeName="string" />
        <ECProperty propertyName="ifctotal_satellite_count" typeName="int" />
    </ECStructClass>
    <ECStructClass typeName="ifcStruct_12d_Model_ifc12dField_10" >
        <ECStructProperty propertyName="ifcInst_Stat_Setup" typeName="ifcStruct_12d_Model_ifc12dField_ifcInst_Stat_Setup_10" />
        <ECStructProperty propertyName="ifcMeasurement" typeName="ifcStruct_12d_Model_ifc12dField_ifcMeasurement_10" />
        <ECStructProperty propertyName="ifcSatellite_Data" typeName="ifcStruct_12d_Model_ifc12dField_ifcSatellite_Data_10" />
    </ECStructClass>
    <ECEntityClass typeName="ifcAspect_12d_Model_9" >
        <BaseClass>bisElementMultiAspect</BaseClass>
        <ECStructProperty propertyName="ifcdynamic" typeName="ifcStruct_12d_Model_ifcdynamic_10" />
        <ECProperty propertyName="ifcGuidance" typeName="string" />
        <ECProperty propertyName="ifcnumber_of_pipes" typeName="int" />
        <ECProperty propertyName="ifcservice_clearance5" typeName="double" />
        <ECProperty propertyName="ifcservice_drainage_invert5" typeName="double" />
        <ECProperty propertyName="ifcservice_height5" typeName="double" />
        <ECProperty propertyName="ifcservice_invert5" typeName="double" />
        <ECProperty propertyName="ifcservice_location5" typeName="double" />
        <ECProperty propertyName="ifcservice_min_clearance5" typeName="double" />
        <ECProperty propertyName="ifcservice_name5" typeName="string" />
        <ECProperty propertyName="ifcservice_width" typeName="double" />
        <ECProperty propertyName="ifcservice_width4" typeName="double" />
        <ECProperty propertyName="ifcwidth" typeName="double" />
        <ECStructProperty propertyName="ifc12dField" typeName="ifcStruct_12d_Model_ifc12dField_10" />
    </ECEntityClass>
</ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus SchemaRemapTestFixture::ImportSchemaFromFile(BeFileName const& fileName)
    {
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext(false, true);
    ctx->AddSchemaLocater(m_ecdb.GetSchemaLocater());

    BeFileName ecdbSchemaSearchPath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(ecdbSchemaSearchPath);
    ecdbSchemaSearchPath.AppendToPath(L"ECSchemas").AppendToPath(L"ECDb");
    ctx->AddSchemaPath(ecdbSchemaSearchPath);

    ECN::ECSchemaPtr ecSchema = nullptr;
    const SchemaReadStatus stat = ECN::ECSchema::ReadFromXmlFile(ecSchema, fileName.GetName(), *ctx);
    //duplicate schema error is ok, as the ReadFromXmlFile reads schema references implicitly.
    if (SchemaReadStatus::Success != stat && SchemaReadStatus::DuplicateSchema != stat)
        return ERROR;

    if (SUCCESS != m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas(),  SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues))
        {
        m_ecdb.AbandonChanges();
        return ERROR;
        }

    m_ecdb.SaveChanges();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus SchemaRemapTestFixture::ImportSchemasFromFolder(BeFileName const& schemaFolder)
    {
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext(false, true);
    ctx->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    ctx->AddSchemaPath(schemaFolder);

    BeFileName ecdbSchemaSearchPath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(ecdbSchemaSearchPath);
    ecdbSchemaSearchPath.AppendToPath(L"ECSchemas").AppendToPath(L"ECDb");
    ctx->AddSchemaPath(ecdbSchemaSearchPath);

    bvector<BeFileName> schemaPaths;
    BeDirectoryIterator::WalkDirsAndMatch(schemaPaths, schemaFolder, L"*.ecschema.xml", false);

    if (schemaPaths.empty())
        return ERROR;

    for (BeFileName const& schemaXmlFile : schemaPaths)
        {
        ECN::ECSchemaPtr ecSchema = nullptr;
        const SchemaReadStatus stat = ECN::ECSchema::ReadFromXmlFile(ecSchema, schemaXmlFile.GetName(), *ctx);
        //duplicate schema error is ok, as the ReadFromXmlFile reads schema references implicitly.
        if (SchemaReadStatus::Success != stat && SchemaReadStatus::DuplicateSchema != stat)
            return ERROR;
        }

    if (SUCCESS != m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas(),  SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues))
        {
        m_ecdb.AbandonChanges();
        return ERROR;
        }

    m_ecdb.SaveChanges();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
/*TEST_F(SchemaRemapTestFixture, ImportSchemasFromExternalFolders)
    {
    //This and the next test are used to diagnose problems with a single or a set of schemas loaded from the local file system.
    //That is useful so the file can be modified and executed many times without the need to rebuild the test.
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetSeverity("ECDb", BentleyApi::NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity("ECObjectsNative", BentleyApi::NativeLogging::LOG_TRACE);
    ASSERT_EQ(SUCCESS, SetupECDb("ImportSchemasFromExternalFolders.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("CREATE VIRTUAL TABLE dgn_SpatialIndex USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"));
    BeFileName schemasFolder(L"F:\\data\\input1\\");
    ASSERT_EQ(SUCCESS, ImportSchemasFromFolder(schemasFolder));

    BeFileName importFolder(L"F:\\data\\input2\\");
    ASSERT_EQ(SUCCESS, ImportSchemasFromFolder(importFolder));
    }*/

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
/*TEST_F(SchemaRemapTestFixture, ImportSchemasFromFiles)
    {
    NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
    NativeLogging::LoggingConfig::SetSeverity("ECDb", BentleyApi::NativeLogging::LOG_TRACE);
    NativeLogging::LoggingConfig::SetSeverity("ECObjectsNative", BentleyApi::NativeLogging::LOG_TRACE);
    ASSERT_EQ(SUCCESS, SetupECDb("ImportSchemasFromFiles.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("CREATE VIRTUAL TABLE dgn_SpatialIndex USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"));
    BeFileName fileName1(L"F:\\defects\\plant_condensed\\ProcessFunctional.01.00.00.ecschema.xml");
    ASSERT_EQ(SUCCESS, ImportSchemaFromFile(fileName1));

    BeFileName fileName2(L"F:\\defects\\plant_condensed\\ProcessFunctional.01.00.02.ecschema.xml");
    ASSERT_EQ(SUCCESS, ImportSchemaFromFile(fileName2));
    }*/

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MixinToBaseClass)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="MyMixin" modifier="Abstract">
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                  <AppliesToEntityClass>MyBaseClass</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
            <ECProperty propertyName="PropMixin1" typeName="string" />
            <ECProperty propertyName="MovingProperty" typeName="string" />
            <ECProperty propertyName="PropMixin2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <BaseClass>MyMixin</BaseClass>
            <ECProperty propertyName="PropA1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropB1" typeName="string" />
            <ECProperty propertyName="PropB2" typeName="string" />
            <ECProperty propertyName="PropB3" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("MixinToBaseClass.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty, PropA1, PropMixin1, PropMixin2) VALUES ('FIRST', 'A1', 'Mix1', 'Mix2')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty, PropA1, PropMixin1, PropMixin2 FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST","PropA1":"A1","PropMixin1":"Mix1","PropMixin2":"Mix2"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="MyMixin" modifier="Abstract">
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                  <AppliesToEntityClass>MyBaseClass</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
            <ECProperty propertyName="PropMixin1" typeName="string" />
            <ECProperty propertyName="MovingProperty" typeName="string" />
            <ECProperty propertyName="PropMixin2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <BaseClass>MyMixin</BaseClass>
            <ECProperty propertyName="PropA1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropB1" typeName="string" />
            <ECProperty propertyName="PropB2" typeName="string" />
            <ECProperty propertyName="PropB3" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Verify instances are intact
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty, PropA1, PropMixin1, PropMixin2 FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST","PropA1":"A1","PropMixin1":"Mix1","PropMixin2":"Mix2"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, DerivedMixinToBaseClass)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="MyBaseMixin" modifier="Abstract">
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                  <AppliesToEntityClass>MyBaseClass</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
            <ECProperty propertyName="PropMixin1" typeName="string" />
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="MyMixin" modifier="Abstract">
            <BaseClass>MyBaseMixin</BaseClass>
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                  <AppliesToEntityClass>MyBaseClass</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
            <ECProperty propertyName="PropMixin2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <BaseClass>MyMixin</BaseClass>
            <ECProperty propertyName="PropA1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropB1" typeName="string" />
            <ECProperty propertyName="PropB2" typeName="string" />
            <ECProperty propertyName="PropB3" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("DerivedMixinToBaseClassIssue.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty, PropA1, PropMixin1, PropMixin2) VALUES ('FIRST', 'A1', 'Mix1', 'Mix2')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty, PropA1, PropMixin1, PropMixin2 FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST","PropA1":"A1","PropMixin1":"Mix1","PropMixin2":"Mix2"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="MyBaseMixin" modifier="Abstract">
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                  <AppliesToEntityClass>MyBaseClass</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
            <ECProperty propertyName="PropMixin1" typeName="string" />
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="MyMixin" modifier="Abstract">
            <BaseClass>MyBaseMixin</BaseClass>
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                  <AppliesToEntityClass>MyBaseClass</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
            <ECProperty propertyName="PropMixin2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <BaseClass>MyMixin</BaseClass>
            <ECProperty propertyName="PropA1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropB1" typeName="string" />
            <ECProperty propertyName="PropB2" typeName="string" />
            <ECProperty propertyName="PropB3" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Verify instances are intact
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty, PropA1, PropMixin1, PropMixin2 FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST","PropA1":"A1","PropMixin1":"Mix1","PropMixin2":"Mix2"}])json"), result);
    }
    }



//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, MixinToBaseClassTwoLevels)
    {
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="MyMixin" modifier="Abstract">
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                  <AppliesToEntityClass>MyBaseClass</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
            <ECProperty propertyName="PropMixin1" typeName="string" />
            <ECProperty propertyName="MovingProperty" typeName="string" />
            <ECProperty propertyName="PropMixin2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="ABase">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>ABase</BaseClass>
            <BaseClass>MyMixin</BaseClass>
            <ECProperty propertyName="PropA1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropB1" typeName="string" />
            <ECProperty propertyName="PropB2" typeName="string" />
            <ECProperty propertyName="PropB3" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("MixinToBaseClassTwoLevels.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (MovingProperty, PropA1, PropMixin1, PropMixin2) VALUES ('FIRST', 'A1', 'Mix1', 'Mix2')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty, PropA1, PropMixin1, PropMixin2 FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST","PropA1":"A1","PropMixin1":"Mix1","PropMixin2":"Mix2"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="MovingProperty" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="MyMixin" modifier="Abstract">
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                  <AppliesToEntityClass>MyBaseClass</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
            <ECProperty propertyName="PropMixin1" typeName="string" />
            <ECProperty propertyName="MovingProperty" typeName="string" />
            <ECProperty propertyName="PropMixin2" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="ABase">
            <BaseClass>MyBaseClass</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>ABase</BaseClass>
            <BaseClass>MyMixin</BaseClass>
            <ECProperty propertyName="PropA1" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropB1" typeName="string" />
            <ECProperty propertyName="PropB2" typeName="string" />
            <ECProperty propertyName="PropB3" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Verify instances are intact
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT MovingProperty, PropA1, PropMixin1, PropMixin2 FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"MovingProperty":"FIRST","PropA1":"A1","PropMixin1":"Mix1","PropMixin2":"Mix2"}])json"), result);
    }
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, BuildingUSMappingProblem)
    {
    //Simplified version of a mapping problem involving several changes in BuildingSpatial, SpatialComposition and BuildingTemplate_US schemas.
    //There are many changes, basically, the base class structure of Building changed, a new base class was injected, along with new properties.
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
<ECSchema schemaName="BuildingTemplate_US" alias="BuildingTemplate_US" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
  <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
  <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
    <ECCustomAttributes>
        <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
    </ECCustomAttributes>
  <ECEntityClass typeName="Element" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECNavigationProperty propertyName="Model" relationshipName="ModelContainsElements" direction="backward" readOnly="true">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.2.0.0">
                    <OnDeleteAction>NoAction</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
        <ECProperty propertyName="LastMod" typeName="dateTime" readOnly="true">
            <ECCustomAttributes>
                <DateTimeInfo xmlns="CoreCustomAttributes.1.0.0">
                    <DateTimeKind>Utc</DateTimeKind>
                </DateTimeInfo>
                <HiddenProperty xmlns="CoreCustomAttributes.1.0.0" />
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="CodeValue" typeName="string">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.2.0.0">
                    <IsNullable>True</IsNullable>
                    <Collation>NoCase</Collation>
                </PropertyMap>
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="UserLabel" typeName="string">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.2.0.0">
                    <IsNullable>True</IsNullable>
                    <Collation>NoCase</Collation>
                    <IsUnique>False</IsUnique>
                </PropertyMap>
            </ECCustomAttributes>
        </ECProperty>
        <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElements" direction="backward" >
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.2.0.0">
                    <OnDeleteAction>NoAction</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
        <ECProperty propertyName="FederationGuid" typeName="binary" extendedTypeName="BeGuid">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.2.0.0">
                    <IsNullable>True</IsNullable>
                    <IsUnique>True</IsUnique>
                </PropertyMap>
                <HiddenProperty xmlns="CoreCustomAttributes.1.0.0" />
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="JsonProperties" typeName="string" extendedTypeName="Json">
            <ECCustomAttributes>
                <HiddenProperty xmlns="CoreCustomAttributes.1.0.0" />
            </ECCustomAttributes>
        </ECProperty>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(1..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Model" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <ShareColumns xmlns="ECDbMap.2.0.0">
                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
        <ECProperty propertyName="IsPrivate" typeName="boolean"/>
        <ECProperty propertyName="IsTemplate" typeName="boolean"/>
        <ECProperty propertyName="JsonProperties" typeName="string" extendedTypeName="Json"/>
        <ECProperty propertyName="LastMod" typeName="dateTime" />
    </ECEntityClass>
  <ECEntityClass typeName="GeometricElement" modifier="Abstract">
        <BaseClass>Element</BaseClass>
        <ECCustomAttributes>
            <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0" />
            <NotSubclassableInReferencingSchemas xmlns="CoreCustomAttributes.1.0.0" />
        </ECCustomAttributes>
    </ECEntityClass>
  <ECEntityClass typeName="GeometricElement3d" modifier="Abstract">
        <BaseClass>GeometricElement</BaseClass>
        <ECCustomAttributes>
            <ShareColumns xmlns="ECDbMap.2.0.0">
                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
        <ECProperty propertyName="InSpatialIndex" typeName="boolean">
        </ECProperty>
        <ECProperty propertyName="Origin" typeName="point3d">
        </ECProperty>
        <ECProperty propertyName="Yaw" typeName="double">
        </ECProperty>
        <ECProperty propertyName="Pitch" typeName="double">
        </ECProperty>
        <ECProperty propertyName="Roll" typeName="double">
        </ECProperty>
        <ECProperty propertyName="BBoxLow" typeName="point3d">
        </ECProperty>
        <ECProperty propertyName="BBoxHigh" typeName="point3d">
        </ECProperty>
        <ECProperty propertyName="GeometryStream" typeName="binary" extendedTypeName="GeometryStream">
        </ECProperty>
    </ECEntityClass>
  <ECEntityClass typeName="SpatialElement" modifier="Abstract">
        <BaseClass>GeometricElement3d</BaseClass>
    </ECEntityClass>
  <ECEntityClass typeName="PhysicalElement" modifier="Abstract">
        <BaseClass>SpatialElement</BaseClass>
    </ECEntityClass>
  <ECEntityClass typeName="SpatialLocationElement" modifier="Abstract">
        <BaseClass>SpatialElement</BaseClass>
    </ECEntityClass>
    <ECRelationshipClass typeName="CompositeComposesSubComposites" description="DEPRECATED" modifier="None" strength="embedding">
        <ECCustomAttributes>
            <Deprecated xmlns="CoreCustomAttributes.01.00.03">
                <Description>Instead, use the SpatialStructureElementAggregatesSpatialStructureElements relationship that is a subclass of this relationship for backwards compatibility.</Description>
            </Deprecated>
        </ECCustomAttributes>
        <Source multiplicity="(0..1)" roleLabel="aggregates" polymorphic="true">
            <Class class="CompositeElement"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is aggregated by" polymorphic="true">
            <Class class="CompositeElement"/>
        </Target>
    </ECRelationshipClass>
  <ECEntityClass typeName="CompositeElement" modifier="Abstract">
        <BaseClass>SpatialLocationElement</BaseClass>
        <ECNavigationProperty propertyName="ComposingElement" relationshipName="CompositeComposesSubComposites" direction="backward" />
        <ECProperty propertyName="FootprintArea" typeName="double" readOnly="true" />
    </ECEntityClass>
  <ECEntityClass typeName="ICompositeVolume" modifier="Abstract" >
    <ECCustomAttributes>
      <IsMixin xmlns="CoreCustomAttributes.01.00.03">
          <AppliesToEntityClass>CompositeElement</AppliesToEntityClass>
      </IsMixin>
      <Deprecated xmlns="CoreCustomAttributes.01.00.03">
          <Description>Deprecated along with CompositeElement so its use is no longer recommended.</Description>
      </Deprecated>
    </ECCustomAttributes>
  </ECEntityClass>
  <ECEntityClass typeName="BuildingBase">
        <BaseClass>CompositeElement</BaseClass>
        <BaseClass>ICompositeVolume</BaseClass>
    </ECEntityClass>
  <ECEntityClass typeName="Building">
        <BaseClass>BuildingBase</BaseClass>
        <ECProperty propertyName="Identity_PART" typeName="string"  />
        <ECProperty propertyName="Identity_FAMILY" typeName="string"  />
        <ECProperty propertyName="ArchBuilding_BuildingName" typeName="string"  />
        <ECProperty propertyName="ArchBuilding_YearConstructed" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_Address1" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_Address2" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_Address3" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_Address4" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_PostalBox" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_City" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_Region" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_PostalCode" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_Country" typeName="string"  />
        <ECProperty propertyName="ObjectIdentity_Mark" typeName="string"  />
        <ECProperty propertyName="ObjectIdentity_InstanceMark" typeName="string"  />
        <ECProperty propertyName="ObjectIdentity_NameAlt" typeName="string"  />
        <ECProperty propertyName="ObjectIdentity_Description" typeName="string"  />
        <ECProperty propertyName="ObjectIdentity_Keynote" typeName="string"  />
        <ECProperty propertyName="ObjectIdentity_Tag" typeName="string"  />
        <ECProperty propertyName="ObjectIdentity_Notes" typeName="string"  />
        <ECProperty propertyName="ObjectClassification_MasterFormat" typeName="string"  />
        <ECProperty propertyName="ObjectClassification_OmniClass" typeName="string"  />
        <ECProperty propertyName="ObjectClassification_UniFormat" typeName="string"  />
    </ECEntityClass>
</ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("BuildingUSMappingProblem.ecdb", schemaItem));

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
<ECSchema schemaName="BuildingTemplate_US" alias="BuildingTemplate_US" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
  <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
  <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
      <ECCustomAttributes>
        <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
    </ECCustomAttributes>
  <ECEntityClass typeName="Element" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECNavigationProperty propertyName="Model" relationshipName="ModelContainsElements" direction="backward" readOnly="true">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.2.0.0">
                    <OnDeleteAction>NoAction</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
        <ECProperty propertyName="LastMod" typeName="dateTime" readOnly="true">
            <ECCustomAttributes>
                <DateTimeInfo xmlns="CoreCustomAttributes.1.0.0">
                    <DateTimeKind>Utc</DateTimeKind>
                </DateTimeInfo>
                <HiddenProperty xmlns="CoreCustomAttributes.1.0.0" />
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="CodeValue" typeName="string">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.2.0.0">
                    <IsNullable>True</IsNullable>
                    <Collation>NoCase</Collation>
                </PropertyMap>
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="UserLabel" typeName="string">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.2.0.0">
                    <IsNullable>True</IsNullable>
                    <Collation>NoCase</Collation>
                    <IsUnique>False</IsUnique>
                </PropertyMap>
            </ECCustomAttributes>
        </ECProperty>
        <ECNavigationProperty propertyName="Parent" relationshipName="ElementOwnsChildElements" direction="backward" >
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.2.0.0">
                    <OnDeleteAction>NoAction</OnDeleteAction>
                </ForeignKeyConstraint>
            </ECCustomAttributes>
        </ECNavigationProperty>
        <ECProperty propertyName="FederationGuid" typeName="binary" extendedTypeName="BeGuid">
            <ECCustomAttributes>
                <PropertyMap xmlns="ECDbMap.2.0.0">
                    <IsNullable>True</IsNullable>
                    <IsUnique>True</IsUnique>
                </PropertyMap>
                <HiddenProperty xmlns="CoreCustomAttributes.1.0.0" />
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="JsonProperties" typeName="string" extendedTypeName="Json">
            <ECCustomAttributes>
                <HiddenProperty xmlns="CoreCustomAttributes.1.0.0" />
            </ECCustomAttributes>
        </ECProperty>
    </ECEntityClass>
    <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
        <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
            <Class class="Element"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
            <Class class="Element"/>
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ModelContainsElements" strength="embedding" modifier="Sealed">
        <Source multiplicity="(1..1)" roleLabel="contains" polymorphic="true">
            <Class class="Model"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is contained by" polymorphic="true">
            <Class class="Element" />
        </Target>
    </ECRelationshipClass>
    <ECEntityClass typeName="Model" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <ShareColumns xmlns="ECDbMap.2.0.0">
                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
        <ECProperty propertyName="IsPrivate" typeName="boolean"/>
        <ECProperty propertyName="IsTemplate" typeName="boolean"/>
        <ECProperty propertyName="JsonProperties" typeName="string" extendedTypeName="Json"/>
        <ECProperty propertyName="LastMod" typeName="dateTime" />
    </ECEntityClass>
  <ECEntityClass typeName="GeometricElement" modifier="Abstract">
        <BaseClass>Element</BaseClass>
        <ECCustomAttributes>
            <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0" />
            <NotSubclassableInReferencingSchemas xmlns="CoreCustomAttributes.1.0.0" />
        </ECCustomAttributes>
    </ECEntityClass>
  <ECEntityClass typeName="GeometricElement3d" modifier="Abstract">
        <BaseClass>GeometricElement</BaseClass>
        <ECCustomAttributes>
            <ShareColumns xmlns="ECDbMap.2.0.0">
                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
        <ECProperty propertyName="InSpatialIndex" typeName="boolean">
        </ECProperty>
        <ECProperty propertyName="Origin" typeName="point3d">
        </ECProperty>
        <ECProperty propertyName="Yaw" typeName="double">
        </ECProperty>
        <ECProperty propertyName="Pitch" typeName="double">
        </ECProperty>
        <ECProperty propertyName="Roll" typeName="double">
        </ECProperty>
        <ECProperty propertyName="BBoxLow" typeName="point3d">
        </ECProperty>
        <ECProperty propertyName="BBoxHigh" typeName="point3d">
        </ECProperty>
        <ECProperty propertyName="GeometryStream" typeName="binary" extendedTypeName="GeometryStream">
        </ECProperty>
    </ECEntityClass>
  <ECEntityClass typeName="SpatialElement" modifier="Abstract">
        <BaseClass>GeometricElement3d</BaseClass>
    </ECEntityClass>
  <ECEntityClass typeName="PhysicalElement" modifier="Abstract">
        <BaseClass>SpatialElement</BaseClass>
    </ECEntityClass>
  <ECEntityClass typeName="SpatialLocationElement" modifier="Abstract">
        <BaseClass>SpatialElement</BaseClass>
  </ECEntityClass>
    <ECRelationshipClass typeName="CompositeComposesSubComposites" description="DEPRECATED" modifier="None" strength="embedding">
        <ECCustomAttributes>
            <Deprecated xmlns="CoreCustomAttributes.01.00.03">
                <Description>Instead, use the SpatialStructureElementAggregatesSpatialStructureElements relationship that is a subclass of this relationship for backwards compatibility.</Description>
            </Deprecated>
        </ECCustomAttributes>
        <Source multiplicity="(0..1)" roleLabel="aggregates" polymorphic="true">
            <Class class="CompositeElement"/>
        </Source>
        <Target multiplicity="(0..*)" roleLabel="is aggregated by" polymorphic="true">
            <Class class="CompositeElement"/>
        </Target>
    </ECRelationshipClass>
  <ECEntityClass typeName="CompositeElement" modifier="Abstract">
        <BaseClass>SpatialLocationElement</BaseClass>
        <ECCustomAttributes>
            <Deprecated xmlns="CoreCustomAttributes.01.00.03">
                <Description>Instead, use the SpatialStructureElement class that is a subclass of this class for backwards compatibility.</Description>
            </Deprecated>
        </ECCustomAttributes>
        <ECNavigationProperty propertyName="ComposingElement" relationshipName="CompositeComposesSubComposites" direction="backward" />
        <ECProperty propertyName="FootprintArea" typeName="double" readOnly="true" />
    </ECEntityClass>
  <ECEntityClass typeName="ISpatialOrganizer" modifier="Abstract" >
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.01.00.03">
                <AppliesToEntityClass>SpatialLocationElement</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
    </ECEntityClass>
  <ECEntityClass typeName="SpatialStructureElement" modifier="Abstract" >
        <BaseClass>CompositeElement</BaseClass>
        <BaseClass>ISpatialOrganizer</BaseClass>
        <ECProperty propertyName="Description" typeName="string" />
    </ECEntityClass>
  <ECEntityClass typeName="Facility" modifier="Abstract" >
        <BaseClass>SpatialStructureElement</BaseClass>
    </ECEntityClass>
  <ECEntityClass typeName="ICompositeVolume" modifier="Abstract" >
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.01.00.03">
                <AppliesToEntityClass>CompositeElement</AppliesToEntityClass>
            </IsMixin>
            <Deprecated xmlns="CoreCustomAttributes.01.00.03">
                <Description>Deprecated along with CompositeElement so its use is no longer recommended.</Description>
            </Deprecated>
        </ECCustomAttributes>
    </ECEntityClass>
  <ECEntityClass typeName="BuildingBase" modifier="None">
        <BaseClass>Facility</BaseClass>
        <BaseClass>ICompositeVolume</BaseClass>
    </ECEntityClass>
  <ECEntityClass typeName="Building">
        <BaseClass>BuildingBase</BaseClass>
        <ECProperty propertyName="Identity_PART" typeName="string"  />
        <ECProperty propertyName="Identity_FAMILY" typeName="string"  />
        <ECProperty propertyName="ArchBuilding_BuildingName" typeName="string"  />
        <ECProperty propertyName="ArchBuilding_YearConstructed" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_Address1" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_Address2" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_Address3" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_Address4" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_PostalBox" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_City" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_Region" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_PostalCode" typeName="string"  />
        <ECProperty propertyName="ObjectPostalAddress_Country" typeName="string"  />
        <ECProperty propertyName="ObjectIdentity_Mark" typeName="string"  />
        <ECProperty propertyName="ObjectIdentity_InstanceMark" typeName="string"  />
        <ECProperty propertyName="ObjectIdentity_NameAlt" typeName="string"  />
        <ECProperty propertyName="ObjectIdentity_Description" typeName="string"  />
        <ECProperty propertyName="ObjectIdentity_Keynote" typeName="string"  />
        <ECProperty propertyName="ObjectIdentity_Tag" typeName="string"  />
        <ECProperty propertyName="ObjectIdentity_Notes" typeName="string"  />
        <ECProperty propertyName="ObjectClassification_MasterFormat" typeName="string"  />
        <ECProperty propertyName="ObjectClassification_OmniClass" typeName="string"  />
        <ECProperty propertyName="ObjectClassification_UniFormat" typeName="string"  />
    </ECEntityClass>
</ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, PutSiblingsIntoHierarchy)
    {
    //This puts two sibling classes below each other in the hierarchy.
    //Building moves below CompositeElement
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
<ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
    xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
    <ECEntityClass typeName="Element" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0" />
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="GeometricElement3d" modifier="Abstract">
        <BaseClass>Element</BaseClass>
        <ECCustomAttributes>
            <ShareColumns xmlns="ECDbMap.2.0.0">
                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="CompositeElement" modifier="Abstract">
        <BaseClass>GeometricElement3d</BaseClass>
        <ECProperty propertyName="X" typeName="string"/><!-- Maps to column 'js1'-->
    </ECEntityClass>
    <ECEntityClass typeName="Building">
        <BaseClass>GeometricElement3d</BaseClass>
        <ECProperty propertyName="A" typeName="string" /><!-- Maps to column 'js1'-->
    </ECEntityClass>
</ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("PutSiblingsIntoHierarchy.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Building (A) VALUES ('A')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT A FROM TestSchema.Building");
    ASSERT_EQ(JsonValue(R"json([{"A":"A"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
<ECSchema schemaName="TestSchema" alias="ts" version="01.00.01"
    xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
    <ECEntityClass typeName="Element" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0" />
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="GeometricElement3d" modifier="Abstract">
        <BaseClass>Element</BaseClass>
        <ECCustomAttributes>
            <ShareColumns xmlns="ECDbMap.2.0.0">
                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="CompositeElement" modifier="Abstract">
        <BaseClass>GeometricElement3d</BaseClass>
        <ECProperty propertyName="X" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Building">
        <BaseClass>CompositeElement</BaseClass>
        <ECProperty propertyName="A" typeName="string" />
    </ECEntityClass>
</ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT A FROM TestSchema.Building");
    ASSERT_EQ(JsonValue(R"json([{"A":"A"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, PutMultipleSiblingsIntoHierarchy)
    {
    //Move Building and Facility classes from GeometricElement3d below CompositeElement
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Element" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0" />
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="GeometricElement3d" modifier="Abstract">
                <BaseClass>Element</BaseClass>
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.2.0.0">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="CompositeElement" modifier="Abstract">
                <BaseClass>GeometricElement3d</BaseClass>
                <ECProperty propertyName="X" typeName="string"/><!-- Maps to column 'js1'-->
            </ECEntityClass>
            <ECEntityClass typeName="Building">
                <BaseClass>GeometricElement3d</BaseClass>
                <ECProperty propertyName="A" typeName="string" /><!-- Maps to column 'js1'-->
            </ECEntityClass>
            <ECEntityClass typeName="Facility">
                <BaseClass>GeometricElement3d</BaseClass>
                <ECProperty propertyName="B" typeName="string" /><!-- Maps to column 'js1'-->
            </ECEntityClass>
            <ECEntityClass typeName="SpecializedFacility">
                <BaseClass>Facility</BaseClass>
                <ECProperty propertyName="C" typeName="string" /><!-- Maps to column 'js2'-->
            </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("PutMultipleSiblingsIntoHierarchy.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Building (A) VALUES ('A')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.SpecializedFacility (B,C) VALUES ('B','C')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT A FROM TestSchema.Building");
    ASSERT_EQ(JsonValue(R"json([{"A":"A"}])json"), result);
    result = GetHelper().ExecuteSelectECSql("SELECT B,C FROM TestSchema.SpecializedFacility");
    ASSERT_EQ(JsonValue(R"json([{"B":"B","C":"C"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Element" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0" />
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="GeometricElement3d" modifier="Abstract">
                <BaseClass>Element</BaseClass>
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.2.0.0">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="CompositeElement" modifier="Abstract">
                <BaseClass>GeometricElement3d</BaseClass>
                <ECProperty propertyName="X" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Building">
                <BaseClass>CompositeElement</BaseClass>
                <ECProperty propertyName="A" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Facility">
                <BaseClass>CompositeElement</BaseClass>
                <ECProperty propertyName="B" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="SpecializedFacility">
                <BaseClass>Facility</BaseClass>
                <ECProperty propertyName="C" typeName="string" />
            </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT A FROM TestSchema.Building");
    ASSERT_EQ(JsonValue(R"json([{"A":"A"}])json"), result);
    result = GetHelper().ExecuteSelectECSql("SELECT B,C FROM TestSchema.SpecializedFacility");
    ASSERT_EQ(JsonValue(R"json([{"B":"B","C":"C"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, PutSiblingsIntoHierarchyWithStruct)
    {
    //Move classes A and B below "NewBase", A uses structs
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="blue" typeName="int" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="int" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="int" displayLabel="Red"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="NewBase">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropBase" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECStructProperty propertyName="PropA" typeName="ColorType"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropB" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("PutSiblingsIntoHierarchyWithStruct.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (PropA.blue, PropA.green, PropA.red) VALUES (1,2,3)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.B (PropB) VALUES ('B')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.NewBase (PropBase) VALUES ('Base')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"PropA":{"blue":1,"green":2,"red":3}}])json"), result);

    result = GetHelper().ExecuteSelectECSql("SELECT PropB FROM TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"PropB":"B"}])json"), result);

    result = GetHelper().ExecuteSelectECSql("SELECT PropBase FROM TestSchema.NewBase");
    ASSERT_EQ(JsonValue(R"json([{"PropBase":"Base"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="blue" typeName="int" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="int" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="int" displayLabel="Red"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="NewBase">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropBase" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>NewBase</BaseClass>
            <ECStructProperty propertyName="PropA" typeName="ColorType"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>NewBase</BaseClass>
            <ECProperty propertyName="PropB" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"PropA":{"blue":1,"green":2,"red":3}}])json"), result);

    result = GetHelper().ExecuteSelectECSql("SELECT PropB FROM TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"PropB":"B"}])json"), result);

    result = GetHelper().ExecuteSelectECSql("SELECT PropBase FROM ONLY TestSchema.NewBase");
    ASSERT_EQ(JsonValue(R"json([{"PropBase":"Base"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, InsertBaseClassRemapSiblingsWithStruct)
    {
    //Insert a new base class "NewBase" into existing Hierarchy
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="blue" typeName="int" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="int" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="int" displayLabel="Red"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECStructProperty propertyName="PropA" typeName="ColorType"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropB" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("InsertBaseClassRemapSiblingsWithStruct.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (PropA.blue, PropA.green, PropA.red) VALUES (1,2,3)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.B (PropB) VALUES ('B')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"PropA":{"blue":1,"green":2,"red":3}}])json"), result);

    result = GetHelper().ExecuteSelectECSql("SELECT PropB FROM TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"PropB":"B"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="blue" typeName="int" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="int" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="int" displayLabel="Red"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="NewBase">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropBase" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>NewBase</BaseClass>
            <ECStructProperty propertyName="PropA" typeName="ColorType"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>NewBase</BaseClass>
            <ECProperty propertyName="PropB" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"PropA":{"blue":1,"green":2,"red":3}}])json"), result);

    result = GetHelper().ExecuteSelectECSql("SELECT PropB FROM TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"PropB":"B"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, InsertTwoConnectedBaseClassesRemapSiblings)
    {
    //Insert new classes "NewBase" and "NewBase2" into existing hierarchy
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="blue" typeName="int" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="int" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="int" displayLabel="Red"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>MyBaseClass</BaseClass>
            <ECStructProperty propertyName="PropA" typeName="ColorType"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropB" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("InsertTwoConnectedBaseClassesRemapSiblings.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (PropA.blue, PropA.green, PropA.red) VALUES (1,2,3)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.B (PropB) VALUES ('B')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"PropA":{"blue":1,"green":2,"red":3}}])json"), result);

    result = GetHelper().ExecuteSelectECSql("SELECT PropB FROM TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"PropB":"B"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="ColorType">
            <ECProperty propertyName="blue" typeName="int" displayLabel="Blue"/>
            <ECProperty propertyName="green" typeName="int" displayLabel="Green"/>
            <ECProperty propertyName="red" typeName="int" displayLabel="Red"/>
          </ECStructClass>
          <ECEntityClass typeName="MyBaseClass">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="NewBase">
            <BaseClass>MyBaseClass</BaseClass>
            <ECProperty propertyName="PropBase" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="NewBase2">
            <BaseClass>NewBase</BaseClass>
            <ECProperty propertyName="PropBase" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>NewBase2</BaseClass>
            <ECStructProperty propertyName="PropA" typeName="ColorType"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>NewBase2</BaseClass>
            <ECProperty propertyName="PropB" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"PropA":{"blue":1,"green":2,"red":3}}])json"), result);

    result = GetHelper().ExecuteSelectECSql("SELECT PropB FROM TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"PropB":"B"}])json"), result);
    }
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, InsertBaseClassTwice)
    {
    //Turn hierarchy from A -> C -> E to A -> B -> C -> D -> E
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="PropA" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropC" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="E">
            <BaseClass>C</BaseClass>
            <ECProperty propertyName="PropE" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("InsertBaseClassTwice.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.E (PropA, PropC, PropE) VALUES ('A','C','E')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA, PropC, PropE FROM TestSchema.E");
    ASSERT_EQ(JsonValue(R"json([{"PropA":"A","PropC":"C","PropE":"E"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="PropA" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropB" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="PropC" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="D">
            <BaseClass>C</BaseClass>
            <ECProperty propertyName="PropD" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="E">
            <BaseClass>D</BaseClass>
            <ECProperty propertyName="PropE" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA, PropC, PropE FROM TestSchema.E");
    ASSERT_EQ(JsonValue(R"json([{"PropA":"A","PropC":"C","PropE":"E"}])json"), result);
    }
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, PutTwoClassesIntoHierarchy)
    {
    //Turn hierarchy from A -> C -> E to A -> B -> C -> D -> E
    //Difference to the previous test is that B and D already exist and just move into the hierarchy (this matters as they have existing mappings)
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="PropA" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropB" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropC" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="D">
            <BaseClass>C</BaseClass>
            <ECProperty propertyName="PropD" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="E">
            <BaseClass>C</BaseClass>
            <ECProperty propertyName="PropE" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("PutTwoClassesIntoHierarchy.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.E (PropA, PropC, PropE) VALUES ('A','C','E')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA, PropC, PropE FROM TestSchema.E");
    ASSERT_EQ(JsonValue(R"json([{"PropA":"A","PropC":"C","PropE":"E"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="PropA" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropB" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="PropC" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="D">
            <BaseClass>C</BaseClass>
            <ECProperty propertyName="PropD" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="E">
            <BaseClass>D</BaseClass>
            <ECProperty propertyName="PropE" typeName="string" />
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA, PropC, PropE FROM TestSchema.E");
    ASSERT_EQ(JsonValue(R"json([{"PropA":"A","PropC":"C","PropE":"E"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, PutSiblingsIntoHierarchyWithNestedStruct)
    {
    //move A and B below X, with A using a nested struct
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="InnerStruct">
            <ECProperty propertyName="a" typeName="string"/>
            <ECProperty propertyName="b" typeName="string"/>
          </ECStructClass>
          <ECStructClass typeName="OuterStruct">
            <ECStructProperty propertyName="c" typeName="InnerStruct"/>
            <ECProperty propertyName="d" typeName="string"/>
          </ECStructClass>
          <ECEntityClass typeName="Z">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="X">
            <BaseClass>Z</BaseClass>
            <ECProperty propertyName="e" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>Z</BaseClass>
            <ECStructProperty propertyName="f" typeName="OuterStruct"/>
            <ECProperty propertyName="g" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>Z</BaseClass>
            <ECStructProperty propertyName="h" typeName="InnerStruct" />
            <ECProperty propertyName="i" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("PutSiblingsIntoHierarchyWithNestedStruct.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (f.c.a, f.c.b, f.d, g) VALUES ('f.c.a' ,'f.c.b', 'f.d', 'g')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.B (h.a, h.b, i) VALUES ('h.a' ,'h.b', 'i')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT f, g FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"f":{"c":{"a":"f.c.a","b":"f.c.b"},"d":"f.d"},"g":"g"}])json"), result);

    result = GetHelper().ExecuteSelectECSql("SELECT h, i FROM TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"h":{"a":"h.a","b":"h.b"},"i":"i"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="InnerStruct">
            <ECProperty propertyName="a" typeName="string"/>
            <ECProperty propertyName="b" typeName="string"/>
          </ECStructClass>
          <ECStructClass typeName="OuterStruct">
            <ECStructProperty propertyName="c" typeName="InnerStruct"/>
            <ECProperty propertyName="d" typeName="string"/>
          </ECStructClass>
          <ECEntityClass typeName="Z">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="X">
            <BaseClass>Z</BaseClass>
            <ECProperty propertyName="e" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>X</BaseClass>
            <ECStructProperty propertyName="f" typeName="OuterStruct"/>
            <ECProperty propertyName="g" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>X</BaseClass>
            <ECStructProperty propertyName="h" typeName="InnerStruct" />
            <ECProperty propertyName="i" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT f, g FROM ONLY TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"f":{"c":{"a":"f.c.a","b":"f.c.b"},"d":"f.d"},"g":"g"}])json"), result);

    result = GetHelper().ExecuteSelectECSql("SELECT h, i FROM ONLY TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"h":{"a":"h.a","b":"h.b"},"i":"i"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, PutSiblingsIntoHierarchyWithPropertyOverrides)
    {
    //Siblings X and A are changed so A derives from X. Class B overrides some properties from A
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Z">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="X">
            <BaseClass>Z</BaseClass>
            <ECProperty propertyName="x" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>Z</BaseClass>
            <ECProperty propertyName="a" typeName="string"/>
            <ECProperty propertyName="b" typeName="string"/>
            <ECProperty propertyName="c" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="a" displayLabel="Overridden Label" typeName="string"/>
            <ECProperty propertyName="d" typeName="string"/>
            <ECProperty propertyName="c" displayLabel="Overridden Label" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("PutSiblingsIntoHierarchyWithPropertyOverrides.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.A (a, b, c) VALUES ('a', 'b', 'c')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.B (a, b, c, d) VALUES ('a2', 'b2', 'c2', 'd2')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT a, b, c FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"a":"a2","b":"b2","c":"c2"},{"a":"a","b":"b","c":"c"}])json"), result);

    result = GetHelper().ExecuteSelectECSql("SELECT a, b, c, d FROM TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"a":"a2","b":"b2","c":"c2","d":"d2"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="InnerStruct">
            <ECProperty propertyName="a" typeName="string"/>
            <ECProperty propertyName="b" typeName="string"/>
          </ECStructClass>
          <ECStructClass typeName="OuterStruct">
            <ECStructProperty propertyName="c" typeName="InnerStruct"/>
            <ECProperty propertyName="d" typeName="string"/>
          </ECStructClass>
          <ECEntityClass typeName="Z">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="X">
            <BaseClass>Z</BaseClass>
            <ECProperty propertyName="x" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="A">
            <BaseClass>X</BaseClass>
            <ECProperty propertyName="a" typeName="string"/>
            <ECProperty propertyName="b" typeName="string"/>
            <ECProperty propertyName="c" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="a" displayLabel="Overridden Label" typeName="string"/>
            <ECProperty propertyName="d" typeName="string"/>
            <ECProperty propertyName="c" displayLabel="Overridden Label" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT a, b, c FROM TestSchema.A");
    ASSERT_EQ(JsonValue(R"json([{"a":"a2","b":"b2","c":"c2"},{"a":"a","b":"b","c":"c"}])json"), result);

    result = GetHelper().ExecuteSelectECSql("SELECT a, b, c, d FROM ONLY TestSchema.B");
    ASSERT_EQ(JsonValue(R"json([{"a":"a2","b":"b2","c":"c2","d":"d2"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, PutBaseClassTurnPropertiesIntoOverrides)
    { //this is about moving a class into the hierarchy which causes properties to turn into overrides
    //property e will be remapped via deleted property mechanism, property g will be remapped through "new override" mechanism
    //the others will be remapped because they occupy columns of the new base class

    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Base">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="a" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="Middle">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="b" typeName="string"/>
            <ECProperty propertyName="d" typeName="string"/>
            <ECProperty propertyName="e" typeName="string"/>
            <ECProperty propertyName="g" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="Leaf">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="b" typeName="string"/>
            <ECProperty propertyName="c" typeName="string"/>
            <ECProperty propertyName="d" typeName="string"/>
            <ECProperty propertyName="e" typeName="string"/>
            <ECProperty propertyName="f" typeName="string"/>
            <ECProperty propertyName="g" typeName="string"/>
            <ECProperty propertyName="h" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="OccupiesSomeSharedColumns">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="z" typeName="string"/>
            <ECProperty propertyName="x" typeName="string"/>
            <ECProperty propertyName="y" typeName="string"/>
            <ECProperty propertyName="u" typeName="string"/>
            <ECProperty propertyName="v" typeName="string"/>
            <ECProperty propertyName="w" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("PutBaseClassTurnPropertiesIntoOverrides.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Leaf (a, b, c, d, e, f, g, h) VALUES ('A','B','C','D','E','F','G','H')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT a, b, c, d, e, f, g, h FROM TestSchema.Leaf");
    ASSERT_EQ(JsonValue(R"json([{"a":"A","b":"B","c":"C","d":"D","e":"E","f":"F","g":"G","h":"H"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Base">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="a" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="Middle">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="b" typeName="string"/>
            <ECProperty propertyName="d" typeName="string"/>
            <ECProperty propertyName="e" typeName="string"/>
            <ECProperty propertyName="g" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="Leaf">
            <BaseClass>Middle</BaseClass>
            <ECProperty propertyName="b" typeName="string"/>
            <ECProperty propertyName="c" typeName="string"/>
            <ECProperty propertyName="d" typeName="string"/>
            <ECProperty propertyName="f" typeName="string"/>
            <ECProperty propertyName="g" typeName="string"/>
            <ECProperty propertyName="h" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="OccupiesSomeSharedColumns">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="z" typeName="string"/>
            <ECProperty propertyName="x" typeName="string"/>
            <ECProperty propertyName="y" typeName="string"/>
            <ECProperty propertyName="u" typeName="string"/>
            <ECProperty propertyName="v" typeName="string"/>
            <ECProperty propertyName="w" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT a, b, c, d, e, f, g, h FROM TestSchema.Leaf");
    ASSERT_EQ(JsonValue(R"json([{"a":"A","b":"B","c":"C","d":"D","e":"E","f":"F","g":"G","h":"H"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, CreateBaseClassTurnPropertiesIntoOverrides)
    { //Same as previous test, just that "Middle" does not exist in the first schema
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Base">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="a" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="Leaf">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="b" typeName="string"/>
            <ECProperty propertyName="c" typeName="string"/>
            <ECProperty propertyName="d" typeName="string"/>
            <ECProperty propertyName="e" typeName="string"/>
            <ECProperty propertyName="f" typeName="string"/>
            <ECProperty propertyName="g" typeName="string"/>
            <ECProperty propertyName="h" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="OccupiesSomeSharedColumns">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="z" typeName="string"/>
            <ECProperty propertyName="x" typeName="string"/>
            <ECProperty propertyName="y" typeName="string"/>
            <ECProperty propertyName="u" typeName="string"/>
            <ECProperty propertyName="v" typeName="string"/>
            <ECProperty propertyName="w" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("PutBaseClassTurnPropertiesIntoOverrides.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Leaf (a, b, c, d, e, f, g, h) VALUES ('A','B','C','D','E','F','G','H')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT a, b, c, d, e, f, g, h FROM TestSchema.Leaf");
    ASSERT_EQ(JsonValue(R"json([{"a":"A","b":"B","c":"C","d":"D","e":"E","f":"F","g":"G","h":"H"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Base">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="a" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="Middle">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="b" typeName="string"/>
            <ECProperty propertyName="d" typeName="string"/>
            <ECProperty propertyName="e" typeName="string"/>
            <ECProperty propertyName="g" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="Leaf">
            <BaseClass>Middle</BaseClass>
            <ECProperty propertyName="b" typeName="string"/>
            <ECProperty propertyName="c" typeName="string"/>
            <ECProperty propertyName="d" typeName="string"/>
            <ECProperty propertyName="f" typeName="string"/>
            <ECProperty propertyName="g" typeName="string"/>
            <ECProperty propertyName="h" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="OccupiesSomeSharedColumns">
            <BaseClass>Base</BaseClass>
            <ECProperty propertyName="z" typeName="string"/>
            <ECProperty propertyName="x" typeName="string"/>
            <ECProperty propertyName="y" typeName="string"/>
            <ECProperty propertyName="w" typeName="string"/>
            <ECProperty propertyName="v" typeName="string"/>
            <ECProperty propertyName="u" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT a, b, c, d, e, f, g, h FROM TestSchema.Leaf");
    ASSERT_EQ(JsonValue(R"json([{"a":"A","b":"B","c":"C","d":"D","e":"E","f":"F","g":"G","h":"H"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, PutSiblingsWithSwappedPropertiesIntoHierarchy)
    {
    //Siblings Duck and Fish both have a name and description but in different order. Making Fish derive from Duck requires its properties to move to the same columns.
    SchemaItem schemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
<ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
    xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
    <ECEntityClass typeName="Element" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0" />
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="GeometricElement3d" modifier="Abstract">
        <BaseClass>Element</BaseClass>
        <ECCustomAttributes>
            <ShareColumns xmlns="ECDbMap.2.0.0">
                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="Animal" modifier="Abstract">
        <BaseClass>GeometricElement3d</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="Duck">
        <BaseClass>Animal</BaseClass>
        <ECProperty propertyName="name" typeName="string" />
        <ECProperty propertyName="description" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Fish">
        <BaseClass>Animal</BaseClass>
        <ECProperty propertyName="description" typeName="string" />
        <ECProperty propertyName="name" typeName="string" />
    </ECEntityClass>
</ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("PutSiblingsWithSwappedPropertiesIntoHierarchy.ecdb", schemaItem));
    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Duck (name, description) VALUES ('Donald','Donald the Duck')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.Fish (name, description) VALUES ('Nemo','Nemo the Clownfish')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT name, description FROM TestSchema.Duck");
    ASSERT_EQ(JsonValue(R"json([{"name":"Donald","description":"Donald the Duck"}])json"), result);
    result = GetHelper().ExecuteSelectECSql("SELECT name, description FROM TestSchema.Fish");
    ASSERT_EQ(JsonValue(R"json([{"name":"Nemo","description":"Nemo the Clownfish"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem editedSchemaItem(R"schema(<?xml version='1.0' encoding='utf-8' ?>
<ECSchema schemaName="TestSchema" alias="ts" version="01.00.01"
    xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
    <ECEntityClass typeName="Element" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.2.0.0">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
            <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0" />
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="GeometricElement3d" modifier="Abstract">
        <BaseClass>Element</BaseClass>
        <ECCustomAttributes>
            <ShareColumns xmlns="ECDbMap.2.0.0">
                <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
            </ShareColumns>
        </ECCustomAttributes>
    </ECEntityClass>
    <ECEntityClass typeName="Animal" modifier="Abstract">
        <BaseClass>GeometricElement3d</BaseClass>
    </ECEntityClass>
    <ECEntityClass typeName="Duck">
        <BaseClass>Animal</BaseClass>
        <ECProperty propertyName="name" typeName="string" />
        <ECProperty propertyName="description" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Fish">
        <BaseClass>Duck</BaseClass>
        <ECProperty propertyName="description" typeName="string" />
        <ECProperty propertyName="name" typeName="string" />
    </ECEntityClass>
</ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(editedSchemaItem, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT name, description FROM TestSchema.Duck");
    ASSERT_EQ(JsonValue(R"json([{"name":"Nemo","description":"Nemo the Clownfish"},{"name":"Donald","description":"Donald the Duck"}])json"), result);
    result = GetHelper().ExecuteSelectECSql("SELECT name, description FROM TestSchema.Fish");
    ASSERT_EQ(JsonValue(R"json([{"name":"Nemo","description":"Nemo the Clownfish"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, InjectBaseClassInBaseSchema)
    {
    //Purpose of this test is to check what happens if we make a change to only the base schema, which affects other schemas.
    //Turn hierarchy from S1:A -> S1:C -> S2:D to S1:A -> S1:B -> S1:C -> S2:D
    SchemaItem s1v1(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="PropA" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropB" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropC" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("InjectBaseClassInBaseSchema.ecdb", s1v1));

    SchemaItem s2(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="Schema2" alias="s2" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="Schema1" version="01.00.00" alias="s1"/>
          <ECEntityClass typeName="D">
            <BaseClass>s1:C</BaseClass>
            <ECProperty propertyName="PropD1" typeName="string"/>
            <ECProperty propertyName="PropD2" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, ImportSchema(s2));

    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO Schema2.D (PropA, PropC, PropD1, PropD2) VALUES ('A','C','D1','D2')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA, PropC, PropD1, PropD2 FROM Schema2.D");
    ASSERT_EQ(JsonValue(R"json([{"PropA":"A","PropC":"C","PropD1":"D1","PropD2":"D2"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem s1v2(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="PropA" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropB" typeName="string"/>
            <ECProperty propertyName="PropB2" typeName="string"/>
            <ECProperty propertyName="PropB3" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="PropC" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(s1v2, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA, PropC, PropD1, PropD2 FROM Schema2.D");
    ASSERT_EQ(JsonValue(R"json([{"PropA":"A","PropC":"C","PropD1":"D1","PropD2":"D2"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, InjectBaseClassInBaseSchema2)
    {
    //Similar to V1 but with deeper hierarchy and a new sibling class alongside class D
    //Turn hierarchy from S1:A -> S1:B -> S1:C -> S2:D -> S2:E to S1:A -> S1:B -> S1:B2 -> S1:C -> S2:D -> S2:E
    //Contains an affitional F class which also derives from C to occupy a shared column
    SchemaItem s1v1(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="PropA" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropB" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="B2">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="PropB2" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="PropC" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("InjectBaseClassInBaseSchema2.ecdb", s1v1));

    SchemaItem s2(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="Schema2" alias="s2" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="Schema1" version="01.00.00" alias="s1"/>
          <ECEntityClass typeName="D">
            <BaseClass>s1:C</BaseClass>
            <ECProperty propertyName="PropD1" typeName="string"/>
            <ECProperty propertyName="PropD2" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="E">
            <BaseClass>D</BaseClass>
            <ECProperty propertyName="PropE" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="F">
            <BaseClass>s1:C</BaseClass>
            <ECProperty propertyName="PropF" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, ImportSchema(s2));

    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO Schema2.D (PropA, PropC, PropD1, PropD2) VALUES ('A','C','D1','D2')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA, PropC, PropD1, PropD2 FROM Schema2.D");
    ASSERT_EQ(JsonValue(R"json([{"PropA":"A","PropC":"C","PropD1":"D1","PropD2":"D2"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem s1v2(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="PropA" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropB" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="B2">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="PropB2" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B2</BaseClass>
            <ECProperty propertyName="PropC" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(s1v2, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA, PropC, PropD1, PropD2 FROM Schema2.D");
    ASSERT_EQ(JsonValue(R"json([{"PropA":"A","PropC":"C","PropD1":"D1","PropD2":"D2"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, InjectBaseClassInBaseSchema3)
    {
    //Similar to "InjectBaseClassInBaseSchema" but different in that class "B" does not exist before the schema update, meaning
    //its properties are mapped during the update, not before it.
    //Turn hierarchy from S1:A -> S1:C -> S2:D to S1:A -> S1:B -> S1:C -> S2:D
    SchemaItem s1v1(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="PropA" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropC" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("InjectBaseClassInBaseSchema3.ecdb", s1v1));

    SchemaItem s2(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="Schema2" alias="s2" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="Schema1" version="01.00.00" alias="s1"/>
          <ECEntityClass typeName="D">
            <BaseClass>s1:C</BaseClass>
            <ECProperty propertyName="PropD1" typeName="string"/>
            <ECProperty propertyName="PropD2" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, ImportSchema(s2));

    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO Schema2.D (PropA, PropC, PropD1, PropD2) VALUES ('A','C','D1','D2')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA, PropC, PropD1, PropD2 FROM Schema2.D");
    ASSERT_EQ(JsonValue(R"json([{"PropA":"A","PropC":"C","PropD1":"D1","PropD2":"D2"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem s1v2(R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="Schema1" alias="s1" version="01.01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="A">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
              <ShareColumns xmlns="ECDbMap.02.00.00">
                  <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
              </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="PropA" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="PropB" typeName="string"/>
            <ECProperty propertyName="PropD1" typeName="string"/>
          </ECEntityClass>
          <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="PropC" typeName="string"/>
          </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(s1v2, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT PropA, PropC, PropD1, PropD2 FROM Schema2.D");
    ASSERT_EQ(JsonValue(R"json([{"PropA":"A","PropC":"C","PropD1":"D1","PropD2":"D2"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, InjectBaseClassInBaseSchema4)
    {
    //Covers the same scenario as the tests InjectBaseClassInBaseSchema1-3, but is based on classes from real
    //schemas, condensed into fewer schemas but still reflecting the actual class hierarchy encountered
    SchemaItem bisCore(R"schema(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BisCore" alias="bis" version="01.00.12" description="The BIS core schema contains classes that all other domain schemas extend." displayLabel="BIS Core" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Element" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="FederationGuid" typeName="binary" extendedTypeName="BeGuid" />
            </ECEntityClass>
          <ECEntityClass typeName="GeometricElement" modifier="Abstract">
                <BaseClass>Element</BaseClass>
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0" />
                </ECCustomAttributes>
            </ECEntityClass>
          <ECEntityClass typeName="GeometricElement3d" modifier="Abstract">
                <BaseClass>GeometricElement</BaseClass>
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.2.0.0">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="InSpatialIndex" typeName="boolean" />
            </ECEntityClass>
          <ECEntityClass typeName="SpatialElement" modifier="Abstract">
                <BaseClass>GeometricElement3d</BaseClass>
            </ECEntityClass>
          <ECEntityClass typeName="SpatialLocationElement" modifier="Abstract">
                <BaseClass>SpatialElement</BaseClass>
            </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("InjectBaseClassInBaseSchema4.ecdb", bisCore));

    SchemaItem spCompV1(R"schema(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="SpatialComposition" alias="spcomp" version="01.00.00" description="Provision of a spatial structure of the project by aggregating spatial elements." xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.12" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECEntityClass typeName="CompositeElement" description="a spatial element that may be Composite of other CompositeElements" modifier="Abstract">
                <BaseClass>bis:SpatialLocationElement</BaseClass>
                <ECProperty propertyName="FootprintArea" typeName="double" displayLabel="FootprintArea" readOnly="true"/>
            </ECEntityClass>
            <ECEntityClass typeName="ICompositeVolume" description="An interface that indicates that the CompositeElement is delimited by a volume" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00.03">
                        <AppliesToEntityClass>CompositeElement</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="Building" description="an element modeling the spatial perspective of a building">
                <BaseClass>CompositeElement</BaseClass>
                <BaseClass>ICompositeVolume</BaseClass>
            </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, ImportSchema(spCompV1));

    SchemaItem ifcDyn(R"schema(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="IFCDynamic" alias="IFC" version="100.03.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="SpatialComposition" version="01.00.00" alias="spcomp"/>
            <ECEntityClass typeName="IfcBuilding" displayLabel="IfcBuilding">
                <BaseClass>spcomp:Building</BaseClass>
                <ECProperty propertyName="ifcCompositionType" typeName="string"/>
            </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, ImportSchema(ifcDyn));

    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO IFCDynamic.IfcBuilding (ifcCompositionType) VALUES ('A')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT ifcCompositionType FROM IFCDynamic.IfcBuilding");
    ASSERT_EQ(JsonValue(R"json([{"ifcCompositionType":"A"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem spCompV2(R"schema(<?xml version="1.0" encoding="UTF-8"?>
      <ECSchema schemaName="SpatialComposition" alias="spcomp" version="01.00.01" description="Classes for defining the Spatial Structure Hierarchy of a project or asset." xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="BisCore" version="01.00.12" alias="bis"/>
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
          <ECEntityClass typeName="CompositeElement" description="DEPRECATED: A spatial element that may be Composite of other CompositeElements" modifier="Abstract">
              <BaseClass>bis:SpatialLocationElement</BaseClass>
              <ECProperty propertyName="FootprintArea" typeName="double" description="The area that this Element projects onto its base plane." displayLabel="FootprintArea" readOnly="true"/>
          </ECEntityClass>
          <ECEntityClass typeName="ISpatialOrganizer" description="An bis:SpatialLocation that organizes bis:SpatialElements using 'SpatialOrganizerHoldsSpatialElements' and 'SpatialOrganizerReferencesSpatialElements'" displayLabel="Spatial Organizer" modifier="Abstract">
              <ECCustomAttributes>
                  <IsMixin xmlns="CoreCustomAttributes.01.00.03">
                      <AppliesToEntityClass>bis:SpatialLocationElement</AppliesToEntityClass>
                  </IsMixin>
              </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="SpatialStructureElement" description="An Element used to form a spatial breakdown structure. As an ISpatialOrganizer, it may explicitly 'hold' or 'reference' SpatialElements." displayLabel="Spatial Structure Element" modifier="Abstract">
              <BaseClass>CompositeElement</BaseClass>
              <BaseClass>ISpatialOrganizer</BaseClass>
              <ECProperty propertyName="Description" typeName="string" description="A human-readable description of this Spatial Structure Element"/>
          </ECEntityClass>
          <ECEntityClass typeName="Facility" description="A volume occupied by a built facility, such as a building, bridge, road, factory/plant, railway, tunnel, etc." modifier="Abstract">
              <BaseClass>SpatialStructureElement</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="ICompositeVolume" description="DEPRECATED: An optional interface that indicates that the CompositeElement is delimited by a volume" modifier="Abstract">
              <ECCustomAttributes>
                  <IsMixin xmlns="CoreCustomAttributes.01.00.03">
                      <AppliesToEntityClass>CompositeElement</AppliesToEntityClass>
                  </IsMixin>
              </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Building" description="An element modeling the spatial perspective of a building">
              <BaseClass>Facility</BaseClass>
              <BaseClass>ICompositeVolume</BaseClass>
          </ECEntityClass>
      </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(spCompV2, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT ifcCompositionType FROM IFCDynamic.IfcBuilding");
    ASSERT_EQ(JsonValue(R"json([{"ifcCompositionType":"A"}])json"), result);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaRemapTestFixture, InjectBaseClass4_Simplified)
    {
    // like the above test, but everything is in a single schema
    SchemaItem bisCore(R"schema(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
          <ECEntityClass typeName="Element" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="FederationGuid" typeName="binary" extendedTypeName="BeGuid" />
            </ECEntityClass>
          <ECEntityClass typeName="GeometricElement" modifier="Abstract">
                <BaseClass>Element</BaseClass>
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0" />
                </ECCustomAttributes>
            </ECEntityClass>
          <ECEntityClass typeName="GeometricElement3d" modifier="Abstract">
                <BaseClass>GeometricElement</BaseClass>
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.2.0.0">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="InSpatialIndex" typeName="boolean" />
            </ECEntityClass>
          <ECEntityClass typeName="SpatialElement" modifier="Abstract">
                <BaseClass>GeometricElement3d</BaseClass>
            </ECEntityClass>
          <ECEntityClass typeName="SpatialLocationElement" modifier="Abstract">
                <BaseClass>SpatialElement</BaseClass>
            </ECEntityClass>

            <ECEntityClass typeName="CompositeElement" description="a spatial element that may be Composite of other CompositeElements" modifier="Abstract">
                <BaseClass>SpatialLocationElement</BaseClass>
                <ECProperty propertyName="FootprintArea" typeName="double" displayLabel="FootprintArea" readOnly="true"/>
            </ECEntityClass>
            <ECEntityClass typeName="ICompositeVolume" description="An interface that indicates that the CompositeElement is delimited by a volume" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00.03">
                        <AppliesToEntityClass>CompositeElement</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="Building" description="an element modeling the spatial perspective of a building">
                <BaseClass>CompositeElement</BaseClass>
                <BaseClass>ICompositeVolume</BaseClass>
            </ECEntityClass>

            <ECEntityClass typeName="IfcBuilding" displayLabel="IfcBuilding">
                <BaseClass>Building</BaseClass>
                <ECProperty propertyName="ifcCompositionType" typeName="string"/>
            </ECEntityClass>
        </ECSchema>
        )schema");

    ASSERT_EQ(SUCCESS, SetupECDb("InjectBaseClass4_Simplified.ecdb", bisCore));

    {
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO TestSchema.IfcBuilding (ifcCompositionType) VALUES ('A')");

    auto result = GetHelper().ExecuteSelectECSql("SELECT ifcCompositionType FROM TestSchema.IfcBuilding");
    ASSERT_EQ(JsonValue(R"json([{"ifcCompositionType":"A"}])json"), result);
    }

    //import edited schema with some changes.
    SchemaItem spCompV2(R"schema(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECEntityClass typeName="Element" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.2.0.0">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="FederationGuid" typeName="binary" extendedTypeName="BeGuid" />
            </ECEntityClass>
          <ECEntityClass typeName="GeometricElement" modifier="Abstract">
                <BaseClass>Element</BaseClass>
                <ECCustomAttributes>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.2.0.0" />
                </ECCustomAttributes>
            </ECEntityClass>
          <ECEntityClass typeName="GeometricElement3d" modifier="Abstract">
                <BaseClass>GeometricElement</BaseClass>
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.2.0.0">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="InSpatialIndex" typeName="boolean" />
            </ECEntityClass>
          <ECEntityClass typeName="SpatialElement" modifier="Abstract">
                <BaseClass>GeometricElement3d</BaseClass>
            </ECEntityClass>
          <ECEntityClass typeName="SpatialLocationElement" modifier="Abstract">
                <BaseClass>SpatialElement</BaseClass>
            </ECEntityClass>

            <ECEntityClass typeName="CompositeElement" description="DEPRECATED: A spatial element that may be Composite of other CompositeElements" modifier="Abstract">
                <BaseClass>SpatialLocationElement</BaseClass>
                <ECProperty propertyName="FootprintArea" typeName="double" description="The area that this Element projects onto its base plane." displayLabel="FootprintArea" readOnly="true"/>
            </ECEntityClass>
            <ECEntityClass typeName="ISpatialOrganizer" description="An bis:SpatialLocation that organizes bis:SpatialElements using 'SpatialOrganizerHoldsSpatialElements' and 'SpatialOrganizerReferencesSpatialElements'" displayLabel="Spatial Organizer" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00.03">
                        <AppliesToEntityClass>SpatialLocationElement</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="SpatialStructureElement" description="An Element used to form a spatial breakdown structure. As an ISpatialOrganizer, it may explicitly 'hold' or 'reference' SpatialElements." displayLabel="Spatial Structure Element" modifier="Abstract">
                <BaseClass>CompositeElement</BaseClass>
                <BaseClass>ISpatialOrganizer</BaseClass>
                <ECProperty propertyName="Description" typeName="string" description="A human-readable description of this Spatial Structure Element"/>
            </ECEntityClass>
            <ECEntityClass typeName="Facility" description="A volume occupied by a built facility, such as a building, bridge, road, factory/plant, railway, tunnel, etc." modifier="Abstract">
                <BaseClass>SpatialStructureElement</BaseClass>
            </ECEntityClass>
            <ECEntityClass typeName="ICompositeVolume" description="DEPRECATED: An optional interface that indicates that the CompositeElement is delimited by a volume" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00.03">
                        <AppliesToEntityClass>CompositeElement</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="Building" description="An element modeling the spatial perspective of a building">
                <BaseClass>Facility</BaseClass>
                <BaseClass>ICompositeVolume</BaseClass>
            </ECEntityClass>

            <ECEntityClass typeName="IfcBuilding" displayLabel="IfcBuilding">
                <BaseClass>Building</BaseClass>
                <ECProperty propertyName="ifcCompositionType" typeName="string"/>
            </ECEntityClass>
        </ECSchema>
        )schema");
    ASSERT_EQ(SUCCESS, ImportSchema(spCompV2, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    {
    auto result = GetHelper().ExecuteSelectECSql("SELECT ifcCompositionType FROM TestSchema.IfcBuilding");
    ASSERT_EQ(JsonValue(R"json([{"ifcCompositionType":"A"}])json"), result);
    }
    }
END_ECDBUNITTESTS_NAMESPACE
