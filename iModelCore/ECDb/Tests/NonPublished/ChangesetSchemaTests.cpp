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
struct ChangesetSchemaTestFixture : public ECDbTestFixture
    {
    };

#define ASSERT_ECSQL(ECDB_OBJ, PREPARESTATUS, STEPSTATUS, ECSQL)   {\
                                                                    ECSqlStatement stmt;\
                                                                    ASSERT_EQ(PREPARESTATUS, stmt.Prepare(ECDB_OBJ, ECSQL));\
                                                                    if (PREPARESTATUS == ECSqlStatus::Success)\
                                                                        ASSERT_EQ(STEPSTATUS, stmt.Step());\
                                                                   }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test basic capture and JSON round-trip
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, CaptureAndRoundTrip)
    {
    SchemaItem schema(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Animal">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                    <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Duck">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="Age" typeName="int" />
          </ECEntityClass>
          <ECEntityClass typeName="Fish">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="Age" typeName="int" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_CaptureAndRoundTrip.ecdb", schema));

    // Capture
    ChangesetSchema captured;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(captured, m_ecdb));

    // Verify captured classes
    auto const& classes = captured.GetClasses();
    ASSERT_TRUE(classes.find("TestSchema:Animal") != classes.end());
    ASSERT_TRUE(classes.find("TestSchema:Duck") != classes.end());
    ASSERT_TRUE(classes.find("TestSchema:Fish") != classes.end());

    // Check Duck has Age property mapped
    auto duckFullMap = captured.GetFullPropertyMap("TestSchema:Duck");
    ASSERT_TRUE(duckFullMap.find("Age") != duckFullMap.end());

    // Round-trip through JSON
    BeJsDocument json;
    captured.ToJson(json);

    ChangesetSchema deserialized;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::FromJson(deserialized, json));

    // Verify deserialized matches original
    auto const& deserializedClasses = deserialized.GetClasses();
    ASSERT_EQ(classes.size(), deserializedClasses.size());

    for (auto const& [key, entry] : classes)
        {
        auto it = deserializedClasses.find(key);
        ASSERT_TRUE(it != deserializedClasses.end()) << "Missing class: " << key.c_str();
        ASSERT_EQ(entry.m_classId, it->second.m_classId) << "ClassId mismatch for: " << key.c_str();
        ASSERT_EQ(entry.m_baseClass, it->second.m_baseClass) << "BaseClass mismatch for: " << key.c_str();
        }

    // Verify full property maps match after round-trip
    auto duckFullMapDeserialized = deserialized.GetFullPropertyMap("TestSchema:Duck");
    ASSERT_EQ(duckFullMap.size(), duckFullMapDeserialized.size());
    for (auto const& [as, pm] : duckFullMap)
        {
        auto it = duckFullMapDeserialized.find(as);
        ASSERT_TRUE(it != duckFullMapDeserialized.end());
        ASSERT_EQ(pm.m_table, it->second.m_table);
        ASSERT_EQ(pm.m_column, it->second.m_column);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test property remap detection (column swap) when a property is moved up the hierarchy.
// Before: Duck.Age → shared col X, Fish.Age → shared col X (defined on each class)
// After:  Animal.Age → shared col Y (moved to base, all derived inherit)
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, PropertyRemapColumnSwap)
    {
    SchemaItem schema1(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Animal">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                    <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                </ShareColumns>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Duck">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="Age" typeName="int" />
            <ECProperty propertyName="Color" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Fish">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="Age" typeName="int" />
            <ECProperty propertyName="Depth" typeName="int" />
          </ECEntityClass>
          <ECEntityClass typeName="Zebra">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="Name" typeName="string" />
            <ECProperty propertyName="Age" typeName="int" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_PropertyRemap.ecdb", schema1));

    // Insert data so we have instances
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Duck (Age, Color) VALUES (5, 'White')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Fish (Age, Depth) VALUES (3, 100)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Zebra (Name, Age) VALUES ('Zeb', 10)");
    m_ecdb.SaveChanges();

    // Capture schema BEFORE
    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));

    // Record Duck's Age column before the remap
    auto duckMapBefore = schemaBefore.GetFullPropertyMap("TestSchema:Duck");
    ASSERT_TRUE(duckMapBefore.find("Age") != duckMapBefore.end());
    Utf8String duckAgeColumnBefore = duckMapBefore["Age"].m_column;

    // Upgrade schema: move Age to Animal base class
    SchemaItem schema2(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="02.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Animal">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                    <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Age" typeName="int" />
          </ECEntityClass>
          <ECEntityClass typeName="Duck">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="Color" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="Fish">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="Depth" typeName="int" />
          </ECEntityClass>
          <ECEntityClass typeName="Zebra">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema2,
        SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    m_ecdb.SaveChanges();

    // Capture schema AFTER
    ChangesetSchema schemaAfter;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaAfter, m_ecdb));

    // Diff
    ChangesetSchemaDiff diff;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchemaDiff::Diff(diff, schemaBefore, schemaAfter));

    // Verify no errors
    ASSERT_FALSE(diff.HasErrors()) << "Diff should not have errors for additive schema change";

    // Check if Age was remapped for Duck (column swap detected)
    auto duckMapAfter = schemaAfter.GetFullPropertyMap("TestSchema:Duck");
    ASSERT_TRUE(duckMapAfter.find("Age") != duckMapAfter.end());
    Utf8String duckAgeColumnAfter = duckMapAfter["Age"].m_column;

    if (duckAgeColumnBefore != duckAgeColumnAfter)
        {
        // If columns changed, verify the diff detected it
        ASSERT_TRUE(diff.NeedsTransform()) << "Diff should detect the column swap";

        bool foundDuckAgeSwap = false;
        for (auto const& swap : diff.m_columnSwaps)
            {
            if (swap.m_classKey == "TestSchema:Duck" && swap.m_accessString == "Age")
                {
                foundDuckAgeSwap = true;
                ASSERT_EQ(duckAgeColumnBefore, swap.m_oldColumn);
                ASSERT_EQ(duckAgeColumnAfter, swap.m_newColumn);
                break;
                }
            }
        ASSERT_TRUE(foundDuckAgeSwap) << "Should detect Duck.Age column swap";
        }

    // Verify data integrity is preserved
    ASSERT_EQ(JsonValue(R"json([{"Age":5}])json"),
        GetHelper().ExecuteSelectECSql("SELECT Age FROM ts.Duck"));
    ASSERT_EQ(JsonValue(R"json([{"Age":3}])json"),
        GetHelper().ExecuteSelectECSql("SELECT Age FROM ts.Fish"));
    ASSERT_EQ(JsonValue(R"json([{"Age":10}])json"),
        GetHelper().ExecuteSelectECSql("SELECT Age FROM ts.Zebra"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test overflow table detection when adding a property to ECStruct causes overflow.
// A struct has enough members to fill shared columns. Adding another member causes
// overflow table creation.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, StructPropertyCausesOverflow)
    {
    // Initial schema: struct S has 3 int members, class Element uses shared columns
    // with MaxSharedColumnsBeforeOverflow=4. Element has p0 (S) = 3 columns used.
    // Plus one more property to fill to exactly 4. No overflow.
    SchemaItem schema1(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="MyStruct" modifier="None">
            <ECProperty propertyName="X" typeName="int" />
            <ECProperty propertyName="Y" typeName="int" />
            <ECProperty propertyName="Z" typeName="int" />
          </ECStructClass>
          <ECEntityClass typeName="Element">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                    <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                </ShareColumns>
            </ECCustomAttributes>
            <ECStructProperty propertyName="S" typeName="MyStruct" />
            <ECProperty propertyName="Label" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="SubElement">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="Extra" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_StructOverflow.ecdb", schema1));

    // Insert some data
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.Element (S, Label) VALUES ({X:1, Y:2, Z:3}, 'hello')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.SubElement (S, Label, Extra) VALUES ({X:4, Y:5, Z:6}, 'world', 'bonus')");
    m_ecdb.SaveChanges();

    // Capture schema BEFORE
    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));

    // Verify no overflow table exists before
    auto elementEntryBefore = schemaBefore.GetClasses().find("TestSchema:Element");
    ASSERT_TRUE(elementEntryBefore != schemaBefore.GetClasses().end());
    bool hasOverflowBefore = false;
    for (auto const& [tbl, info] : elementEntryBefore->second.m_tables)
        {
        if (info.m_type == "Overflow")
            hasOverflowBefore = true;
        }
    // Note: whether overflow exists initially depends on the mapper. We record the state.

    // Upgrade: Add a new member W to MyStruct. This pushes struct S to need 4 columns,
    // which together with Label exceeds MaxSharedColumnsBeforeOverflow=4, triggering overflow.
    SchemaItem schema2(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="02.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="MyStruct" modifier="None">
            <ECProperty propertyName="X" typeName="int" />
            <ECProperty propertyName="Y" typeName="int" />
            <ECProperty propertyName="Z" typeName="int" />
            <ECProperty propertyName="W" typeName="int" />
          </ECStructClass>
          <ECEntityClass typeName="Element">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                    <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                </ShareColumns>
            </ECCustomAttributes>
            <ECStructProperty propertyName="S" typeName="MyStruct" />
            <ECProperty propertyName="Label" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="SubElement">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="Extra" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema2,
        SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    m_ecdb.SaveChanges();

    // Capture schema AFTER
    ChangesetSchema schemaAfter;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaAfter, m_ecdb));

    // Diff
    ChangesetSchemaDiff diff;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchemaDiff::Diff(diff, schemaBefore, schemaAfter));

    // Verify no errors
    ASSERT_FALSE(diff.HasErrors()) << "Diff should not have errors for additive schema change";

    // Check if overflow table was detected
    auto elementEntryAfter = schemaAfter.GetClasses().find("TestSchema:Element");
    ASSERT_TRUE(elementEntryAfter != schemaAfter.GetClasses().end());
    bool hasOverflowAfter = false;
    Utf8String overflowTableName;
    for (auto const& [tbl, info] : elementEntryAfter->second.m_tables)
        {
        if (info.m_type == "Overflow")
            {
            hasOverflowAfter = true;
            overflowTableName = tbl;
            }
        }

    if (hasOverflowAfter && !hasOverflowBefore)
        {
        // Overflow was created — verify diff detected it
        bool foundOverflow = false;
        for (auto const& ovf : diff.m_overflowTablesAdded)
            {
            if (ovf.m_classKey == "TestSchema:Element" || ovf.m_classKey == "TestSchema:SubElement")
                {
                foundOverflow = true;
                ASSERT_FALSE(ovf.m_overflowTable.empty());
                ASSERT_FALSE(ovf.m_parentTable.empty());
                }
            }
        ASSERT_TRUE(foundOverflow) << "Diff should detect newly created overflow table";
        }

    // Also check if struct property S.W now maps to a column (possibly in overflow)
    auto elementFullMapAfter = schemaAfter.GetFullPropertyMap("TestSchema:Element");
    ASSERT_TRUE(elementFullMapAfter.find("S.W") != elementFullMapAfter.end())
        << "New struct member S.W should have a property map";

    // If overflow exists, check that some struct member maps to the overflow table
    if (hasOverflowAfter)
        {
        bool somePropertyInOverflow = false;
        for (auto const& [as, pm] : elementFullMapAfter)
            {
            if (pm.m_table == overflowTableName)
                {
                somePropertyInOverflow = true;
                break;
                }
            }
        ASSERT_TRUE(somePropertyInOverflow)
            << "At least one property should map to the overflow table";
        }

    // Verify existing data is still accessible
    ASSERT_EQ(JsonValue(R"json([{"Label":"hello"}])json"),
        GetHelper().ExecuteSelectECSql("SELECT Label FROM ts.Element WHERE S.X=1"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test that missing class in "after" produces an error in the diff.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, DiffErrorMissingClass)
    {
    // Construct "before" with a class that won't exist in "after"
    ChangesetSchema before;
    {
    ChangesetSchemaClassEntry entry;
    entry.m_classKey = "TestSchema:Ghost";
    entry.m_classId = ECClassId((uint64_t)999);
    entry.m_tables["ts_Ghost"] = ChangesetSchemaTableInfo("Primary");
    entry.m_propertyMaps["Name"] = ChangesetSchemaPropertyMap("ts_Ghost", "js1");
    before.GetClassesR()["TestSchema:Ghost"] = std::move(entry);
    }

    // "after" is empty — class disappeared
    ChangesetSchema after;

    ChangesetSchemaDiff diff;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchemaDiff::Diff(diff, before, after));

    ASSERT_TRUE(diff.HasErrors());
    ASSERT_FALSE(diff.NeedsTransform());
    ASSERT_EQ(1u, diff.m_errors.size());
    ASSERT_EQ(ChangesetSchemaDiff::MissingMapping::Kind::Class, diff.m_errors[0].m_kind);
    ASSERT_EQ(Utf8String("TestSchema:Ghost"), diff.m_errors[0].m_classKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test that missing property in "after" produces an error in the diff.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, DiffErrorMissingProperty)
    {
    ChangesetSchema before;
    {
    ChangesetSchemaClassEntry entry;
    entry.m_classKey = "TestSchema:Animal";
    entry.m_classId = ECClassId((uint64_t)10);
    entry.m_tables["ts_Animal"] = ChangesetSchemaTableInfo("Primary");
    entry.m_propertyMaps["Age"] = ChangesetSchemaPropertyMap("ts_Animal", "js1");
    entry.m_propertyMaps["Name"] = ChangesetSchemaPropertyMap("ts_Animal", "js2");
    before.GetClassesR()["TestSchema:Animal"] = std::move(entry);
    }

    ChangesetSchema after;
    {
    ChangesetSchemaClassEntry entry;
    entry.m_classKey = "TestSchema:Animal";
    entry.m_classId = ECClassId((uint64_t)10);
    entry.m_tables["ts_Animal"] = ChangesetSchemaTableInfo("Primary");
    // "Name" property mapping is MISSING
    entry.m_propertyMaps["Age"] = ChangesetSchemaPropertyMap("ts_Animal", "js1");
    after.GetClassesR()["TestSchema:Animal"] = std::move(entry);
    }

    ChangesetSchemaDiff diff;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchemaDiff::Diff(diff, before, after));

    ASSERT_TRUE(diff.HasErrors());
    ASSERT_FALSE(diff.NeedsTransform());
    ASSERT_EQ(1u, diff.m_errors.size());
    ASSERT_EQ(ChangesetSchemaDiff::MissingMapping::Kind::Property, diff.m_errors[0].m_kind);
    ASSERT_EQ(Utf8String("TestSchema:Animal"), diff.m_errors[0].m_classKey);
    ASSERT_EQ(Utf8String("Name"), diff.m_errors[0].m_accessString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test that a disappeared table in "after" produces an error in the diff.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, DiffErrorDisappearedTable)
    {
    ChangesetSchema before;
    {
    ChangesetSchemaClassEntry entry;
    entry.m_classKey = "TestSchema:Element";
    entry.m_classId = ECClassId((uint64_t)20);
    entry.m_tables["ts_Element"] = ChangesetSchemaTableInfo("Primary");
    entry.m_tables["ts_Element_Joined"] = ChangesetSchemaTableInfo("Joined");
    entry.m_propertyMaps["Label"] = ChangesetSchemaPropertyMap("ts_Element", "js1");
    before.GetClassesR()["TestSchema:Element"] = std::move(entry);
    }

    ChangesetSchema after;
    {
    ChangesetSchemaClassEntry entry;
    entry.m_classKey = "TestSchema:Element";
    entry.m_classId = ECClassId((uint64_t)20);
    // Joined table MISSING
    entry.m_tables["ts_Element"] = ChangesetSchemaTableInfo("Primary");
    entry.m_propertyMaps["Label"] = ChangesetSchemaPropertyMap("ts_Element", "js1");
    after.GetClassesR()["TestSchema:Element"] = std::move(entry);
    }

    ChangesetSchemaDiff diff;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchemaDiff::Diff(diff, before, after));

    ASSERT_TRUE(diff.HasErrors());
    ASSERT_FALSE(diff.NeedsTransform());
    ASSERT_EQ(1u, diff.m_errors.size());
    ASSERT_EQ(ChangesetSchemaDiff::MissingMapping::Kind::Table, diff.m_errors[0].m_kind);
    ASSERT_EQ(Utf8String("TestSchema:Element"), diff.m_errors[0].m_classKey);
    ASSERT_EQ(Utf8String("ts_Element_Joined"), diff.m_errors[0].m_tableName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test class ID remap detection.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, DiffClassIdRemap)
    {
    ChangesetSchema before;
    {
    ChangesetSchemaClassEntry entry;
    entry.m_classKey = "TestSchema:Duck";
    entry.m_classId = ECClassId((uint64_t)0x41);
    entry.m_tables["ts_Animal"] = ChangesetSchemaTableInfo("Primary");
    entry.m_propertyMaps["Age"] = ChangesetSchemaPropertyMap("ts_Animal", "js1");
    before.GetClassesR()["TestSchema:Duck"] = std::move(entry);
    }

    ChangesetSchema after;
    {
    ChangesetSchemaClassEntry entry;
    entry.m_classKey = "TestSchema:Duck";
    entry.m_classId = ECClassId((uint64_t)0x51); // different ID
    entry.m_tables["ts_Animal"] = ChangesetSchemaTableInfo("Primary");
    entry.m_propertyMaps["Age"] = ChangesetSchemaPropertyMap("ts_Animal", "js1");
    after.GetClassesR()["TestSchema:Duck"] = std::move(entry);
    }

    ChangesetSchemaDiff diff;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchemaDiff::Diff(diff, before, after));

    ASSERT_FALSE(diff.HasErrors());
    ASSERT_TRUE(diff.NeedsTransform());
    ASSERT_EQ(1u, diff.m_classIdRemaps.size());
    ASSERT_EQ(Utf8String("TestSchema:Duck"), diff.m_classIdRemaps[0].m_classKey);
    ASSERT_EQ(ECClassId((uint64_t)0x41), diff.m_classIdRemaps[0].m_oldClassId);
    ASSERT_EQ(ECClassId((uint64_t)0x51), diff.m_classIdRemaps[0].m_newClassId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test column swap detection with inheritance.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, DiffColumnSwapWithInheritance)
    {
    // Before: Duck inherits from Animal (no props), Duck has Age→js1
    // After: Animal now has Age→js2 (moved up), Duck inherits it
    ChangesetSchema before;
    {
    ChangesetSchemaClassEntry animalEntry;
    animalEntry.m_classKey = "TestSchema:Animal";
    animalEntry.m_classId = ECClassId((uint64_t)0x40);
    animalEntry.m_tables["ts_Animal"] = ChangesetSchemaTableInfo("Primary");
    before.GetClassesR()["TestSchema:Animal"] = std::move(animalEntry);

    ChangesetSchemaClassEntry duckEntry;
    duckEntry.m_classKey = "TestSchema:Duck";
    duckEntry.m_classId = ECClassId((uint64_t)0x41);
    duckEntry.m_baseClass = "TestSchema:Animal";
    duckEntry.m_tables["ts_Animal"] = ChangesetSchemaTableInfo("Primary");
    duckEntry.m_propertyMaps["Age"] = ChangesetSchemaPropertyMap("ts_Animal", "js1");
    before.GetClassesR()["TestSchema:Duck"] = std::move(duckEntry);
    }

    ChangesetSchema after;
    {
    ChangesetSchemaClassEntry animalEntry;
    animalEntry.m_classKey = "TestSchema:Animal";
    animalEntry.m_classId = ECClassId((uint64_t)0x40);
    animalEntry.m_tables["ts_Animal"] = ChangesetSchemaTableInfo("Primary");
    animalEntry.m_propertyMaps["Age"] = ChangesetSchemaPropertyMap("ts_Animal", "js2"); // now on base
    after.GetClassesR()["TestSchema:Animal"] = std::move(animalEntry);

    ChangesetSchemaClassEntry duckEntry;
    duckEntry.m_classKey = "TestSchema:Duck";
    duckEntry.m_classId = ECClassId((uint64_t)0x41);
    duckEntry.m_baseClass = "TestSchema:Animal";
    duckEntry.m_tables["ts_Animal"] = ChangesetSchemaTableInfo("Primary");
    // Duck no longer has its own Age mapping — it inherits from Animal
    after.GetClassesR()["TestSchema:Duck"] = std::move(duckEntry);
    }

    ChangesetSchemaDiff diff;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchemaDiff::Diff(diff, before, after));

    ASSERT_FALSE(diff.HasErrors());
    ASSERT_TRUE(diff.NeedsTransform());

    // Duck's Age was at js1, now it's at js2 (inherited from Animal)
    bool foundDuckAgeSwap = false;
    for (auto const& swap : diff.m_columnSwaps)
        {
        if (swap.m_classKey == "TestSchema:Duck" && swap.m_accessString == "Age")
            {
            foundDuckAgeSwap = true;
            ASSERT_EQ(Utf8String("js1"), swap.m_oldColumn);
            ASSERT_EQ(Utf8String("js2"), swap.m_newColumn);
            }
        }
    ASSERT_TRUE(foundDuckAgeSwap) << "Should detect Duck.Age column swap from js1 to js2";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test overflow table detection in diff.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, DiffOverflowTableAdded)
    {
    ChangesetSchema before;
    {
    ChangesetSchemaClassEntry entry;
    entry.m_classKey = "TestSchema:Element";
    entry.m_classId = ECClassId((uint64_t)0x30);
    entry.m_tables["ts_Element"] = ChangesetSchemaTableInfo("Primary");
    entry.m_propertyMaps["Label"] = ChangesetSchemaPropertyMap("ts_Element", "js1");
    before.GetClassesR()["TestSchema:Element"] = std::move(entry);
    }

    ChangesetSchema after;
    {
    ChangesetSchemaClassEntry entry;
    entry.m_classKey = "TestSchema:Element";
    entry.m_classId = ECClassId((uint64_t)0x30);
    entry.m_tables["ts_Element"] = ChangesetSchemaTableInfo("Primary");
    entry.m_tables["ts_Element_Overflow"] = ChangesetSchemaTableInfo("Overflow"); // NEW
    entry.m_propertyMaps["Label"] = ChangesetSchemaPropertyMap("ts_Element", "js1");
    entry.m_propertyMaps["NewProp"] = ChangesetSchemaPropertyMap("ts_Element_Overflow", "os1");
    after.GetClassesR()["TestSchema:Element"] = std::move(entry);
    }

    ChangesetSchemaDiff diff;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchemaDiff::Diff(diff, before, after));

    ASSERT_FALSE(diff.HasErrors());
    ASSERT_TRUE(diff.NeedsTransform());
    ASSERT_EQ(1u, diff.m_overflowTablesAdded.size());
    ASSERT_EQ(Utf8String("TestSchema:Element"), diff.m_overflowTablesAdded[0].m_classKey);
    ASSERT_EQ(Utf8String("ts_Element_Overflow"), diff.m_overflowTablesAdded[0].m_overflowTable);
    ASSERT_EQ(Utf8String("ts_Element"), diff.m_overflowTablesAdded[0].m_parentTable);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test diff JSON serialization.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, DiffToJson)
    {
    ChangesetSchemaDiff diff;

    // Add a class ID remap
    ChangesetSchemaDiff::ClassIdRemap remap;
    remap.m_classKey = "TestSchema:Duck";
    remap.m_oldClassId = ECClassId((uint64_t)0x41);
    remap.m_newClassId = ECClassId((uint64_t)0x51);
    diff.m_classIdRemaps.push_back(std::move(remap));

    // Add a column swap
    ChangesetSchemaDiff::ColumnSwap swap;
    swap.m_classKey = "TestSchema:Duck";
    swap.m_accessString = "Age";
    swap.m_oldTable = "ts_Animal";
    swap.m_oldColumn = "js1";
    swap.m_newTable = "ts_Animal";
    swap.m_newColumn = "js2";
    diff.m_columnSwaps.push_back(std::move(swap));

    // Serialize
    BeJsDocument json;
    diff.ToJson(json);

    ASSERT_TRUE(json.isMember("classIdRemaps"));
    ASSERT_TRUE(json["classIdRemaps"].isArray());
    ASSERT_EQ(1u, json["classIdRemaps"].size());
    ASSERT_STREQ("TestSchema:Duck", json["classIdRemaps"][0u]["class"].asCString());

    ASSERT_TRUE(json.isMember("columnSwaps"));
    ASSERT_TRUE(json["columnSwaps"].isArray());
    ASSERT_EQ(1u, json["columnSwaps"].size());
    ASSERT_STREQ("Age", json["columnSwaps"][0u]["accessString"].asCString());
    ASSERT_STREQ("js1", json["columnSwaps"][0u]["old"]["column"].asCString());
    ASSERT_STREQ("js2", json["columnSwaps"][0u]["new"]["column"].asCString());
    }

END_ECDBUNITTESTS_NAMESPACE
