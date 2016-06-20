/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbJoinedTable_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "SchemaImportTestFixture.h"
#include "NestedStructArrayTestSchemaHelper.h"
#include <initializer_list>
#include <cmath>
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         10/15
//---------------+---------------+---------------+---------------+---------------+-------
struct JoinedTableECDbMapStrategyTests: ECDbMappingTestFixture
    {
    protected:
        void AssertTableLayouts(ECDbCR, bmap<Utf8String, Utf8String> const& tableLayouts, Utf8CP scenario) const;
        ECInstanceId InsertTestInstance (ECDbCR ecdb, Utf8CP ecsql);
        Utf8String ToInsertECSql (ECDbCR ecdb, Utf8CP className);
        Utf8String ToSelectECSql (ECDbCR ecdb, Utf8CP className);
        void VerifyInsertedInstance (ECDbR ecdb, Utf8CP ecsql, ECInstanceId sourceInstanceId, ECInstanceId targetInstanceId, ECClassId sourceClassId, ECClassId targetClassId);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                      11/15
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableECDbMapStrategyTests, TableLayout)
    {
    struct TestItem
        {
        SchemaItem m_testSchema;
        bmap<Utf8String, Utf8String> m_expectedTableLayout;

        explicit TestItem(SchemaItem const& testSchema) : m_testSchema(testSchema) {}
        //!@param[in] columnNames space-separated, alphabetically sorted list of lower-cased column names
        void AddTableLayout(Utf8CP tableName, Utf8CP columnNames) { m_expectedTableLayout[tableName] = columnNames; }
        };

    std::vector<TestItem> testItems;
    //JoinedTablePerDirectSubclass tests
    TestItem testItem(SchemaItem("JoinedTablePerDirectSubclass_on_c0",
                                 "<?xml version='1.0' encoding='utf-8'?>"
                                 "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='ts' version='1.0'"
                                 "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                 "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                                 "    <ECEntityClass typeName='C0'>"
                                 "        <ECCustomAttributes>"
                                 "            <ClassMap xmlns='ECDbMap.01.00'>"
                                 "                <MapStrategy>"
                                 "                    <Strategy>SharedTable</Strategy>"
                                 "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
                                 "                    <Options>JoinedTablePerDirectSubclass</Options>"
                                 "                </MapStrategy>"
                                 "            </ClassMap>"
                                 "        </ECCustomAttributes>"
                                 "        <ECProperty propertyName='A' typeName='long'/>"
                                 "        <ECProperty propertyName='B' typeName='string'/>"
                                 "    </ECEntityClass>"
                                 "   <ECEntityClass typeName='C1'>"
                                 "        <BaseClass>C0</BaseClass>"
                                 "        <ECProperty propertyName='C' typeName='long'/>"
                                 "        <ECProperty propertyName='D' typeName='string'/>"
                                 "    </ECEntityClass>"
                                 "   <ECEntityClass typeName='C2'>"
                                 "        <BaseClass>C0</BaseClass>"
                                 "        <ECProperty propertyName='E' typeName='long'/>"
                                 "        <ECProperty propertyName='F' typeName='string'/>"
                                 "    </ECEntityClass>"
                                 "   <ECEntityClass typeName='C11'>"
                                 "        <BaseClass>C1</BaseClass>"
                                 "        <ECProperty propertyName='G' typeName='long'/>"
                                 "        <ECProperty propertyName='H' typeName='string'/>"
                                 "    </ECEntityClass>"
                                 "   <ECEntityClass typeName='C21'>"
                                 "        <BaseClass>C2</BaseClass>"
                                 "        <ECProperty propertyName='I' typeName='long'/>"
                                 "        <ECProperty propertyName='J' typeName='string'/>"
                                 "    </ECEntityClass>"
                                 "</ECSchema>"));
    testItem.AddTableLayout("ts_C0", "a b");
    testItem.AddTableLayout("ts_C1", "c d g h");
    testItem.AddTableLayout("ts_C2", "e f i j");
    testItems.push_back(testItem);

    testItem = TestItem(SchemaItem("JoinedTablePerDirectSubclass_on_c1",
                                   "<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='ts' version='1.0'"
                                   "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='C0'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.01.00'>"
                                   "                <MapStrategy>"
                                   "                    <Strategy>SharedTable</Strategy>"
                                   "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
                                   "                </MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='C0_A' typeName='long'/>"
                                   "        <ECProperty propertyName='C0_B' typeName='string'/>"
                                   "    </ECEntityClass>"
                                   "   <ECEntityClass typeName='C1'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.01.00'>"
                                   "                <MapStrategy>"
                                   "                    <Options>JoinedTablePerDirectSubclass</Options>"
                                   "                </MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <BaseClass>C0</BaseClass>"
                                   "        <ECProperty propertyName='C1_A' typeName='long'/>"
                                   "        <ECProperty propertyName='C1_B' typeName='string'/>"
                                   "    </ECEntityClass>"
                                   "   <ECEntityClass typeName='C2'>"
                                   "        <BaseClass>C0</BaseClass>"
                                   "        <ECProperty propertyName='C2_A' typeName='long'/>"
                                   "        <ECProperty propertyName='C2_B' typeName='string'/>"
                                   "    </ECEntityClass>"
                                   "   <ECEntityClass typeName='C11'>"
                                   "        <BaseClass>C1</BaseClass>"
                                   "        <ECProperty propertyName='C11_A' typeName='long'/>"
                                   "        <ECProperty propertyName='C11_B' typeName='string'/>"
                                   "    </ECEntityClass>"
                                   "   <ECEntityClass typeName='C12'>"
                                   "        <BaseClass>C1</BaseClass>"
                                   "        <ECProperty propertyName='C12_A' typeName='long'/>"
                                   "        <ECProperty propertyName='C12_B' typeName='string'/>"
                                   "    </ECEntityClass>"
                                   "   <ECEntityClass typeName='C111'>"
                                   "        <BaseClass>C11</BaseClass>"
                                   "        <ECProperty propertyName='C111_A' typeName='long'/>"
                                   "        <ECProperty propertyName='C111_B' typeName='string'/>"
                                   "    </ECEntityClass>"
                                   "   <ECEntityClass typeName='C21'>"
                                   "        <BaseClass>C2</BaseClass>"
                                   "        <ECProperty propertyName='C21_A' typeName='long'/>"
                                   "        <ECProperty propertyName='C21_B' typeName='string'/>"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"));

    testItem.AddTableLayout("ts_C0", "c0_a c0_b c1_a c1_b c21_a c21_b c2_a c2_b");
    testItem.AddTableLayout("ts_C11", "c111_a c111_b c11_a c11_b");
    testItem.AddTableLayout("ts_C12", "c12_a c12_b");
    testItems.push_back(testItem);

    testItem = TestItem(SchemaItem("JoinedTablePerDirectSubclass_on_c1_and_c2",
                                   "<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='ts' version='1.0'"
                                   "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='C0'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.01.00'>"
                                   "                <MapStrategy>"
                                   "                    <Strategy>SharedTable</Strategy>"
                                   "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
                                   "                </MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='A' typeName='long'/>"
                                   "        <ECProperty propertyName='B' typeName='string'/>"
                                   "    </ECEntityClass>"
                                   "   <ECEntityClass typeName='C1'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.01.00'>"
                                   "                <MapStrategy>"
                                   "                    <Options>JoinedTablePerDirectSubclass</Options>"
                                   "                </MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <BaseClass>C0</BaseClass>"
                                   "        <ECProperty propertyName='C' typeName='long'/>"
                                   "        <ECProperty propertyName='D' typeName='string'/>"
                                   "    </ECEntityClass>"
                                   "   <ECEntityClass typeName='C2'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.01.00'>"
                                   "                <MapStrategy>"
                                   "                    <Options>JoinedTablePerDirectSubclass</Options>"
                                   "                </MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <BaseClass>C0</BaseClass>"
                                   "        <ECProperty propertyName='E' typeName='long'/>"
                                   "        <ECProperty propertyName='F' typeName='string'/>"
                                   "    </ECEntityClass>"
                                   "   <ECEntityClass typeName='C11'>"
                                   "        <BaseClass>C1</BaseClass>"
                                   "        <ECProperty propertyName='G' typeName='long'/>"
                                   "        <ECProperty propertyName='H' typeName='string'/>"
                                   "    </ECEntityClass>"
                                   "   <ECEntityClass typeName='C21'>"
                                   "        <BaseClass>C2</BaseClass>"
                                   "        <ECProperty propertyName='I' typeName='long'/>"
                                   "        <ECProperty propertyName='J' typeName='string'/>"
                                   "    </ECEntityClass>"
                                   "</ECSchema>"));

    testItem.AddTableLayout("ts_C0", "a b c d e f");
    testItem.AddTableLayout("ts_C11", "g h");
    testItem.AddTableLayout("ts_C21", "i j");
    testItems.push_back(testItem);

    for (TestItem const& testItem : testItems)
        {
        Utf8String ecdbName;
        ecdbName.Sprintf("joinedtablemapstrategy_%s.ecdb", testItem.m_testSchema.m_name.c_str());
        ECDbR ecdb = SetupECDb(ecdbName.c_str(), testItem.m_testSchema);
        ASSERT_TRUE(ecdb.IsDbOpen());

        AssertTableLayouts(ecdb, testItem.m_expectedTableLayout, testItem.m_testSchema.m_name.c_str());
        ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         10/05
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableECDbMapStrategyTests, BasicCRUD)
    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back (SchemaItem (
        "JoinedTablePerDirectSubclass on Root",
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' >"
        "        <BaseClass>Boo</BaseClass>"
        "        <ECProperty propertyName='G' typeName='long'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "    </ECEntityClass>"
        "</ECSchema>"));

    testSchemas.push_back (SchemaItem (
        "JoinedTablePerDirectSubclass on Root and SharedColumnsForSubclasses",
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>JoinedTablePerDirectSubclass, SharedColumnsForSubclasses</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' >"
        "        <BaseClass>Boo</BaseClass>"
        "        <ECProperty propertyName='G' typeName='long'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "    </ECEntityClass>"
        "</ECSchema>"));

    testSchemas.push_back (SchemaItem (
        "JoinedTablePerDirectSubclass on single direct subclass",
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' >"
        "        <BaseClass>Boo</BaseClass>"
        "        <ECProperty propertyName='G' typeName='long'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "    </ECEntityClass>"
        "</ECSchema>"));

    testSchemas.push_back (SchemaItem (
        "JoinedTablePerDirectSubclass on single direct subclass and SharedColumnsForSubclasses on Root",
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>SharedColumnsForSubclasses</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' >"
        "        <BaseClass>Boo</BaseClass>"
        "        <ECProperty propertyName='G' typeName='long'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "    </ECEntityClass>"
        "</ECSchema>"));

    testSchemas.push_back (SchemaItem (
        "JoinedTablePerDirectSubclass on both subclasses",
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' >"
        "        <BaseClass>Boo</BaseClass>"
        "        <ECProperty propertyName='G' typeName='long'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "    </ECEntityClass>"
        "</ECSchema>"));

    testSchemas.push_back (SchemaItem(
        "JoinedTablePerDirectSubclass on both subclasses and shared columns",
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>SharedColumnsForSubclasses</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' >"
        "        <BaseClass>Boo</BaseClass>"
        "        <ECProperty propertyName='G' typeName='long'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "    </ECEntityClass>"
        "</ECSchema>"));

    auto assertNonSelectECSql = [] (ECDbCR ecdb, Utf8CP testName, Utf8CP ecsql)
        {
        ECSqlStatement stmt;
        LOG.infov("Executing : %s", ecsql);
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql)) << testName << " ECSQL: " <<  ecsql;
        LOG.infov("NativeSQL : %s", stmt.GetNativeSql());
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << testName << " ECSQL: " << ecsql;
        };

    auto assertSelectECSql = [] (ECDbCR ecdb, Utf8CP testName, Utf8CP ecsql, int columnCountExpected, int rowCountExpected)
        {
        ECSqlStatement stmt;
        LOG.infov("Executing : %s", ecsql);
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql)) << testName << " ECSQL: " << ecsql;
        LOG.infov("NativeSQL : %s", stmt.GetNativeSql());
        ASSERT_EQ(columnCountExpected, stmt.GetColumnCount()) << testName << " ECSQL: " << ecsql;

        int actualRowCount = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
            actualRowCount++;

        ASSERT_EQ(rowCountExpected, actualRowCount) << testName << " ECSQL: " << ecsql;
        };

    std::vector<Utf8String> nonSelectECSqls {"UPDATE dgn.Goo SET A = ?, B = 'bb1', C = :c1, D = 'dd1' WHERE  A = ? AND B = :b1;",
                                    "UPDATE dgn.Goo SET A = ?, B = 'bb1' WHERE  A = ? AND B = :b1;",
                                    "UPDATE dgn.Goo SET C = :c1, D = 'dd1' WHERE  A = ? AND B = :b1;",
                                    "UPDATE dgn.Foo SET A = 2, B = 'bb1' WHERE  A = 101 AND B = 'b1';",
                                    "INSERT INTO dgn.Goo(A, B, C, D) VALUES(:a,'b1',:c,'d1');",
                                    "INSERT INTO dgn.Goo(ECInstanceId, A, B, C, D) VALUES(120, 102,'b2',202,'d2');",
                                    "INSERT INTO dgn.Goo(A, B, C, D) VALUES(103,'b3',203,'d3');",
                                    "INSERT INTO dgn.Goo(A, B) VALUES(104,'b4');",
                                    "INSERT INTO dgn.Foo(A, B) VALUES(105,'b5');",

                                    "INSERT INTO dgn.Boo(A, B, E, F) VALUES(:a,'b6',:c,'f1');",
                                    "INSERT INTO dgn.Boo(ECInstanceId, A, B, E, F) VALUES(130, 102,'b8',202,'f2');",
                                    "INSERT INTO dgn.Boo(A, B, E, F) VALUES(103,'b9',203,'f3');",
                                    "INSERT INTO dgn.Boo(A, B) VALUES(105,'b10');",
                                    "INSERT INTO dgn.Foo(A, B) VALUES(104,'b11');",

                                    "INSERT INTO dgn.Roo(A, B, G, H) VALUES(:a,'b12',:c,'h1');",
                                    "INSERT INTO dgn.Roo(ECInstanceId, A, B, G, H) VALUES(140, 102,'b13',202,'h2');",
                                    "INSERT INTO dgn.Roo(A, B, G, H) VALUES(103,'b14',203,'h3');",
                                    "INSERT INTO dgn.Roo(A, B) VALUES(105,'b15');",
                                    "INSERT INTO dgn.Foo(A, B) VALUES(104,'b16');",
                                    "INSERT INTO dgn.Foo(A, B) VALUES(104,'b17');",
                                    "DELETE FROM dgn.Foo WHERE ECInstanceId = 1 AND A = 101 AND B = 'b1';",
                                    "DELETE FROM dgn.Goo WHERE ECInstanceId = 1 AND A = 101 AND B = 'b1';",
                                    "DELETE FROM dgn.Goo WHERE ECInstanceId = 1 AND (A = 101 AND B = 'b1') AND (C = 101 AND D = 'd1');"};



    for (SchemaItem const& testSchema : testSchemas)
        {
        ECDbR ecdb = SetupECDb("JoinedTableTest.ecdb", testSchema);
        ASSERT_TRUE(ecdb.IsDbOpen());

        for (Utf8StringCR nonSelectECSql : nonSelectECSqls)
            {
            assertNonSelectECSql(ecdb, testSchema.m_name.c_str(), nonSelectECSql.c_str());
            }

        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B FROM dgn.Foo", 3, 16);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B FROM ONLY dgn.Foo", 3, 4);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B FROM dgn.Foo WHERE A = 102 AND B = 'b2'", 3, 1);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B FROM ONLY dgn.Foo WHERE A = 102 AND B = 'b2'", 3, 0);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B FROM ONLY dgn.Foo  WHERE A = 104 AND B = 'b17'", 3, 1);

        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B, C, D FROM dgn.Goo", 5, 4);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B FROM dgn.Goo", 3, 4);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, C, D FROM dgn.Goo", 3, 4);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B, C, D FROM ONLY dgn.Goo", 5, 4);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B, C, D FROM dgn.Goo WHERE A = 102 AND B ='b2' AND C = 202 AND D ='d2'", 5, 1);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B, C, D FROM ONLY dgn.Goo WHERE A = 102 AND B ='b2' AND C = 202 AND D ='d2'", 5, 1);

        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B, E, F FROM dgn.Boo", 5, 8);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B FROM dgn.Boo", 3, 8);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, E, F FROM dgn.Boo", 3, 8);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B, E, F FROM ONLY dgn.Boo", 5, 4);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B, E, F FROM dgn.Boo WHERE A = 102 AND B ='b8' AND E = 202 AND F ='f2'", 5, 1);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B, E, F FROM ONLY dgn.Boo WHERE A = 102 AND B ='b8' AND E = 202 AND F ='f2'", 5, 1);

        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B, G, H FROM dgn.Roo", 5, 4);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B FROM dgn.Roo", 3, 4);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, G, H FROM dgn.Roo", 3, 4);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B, G, H FROM ONLY dgn.Roo", 5, 4);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B, G, H FROM dgn.Roo WHERE A = 102 AND B ='b13' AND G = 202 AND H ='h2'", 5, 1);
        assertSelectECSql(ecdb, testSchema.m_name.c_str(), "SELECT ECInstanceId, A, B, G, H FROM ONLY dgn.Roo WHERE A = 102 AND B ='b13' AND G = 202 AND H ='h2'", 5, 1);

        ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECDbMapStrategyTests, AcrossMultipleSchemaImports)
    {
    SchemaItem baseTestSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='ReferredSchema' nameSpacePrefix='rs' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <Options>JoinedTablePerDirectSubclass</Options>"
        "                  <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' >"
        "         <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>", true, "Mapstrategy Option JoinedTablePerDirectSubclass (applied to subclasses) is expected to succeed");

    SchemaItem secondTestItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECSchemaReference name='ReferredSchema' version='01.00' prefix='rs' />"
        "    <ECEntityClass typeName='Sub2' >"
        "         <BaseClass>rs:Base</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' >"
        "         <BaseClass>rs:Sub1</BaseClass>"
        "        <ECProperty propertyName='P11' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>", true, "Mapstrategy Option JoinedTablePerDirectSubclass (applied to subclasses) is expected to be honored from base Class of Refered schema");

    Utf8String ecdbFilePath;
    {
    ECDbR ecdb = SetupECDb("JoinedTablePerDirectSubclass.ecdb", baseTestSchema);
    ASSERT_TRUE(ecdb.IsDbOpen());
    ecdb.SaveChanges();
    ecdbFilePath = ecdb.GetDbFileName();
    ecdb.CloseDb();
    }

    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbFilePath.c_str(), ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, secondTestItem);
    ASSERT_FALSE(asserted);
    ASSERT_TRUE(ecdb.IsDbOpen());

    bmap<Utf8String, Utf8String> expectedTableLayouts;
    expectedTableLayouts["rs_Base"] = "p0";
    expectedTableLayouts["rs_Sub1"] = "p1 p11";
    expectedTableLayouts["ts_Sub2"] = "p2";
    AssertTableLayouts(ecdb, expectedTableLayouts, "JoinedTablePerDirectSubclass in base schema imported in separate session");

    //verify that joined table option was resolved correctly. Need to look at the ec_ClassMap table directly to check that.
    std::map<ECClassId, PersistedMapStrategy> expectedResults {
            {ecdb.Schemas().GetECClassId("ReferredSchema","Base"), PersistedMapStrategy(PersistedMapStrategy::Strategy::SharedTable, PersistedMapStrategy::Options::ParentOfJoinedTable, true)},
            {ecdb.Schemas().GetECClassId("ReferredSchema","Sub1"), PersistedMapStrategy(PersistedMapStrategy::Strategy::SharedTable, PersistedMapStrategy::Options::JoinedTable, true)},
            {ecdb.Schemas().GetECClassId("TestSchema","Sub2"), PersistedMapStrategy(PersistedMapStrategy::Strategy::SharedTable, PersistedMapStrategy::Options::JoinedTable, true)},
            {ecdb.Schemas().GetECClassId("TestSchema","Sub11"), PersistedMapStrategy(PersistedMapStrategy::Strategy::SharedTable, PersistedMapStrategy::Options::JoinedTable, true)}
        };

    for (std::pair<ECClassId, PersistedMapStrategy> const& kvPair : expectedResults)
        {
        ECClassId classId = kvPair.first;
        PersistedMapStrategy expectedMapStrategy = kvPair.second;
        PersistedMapStrategy actualMapStrategy;

        ASSERT_TRUE(TryGetPersistedMapStrategy(actualMapStrategy, ecdb, classId));
        ASSERT_EQ(expectedMapStrategy.m_strategy, actualMapStrategy.m_strategy);
        ASSERT_EQ(expectedMapStrategy.m_options, actualMapStrategy.m_options);
        ASSERT_EQ(expectedMapStrategy.m_appliesToSubclasses, actualMapStrategy.m_appliesToSubclasses);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         10/15
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableECDbMapStrategyTests, InsertWithParameterBinding)
    {
    SchemaItem testSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' >"
        "        <BaseClass>Boo</BaseClass>"
        "        <ECProperty propertyName='G' typeName='long'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "    </ECEntityClass>"
        "</ECSchema>");

    ECDbR db = SetupECDb("JoinedTableTest.ecdb", testSchema);
    ASSERT_TRUE(db.IsDbOpen());

    ECSqlStatement stmt;
    //-----------------------------INSERT----------------------------------------------------
    ASSERT_EQ(stmt.Prepare(db, "INSERT INTO dgn.Goo (ECInstanceId, A, B, C, D ) VALUES ( :id, :a, :b, :c, :d)"), ECSqlStatus::Success);
    auto idIndex = stmt.GetParameterIndex("id");
    auto aIndex = stmt.GetParameterIndex("a");
    auto bIndex = stmt.GetParameterIndex("b");
    auto cIndex = stmt.GetParameterIndex("c");
    auto dIndex = stmt.GetParameterIndex("d");
    
    ASSERT_EQ(idIndex, 1);
    ASSERT_EQ(aIndex, 2);
    ASSERT_EQ(bIndex, 3);
    ASSERT_EQ(cIndex, 4);
    ASSERT_EQ(dIndex, 5);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(idIndex, 101));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(aIndex, 10000));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(bIndex, "a1000", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(cIndex, 20000));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(dIndex, "d2000", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);

    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(idIndex, 102));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(aIndex, 20000));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(bIndex, "a2000", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(cIndex, 30000));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(dIndex, "d4000", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);

    stmt.Finalize();

    //-----------------------------SELECT----------------------------------------------------
    ASSERT_EQ(stmt.Prepare(db, "SELECT A, B, C, D FROM dgn.Goo WHERE ECInstanceId = :id"), ECSqlStatus::Success);
    idIndex = stmt.GetParameterIndex("id");
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(idIndex, 101));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

    ASSERT_EQ(stmt.GetValueInt64(0), 10000);
    ASSERT_STRCASEEQ(stmt.GetValueText(1), "a1000");
    ASSERT_EQ(stmt.GetValueInt64(2), 20000);
    ASSERT_STRCASEEQ(stmt.GetValueText(3), "d2000");
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);

    stmt.Finalize();

    //-----------------------------UPDATE----------------------------------------------------

    ASSERT_EQ(stmt.Prepare(db, "UPDATE dgn.Goo SET A= :a, B= :b, C= :c, D= :d WHERE ECInstanceId = :id"), ECSqlStatus::Success);

    aIndex = stmt.GetParameterIndex("a");
    bIndex = stmt.GetParameterIndex("b");
    cIndex = stmt.GetParameterIndex("c");
    dIndex = stmt.GetParameterIndex("d");
    idIndex = stmt.GetParameterIndex("id");

    ASSERT_EQ(aIndex, 1);
    ASSERT_EQ(bIndex, 2);
    ASSERT_EQ(cIndex, 3);
    ASSERT_EQ(dIndex, 4);
    ASSERT_EQ(idIndex, 5);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(idIndex, 101));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(aIndex, 10001));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(bIndex, "a1001", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(cIndex, 20001));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(dIndex, "d2001", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();
    db.SaveChanges();
    ASSERT_EQ(stmt.Prepare(db, "SELECT A, B, C, D FROM dgn.Goo WHERE ECInstanceId = 101"), ECSqlStatus::Success);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

    ASSERT_EQ(stmt.GetValueInt64(0), 10001);
    ASSERT_STRCASEEQ(stmt.GetValueText(1), "a1001");
    ASSERT_EQ(stmt.GetValueInt64(2), 20001);
    ASSERT_STRCASEEQ(stmt.GetValueText(3), "d2001");
    stmt.Finalize();

    //-----------------------------DELETE----------------------------------------------------

    ASSERT_EQ(stmt.Prepare(db, "DELETE FROM dgn.Goo WHERE ECInstanceId = :id"), ECSqlStatus::Success);

    idIndex = stmt.GetParameterIndex("id");
    ASSERT_EQ(idIndex, 1);
    auto bindR = stmt.BindInt64(idIndex, 101);
    ASSERT_EQ(ECSqlStatus::Success, bindR);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(stmt.Prepare(db, "SELECT A, B, C, D FROM dgn.Goo WHERE ECInstanceId = 101"), ECSqlStatus::Success);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         10/15
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableECDbMapStrategyTests, InsertWithUnnamedParameterBinding)
    {
    SchemaItem testSchema("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' >"
        "        <BaseClass>Boo</BaseClass>"
        "        <ECProperty propertyName='G' typeName='long'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "    </ECEntityClass>"
        "</ECSchema>");

    ECDbR db = SetupECDb("JoinedTableTest.ecdb", testSchema);
    ASSERT_TRUE(db.IsDbOpen());

    ECSqlStatement stmt;
    //-----------------------------INSERT----------------------------------------------------
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO dgn.Goo (ECInstanceId, A, B, C, D ) VALUES ( ?, ?, ?, ?, ?)"));

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 101));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(2, 10000));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(3, "a1000", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(4, 20000));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(5, "d2000", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);

    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 102));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(2, 20000));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(3, "a2000", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(4, 30000));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(5, "d4000", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);

    stmt.Finalize();

    //-----------------------------SELECT----------------------------------------------------
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT A, B, C, D FROM dgn.Goo WHERE ECInstanceId = ?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 101));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

    ASSERT_EQ(stmt.GetValueInt64(0), 10000);
    ASSERT_STRCASEEQ(stmt.GetValueText(1), "a1000");
    ASSERT_EQ(stmt.GetValueInt64(2), 20000);
    ASSERT_STRCASEEQ(stmt.GetValueText(3), "d2000");
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);

    stmt.Finalize();

    //-----------------------------UPDATE----------------------------------------------------

    ASSERT_EQ(stmt.Prepare(db, "UPDATE dgn.Goo SET A= :a, B= :b, C= :c, D= :d WHERE ECInstanceId = ?"), ECSqlStatus::Success);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 10001));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "a1001", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(3, 20001));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(4, "d2001", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(5, 101));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    db.SaveChanges();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT A, B, C, D FROM dgn.Goo WHERE ECInstanceId = 101"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(10001, stmt.GetValueInt64(0));
    ASSERT_STRCASEEQ("a1001", stmt.GetValueText(1));
    ASSERT_EQ(20001, stmt.GetValueInt64(2));
    ASSERT_STRCASEEQ("d2001", stmt.GetValueText(3));
    stmt.Finalize();

    //-----------------------------DELETE----------------------------------------------------

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "DELETE FROM dgn.Goo WHERE ECInstanceId = ?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 101));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT A, B, C, D FROM dgn.Goo WHERE ECInstanceId = 101"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         10/15
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableECDbMapStrategyTests, AbstractBaseAndEmptyChildClass)
    {
    SchemaItem testSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>");

    ECDbR db = SetupECDb("JoinedTableTest.ecdb", testSchema);
    ASSERT_TRUE(db.IsDbOpen());

    auto assert_ecsql = [&db] (Utf8CP sql, ECSqlStatus expectedStatus, DbResult expectedStepStatus)
        {
        ECSqlStatement stmt;
        LOG.infov("Executing : %s", sql);
        ASSERT_EQ(stmt.Prepare(db, sql), expectedStatus);
        LOG.infov("NativeSQL : %s", stmt.GetNativeSql());
        if (expectedStatus == ECSqlStatus::Success)
            {
            ASSERT_EQ(stmt.Step(), expectedStepStatus);
            }
        };
    auto assert_ecsql2 = [&db] (Utf8CP sql, ECSqlStatus expectedStatus, int columnCountExpected, int rowCountExpected)
        {
        ECSqlStatement stmt;
        LOG.infov("Executing : %s", sql);
        ASSERT_EQ(stmt.Prepare(db, sql), expectedStatus);
        LOG.infov("NativeSQL : %s", stmt.GetNativeSql());
        ASSERT_EQ(stmt.GetColumnCount(), columnCountExpected);

        int realRowCount = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
            realRowCount++;

        ASSERT_EQ(realRowCount, rowCountExpected);
        };


    assert_ecsql("INSERT INTO dgn.Goo(ECInstanceId, A, B, C, D) VALUES(120, 102,'b2',202,'d2');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO dgn.Goo(A, B, C, D) VALUES(:a,'b1',:c,'d1');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO dgn.Goo(A, B, C, D) VALUES(103,'b3',203,'d3');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO dgn.Goo(A, B) VALUES(104,'b4');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO bs.Foo(A, B) VALUES(105,'b5');", ECSqlStatus::InvalidECSql, DbResult::BE_SQLITE_DONE);

    assert_ecsql2("SELECT ECInstanceId, A, B FROM dgn.Foo", ECSqlStatus::Success, 3, 4);
    assert_ecsql2("SELECT ECInstanceId, A, B FROM ONLY dgn.Foo", ECSqlStatus::Success, 3, 0);
    assert_ecsql2("SELECT ECInstanceId, A, B FROM dgn.Foo WHERE A = 102 AND B = 'b2'", ECSqlStatus::Success, 3, 1);
    assert_ecsql2("SELECT ECInstanceId, A, B FROM ONLY dgn.Foo WHERE A = 102 AND B = 'b2'", ECSqlStatus::Success, 3, 0);
    assert_ecsql2("SELECT ECInstanceId, A, B FROM ONLY dgn.Foo  WHERE A = 104 AND B = 'b17'", ECSqlStatus::Success, 3, 0);

    assert_ecsql("UPDATE dgn.Goo SET A = :c1, B = 'dd1' WHERE  A = ? AND B = :b1;", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("UPDATE dgn.Foo SET A = 2, B = 'bb1' WHERE  A = 101 AND B = 'b1';", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);


    assert_ecsql2("SELECT ECInstanceId, A, B, C, D FROM dgn.Goo", ECSqlStatus::Success, 5, 4);
    assert_ecsql2("SELECT ECInstanceId, A, B FROM dgn.Goo", ECSqlStatus::Success, 3, 4);
    assert_ecsql2("SELECT ECInstanceId, C, D FROM dgn.Goo", ECSqlStatus::Success, 3, 4);
    assert_ecsql2("SELECT ECInstanceId, A, B, C, D FROM ONLY dgn.Goo", ECSqlStatus::Success, 5, 4);
    assert_ecsql2("SELECT ECInstanceId, A, B, C, D FROM dgn.Goo WHERE A = 102 AND B ='b2' AND C = 202 AND D ='d2'", ECSqlStatus::Success, 5, 1);
    assert_ecsql2("SELECT ECInstanceId, A, B, C, D FROM ONLY dgn.Goo WHERE A = 102 AND B ='b2' AND C = 202 AND D ='d2'", ECSqlStatus::Success, 5, 1);

    assert_ecsql("DELETE FROM dgn.Foo WHERE ECInstanceId = 1 AND A = 101 AND B = 'b1';", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("DELETE FROM dgn.Goo WHERE ECInstanceId = 1 AND A = 101 AND B = 'b1';", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("DELETE FROM dgn.Goo WHERE ECInstanceId = 1 AND (A = 101 AND B = 'b1') AND (C = 101 AND D = 'd1');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                      11/15
//---------------+---------------+---------------+---------------+---------------+-------
void JoinedTableECDbMapStrategyTests::AssertTableLayouts(ECDbCR ecdb, bmap<Utf8String, Utf8String> const& tableLayouts, Utf8CP scenario) const
    {
    for (bpair<Utf8String, Utf8String> const& kvPair : tableLayouts)
        {
        Utf8CP tableName = kvPair.first.c_str();
        Utf8CP expectedColNames = kvPair.second.c_str();

        Utf8String sql;
        sql.Sprintf("SELECT * FROM %s LIMIT 0", tableName);

        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, sql.c_str())) << "Scenario: " << scenario << ". Expected table " << tableName << " does not exist. Error: " << ecdb.GetLastError().c_str();

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        const int actualColCount = stmt.GetColumnCount();
        bvector<Utf8String> actualColNameList;
        for (int i = 0; i < actualColCount; i++)
            {
            Utf8String actualColName (stmt.GetColumnName(i));
            actualColName.ToLower();
            if (actualColName.EndsWith("ecinstanceid") ||
                actualColName.Equals("ecclassid"))
                continue;

            actualColNameList.push_back(actualColName);
            }
        std::sort(actualColNameList.begin(), actualColNameList.end());
        Utf8String actualColNames;
        bool isFirstItem = true;
        for (Utf8StringCR name : actualColNameList)
            {
            if (!isFirstItem)
                actualColNames.append(" ");

            actualColNames.append(name);
            isFirstItem = false;
            }

        ASSERT_STREQ(expectedColNames, actualColNames.c_str()) << "Scenario: " << scenario << ". Unexpected layout of table " << tableName;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String JoinedTableECDbMapStrategyTests::ToInsertECSql (ECDbCR ecdb, Utf8CP className)
    {
    ECN::ECClassCP ecClass = ecdb.Schemas ().GetECClass ("JoinedTableTest", className);
    EXPECT_TRUE (ecClass != nullptr);

    Utf8String insertECSql = "INSERT INTO ";
    insertECSql.append (ecClass->GetECSqlName()).append (" (SourceECInstanceId, TargetECInstanceId, SourceECClassId, TargetECClassId) VALUES (%llu, %llu, %llu, %llu)");

    return insertECSql;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceId JoinedTableECDbMapStrategyTests::InsertTestInstance (ECDbCR ecdb, Utf8CP ecsql)
    {
    ECSqlStatement stmt;
    auto stat = stmt.Prepare (ecdb, ecsql);
    if (stat != ECSqlStatus::Success)
        {
        EXPECT_EQ (ECSqlStatus::Success, stat) << "Inserting test instance with '" << ecsql << "' failed. Preparation failed";
        return ECInstanceId ();
        }

    ECInstanceKey newECInstanceKey;
    DbResult stepStat = stmt.Step (newECInstanceKey);
    if (stepStat != BE_SQLITE_DONE)
        {
        EXPECT_EQ (BE_SQLITE_DONE, stepStat) << "Inserting test instance with '" << ecsql << "' failed. Step failed";
        return ECInstanceId ();
        }
    else
        return newECInstanceKey.GetECInstanceId ();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String JoinedTableECDbMapStrategyTests::ToSelectECSql (ECDbCR ecdb, Utf8CP className)
    {
    ECN::ECClassCP ecClass = ecdb.Schemas ().GetECClass ("JoinedTableTest", className);
    EXPECT_TRUE (ecClass != nullptr);

    Utf8String selecteECSql = "SELECT ";
    selecteECSql.append (className).append (".* FROM ").append (ecClass->GetECSqlName()).append (" WHERE ECInstanceId = %llu");

    return selecteECSql;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
void JoinedTableECDbMapStrategyTests::VerifyInsertedInstance (ECDbR ecdb, Utf8CP ecsql, ECInstanceId sourceInstanceId, ECInstanceId targetInstanceId, ECClassId sourceClassId, ECClassId targetClassId)
    {
    ECSqlStatement stmt;

    auto stat = stmt.Prepare (ecdb, ecsql);
    if (stat != ECSqlStatus::Success)
        {
        ASSERT_EQ (ECSqlStatus::Success, stat) << "Select test instance with '" << ecsql << "' failed. Preparation failed";
        }

    ASSERT_EQ (BE_SQLITE_ROW, stmt.Step ()) << "Select test instance with '" << ecsql << "' failed. Step failed";
    ASSERT_EQ (sourceInstanceId.GetValue (), stmt.GetValueInt64 (1)) << "Get Source InstanceId failed : " << ecsql;
    ASSERT_EQ (sourceClassId, stmt.GetValueId<ECClassId>(2)) << "Get SourceClassId failed : " << ecsql;
    ASSERT_EQ (targetInstanceId.GetValue (), stmt.GetValueInt64 (3)) << "Get TargetInstanceId failed : " << ecsql;
    ASSERT_EQ (targetClassId, stmt.GetValueId<ECClassId>(4)) << "Get TargetClassId failed : " << ecsql;
    }

//---------------------------------------------------------------------------------------
//           Foo  (JoinedTablePerDirectSubclass)  
//            |
//           Goo
//
//-------------SelfJoin relationships-----------------
//      Foo  <- FooHasFoo(REFERENCING) -> Foo
//      Foo  <- FooHasManyFoo(REFERENCING) -> Foo
//      Foo  <- ManyFooHasManyFoo(REFERENCING) -> Foo
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (JoinedTableECDbMapStrategyTests, SelfJoinRelationships)
    {
    SchemaItem testSchema (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='FooHasFoo' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='FooHasManyFoo' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Foo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='ManyFooHasManyFoo' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Foo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    ECDbR db = SetupECDb ("JoinedTableTest.ecdb", testSchema);
    ASSERT_TRUE (db.IsDbOpen ());

    ECClassId fooClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Foo");

    EC::ECInstanceId fooInstanceId1;
    EC::ECInstanceId fooInstanceId2;

    //Insert two Instances each Per class
    {
    fooInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Foo (A, B) VALUES(100001, 'Class Foo Instance 1')");
    fooInstanceId2 = InsertTestInstance (db, "INSERT INTO dgn.Foo (A, B) VALUES(100002, 'Class Foo Instance 2')");

    if (!fooInstanceId1.IsValid () || !fooInstanceId2.IsValid ())
        ASSERT_TRUE (false) << "Instance Id's not valid";
    }

    //Insert 1-1 Relationship
    {
    EC::ECInstanceId fooHasFooInstanceId1;
    Savepoint savePoint (db, "1-1 Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "FooHasFoo").c_str(), fooInstanceId1.GetValue (), fooInstanceId1.GetValue (), fooClassId.GetValue(), fooClassId.GetValue());
    fooHasFooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasFoo").c_str (), fooHasFooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, fooInstanceId1, fooClassId, fooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId fooHasManyFooInstanceId1;
    EC::ECInstanceId fooHasManyFooInstanceId2;
    Savepoint savePoint (db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyFoo").c_str (), fooInstanceId1.GetValue (), fooInstanceId1.GetValue (), fooClassId.GetValue(), fooClassId.GetValue());
    fooHasManyFooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyFoo").c_str (), fooInstanceId1.GetValue (), fooInstanceId2.GetValue (), fooClassId.GetValue(), fooClassId.GetValue());
    fooHasManyFooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyFoo").c_str (), fooHasManyFooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, fooInstanceId1, fooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyFoo").c_str (), fooHasManyFooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, fooInstanceId2, fooClassId, fooClassId);
    }

    //Insert N-N Relationship
    {
    EC::ECInstanceId manyFooHasManyFooInstanceId1;
    EC::ECInstanceId manyFooHasManyFooInstanceId2;
    EC::ECInstanceId manyFooHasManyFooInstanceId3;
    EC::ECInstanceId manyFooHasManyFooInstanceId4;
    Savepoint savePoint (db, "N-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyFoo").c_str (), fooInstanceId1.GetValue (), fooInstanceId1.GetValue (), fooClassId.GetValue(), fooClassId.GetValue());
    manyFooHasManyFooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyFoo").c_str (), fooInstanceId1.GetValue (), fooInstanceId2.GetValue (), fooClassId.GetValue(), fooClassId.GetValue());
    manyFooHasManyFooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyFoo").c_str (), fooInstanceId2.GetValue (), fooInstanceId1.GetValue (), fooClassId.GetValue(), fooClassId.GetValue());
    manyFooHasManyFooInstanceId3 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyFoo").c_str (), fooInstanceId2.GetValue (), fooInstanceId2.GetValue (), fooClassId.GetValue(), fooClassId.GetValue());
    manyFooHasManyFooInstanceId4 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyFoo").c_str (), manyFooHasManyFooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, fooInstanceId1, fooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyFoo").c_str (), manyFooHasManyFooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, fooInstanceId2, fooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyFoo").c_str (), manyFooHasManyFooInstanceId3.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId2, fooInstanceId1, fooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyFoo").c_str (), manyFooHasManyFooInstanceId4.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId2, fooInstanceId2, fooClassId, fooClassId);
    }

    db.Schemas().CreateECClassViewsInDb();
    }

//---------------------------------------------------------------------------------------
//           Foo  (JoinedTablePerDirectSubclass)  
//            |
//           Goo
//
//------------- Relationships Of base class with Direct Derived class -----------------
//      Foo <- FooHasGoo(REFERENCING) -> Goo
//      Foo <- FooHasManyGoo(REFERENCING) -> Goo
//      Foo <- ManyFooHasManyGoo(REFERENCING) -> Goo
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (JoinedTableECDbMapStrategyTests, BaseAndDirectDerivedClassRelationship)
    {
    SchemaItem testSchema (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='FooHasGoo' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Goo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='FooHasManyGoo' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Goo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='ManyFooHasManyGoo' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Goo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    ECDbR db = SetupECDb ("JoinedTableTest.ecdb", testSchema);
    ASSERT_TRUE (db.IsDbOpen ());

    ECClassId fooClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Foo");
    ECClassId gooClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Goo");

    EC::ECInstanceId fooInstanceId1;
    EC::ECInstanceId fooInstanceId2;
    EC::ECInstanceId gooInstanceId1;
    EC::ECInstanceId gooInstanceId2;

    //Insert Instances for Constraint classes of Relationships
    {
    fooInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Foo (A, B) VALUES(100001, 'Class Foo Instance 1')");
    fooInstanceId2 = InsertTestInstance (db, "INSERT INTO dgn.Foo (A, B) VALUES(100002, 'Class Foo Instance 2')");
    gooInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Goo (C, D) VALUES(200001, 'Class Goo Instance 1')");
    gooInstanceId2 = InsertTestInstance (db, "INSERT INTO dgn.Goo (C, D) VALUES(200002, 'Class Goo Instance 2')");

    if (!fooInstanceId1.IsValid () || !fooInstanceId2.IsValid () || !gooInstanceId1.IsValid () || !gooInstanceId2.IsValid ())
        ASSERT_TRUE (false) << "Instance Id's not valid";
    }

    //Insert 1-1 Relationship
    {
    EC::ECInstanceId fooHasGooInstanceId1;
    Savepoint savePoint (db, "1-1 Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "FooHasGoo").c_str (), fooInstanceId1.GetValue (), gooInstanceId1.GetValue (), fooClassId.GetValue(), gooClassId.GetValue());
    fooHasGooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasGoo").c_str (), fooHasGooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, gooInstanceId1, fooClassId, gooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId fooHasManyGooInstanceId1;
    EC::ECInstanceId fooHasManyGooInstanceId2;
    Savepoint savePoint (db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyGoo").c_str (), fooInstanceId1.GetValue (), gooInstanceId1.GetValue (), fooClassId.GetValue(), gooClassId.GetValue());
    fooHasManyGooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyGoo").c_str (), fooInstanceId1.GetValue (), gooInstanceId2.GetValue (), fooClassId.GetValue(), gooClassId.GetValue());
    fooHasManyGooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyGoo").c_str (), fooHasManyGooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, gooInstanceId1, fooClassId, gooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyGoo").c_str (), fooHasManyGooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, gooInstanceId2, fooClassId, gooClassId);
    }

    //Insert N-N Relationship
    {
    EC::ECInstanceId manyFooHasManyGooInstanceId1;
    EC::ECInstanceId manyFooHasManyGooInstanceId2;
    EC::ECInstanceId manyFooHasManyGooInstanceId3;
    EC::ECInstanceId manyFooHasManyGooInstanceId4;
    Savepoint savePoint (db, "N-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyGoo").c_str (), fooInstanceId1.GetValue (), gooInstanceId1.GetValue (), fooClassId.GetValue(), gooClassId.GetValue());
    manyFooHasManyGooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyGoo").c_str (), fooInstanceId1.GetValue (), gooInstanceId2.GetValue (), fooClassId.GetValue(), gooClassId.GetValue());
    manyFooHasManyGooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyGoo").c_str (), fooInstanceId2.GetValue (), gooInstanceId1.GetValue (), fooClassId.GetValue(), gooClassId.GetValue());
    manyFooHasManyGooInstanceId3 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyGoo").c_str (), fooInstanceId2.GetValue (), gooInstanceId2.GetValue (), fooClassId.GetValue(), gooClassId.GetValue());
    manyFooHasManyGooInstanceId4 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyGoo").c_str (), manyFooHasManyGooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, gooInstanceId1, fooClassId, gooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyGoo").c_str (), manyFooHasManyGooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, gooInstanceId2, fooClassId, gooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyGoo").c_str (), manyFooHasManyGooInstanceId3.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId2, gooInstanceId1, fooClassId, gooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyGoo").c_str (), manyFooHasManyGooInstanceId4.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId2, gooInstanceId2, fooClassId, gooClassId);
    }
    }

//---------------------------------------------------------------------------------------
//           Foo  (JoinedTablePerDirectSubclass)  
//            |
//           Goo
//
//------------- Relationship of base class with Direct Derived class With Key Property -----------------
//      Foo <- FooHasGooWithKeyProp(REFERENCING) -> Goo
//      Foo <- FooHasManyGooWithKeyProp(REFERENCING) -> Goo
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (JoinedTableECDbMapStrategyTests, RelationshipsWithKeyProp)
    {
    SchemaItem testSchema (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='A1' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='FooHasGooWithKeyProp' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Goo' >"
        "        <Key><Property name='A'/></Key>"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='FooHasManyGooWithKeyProp' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Goo' >"
        "        <Key><Property name='A1'/></Key>"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    ECDbR db = SetupECDb ("JoinedTableTest.ecdb", testSchema);
    ASSERT_TRUE (db.IsDbOpen ());

    ECClassId fooClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Foo");
    ECClassId gooClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Goo");

    EC::ECInstanceId fooInstanceId1;
    EC::ECInstanceId gooInstanceId1;
    EC::ECInstanceId gooInstanceId2;

    //Insert Instances for Constraint classes of Relationships
    {
    fooInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Foo (B) VALUES('Class Foo Instance 1')");
    gooInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Goo (C, D) VALUES(200001, 'Class Goo Instance 1')");
    gooInstanceId2 = InsertTestInstance (db, "INSERT INTO dgn.Goo (C, D) VALUES(200002, 'Class Goo Instance 2')");

    if (!fooInstanceId1.IsValid () || !gooInstanceId1.IsValid () || !gooInstanceId2.IsValid ())
        ASSERT_TRUE (false) << "Instance Id's not valid";
    }

    //Insert 1-1 Relationship
    {
    EC::ECInstanceId fooHasGooInstanceId1;
    Savepoint savePoint (db, "1-1 Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "FooHasGooWithKeyProp").c_str (), fooInstanceId1.GetValue (), gooInstanceId1.GetValue (), fooClassId.GetValue(), gooClassId.GetValue());
    fooHasGooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();
    db.SaveChanges();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasGooWithKeyProp").c_str (), fooHasGooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, gooInstanceId1, fooClassId, gooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId fooHasManyGooInstanceId1;
    EC::ECInstanceId fooHasManyGooInstanceId2;
    Savepoint savePoint (db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyGooWithKeyProp").c_str (), fooInstanceId1.GetValue (), gooInstanceId1.GetValue (), fooClassId.GetValue(), gooClassId.GetValue());
    fooHasManyGooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyGooWithKeyProp").c_str (), fooInstanceId1.GetValue (), gooInstanceId2.GetValue (), fooClassId.GetValue(), gooClassId.GetValue());
    fooHasManyGooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();
    db.SaveChanges();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyGooWithKeyProp").c_str (), fooHasManyGooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, gooInstanceId1, fooClassId, gooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyGooWithKeyProp").c_str (), fooHasManyGooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, gooInstanceId2, fooClassId, gooClassId);
    }
    }

//---------------------------------------------------------------------------------------
//           Foo  (JoinedTablePerDirectSubclass)  
//     _______|______
//    |              |
//   Goo            Boo
//                   |
//                  Roo
//
//------------- Relationship Between two Derived Classes -----------------
//      Goo <- GooHasRoo(REFERENCING) -> Roo
//      Goo <- GooHasManyRoo(REFERENCING) -> Roo
//      Goo <- ManyGooHasManyRoo(REFERENCING) -> Roo
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (JoinedTableECDbMapStrategyTests, RelationshipBetweenSubClasses)
    {
    SchemaItem testSchema (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' >"
        "        <BaseClass>Boo</BaseClass>"
        "        <ECProperty propertyName='G' typeName='long'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='GooHasRoo'  strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Source>"
        "    <Target cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='GooHasManyRoo' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='ManyGooHasManyRoo' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    ECDbR db = SetupECDb ("JoinedTableTest.ecdb", testSchema);
    ASSERT_TRUE (db.IsDbOpen ());

    ECClassId gooClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Goo");
    ECClassId rooClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Roo");

    EC::ECInstanceId gooInstanceId1;
    EC::ECInstanceId gooInstanceId2;
    EC::ECInstanceId rooInstanceId1;
    EC::ECInstanceId rooInstanceId2;

    //Insert Instances for Constraint classes of Relationships
    {
    gooInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Goo (C, D) VALUES(100001, 'Class Goo Instance 1')");
    gooInstanceId2 = InsertTestInstance (db, "INSERT INTO dgn.Goo (C, D) VALUES(100002, 'Class Goo Instance 2')");
    rooInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Roo (G, H) VALUES(200001, 'Class Roo Instance 1')");
    rooInstanceId2 = InsertTestInstance (db, "INSERT INTO dgn.Roo (G, H) VALUES(200002, 'Class Roo Instance 2')");

    if (!gooInstanceId1.IsValid () || !gooInstanceId2.IsValid () || !rooInstanceId1.IsValid () || !rooInstanceId2.IsValid ())
        ASSERT_TRUE (false) << "Instance Id's not valid";
    }

    //Insert 1-1 Relationship
    {
    EC::ECInstanceId gooHasRooInstanceId1;
    Savepoint savePoint (db, "1-1 Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "GooHasRoo").c_str (), gooInstanceId1.GetValue (), rooInstanceId1.GetValue (), gooClassId.GetValue(), rooClassId.GetValue());
    gooHasRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "GooHasRoo").c_str (), gooHasRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, rooInstanceId1, gooClassId, rooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId gooHasManyRooInstanceId1;
    EC::ECInstanceId gooHasManyRooInstanceId2;
    Savepoint savePoint (db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "GooHasManyRoo").c_str (), gooInstanceId1.GetValue (), rooInstanceId1.GetValue (), gooClassId.GetValue(), rooClassId.GetValue());
    gooHasManyRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "GooHasManyRoo").c_str (), gooInstanceId1.GetValue (), rooInstanceId2.GetValue (), gooClassId.GetValue(), rooClassId.GetValue());
    gooHasManyRooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "GooHasManyRoo").c_str (), gooHasManyRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, rooInstanceId1, gooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "GooHasManyRoo").c_str (), gooHasManyRooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, rooInstanceId2, gooClassId, rooClassId);
    }

    //Insert N-N Relationship
    {
    EC::ECInstanceId manyGooHasManyRooInstanceId1;
    EC::ECInstanceId manyGooHasManyRooInstanceId2;
    EC::ECInstanceId manyGooHasManyRooInstanceId3;
    EC::ECInstanceId manyGooHasManyRooInstanceId4;
    Savepoint savePoint (db, "N-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "ManyGooHasManyRoo").c_str (), gooInstanceId1.GetValue (), rooInstanceId1.GetValue (), gooClassId.GetValue(), rooClassId.GetValue());
    manyGooHasManyRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyGooHasManyRoo").c_str (), gooInstanceId1.GetValue (), rooInstanceId2.GetValue (), gooClassId.GetValue(), rooClassId.GetValue());
    manyGooHasManyRooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyGooHasManyRoo").c_str (), gooInstanceId2.GetValue (), rooInstanceId1.GetValue (), gooClassId.GetValue(), rooClassId.GetValue());
    manyGooHasManyRooInstanceId3 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyGooHasManyRoo").c_str (), gooInstanceId2.GetValue (), rooInstanceId2.GetValue (), gooClassId.GetValue(), rooClassId.GetValue());
    manyGooHasManyRooInstanceId4 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "ManyGooHasManyRoo").c_str (), manyGooHasManyRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, rooInstanceId1, gooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyGooHasManyRoo").c_str (), manyGooHasManyRooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, rooInstanceId2, gooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyGooHasManyRoo").c_str (), manyGooHasManyRooInstanceId3.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId2, rooInstanceId1, gooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyGooHasManyRoo").c_str (), manyGooHasManyRooInstanceId4.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId2, rooInstanceId2, gooClassId, rooClassId);
    }
    }

//---------------------------------------------------------------------------------------
//           Foo  (JoinedTablePerDirectSubclass)  
//     _______|______
//    |              |
//   Goo            Boo
//                   |
//                  Roo
//
//------------- Relationship Between two Derived Classes With Key Properties-----------------
//      Goo <- GooHasRooWithKeyProp(REFERENCING) -> Roo
//      Goo <- GooHasManyRooWithKeyProp(REFERENCING) -> Roo
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (JoinedTableECDbMapStrategyTests, RelationshipBetweenSubClassesWithKeyProp)
    {
    SchemaItem testSchema (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='E1' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' >"
        "        <BaseClass>Boo</BaseClass>"
        "        <ECProperty propertyName='G' typeName='long'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='GooHasRooWithKeyProp' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Source>"
        "    <Target cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "           <Key>"
        "              <Property name='E'/>"
        "           </Key>"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='GooHasManyRooWithKeyProp' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "           <Key>"
        "              <Property name='E1'/>"
        "           </Key>"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    ECDbR db = SetupECDb ("JoinedTableTest.ecdb", testSchema);
    ASSERT_TRUE (db.IsDbOpen ());

    ECClassId gooClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Goo");
    ECClassId rooClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Roo");

    EC::ECInstanceId gooInstanceId1;
    EC::ECInstanceId rooInstanceId1;
    EC::ECInstanceId rooInstanceId2;

    //Insert Instances for Constraint classes of Relationships
    {
    gooInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Goo (C, D) VALUES(100001, 'Class Goo Instance 1')");
    rooInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Roo (G, H) VALUES(200001, 'Class Roo Instance 1')");
    rooInstanceId2 = InsertTestInstance (db, "INSERT INTO dgn.Roo (G, H) VALUES(200002, 'Class Roo Instance 2')");

    if (!gooInstanceId1.IsValid () || !rooInstanceId1.IsValid () || !rooInstanceId2.IsValid ())
        ASSERT_TRUE (false) << "Instance Id's not valid";
    }

    //Insert 1-1 Relationship
    {
    EC::ECInstanceId gooHasRooInstanceId1;
    Savepoint savePoint (db, "1-1 Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "GooHasRooWithKeyProp").c_str (), gooInstanceId1.GetValue (), rooInstanceId1.GetValue (), gooClassId.GetValue(), rooClassId.GetValue());
    gooHasRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "GooHasRooWithKeyProp").c_str (), gooHasRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, rooInstanceId1, gooClassId, rooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId gooHasManyRooInstanceId1;
    EC::ECInstanceId gooHasManyRooInstanceId2;
    Savepoint savePoint (db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "GooHasManyRooWithKeyProp").c_str (), gooInstanceId1.GetValue (), rooInstanceId1.GetValue (), gooClassId.GetValue(), rooClassId.GetValue());
    gooHasManyRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "GooHasManyRooWithKeyProp").c_str (), gooInstanceId1.GetValue (), rooInstanceId2.GetValue (), gooClassId.GetValue(), rooClassId.GetValue());
    gooHasManyRooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "GooHasManyRooWithKeyProp").c_str (), gooHasManyRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, rooInstanceId1, gooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "GooHasManyRooWithKeyProp").c_str (), gooHasManyRooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, rooInstanceId2, gooClassId, rooClassId);
    }
    }

//---------------------------------------------------------------------------------------
//        Roo(Standalone Class)                  Foo  (JoinedTablePerDirectSubclass)  
//                                                |
//                                               Goo
//
//------ Relationship of a standalone class with a class have JoinedTable strategy ------
//      Foo <- FooHasRoo(REFERENCING) -> Roo
//      Foo <- FooHasManyRoo(REFERENCING) -> Roo
//      Foo <- ManyFooHasManyRoo(REFERENCING) -> Roo
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (JoinedTableECDbMapStrategyTests, RelationshipWithStandAloneClass)
    {
    SchemaItem testSchema (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' >"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='FooHasRoo' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='FooHasManyRoo' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='ManyFooHasManyRoo' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    ECDbR db = SetupECDb ("JoinedTableTest.ecdb", testSchema);
    ASSERT_TRUE (db.IsDbOpen ());

    ECClassId fooClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Foo");
    ECClassId rooClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Roo");

    EC::ECInstanceId fooInstanceId1;
    EC::ECInstanceId fooInstanceId2;
    EC::ECInstanceId rooInstanceId1;
    EC::ECInstanceId rooInstanceId2;

    //Insert Instances for Constraint classes of Relationships
    {
    fooInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Foo (A, B) VALUES(100001, 'Class Foo Instance 1')");
    fooInstanceId2 = InsertTestInstance (db, "INSERT INTO dgn.Foo (A, B) VALUES(100002, 'Class Foo Instance 2')");
    rooInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Roo (E, F) VALUES(200001, 'Class Roo Instance 1')");
    rooInstanceId2 = InsertTestInstance (db, "INSERT INTO dgn.Roo (E, F) VALUES(200002, 'Class Roo Instance 2')");

    if (!fooInstanceId1.IsValid () || !fooInstanceId2.IsValid () || !rooInstanceId1.IsValid () || !rooInstanceId2.IsValid ())
        ASSERT_TRUE (false) << "Instance Id's not valid";
    }

    //Insert 1-1 Relationship
    {
    EC::ECInstanceId fooHasRooInstanceId1;
    Savepoint savePoint (db, "1-1 Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "FooHasRoo").c_str (), fooInstanceId1.GetValue (), rooInstanceId1.GetValue (), fooClassId.GetValue(), rooClassId.GetValue());
    fooHasRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());
    
    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasRoo").c_str (), fooHasRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, rooInstanceId1, fooClassId, rooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId fooHasManyRooInstanceId1;
    EC::ECInstanceId fooHasManyRooInstanceId2;
    Savepoint savePoint (db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyRoo").c_str (), fooInstanceId1.GetValue (), rooInstanceId1.GetValue (), fooClassId.GetValue(), rooClassId.GetValue());
    fooHasManyRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyRoo").c_str (), fooInstanceId1.GetValue (), rooInstanceId2.GetValue (), fooClassId.GetValue(), rooClassId.GetValue());
    fooHasManyRooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyRoo").c_str (), fooHasManyRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, rooInstanceId1, fooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyRoo").c_str (), fooHasManyRooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, rooInstanceId2, fooClassId, rooClassId);
    }

    //Insert N-N Relationship
    {
    EC::ECInstanceId manyFooHasManyRooInstanceId1;
    EC::ECInstanceId manyFooHasManyRooInstanceId2;
    EC::ECInstanceId manyFooHasManyRooInstanceId3;
    EC::ECInstanceId manyFooHasManyRooInstanceId4;
    Savepoint savePoint (db, "N-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyRoo").c_str (), fooInstanceId1.GetValue (), rooInstanceId1.GetValue (), fooClassId.GetValue(), rooClassId.GetValue());
    manyFooHasManyRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyRoo").c_str (), fooInstanceId1.GetValue (), rooInstanceId2.GetValue (), fooClassId.GetValue(), rooClassId.GetValue());
    manyFooHasManyRooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyRoo").c_str (), fooInstanceId2.GetValue (), rooInstanceId1.GetValue (), fooClassId.GetValue(), rooClassId.GetValue());
    manyFooHasManyRooInstanceId3 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyRoo").c_str (), fooInstanceId2.GetValue (), rooInstanceId2.GetValue (), fooClassId.GetValue(), rooClassId.GetValue());
    manyFooHasManyRooInstanceId4 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyRoo").c_str (), manyFooHasManyRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, rooInstanceId1, fooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyRoo").c_str (), manyFooHasManyRooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, rooInstanceId2, fooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyRoo").c_str (), manyFooHasManyRooInstanceId3.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId2, rooInstanceId1, fooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyRoo").c_str (), manyFooHasManyRooInstanceId4.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId2, rooInstanceId2, fooClassId, rooClassId);
    }
    }

//---------------------------------------------------------------------------------------
//                  Foo  (JoinedTablePerDirectSubclass)            Roo(Standalone Class)
//                   |                                                                  
//                  Goo                                                                 
//
//------ Relationship of a class having JoinedTable strategy with standalone class ------
//      Roo <- RooHasFoo(REFERENCING) -> Foo
//      Roo <- RooHasManyFoo(REFERENCING) -> Foo
//      Roo <- RanyFooHasManyFoo(REFERENCING) -> Foo
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (JoinedTableECDbMapStrategyTests, RelationshipWithStandAloneClass1)
    {
    SchemaItem testSchema (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Roo' >"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='RooHasFoo' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Roo' />"
        "    </Source>"
        "    <Target cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='RooHasManyFoo' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Roo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Foo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='ManyRooHasManyFoo' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Roo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Foo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    ECDbR db = SetupECDb ("JoinedTableTest.ecdb", testSchema);
    ASSERT_TRUE (db.IsDbOpen ());

    ECClassId fooClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Foo");
    ECClassId rooClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Roo");

    EC::ECInstanceId fooInstanceId1;
    EC::ECInstanceId fooInstanceId2;
    EC::ECInstanceId rooInstanceId1;
    EC::ECInstanceId rooInstanceId2;

    //Insert Instances for Constraint classes of Relationships
    {
    fooInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Foo (A, B) VALUES(100001, 'Class Foo Instance 1')");
    fooInstanceId2 = InsertTestInstance (db, "INSERT INTO dgn.Foo (A, B) VALUES(100002, 'Class Foo Instance 2')");
    rooInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Roo (E, F) VALUES(200001, 'Class Roo Instance 1')");
    rooInstanceId2 = InsertTestInstance (db, "INSERT INTO dgn.Roo (E, F) VALUES(200002, 'Class Roo Instance 2')");

    if (!fooInstanceId1.IsValid () || !fooInstanceId2.IsValid () || !rooInstanceId1.IsValid () || !rooInstanceId2.IsValid ())
        ASSERT_TRUE (false) << "Instance Id's not valid";
    }

    //Insert 1-1 Relationship
    {
    EC::ECInstanceId rooHasFooInstanceId1;
    Savepoint savePoint (db, "1-1 Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "RooHasFoo").c_str (), rooInstanceId1.GetValue (),fooInstanceId1.GetValue (), rooClassId.GetValue(), fooClassId.GetValue());
    rooHasFooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "RooHasFoo").c_str (), rooHasFooInstanceId1.GetValue());
    VerifyInsertedInstance (db, ecsql.c_str (), rooInstanceId1, fooInstanceId1, rooClassId, fooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId rooHasManyFooInstanceId1;
    EC::ECInstanceId rooHasManyFooInstanceId2;
    Savepoint savePoint (db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "RooHasManyFoo").c_str (), rooInstanceId1.GetValue (), fooInstanceId1.GetValue (), rooClassId.GetValue(), fooClassId.GetValue());
    rooHasManyFooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "RooHasManyFoo").c_str (), rooInstanceId1.GetValue (), fooInstanceId2.GetValue (), rooClassId.GetValue(), fooClassId.GetValue());
    rooHasManyFooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "RooHasManyFoo").c_str (), rooHasManyFooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), rooInstanceId1, fooInstanceId1, rooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "RooHasManyFoo").c_str (), rooHasManyFooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), rooInstanceId1, fooInstanceId2, rooClassId, fooClassId);
    }

    //Insert N-N Relationship
    {
    EC::ECInstanceId manyRooHasManyFooInstanceId1;
    EC::ECInstanceId manyRooHasManyFooInstanceId2;
    EC::ECInstanceId manyRooHasManyFooInstanceId3;
    EC::ECInstanceId manyRooHasManyFooInstanceId4;
    Savepoint savePoint (db, "N-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "ManyRooHasManyFoo").c_str (), rooInstanceId1.GetValue (), fooInstanceId1.GetValue (), rooClassId.GetValue(), fooClassId.GetValue());
    manyRooHasManyFooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyRooHasManyFoo").c_str (), rooInstanceId1.GetValue (), fooInstanceId2.GetValue (), rooClassId.GetValue(), fooClassId.GetValue());
    manyRooHasManyFooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyRooHasManyFoo").c_str (), rooInstanceId2.GetValue (), fooInstanceId1.GetValue (), rooClassId.GetValue(), fooClassId.GetValue());
    manyRooHasManyFooInstanceId3 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyRooHasManyFoo").c_str (), rooInstanceId2.GetValue (), fooInstanceId2.GetValue (), rooClassId.GetValue(), fooClassId.GetValue());
    manyRooHasManyFooInstanceId4 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "ManyRooHasManyFoo").c_str (), manyRooHasManyFooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), rooInstanceId1, fooInstanceId1, rooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyRooHasManyFoo").c_str (), manyRooHasManyFooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), rooInstanceId1, fooInstanceId2, rooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyRooHasManyFoo").c_str (), manyRooHasManyFooInstanceId3.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), rooInstanceId2, fooInstanceId1, rooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyRooHasManyFoo").c_str (), manyRooHasManyFooInstanceId4.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), rooInstanceId2, fooInstanceId2, rooClassId, fooClassId);
    }
    }

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
//                 IFace  (JoinedTablePerDirectSubclass)          Body(Standalone Class) 
//                   |                                                                  
//                  Foo                                                                 
//                   |                                                                  
//                  Goo                              Goo<>Body                          
//                   |                               Boo<>Body                          
//                  Boo                                                                 
//------ Relationship of a class having JoinedTable strategy with standalone class ------
//      IFace <- IFaceHasBody(REFERENCING) -> Body
//      IFace <- IFaceHasManyBody(REFERENCING) -> Body
//      IFace <- ManyIFaceHaveManyBody(REFERENCING) -> Body
//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (JoinedTableECDbMapStrategyTests, PolymorphicRelationshipWithStandAloneClass)
    {
    SchemaItem testSchema (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "   <ECEntityClass typeName='IFace' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='IFace_L' typeName='long'/>"
        "   </ECEntityClass>"
        "    <ECEntityClass typeName='Foo'>"
        "        <BaseClass>IFace</BaseClass>"
        "        <ECProperty propertyName='Foo_L' typeName='long'/>"
        "        <ECProperty propertyName='Foo_S' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='Goo_L' typeName='long'/>"
        "        <ECProperty propertyName='Goo_S' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Boo'>"
        "        <BaseClass>Goo</BaseClass>"
        "        <ECProperty propertyName='Boo_L' typeName='long'/>"
        "        <ECProperty propertyName='Boo_S' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Body'>"
        "        <ECProperty propertyName='Body_L' typeName='long'/>"
        "        <ECProperty propertyName='Body_S' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='IFaceHasBody' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'IFace' />"
        "    </Source>"
        "    <Target cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Body' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='IFaceHasManyBody' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'IFace' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Body' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ManyIFaceHaveManyBody' strength='referencing' modifier='Sealed'>"
        "       <Source cardinality='(0,N)' polymorphic='True'>"
        "           <Class class = 'IFace' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class = 'Body' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>");

    ECDbR db = SetupECDb ("JoinedTableTest.ecdb", testSchema);
    ASSERT_TRUE (db.IsDbOpen ());

    ECClassId booClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Boo");
    ECClassId gooClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Goo");
    ECClassId bodyClassId = db.Schemas ().GetECClassId ("JoinedTableTest", "Body");

    EC::ECInstanceId gooInstanceId1;
    EC::ECInstanceId gooInstanceId2;
    EC::ECInstanceId booInstanceId1;
    EC::ECInstanceId booInstanceId2;
    EC::ECInstanceId bodyInstanceId1;
    EC::ECInstanceId bodyInstanceId2;

    //Insert Instances for Constraint classes of Relationships
    {
    gooInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Goo(Foo_L, Foo_S, Goo_L, Goo_S) VALUES(101, '::101', 102, '::102')");
    gooInstanceId2 = InsertTestInstance (db, "INSERT INTO dgn.Goo(Foo_L, Foo_S, Goo_L, Goo_S) VALUES(102, '::102', 103, '::103')");
    booInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Boo(Foo_L, Foo_S, Boo_L, Boo_S) VALUES(201, '::201', 202, '::202')");
    booInstanceId2 = InsertTestInstance (db, "INSERT INTO dgn.Boo(Foo_L, Foo_S, Boo_L, Boo_S) VALUES(202, '::202', 203, '::203')");
    bodyInstanceId1 = InsertTestInstance (db, "INSERT INTO dgn.Body(Body_L, Body_S) VALUES(301, '::302')");
    bodyInstanceId2 = InsertTestInstance (db, "INSERT INTO dgn.Body(Body_L, Body_S) VALUES(302, '::303')");

    if (!gooInstanceId1.IsValid () || !gooInstanceId2.IsValid () || !booInstanceId1.IsValid () || !booInstanceId2.IsValid ())
        ASSERT_TRUE (false) << "Instance Id's not valid";
    }

    //Insert 1-1 Relationship
    {
    EC::ECInstanceId IFaceHasBodyInstanceId1;
    EC::ECInstanceId IFaceHasBodyInstanceId2;
    Savepoint savePoint (db, "1-1 Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "IFaceHasBody").c_str (), gooInstanceId1.GetValue (), bodyInstanceId1.GetValue (), gooClassId.GetValue(), bodyClassId.GetValue());
    IFaceHasBodyInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "IFaceHasBody").c_str (), booInstanceId1.GetValue (), bodyInstanceId2.GetValue (), booClassId.GetValue(), bodyClassId.GetValue());
    IFaceHasBodyInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "IFaceHasBody").c_str (), IFaceHasBodyInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, bodyInstanceId1, gooClassId, bodyClassId);

    ecsql.Sprintf (ToSelectECSql (db, "IFaceHasBody").c_str (), IFaceHasBodyInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), booInstanceId1, bodyInstanceId2, booClassId, bodyClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId IFaceHasManyBodyInstanceId1;
    EC::ECInstanceId IFaceHasManyBodyInstanceId2;
    Savepoint savePoint (db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "IFaceHasManyBody").c_str (), gooInstanceId1.GetValue (), bodyInstanceId1.GetValue (), gooClassId.GetValue(), bodyClassId.GetValue());
    IFaceHasManyBodyInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "IFaceHasManyBody").c_str (), booInstanceId1.GetValue (), bodyInstanceId2.GetValue (), booClassId.GetValue(), bodyClassId.GetValue());
    IFaceHasManyBodyInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "IFaceHasManyBody").c_str (), IFaceHasManyBodyInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, bodyInstanceId1, gooClassId, bodyClassId);

    ecsql.Sprintf (ToSelectECSql (db, "IFaceHasManyBody").c_str (), IFaceHasManyBodyInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), booInstanceId1, bodyInstanceId2, booClassId, bodyClassId);
    }

    //Insert N-N Relationship
    {
    EC::ECInstanceId ManyIFaceHaveManyBodyInstanceId1;
    EC::ECInstanceId ManyIFaceHaveManyBodyInstanceId2;
    EC::ECInstanceId ManyIFaceHaveManyBodyInstanceId3;
    EC::ECInstanceId ManyIFaceHaveManyBodyInstanceId4;
    Savepoint savePoint (db, "N-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "ManyIFaceHaveManyBody").c_str (), gooInstanceId1.GetValue (), bodyInstanceId1.GetValue (), gooClassId.GetValue(), bodyClassId.GetValue());
    ManyIFaceHaveManyBodyInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyIFaceHaveManyBody").c_str (), gooInstanceId2.GetValue (), bodyInstanceId2.GetValue (), gooClassId.GetValue(), bodyClassId.GetValue());
    ManyIFaceHaveManyBodyInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyIFaceHaveManyBody").c_str (), booInstanceId1.GetValue (), bodyInstanceId1.GetValue (), booClassId.GetValue(), bodyClassId.GetValue());
    ManyIFaceHaveManyBodyInstanceId3 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyIFaceHaveManyBody").c_str (), booInstanceId2.GetValue (), bodyInstanceId2.GetValue (), booClassId.GetValue(), bodyClassId.GetValue());
    ManyIFaceHaveManyBodyInstanceId4 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "ManyIFaceHaveManyBody").c_str (), ManyIFaceHaveManyBodyInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, bodyInstanceId1, gooClassId, bodyClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyIFaceHaveManyBody").c_str (), ManyIFaceHaveManyBodyInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId2, bodyInstanceId2, gooClassId, bodyClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyIFaceHaveManyBody").c_str (), ManyIFaceHaveManyBodyInstanceId3.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), booInstanceId1, bodyInstanceId1, booClassId, bodyClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyIFaceHaveManyBody").c_str (), ManyIFaceHaveManyBodyInstanceId4.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), booInstanceId2, bodyInstanceId2, booClassId, bodyClassId);
    }
    db.Schemas ().CreateECClassViewsInDb ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECDbMapStrategyTests, DropFKConstraintForSharedColumnForSubClasses)
    {
    SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "  <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='A'>"
                        "        <ECProperty propertyName='AName' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B'>"
                        "     <ECCustomAttributes>"
                        "        <ClassMap xmlns='ECDbMap.01.00'>"
                        "             <MapStrategy>"
                        "                <Strategy>SharedTable</Strategy>"
                        "                <AppliesToSubclasses>True</AppliesToSubclasses>"
                        "                <Options>JoinedTablePerDirectSubclass</Options>"
                        "             </MapStrategy>"
                        "        </ClassMap>"
                        "     </ECCustomAttributes>"
                        "        <ECProperty propertyName='BName' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B1Sub'>"
                        "     <ECCustomAttributes>"
                        "        <ClassMap xmlns='ECDbMap.01.00'>"
                        "             <MapStrategy>"
                        "                <Options>SharedColumnsForSubClasses</Options>"
                        "             </MapStrategy>"
                        "        </ClassMap>"
                        "     </ECCustomAttributes>"
                        "        <BaseClass>B</BaseClass>"
                        "        <ECProperty propertyName='B1SubName' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B1SubSub'>"
                        "        <BaseClass>B1Sub</BaseClass>"
                        "        <ECProperty propertyName='B1SubSubName' typeName='string' />"
                        "        <ECProperty propertyName='AId' typeName='long' />"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='Rel1' strength='embedding' modifier='Sealed'>"
                        "      <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
                        "          <OnDeleteAction>Restrict</OnDeleteAction>"
                        "        </ForeignKeyRelationshipMap>"
                        "     </ECCustomAttributes>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B1SubSub'>"
                        "           <Key>"
                        "              <Property name='AId'/>"
                        "           </Key>"
                        "      </Class>"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "fkconstraintsonsharedcolumnsforsubclasses.ecdb");
    ASSERT_FALSE(asserted);

    AssertForeignKey(false, ecdb, "ts_B1Sub", "sc_02");

    ECInstanceKey sourceKey;
    ECInstanceKey targetKey;
    ECInstanceKey relKey;

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.A VALUES('A_prop')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step(sourceKey));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.B1SubSub(BName, B1SubName, B1SubSubName) VALUES('B_prop', 'B1Sub_prop', 'B1SubSub_prop')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step(targetKey));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.Rel1(SourceECInstanceId, TargetECInstanceId, SourceECClassId, TargetECClassId) VALUES(?,?,?,?)"));
    statement.BindId(1, sourceKey.GetECInstanceId());
    statement.BindId(2, targetKey.GetECInstanceId());
    statement.BindId(3, sourceKey.GetECClassId());
    statement.BindId(4, targetKey.GetECClassId());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(relKey));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT SourceECInstanceId, TargetECInstanceId FROM ts.Rel1 WHERE ECInstanceId = ?"));
    statement.BindId(1, relKey.GetECInstanceId());
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(sourceKey.GetECInstanceId().GetValue(), statement.GetValueInt64(0));
    ASSERT_EQ(targetKey.GetECInstanceId().GetValue(), statement.GetValueInt64(1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECDbMapStrategyTests, VerifyONDeleteRestrictWithJoinedTable)
    {
    SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "  <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='A'>"
                        "        <ECProperty propertyName='A_prop' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B'>"
                        "      <ECCustomAttributes>"
                        "        <ClassMap xmlns='ECDbMap.01.00'>"
                        "          <MapStrategy>"
                        "             <Strategy>SharedTable</Strategy>"
                        "             <AppliesToSubclasses>True</AppliesToSubclasses>"
                        "             <Options>JoinedTablePerDirectSubclass</Options>"
                        "          </MapStrategy>"
                        "        </ClassMap>"
                        "     </ECCustomAttributes>"
                        "        <ECProperty propertyName='B_prop' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B1'>"
                        "        <BaseClass>B</BaseClass>"
                        "        <ECProperty propertyName='B1_prop' typeName='string' />"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='AOwnsB' strength='embedding' modifier='Sealed'>"
                        "      <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
                        "          <OnDeleteAction>Cascade</OnDeleteAction>"
                        "        </ForeignKeyRelationshipMap>"
                        "     </ECCustomAttributes>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B'/>"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  <ECRelationshipClass typeName='AOwnsB1' strength='embedding' modifier='Sealed'>"
                        "      <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
                        "          <OnDeleteAction>Restrict</OnDeleteAction>"
                        "        </ForeignKeyRelationshipMap>"
                        "     </ECCustomAttributes>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B1'/>"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  </ECSchema>",
                        true, "Supported cases");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "ondeleterestrictforjoinedtable.ecdb");
    ASSERT_FALSE(asserted);

    ECInstanceKey sourceKey;
    ECInstanceKey targetKey;

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.A VALUES('A1')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step(sourceKey));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.B VALUES('B1')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step(targetKey));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.AOwnsB(SourceECInstanceId, TargetECInstanceId, SourceECClassId, TargetECClassId) VALUES(?,?,?,?)"));
    statement.BindId(1, sourceKey.GetECInstanceId());
    statement.BindId(2, targetKey.GetECInstanceId());
    statement.BindId(3, sourceKey.GetECClassId());
    statement.BindId(4, targetKey.GetECClassId());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ECInstanceKey sourceKey1;
    ECInstanceKey targetKey1;

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.A VALUES('A2')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step(sourceKey1));
    statement.Finalize();


    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.B1(B_prop, B1_prop) VALUES('B2', 'B11')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step(targetKey1));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.AOwnsB1(SourceECInstanceId, TargetECInstanceId, SourceECClassId, TargetECClassId) VALUES(?,?,?,?)"));
    statement.BindId(1, sourceKey1.GetECInstanceId());
    statement.BindId(2, targetKey1.GetECInstanceId());
    statement.BindId(3, sourceKey1.GetECClassId());
    statement.BindId(4, targetKey1.GetECClassId());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM ts.A WHERE ECInstanceId = ?"));
    statement.BindId(1, sourceKey.GetECInstanceId());
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Reset();
    statement.ClearBindings();

    statement.BindId(1, sourceKey1.GetECInstanceId());
    ASSERT_EQ(DbResult::BE_SQLITE_CONSTRAINT_TRIGGER, statement.Step());
    statement.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
struct JoinedTableECSqlStatementsTests : ECDbMappingTestFixture
    {
    void ImportSchemaWithCA (ECSchemaPtr& ecSchema, Utf8CP className);

    void SetUpECSqlStatementTestsDb ();

    void SetUpNestedStructArrayDb ();
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
void JoinedTableECSqlStatementsTests::ImportSchemaWithCA(ECSchemaPtr& ecSchema, Utf8CP className)
    {
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(GetECDb().GetSchemaLocater());
    SchemaKey ecdbmapKey = SchemaKey("ECDbMap", 1, 0);
    ECSchemaPtr ecdbmapSchema = readContext->LocateSchema(ecdbmapKey, SchemaMatchType::LatestCompatible);
    ASSERT_TRUE(ecdbmapSchema.IsValid());
    readContext->AddSchema(*ecSchema);
    ecSchema->AddReferencedSchema(*ecdbmapSchema);

    ECClassP personClass = ecSchema->GetClassP(className);
    ASSERT_TRUE(personClass != nullptr);

    ECClassCP ca = ecdbmapSchema->GetClassCP("ClassMap");
    EXPECT_TRUE(ca != nullptr);
    auto customAttribute = ca->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_TRUE(customAttribute != nullptr);
    ASSERT_TRUE(customAttribute->SetValue("MapStrategy.Strategy", ECValue("SharedTable")) == ECObjectsStatus::Success);
    ASSERT_TRUE(customAttribute->SetValue("MapStrategy.Options", ECValue("JoinedTablePerDirectSubclass")) == ECObjectsStatus::Success);
    ASSERT_TRUE(customAttribute->SetValue("MapStrategy.AppliesToSubclasses", ECValue(true)) == ECObjectsStatus::Success);
    ASSERT_TRUE(personClass->SetCustomAttribute(*customAttribute) == ECObjectsStatus::Success);

    ASSERT_EQ(SUCCESS, GetECDb().Schemas().ImportECSchemas(readContext->GetCache()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
void JoinedTableECSqlStatementsTests::SetUpECSqlStatementTestsDb()
    {
    SetupECDb("JoinedTableECSqlStatementTests.ecdb");

    ECSchemaPtr schemaPtr = ECDbTestUtility::ReadECSchemaFromDisk(L"ECSqlStatementTests.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr != NULL);

    ImportSchemaWithCA(schemaPtr, "Person");

    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(GetECDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
void JoinedTableECSqlStatementsTests::SetUpNestedStructArrayDb()
    {
    SetupECDb("JoinedTableECSqlStatementTests.ecdb");

    ECSchemaPtr schemaPtr = ECDbTestUtility::ReadECSchemaFromDisk(L"NestedStructArrayTest.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr != NULL);

    ImportSchemaWithCA(schemaPtr, "ClassA");

    NestedStructArrayTestSchemaHelper::PopulateNestedStructArrayDb(GetECDb(), true);
    }

struct PowSqlFunction : ScalarFunction
    {
    private:

        virtual void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
            {
            if (args[0].IsNull() || args[1].IsNull())
                {
                ctx.SetResultError("Arguments to POW must not be NULL", -1);
                return;
                }

            double base = args[0].GetValueDouble();
            double exp = args[1].GetValueDouble();

            double res = std::pow(base, exp);
            ctx.SetResultDouble(res);
            }

    public:
        PowSqlFunction() : ScalarFunction("POW", 2, DbValueType::FloatVal) {}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECSqlStatementsTests, PopulateECSql_TestDbWithTestData)
    {
    SetUpECSqlStatementTestsDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECSqlStatementsTests, PopulateECSql_NestedStructArrayDb)
    {
    SetUpNestedStructArrayDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECSqlStatementsTests, UpdateForAbstractBaseClass)
    {
    SetUpECSqlStatementTestsDb();

    //update Instance
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "UPDATE ECST.Customer Set Phone=10000, PersonName.FirstName='Jones' WHERE PersonName.FirstName='Charles' AND PersonName.LastName='Baron'"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    //verify Updated Instance
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT Phone FROM ECST.Person WHERE PersonName.FirstName='Jones' AND PersonName.LastName='Baron'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(10000, statement.GetValueInt(0));
    statement.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECSqlStatementsTests, DeleteForAbstractBaseClass)
    {
    SetUpECSqlStatementTestsDb();

    //Delete Instance
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "DELETE FROM ECST.Person WHERE PersonName.FirstName='Charles' AND PersonName.LastName='Baron'"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    //Verify Delete
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "SELECT * FROM ECST.Person WHERE PersonName.FirstName='Charles' AND PersonName.LastName='Baron'"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECSqlStatementsTests, PersistSqlForQueryOnAbstractBaseClass)
    {
    SetUpECSqlStatementTestsDb();

    Utf8CP expectedGeneratedECSql = "SELECT [Person].[ECInstanceId] FROM (SELECT [ECST_Person].ECClassId,[ECST_Person].[ECInstanceId] FROM [ECST_Person]) [Person]";
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT ECInstanceId FROM ECST.Person"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ(expectedGeneratedECSql, stmt.GetNativeSql());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECSqlStatementsTests, UnionTests)
    {
    SetUpECSqlStatementTestsDb();

    int rowCount;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT COUNT(*) FROM (SELECT CompanyName FROM ECST.Supplier UNION ALL SELECT CompanyName FROM ECST.Shipper)"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
    int count = stmt.GetValueInt(0);
    EXPECT_EQ(6, count);
    stmt.Finalize();

    //Select Statement containing Union All Clause and also Order By clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT CompanyName, Phone FROM ECST.Supplier UNION ALL SELECT CompanyName, Phone FROM ECST.Shipper ORDER BY Phone"));
    rowCount = 0;
    Utf8CP expectedContactNames = "ABCD-Rio Grand-GHIJ-Rio Grand-Rue Perisnon-Salguero-";
    Utf8String actualContactNames;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualContactNames.append(stmt.GetValueText(0));
        actualContactNames.append("-");
        rowCount++;
        }
    ASSERT_STREQ(expectedContactNames, actualContactNames.c_str()) << stmt.GetECSql();
    ASSERT_EQ(6, rowCount);
    stmt.Finalize();

    //Select Statement using UNION Clause, so we should get only distinct results
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT City FROM ECST.Supplier UNION SELECT City FROM ECST.Customer ORDER BY City"));
    rowCount = 0;
    Utf8CP expectedCityNames = "ALASKA-AUSTIN-CA-MD-NC-SAN JOSE-";
    Utf8String actualCityNames;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualCityNames.append(stmt.GetValueText(0));
        actualCityNames.append("-");
        rowCount++;
        }
    ASSERT_STREQ(expectedCityNames, actualCityNames.c_str()) << stmt.GetECSql();
    ASSERT_EQ(6, rowCount);
    stmt.Finalize();

    //Select Statement Using UNION ALL Clause so we should get even Duplicate Results
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT City FROM ECST.Supplier UNION ALL SELECT City FROM ECST.Customer ORDER BY City"));
    rowCount = 0;
    expectedCityNames = "ALASKA-AUSTIN-CA-MD-NC-SAN JOSE-SAN JOSE-";
    actualCityNames.clear();
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualCityNames.append(stmt.GetValueText(0));
        actualCityNames.append("-");
        rowCount++;
        }
    ASSERT_STREQ(expectedCityNames, actualCityNames.c_str()) << stmt.GetECSql();
    ASSERT_EQ(7, rowCount);
    stmt.Finalize();

    //use Custom Scaler function in union query
    PowSqlFunction func;
    ASSERT_EQ(0, GetECDb().AddFunction(func));
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "Select POW(ECInstanceId, 2), GetECClassId() ECClassId, ECInstanceId From ECST.Supplier UNION ALL Select POW(ECInstanceId, 2), GetECClassId() ECClassId, ECInstanceId From ECST.Customer ORDER BY ECInstanceId"));
    rowCount = 0;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        int base = stmt.GetValueInt(2);
        ASSERT_EQ(std::pow(base, 2), stmt.GetValueInt(0));
        rowCount++;
        }
    ASSERT_EQ(7, rowCount);
    stmt.Finalize();

    //use aggregate function in Union Query
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT Count(*), AVG(Phone) FROM (SELECT Phone FROM ECST.Supplier UNION ALL SELECT Phone FROM ECST.Customer)"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
    ASSERT_EQ(7, stmt.GetValueInt(0));
    ASSERT_EQ(1400, stmt.GetValueInt(1));
    stmt.Finalize();

    //Use GROUP BY clause in Union Query
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT COUNT(*), Phone FROM (SELECT GetECClassId() ECClassId, Phone FROM ECST.Supplier UNION ALL SELECT GetECClassId() ECClassId, Phone FROM ECST.Customer) GROUP BY ECClassId ORDER BY Phone"));

    //Get Row one
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(3, stmt.GetValueInt(0));
    ASSERT_EQ(1300, stmt.GetValueDouble(1));

    //Get Row two
    ASSERT_TRUE(stmt.Step() == BE_SQLITE_ROW);
    ASSERT_EQ(4, stmt.GetValueInt(0));
    ASSERT_EQ(1700, stmt.GetValueDouble(1));

    ASSERT_TRUE(stmt.Step() == BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECSqlStatementsTests, PolymorphicUpdate)
    {
    // Create and populate a sample project
    SetUpNestedStructArrayDb();

    //Updates the instances of ClassA all the Derived Classes Properties values should also be changed. 
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "UPDATE nsat.ClassA SET T='UpdatedValue', I=2"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT ECInstanceId, GetECClassId(), I,T FROM nsat.ClassA"));
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        EXPECT_EQ(2, stmt.GetValueInt(2)) << "Int Value don't match for instance " << stmt.GetValueInt64(0) << " with class id: " << stmt.GetValueInt64(1);
        EXPECT_STREQ("UpdatedValue", stmt.GetValueText(3)) << "String value don't match for instance " << stmt.GetValueInt64(0) << " with class id: " << stmt.GetValueInt64(1);
        }
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECSqlStatementsTests, PolymorphicDelete)
    {
    SetUpNestedStructArrayDb();

    ASSERT_TRUE(GetECDb().TableExists("nsat_DerivedA"));
    ASSERT_TRUE(GetECDb().TableExists("nsat_DerivedB"));
    ASSERT_FALSE(GetECDb().TableExists("nsat_DoubleDerivedA"));
    ASSERT_FALSE(GetECDb().TableExists("nsat_DoubleDerivedC"));

    //Delete all Instances of the base class, all the structArrays and relationships should also be deleted.
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(GetECDb(), "DELETE FROM nsat.ClassA"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();
    GetECDb().SaveChanges();

    bvector<Utf8String> tableNames = {"ClassA", "DerivedA", "DerivedB", "BaseHasDerivedA", "DerivedBHasChildren"};

    for (Utf8StringCR tableName : tableNames)
        {
        Utf8String selectSql = "SELECT count(*) FROM nsat_";
        selectSql.append(tableName);
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(GetECDb(), selectSql.c_str())) << "Prepare failed for " << selectSql.c_str();
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Step failed for " << selectSql.c_str();
        ASSERT_EQ(0, stmt.GetValueInt(0)) << "Table " << tableName.c_str() << " is expected to be empty after DELETE FROM nsat.ClassA";
        stmt.Finalize();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECSqlStatementsTests, ExceptTests)
    {
    SetUpECSqlStatementTestsDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT CompanyName FROM ECST.Supplier EXCEPT SELECT CompanyName FROM ECST.Shipper"));
    int rowCount = 0;
    Utf8CP expectedContactNames = "ABCD-GHIJ-";
    Utf8String actualContactNames;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualContactNames.append(stmt.GetValueText(0));
        actualContactNames.append("-");
        rowCount++;
        }
    ASSERT_STREQ(expectedContactNames, actualContactNames.c_str()) << stmt.GetECSql();
    ASSERT_EQ(2, rowCount);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT ContactTitle FROM ECST.Customer EXCEPT SELECT ContactTitle FROM ECST.Supplier"));
    rowCount = 0;
    expectedContactNames = "AM-Adm-SPIELMANN-";
    actualContactNames.clear();
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualContactNames.append(stmt.GetValueText(0));
        actualContactNames.append("-");
        rowCount++;
        }
    ASSERT_STREQ(expectedContactNames, actualContactNames.c_str()) << stmt.GetECSql();
    ASSERT_EQ(3, rowCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECSqlStatementsTests, IntersectTests)
    {
    SetUpECSqlStatementTestsDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT CompanyName FROM ECST.Supplier INTERSECT SELECT CompanyName FROM ECST.Shipper ORDER BY CompanyName"));
    int rowCount = 0;
    Utf8CP expectedContactNames = "Rio Grand";
    Utf8String actualContactNames;
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualContactNames.append(stmt.GetValueText(0));
        rowCount++;
        }
    ASSERT_STREQ(expectedContactNames, actualContactNames.c_str());
    ASSERT_EQ(1, rowCount);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT ContactTitle FROM ECST.Supplier INTERSECT SELECT ContactTitle FROM ECST.Customer ORDER BY ContactTitle"));
    rowCount = 0;
    expectedContactNames = "Brathion";
    actualContactNames.clear();
    while (stmt.Step() != BE_SQLITE_DONE)
        {
        actualContactNames.append(stmt.GetValueText(0));
        rowCount++;
        }
    ASSERT_STREQ(expectedContactNames, actualContactNames.c_str());
    ASSERT_EQ(1, rowCount);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECDbMapStrategyTests, JoinedTableForClassesWithoutBusinessProperties)
    {
    SchemaItem testSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='BaseClass'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='B1' typeName='long'/>"
        "        <ECProperty propertyName='B2' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Derived0'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECProperty propertyName='D01' typeName='long'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Derived1' modifier='Abstract'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Derived2'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>");

    ECDbR db = SetupECDb("ClassesWithoutBusinessProperties.ecdb", testSchema);
    ASSERT_TRUE(db.IsDbOpen());

    //Verify that only one base table and one Secondary Table have been created, rest of the tables will not be mapped as joined table because they don't have business properties.
    Statement statement;
    ASSERT_EQ(BE_SQLITE_OK, statement.Prepare(db, "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name LIKE 'dgn_%'"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(4, statement.GetValueInt(0));
    }

END_ECDBUNITTESTS_NAMESPACE
