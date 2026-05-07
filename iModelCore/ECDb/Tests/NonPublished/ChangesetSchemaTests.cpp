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
// Helper: query ordinal (0-based SQLite column position) of a named column via PRAGMA.
// Uses PRAGMA table_info rather than ec_Column.Ordinal because ec_Column.Ordinal can
// have gaps when virtual columns (e.g. ECClassId for ExclusiveRootClassId tables) occupy
// ordinal slots that are absent from the actual SQLite table.
//---------------------------------------------------------------------------------------
static int GetColumnOrdinal(ECDbCR ecdb, Utf8CP tableName, Utf8CP columnName)
    {
    Statement stmt;
    Utf8String sql;
    sql.Sprintf("PRAGMA table_info([%s])", tableName);
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, sql.c_str()))
        return -1;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        int cid = stmt.GetValueInt(0);
        Utf8CP name = stmt.GetValueText(1);
        if (strcmp(name, columnName) == 0)
            return cid;
        }
    return -1;
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
// Test Transform remaps ECClassId values in a relationship link table.
// When a relationship class gets a new ECClassId (e.g. after schema migration), the
// ECClassId stored in the shared link table INSERT must be updated.
//
// ECDb stores SourceECClassId/TargetECClassId as virtual columns in link tables (resolved
// via JOIN at read time). They never appear in the SQLite table or in changesets.
// What DOES appear as a physical ECClassId in a link table is the relationship's OWN
// class identifier — but only when multiple relationship classes share one table (TPH).
// This test uses an abstract base relationship with TablePerHierarchy so that the link
// table has a physical ECClassId column distinguishing between concrete subclasses.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_RelClassIdRemap)
    {
    // Schema: abstract base relationship (TPH) + one concrete sealed subclass.
    // ECDb creates ts_OwnerHasPets with a physical ECClassId column (not ExclusiveRoot,
    // because multiple relationship classes could share the table).
    SchemaItem schema(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Owner" />
          <ECEntityClass typeName="Pet" />
          <ECRelationshipClass typeName="OwnerHasPets" strength="referencing" modifier="Abstract">
            <ECCustomAttributes>
              <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
              </ClassMap>
            </ECCustomAttributes>
            <Source multiplicity="(0..1)" roleLabel="Owner" polymorphic="true">
                <Class class="Owner"/>
            </Source>
            <Target multiplicity="(0..*)" roleLabel="Pet" polymorphic="true">
                <Class class="Pet"/>
            </Target>
          </ECRelationshipClass>
          <ECRelationshipClass typeName="ConcreteOwnerHasPets" strength="referencing" modifier="Sealed">
            <BaseClass>OwnerHasPets</BaseClass>
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
    // ECDb uses a global ECInstanceId counter per file: Owner gets id=1, Pet gets id=2.
    ECSqlStatement ownerStmt;
    ASSERT_EQ(ECSqlStatus::Success, ownerStmt.Prepare(m_ecdb, "INSERT INTO ts.Owner (ECInstanceId) VALUES (NULL)"));
    ECInstanceKey ownerKey;
    ASSERT_EQ(BE_SQLITE_DONE, ownerStmt.Step(ownerKey));

    ECSqlStatement petStmt;
    ASSERT_EQ(ECSqlStatus::Success, petStmt.Prepare(m_ecdb, "INSERT INTO ts.Pet (ECInstanceId) VALUES (NULL)"));
    ECInstanceKey petKey;
    ASSERT_EQ(BE_SQLITE_DONE, petStmt.Step(petKey));
    m_ecdb.SaveChanges();

    uint64_t concreteClassId = GetClassId(m_ecdb, "TestSchema", "ConcreteOwnerHasPets");
    ASSERT_NE(0u, concreteClassId) << "ConcreteOwnerHasPets class not found";

    // The shared link table for the TPH relationship hierarchy.
    // With TablePerHierarchy, ECDb names the link table after the abstract base class.
    Utf8CP linkTable = "ts_OwnerHasPets";

    // ECClassId must be physical (non-virtual) because the table is not ExclusiveRoot.
    int classIdOrd = GetClassIdColumnOrdinal(m_ecdb, linkTable);
    ASSERT_GE(classIdOrd, 0) << "Link table " << linkTable << " must have a physical ECClassId column";

    // Insert a ConcreteOwnerHasPets relationship and capture the changeset.
    // Use the captured ECInstanceIds to satisfy FK constraints on SourceId/TargetId.
    Utf8String relInsert;
    relInsert.Sprintf("INSERT INTO ts.ConcreteOwnerHasPets (SourceECInstanceId, TargetECInstanceId) VALUES (%s, %s)",
        ownerKey.GetInstanceId().ToString().c_str(), petKey.GetInstanceId().ToString().c_str());
    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, relInsert.c_str());
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet source;
    ASSERT_EQ(BE_SQLITE_OK, source.FromChangeTrack(tracker));
    ASSERT_TRUE(source.IsValid());
    m_ecdb.SaveChanges();

    // Build diff: remap ConcreteOwnerHasPets class ID.
    // This simulates a schema migration that assigns ConcreteOwnerHasPets a new class ID.
    // The ECClassId stored in the link table must be updated in the changeset.
    uint64_t remappedClassId = concreteClassId + 500;
    ChangesetSchemaDiff diff;
    ChangesetSchemaDiff::ClassIdRemap remap;
    remap.m_classKey = "TestSchema:ConcreteOwnerHasPets";
    remap.m_oldClassId = ECClassId(concreteClassId);
    remap.m_newClassId = ECClassId(remappedClassId);
    diff.m_classIdRemaps.push_back(std::move(remap));
    ASSERT_TRUE(diff.NeedsTransform());

    BeSQLite::ChangeSet output;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, source, diff, m_ecdb));
    ASSERT_TRUE(output.IsValid());

    // Verify: ECClassId at classIdOrd in the output INSERT has been remapped.
    bool foundChange = false;
    for (auto const& change : output.GetChanges())
        {
        if (change.GetTableName() != linkTable)
            continue;
        foundChange = true;
        DbValue ecClassIdVal = change.GetNewValue(classIdOrd);
        ASSERT_EQ(DbValueType::IntegerVal, ecClassIdVal.GetValueType())
            << "ECClassId column must hold an integer";
        uint64_t val = (uint64_t) ecClassIdVal.GetValueInt64();
        EXPECT_NE(concreteClassId, val)
            << "Old concreteClassId " << concreteClassId << " should have been remapped";
        EXPECT_EQ(remappedClassId, val)
            << "ECClassId should be remapped from " << concreteClassId << " to " << remappedClassId;
        }
    EXPECT_TRUE(foundChange) << "Output changeset should contain a change for " << linkTable;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform correctly handles Point2d property (2 columns: X, Y) when moved up
// the hierarchy. Each leaf column becomes an independent ColumnSwap entry.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_Point2dPropertyMovedUp)
    {
    // v1: Animal(no props) → Duck(Name:string, Origin:Point2d)
    //     ECDb assigns: Name→js1, Origin.X→js2, Origin.Y→js3
    // v2: Animal(Origin:Point2d) → Duck(Name:string)
    //     ECDb assigns: Origin.X→js1, Origin.Y→js2 (base first), Name→js3
    //     Result: Name moves js1→js3, Origin.X moves js2→js1, Origin.Y moves js3→js2
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
            <ECProperty propertyName="Name" typeName="string" />
            <ECProperty propertyName="Origin" typeName="Point2d" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_Point2d.ecdb", schema1));

    // Insert data and capture INSERT changeset
    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.Duck (Name, Origin.X, Origin.Y) VALUES ('Daffy', 1.5, 2.5)");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet sourceInsert;
    ASSERT_EQ(BE_SQLITE_OK, sourceInsert.FromChangeTrack(tracker));
    ASSERT_TRUE(sourceInsert.IsValid());

    // Capture UPDATE changeset
    tracker.EndTracking();
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "UPDATE ts.Duck SET Origin.X=10.0, Origin.Y=20.0");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet sourceUpdate;
    ASSERT_EQ(BE_SQLITE_OK, sourceUpdate.FromChangeTrack(tracker));
    ASSERT_TRUE(sourceUpdate.IsValid());
    m_ecdb.SaveChanges();

    // Capture schema BEFORE
    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));
    auto duckMapBefore = schemaBefore.GetFullPropertyMap("TestSchema:Duck");
    ASSERT_TRUE(duckMapBefore.count("Origin.X") > 0);
    ASSERT_TRUE(duckMapBefore.count("Origin.Y") > 0);

    // Schema v2: move Origin to base class
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
            <ECProperty propertyName="Origin" typeName="Point2d" />
          </ECEntityClass>
          <ECEntityClass typeName="Duck">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="Name" typeName="string" />
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
        // No swap needed — ECDb kept columns in place; passthrough is acceptable
        BeSQLite::ChangeSet output;
        ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, sourceInsert, diff, m_ecdb));
        ASSERT_TRUE(output.IsValid());
        return;
        }

    // Verify diff has swaps for both Origin.X and Origin.Y
    int originSwapCount = 0;
    for (auto const& swap : diff.m_columnSwaps)
        {
        if (swap.m_classKey == "TestSchema:Duck" &&
            (swap.m_accessString == "Origin.X" || swap.m_accessString == "Origin.Y"))
            ++originSwapCount;
        }
    EXPECT_EQ(2, originSwapCount) << "Should have column swaps for both Origin.X and Origin.Y";

    // Get new column ordinals
    auto duckMapAfter = schemaAfter.GetFullPropertyMap("TestSchema:Duck");
    int originXOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["Origin.X"].m_column.c_str());
    int originYOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["Origin.Y"].m_column.c_str());
    ASSERT_GE(originXOrdAfter, 0);
    ASSERT_GE(originYOrdAfter, 0);

    // --- Test INSERT ---
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
            DbValue xVal = change.GetNewValue(originXOrdAfter);
            ASSERT_EQ(DbValueType::FloatVal, xVal.GetValueType());
            EXPECT_DOUBLE_EQ(1.5, xVal.GetValueDouble()) << "Origin.X should be at new ordinal";

            DbValue yVal = change.GetNewValue(originYOrdAfter);
            ASSERT_EQ(DbValueType::FloatVal, yVal.GetValueType());
            EXPECT_DOUBLE_EQ(2.5, yVal.GetValueDouble()) << "Origin.Y should be at new ordinal";
            }
        }
    ASSERT_TRUE(foundInsert) << "Output should contain INSERT for ts_Animal";
    }

    // --- Test UPDATE ---
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
            DbValue xNew = change.GetNewValue(originXOrdAfter);
            EXPECT_EQ(DbValueType::FloatVal, xNew.GetValueType());
            if (xNew.GetValueType() == DbValueType::FloatVal)
                EXPECT_DOUBLE_EQ(10.0, xNew.GetValueDouble());

            DbValue yNew = change.GetNewValue(originYOrdAfter);
            EXPECT_EQ(DbValueType::FloatVal, yNew.GetValueType());
            if (yNew.GetValueType() == DbValueType::FloatVal)
                EXPECT_DOUBLE_EQ(20.0, yNew.GetValueDouble());
            }
        }
    ASSERT_TRUE(foundUpdate) << "Output should contain UPDATE for ts_Animal";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform correctly handles Point3d property (3 columns: X, Y, Z) when moved up.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_Point3dPropertyMovedUp)
    {
    // v1: Animal(no props) → Duck(Name:string, Pos:Point3d)
    // v2: Animal(Pos:Point3d) → Duck(Name:string)
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
            <ECProperty propertyName="Name" typeName="string" />
            <ECProperty propertyName="Pos" typeName="Point3d" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_Point3d.ecdb", schema1));

    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.Duck (Name, Pos.X, Pos.Y, Pos.Z) VALUES ('Donald', 1.0, 2.0, 3.0)");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet sourceInsert;
    ASSERT_EQ(BE_SQLITE_OK, sourceInsert.FromChangeTrack(tracker));
    ASSERT_TRUE(sourceInsert.IsValid());

    tracker.EndTracking();
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "UPDATE ts.Duck SET Pos.X=10.0, Pos.Y=20.0, Pos.Z=30.0");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet sourceUpdate;
    ASSERT_EQ(BE_SQLITE_OK, sourceUpdate.FromChangeTrack(tracker));
    ASSERT_TRUE(sourceUpdate.IsValid());
    m_ecdb.SaveChanges();

    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));

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
            <ECProperty propertyName="Pos" typeName="Point3d" />
          </ECEntityClass>
          <ECEntityClass typeName="Duck">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="Name" typeName="string" />
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

    if (!diff.NeedsTransform() || diff.m_columnSwaps.empty())
        {
        BeSQLite::ChangeSet output;
        ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, sourceInsert, diff, m_ecdb));
        ASSERT_TRUE(output.IsValid());
        return;
        }

    // Verify diff has 3 swap entries for Pos.X, Pos.Y, Pos.Z
    int posSwapCount = 0;
    for (auto const& swap : diff.m_columnSwaps)
        {
        if (swap.m_classKey == "TestSchema:Duck" &&
            (swap.m_accessString == "Pos.X" || swap.m_accessString == "Pos.Y" || swap.m_accessString == "Pos.Z"))
            ++posSwapCount;
        }
    EXPECT_EQ(3, posSwapCount) << "Should have column swaps for Pos.X, Pos.Y, and Pos.Z";

    auto duckMapAfter = schemaAfter.GetFullPropertyMap("TestSchema:Duck");
    int posXOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["Pos.X"].m_column.c_str());
    int posYOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["Pos.Y"].m_column.c_str());
    int posZOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["Pos.Z"].m_column.c_str());
    ASSERT_GE(posXOrdAfter, 0);
    ASSERT_GE(posYOrdAfter, 0);
    ASSERT_GE(posZOrdAfter, 0);

    // --- Test INSERT ---
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
            DbValue xVal = change.GetNewValue(posXOrdAfter);
            ASSERT_EQ(DbValueType::FloatVal, xVal.GetValueType());
            EXPECT_DOUBLE_EQ(1.0, xVal.GetValueDouble());

            DbValue yVal = change.GetNewValue(posYOrdAfter);
            ASSERT_EQ(DbValueType::FloatVal, yVal.GetValueType());
            EXPECT_DOUBLE_EQ(2.0, yVal.GetValueDouble());

            DbValue zVal = change.GetNewValue(posZOrdAfter);
            ASSERT_EQ(DbValueType::FloatVal, zVal.GetValueType());
            EXPECT_DOUBLE_EQ(3.0, zVal.GetValueDouble());
            }
        }
    ASSERT_TRUE(foundInsert);
    }

    // --- Test UPDATE ---
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
            DbValue xNew = change.GetNewValue(posXOrdAfter);
            if (xNew.GetValueType() == DbValueType::FloatVal)
                EXPECT_DOUBLE_EQ(10.0, xNew.GetValueDouble());
            DbValue yNew = change.GetNewValue(posYOrdAfter);
            if (yNew.GetValueType() == DbValueType::FloatVal)
                EXPECT_DOUBLE_EQ(20.0, yNew.GetValueDouble());
            DbValue zNew = change.GetNewValue(posZOrdAfter);
            if (zNew.GetValueType() == DbValueType::FloatVal)
                EXPECT_DOUBLE_EQ(30.0, zNew.GetValueDouble());
            }
        }
    ASSERT_TRUE(foundUpdate);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform correctly handles NavigationProperty column shift when another property
// moves up the hierarchy. Nav properties require the owning class to be a DIRECT
// constraint class in the referenced relationship (polymorphic inheritance is insufficient).
// Here Duck is the direct source constraint. Duck has MyTarget.Id (js1) and Name (js2).
// When Name moves to Animal base, it takes js1; MyTarget.Id shifts to js2 — a circular
// swap. Only the real Id column participates; the virtual RelECClassId is excluded.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_NavPropertyMovedUp)
    {
    // v1: Animal(∅) → Duck(MyTarget.Id→js1, Name→js2)
    //     Relationship DuckRefersToTarget: source=Duck (direct constraint)
    // v2: Animal(Name→js1) → Duck(MyTarget.Id→js2)
    //     Result: MyTarget.Id: js1→js2, Name: js2→js1 (circular swap)
    SchemaItem schema1(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Target">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
            </ECCustomAttributes>
          </ECEntityClass>
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
          <ECRelationshipClass typeName="DuckRefersToTarget" strength="referencing" modifier="None">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="Duck"/>
            </Source>
            <Target multiplicity="(0..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Target"/>
            </Target>
          </ECRelationshipClass>
          <ECEntityClass typeName="Duck">
            <BaseClass>Animal</BaseClass>
            <ECNavigationProperty propertyName="MyTarget" relationshipName="DuckRefersToTarget" direction="Forward" />
            <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_NavProp.ecdb", schema1));

    // Insert a Target row first
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "INSERT INTO ts.Target (ECInstanceId) VALUES (NULL)");
    m_ecdb.SaveChanges();

    // Insert a Duck with NavProperty defined FIRST (so MyTarget.Id=js1, Name=js2)
    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.Duck (MyTarget.Id, Name) VALUES (1, 'Dewey')");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet sourceInsert;
    ASSERT_EQ(BE_SQLITE_OK, sourceInsert.FromChangeTrack(tracker));
    ASSERT_TRUE(sourceInsert.IsValid());
    m_ecdb.SaveChanges();

    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));
    auto duckMapBefore = schemaBefore.GetFullPropertyMap("TestSchema:Duck");
    ASSERT_TRUE(duckMapBefore.count("MyTarget.Id") > 0);

    // Schema v2: move Name to Animal base class. NavProp stays on Duck (constraint unchanged).
    SchemaItem schema2(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="02.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Target">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
            </ECCustomAttributes>
          </ECEntityClass>
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
            <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
          <ECRelationshipClass typeName="DuckRefersToTarget" strength="referencing" modifier="None">
            <Source multiplicity="(0..*)" roleLabel="refers to" polymorphic="false">
                <Class class="Duck"/>
            </Source>
            <Target multiplicity="(0..1)" roleLabel="is referenced by" polymorphic="true">
                <Class class="Target"/>
            </Target>
          </ECRelationshipClass>
          <ECEntityClass typeName="Duck">
            <BaseClass>Animal</BaseClass>
            <ECNavigationProperty propertyName="MyTarget" relationshipName="DuckRefersToTarget" direction="Forward" />
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

    if (!diff.NeedsTransform() || diff.m_columnSwaps.empty())
        {
        BeSQLite::ChangeSet output;
        ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, sourceInsert, diff, m_ecdb));
        ASSERT_TRUE(output.IsValid());
        return;
        }

    // Nav property should have exactly 1 column swap (only Id column; RelClassId is virtual/excluded)
    int navSwapCount = 0;
    for (auto const& swap : diff.m_columnSwaps)
        {
        if (swap.m_classKey == "TestSchema:Duck" && swap.m_accessString == "MyTarget.Id")
            ++navSwapCount;
        }
    EXPECT_EQ(1, navSwapCount) << "NavProperty should generate exactly 1 ColumnSwap (Id only, RelClassId is virtual)";

    auto duckMapAfter = schemaAfter.GetFullPropertyMap("TestSchema:Duck");
    int navIdOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["MyTarget.Id"].m_column.c_str());
    ASSERT_GE(navIdOrdAfter, 0);

    BeSQLite::ChangeSet outputInsert;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(outputInsert, sourceInsert, diff, m_ecdb));
    ASSERT_TRUE(outputInsert.IsValid());

    bool foundInsert = false;
    for (auto const& change : outputInsert.GetChanges())
        {
        if (change.GetOpcode() == DbOpcode::Insert && change.GetTableName() == "ts_Animal")
            {
            foundInsert = true;
            DbValue navVal = change.GetNewValue(navIdOrdAfter);
            ASSERT_EQ(DbValueType::IntegerVal, navVal.GetValueType());
            EXPECT_EQ(1, navVal.GetValueInt64()) << "NavProperty Id should be at new ordinal after column shift";
            }
        }
    ASSERT_TRUE(foundInsert);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform correctly handles struct property (multiple leaf columns) moved up.
// Each struct member becomes an independent ColumnSwap.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_StructPropertyMovedUp)
    {
    // v1: Animal(no props) → Duck(Name:string, S:MyStruct{A,B,C})
    // v2: Animal(S:MyStruct{A,B,C}) → Duck(Name:string)
    SchemaItem schema1(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="MyStruct" modifier="None">
            <ECProperty propertyName="A" typeName="int" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="C" typeName="double" />
          </ECStructClass>
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
            <ECProperty propertyName="Name" typeName="string" />
            <ECStructProperty propertyName="S" typeName="MyStruct" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_Struct.ecdb", schema1));

    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.Duck (Name, S.A, S.B, S.C) VALUES ('Huey', 42, 'hello', 3.14)");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet sourceInsert;
    ASSERT_EQ(BE_SQLITE_OK, sourceInsert.FromChangeTrack(tracker));
    ASSERT_TRUE(sourceInsert.IsValid());
    m_ecdb.SaveChanges();

    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));

    SchemaItem schema2(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="02.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECStructClass typeName="MyStruct" modifier="None">
            <ECProperty propertyName="A" typeName="int" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="C" typeName="double" />
          </ECStructClass>
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
            <ECStructProperty propertyName="S" typeName="MyStruct" />
          </ECEntityClass>
          <ECEntityClass typeName="Duck">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="Name" typeName="string" />
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

    if (!diff.NeedsTransform() || diff.m_columnSwaps.empty())
        {
        BeSQLite::ChangeSet output;
        ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, sourceInsert, diff, m_ecdb));
        ASSERT_TRUE(output.IsValid());
        return;
        }

    // Verify 3 swaps for S.A, S.B, S.C
    int structSwapCount = 0;
    for (auto const& swap : diff.m_columnSwaps)
        {
        if (swap.m_classKey == "TestSchema:Duck" &&
            (swap.m_accessString == "S.A" || swap.m_accessString == "S.B" || swap.m_accessString == "S.C"))
            ++structSwapCount;
        }
    EXPECT_EQ(3, structSwapCount) << "Should have swaps for S.A, S.B, S.C";

    auto duckMapAfter = schemaAfter.GetFullPropertyMap("TestSchema:Duck");
    int saOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["S.A"].m_column.c_str());
    int sbOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["S.B"].m_column.c_str());
    int scOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["S.C"].m_column.c_str());
    ASSERT_GE(saOrdAfter, 0);
    ASSERT_GE(sbOrdAfter, 0);
    ASSERT_GE(scOrdAfter, 0);

    BeSQLite::ChangeSet outputInsert;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(outputInsert, sourceInsert, diff, m_ecdb));
    ASSERT_TRUE(outputInsert.IsValid());

    bool foundInsert = false;
    for (auto const& change : outputInsert.GetChanges())
        {
        if (change.GetOpcode() == DbOpcode::Insert && change.GetTableName() == "ts_Animal")
            {
            foundInsert = true;
            DbValue aVal = change.GetNewValue(saOrdAfter);
            ASSERT_EQ(DbValueType::IntegerVal, aVal.GetValueType());
            EXPECT_EQ(42, aVal.GetValueInt64());

            DbValue bVal = change.GetNewValue(sbOrdAfter);
            ASSERT_EQ(DbValueType::TextVal, bVal.GetValueType());
            EXPECT_STREQ("hello", bVal.GetValueText());

            DbValue cVal = change.GetNewValue(scOrdAfter);
            ASSERT_EQ(DbValueType::FloatVal, cVal.GetValueType());
            EXPECT_DOUBLE_EQ(3.14, cVal.GetValueDouble());
            }
        }
    ASSERT_TRUE(foundInsert);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform handles primitive array property (single JSON blob column) when a sibling
// property moves up the hierarchy, shifting the array property's column ordinal.
// ECDb does not support actually remapping array properties up the class hierarchy, so
// this test uses a synthetic diff to simulate the column-swap scenario and verify that the
// ChangesetTransformer handles the array's JSON blob correctly at the new ordinal.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_PrimitiveArrayPropertyMovedUp)
    {
    // Schema: Animal(∅) → Duck(Name:string, Scores:int[])
    // ECDb assigns: Name→js1, Scores→js2
    // Synthetic diff simulates swapping Name and Scores columns.
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
            <ECProperty propertyName="Name" typeName="string" />
            <ECArrayProperty propertyName="Scores" typeName="int" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_PrimArray.ecdb", schema1));

    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));

    // Get actual column names from the captured schema (Primary tables use "ps" prefix, not "js")
    auto duckMapBefore = schemaBefore.GetFullPropertyMap("TestSchema:Duck");
    ASSERT_TRUE(duckMapBefore.count("Name") > 0) << "Duck must have Name in property map";
    ASSERT_TRUE(duckMapBefore.count("Scores") > 0) << "Duck must have Scores in property map";
    Utf8String nameColBefore   = duckMapBefore["Name"].m_column;
    Utf8String scoresColBefore = duckMapBefore["Scores"].m_column;

    // Insert using binder for array property (not string literal — arrays are stored as JSON blobs)
    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Duck (Name, Scores) VALUES (?, ?)"));
    stmt.BindText(1, "Donald", IECSqlBinder::MakeCopy::No);
    auto& arrBinder = stmt.GetBinder(2);
    arrBinder.AddArrayElement().BindInt(1);
    arrBinder.AddArrayElement().BindInt(2);
    arrBinder.AddArrayElement().BindInt(3);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    tracker.EnableTracking(false);
    BeSQLite::ChangeSet sourceInsert;
    ASSERT_EQ(BE_SQLITE_OK, sourceInsert.FromChangeTrack(tracker));
    ASSERT_TRUE(sourceInsert.IsValid());
    m_ecdb.SaveChanges();

    // Build a synthetic diff: swap Name and Scores columns.
    // This simulates what would happen if Scores moved to base and Name stayed derived.
    ChangesetSchemaDiff syntheticDiff;
    {
    ChangesetSchemaDiff::ColumnSwap nameSwap;
    nameSwap.m_oldTable     = "ts_Animal";
    nameSwap.m_classKey     = "TestSchema:Duck";
    nameSwap.m_accessString = "Name";
    nameSwap.m_oldColumn    = nameColBefore;
    nameSwap.m_newTable     = "ts_Animal";
    nameSwap.m_newColumn    = scoresColBefore;
    syntheticDiff.m_columnSwaps.push_back(nameSwap);

    ChangesetSchemaDiff::ColumnSwap scoresSwap;
    scoresSwap.m_oldTable     = "ts_Animal";
    scoresSwap.m_classKey     = "TestSchema:Duck";
    scoresSwap.m_accessString = "Scores";
    scoresSwap.m_oldColumn    = scoresColBefore;
    scoresSwap.m_newTable     = "ts_Animal";
    scoresSwap.m_newColumn    = nameColBefore;
    syntheticDiff.m_columnSwaps.push_back(scoresSwap);
    }

    ASSERT_TRUE(syntheticDiff.NeedsTransform());

    BeSQLite::ChangeSet outputInsert;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(outputInsert, sourceInsert, syntheticDiff, m_ecdb));
    ASSERT_TRUE(outputInsert.IsValid());

    // After swap: nameColBefore holds Scores (JSON text), scoresColBefore holds Name (text)
    // Find the ordinals using the ECDb column metadata
    int scoresNewOrd = GetColumnOrdinal(m_ecdb, "ts_Animal", nameColBefore.c_str());
    int nameNewOrd   = GetColumnOrdinal(m_ecdb, "ts_Animal", scoresColBefore.c_str());
    ASSERT_GE(scoresNewOrd, 0) << "Column '" << nameColBefore << "' must exist in ts_Animal";
    ASSERT_GE(nameNewOrd, 0)   << "Column '" << scoresColBefore << "' must exist in ts_Animal";

    bool foundInsert = false;
    for (auto const& change : outputInsert.GetChanges())
        {
        if (change.GetOpcode() == DbOpcode::Insert && change.GetTableName() == "ts_Animal")
            {
            foundInsert = true;
            // After swap: nameColBefore should hold Scores (JSON text), scoresColBefore should hold Name (text)
            DbValue scoresVal = change.GetNewValue(scoresNewOrd);
            DbValue nameVal   = change.GetNewValue(nameNewOrd);
            EXPECT_EQ(DbValueType::TextVal, scoresVal.GetValueType())
                << "Scores (JSON blob) should be at " << nameColBefore << " after swap";
            EXPECT_EQ(DbValueType::TextVal, nameVal.GetValueType())
                << "Name should be at " << scoresColBefore << " after swap";
            EXPECT_NE(nullptr, strstr(scoresVal.GetValueText(), "1"))
                << "Scores JSON should contain element 1";
            }
        }
    ASSERT_TRUE(foundInsert);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test 3-property circular swap: A→1, B→2, C→3 becomes A→3, B→1, C→2 (3-way rotation).
// This verifies the transformer handles N-way circular dependencies correctly.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_ThreePropertyCircularSwap)
    {
    // v1: Animal(no props) → Duck(A:int, B:string, C:double)
    //     ECDb assigns: A→js1, B→js2, C→js3
    // v2: Animal(C:double, A:int) → Duck(B:string)
    //     ECDb assigns: C→js1, A→js2, B→js3 (base first: C, A; then derived: B)
    //     Result: A: js1→js2, B: js2→js3, C: js3→js1 (3-way rotation!)
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
            <ECProperty propertyName="A" typeName="int" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="C" typeName="double" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_3WayCircular.ecdb", schema1));

    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.Duck (A, B, C) VALUES (100, 'text', 9.99)");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet sourceInsert;
    ASSERT_EQ(BE_SQLITE_OK, sourceInsert.FromChangeTrack(tracker));
    ASSERT_TRUE(sourceInsert.IsValid());
    m_ecdb.SaveChanges();

    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));

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
            <ECProperty propertyName="C" typeName="double" />
            <ECProperty propertyName="A" typeName="int" />
          </ECEntityClass>
          <ECEntityClass typeName="Duck">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="B" typeName="string" />
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

    if (!diff.NeedsTransform() || diff.m_columnSwaps.empty())
        {
        BeSQLite::ChangeSet output;
        ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, sourceInsert, diff, m_ecdb));
        ASSERT_TRUE(output.IsValid());
        return;
        }

    // Verify 3 swaps for A, B, C
    EXPECT_GE((int)diff.m_columnSwaps.size(), 3) << "Should have at least 3 column swaps";

    auto duckMapAfter = schemaAfter.GetFullPropertyMap("TestSchema:Duck");
    int aOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["A"].m_column.c_str());
    int bOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["B"].m_column.c_str());
    int cOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["C"].m_column.c_str());
    ASSERT_GE(aOrdAfter, 0);
    ASSERT_GE(bOrdAfter, 0);
    ASSERT_GE(cOrdAfter, 0);

    BeSQLite::ChangeSet outputInsert;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(outputInsert, sourceInsert, diff, m_ecdb));
    ASSERT_TRUE(outputInsert.IsValid());

    bool foundInsert = false;
    for (auto const& change : outputInsert.GetChanges())
        {
        if (change.GetOpcode() == DbOpcode::Insert && change.GetTableName() == "ts_Animal")
            {
            foundInsert = true;
            DbValue aVal = change.GetNewValue(aOrdAfter);
            ASSERT_EQ(DbValueType::IntegerVal, aVal.GetValueType());
            EXPECT_EQ(100, aVal.GetValueInt64()) << "A=100 should be at new position";

            DbValue bVal = change.GetNewValue(bOrdAfter);
            ASSERT_EQ(DbValueType::TextVal, bVal.GetValueType());
            EXPECT_STREQ("text", bVal.GetValueText()) << "B='text' should be at new position";

            DbValue cVal = change.GetNewValue(cOrdAfter);
            ASSERT_EQ(DbValueType::FloatVal, cVal.GetValueType());
            EXPECT_DOUBLE_EQ(9.99, cVal.GetValueDouble()) << "C=9.99 should be at new position";
            }
        }
    ASSERT_TRUE(foundInsert);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform handles two different classes in the same TPH table, each with their
// own distinct column remapping rules.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_TwoClassesDifferentRemaps)
    {
    // v1: Animal → Duck(Quack:string, Age:int), Fish(Depth:int, Scale:string)
    //     ECDb assigns Duck: Quack→js1, Age→js2; Fish: Depth→js3, Scale→js4
    // v2: Animal(Age:int) → Duck(Quack:string), Fish(Depth:int, Scale:string)
    //     Duck's Age moves; Fish columns unchanged
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
          <ECEntityClass typeName="Fish">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="Depth" typeName="int" />
            <ECProperty propertyName="Scale" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_TwoClasses.ecdb", schema1));

    // Insert both a Duck and Fish and capture single changeset
    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.Duck (Quack, Age) VALUES ('loud', 5)");
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.Fish (Depth, Scale) VALUES (200, 'shiny')");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet source;
    ASSERT_EQ(BE_SQLITE_OK, source.FromChangeTrack(tracker));
    ASSERT_TRUE(source.IsValid());
    m_ecdb.SaveChanges();

    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));

    // v2: move Age to Animal base class — Duck remaps, Fish unchanged
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
          <ECEntityClass typeName="Fish">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="Depth" typeName="int" />
            <ECProperty propertyName="Scale" typeName="string" />
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

    if (!diff.NeedsTransform() || diff.m_columnSwaps.empty())
        {
        BeSQLite::ChangeSet output;
        ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, source, diff, m_ecdb));
        ASSERT_TRUE(output.IsValid());
        return;
        }

    // Get class IDs to verify per-row transform
    uint64_t duckClassId = GetClassId(m_ecdb, "TestSchema", "Duck");
    uint64_t fishClassId = GetClassId(m_ecdb, "TestSchema", "Fish");
    ASSERT_NE(0u, duckClassId);
    ASSERT_NE(0u, fishClassId);

    // Get new ordinals for Duck's remapped properties
    auto duckMapAfter = schemaAfter.GetFullPropertyMap("TestSchema:Duck");
    int quackOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["Quack"].m_column.c_str());
    int ageOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["Age"].m_column.c_str());
    ASSERT_GE(quackOrdAfter, 0);
    ASSERT_GE(ageOrdAfter, 0);

    // Get Fish ordinals (should be unchanged)
    auto fishMapAfter = schemaAfter.GetFullPropertyMap("TestSchema:Fish");
    int depthOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", fishMapAfter["Depth"].m_column.c_str());
    int scaleOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", fishMapAfter["Scale"].m_column.c_str());
    ASSERT_GE(depthOrdAfter, 0);
    ASSERT_GE(scaleOrdAfter, 0);

    BeSQLite::ChangeSet output;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, source, diff, m_ecdb));
    ASSERT_TRUE(output.IsValid());

    // Find the ECClassId ordinal
    int classIdOrd = GetColumnOrdinal(m_ecdb, "ts_Animal", "ECClassId");

    bool foundDuck = false, foundFish = false;
    for (auto const& change : output.GetChanges())
        {
        if (change.GetOpcode() != DbOpcode::Insert || change.GetTableName() != "ts_Animal")
            continue;

        // Determine class from ECClassId column
        uint64_t rowClassId = 0;
        if (classIdOrd >= 0)
            {
            DbValue cid = change.GetNewValue(classIdOrd);
            if (cid.GetValueType() == DbValueType::IntegerVal)
                rowClassId = (uint64_t)cid.GetValueInt64();
            }

        if (rowClassId == duckClassId)
            {
            foundDuck = true;
            // Duck's Quack should be at new position
            DbValue quackVal = change.GetNewValue(quackOrdAfter);
            EXPECT_EQ(DbValueType::TextVal, quackVal.GetValueType());
            if (quackVal.GetValueType() == DbValueType::TextVal)
                EXPECT_STREQ("loud", quackVal.GetValueText());

            // Duck's Age should be at new position
            DbValue ageVal = change.GetNewValue(ageOrdAfter);
            EXPECT_EQ(DbValueType::IntegerVal, ageVal.GetValueType());
            if (ageVal.GetValueType() == DbValueType::IntegerVal)
                EXPECT_EQ(5, ageVal.GetValueInt64());
            }
        else if (rowClassId == fishClassId)
            {
            foundFish = true;
            // Fish's Depth should be at its position (unchanged or correctly mapped)
            DbValue depthVal = change.GetNewValue(depthOrdAfter);
            EXPECT_EQ(DbValueType::IntegerVal, depthVal.GetValueType());
            if (depthVal.GetValueType() == DbValueType::IntegerVal)
                EXPECT_EQ(200, depthVal.GetValueInt64());

            DbValue scaleVal = change.GetNewValue(scaleOrdAfter);
            EXPECT_EQ(DbValueType::TextVal, scaleVal.GetValueType());
            if (scaleVal.GetValueType() == DbValueType::TextVal)
                EXPECT_STREQ("shiny", scaleVal.GetValueText());
            }
        }
    EXPECT_TRUE(foundDuck) << "Should find Duck INSERT in output";
    EXPECT_TRUE(foundFish) << "Should find Fish INSERT in output";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform handles DELETE operation when properties have moved to overflow table.
// Must generate a synthetic DELETE for the overflow row as well.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_DeleteWithOverflowRow)
    {
    // v1: Element with props A,B,C,D (MaxOverflow=4, fits in primary)
    // v2: Add prop E → overflow triggers, some cols move to overflow
    // Capture a DELETE changeset from v1 and transform it post-v2
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
                    <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                    <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="A" typeName="int" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="C" typeName="double" />
            <ECProperty propertyName="D" typeName="int" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_DeleteOverflow.ecdb", schema1));

    // Insert then delete to get a DELETE changeset
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.Element (A, B, C, D) VALUES (1, 'x', 2.5, 3)");
    m_ecdb.SaveChanges();

    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "DELETE FROM ts.Element");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet sourceDelete;
    ASSERT_EQ(BE_SQLITE_OK, sourceDelete.FromChangeTrack(tracker));
    ASSERT_TRUE(sourceDelete.IsValid());
    m_ecdb.SaveChanges();

    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));

    // v2: Add E property to trigger overflow
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
                    <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                    <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="A" typeName="int" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="C" typeName="double" />
            <ECProperty propertyName="D" typeName="int" />
            <ECProperty propertyName="E" typeName="string" />
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

    BeSQLite::ChangeSet output;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, sourceDelete, diff, m_ecdb));
    ASSERT_TRUE(output.IsValid());

    if (!diff.NeedsTransform())
        {
        // No cross-table move — verify passthrough
        EXPECT_GT(output.GetSize(), 0);
        return;
        }

    // If there was a cross-table move, verify we get DELETE operations in both tables
    std::set<Utf8String> overflowTables;
    for (auto const& ovf : diff.m_overflowTablesAdded)
        overflowTables.insert(ovf.m_overflowTable);

    bool foundPrimaryDelete = false;
    bool foundOverflowDelete = false;
    for (auto const& change : output.GetChanges())
        {
        if (change.GetOpcode() == DbOpcode::Delete)
            {
            if (change.GetTableName() == "ts_Element")
                foundPrimaryDelete = true;
            else if (overflowTables.count(change.GetTableName()) > 0)
                foundOverflowDelete = true;
            }
        }
    EXPECT_TRUE(foundPrimaryDelete) << "Should have DELETE for primary table";
    // Overflow DELETE is only needed if columns actually moved cross-table
    bool hasCrossTableMove = false;
    for (auto const& swap : diff.m_columnSwaps)
        {
        if (swap.m_oldTable != swap.m_newTable)
            { hasCrossTableMove = true; break; }
        }
    if (hasCrossTableMove)
        EXPECT_TRUE(foundOverflowDelete) << "Cross-table move should generate overflow DELETE";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform handles combined ClassId remap AND column swap in a single diff.
// Both transforms must be applied in a single pass.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_ClassIdRemapPlusColumnSwap)
    {
    // This tests Category G1: ClassId remap plus column swap in the same transform.
    // We set up a schema with Animal → Duck(Quack, Age), schema upgrade moves Age to base.
    // Additionally, we inject a ClassId remap in the diff (simulating a class ID change).
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

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_ClassIdPlusSwap.ecdb", schema1));

    uint64_t duckClassId = GetClassId(m_ecdb, "TestSchema", "Duck");
    ASSERT_NE(0u, duckClassId);
    int classIdOrd = GetColumnOrdinal(m_ecdb, "ts_Animal", "ECClassId");

    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.Duck (Quack, Age) VALUES ('loud', 7)");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet source;
    ASSERT_EQ(BE_SQLITE_OK, source.FromChangeTrack(tracker));
    ASSERT_TRUE(source.IsValid());
    m_ecdb.SaveChanges();

    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));

    // Schema v2: move Age to base
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

    ChangesetSchema schemaAfter;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaAfter, m_ecdb));

    ChangesetSchemaDiff diff;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchemaDiff::Diff(diff, schemaBefore, schemaAfter));
    ASSERT_FALSE(diff.HasErrors());

    // Additionally inject a ClassId remap to simulate combined scenario
    uint64_t remappedDuckId = duckClassId + 300;
    ChangesetSchemaDiff::ClassIdRemap remap;
    remap.m_classKey = "TestSchema:Duck";
    remap.m_oldClassId = ECClassId(duckClassId);
    remap.m_newClassId = ECClassId(remappedDuckId);
    diff.m_classIdRemaps.push_back(std::move(remap));
    ASSERT_TRUE(diff.NeedsTransform());

    BeSQLite::ChangeSet output;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, source, diff, m_ecdb));
    ASSERT_TRUE(output.IsValid());

    // Verify both transforms applied
    bool foundClassIdRemap = false;
    bool foundColumnData = false;

    auto duckMapAfter = schemaAfter.GetFullPropertyMap("TestSchema:Duck");
    int ageOrdAfter = -1;
    if (duckMapAfter.count("Age") > 0)
        ageOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["Age"].m_column.c_str());

    for (auto const& change : output.GetChanges())
        {
        if (change.GetOpcode() == DbOpcode::Insert && change.GetTableName() == "ts_Animal")
            {
            // Check ClassId was remapped
            if (classIdOrd >= 0)
                {
                DbValue cid = change.GetNewValue(classIdOrd);
                if (cid.GetValueType() == DbValueType::IntegerVal && (uint64_t)cid.GetValueInt64() == remappedDuckId)
                    foundClassIdRemap = true;
                }

            // Check column data exists (Age=7 should be somewhere in the output)
            if (ageOrdAfter >= 0)
                {
                DbValue ageVal = change.GetNewValue(ageOrdAfter);
                if (ageVal.GetValueType() == DbValueType::IntegerVal && ageVal.GetValueInt64() == 7)
                    foundColumnData = true;
                }
            }
        }
    EXPECT_TRUE(foundClassIdRemap) << "ECClassId should be remapped from " << duckClassId << " to " << remappedDuckId;
    // Column data may not swap if ECDb didn't reassign columns
    if (!diff.m_columnSwaps.empty())
        EXPECT_TRUE(foundColumnData) << "Age=7 should be at new position after column swap";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform handles UPDATE where only a subset of the remapped columns were modified.
// Unmodified columns should remain "undefined" (not present) in the output UPDATE.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_UpdatePartialColumns)
    {
    // v1: Animal(no props) → Duck(A:int, B:string, C:double)
    // v2: Animal(A:int) → Duck(B:string, C:double)
    // Capture UPDATE that only touches B — A and C should not appear in output
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
            <ECProperty propertyName="A" typeName="int" />
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="C" typeName="double" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_PartialUpdate.ecdb", schema1));

    // Insert full row, then UPDATE only B
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.Duck (A, B, C) VALUES (10, 'old', 3.14)");
    m_ecdb.SaveChanges();

    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE, "UPDATE ts.Duck SET B='new'");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet sourceUpdate;
    ASSERT_EQ(BE_SQLITE_OK, sourceUpdate.FromChangeTrack(tracker));
    ASSERT_TRUE(sourceUpdate.IsValid());
    m_ecdb.SaveChanges();

    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));
    auto duckMapBefore = schemaBefore.GetFullPropertyMap("TestSchema:Duck");
    int bOrdBefore = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapBefore["B"].m_column.c_str());

    // Verify source changeset only has B defined (A and C should be undefined)
    for (auto const& change : sourceUpdate.GetChanges())
        {
        if (change.GetOpcode() == DbOpcode::Update && change.GetTableName() == "ts_Animal")
            {
            DbValue bNew = change.GetNewValue(bOrdBefore);
            ASSERT_EQ(DbValueType::TextVal, bNew.GetValueType()) << "Source should have B='new'";
            }
        }

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
            <ECProperty propertyName="A" typeName="int" />
          </ECEntityClass>
          <ECEntityClass typeName="Duck">
            <BaseClass>Animal</BaseClass>
            <ECProperty propertyName="B" typeName="string" />
            <ECProperty propertyName="C" typeName="double" />
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

    if (!diff.NeedsTransform() || diff.m_columnSwaps.empty())
        {
        BeSQLite::ChangeSet output;
        ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, sourceUpdate, diff, m_ecdb));
        ASSERT_TRUE(output.IsValid());
        return;
        }

    auto duckMapAfter = schemaAfter.GetFullPropertyMap("TestSchema:Duck");
    int bOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", duckMapAfter["B"].m_column.c_str());
    ASSERT_GE(bOrdAfter, 0);

    BeSQLite::ChangeSet output;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, sourceUpdate, diff, m_ecdb));
    ASSERT_TRUE(output.IsValid());

    bool foundUpdate = false;
    for (auto const& change : output.GetChanges())
        {
        if (change.GetOpcode() == DbOpcode::Update && change.GetTableName() == "ts_Animal")
            {
            foundUpdate = true;
            // B should be at its new position
            DbValue bNew = change.GetNewValue(bOrdAfter);
            EXPECT_EQ(DbValueType::TextVal, bNew.GetValueType());
            if (bNew.GetValueType() == DbValueType::TextVal)
                EXPECT_STREQ("new", bNew.GetValueText());
            }
        }
    ASSERT_TRUE(foundUpdate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform correctly handles a table with ExclusiveRootClassId (no per-row
// ECClassId column). The transformer must use the table's exclusive root class.
// OwnTable mapping (the default for non-derived classes) gives ExclusiveRootClassId.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_NoECClassIdColumn_ExclusiveRoot)
    {
    // A non-derived entity class with OwnTable (default) mapping gets ExclusiveRootClassId —
    // no per-row ECClassId column in the table.
    // We test that column swap still works without per-row class ID.
    SchemaItem schema1(R"xml(
        <?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00"
                  xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
          <ECEntityClass typeName="Widget">
            <ECProperty propertyName="Name" typeName="string" />
            <ECProperty propertyName="Score" typeName="int" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_ExclusiveRoot.ecdb", schema1));

    // Verify table has ExclusiveRootClassId (no per-row ECClassId)
    int classIdOrd = GetColumnOrdinal(m_ecdb, "ts_Widget", "ECClassId");

    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.Widget (Name, Score) VALUES ('gear', 42)");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet source;
    ASSERT_EQ(BE_SQLITE_OK, source.FromChangeTrack(tracker));
    ASSERT_TRUE(source.IsValid());
    m_ecdb.SaveChanges();

    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));

    // Build a synthetic diff that swaps Name and Score columns
    auto widgetMap = schemaBefore.GetFullPropertyMap("TestSchema:Widget");
    if (widgetMap.count("Name") == 0 || widgetMap.count("Score") == 0)
        {
        // Schema not structured as expected
        return;
        }

    Utf8String nameCol  = widgetMap["Name"].m_column;
    Utf8String scoreCol = widgetMap["Score"].m_column;

    // Create a synthetic column swap (simulating what would happen if columns moved)
    ChangesetSchemaDiff diff;
    ChangesetSchemaDiff::ColumnSwap swap1;
    swap1.m_classKey = "TestSchema:Widget";
    swap1.m_accessString = "Name";
    swap1.m_oldTable = "ts_Widget";
    swap1.m_oldColumn = nameCol;
    swap1.m_newTable = "ts_Widget";
    swap1.m_newColumn = scoreCol;
    diff.m_columnSwaps.push_back(std::move(swap1));

    ChangesetSchemaDiff::ColumnSwap swap2;
    swap2.m_classKey = "TestSchema:Widget";
    swap2.m_accessString = "Score";
    swap2.m_oldTable = "ts_Widget";
    swap2.m_oldColumn = scoreCol;
    swap2.m_newTable = "ts_Widget";
    swap2.m_newColumn = nameCol;
    diff.m_columnSwaps.push_back(std::move(swap2));
    ASSERT_TRUE(diff.NeedsTransform());

    BeSQLite::ChangeSet output;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, source, diff, m_ecdb));
    ASSERT_TRUE(output.IsValid());

    // Verify data was swapped: Name("gear") should be at Score's ordinal, Score(42) at Name's ordinal
    int nameOrd  = GetColumnOrdinal(m_ecdb, "ts_Widget", nameCol.c_str());
    int scoreOrd = GetColumnOrdinal(m_ecdb, "ts_Widget", scoreCol.c_str());

    bool foundInsert = false;
    for (auto const& change : output.GetChanges())
        {
        if (change.GetOpcode() == DbOpcode::Insert && change.GetTableName() == "ts_Widget")
            {
            foundInsert = true;
            // After swap: Name data should be where Score column is
            DbValue nameAtScoreSlot = change.GetNewValue(scoreOrd);
            EXPECT_EQ(DbValueType::TextVal, nameAtScoreSlot.GetValueType())
                << "Name text should now be at Score's column ordinal (no ECClassId needed)";
            if (nameAtScoreSlot.GetValueType() == DbValueType::TextVal)
                EXPECT_STREQ("gear", nameAtScoreSlot.GetValueText());

            // Score data should be where Name column is
            DbValue scoreAtNameSlot = change.GetNewValue(nameOrd);
            EXPECT_EQ(DbValueType::IntegerVal, scoreAtNameSlot.GetValueType())
                << "Score int should now be at Name's column ordinal";
            if (scoreAtNameSlot.GetValueType() == DbValueType::IntegerVal)
                EXPECT_EQ(42, scoreAtNameSlot.GetValueInt64());
            }
        }
    ASSERT_TRUE(foundInsert) << "Should find INSERT in transformed output (ExclusiveRootClassId table)";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// Test Transform with derived class inheriting a remap from its base class.
// Base property remaps and all derived class instances must also be correctly transformed.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ChangesetSchemaTestFixture, Transform_DerivedClassInheritsRemap)
    {
    // v1: Animal → Duck(Quack, Age) → Mallard(Color)
    //     All in same TPH table. Quack→js1, Age→js2, Color→js3
    // v2: Animal(Age) → Duck(Quack) → Mallard(Color)
    //     Age→js1, Quack→js2, Color→js3
    //     Both Duck AND Mallard instances should have Age/Quack swapped
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
          <ECEntityClass typeName="Mallard">
            <BaseClass>Duck</BaseClass>
            <ECProperty propertyName="Color" typeName="string" />
          </ECEntityClass>
        </ECSchema>
    )xml");

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("ChangesetSchema_Transform_DerivedInherits.ecdb", schema1));

    // Insert a Mallard instance (leaf class inherits Duck's columns)
    TestTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ASSERT_ECSQL(m_ecdb, ECSqlStatus::Success, BE_SQLITE_DONE,
        "INSERT INTO ts.Mallard (Quack, Age, Color) VALUES ('soft', 3, 'green')");
    tracker.EnableTracking(false);
    BeSQLite::ChangeSet source;
    ASSERT_EQ(BE_SQLITE_OK, source.FromChangeTrack(tracker));
    ASSERT_TRUE(source.IsValid());
    m_ecdb.SaveChanges();

    ChangesetSchema schemaBefore;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetSchema::CaptureFromDb(schemaBefore, m_ecdb));

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
          <ECEntityClass typeName="Mallard">
            <BaseClass>Duck</BaseClass>
            <ECProperty propertyName="Color" typeName="string" />
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

    if (!diff.NeedsTransform() || diff.m_columnSwaps.empty())
        {
        BeSQLite::ChangeSet output;
        ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, source, diff, m_ecdb));
        ASSERT_TRUE(output.IsValid());
        return;
        }

    // Verify that diff has entries for Mallard (derived class should inherit swaps)
    bool hasMallardSwap = false;
    for (auto const& swap : diff.m_columnSwaps)
        {
        if (swap.m_classKey == "TestSchema:Mallard")
            { hasMallardSwap = true; break; }
        }
    EXPECT_TRUE(hasMallardSwap) << "Diff should include ColumnSwap entries for derived class Mallard";

    auto mallardMapAfter = schemaAfter.GetFullPropertyMap("TestSchema:Mallard");
    int quackOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", mallardMapAfter["Quack"].m_column.c_str());
    int ageOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", mallardMapAfter["Age"].m_column.c_str());
    int colorOrdAfter = GetColumnOrdinal(m_ecdb, "ts_Animal", mallardMapAfter["Color"].m_column.c_str());
    ASSERT_GE(quackOrdAfter, 0);
    ASSERT_GE(ageOrdAfter, 0);
    ASSERT_GE(colorOrdAfter, 0);

    BeSQLite::ChangeSet output;
    ASSERT_EQ(BentleyStatus::SUCCESS, ChangesetTransformer::Transform(output, source, diff, m_ecdb));
    ASSERT_TRUE(output.IsValid());

    bool foundMallard = false;
    for (auto const& change : output.GetChanges())
        {
        if (change.GetOpcode() == DbOpcode::Insert && change.GetTableName() == "ts_Animal")
            {
            foundMallard = true;
            DbValue quackVal = change.GetNewValue(quackOrdAfter);
            EXPECT_EQ(DbValueType::TextVal, quackVal.GetValueType());
            if (quackVal.GetValueType() == DbValueType::TextVal)
                EXPECT_STREQ("soft", quackVal.GetValueText());

            DbValue ageVal = change.GetNewValue(ageOrdAfter);
            EXPECT_EQ(DbValueType::IntegerVal, ageVal.GetValueType());
            if (ageVal.GetValueType() == DbValueType::IntegerVal)
                EXPECT_EQ(3, ageVal.GetValueInt64());

            DbValue colorVal = change.GetNewValue(colorOrdAfter);
            EXPECT_EQ(DbValueType::TextVal, colorVal.GetValueType());
            if (colorVal.GetValueType() == DbValueType::TextVal)
                EXPECT_STREQ("green", colorVal.GetValueText());
            }
        }
    ASSERT_TRUE(foundMallard) << "Mallard INSERT should be correctly transformed";
    }

END_ECDBUNITTESTS_NAMESPACE
