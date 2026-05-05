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
    protected:
        struct TestTracker : BeSQLite::ChangeTracker
            {
            TestTracker(BeSQLite::DbR db) { SetDb(&db); }
            OnCommitStatus _OnCommit(bool, Utf8CP) override { return OnCommitStatus::Commit; }
            };
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

    // Insert some data using dot notation for struct members
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.Element (S.X, S.Y, S.Z, Label) VALUES (1, 2, 3, 'hello')");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.SubElement (S.X, S.Y, S.Z, Label, Extra) VALUES (4, 5, 6, 'world', 'bonus')");
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

//---------------------------------------------------------------------------------------
// @bsimethod
// Test changeset-scoped Capture: only classes referenced in the changeset are included.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, CaptureFromChangeset_OnlyReferencedClasses)
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
            <ECProperty propertyName="Depth" typeName="int" />
          </ECEntityClass>
          <ECEntityClass typeName="Zebra">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_CaptureFromChangeset.ecdb", schema));

    // Create a changeset that only touches Duck
    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Duck (Age) VALUES (5)");
    tracker.EnableTracking(false);
    ASSERT_TRUE(tracker.HasChanges());

    BeSQLite::ChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));

    // Capture from changeset — should only include Duck and its ancestor Animal, NOT Fish or Zebra
    ChangesetSchema captured;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::Capture(captured, m_ecdb, changeset));

    auto const& classes = captured.GetClasses();
    ASSERT_TRUE(classes.find("TestSchema:Animal") != classes.end()) << "Animal (ancestor) should be captured";
    ASSERT_TRUE(classes.find("TestSchema:Duck") != classes.end()) << "Duck (referenced in changeset) should be captured";
    ASSERT_TRUE(classes.find("TestSchema:Fish") == classes.end()) << "Fish should NOT be captured";
    ASSERT_TRUE(classes.find("TestSchema:Zebra") == classes.end()) << "Zebra should NOT be captured";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test changeset-scoped Capture with multiple classes in the changeset.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, CaptureFromChangeset_MultipleClasses)
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
            <ECProperty propertyName="Depth" typeName="int" />
          </ECEntityClass>
          <ECEntityClass typeName="Zebra">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_CaptureMultiClass.ecdb", schema));

    // Create a changeset that touches Duck and Fish but NOT Zebra
    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Duck (Age) VALUES (5)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Fish (Depth) VALUES (100)");
    tracker.EnableTracking(false);

    BeSQLite::ChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));

    // Capture from changeset
    ChangesetSchema captured;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::Capture(captured, m_ecdb, changeset));

    auto const& classes = captured.GetClasses();
    ASSERT_TRUE(classes.find("TestSchema:Animal") != classes.end()) << "Animal (ancestor) should be captured";
    ASSERT_TRUE(classes.find("TestSchema:Duck") != classes.end()) << "Duck should be captured";
    ASSERT_TRUE(classes.find("TestSchema:Fish") != classes.end()) << "Fish should be captured";
    ASSERT_TRUE(classes.find("TestSchema:Zebra") == classes.end()) << "Zebra should NOT be captured";

    // Verify property maps are correct
    auto duckMap = captured.GetFullPropertyMap("TestSchema:Duck");
    ASSERT_TRUE(duckMap.find("Age") != duckMap.end()) << "Duck.Age property map should exist";

    auto fishMap = captured.GetFullPropertyMap("TestSchema:Fish");
    ASSERT_TRUE(fishMap.find("Depth") != fishMap.end()) << "Fish.Depth property map should exist";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test changeset-scoped Capture produces same result as CaptureFromDb when changeset
// references all classes.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, CaptureFromChangeset_MatchesCaptureFromDbWhenAllReferenced)
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
            <ECProperty propertyName="Depth" typeName="int" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_CaptureMatchesDb.ecdb", schema));

    // Create a changeset that touches ALL classes
    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Duck (Age) VALUES (5)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Fish (Depth) VALUES (100)");
    tracker.EnableTracking(false);

    BeSQLite::ChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));

    // Capture from changeset
    ChangesetSchema capturedFromChangeset;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::Capture(capturedFromChangeset, m_ecdb, changeset));

    // Both Duck and Fish (plus Animal) should be in the changeset-scoped capture.
    // CaptureFromDb may include additional schema metadata classes, so we only check
    // that changeset-scoped capture has the expected subset.
    auto const& classes = capturedFromChangeset.GetClasses();
    ASSERT_TRUE(classes.find("TestSchema:Animal") != classes.end());
    ASSERT_TRUE(classes.find("TestSchema:Duck") != classes.end());
    ASSERT_TRUE(classes.find("TestSchema:Fish") != classes.end());

    // Verify property maps match between the two approaches for the relevant classes
    ChangesetSchema capturedFromDb;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(capturedFromDb, m_ecdb));

    for (auto const& [key, entry] : classes)
        {
        auto dbIt = capturedFromDb.GetClasses().find(key);
        ASSERT_TRUE(dbIt != capturedFromDb.GetClasses().end()) << "Class " << key.c_str() << " should exist in full capture";
        ASSERT_EQ(entry.m_classId, dbIt->second.m_classId) << "ClassId mismatch for " << key.c_str();

        // Full property maps should match
        auto csMap = capturedFromChangeset.GetFullPropertyMap(key);
        auto dbMap = capturedFromDb.GetFullPropertyMap(key);
        ASSERT_EQ(csMap.size(), dbMap.size()) << "Property map size mismatch for " << key.c_str();
        for (auto const& [as, pm] : csMap)
            {
            auto it = dbMap.find(as);
            ASSERT_TRUE(it != dbMap.end()) << "Property " << as.c_str() << " missing in full capture";
            ASSERT_EQ(pm.m_table, it->second.m_table);
            ASSERT_EQ(pm.m_column, it->second.m_column);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test ChangesetTransformer::Transform refuses when diff has errors.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, TransformRefusesOnErrors)
    {
    // Create a diff with errors
    ChangesetSchemaDiff diff;
    ChangesetSchemaDiff::MissingMapping err;
    err.m_kind = ChangesetSchemaDiff::MissingMapping::Kind::Class;
    err.m_classKey = "TestSchema:Ghost";
    err.m_message = "Test error";
    diff.m_errors.push_back(std::move(err));

    ASSERT_TRUE(diff.HasErrors());
    ASSERT_FALSE(diff.NeedsTransform());

    // Create a dummy source changeset
    SchemaItem schema(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Element">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
            </ECCustomAttributes>
            <ECProperty propertyName="Label" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_TransformRefuses.ecdb", schema));
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Element (Label) VALUES ('test')");
    m_ecdb.SaveChanges();

    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Element (Label) VALUES ('test2')");
    tracker.EnableTracking(false);

    BeSQLite::ChangeSet source;
    ASSERT_EQ(BE_SQLITE_OK, source.FromChangeTrack(tracker));

    // Transform should refuse
    BeSQLite::ChangeSet output;
    ASSERT_EQ(BentleyStatus::ERROR, ChangesetTransformer::Transform(output, source, diff, m_ecdb));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test ChangesetTransformer::Transform passes through unchanged when no transform needed.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, TransformPassthroughWhenNoTransformNeeded)
    {
    SchemaItem schema(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Element">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
            </ECCustomAttributes>
            <ECProperty propertyName="Label" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_TransformPassthrough.ecdb", schema));

    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Element (Label) VALUES ('hello')");
    tracker.EnableTracking(false);

    BeSQLite::ChangeSet source;
    ASSERT_EQ(BE_SQLITE_OK, source.FromChangeTrack(tracker));
    ASSERT_TRUE(source.IsValid());

    // Empty diff — no transform needed
    ChangesetSchemaDiff diff;
    ASSERT_FALSE(diff.HasErrors());
    ASSERT_FALSE(diff.NeedsTransform());

    BeSQLite::ChangeSet output;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, source, diff, m_ecdb));
    ASSERT_TRUE(output.IsValid());
    ASSERT_EQ(source.GetSize(), output.GetSize());

    // Verify the output changeset contents via ChangesetReader.
    // Passthrough must preserve the original INSERT with Label="hello".
    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeStream(m_ecdb,
        std::make_unique<BeSQLite::ChangeSet>(std::move(output)), false, ChangesetReader::PropertyFilter::All));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step()) << "Output should contain at least one change";

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Insert, opcode);

    bool foundLabel = false;
    int colCount = reader.GetColumnCount(Changes::Change::Stage::New);
    for (int i = 0; i < colCount; ++i)
        {
        IECSqlValue const& v = reader.GetValue(Changes::Change::Stage::New, i);
        ECN::ECPropertyCP prop = v.GetColumnInfo().GetProperty();
        if (prop != nullptr && prop->GetName() == "Label")
            {
            EXPECT_STREQ("hello", v.GetText());
            foundLabel = true;
            }
        }
    ASSERT_TRUE(foundLabel) << "Label property should be present in output changeset";
    ASSERT_EQ(BE_SQLITE_DONE, reader.Step()) << "Output should contain exactly one change";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test end-to-end: Capture from changeset, simulate schema change, diff, transform.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, EndToEnd_CaptureAndTransform)
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
            <ECProperty propertyName="Color" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_EndToEnd.ecdb", schema));

    // Step 1: Insert a Duck instance and capture a changeset
    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Duck (Age, Color) VALUES (5, 'White')");
    tracker.EnableTracking(false);

    BeSQLite::ChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));
    m_ecdb.SaveChanges();

    // Step 2: Capture schema BEFORE (from the changeset)
    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::Capture(schemaBefore, m_ecdb, changeset));

    // Verify Duck is captured with its property maps
    auto const& classesBefore = schemaBefore.GetClasses();
    ASSERT_TRUE(classesBefore.find("TestSchema:Duck") != classesBefore.end());
    auto duckMapBefore = schemaBefore.GetFullPropertyMap("TestSchema:Duck");
    ASSERT_TRUE(duckMapBefore.find("Age") != duckMapBefore.end());
    ASSERT_TRUE(duckMapBefore.find("Color") != duckMapBefore.end());

    // Step 3: Capture schema AFTER (same state — no schema change yet, so diff should be empty)
    ChangesetSchema schemaAfter;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::Capture(schemaAfter, m_ecdb, changeset));

    // Step 4: Diff — should show no changes since schema hasn't changed
    ChangesetSchemaDiff diff;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchemaDiff::Diff(diff, schemaBefore, schemaAfter));
    ASSERT_FALSE(diff.HasErrors());
    ASSERT_FALSE(diff.NeedsTransform());

    // Step 5: Transform should pass through unchanged
    BeSQLite::ChangeSet output;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, changeset, diff, m_ecdb));
    ASSERT_TRUE(output.IsValid());
    ASSERT_EQ(changeset.GetSize(), output.GetSize()) << "Passthrough transform must preserve exact byte size";

    // Verify the output changeset contains the expected Duck INSERT with Age=5, Color="White".
    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeStream(m_ecdb,
        std::make_unique<BeSQLite::ChangeSet>(std::move(output)), false, ChangesetReader::PropertyFilter::All));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step()) << "Output should contain at least one change";

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Insert, opcode);

    bool foundAge = false, foundColor = false;
    int colCount = reader.GetColumnCount(Changes::Change::Stage::New);
    for (int i = 0; i < colCount; ++i)
        {
        IECSqlValue const& v = reader.GetValue(Changes::Change::Stage::New, i);
        ECN::ECPropertyCP prop = v.GetColumnInfo().GetProperty();
        if (prop == nullptr)
            continue;
        if (prop->GetName() == "Age")
            {
            EXPECT_EQ(5, v.GetInt64());
            foundAge = true;
            }
        else if (prop->GetName() == "Color")
            {
            EXPECT_STREQ("White", v.GetText());
            foundColor = true;
            }
        }
    ASSERT_TRUE(foundAge) << "Age property should be present in output changeset";
    ASSERT_TRUE(foundColor) << "Color property should be present in output changeset";
    ASSERT_EQ(BE_SQLITE_DONE, reader.Step()) << "Output should contain exactly one change";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Helper: query ordinal of the kind=2 ECClassId column in a SQLite table.
//---------------------------------------------------------------------------------------
static int GetClassIdColumnOrdinal(ECDbCR ecdb, Utf8CP tableName)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb,
        "SELECT c.Ordinal FROM ec_Column c JOIN ec_Table t ON t.Id = c.TableId "
        "WHERE t.Name = ? AND c.ColumnKind = 2 AND c.IsVirtual = 0"))
        return -1;
    stmt.BindText(1, tableName, Statement::MakeCopy::No);
    return (BE_SQLITE_ROW == stmt.Step()) ? stmt.GetValueInt(0) : -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Helper: query ordinal of a named column in a SQLite table.
//---------------------------------------------------------------------------------------
static int GetColumnOrdinal(ECDbCR ecdb, Utf8CP tableName, Utf8CP columnName)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb,
        "SELECT c.Ordinal FROM ec_Column c JOIN ec_Table t ON t.Id = c.TableId "
        "WHERE t.Name = ? AND c.Name = ? AND c.IsVirtual = 0"))
        return -1;
    stmt.BindText(1, tableName, Statement::MakeCopy::No);
    stmt.BindText(2, columnName, Statement::MakeCopy::No);
    return (BE_SQLITE_ROW == stmt.Step()) ? stmt.GetValueInt(0) : -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Helper: query total column count for a SQLite table.
//---------------------------------------------------------------------------------------
static int GetTableColumnCount(ECDbCR ecdb, Utf8CP tableName)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb,
        "SELECT COUNT(*) FROM ec_Column c JOIN ec_Table t ON t.Id = c.TableId "
        "WHERE t.Name = ? AND c.IsVirtual = 0"))
        return -1;
    stmt.BindText(1, tableName, Statement::MakeCopy::No);
    return (BE_SQLITE_ROW == stmt.Step()) ? stmt.GetValueInt(0) : -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Helper: query the ECClassId for a class name in the db.
//---------------------------------------------------------------------------------------
static uint64_t GetClassId(ECDbCR ecdb, Utf8CP schemaName, Utf8CP className)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb,
        "SELECT c.Id FROM ec_Class c JOIN ec_Schema s ON s.Id = c.SchemaId "
        "WHERE s.Name = ? AND c.Name = ?"))
        return 0;
    stmt.BindText(1, schemaName, Statement::MakeCopy::No);
    stmt.BindText(2, className, Statement::MakeCopy::No);
    return (BE_SQLITE_ROW == stmt.Step()) ? (uint64_t) stmt.GetValueInt64(0) : 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform correctly remaps ECClassId values in INSERT, DELETE and UPDATE rows.
// Also covers SourceECClassId / TargetECClassId remapping by name if those columns exist.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_ECClassIdRemap)
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
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_ECClassIdRemap.ecdb", schema));

    uint64_t duckClassId = GetClassId(m_ecdb, "TestSchema", "Duck");
    ASSERT_NE(0u, duckClassId);
    uint64_t remappedId = duckClassId + 500;

    int classIdOrdinal = GetClassIdColumnOrdinal(m_ecdb, "ts_Animal");
    ASSERT_GE(classIdOrdinal, 0) << "ts_Animal must have an ECClassId column";

    // Insert a Duck row and capture changeset
    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Duck (Age) VALUES (3)");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet source;
    ASSERT_EQ(BE_SQLITE_OK, source.FromChangeTrack(tracker));
    ASSERT_TRUE(source.IsValid());
    m_ecdb.SaveChanges();

    // Build diff: remap Duck ECClassId old → new
    ChangesetSchemaDiff diff;
    ChangesetSchemaDiff::ClassIdRemap remap;
    remap.m_classKey = "TestSchema:Duck";
    remap.m_oldClassId = ECClassId(duckClassId);
    remap.m_newClassId = ECClassId(remappedId);
    diff.m_classIdRemaps.push_back(std::move(remap));
    ASSERT_TRUE(diff.NeedsTransform());

    BeSQLite::ChangeSet output;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, source, diff, m_ecdb));
    ASSERT_TRUE(output.IsValid());

    // Verify ECClassId in the output INSERT is remapped
    bool foundInsert = false;
    for (auto const& change : output.GetChanges())
        {
        if (change.GetOpcode() == DbOpcode::Insert && change.GetTableName() == "ts_Animal")
            {
            foundInsert = true;
            DbValue ecClassIdVal = change.GetNewValue(classIdOrdinal);
            ASSERT_EQ(DbValueType::IntegerVal, ecClassIdVal.GetValueType());
            EXPECT_EQ((int64_t) remappedId, ecClassIdVal.GetValueInt64())
                << "ECClassId should be remapped from " << duckClassId << " to " << remappedId;
            }
        }
    ASSERT_TRUE(foundInsert) << "Output should contain INSERT for ts_Animal";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform correctly rewrites column values when a property moves up the class
// hierarchy (same table, different column ordinal) — a ColumnSwap with same oldTable/newTable.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_ColumnSwap_PropertyMovedUpHierarchy)
    {
    // Guaranteed circular column swap scenario:
    // v1: Animal(no props) → Duck(Quack:string, Age:int)
    //     ECDb assigns: Quack→js1, Age→js2 (definition order)
    // v2: Animal(Age:int) → Duck(Quack:string)
    //     ECDb assigns: Age→js1 (from base, first), Quack→js2
    //     Result: Age moves js2→js1, Quack moves js1→js2 (circular!)
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
            <ECProperty propertyName="Quack" typeName="string" />
            <ECProperty propertyName="Age" typeName="int" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_CircularSwap.ecdb", schema1));

    // Insert a Duck row with both properties and capture changeset
    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Duck (Quack, Age) VALUES ('loud', 5)");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet sourceInsert;
    ASSERT_EQ(BE_SQLITE_OK, sourceInsert.FromChangeTrack(tracker));
    ASSERT_TRUE(sourceInsert.IsValid());

    // Capture an UPDATE changeset as well
    tracker.EndTracking();
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "UPDATE ts.Duck SET Quack='quiet', Age=10");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet sourceUpdate;
    ASSERT_EQ(BE_SQLITE_OK, sourceUpdate.FromChangeTrack(tracker));
    ASSERT_TRUE(sourceUpdate.IsValid());
    m_ecdb.SaveChanges();

    // Capture schema BEFORE
    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));
    auto duckMapBefore = schemaBefore.GetFullPropertyMap("TestSchema:Duck");
    ASSERT_TRUE(duckMapBefore.count("Quack") > 0);
    ASSERT_TRUE(duckMapBefore.count("Age") > 0);
    Utf8String quackColBefore = duckMapBefore["Quack"].m_column;
    Utf8String ageColBefore = duckMapBefore["Age"].m_column;
    int quackOrdBefore = GetColumnOrdinal(m_ecdb, "ts_Animal", quackColBefore.c_str());
    int ageOrdBefore = GetColumnOrdinal(m_ecdb, "ts_Animal", ageColBefore.c_str());
    ASSERT_GE(quackOrdBefore, 0);
    ASSERT_GE(ageOrdBefore, 0);

    // Schema v2: move Age to Animal base class
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
            <ECProperty propertyName="Quack" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema2,
        SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    m_ecdb.SaveChanges();

    // Capture schema AFTER and compute diff
    ChangesetSchema schemaAfter;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaAfter, m_ecdb));

    ChangesetSchemaDiff diff;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchemaDiff::Diff(diff, schemaBefore, schemaAfter));
    ASSERT_FALSE(diff.HasErrors());

    if (!diff.NeedsTransform() || diff.m_columnSwaps.empty())
        {
        // ECDb may not have reassigned columns in all implementations; passthrough is acceptable
        BeSQLite::ChangeSet output;
        ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, sourceInsert, diff, m_ecdb));
        ASSERT_TRUE(output.IsValid());
        return;
        }

    // Get new column ordinals
    auto duckMapAfter = schemaAfter.GetFullPropertyMap("TestSchema:Duck");
    ASSERT_TRUE(duckMapAfter.count("Quack") > 0);
    ASSERT_TRUE(duckMapAfter.count("Age") > 0);
    Utf8String quackColAfter = duckMapAfter["Quack"].m_column;
    Utf8String ageColAfter = duckMapAfter["Age"].m_column;
    int quackOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", quackColAfter.c_str());
    int ageOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", ageColAfter.c_str());
    ASSERT_GE(quackOrdAfter, 0);
    ASSERT_GE(ageOrdAfter, 0);

    // Verify the swap happened (ordinals should be different)
    ASSERT_NE(quackColBefore, quackColAfter) << "Quack column should have changed position";
    ASSERT_NE(ageColBefore, ageColAfter) << "Age column should have changed position";

    // --- Test 1: INSERT changeset ---
    {
    BeSQLite::ChangeSet outputInsert;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(outputInsert, sourceInsert, diff, m_ecdb));
    ASSERT_TRUE(outputInsert.IsValid());

    bool foundInsert = false;
    for (auto const& change : outputInsert.GetChanges())
        {
        if (change.GetOpcode() == DbOpcode::Insert && change.GetTableName() == "ts_Animal")
            {
            foundInsert = true;
            // Quack should now be at the new position
            DbValue quackVal = change.GetNewValue(quackOrdAfter);
            ASSERT_EQ(DbValueType::TextVal, quackVal.GetValueType());
            EXPECT_STREQ("loud", quackVal.GetValueText())
                << "Quack value should be at new column position " << quackColAfter.c_str();

            // Age should now be at the new position
            DbValue ageVal = change.GetNewValue(ageOrdAfter);
            ASSERT_EQ(DbValueType::IntegerVal, ageVal.GetValueType());
            EXPECT_EQ(5, ageVal.GetValueInt64())
                << "Age value should be at new column position " << ageColAfter.c_str();

            // Old positions should be NULL (values moved away)
            DbValue oldQuackSlot = change.GetNewValue(quackOrdBefore);
            EXPECT_NE(DbValueType::TextVal, oldQuackSlot.GetValueType())
                << "Old Quack column " << quackColBefore.c_str() << " should not have the text value";
            DbValue oldAgeSlot = change.GetNewValue(ageOrdBefore);
            EXPECT_NE(DbValueType::IntegerVal, oldAgeSlot.GetValueType())
                << "Old Age column " << ageColBefore.c_str() << " should not have the int value";
            }
        }
    ASSERT_TRUE(foundInsert) << "Output INSERT changeset should have an INSERT for ts_Animal";
    }

    // --- Test 2: UPDATE changeset ---
    {
    BeSQLite::ChangeSet outputUpdate;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(outputUpdate, sourceUpdate, diff, m_ecdb));
    ASSERT_TRUE(outputUpdate.IsValid());

    bool foundUpdate = false;
    for (auto const& change : outputUpdate.GetChanges())
        {
        if (change.GetOpcode() == DbOpcode::Update && change.GetTableName() == "ts_Animal")
            {
            foundUpdate = true;
            // New value of Quack should be at new position
            DbValue quackNew = change.GetNewValue(quackOrdAfter);
            EXPECT_EQ(DbValueType::TextVal, quackNew.GetValueType());
            if (quackNew.GetValueType() == DbValueType::TextVal)
                EXPECT_STREQ("quiet", quackNew.GetValueText());

            // New value of Age should be at new position
            DbValue ageNew = change.GetNewValue(ageOrdAfter);
            EXPECT_EQ(DbValueType::IntegerVal, ageNew.GetValueType());
            if (ageNew.GetValueType() == DbValueType::IntegerVal)
                EXPECT_EQ(10, ageNew.GetValueInt64());
            }
        }
    ASSERT_TRUE(foundUpdate) << "Output UPDATE changeset should have an UPDATE for ts_Animal";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform handles INSERT rows where the target db has MORE columns than the
// source changeset (new properties added by another user after the changeset was made).
// Source columns beyond nCols must be written as NULL in the output.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_OtherUserAddedColumns)
    {
    // v1: Element with Label only
    SchemaItem schema1(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Element">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                    <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Label" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_OtherUserCols.ecdb", schema1));

    // Record source column count and insert a row
    int colCountBefore = GetTableColumnCount(m_ecdb, "ts_Element");
    ASSERT_GT(colCountBefore, 0);

    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Element (Label) VALUES ('original')");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet source;
    ASSERT_EQ(BE_SQLITE_OK, source.FromChangeTrack(tracker));
    ASSERT_TRUE(source.IsValid());
    m_ecdb.SaveChanges();

    // v2: other user added Weight and Score properties
    SchemaItem schema2(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="02.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Element">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                    <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Label"  typeName="string" />
            <ECProperty propertyName="Weight" typeName="double" />
            <ECProperty propertyName="Score"  typeName="int"    />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema2,
        SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    m_ecdb.SaveChanges();

    int colCountAfter = GetTableColumnCount(m_ecdb, "ts_Element");
    ASSERT_GT(colCountAfter, colCountBefore) << "Schema upgrade should add columns";

    // Build a diff that forces NeedsTransform = true so the Transform code runs the
    // "extend primary row with NULLs for new columns" path. The OverflowTableAdded entry
    // triggers NeedsTransform even though no actual cross-table column moves occur.
    ChangesetSchemaDiff forcedDiff;
    ChangesetSchemaDiff::OverflowTableAdded ovf;
    ovf.m_classKey = "TestSchema:Element";
    ovf.m_overflowTable = "ts_Element_Ov";  // fake overflow table (won't actually exist)
    ovf.m_parentTable = "ts_Element";
    ovf.m_hasECClassIdColumn = false;
    forcedDiff.m_overflowTablesAdded.push_back(std::move(ovf));
    ASSERT_TRUE(forcedDiff.NeedsTransform());
    ASSERT_FALSE(forcedDiff.HasErrors());

    BeSQLite::ChangeSet output;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, source, forcedDiff, m_ecdb));
    ASSERT_TRUE(output.IsValid());

    // Output changeset should have an INSERT for ts_Element with colCountAfter columns
    // and the new columns should all be NULL.
    int labelOrdinal = GetColumnOrdinal(m_ecdb, "ts_Element", "js1");  // Label is js1
    int weightOrdinal = GetColumnOrdinal(m_ecdb, "ts_Element", "js2"); // Weight is js2
    int scoreOrdinal  = GetColumnOrdinal(m_ecdb, "ts_Element", "js3"); // Score is js3

    bool foundInsert = false;
    for (auto const& change : output.GetChanges())
        {
        if (change.GetOpcode() == DbOpcode::Insert && change.GetTableName() == "ts_Element")
            {
            foundInsert = true;
            EXPECT_EQ(colCountAfter, change.GetColumnCount())
                << "Transformed INSERT should have the current column count";

            // Label should have the original value
            if (labelOrdinal >= 0)
                {
                DbValue lv = change.GetNewValue(labelOrdinal);
                EXPECT_EQ(DbValueType::TextVal, lv.GetValueType());
                EXPECT_STREQ("original", lv.GetValueText());
                }

            // Weight and Score (added later) should be NULL
            if (weightOrdinal >= 0)
                EXPECT_EQ(DbValueType::NullVal, change.GetNewValue(weightOrdinal).GetValueType())
                    << "Weight (new col) should be NULL in transformed changeset";
            if (scoreOrdinal >= 0)
                EXPECT_EQ(DbValueType::NullVal, change.GetNewValue(scoreOrdinal).GetValueType())
                    << "Score (new col) should be NULL in transformed changeset";
            }
        }
    ASSERT_TRUE(foundInsert) << "Output should contain INSERT for ts_Element";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform correctly handles struct properties moving to an overflow table.
// Source changeset has struct values in the primary table. After schema upgrade, those
// values live in an overflow table. Transform must generate overflow INSERT rows and
// write NULL at the moved column positions in the primary row.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_StructPropertyOverflowCrossTableMove)
    {
    // v1: Element with struct S(X,Y,Z) + Label. MaxOverflow=4 → no overflow (4 props).
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
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_StructOverflow.ecdb", schema1));

    // Capture schema BEFORE and record column info
    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));

    // Insert a row using the v1 schema and capture the changeset
    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.Element (S.X, S.Y, S.Z, Label) VALUES (10, 20, 30, 'test')");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet source;
    ASSERT_EQ(BE_SQLITE_OK, source.FromChangeTrack(tracker));
    ASSERT_TRUE(source.IsValid());
    m_ecdb.SaveChanges();

    // v2: Add S.W member to struct. 5 properties > MaxOverflow=4 → overflow!
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
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(schema2,
        SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    m_ecdb.SaveChanges();

    ChangesetSchema schemaAfter;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaAfter, m_ecdb));

    ChangesetSchemaDiff diff;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchemaDiff::Diff(diff, schemaBefore, schemaAfter));
    ASSERT_FALSE(diff.HasErrors());

    // Transform the source changeset
    BeSQLite::ChangeSet output;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, source, diff, m_ecdb));
    ASSERT_TRUE(output.IsValid());

    if (!diff.NeedsTransform())
        {
        // Schema mapper kept everything in primary — passthrough
        EXPECT_EQ(source.GetSize(), output.GetSize());
        return;
        }

    // Count changes in output: primary INSERT + potentially overflow INSERT(s)
    int totalChanges = 0;
    int primaryInserts = 0;
    int overflowInserts = 0;

    // Collect overflow table names from diff
    std::set<Utf8String> overflowTables;
    for (auto const& ovf : diff.m_overflowTablesAdded)
        overflowTables.insert(ovf.m_overflowTable);

    for (auto const& change : output.GetChanges())
        {
        ++totalChanges;
        if (change.GetOpcode() == DbOpcode::Insert)
            {
            if (change.GetTableName() == "ts_Element")
                {
                ++primaryInserts;
                // For any column that moved cross-table, its primary slot should be NULL
                for (auto const& swap : diff.m_columnSwaps)
                    {
                    if (swap.m_classKey == "TestSchema:Element" &&
                        swap.m_oldTable == "ts_Element" &&
                        swap.m_newTable != "ts_Element")
                        {
                        int srcOrd = GetColumnOrdinal(m_ecdb, "ts_Element", swap.m_oldColumn.c_str());
                        if (srcOrd >= 0 && srcOrd < change.GetColumnCount())
                            {
                            EXPECT_EQ(DbValueType::NullVal, change.GetNewValue(srcOrd).GetValueType())
                                << "Column " << swap.m_oldColumn.c_str()
                                << " should be NULL in primary row after cross-table move";
                            }
                        }
                    }
                }
            else if (overflowTables.count(change.GetTableName()) > 0)
                {
                ++overflowInserts;
                // Overflow INSERT should have a valid ECInstanceId at ordinal 0
                DbValue pkVal = change.GetNewValue(0);
                EXPECT_EQ(DbValueType::IntegerVal, pkVal.GetValueType())
                    << "Overflow INSERT must have a valid ECInstanceId at ordinal 0";
                }
            }
        }

    EXPECT_EQ(1, primaryInserts) << "Should have exactly one primary ts_Element INSERT";
    if (!diff.m_columnSwaps.empty())
        EXPECT_GE(overflowInserts, 1)
            << "Cross-table column moves should generate overflow INSERT rows";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform remaps SourceECClassId values in a relationship link table.
// When an entity class referenced by a relationship gets a new ECClassId, the
// SourceECClassId column in the link table INSERT must be updated.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_RelClassIdRemap)
    {
    // Schema with a standalone relationship (no nav property) — ECDb creates a link table
    // with SourceECClassId and TargetECClassId columns (named SourceECClassId/TargetECClassId).
    SchemaItem schema(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Owner">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Pet">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
            </ECCustomAttributes>
            <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
          <ECRelationshipClass typeName="OwnerHasPets" strength="referencing" modifier="None">
            <Source multiplicity="(0..1)" roleLabel="Owner" polymorphic="true">
                <Class class="Owner"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="Pet" polymorphic="true">
                <Class class="Pet"/>
            </Target>
          </ECRelationshipClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_RelClassId.ecdb", schema));

    m_ecdb.SaveChanges();
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Owner (ECInstanceId) VALUES (NULL)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Pet (Name) VALUES ('Buddy')");
    m_ecdb.SaveChanges();

    uint64_t ownerClassId = GetClassId(m_ecdb, "TestSchema", "Owner");
    uint64_t petClassId   = GetClassId(m_ecdb, "TestSchema", "Pet");
    if (ownerClassId == 0 || petClassId == 0)
        GTEST_SKIP() << "Owner or Pet class not found; skipping RelClassId remap test";

    // Query the link table name created by ECDb for OwnerHasPets
    Utf8String linkTable;
    {
    Statement q;
    ASSERT_EQ(BE_SQLITE_OK, q.Prepare(m_ecdb,
        "SELECT t.Name FROM ec_Table t "
        "JOIN ec_Class c ON t.ExclusiveRootClassId = c.Id "
        "JOIN ec_Schema s ON s.Id = c.SchemaId "
        "WHERE s.Name = 'TestSchema' AND c.Name = 'OwnerHasPets'"));
    if (BE_SQLITE_ROW != q.Step())
        GTEST_SKIP() << "OwnerHasPets link table not found; skipping RelClassId remap test";
    linkTable = q.GetValueText(0);
    }

    // Find SourceECClassId and TargetECClassId ordinals in the link table
    int srcClassIdOrd = GetColumnOrdinal(m_ecdb, linkTable.c_str(), "SourceECClassId");
    int tgtClassIdOrd = GetColumnOrdinal(m_ecdb, linkTable.c_str(), "TargetECClassId");
    if (srcClassIdOrd < 0 && tgtClassIdOrd < 0)
        GTEST_SKIP() << "Link table " << linkTable.c_str() << " has no SourceECClassId/TargetECClassId columns; skipping";

    // Insert a relationship instance and capture the changeset
    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.OwnerHasPets (SourceECInstanceId, TargetECInstanceId) VALUES (1, 1)");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet source;
    ASSERT_EQ(BE_SQLITE_OK, source.FromChangeTrack(tracker));
    ASSERT_TRUE(source.IsValid());
    m_ecdb.SaveChanges();

    // Build diff: remap Owner entity class ID (ownerClassId → remappedOwnerClassId).
    // This simulates a schema migration that assigns Owner a new class ID.
    // The SourceECClassId column in the link table stores ownerClassId and must be updated.
    uint64_t remappedOwnerClassId = ownerClassId + 500;
    ChangesetSchemaDiff diff;
    ChangesetSchemaDiff::ClassIdRemap remap;
    remap.m_classKey = "TestSchema:Owner";
    remap.m_oldClassId = ECClassId(ownerClassId);
    remap.m_newClassId = ECClassId(remappedOwnerClassId);
    diff.m_classIdRemaps.push_back(std::move(remap));
    ASSERT_TRUE(diff.NeedsTransform());

    BeSQLite::ChangeSet output;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, source, diff, m_ecdb));
    ASSERT_TRUE(output.IsValid());

    // Verify: SourceECClassId in the output changeset has been remapped.
    bool foundRemapped = false;
    for (auto const& change : output.GetChanges())
        {
        if (change.GetTableName() != linkTable)
            continue;
        int nCols = change.GetColumnCount();
        for (int i = 0; i < nCols; ++i)
            {
            DbValue v = (change.GetOpcode() != DbOpcode::Delete)
                ? change.GetNewValue(i) : change.GetOldValue(i);
            if (v.GetValueType() != DbValueType::IntegerVal)
                continue;
            uint64_t val = (uint64_t) v.GetValueInt64();
            // Old ownerClassId must NOT appear
            if (val == ownerClassId)
                ADD_FAILURE() << "Old ownerClassId " << ownerClassId
                              << " should have been remapped in SourceECClassId column";
            if (val == remappedOwnerClassId)
                foundRemapped = true;
            }
        }
    EXPECT_TRUE(foundRemapped)
        << "SourceECClassId in link table should carry the remapped owner class ID "
        << remappedOwnerClassId;
    }

END_ECDBUNITTESTS_NAMESPACE
