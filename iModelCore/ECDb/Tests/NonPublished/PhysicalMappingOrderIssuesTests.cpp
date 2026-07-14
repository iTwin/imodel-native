/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PhysicalMappingOrderIssuesTestFixture : public ECDbTestFixture
    {
    protected:
        struct Mapping
            {
            Utf8String m_tableName;
            Utf8String m_columnName;
            int64_t m_columnId = -1;
            int64_t m_propertyMapId = -1;
            int64_t m_propertyPathId = -1;

            bool IsValid() const { return m_columnId >= 0; }
            };

        //! Base class mapped TablePerHierarchy + ShareColumns with MaxSharedColumnsBeforeOverflow=4.
        //! The tight threshold ensures Sub2 (3 properties) determines the overflow boundary.
        Utf8CP GetCommonSchema() const
            {
            return R"xml(
    <ECSchema schemaName="Common" alias="cmn" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECEntityClass typeName="Element" modifier="Abstract">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="CommonProp" typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml";
            }

        //! Same Common schema but with MaxSharedColumnsBeforeOverflow=32 so all subclass
        //! properties fit in the primary table. Used by PhysicalLayoutIsImportOrderDependent
        //! to show that slot assignment diverges by import order when there is no overflow.
        Utf8CP GetCommonSchemaImportOrder() const
            {
            return R"xml(
    <ECSchema schemaName="Common" alias="cmn" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECEntityClass typeName="Element" modifier="Abstract">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="CommonProp" typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml";
            }

        //! Sub1: contributes 2 subclass properties (A1, A2) on ClassA.
        Utf8CP GetSub1Schema() const
            {
            return R"xml(
    <ECSchema schemaName="Sub1" alias="sub1" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="Common" version="01.00.00" alias="cmn"/>
        <ECEntityClass typeName="ClassA">
            <BaseClass>cmn:Element</BaseClass>
            <ECProperty propertyName="A1" typeName="string"/>
            <ECProperty propertyName="A2" typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml";
            }

        //! Sub2: contributes 3 subclass properties (B1, B2, B3) on ClassB — wider than Sub1,
        //! so ClassB determines the shared-column count and the overflow boundary.
        Utf8CP GetSub2Schema() const
            {
            return R"xml(
    <ECSchema schemaName="Sub2" alias="sub2" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="Common" version="01.00.00" alias="cmn"/>
        <ECEntityClass typeName="ClassB">
            <BaseClass>cmn:Element</BaseClass>
            <ECProperty propertyName="B1" typeName="string"/>
            <ECProperty propertyName="B2" typeName="string"/>
            <ECProperty propertyName="B3" typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml";
            }

        //! Sub3: contributes 6 subclass properties (C1..C6) on ClassC. Only imported by some
        //! files to demonstrate the schema-subset issue (Issue 5).
        Utf8CP GetSub3Schema() const
            {
            return R"xml(
    <ECSchema schemaName="Sub3" alias="sub3" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="Common" version="01.00.00" alias="cmn"/>
        <ECEntityClass typeName="ClassC">
            <BaseClass>cmn:Element</BaseClass>
            <ECProperty propertyName="C1" typeName="string"/>
            <ECProperty propertyName="C2" typeName="string"/>
            <ECProperty propertyName="C3" typeName="string"/>
            <ECProperty propertyName="C4" typeName="string"/>
            <ECProperty propertyName="C5" typeName="string"/>
            <ECProperty propertyName="C6" typeName="string"/>
        </ECEntityClass>
    </ECSchema>)xml";
            }

        // Reads the physical mapping (table, column, and the ec_* row ids) for a single-column
        // property. Works on the currently open m_ecdb.
        Mapping GetMapping(Utf8CP alias, Utf8CP className, Utf8CP propAccessString)
            {
            Mapping mapping;
            const ECClassId classId = m_ecdb.Schemas().GetClassId(alias, className, SchemaLookupMode::AutoDetect);
            EXPECT_TRUE(classId.IsValid()) << alias << ":" << className;
            if (!classId.IsValid())
                return mapping;

            CachedStatementPtr stmt = m_ecdb.GetCachedStatement(
                "SELECT t.Name, c.Name, c.Id, pm.Id, pm.PropertyPathId "
                "FROM ec_PropertyMap pm "
                "  INNER JOIN ec_PropertyPath pp ON pp.Id = pm.PropertyPathId "
                "  INNER JOIN ec_Column c ON c.Id = pm.ColumnId "
                "  INNER JOIN ec_Table t ON t.Id = c.TableId "
                "WHERE pm.ClassId = ?1 AND instr('.' || pp.AccessString || '.', '.' || ?2 || '.') = 1 "
                "ORDER BY t.Name, c.Id");
            EXPECT_TRUE(stmt.IsValid());
            if (!stmt.IsValid())
                return mapping;

            stmt->BindId(1, classId);
            stmt->BindText(2, propAccessString, Statement::MakeCopy::No);
            if (BE_SQLITE_ROW == stmt->Step())
                {
                mapping.m_tableName = stmt->GetValueText(0);
                mapping.m_columnName = stmt->GetValueText(1);
                mapping.m_columnId = stmt->GetValueInt64(2);
                mapping.m_propertyMapId = stmt->GetValueInt64(3);
                mapping.m_propertyPathId = stmt->GetValueInt64(4);
                }

            return mapping;
            }

        // Returns the physical columns of a table, in physical (ordinal) order.
        std::vector<Utf8String> GetPhysicalColumnOrder(Utf8CP tableName)
            {
            return GetHelper().GetColumnNames(tableName);
            }

        // Returns the physical column name that `alias:className.propName` is mapped to.
        Utf8String GetMappedColumnName(Utf8CP alias, Utf8CP className, Utf8CP propName)
            {
            return GetHelper().GetPropertyMapColumn(AccessString(alias, className, propName)).GetName();
            }
    };

//=======================================================================================
// Physical ROW IDs of the mapping metadata (ec_PropertyMap.Id and
// ec_PropertyPath.Id; the same reasoning applies to ec_Table / ec_Column / ec_Index).
//
// These rows are created in class-import order and drawn from per-kind monotonic
// counters, so the SAME logical mapping gets DIFFERENT row ids depending on order. This
// is a genuine, reliably reproducible physical divergence.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PhysicalMappingOrderIssuesTestFixture, PhysicalMappingRowIds_AreImportOrderDependent)
    {
    // File 1: Common -> Sub1 -> Sub2 (ClassA is mapped BEFORE ClassB)
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("physmap_ids_1.ecdb"));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetCommonSchema())));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetSub1Schema())));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetSub2Schema())));
    const Mapping order1_A1 = GetMapping("sub1", "ClassA", "A1");
    m_ecdb.CloseDb();

    // File 2: Common -> Sub2 -> Sub1 (ClassA is mapped AFTER ClassB)
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("physmap_ids_2.ecdb"));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetCommonSchema())));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetSub2Schema())));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetSub1Schema())));
    const Mapping order2_A1 = GetMapping("sub1", "ClassA", "A1");

    ASSERT_TRUE(order1_A1.IsValid());
    ASSERT_TRUE(order2_A1.IsValid());

    printf("PhysicalMappingRowIds: order1 pm=%lld pp=%lld col=%lld | order2 pm=%lld pp=%lld col=%lld\n",
           (long long) order1_A1.m_propertyMapId, (long long) order1_A1.m_propertyPathId, (long long) order1_A1.m_columnId,
           (long long) order2_A1.m_propertyMapId, (long long) order2_A1.m_propertyPathId, (long long) order2_A1.m_columnId);

    // Same schema, different import order => different physical row identity. This is the
    // physical analogue of the logical ec_Class.Id divergence that ID reservation fixes.
    // If these ever start matching, the physical row-id kinds have become order-independent
    // on their own and this issue can be closed without reserving them.
    EXPECT_NE(order1_A1.m_propertyMapId, order2_A1.m_propertyMapId)
        << "ec_PropertyMap.Id for ClassA.A1 was stable across import order";
    EXPECT_NE(order1_A1.m_propertyPathId, order2_A1.m_propertyPathId)
        << "ec_PropertyPath.Id for ClassA.A1 was stable across import order";
    }

//=======================================================================================
// The physical shape of a shared table is a function of ALL
// classes mapped into it, i.e. of WHICH schemas the file actually contains - not of any
// single schema in isolation. Two files that legitimately hold different subsets of the
// hierarchy will always have differently shaped tables.
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PhysicalMappingOrderIssuesTestFixture, TableWidthDependsOnSchemaSubset_NotSolvableByReservation)
    {
    // File 1: Common + Sub1 + Sub2 + Sub3 (full hierarchy; ClassC is 6 properties wide)
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("physmap_subset_full.ecdb"));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetCommonSchema())));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetSub1Schema())));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetSub2Schema())));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetSub3Schema())));
    const int fullColumnCount = GetHelper().GetColumnCount("cmn_Element");
    const int fullOverflowExists = GetHelper().TableExists("cmn_Element_Overflow") ? 1 : 0;
    m_ecdb.CloseDb();

    // File 2: Common + Sub1 only (a legitimate subset - never saw Sub2/Sub3)
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("physmap_subset_partial.ecdb"));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetCommonSchema())));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetSub1Schema())));
    const int partialColumnCount = GetHelper().GetColumnCount("cmn_Element");
    const int partialOverflowExists = GetHelper().TableExists("cmn_Element_Overflow") ? 1 : 0;

    printf("TableWidth: full cols=%d overflow=%d | partial cols=%d overflow=%d\n",
           fullColumnCount, fullOverflowExists, partialColumnCount, partialOverflowExists);

    // The shared table is genuinely a different shape - this is inherent, not a bug, and
    // cannot be reconciled by reservation. Reservation keeps ClassA.A1 in the same slot in
    // both files, but the surrounding table remains subset-dependent.
    EXPECT_NE(fullColumnCount, partialColumnCount)
        << "cmn_Element has the same physical width regardless of which subclasses the file contains";
    }

//---------------------------------------------------------------------------------------
// Proves that the physical mapping produced by a schema import (which shared column a
// property lands in) depends on the ORDER in which schemas are imported — it is NOT a
// pure function of the schema. This is the "structural changes are order-dependent"
// problem discussed in DeterministicSchemaRegistry.md (section A.8).
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(PhysicalMappingOrderIssuesTestFixture, PhysicalLayoutIsImportOrderDependent)
    {
    // --- File 1: import order Common -> Sub1 -> Sub2 -----------------------------------
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("schemaimportorder_1.ecdb"));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetCommonSchemaImportOrder())));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetSub1Schema())));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetSub2Schema())));

    const Utf8String order1_A1 = GetMappedColumnName("sub1", "ClassA", "A1");
    const Utf8String order1_A2 = GetMappedColumnName("sub1", "ClassA", "A2");
    const Utf8String order1_B1 = GetMappedColumnName("sub2", "ClassB", "B1");

    ASSERT_FALSE(order1_A1.empty()) << "Expected ClassA.A1 to be mapped to a shared column";

    m_ecdb.CloseDb();

    // --- File 2: import order Common -> Sub2 -> Sub1 (Sub1 and Sub2 swapped) -----------
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("schemaimportorder_2.ecdb"));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetCommonSchemaImportOrder())));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetSub2Schema())));
    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(GetSub1Schema())));

    const Utf8String order2_A1 = GetMappedColumnName("sub1", "ClassA", "A1");
    const Utf8String order2_A2 = GetMappedColumnName("sub1", "ClassA", "A2");
    const Utf8String order2_B1 = GetMappedColumnName("sub2", "ClassB", "B1");

    ASSERT_FALSE(order2_A1.empty()) << "Expected ClassA.A1 to be mapped to a shared column";

    // --- The proof --------------------------------------------------------------------
    // Same schema, different import order => different physical column assignment.
    // If this ever starts passing as STREQ, physical mapping may in fact be
    // order-independent for this case and section A.8 of the design doc should be
    // revisited before relying on it.
    EXPECT_STRNE(order1_A1.c_str(), order2_A1.c_str())
        << "ClassA.A1 landed in the same shared column regardless of import order - "
           "physical layout may not be order-dependent after all; revisit A.8.";

    // Belt-and-suspenders: ClassB's first property also shifts.
    EXPECT_STRNE(order1_B1.c_str(), order2_B1.c_str())
        << "ClassB.B1 landed in the same shared column regardless of import order.";

    // Document what actually happened for anyone reading the test output.
    printf("Import order 1 (Common,Sub1,Sub2): A1=%s A2=%s B1=%s\n", order1_A1.c_str(), order1_A2.c_str(), order1_B1.c_str());
    printf("Import order 2 (Common,Sub2,Sub1): A1=%s A2=%s B1=%s\n", order2_A1.c_str(), order2_A2.c_str(), order2_B1.c_str());
    }

END_ECDBUNITTESTS_NAMESPACE
