/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ECDb/Optimizer.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct OptimizerTestFixture : ECDbTestFixture
    {
    struct MappedColumn
        {
        uint64_t m_mapId = 0;
        uint64_t m_columnId = 0;
        Utf8String m_table;
        Utf8String m_column;
        };

    bvector<MappedColumn> GetMappedColumns(Utf8CP schemaName, Utf8CP className, Utf8CP propertyName)
        {
        ECClassCP ecClass = m_ecdb.Schemas().GetClass(schemaName, className);
        EXPECT_NE(nullptr, ecClass);
        bvector<MappedColumn> result;
        if (ecClass == nullptr)
            return result;

        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, R"sql(
            SELECT pm.Id,col.Id,t.Name,col.Name
            FROM ec_PropertyMap pm
            JOIN ec_PropertyPath pp ON pp.Id=pm.PropertyPathId
            JOIN ec_Property p ON p.Id=pp.RootPropertyId
            JOIN ec_Column col ON col.Id=pm.ColumnId
            JOIN ec_Table t ON t.Id=col.TableId
            WHERE pm.ClassId=? AND p.Name=? AND col.ColumnKind=4
            ORDER BY t.Id,col.Ordinal)sql"));
        stmt.BindId(1, ecClass->GetId());
        stmt.BindText(2, propertyName, Statement::MakeCopy::No);
        while (stmt.Step() == BE_SQLITE_ROW)
            {
            MappedColumn column;
            column.m_mapId = stmt.GetValueUInt64(0);
            column.m_columnId = stmt.GetValueUInt64(1);
            column.m_table = stmt.GetValueText(2);
            column.m_column = stmt.GetValueText(3);
            result.push_back(column);
            }
        return result;
        }

    void ExpectNoOptimizerTempTables()
        {
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb,
            "SELECT COUNT(*) FROM sqlite_temp_master WHERE name LIKE 'ecdbopt_%'"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_EQ(0, stmt.GetValueInt(0));
        }

    // Returns the physical name and ECClassId column of the single overflow table, or empty on failure.
    bool GetSingleOverflowTable(Utf8StringR tableName, Utf8StringR classIdColumn)
        {
        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, R"sql(
            SELECT t.Name, c.Name
            FROM ec_Table t
            JOIN ec_Column c ON c.TableId=t.Id AND c.ColumnKind=2 AND c.IsVirtual=0
            WHERE t.Type=3)sql"));
        if (stmt.Step() != BE_SQLITE_ROW)
            return false;
        tableName = stmt.GetValueText(0);
        classIdColumn = stmt.GetValueText(1);
        return stmt.Step() == BE_SQLITE_DONE; // exactly one overflow table
        }

    int CountRowsIn(Utf8StringCR tableName)
        {
        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT COUNT(*) FROM [%s]", tableName.c_str()).GetUtf8CP()));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        return stmt.GetValueInt(0);
        }
    };

TEST_F(OptimizerTestFixture, NoneDoesNotChangeCallerTransaction)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());
    int const transactionDepth = m_ecdb.GetCurrentSavepointDepth();

    Optimizer::Result result;
    EXPECT_EQ(BE_SQLITE_OK, Optimizer(m_ecdb).Optimize(Optimizer::Options::None, result));
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(transactionDepth, m_ecdb.GetCurrentSavepointDepth());

    BeJsDocument json;
    result.ToJson(json);
    EXPECT_FALSE(json["dryRun"].asBool());
    EXPECT_EQ(BE_SQLITE_OK, json["status"].asInt());
    EXPECT_STREQ("none", json["failedOption"].asCString());
    EXPECT_EQ(0, json["phases"].size());
    }

TEST_F(OptimizerTestFixture, AllOptionsExecuteWithoutOwningCallerTransaction)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());
    int const transactionDepth = m_ecdb.GetCurrentSavepointDepth();

    Optimizer::Result result;
    ASSERT_EQ(BE_SQLITE_OK, Optimizer(m_ecdb).Optimize(Optimizer::Options::All, result));
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(10, result.GetPhases().size());
    EXPECT_EQ(transactionDepth, m_ecdb.GetCurrentSavepointDepth());
    }

TEST_F(OptimizerTestFixture, DryRunAllWorksOnReadonlyECDb)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb(ECDb::OpenParams(ECDb::OpenMode::Readonly)));

    Optimizer::Result result;
    ASSERT_EQ(BE_SQLITE_OK, Optimizer(m_ecdb).DryRun(Optimizer::Options::All, result));
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_TRUE(result.IsDryRun());
    EXPECT_EQ(10, result.GetPhases().size());
    }

TEST_F(OptimizerTestFixture, PurgeInvalidClassIdsDryRunAndExecute)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Base" modifier="None">
                <ECProperty propertyName="Name" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Derived" modifier="Sealed">
                <BaseClass>Base</BaseClass>
            </ECEntityClass>
        </ECSchema>)xml")));

    ECSqlStatement insert;
    ECInstanceKey key;
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(m_ecdb, "INSERT INTO ts.Derived(Name) VALUES('invalid class id')"));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step(key));
    insert.Finalize();

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql(SqlPrintfString(
        "UPDATE ts_Base SET ECClassId=0x7fffffffffffffff WHERE Id=%s",
        key.GetInstanceId().ToHexStr().c_str())));

    int const transactionDepth = m_ecdb.GetCurrentSavepointDepth();
    Optimizer optimizer(m_ecdb);
    Optimizer::Result dryRun;
    ASSERT_EQ(BE_SQLITE_OK, optimizer.DryRun(Optimizer::Options::PurgeInvalidClassIds, dryRun));
    ASSERT_EQ(1, dryRun.GetPhases().size());
    EXPECT_EQ(1, dryRun.GetPhases().front().m_candidates);
    EXPECT_EQ(0, dryRun.GetPhases().front().m_changed);
    EXPECT_EQ(transactionDepth, m_ecdb.GetCurrentSavepointDepth());

    Statement count;
    ASSERT_EQ(BE_SQLITE_OK, count.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts_Base"));
    ASSERT_EQ(BE_SQLITE_ROW, count.Step());
    EXPECT_EQ(1, count.GetValueInt(0));
    count.Finalize();

    Optimizer::Result optimized;
    ASSERT_EQ(BE_SQLITE_OK, optimizer.Optimize(Optimizer::Options::PurgeInvalidClassIds, optimized));
    ASSERT_EQ(1, optimized.GetPhases().size());
    EXPECT_EQ(1, optimized.GetPhases().front().m_candidates);
    EXPECT_EQ(1, optimized.GetPhases().front().m_changed);
    EXPECT_EQ(transactionDepth, m_ecdb.GetCurrentSavepointDepth());

    ASSERT_EQ(BE_SQLITE_OK, count.Prepare(m_ecdb, "SELECT COUNT(*) FROM ts_Base"));
    ASSERT_EQ(BE_SQLITE_ROW, count.Step());
    EXPECT_EQ(0, count.GetValueInt(0));
    }

TEST_F(OptimizerTestFixture, DropUnusedSchemasPreservesSystemSchemas)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(
        <ECSchema schemaName="UnusedSchema" alias="unused" version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="UnusedClass" modifier="Sealed">
                <ECProperty propertyName="Value" typeName="int"/>
            </ECEntityClass>
        </ECSchema>)xml")));
    ASSERT_TRUE(m_ecdb.Schemas().ContainsSchema("UnusedSchema"));
    ASSERT_TRUE(m_ecdb.Schemas().ContainsSchema("ECDbMap"));

    Optimizer optimizer(m_ecdb);
    Optimizer::Result dryRun;
    ASSERT_EQ(BE_SQLITE_OK, optimizer.DryRun(Optimizer::Options::DropUnusedSchemas, dryRun));
    EXPECT_TRUE(m_ecdb.Schemas().ContainsSchema("UnusedSchema"));
    ASSERT_EQ(1, dryRun.GetPhases().size());
    EXPECT_GE(dryRun.GetPhases().front().m_candidates, 1);

    Optimizer::Result optimized;
    ASSERT_EQ(BE_SQLITE_OK, optimizer.Optimize(Optimizer::Options::DropUnusedSchemas, optimized));
    EXPECT_FALSE(m_ecdb.Schemas().ContainsSchema("UnusedSchema"));
    EXPECT_TRUE(m_ecdb.Schemas().ContainsSchema("ECDbMap"));
    }

TEST_F(OptimizerTestFixture, DropsOnlyEmptyDynamicClassesAndProperties)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(
        <ECSchema schemaName="DynamicTest" alias="dt" version="1.2.3"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="Used" modifier="Sealed">
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00.00"/>
                </ECCustomAttributes>
                <ECProperty propertyName="Populated" typeName="string"/>
                <ECProperty propertyName="Empty" typeName="point3d"/>
            </ECEntityClass>
            <ECEntityClass typeName="EmptyClass" modifier="Sealed">
                <ECProperty propertyName="Value" typeName="int"/>
            </ECEntityClass>
            <ECEntityClass typeName="EmptyBase" modifier="None"/>
            <ECEntityClass typeName="EmptyDerived" modifier="Sealed">
                <BaseClass>EmptyBase</BaseClass>
            </ECEntityClass>
        </ECSchema>)xml")));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO dt.Used(Populated) VALUES('kept')"));
    int const transactionDepth = m_ecdb.GetCurrentSavepointDepth();

    Optimizer::Result result;
    ASSERT_EQ(BE_SQLITE_OK, Optimizer(m_ecdb).Optimize(
        Optimizer::Options::DropEmptyDynamicClasses | Optimizer::Options::DropEmptyDynamicProperties, result));
    EXPECT_EQ(transactionDepth, m_ecdb.GetCurrentSavepointDepth());

    ASSERT_EQ(2, result.GetPhases().size());
    EXPECT_EQ(3, result.GetPhases()[0].m_changed);
    EXPECT_EQ(1, result.GetPhases()[1].m_changed);
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("DynamicTest", "EmptyClass"));
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("DynamicTest", "EmptyBase"));
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("DynamicTest", "EmptyDerived"));

    ECClassCP usedClass = m_ecdb.Schemas().GetClass("DynamicTest", "Used");
    ASSERT_NE(nullptr, usedClass);
    EXPECT_NE(nullptr, usedClass->GetPropertyP("Populated", false));
    EXPECT_EQ(nullptr, usedClass->GetPropertyP("Empty", false));

    Statement version;
    ASSERT_EQ(BE_SQLITE_OK, version.Prepare(m_ecdb,
        "SELECT VersionDigit1,VersionDigit2,VersionDigit3 FROM ec_Schema WHERE Name='DynamicTest'"));
    ASSERT_EQ(BE_SQLITE_ROW, version.Step());
    EXPECT_EQ(1, version.GetValueInt(0));
    EXPECT_EQ(2, version.GetValueInt(1));
    EXPECT_EQ(3, version.GetValueInt(2));
    }

TEST_F(OptimizerTestFixture, CompactSharedColumnsOrdersPopulatedBeforeNullOnlyAndHandlesSwap)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(
        <ECSchema schemaName="CompactTest" alias="ct" version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Item" modifier="Sealed">
                <ECCustomAttributes><ShareColumns xmlns="ECDbMap.02.00.00"/></ECCustomAttributes>
                <ECProperty propertyName="Empty" typeName="string"/>
                <ECProperty propertyName="Value" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml")));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO ct.Item(Value) VALUES('value')"));

    auto emptyBefore = GetMappedColumns("CompactTest", "Item", "Empty");
    auto valueBefore = GetMappedColumns("CompactTest", "Item", "Value");
    ASSERT_EQ(1, emptyBefore.size());
    ASSERT_EQ(1, valueBefore.size());
    ASSERT_LT(emptyBefore[0].m_columnId, valueBefore[0].m_columnId);

    Optimizer optimizer(m_ecdb);
    int const transactionDepth = m_ecdb.GetCurrentSavepointDepth();
    Optimizer::Result dryRun;
    ASSERT_EQ(BE_SQLITE_OK, optimizer.DryRun(Optimizer::Options::CompactSharedColumns, dryRun));
    ASSERT_EQ(1, dryRun.GetPhases().size());
    EXPECT_EQ(2, dryRun.GetPhases()[0].m_candidates);
    EXPECT_EQ(0, dryRun.GetPhases()[0].m_changed);
    EXPECT_EQ(emptyBefore[0].m_columnId, GetMappedColumns("CompactTest", "Item", "Empty")[0].m_columnId);
    EXPECT_EQ(transactionDepth, m_ecdb.GetCurrentSavepointDepth());
    ExpectNoOptimizerTempTables();

    Optimizer::Result result;
    ASSERT_EQ(BE_SQLITE_OK, optimizer.Optimize(Optimizer::Options::CompactSharedColumns, result));
    EXPECT_EQ(2, result.GetPhases()[0].m_changed);
    EXPECT_EQ(transactionDepth, m_ecdb.GetCurrentSavepointDepth());
    ExpectNoOptimizerTempTables();
    auto emptyAfter = GetMappedColumns("CompactTest", "Item", "Empty");
    auto valueAfter = GetMappedColumns("CompactTest", "Item", "Value");
    ASSERT_EQ(valueBefore[0].m_columnId, emptyAfter[0].m_columnId);
    ASSERT_EQ(emptyBefore[0].m_columnId, valueAfter[0].m_columnId);

    ECSqlStatement select;
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(m_ecdb, "SELECT Empty,Value FROM ct.Item"));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_TRUE(select.IsValueNull(0));
    EXPECT_STREQ("value", select.GetValueText(1));

    Optimizer::Result noOp;
    ASSERT_EQ(BE_SQLITE_OK, optimizer.Optimize(Optimizer::Options::CompactSharedColumns, noOp));
    EXPECT_EQ(0, noOp.GetPhases()[0].m_candidates);
    ExpectNoOptimizerTempTables();
    }

TEST_F(OptimizerTestFixture, CompactSharedColumnsMovesAcrossOverflowWithStagedCycle)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(
        <ECSchema schemaName="CompactOverflow" alias="co" version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Item" modifier="Sealed">
                <ECCustomAttributes>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="EmptyA" typeName="string"/>
                <ECProperty propertyName="EmptyB" typeName="string"/>
                <ECProperty propertyName="Value" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml")));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO co.Item(Value) VALUES('overflow value')"));

    auto emptyBefore = GetMappedColumns("CompactOverflow", "Item", "EmptyA");
    auto valueBefore = GetMappedColumns("CompactOverflow", "Item", "Value");
    ASSERT_EQ(1, emptyBefore.size());
    ASSERT_EQ(1, valueBefore.size());
    ASSERT_FALSE(emptyBefore[0].m_table.EqualsI(valueBefore[0].m_table));

    Optimizer::Result result;
    ASSERT_EQ(BE_SQLITE_OK, Optimizer(m_ecdb).Optimize(Optimizer::Options::CompactSharedColumns, result));
    EXPECT_EQ(3, result.GetPhases()[0].m_changed);

    auto valueAfter = GetMappedColumns("CompactOverflow", "Item", "Value");
    ASSERT_EQ(emptyBefore[0].m_columnId, valueAfter[0].m_columnId);
    ECSqlStatement select;
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(m_ecdb, "SELECT EmptyA,EmptyB,Value FROM co.Item"));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_TRUE(select.IsValueNull(0));
    EXPECT_TRUE(select.IsValueNull(1));
    EXPECT_STREQ("overflow value", select.GetValueText(2));
    }

TEST_F(OptimizerTestFixture, CompactSharedColumnsKeepsIndivisibleGroupsWhenCapacityIsInsufficient)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(
        <ECSchema schemaName="CompactGroups" alias="cg" version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECStructClass typeName="Pair"><ECProperty propertyName="A" typeName="double"/><ECProperty propertyName="B" typeName="double"/></ECStructClass>
            <ECEntityClass typeName="PointItem" modifier="Sealed">
                <ECCustomAttributes><ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow></ShareColumns></ECCustomAttributes>
                <ECProperty propertyName="Point" typeName="point2d"/>
                <ECProperty propertyName="Value" typeName="double"/>
            </ECEntityClass>
            <ECEntityClass typeName="StructItem" modifier="Sealed">
                <ECCustomAttributes><ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow></ShareColumns></ECCustomAttributes>
                <ECStructProperty propertyName="PairValue" typeName="Pair"/>
                <ECProperty propertyName="Value" typeName="double"/>
            </ECEntityClass>
        </ECSchema>)xml")));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO cg.PointItem(Value) VALUES(1.0)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO cg.StructItem(Value) VALUES(2.0)"));

    auto pointBefore = GetMappedColumns("CompactGroups", "PointItem", "Point");
    auto structBefore = GetMappedColumns("CompactGroups", "StructItem", "PairValue");
    ASSERT_EQ(2, pointBefore.size());
    ASSERT_EQ(2, structBefore.size());

    Optimizer::Result result;
    ASSERT_EQ(BE_SQLITE_OK, Optimizer(m_ecdb).Optimize(Optimizer::Options::CompactSharedColumns, result));
    EXPECT_GE(result.GetPhases()[0].m_skipped, 2);
    auto pointAfter = GetMappedColumns("CompactGroups", "PointItem", "Point");
    auto structAfter = GetMappedColumns("CompactGroups", "StructItem", "PairValue");
    EXPECT_EQ(pointBefore[0].m_columnId, pointAfter[0].m_columnId);
    EXPECT_EQ(pointBefore[1].m_columnId, pointAfter[1].m_columnId);
    EXPECT_EQ(structBefore[0].m_columnId, structAfter[0].m_columnId);
    EXPECT_EQ(structBefore[1].m_columnId, structAfter[1].m_columnId);
    }

TEST_F(OptimizerTestFixture, CompactSharedColumnsCompactsInheritedMappingsAndPreservesCrud)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(
        <ECSchema schemaName="CompactInheritance" alias="ci" version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Base" modifier="None">
                <ECCustomAttributes><ShareColumns xmlns="ECDbMap.02.00.00"/></ECCustomAttributes>
                <ECProperty propertyName="Empty" typeName="string"/>
                <ECProperty propertyName="InheritedValue" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Derived" modifier="Sealed">
                <BaseClass>Base</BaseClass>
            </ECEntityClass>
        </ECSchema>)xml")));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO ci.Derived(InheritedValue) VALUES('value')"));
    auto emptyBefore = GetMappedColumns("CompactInheritance", "Base", "Empty");
    auto valueBefore = GetMappedColumns("CompactInheritance", "Base", "InheritedValue");
    ASSERT_EQ(1, emptyBefore.size());
    ASSERT_EQ(1, valueBefore.size());

    Optimizer::Result result;
    ASSERT_EQ(BE_SQLITE_OK, Optimizer(m_ecdb).Optimize(Optimizer::Options::CompactSharedColumns, result));
    EXPECT_EQ(2, result.GetPhases()[0].m_changed);
    auto emptyAfter = GetMappedColumns("CompactInheritance", "Base", "Empty");
    auto valueAfter = GetMappedColumns("CompactInheritance", "Base", "InheritedValue");
    EXPECT_EQ(valueBefore[0].m_columnId, emptyAfter[0].m_columnId);
    EXPECT_EQ(emptyBefore[0].m_columnId, valueAfter[0].m_columnId);

    ECSqlStatement select;
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(m_ecdb, "SELECT Empty,InheritedValue FROM ci.Derived"));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_TRUE(select.IsValueNull(0));
    EXPECT_STREQ("value", select.GetValueText(1));
    select.Finalize();

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key,
        "INSERT INTO ci.Derived(Empty,InheritedValue) VALUES('new empty','new value')"));
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(m_ecdb,
        "SELECT Empty,InheritedValue FROM ci.Derived WHERE Empty='new empty'"));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_STREQ("new empty", select.GetValueText(0));
    EXPECT_STREQ("new value", select.GetValueText(1));
    }

//---------------------------------------------------------------------------------------
// All must never remove any ECDb-owned built-in schema (authoritative protected set).
//---------------------------------------------------------------------------------------
TEST_F(OptimizerTestFixture, AllPreservesEcdbBuiltInSchemas)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());

    for (Utf8CP schemaName : {"ECDbMeta", "ECDbFileInfo", "ECDbSystem", "ECDbMap", "ECDbSchemaPolicies", "ECDbChange"})
        ASSERT_TRUE(m_ecdb.Schemas().ContainsSchema(schemaName)) << schemaName;

    Optimizer::Result result;
    ASSERT_EQ(BE_SQLITE_OK, Optimizer(m_ecdb).Optimize(Optimizer::Options::All, result));
    EXPECT_TRUE(result.IsSuccess());

    for (Utf8CP schemaName : {"ECDbMeta", "ECDbFileInfo", "ECDbSystem", "ECDbMap", "ECDbSchemaPolicies", "ECDbChange"})
        EXPECT_TRUE(m_ecdb.Schemas().ContainsSchema(schemaName)) << schemaName;
    }

//---------------------------------------------------------------------------------------
// Overflow rows of the mapped class and of a derived class are valid and must be retained.
//---------------------------------------------------------------------------------------
TEST_F(OptimizerTestFixture, CompactSharedColumnsRetainsDirectAndDerivedOverflowRows)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(
        <ECSchema schemaName="OverflowValid" alias="ov" version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Base" modifier="None">
                <ECCustomAttributes><ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow></ShareColumns></ECCustomAttributes>
                <ECProperty propertyName="P1" typeName="string"/>
                <ECProperty propertyName="P2" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Derived" modifier="Sealed"><BaseClass>Base</BaseClass></ECEntityClass>
        </ECSchema>)xml")));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO ov.Base(P1,P2) VALUES('a','b')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO ov.Derived(P1,P2) VALUES('c','d')"));

    Utf8String overflowTable, classIdColumn;
    ASSERT_TRUE(GetSingleOverflowTable(overflowTable, classIdColumn));
    ASSERT_EQ(2, CountRowsIn(overflowTable));

    Optimizer::Result result;
    ASSERT_EQ(BE_SQLITE_OK, Optimizer(m_ecdb).Optimize(Optimizer::Options::CompactSharedColumns, result));
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_EQ(2, CountRowsIn(overflowTable)); // both the mapped class and the derived class remain valid
    }

//---------------------------------------------------------------------------------------
// An overflow row whose stored ECClassId is not valid for the overflow table is removed.
//---------------------------------------------------------------------------------------
TEST_F(OptimizerTestFixture, CompactSharedColumnsDeletesOverflowRowOfUnmappedClass)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(
        <ECSchema schemaName="OverflowOrphan" alias="oo" version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Item" modifier="None">
                <ECCustomAttributes><ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow></ShareColumns></ECCustomAttributes>
                <ECProperty propertyName="P1" typeName="string"/>
                <ECProperty propertyName="P2" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Unrelated" modifier="Sealed">
                <ECProperty propertyName="Value" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml")));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO oo.Item(P1,P2) VALUES('a','b')"));

    Utf8String overflowTable, classIdColumn;
    ASSERT_TRUE(GetSingleOverflowTable(overflowTable, classIdColumn));
    ASSERT_EQ(1, CountRowsIn(overflowTable));

    ECClassCP unrelated = m_ecdb.Schemas().GetClass("OverflowOrphan", "Unrelated");
    ASSERT_NE(nullptr, unrelated);
    // Corrupt the overflow row so its class id is a real class that is not valid for this overflow table.
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql(SqlPrintfString("UPDATE [%s] SET [%s]=%s",
        overflowTable.c_str(), classIdColumn.c_str(), unrelated->GetId().ToHexStr().c_str()).GetUtf8CP()));

    Optimizer optimizer(m_ecdb);
    Optimizer::Result dryRun;
    ASSERT_EQ(BE_SQLITE_OK, optimizer.DryRun(Optimizer::Options::CompactSharedColumns, dryRun));
    ASSERT_EQ(1, dryRun.GetPhases().size());
    EXPECT_GE(dryRun.GetPhases()[0].m_candidates, 1);
    EXPECT_EQ(0, dryRun.GetPhases()[0].m_changed);
    EXPECT_EQ(1, CountRowsIn(overflowTable)); // dry-run must not modify main

    Optimizer::Result result;
    ASSERT_EQ(BE_SQLITE_OK, optimizer.Optimize(Optimizer::Options::CompactSharedColumns, result));
    EXPECT_GE(result.GetPhases()[0].m_changed, 1);
    EXPECT_EQ(0, CountRowsIn(overflowTable)); // orphan overflow row removed
    ExpectNoOptimizerTempTables();
    }

//---------------------------------------------------------------------------------------
// Dry-run must use the planned property-map destinations when deciding whether overflow
// rows will still belong to a class after compaction.
//---------------------------------------------------------------------------------------
TEST_F(OptimizerTestFixture, CompactSharedColumnsProjectsOverflowCleanupInDryRun)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(
        <ECSchema schemaName="ProjectedOverflow" alias="po" version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECCustomAttributes><DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/></ECCustomAttributes>
            <ECEntityClass typeName="Item" modifier="Sealed">
                <ECCustomAttributes><ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow></ShareColumns></ECCustomAttributes>
                <ECProperty propertyName="Empty" typeName="string"/>
                <ECProperty propertyName="Value" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml")));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO po.Item(Value) VALUES('value')"));

    Utf8String overflowTable, classIdColumn;
    ASSERT_TRUE(GetSingleOverflowTable(overflowTable, classIdColumn));
    ASSERT_EQ(1, CountRowsIn(overflowTable));
    auto valueColumns = GetMappedColumns("ProjectedOverflow", "Item", "Value");
    ASSERT_EQ(1, valueColumns.size());
    ASSERT_STRCASEEQ(overflowTable.c_str(), valueColumns[0].m_table.c_str());

    // Leave the populated property as the overflow table's only mapping. Compaction can then move
    // it into the vacant primary slot, making the existing overflow row obsolete.
    Optimizer optimizer(m_ecdb);
    Optimizer::Result dropProperty;
    ASSERT_EQ(BE_SQLITE_OK, optimizer.Optimize(Optimizer::Options::DropEmptyDynamicProperties, dropProperty));
    ASSERT_EQ(nullptr, m_ecdb.Schemas().GetClass("ProjectedOverflow", "Item")->GetPropertyP("Empty", false));

    Optimizer::Result dryRun;
    ASSERT_EQ(BE_SQLITE_OK, optimizer.DryRun(Optimizer::Options::CompactSharedColumns, dryRun));
    ASSERT_EQ(1, dryRun.GetPhases().size());
    EXPECT_GE(dryRun.GetPhases()[0].m_candidates, 2); // one mapping group and its obsolete overflow row
    EXPECT_EQ(0, dryRun.GetPhases()[0].m_changed);
    EXPECT_EQ(1, CountRowsIn(overflowTable));
    ExpectNoOptimizerTempTables();

    Optimizer::Result optimized;
    ASSERT_EQ(BE_SQLITE_OK, optimizer.Optimize(Optimizer::Options::CompactSharedColumns, optimized));
    ASSERT_EQ(1, optimized.GetPhases().size());
    EXPECT_EQ(dryRun.GetPhases()[0].m_candidates, optimized.GetPhases()[0].m_candidates);
    EXPECT_EQ(dryRun.GetPhases()[0].m_candidates, optimized.GetPhases()[0].m_changed);
    EXPECT_EQ(0, CountRowsIn(overflowTable));
    valueColumns = GetMappedColumns("ProjectedOverflow", "Item", "Value");
    ASSERT_EQ(1, valueColumns.size());
    EXPECT_STRNE(overflowTable.c_str(), valueColumns[0].m_table.c_str());
    ExpectNoOptimizerTempTables();
    }

//---------------------------------------------------------------------------------------
// Dropping an emptied dynamic property leaves an unmapped shared column that
// DropUnmappedSharedColumns removes without requiring compaction; CRUD must still work.
//---------------------------------------------------------------------------------------
TEST_F(OptimizerTestFixture, DropUnmappedSharedColumnsRemovesUnmappedColumnAndPreservesCrud)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDbForCurrentTest());
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(R"xml(
        <ECSchema schemaName="UnmappedColumns" alias="uc" version="1.0.0"
            xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECCustomAttributes><DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/></ECCustomAttributes>
            <ECEntityClass typeName="Item" modifier="Sealed">
                <ECCustomAttributes><ShareColumns xmlns="ECDbMap.02.00.00"/></ECCustomAttributes>
                <ECProperty propertyName="Keep" typeName="string"/>
                <ECProperty propertyName="Remove" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml")));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO uc.Item(Keep) VALUES('kept')"));

    auto sharedColumnCount = [&]()
        {
        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb,
            "SELECT COUNT(*) FROM ec_Column WHERE ColumnKind=4 AND IsVirtual=0"));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        return stmt.GetValueInt(0);
        };
    int const before = sharedColumnCount();

    Optimizer optimizer(m_ecdb);
    Optimizer::Result dropProps;
    ASSERT_EQ(BE_SQLITE_OK, optimizer.Optimize(Optimizer::Options::DropEmptyDynamicProperties, dropProps));
    EXPECT_EQ(nullptr, m_ecdb.Schemas().GetClass("UnmappedColumns", "Item")->GetPropertyP("Remove", false));

    Optimizer::Result dropColumns;
    ASSERT_EQ(BE_SQLITE_OK, optimizer.Optimize(Optimizer::Options::DropUnmappedSharedColumns, dropColumns));
    EXPECT_TRUE(dropColumns.IsSuccess());
    EXPECT_GE(dropColumns.GetPhases()[0].m_changed, 1);
    EXPECT_LT(sharedColumnCount(), before);

    // CRUD on the optimized mapping must still work end-to-end.
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(key, "INSERT INTO uc.Item(Keep) VALUES('kept2')"));
    ECSqlStatement select;
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(m_ecdb, "SELECT Keep FROM uc.Item ORDER BY Keep"));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_STREQ("kept", select.GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_STREQ("kept2", select.GetValueText(0));
    }

END_ECDBUNITTESTS_NAMESPACE
