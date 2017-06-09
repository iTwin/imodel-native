/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/JoinedTableTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
struct JoinedTableTestFixture : DbMappingTestFixture
    {
    protected:
        void AssertTableLayouts(ECDbCR, bmap<Utf8String, Utf8String> const& tableLayouts) const;
        ECInstanceId InsertTestInstance(ECDbCR ecdb, Utf8CP ecsql);
        Utf8String ToInsertECSql(ECDbCR ecdb, Utf8CP className);
        Utf8String ToSelectECSql(ECDbCR ecdb, Utf8CP className);
        void VerifyInsertedInstance(ECDbR ecdb, Utf8CP ecsql, ECInstanceId sourceInstanceId, ECInstanceId targetInstanceId, ECClassId sourceClassId, ECClassId targetClassId);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                      11/15
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableTestFixture, TableLayout)
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
    TestItem testItem(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                 "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='ts' version='1.0'"
                                 "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                 "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                 "    <ECEntityClass typeName='C0'>"
                                 "        <ECCustomAttributes>"
                                 "            <ClassMap xmlns='ECDbMap.02.00'>"
                                 "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                 "            </ClassMap>"
                                 "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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

    testItem = TestItem(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='ts' version='1.0'"
                                   "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='C0'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='C0_A' typeName='long'/>"
                                   "        <ECProperty propertyName='C0_B' typeName='string'/>"
                                   "    </ECEntityClass>"
                                   "   <ECEntityClass typeName='C1'>"
                                   "        <ECCustomAttributes>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00' />"
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

    testItem = TestItem(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='ts' version='1.0'"
                                   "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='C0'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='A' typeName='long'/>"
                                   "        <ECProperty propertyName='B' typeName='string'/>"
                                   "    </ECEntityClass>"
                                   "   <ECEntityClass typeName='C1'>"
                                   "        <ECCustomAttributes>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <BaseClass>C0</BaseClass>"
                                   "        <ECProperty propertyName='C' typeName='long'/>"
                                   "        <ECProperty propertyName='D' typeName='string'/>"
                                   "    </ECEntityClass>"
                                   "   <ECEntityClass typeName='C2'>"
                                   "        <ECCustomAttributes>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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
        ASSERT_EQ(SUCCESS, SetupECDb("joinedtablemapstrategy.ecdb", testItem.m_testSchema));

        AssertTableLayouts(m_ecdb, testItem.m_expectedTableLayout);
        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         10/05
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableTestFixture, BasicCRUD)
    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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

    testSchemas.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "               <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
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

    testSchemas.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
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
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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

    testSchemas.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.3.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' >"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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

    testSchemas.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Boo' >"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
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

    auto assertNonSelectECSql = [] (ECDbCR ecdb,Utf8CP ecsql)
        {
        ECSqlStatement stmt;
        LOG.infov("Executing : %s", ecsql);
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql)) << " ECSQL: " << ecsql;
        LOG.infov("NativeSQL : %s", stmt.GetNativeSql());
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << " ECSQL: " << ecsql;
        };

    auto assertSelectECSql = [] (ECDbCR ecdb, Utf8CP ecsql, int columnCountExpected, int rowCountExpected)
        {
        ECSqlStatement stmt;
        LOG.infov("Executing : %s", ecsql);
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql)) << " ECSQL: " << ecsql;
        LOG.infov("NativeSQL : %s", stmt.GetNativeSql());
        ASSERT_EQ(columnCountExpected, stmt.GetColumnCount()) << " ECSQL: " << ecsql;

        int actualRowCount = 0;
        while (stmt.Step() == BE_SQLITE_ROW)
            actualRowCount++;

        ASSERT_EQ(rowCountExpected, actualRowCount) << " ECSQL: " << ecsql;
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



    int fileNameSuffix = 0;
    for (SchemaItem const& testSchema : testSchemas)
        {
        fileNameSuffix++;
        Utf8String fileName;
        fileName.Sprintf("JoinedTableTest%d.ecdb", fileNameSuffix);
        ASSERT_EQ(SUCCESS, SetupECDb(fileName.c_str(), testSchema));
        for (Utf8StringCR nonSelectECSql : nonSelectECSqls)
            {
            assertNonSelectECSql(m_ecdb, nonSelectECSql.c_str());
            }

        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B FROM dgn.Foo", 3, 16);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B FROM ONLY dgn.Foo", 3, 4);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B FROM dgn.Foo WHERE A = 102 AND B = 'b2'", 3, 1);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B FROM ONLY dgn.Foo WHERE A = 102 AND B = 'b2'", 3, 0);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B FROM ONLY dgn.Foo  WHERE A = 104 AND B = 'b17'", 3, 1);

        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B, C, D FROM dgn.Goo", 5, 4);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B FROM dgn.Goo", 3, 4);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, C, D FROM dgn.Goo", 3, 4);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B, C, D FROM ONLY dgn.Goo", 5, 4);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B, C, D FROM dgn.Goo WHERE A = 102 AND B ='b2' AND C = 202 AND D ='d2'", 5, 1);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B, C, D FROM ONLY dgn.Goo WHERE A = 102 AND B ='b2' AND C = 202 AND D ='d2'", 5, 1);

        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B, E, F FROM dgn.Boo", 5, 8);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B FROM dgn.Boo", 3, 8);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, E, F FROM dgn.Boo", 3, 8);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B, E, F FROM ONLY dgn.Boo", 5, 4);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B, E, F FROM dgn.Boo WHERE A = 102 AND B ='b8' AND E = 202 AND F ='f2'", 5, 1);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B, E, F FROM ONLY dgn.Boo WHERE A = 102 AND B ='b8' AND E = 202 AND F ='f2'", 5, 1);

        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B, G, H FROM dgn.Roo", 5, 4);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B FROM dgn.Roo", 3, 4);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, G, H FROM dgn.Roo", 3, 4);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B, G, H FROM ONLY dgn.Roo", 5, 4);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B, G, H FROM dgn.Roo WHERE A = 102 AND B ='b13' AND G = 202 AND H ='h2'", 5, 1);
        assertSelectECSql(m_ecdb, "SELECT ECInstanceId, A, B, G, H FROM ONLY dgn.Roo WHERE A = 102 AND B ='b13' AND G = 202 AND H ='h2'", 5, 1);

        m_ecdb.CloseDb();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                      04/17
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableTestFixture, Update)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("joinedtable.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                   <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                   <ECEntityClass typeName="Base" modifier="Abstract">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Prop1" typeName="string" />
                        <ECProperty propertyName="Prop2" typeName="int" />
                    </ECEntityClass>
                    <ECEntityClass typeName="Sub">
                      <BaseClass>Base</BaseClass>
                      <ECProperty propertyName="SubProp1" typeName="string" />
                      <ECProperty propertyName="SubProp2" typeName="int" />
                     </ECEntityClass>
               </ECSchema>)xml")));

    auto assertRow = [] (ECDbCR ecdb, std::tuple<Utf8CP, int, Utf8CP, int> const& expectedRowValues, ECInstanceKey const& key)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Prop1,Prop2,SubProp1,SubProp2 FROM ts.Sub WHERE ECInstanceId=?"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << key.GetInstanceId().ToString().c_str();
        ASSERT_STREQ(std::get<0>(expectedRowValues), stmt.GetValueText(0)) << key.GetInstanceId().ToString().c_str();
        ASSERT_EQ(std::get<1>(expectedRowValues), stmt.GetValueInt(1)) << key.GetInstanceId().ToString().c_str();
        ASSERT_STREQ(std::get<2>(expectedRowValues), stmt.GetValueText(2)) << key.GetInstanceId().ToString().c_str();
        ASSERT_EQ(std::get<3>(expectedRowValues), stmt.GetValueInt(3)) << key.GetInstanceId().ToString().c_str();
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << key.GetInstanceId().ToString().c_str();
        };
    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Sub(Prop1,Prop2,SubProp1,SubProp2) VALUES('1',1,'1',1)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << stmt.GetECSql();
    assertRow(m_ecdb, {"1",1,"1",1}, key);
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET Prop1=?, Prop2=?, SubProp1=?,SubProp2=? WHERE ECInstanceId=?"));

    std::tuple<Utf8CP, int, Utf8CP, int> expectedRow {"2",2,"2",2};
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, std::get<0>(expectedRow), IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, std::get<1>(expectedRow)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(3, std::get<2>(expectedRow), IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(4, std::get<3>(expectedRow)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(5, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(1, m_ecdb.GetModifiedRowCount()) << stmt.GetECSql();
    stmt.Finalize();
    assertRow(m_ecdb, expectedRow, key);

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET Prop1=?, Prop2=?, SubProp1=?,SubProp2=? WHERE Prop2=2 AND SubProp2=2"));
    expectedRow = std::tuple<Utf8CP, int, Utf8CP, int> {"3",3,"3",3};
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, std::get<0>(expectedRow), IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, std::get<1>(expectedRow)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(3, std::get<2>(expectedRow), IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(4, std::get<3>(expectedRow)));
    int totalModifiedRowCountBefore = m_ecdb.GetTotalModifiedRowCount();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << "SQL: " << stmt.GetNativeSql();
    ASSERT_EQ(2, m_ecdb.GetTotalModifiedRowCount() - totalModifiedRowCountBefore) << stmt.GetECSql() << "SQL: " << stmt.GetNativeSql();
    stmt.Finalize();
    assertRow(m_ecdb, expectedRow, key);

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET Prop1=?, Prop2=?, SubProp1=?,SubProp2=? WHERE Prop2=3 AND SubProp2=1"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "4", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, 4));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(3, "4", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(4, 4));
    totalModifiedRowCountBefore = m_ecdb.GetTotalModifiedRowCount();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(0, m_ecdb.GetTotalModifiedRowCount() - totalModifiedRowCountBefore) << stmt.GetECSql();
    //values shouldn't have been modified, so must have values from previous update
    assertRow(m_ecdb, expectedRow, key);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  04/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableTestFixture, ECSqlInsertAffectingOneTableOnly)
    {
    SetupECDb("ECSqlUpdateOptimization.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
           <ECEntityClass typeName="Base" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                </ECCustomAttributes>"
               <ECProperty propertyName="Code" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub" modifier="Sealed">
                <BaseClass>Base</BaseClass>
               <ECProperty propertyName="SubNo" typeName="int" />
           </ECEntityClass>
        </ECSchema>)xml"));
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    auto assertRow = [] (ECDbCR ecdb, ECInstanceKey const& key, std::pair<int const*, int const*> const& expectedValues)
        {
        Statement stmt;
        //primary table
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT ECClassId,Code FROM ts_Base WHERE Id=?"));
        ASSERT_EQ(BE_SQLITE_OK, stmt.BindId(1, key.GetInstanceId()));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(key.GetClassId().GetValue(), stmt.GetValueUInt64(0)) << "ECClassId";
        if (expectedValues.first == nullptr)
            ASSERT_TRUE(stmt.IsColumnNull(1)) << "Code is expected to be NULL";
        else
            ASSERT_EQ(*expectedValues.first, stmt.GetValueInt(1)) << "Code";
        stmt.Finalize();

        //joined table
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT ECClassId,SubNo FROM ts_Sub WHERE BaseId=?"));
        ASSERT_EQ(BE_SQLITE_OK, stmt.BindId(1, key.GetInstanceId()));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_EQ(key.GetClassId().GetValue(), stmt.GetValueUInt64(0)) << "ECClassId";
        if (expectedValues.second == nullptr)
            ASSERT_TRUE(stmt.IsColumnNull(1)) << "SubNo is expected to be NULL";
        else
            ASSERT_EQ(*expectedValues.second, stmt.GetValueInt(1)) << "SubNo";
        };

    const int expectedCode = 100;
    const int expectedSubNo = 200;

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Sub(Code,SubNo) VALUES(?,?)"));

    //don't insert any values
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    assertRow(m_ecdb, key, {nullptr, nullptr});
    stmt.Reset();
    stmt.ClearBindings();

    //insert first value only
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1,expectedCode));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    assertRow(m_ecdb, key, {&expectedCode, nullptr});
    stmt.Reset();
    stmt.ClearBindings();

    //insert second value only
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, expectedSubNo));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    assertRow(m_ecdb, key, {nullptr, &expectedSubNo});
    stmt.Reset();
    stmt.ClearBindings();

    //insert both values
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, expectedCode));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, expectedSubNo));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    assertRow(m_ecdb, key, {&expectedCode, &expectedSubNo});
    stmt.Reset();
    stmt.ClearBindings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  04/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableTestFixture, ECSqlUpdateOptimization)
    {
    SetupECDb("ECSqlUpdateOptimization.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
           <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
           <ECEntityClass typeName="Base" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                </ECCustomAttributes>"
               <ECProperty propertyName="Code" typeName="int" />
           </ECEntityClass>
           <ECEntityClass typeName="Sub" modifier="Sealed">
                <BaseClass>Base</BaseClass>
               <ECProperty propertyName="SubNo" typeName="int" />
           </ECEntityClass>
        </ECSchema>)xml"));
    ASSERT_TRUE(m_ecdb.IsDbOpen());

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Sub(Code,SubNo) VALUES(100,200)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    auto assertNativeSql = [] (Utf8CP expected, ECSqlStatement const& stmt)
        {
        bvector<Utf8String> expectedSqls, actualSqls;
        BeStringUtilities::Split(expected, ";", expectedSqls);
        BeStringUtilities::Split(stmt.GetNativeSql(), ";", actualSqls);
        ASSERT_EQ(expectedSqls.size(), actualSqls.size()) << stmt.GetECSql();
        std::sort(expectedSqls.begin(), expectedSqls.end());
        std::sort(actualSqls.begin(), actualSqls.end());
        for (size_t i = 0; i < expectedSqls.size(); i++)
            {
            ASSERT_STREQ(expectedSqls[i].c_str(), actualSqls[i].c_str()) << stmt.GetECSql();
            }
        };

    ECClassId subClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Sub");
    ASSERT_TRUE(subClassId.IsValid());
    Utf8String subClassIdStr = subClassId.ToString();

    //Set prop in first table
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET Code=200 WHERE Code=100"));
    Utf8String expectedSql;
    expectedSql.Sprintf("UPDATE [ts_Base] SET [Code]=200 WHERE [Code]=100 AND (ECClassId IN (SELECT ClassId FROM ec_cache_ClassHierarchy WHERE BaseClassId=%s))",
                        subClassIdStr.c_str());
    assertNativeSql(expectedSql.c_str(), stmt);
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET Code=200 WHERE SubNo=200"));
    assertNativeSql("UPDATE [ts_Base] SET [Code]=200 WHERE InVirtualSet(:_ecdb_ecsqlparam_id_col1,[Id])", stmt);
    stmt.Finalize();

    //now without classid filter
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET Code=200 WHERE Code=100 ECSQLOPTIONS NoECClassIdFilter"));
    assertNativeSql("UPDATE [ts_Base] SET [Code]=200 WHERE [Code]=100", stmt);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET Code=200 WHERE SubNo=200 ECSQLOPTIONS NoECClassIdFilter"));
    assertNativeSql("UPDATE [ts_Base] SET [Code]=200 WHERE InVirtualSet(:_ecdb_ecsqlparam_id_col1,[Id])", stmt);
    stmt.Finalize();

    //Set prop in second table
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET SubNo=300 WHERE SubNo=200"));
    expectedSql.Sprintf("UPDATE [ts_Sub] SET [SubNo]=300 WHERE [SubNo]=200 AND (ECClassId IN (SELECT ClassId FROM ec_cache_ClassHierarchy WHERE BaseClassId=%s))",
                        subClassIdStr.c_str());
    assertNativeSql(expectedSql.c_str(), stmt);
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET SubNo=300 WHERE Code=100"));
    assertNativeSql("UPDATE [ts_Sub] SET [SubNo]=300 WHERE InVirtualSet(:_ecdb_ecsqlparam_id_col1,[BaseId])", stmt);
    stmt.Finalize();

    //now without classid filter
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET SubNo=300 WHERE SubNo=200 ECSQLOPTIONS NoECClassIdFilter"));
    assertNativeSql("UPDATE [ts_Sub] SET [SubNo]=300 WHERE [SubNo]=200", stmt);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET SubNo=300 WHERE Code=100 ECSQLOPTIONS NoECClassIdFilter"));
    assertNativeSql("UPDATE [ts_Sub] SET [SubNo]=300 WHERE InVirtualSet(:_ecdb_ecsqlparam_id_col1,[BaseId])", stmt);
    stmt.Finalize();

    //Set prop in both tables
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET Code=200, SubNo=300 WHERE Code=100"));
    assertNativeSql("UPDATE [ts_Base] SET [Code]=200 WHERE InVirtualSet(:_ecdb_ecsqlparam_id_col1,[Id]);"
                    "UPDATE [ts_Sub] SET [SubNo]=300 WHERE InVirtualSet(:_ecdb_ecsqlparam_id_col1,[BaseId])", stmt);
    stmt.Finalize();

    //now without classid filter
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET Code=200, SubNo=300 WHERE Code=100 ECSQLOPTIONS NoECClassIdFilter"));
    assertNativeSql("UPDATE [ts_Base] SET [Code]=200 WHERE InVirtualSet(:_ecdb_ecsqlparam_id_col1,[Id]);UPDATE [ts_Sub] SET [SubNo]=300 WHERE InVirtualSet(:_ecdb_ecsqlparam_id_col1,[BaseId])", stmt);
    stmt.Finalize();

    //Set prop in both tables, but filter by ECInstanceId
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET Code=200, SubNo=300 WHERE ECInstanceId=?"));
    expectedSql.Sprintf("UPDATE [ts_Base] SET [Code]=200 WHERE [Id]=:_ecdb_ecsqlparam_ix1_col1 AND (ECClassId IN (SELECT ClassId FROM ec_cache_ClassHierarchy WHERE BaseClassId=%s));"
                        "UPDATE [ts_Sub] SET [SubNo]=300 WHERE [BaseId]=:_ecdb_ecsqlparam_ix1_col1 AND (ECClassId IN (SELECT ClassId FROM ec_cache_ClassHierarchy WHERE BaseClassId=%s))",
                        subClassIdStr.c_str(), subClassIdStr.c_str());
    assertNativeSql(expectedSql.c_str(), stmt);
    stmt.Finalize();

    //now without classid filter
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET Code=200, SubNo=300 WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter"));
    assertNativeSql("UPDATE [ts_Base] SET [Code]=200 WHERE [Id]=:_ecdb_ecsqlparam_ix1_col1;UPDATE [ts_Sub] SET [SubNo]=300 WHERE [BaseId]=:_ecdb_ecsqlparam_ix1_col1", stmt);
    stmt.Finalize();

    //Set prop in both tables, but filter by ECClassId
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET Code=200, SubNo=300 WHERE ECClassId=?"));
    expectedSql.Sprintf("UPDATE [ts_Base] SET [Code]=200 WHERE [ECClassId]=:_ecdb_ecsqlparam_ix1_col1 AND (ECClassId IN (SELECT ClassId FROM ec_cache_ClassHierarchy WHERE BaseClassId=%s));"
                        "UPDATE [ts_Sub] SET [SubNo]=300 WHERE [ECClassId]=:_ecdb_ecsqlparam_ix1_col1 AND (ECClassId IN (SELECT ClassId FROM ec_cache_ClassHierarchy WHERE BaseClassId=%s))",
                        subClassIdStr.c_str(), subClassIdStr.c_str());
    assertNativeSql(expectedSql.c_str(), stmt);
    stmt.Finalize();

    //now without classid filter
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET Code=200, SubNo=300 WHERE ECClassId=? ECSQLOPTIONS NoECClassIdFilter"));
    assertNativeSql("UPDATE [ts_Base] SET [Code]=200 WHERE [ECClassId]=:_ecdb_ecsqlparam_ix1_col1;UPDATE [ts_Sub] SET [SubNo]=300 WHERE [ECClassId]=:_ecdb_ecsqlparam_ix1_col1", stmt);
    stmt.Finalize();

    //Set prop in both tables, but filter by ECInstanceId and a regular prop
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Sub SET Code=200, SubNo=300 WHERE ECInstanceId=? OR Code=?"));
    assertNativeSql("UPDATE [ts_Base] SET [Code]=200 WHERE InVirtualSet(:_ecdb_ecsqlparam_id_col1,[Id]);"
                    "UPDATE [ts_Sub] SET [SubNo]=300 WHERE InVirtualSet(:_ecdb_ecsqlparam_id_col1,[BaseId])", stmt);
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                         1/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableTestFixture, CRUDOnColumnTypes_Physical_Shared_Overflow)
    {
    SchemaItem schemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Goo'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Foo'>"
        "       <BaseClass>Goo</BaseClass>"
        "        <ECProperty propertyName='B' typeName='int'/>"
        "        <ECProperty propertyName='C' typeName='double'/>"
        "    </ECEntityClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("crud.ecdb", schemaItem));

    ECSqlStatement stmt;
    Utf8String sql;
    Statement sqlstmt;

    //Verifying that the properties are mapped correctly to the desired columns
    std::vector<Utf8Char> Props = {'A','B','C'};
    for (size_t i = 0; i < 3; i++)
        {
        sql.Sprintf("Select ColumnKind from ec_Column c Inner Join ec_PropertyMap pm on c.id=pm.ColumnId Inner join ec_PropertyPath pp on pm.PropertyPathId=pp.Id Where AccessString='%c'", Props[i]);
        ASSERT_EQ(DbResult::BE_SQLITE_OK, sqlstmt.Prepare(m_ecdb, sql.c_str())) << "Prepare failed for sql: " << sql;
        ASSERT_EQ(DbResult::BE_SQLITE_ROW, sqlstmt.Step());
        switch (i)
            {
                case 0:
                    ASSERT_EQ(64, sqlstmt.GetValueInt(0)); //Column A is unshared Data Column
                    break;
                default:
                    ASSERT_EQ(128, sqlstmt.GetValueInt(0)); //other columns are a Shared Column
                    break;
            }
        sqlstmt.Finalize();
        }

    //-----------------------------INSERT----------------------------------------------------
    ASSERT_EQ(stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo (ECInstanceId, A, B, C) VALUES (?,?,?,?)"), ECSqlStatus::Success);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 100));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "val1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(3, 22));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(4, 100.12));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);

    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 200));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "val2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(3, 34));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(4, 34.56));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);

    stmt.Finalize();

    //-----------------------------SELECT----------------------------------------------------
    ASSERT_EQ(stmt.Prepare(m_ecdb, "SELECT A, B, C FROM ts.Foo WHERE ECInstanceId = ?"), ECSqlStatus::Success);
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 100));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

    ASSERT_STRCASEEQ(stmt.GetValueText(0), "val1");
    ASSERT_EQ(stmt.GetValueInt(1), 22);
    ASSERT_EQ(stmt.GetValueDouble(2), 100.12);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);

    stmt.Finalize();

    //-----------------------------UPDATE----------------------------------------------------

    ASSERT_EQ(stmt.Prepare(m_ecdb, "UPDATE ts.Foo SET A= ?, B= ?, C= ? WHERE ECInstanceId = ?"), ECSqlStatus::Success);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "modVal1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, 44));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(3, 78.21));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(4, 100));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);

    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_EQ(stmt.Prepare(m_ecdb, "SELECT A, B, C FROM ts.Foo WHERE ECInstanceId = 100"), ECSqlStatus::Success);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

    ASSERT_STRCASEEQ(stmt.GetValueText(0), "modVal1");
    ASSERT_EQ(stmt.GetValueInt(1), 44);
    ASSERT_EQ(stmt.GetValueDouble(2), 78.21);
    stmt.Finalize();

    //-----------------------------DELETE----------------------------------------------------

    ASSERT_EQ(stmt.Prepare(m_ecdb, "DELETE FROM ts.Foo WHERE ECInstanceId = ?"), ECSqlStatus::Success);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 200));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(stmt.Prepare(m_ecdb, "SELECT A, B, C FROM ts.Foo WHERE ECInstanceId = 200"), ECSqlStatus::Success);
    ASSERT_NE(stmt.Step(), BE_SQLITE_ROW);
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableTestFixture, AcrossMultipleSchemaImports)
    {
    SchemaItem baseTestSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='ReferredSchema' nameSpacePrefix='rs' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' >"
        "         <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    SchemaItem secondTestItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECSchemaReference name='ReferredSchema' version='01.00' prefix='rs' />"
        "    <ECEntityClass typeName='Sub2' >"
        "         <BaseClass>rs:Base</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' >"
        "         <BaseClass>rs:Sub1</BaseClass>"
        "        <ECProperty propertyName='P11' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    Utf8String ecdbFilePath;
    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, baseTestSchema, "JoinedTablePerDirectSubclass.ecdb")) << "Mapstrategy Option JoinedTablePerDirectSubclass (applied to subclasses) is expected to succeed";
    ecdbFilePath = ecdb.GetDbFileName();
    ecdb.CloseDb();
    }

    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbFilePath.c_str(), ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    ASSERT_EQ(SUCCESS, ImportSchema(ecdb, secondTestItem)) << "Mapstrategy Option JoinedTablePerDirectSubclass (applied to subclasses) is expected to be honored from base Class of Refered schema";

    bmap<Utf8String, Utf8String> expectedTableLayouts;
    expectedTableLayouts["rs_Base"] = "p0";
    expectedTableLayouts["rs_Sub1"] = "p1 p11";
    expectedTableLayouts["ts_Sub2"] = "p2";
    AssertTableLayouts(ecdb, expectedTableLayouts);

    //verify that joined table option was resolved correctly. Need to look at the ec_ClassMap table directly to check that.
    std::map<ECClassId, MapStrategyInfo> expectedResults {
            {ecdb.Schemas().GetClassId("ReferredSchema","Base"), MapStrategyInfo(MapStrategyInfo::Strategy::TablePerHierarchy, MapStrategyInfo::JoinedTableInfo::ParentOfJoinedTable)},
            {ecdb.Schemas().GetClassId("ReferredSchema","Sub1"), MapStrategyInfo(MapStrategyInfo::Strategy::TablePerHierarchy, MapStrategyInfo::JoinedTableInfo::JoinedTable)},
            {ecdb.Schemas().GetClassId("TestSchema","Sub2"), MapStrategyInfo(MapStrategyInfo::Strategy::TablePerHierarchy, MapStrategyInfo::JoinedTableInfo::JoinedTable)},
            {ecdb.Schemas().GetClassId("TestSchema","Sub11"), MapStrategyInfo(MapStrategyInfo::Strategy::TablePerHierarchy, MapStrategyInfo::JoinedTableInfo::JoinedTable)}
        };

    for (std::pair<ECClassId, MapStrategyInfo> const& kvPair : expectedResults)
        {
        ECClassId classId = kvPair.first;
        MapStrategyInfo const& expectedMapStrategy = kvPair.second;
        MapStrategyInfo actualMapStrategy;

        ASSERT_TRUE(TryGetMapStrategyInfo(actualMapStrategy, ecdb, classId));
        ASSERT_EQ(expectedMapStrategy.m_strategy, actualMapStrategy.m_strategy);
        ASSERT_EQ(expectedMapStrategy.m_tphInfo, actualMapStrategy.m_tphInfo);
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         10/15
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableTestFixture, VerifyWhereClauseOptimization)
    {
    SchemaItem testSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' alias='dgn' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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

    ASSERT_EQ(SUCCESS, SetupECDb("JoinedTableTest.ecdb", testSchema));

    ECClassId gooClassId = m_ecdb.Schemas().GetClassId("JoinedTableTest", "Goo");
    ASSERT_TRUE(gooClassId.IsValid());
    //Only properties in joined table accessed should not unnecessarily add join to base table.
    {
    ECSqlStatement stmt;
    ASSERT_EQ(stmt.Prepare(m_ecdb, "UPDATE dgn.Goo SET C= :c, D= :d WHERE ECInstanceId = :id ECSQLOPTIONS NoECClassIdFilter"), ECSqlStatus::Success);
    ASSERT_STREQ("UPDATE [dgn_Goo] SET [C]=:c_col1,[D]=:d_col1 WHERE [FooId]=:id_col1", stmt.GetNativeSql());
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(stmt.Prepare(m_ecdb, "DELETE FROM dgn.Goo WHERE ECInstanceId = :id ECSQLOPTIONS NoECClassIdFilter "), ECSqlStatus::Success);
    ASSERT_STREQ("DELETE FROM [dgn_Foo] WHERE [Id]=:id_col1", stmt.GetNativeSql());
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(stmt.Prepare(m_ecdb, "UPDATE dgn.Goo SET C= :c, D= :d WHERE ECInstanceId = :id"), ECSqlStatus::Success);
    Utf8String expectedSql;
    expectedSql.Sprintf("UPDATE [dgn_Goo] SET [C]=:c_col1,[D]=:d_col1 WHERE [FooId]=:id_col1 AND (ECClassId IN (SELECT ClassId FROM ec_cache_ClassHierarchy WHERE BaseClassId=%s))",
    gooClassId.ToString().c_str());
    ASSERT_STREQ(expectedSql.c_str(), stmt.GetNativeSql()) << stmt.GetECSql();
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(stmt.Prepare(m_ecdb, "DELETE FROM dgn.Goo WHERE ECInstanceId = :id"), ECSqlStatus::Success);

    Utf8String expectedSql;
    expectedSql.Sprintf("DELETE FROM [dgn_Foo] WHERE [Id]=:id_col1 AND (ECClassId IN (SELECT ClassId FROM ec_cache_ClassHierarchy WHERE BaseClassId=%s))",
                        gooClassId.ToString().c_str());
    ASSERT_STREQ(expectedSql.c_str(), stmt.GetNativeSql()) << stmt.GetECSql();
    }

    //accessing property of parent should add join to base
    {
    ECSqlStatement stmt;
    ASSERT_EQ(stmt.Prepare(m_ecdb, "UPDATE dgn.Goo SET C= :c, D= :d WHERE ECInstanceId = :id AND A = :a ECSQLOPTIONS NoECClassIdFilter "), ECSqlStatus::Success);
    ASSERT_STREQ("UPDATE [dgn_Goo] SET [C]=:c_col1,[D]=:d_col1 WHERE InVirtualSet(:_ecdb_ecsqlparam_id_col1,[FooId])", stmt.GetNativeSql());
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(stmt.Prepare(m_ecdb, "DELETE FROM dgn.Goo WHERE  ECInstanceId = :id AND A = :a  ECSQLOPTIONS NoECClassIdFilter "), ECSqlStatus::Success);
    ASSERT_STREQ("DELETE FROM [dgn_Foo] WHERE [Id]=:id_col1 AND [A]=:a_col1", stmt.GetNativeSql());
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(stmt.Prepare(m_ecdb, "UPDATE dgn.Goo SET C= :c, D= :d WHERE ECInstanceId = :id AND A = :a"), ECSqlStatus::Success);
    ASSERT_STREQ("UPDATE [dgn_Goo] SET [C]=:c_col1,[D]=:d_col1 WHERE InVirtualSet(:_ecdb_ecsqlparam_id_col1,[FooId])", stmt.GetNativeSql()) << stmt.GetECSql();
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(stmt.Prepare(m_ecdb, "DELETE FROM dgn.Goo WHERE  ECInstanceId = :id AND A = :a"), ECSqlStatus::Success);

    Utf8String expectedSql;
    expectedSql.Sprintf("DELETE FROM [dgn_Foo] WHERE [Id]=:id_col1 AND [A]=:a_col1 AND (ECClassId IN (SELECT ClassId FROM ec_cache_ClassHierarchy WHERE BaseClassId=%s))",
                        gooClassId.ToString().c_str());
    ASSERT_STREQ(stmt.GetNativeSql(), expectedSql.c_str()) << stmt.GetECSql();
    }

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         10/15
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableTestFixture, InsertWithParameterBinding)
    {
    SchemaItem testSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' alias='dgn' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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

    ASSERT_EQ(SUCCESS, SetupECDb("JoinedTableTest.ecdb", testSchema));

    ECSqlStatement stmt;
    //-----------------------------INSERT----------------------------------------------------
    ASSERT_EQ(stmt.Prepare(m_ecdb, "INSERT INTO dgn.Goo (ECInstanceId, A, B, C, D ) VALUES ( :id, :a, :b, :c, :d)"), ECSqlStatus::Success);
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
    ASSERT_EQ(stmt.Prepare(m_ecdb, "SELECT A, B, C, D FROM dgn.Goo WHERE ECInstanceId = :id"), ECSqlStatus::Success);
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

    ASSERT_EQ(stmt.Prepare(m_ecdb, "UPDATE dgn.Goo SET A= :a, B= :b, C= :c, D= :d WHERE ECInstanceId = :id"), ECSqlStatus::Success);

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
    m_ecdb.SaveChanges();
    ASSERT_EQ(stmt.Prepare(m_ecdb, "SELECT A, B, C, D FROM dgn.Goo WHERE ECInstanceId = 101"), ECSqlStatus::Success);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

    ASSERT_EQ(stmt.GetValueInt64(0), 10001);
    ASSERT_STRCASEEQ(stmt.GetValueText(1), "a1001");
    ASSERT_EQ(stmt.GetValueInt64(2), 20001);
    ASSERT_STRCASEEQ(stmt.GetValueText(3), "d2001");
    stmt.Finalize();

    //-----------------------------DELETE----------------------------------------------------

    ASSERT_EQ(stmt.Prepare(m_ecdb, "DELETE FROM dgn.Goo WHERE ECInstanceId = :id"), ECSqlStatus::Success);

    idIndex = stmt.GetParameterIndex("id");
    ASSERT_EQ(idIndex, 1);
    auto bindR = stmt.BindInt64(idIndex, 101);
    ASSERT_EQ(ECSqlStatus::Success, bindR);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(stmt.Prepare(m_ecdb, "SELECT A, B, C, D FROM dgn.Goo WHERE ECInstanceId = 101"), ECSqlStatus::Success);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         10/15
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableTestFixture, InsertWithUnnamedParameterBinding)
    {
    SchemaItem testSchema("<?xml version='1.0' encoding='utf-8'?>"
                          "<ECSchema schemaName='JoinedTableTest' alias='dgn' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                          "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                          "    <ECEntityClass typeName='Foo' >"
                          "        <ECCustomAttributes>"
                          "            <ClassMap xmlns='ECDbMap.02.00'>"
                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                          "            </ClassMap>"
                          "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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

    ASSERT_EQ(SUCCESS, SetupECDb("JoinedTableTest.ecdb", testSchema));

    ECSqlStatement stmt;
    //-----------------------------INSERT----------------------------------------------------
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO dgn.Goo (ECInstanceId, A, B, C, D ) VALUES ( ?, ?, ?, ?, ?)"));

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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT A, B, C, D FROM dgn.Goo WHERE ECInstanceId = ?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 101));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);

    ASSERT_EQ(stmt.GetValueInt64(0), 10000);
    ASSERT_STRCASEEQ(stmt.GetValueText(1), "a1000");
    ASSERT_EQ(stmt.GetValueInt64(2), 20000);
    ASSERT_STRCASEEQ(stmt.GetValueText(3), "d2000");
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);

    stmt.Finalize();

    //-----------------------------UPDATE----------------------------------------------------

    ASSERT_EQ(stmt.Prepare(m_ecdb, "UPDATE dgn.Goo SET A= :a, B= :b, C= :c, D= :d WHERE ECInstanceId = ?"), ECSqlStatus::Success);

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 10001));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "a1001", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(3, 20001));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(4, "d2001", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(5, 101));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT A, B, C, D FROM dgn.Goo WHERE ECInstanceId = 101"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(10001, stmt.GetValueInt64(0));
    ASSERT_STRCASEEQ("a1001", stmt.GetValueText(1));
    ASSERT_EQ(20001, stmt.GetValueInt64(2));
    ASSERT_STRCASEEQ("d2001", stmt.GetValueText(3));
    stmt.Finalize();

    //-----------------------------DELETE----------------------------------------------------

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM dgn.Goo WHERE ECInstanceId = ?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(1, 101));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT A, B, C, D FROM dgn.Goo WHERE ECInstanceId = 101"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         10/15
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableTestFixture, AbstractBaseAndEmptyChildClass)
    {
    SchemaItem testSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' alias='dgn' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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

    ASSERT_EQ(SUCCESS, SetupECDb("JoinedTableTest.ecdb", testSchema));

    auto assert_ecsql = [this] (Utf8CP sql, ECSqlStatus expectedStatus, DbResult expectedStepStatus)
        {
        ECSqlStatement stmt;
        LOG.infov("Executing : %s", sql);
        ASSERT_EQ(stmt.Prepare(m_ecdb, sql), expectedStatus);
        LOG.infov("NativeSQL : %s", stmt.GetNativeSql());
        if (expectedStatus == ECSqlStatus::Success)
            {
            ASSERT_EQ(stmt.Step(), expectedStepStatus);
            }
        };
    auto assert_ecsql2 = [this] (Utf8CP sql, ECSqlStatus expectedStatus, int columnCountExpected, int rowCountExpected)
        {
        ECSqlStatement stmt;
        LOG.infov("Executing : %s", sql);
        ASSERT_EQ(stmt.Prepare(m_ecdb, sql), expectedStatus);
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
void JoinedTableTestFixture::AssertTableLayouts(ECDbCR ecdb, bmap<Utf8String, Utf8String> const& tableLayouts) const
    {
    for (bpair<Utf8String, Utf8String> const& kvPair : tableLayouts)
        {
        Utf8CP tableName = kvPair.first.c_str();
        Utf8CP expectedColNames = kvPair.second.c_str();

        Utf8String sql;
        sql.Sprintf("SELECT * FROM %s LIMIT 0", tableName);

        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, sql.c_str())) << "Expected table " << tableName << " does not exist. Error: " << ecdb.GetLastError().c_str();

        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
        const int actualColCount = stmt.GetColumnCount();
        bvector<Utf8String> actualColNameList;
        for (int i = 0; i < actualColCount; i++)
            {
            Utf8String actualColName(stmt.GetColumnName(i));
            actualColName.ToLower();
            if (actualColName.EndsWith("id") ||
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

        ASSERT_STREQ(expectedColNames, actualColNames.c_str()) << "Unexpected layout of table " << tableName;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String JoinedTableTestFixture::ToInsertECSql(ECDbCR ecdb, Utf8CP className)
    {
    ECN::ECClassCP ecClass = ecdb.Schemas().GetClass("JoinedTableTest", className);
    EXPECT_TRUE(ecClass != nullptr);

    Utf8String insertECSql = "INSERT INTO ";
    insertECSql.append(ecClass->GetECSqlName()).append(" (SourceECInstanceId, TargetECInstanceId, SourceECClassId, TargetECClassId) VALUES (%llu, %llu, %llu, %llu)");

    return insertECSql;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
ECInstanceId JoinedTableTestFixture::InsertTestInstance(ECDbCR ecdb, Utf8CP ecsql)
    {
    ECSqlStatement stmt;
    auto stat = stmt.Prepare(ecdb, ecsql);
    if (stat != ECSqlStatus::Success)
        {
        EXPECT_EQ(ECSqlStatus::Success, stat) << "Inserting test instance with '" << ecsql << "' failed. Preparation failed";
        return ECInstanceId();
        }

    ECInstanceKey newECInstanceKey;
    DbResult stepStat = stmt.Step(newECInstanceKey);
    if (stepStat != BE_SQLITE_DONE)
        {
        EXPECT_EQ(BE_SQLITE_DONE, stepStat) << "Inserting test instance with '" << ecsql << "' failed. Step failed";
        return ECInstanceId();
        }
    else
        return newECInstanceKey.GetInstanceId();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String JoinedTableTestFixture::ToSelectECSql(ECDbCR ecdb, Utf8CP className)
    {
    ECN::ECClassCP ecClass = ecdb.Schemas().GetClass("JoinedTableTest", className);
    EXPECT_TRUE(ecClass != nullptr);

    Utf8String selecteECSql = "SELECT ";
    selecteECSql.append(className).append(".* FROM ").append(ecClass->GetECSqlName()).append(" WHERE ECInstanceId = %llu");

    return selecteECSql;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  11/15
//+---------------+---------------+---------------+---------------+---------------+------
void JoinedTableTestFixture::VerifyInsertedInstance(ECDbR ecdb, Utf8CP ecsql, ECInstanceId sourceInstanceId, ECInstanceId targetInstanceId, ECClassId sourceClassId, ECClassId targetClassId)
    {
    ECSqlStatement stmt;

    auto stat = stmt.Prepare(ecdb, ecsql);
    if (stat != ECSqlStatus::Success)
        {
        ASSERT_EQ(ECSqlStatus::Success, stat) << "Select test instance with '" << ecsql << "' failed. Preparation failed";
        }

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Select test instance with '" << ecsql << "' failed. Step failed";
    ASSERT_EQ(sourceInstanceId.GetValue(), stmt.GetValueInt64(2)) << "Get Source InstanceId failed : " << ecsql;
    ASSERT_EQ(sourceClassId, stmt.GetValueId<ECClassId>(3)) << "Get SourceClassId failed : " << ecsql;
    ASSERT_EQ(targetInstanceId.GetValue(), stmt.GetValueInt64(4)) << "Get TargetInstanceId failed : " << ecsql;
    ASSERT_EQ(targetClassId, stmt.GetValueId<ECClassId>(5)) << "Get TargetClassId failed : " << ecsql;
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
TEST_F(JoinedTableTestFixture, SelfJoinRelationships)
    {
    SchemaItem testSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' alias='dgn' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "        <ECNavigationProperty propertyName ='Partner' relationshipName = 'FooHasFoo' direction='Backward' />"
        "        <ECNavigationProperty propertyName ='Parent' relationshipName = 'FooHasManyFoo' direction='Backward' />"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='FooHasFoo' strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Foo' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='FooHasManyFoo' strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Foo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='ManyFooHasManyFoo' strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Foo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("JoinedTableTest.ecdb", testSchema));

    ECClassId fooClassId = m_ecdb.Schemas().GetClassId("JoinedTableTest", "Foo");

    EC::ECInstanceId fooInstanceId1;
    EC::ECInstanceId fooInstanceId2;

    //Insert two Instances each Per class
    {
    fooInstanceId1 = InsertTestInstance(m_ecdb, "INSERT INTO dgn.Foo (A, B) VALUES(100001, 'Class Foo Instance 1')");
    fooInstanceId2 = InsertTestInstance(m_ecdb, "INSERT INTO dgn.Foo (A, B) VALUES(100002, 'Class Foo Instance 2')");

    if (!fooInstanceId1.IsValid() || !fooInstanceId2.IsValid())
        ASSERT_TRUE(false) << "Instance Id's not valid";
    }

    //Insert 1-1 Relationship
    {
    EC::ECInstanceId fooHasFooInstanceId1;
    Savepoint savePoint(m_ecdb, "1-1 Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(m_ecdb, "FooHasFoo").c_str(), fooInstanceId1.GetValue(), fooInstanceId1.GetValue(), fooClassId.GetValue(), fooClassId.GetValue());
    fooHasFooInstanceId1 = InsertTestInstance(m_ecdb, ecsql.c_str());

    savePoint.Commit();
    ecsql.Sprintf(ToSelectECSql(m_ecdb, "FooHasFoo").c_str(), fooHasFooInstanceId1.GetValue());
    VerifyInsertedInstance(m_ecdb, ecsql.c_str(), fooInstanceId1, fooInstanceId1, fooClassId, fooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId fooHasManyFooInstanceId1;
    EC::ECInstanceId fooHasManyFooInstanceId2;
    Savepoint savePoint(m_ecdb, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(m_ecdb, "FooHasManyFoo").c_str(), fooInstanceId1.GetValue(), fooInstanceId1.GetValue(), fooClassId.GetValue(), fooClassId.GetValue());
    fooHasManyFooInstanceId1 = InsertTestInstance(m_ecdb, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(m_ecdb, "FooHasManyFoo").c_str(), fooInstanceId1.GetValue(), fooInstanceId2.GetValue(), fooClassId.GetValue(), fooClassId.GetValue());
    fooHasManyFooInstanceId2 = InsertTestInstance(m_ecdb, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(m_ecdb, "FooHasManyFoo").c_str(), fooHasManyFooInstanceId1.GetValue());
    VerifyInsertedInstance(m_ecdb, ecsql.c_str(), fooInstanceId1, fooInstanceId1, fooClassId, fooClassId);

    ecsql.Sprintf(ToSelectECSql(m_ecdb, "FooHasManyFoo").c_str(), fooHasManyFooInstanceId2.GetValue());
    VerifyInsertedInstance(m_ecdb, ecsql.c_str(), fooInstanceId1, fooInstanceId2, fooClassId, fooClassId);
    }

    //Insert N-N Relationship
    {
    EC::ECInstanceId manyFooHasManyFooInstanceId1;
    EC::ECInstanceId manyFooHasManyFooInstanceId2;
    EC::ECInstanceId manyFooHasManyFooInstanceId3;
    EC::ECInstanceId manyFooHasManyFooInstanceId4;
    Savepoint savePoint(m_ecdb, "N-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(m_ecdb, "ManyFooHasManyFoo").c_str(), fooInstanceId1.GetValue(), fooInstanceId1.GetValue(), fooClassId.GetValue(), fooClassId.GetValue());
    manyFooHasManyFooInstanceId1 = InsertTestInstance(m_ecdb, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(m_ecdb, "ManyFooHasManyFoo").c_str(), fooInstanceId1.GetValue(), fooInstanceId2.GetValue(), fooClassId.GetValue(), fooClassId.GetValue());
    manyFooHasManyFooInstanceId2 = InsertTestInstance(m_ecdb, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(m_ecdb, "ManyFooHasManyFoo").c_str(), fooInstanceId2.GetValue(), fooInstanceId1.GetValue(), fooClassId.GetValue(), fooClassId.GetValue());
    manyFooHasManyFooInstanceId3 = InsertTestInstance(m_ecdb, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(m_ecdb, "ManyFooHasManyFoo").c_str(), fooInstanceId2.GetValue(), fooInstanceId2.GetValue(), fooClassId.GetValue(), fooClassId.GetValue());
    manyFooHasManyFooInstanceId4 = InsertTestInstance(m_ecdb, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(m_ecdb, "ManyFooHasManyFoo").c_str(), manyFooHasManyFooInstanceId1.GetValue());
    VerifyInsertedInstance(m_ecdb, ecsql.c_str(), fooInstanceId1, fooInstanceId1, fooClassId, fooClassId);

    ecsql.Sprintf(ToSelectECSql(m_ecdb, "ManyFooHasManyFoo").c_str(), manyFooHasManyFooInstanceId2.GetValue());
    VerifyInsertedInstance(m_ecdb, ecsql.c_str(), fooInstanceId1, fooInstanceId2, fooClassId, fooClassId);

    ecsql.Sprintf(ToSelectECSql(m_ecdb, "ManyFooHasManyFoo").c_str(), manyFooHasManyFooInstanceId3.GetValue());
    VerifyInsertedInstance(m_ecdb, ecsql.c_str(), fooInstanceId2, fooInstanceId1, fooClassId, fooClassId);

    ecsql.Sprintf(ToSelectECSql(m_ecdb, "ManyFooHasManyFoo").c_str(), manyFooHasManyFooInstanceId4.GetValue());
    VerifyInsertedInstance(m_ecdb, ecsql.c_str(), fooInstanceId2, fooInstanceId2, fooClassId, fooClassId);
    }

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
TEST_F(JoinedTableTestFixture, BaseAndDirectDerivedClassRelationship)
    {
    SchemaItem testSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' alias='dgn' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "        <ECNavigationProperty propertyName='Foo1' relationshipName='FooHasGoo' direction='Backward'/>"
        "        <ECNavigationProperty propertyName='Foo2' relationshipName='FooHasManyGoo' direction='Backward'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='FooHasGoo' strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..1)' roleLabel='references' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..1)' roleLabel='references' polymorphic='True'>"
        "      <Class class = 'Goo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='FooHasManyGoo' strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..1)' roleLabel='has' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' roleLabel='referred to by' polymorphic='True'>"
        "      <Class class = 'Goo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='ManyFooHasManyGoo' strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..*)' roleLabel='referred to by' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' roleLabel='referred to by' polymorphic='True'>"
        "      <Class class = 'Goo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("JoinedTableTest.ecdb", testSchema));

    ECClassId fooClassId = m_ecdb.Schemas().GetClassId("JoinedTableTest", "Foo");
    ECClassId gooClassId = m_ecdb.Schemas().GetClassId("JoinedTableTest", "Goo");

    EC::ECInstanceId fooInstanceId1;
    EC::ECInstanceId fooInstanceId2;
    EC::ECInstanceId gooInstanceId1;
    EC::ECInstanceId gooInstanceId2;

    //Insert Instances for Constraint classes of Relationships
    {
    fooInstanceId1 = InsertTestInstance(m_ecdb, "INSERT INTO dgn.Foo (A, B) VALUES(100001, 'Class Foo Instance 1')");
    fooInstanceId2 = InsertTestInstance(m_ecdb, "INSERT INTO dgn.Foo (A, B) VALUES(100002, 'Class Foo Instance 2')");
    gooInstanceId1 = InsertTestInstance(m_ecdb, "INSERT INTO dgn.Goo (C, D) VALUES(200001, 'Class Goo Instance 1')");
    gooInstanceId2 = InsertTestInstance(m_ecdb, "INSERT INTO dgn.Goo (C, D) VALUES(200002, 'Class Goo Instance 2')");

    if (!fooInstanceId1.IsValid() || !fooInstanceId2.IsValid() || !gooInstanceId1.IsValid() || !gooInstanceId2.IsValid())
        ASSERT_TRUE(false) << "Instance Id's not valid";
    }

    //Insert 1-1 Relationship
    {
    EC::ECInstanceId fooHasGooInstanceId1;
    Savepoint savePoint(m_ecdb, "1-1 Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(m_ecdb, "FooHasGoo").c_str(), fooInstanceId1.GetValue(), gooInstanceId1.GetValue(), fooClassId.GetValue(), gooClassId.GetValue());
    fooHasGooInstanceId1 = InsertTestInstance(m_ecdb, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(m_ecdb, "FooHasGoo").c_str(), fooHasGooInstanceId1.GetValue());
    VerifyInsertedInstance(m_ecdb, ecsql.c_str(), fooInstanceId1, gooInstanceId1, fooClassId, gooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId fooHasManyGooInstanceId1;
    EC::ECInstanceId fooHasManyGooInstanceId2;
    Savepoint savePoint(m_ecdb, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(m_ecdb, "FooHasManyGoo").c_str(), fooInstanceId1.GetValue(), gooInstanceId1.GetValue(), fooClassId.GetValue(), gooClassId.GetValue());
    fooHasManyGooInstanceId1 = InsertTestInstance(m_ecdb, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(m_ecdb, "FooHasManyGoo").c_str(), fooInstanceId1.GetValue(), gooInstanceId2.GetValue(), fooClassId.GetValue(), gooClassId.GetValue());
    fooHasManyGooInstanceId2 = InsertTestInstance(m_ecdb, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(m_ecdb, "FooHasManyGoo").c_str(), fooHasManyGooInstanceId1.GetValue());
    VerifyInsertedInstance(m_ecdb, ecsql.c_str(), fooInstanceId1, gooInstanceId1, fooClassId, gooClassId);

    ecsql.Sprintf(ToSelectECSql(m_ecdb, "FooHasManyGoo").c_str(), fooHasManyGooInstanceId2.GetValue());
    VerifyInsertedInstance(m_ecdb, ecsql.c_str(), fooInstanceId1, gooInstanceId2, fooClassId, gooClassId);
    }

    //Insert N-N Relationship
    {
    EC::ECInstanceId manyFooHasManyGooInstanceId1;
    EC::ECInstanceId manyFooHasManyGooInstanceId2;
    EC::ECInstanceId manyFooHasManyGooInstanceId3;
    EC::ECInstanceId manyFooHasManyGooInstanceId4;
    Savepoint savePoint(m_ecdb, "N-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(m_ecdb, "ManyFooHasManyGoo").c_str(), fooInstanceId1.GetValue(), gooInstanceId1.GetValue(), fooClassId.GetValue(), gooClassId.GetValue());
    manyFooHasManyGooInstanceId1 = InsertTestInstance(m_ecdb, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(m_ecdb, "ManyFooHasManyGoo").c_str(), fooInstanceId1.GetValue(), gooInstanceId2.GetValue(), fooClassId.GetValue(), gooClassId.GetValue());
    manyFooHasManyGooInstanceId2 = InsertTestInstance(m_ecdb, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(m_ecdb, "ManyFooHasManyGoo").c_str(), fooInstanceId2.GetValue(), gooInstanceId1.GetValue(), fooClassId.GetValue(), gooClassId.GetValue());
    manyFooHasManyGooInstanceId3 = InsertTestInstance(m_ecdb, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(m_ecdb, "ManyFooHasManyGoo").c_str(), fooInstanceId2.GetValue(), gooInstanceId2.GetValue(), fooClassId.GetValue(), gooClassId.GetValue());
    manyFooHasManyGooInstanceId4 = InsertTestInstance(m_ecdb, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(m_ecdb, "ManyFooHasManyGoo").c_str(), manyFooHasManyGooInstanceId1.GetValue());
    VerifyInsertedInstance(m_ecdb, ecsql.c_str(), fooInstanceId1, gooInstanceId1, fooClassId, gooClassId);

    ecsql.Sprintf(ToSelectECSql(m_ecdb, "ManyFooHasManyGoo").c_str(), manyFooHasManyGooInstanceId2.GetValue());
    VerifyInsertedInstance(m_ecdb, ecsql.c_str(), fooInstanceId1, gooInstanceId2, fooClassId, gooClassId);

    ecsql.Sprintf(ToSelectECSql(m_ecdb, "ManyFooHasManyGoo").c_str(), manyFooHasManyGooInstanceId3.GetValue());
    VerifyInsertedInstance(m_ecdb, ecsql.c_str(), fooInstanceId2, gooInstanceId1, fooClassId, gooClassId);

    ecsql.Sprintf(ToSelectECSql(m_ecdb, "ManyFooHasManyGoo").c_str(), manyFooHasManyGooInstanceId4.GetValue());
    VerifyInsertedInstance(m_ecdb, ecsql.c_str(), fooInstanceId2, gooInstanceId2, fooClassId, gooClassId);
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
TEST_F(JoinedTableTestFixture, RelationshipBetweenSubClasses)
    {
    SchemaItem testSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' alias='dgn' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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
        "        <ECNavigationProperty propertyName='Goo1' relationshipName='GooHasRoo' direction='Backward'/>"
        "        <ECNavigationProperty propertyName='Goo2' relationshipName='GooHasManyRoo' direction='Backward'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='GooHasRoo'  strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..1)' roleLabel='references' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Source>"
        "    <Target multiplicity='(0..1)' roleLabel='references' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='GooHasManyRoo' strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..1)' roleLabel='references' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' roleLabel='referenced by' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='ManyGooHasManyRoo' strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..*)' roleLabel='references' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' roleLabel='references' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("JoinedTableTest.ecdb", testSchema));
    ECDbR db = m_ecdb;
    db.SaveChanges();
    ECClassId gooClassId = db.Schemas().GetClassId("JoinedTableTest", "Goo");
    ECClassId rooClassId = db.Schemas().GetClassId("JoinedTableTest", "Roo");

    EC::ECInstanceId gooInstanceId1;
    EC::ECInstanceId gooInstanceId2;
    EC::ECInstanceId rooInstanceId1;
    EC::ECInstanceId rooInstanceId2;

    //Insert Instances for Constraint classes of Relationships
    {
    gooInstanceId1 = InsertTestInstance(db, "INSERT INTO dgn.Goo (C, D) VALUES(100001, 'Class Goo Instance 1')");
    gooInstanceId2 = InsertTestInstance(db, "INSERT INTO dgn.Goo (C, D) VALUES(100002, 'Class Goo Instance 2')");
    rooInstanceId1 = InsertTestInstance(db, "INSERT INTO dgn.Roo (G, H) VALUES(200001, 'Class Roo Instance 1')");
    rooInstanceId2 = InsertTestInstance(db, "INSERT INTO dgn.Roo (G, H) VALUES(200002, 'Class Roo Instance 2')");

    if (!gooInstanceId1.IsValid() || !gooInstanceId2.IsValid() || !rooInstanceId1.IsValid() || !rooInstanceId2.IsValid())
        ASSERT_TRUE(false) << "Instance Id's not valid";
    }

    //Insert 1-1 Relationship
    {
    EC::ECInstanceId gooHasRooInstanceId1;
    Savepoint savePoint(db, "1-1 Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(db, "GooHasRoo").c_str(), gooInstanceId1.GetValue(), rooInstanceId1.GetValue(), gooClassId.GetValue(), rooClassId.GetValue());
    gooHasRooInstanceId1 = InsertTestInstance(db, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(db, "GooHasRoo").c_str(), gooHasRooInstanceId1.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), gooInstanceId1, rooInstanceId1, gooClassId, rooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId gooHasManyRooInstanceId1;
    EC::ECInstanceId gooHasManyRooInstanceId2;
    Savepoint savePoint(db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(db, "GooHasManyRoo").c_str(), gooInstanceId1.GetValue(), rooInstanceId1.GetValue(), gooClassId.GetValue(), rooClassId.GetValue());
    gooHasManyRooInstanceId1 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "GooHasManyRoo").c_str(), gooInstanceId1.GetValue(), rooInstanceId2.GetValue(), gooClassId.GetValue(), rooClassId.GetValue());
    gooHasManyRooInstanceId2 = InsertTestInstance(db, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(db, "GooHasManyRoo").c_str(), gooHasManyRooInstanceId1.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), gooInstanceId1, rooInstanceId1, gooClassId, rooClassId);

    ecsql.Sprintf(ToSelectECSql(db, "GooHasManyRoo").c_str(), gooHasManyRooInstanceId2.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), gooInstanceId1, rooInstanceId2, gooClassId, rooClassId);
    }

    //Insert N-N Relationship
    {
    EC::ECInstanceId manyGooHasManyRooInstanceId1;
    EC::ECInstanceId manyGooHasManyRooInstanceId2;
    EC::ECInstanceId manyGooHasManyRooInstanceId3;
    EC::ECInstanceId manyGooHasManyRooInstanceId4;
    Savepoint savePoint(db, "N-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(db, "ManyGooHasManyRoo").c_str(), gooInstanceId1.GetValue(), rooInstanceId1.GetValue(), gooClassId.GetValue(), rooClassId.GetValue());
    manyGooHasManyRooInstanceId1 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "ManyGooHasManyRoo").c_str(), gooInstanceId1.GetValue(), rooInstanceId2.GetValue(), gooClassId.GetValue(), rooClassId.GetValue());
    manyGooHasManyRooInstanceId2 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "ManyGooHasManyRoo").c_str(), gooInstanceId2.GetValue(), rooInstanceId1.GetValue(), gooClassId.GetValue(), rooClassId.GetValue());
    manyGooHasManyRooInstanceId3 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "ManyGooHasManyRoo").c_str(), gooInstanceId2.GetValue(), rooInstanceId2.GetValue(), gooClassId.GetValue(), rooClassId.GetValue());
    manyGooHasManyRooInstanceId4 = InsertTestInstance(db, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(db, "ManyGooHasManyRoo").c_str(), manyGooHasManyRooInstanceId1.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), gooInstanceId1, rooInstanceId1, gooClassId, rooClassId);

    ecsql.Sprintf(ToSelectECSql(db, "ManyGooHasManyRoo").c_str(), manyGooHasManyRooInstanceId2.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), gooInstanceId1, rooInstanceId2, gooClassId, rooClassId);

    ecsql.Sprintf(ToSelectECSql(db, "ManyGooHasManyRoo").c_str(), manyGooHasManyRooInstanceId3.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), gooInstanceId2, rooInstanceId1, gooClassId, rooClassId);

    ecsql.Sprintf(ToSelectECSql(db, "ManyGooHasManyRoo").c_str(), manyGooHasManyRooInstanceId4.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), gooInstanceId2, rooInstanceId2, gooClassId, rooClassId);
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
TEST_F(JoinedTableTestFixture, RelationshipWithStandAloneClass)
    {
    SchemaItem testSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' alias='dgn' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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
        "        <ECNavigationProperty propertyName='Foo1' relationshipName='FooHasRoo' direction='Backward'/>"
        "        <ECNavigationProperty propertyName='Foo2' relationshipName='FooHasManyRoo' direction='Backward'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='FooHasRoo' strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='FooHasManyRoo' strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='referenced by'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='ManyFooHasManyRoo' strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("JoinedTableTest.ecdb", testSchema));
    ECDbR db = m_ecdb;

    ECClassId fooClassId = db.Schemas().GetClassId("JoinedTableTest", "Foo");
    ECClassId rooClassId = db.Schemas().GetClassId("JoinedTableTest", "Roo");

    EC::ECInstanceId fooInstanceId1;
    EC::ECInstanceId fooInstanceId2;
    EC::ECInstanceId rooInstanceId1;
    EC::ECInstanceId rooInstanceId2;

    //Insert Instances for Constraint classes of Relationships
    {
    fooInstanceId1 = InsertTestInstance(db, "INSERT INTO dgn.Foo (A, B) VALUES(100001, 'Class Foo Instance 1')");
    fooInstanceId2 = InsertTestInstance(db, "INSERT INTO dgn.Foo (A, B) VALUES(100002, 'Class Foo Instance 2')");
    rooInstanceId1 = InsertTestInstance(db, "INSERT INTO dgn.Roo (E, F) VALUES(200001, 'Class Roo Instance 1')");
    rooInstanceId2 = InsertTestInstance(db, "INSERT INTO dgn.Roo (E, F) VALUES(200002, 'Class Roo Instance 2')");

    if (!fooInstanceId1.IsValid() || !fooInstanceId2.IsValid() || !rooInstanceId1.IsValid() || !rooInstanceId2.IsValid())
        ASSERT_TRUE(false) << "Instance Id's not valid";
    }

    //Insert 1-1 Relationship
    {
    EC::ECInstanceId fooHasRooInstanceId1;
    Savepoint savePoint(db, "1-1 Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(db, "FooHasRoo").c_str(), fooInstanceId1.GetValue(), rooInstanceId1.GetValue(), fooClassId.GetValue(), rooClassId.GetValue());
    fooHasRooInstanceId1 = InsertTestInstance(db, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(db, "FooHasRoo").c_str(), fooHasRooInstanceId1.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), fooInstanceId1, rooInstanceId1, fooClassId, rooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId fooHasManyRooInstanceId1;
    EC::ECInstanceId fooHasManyRooInstanceId2;
    Savepoint savePoint(db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(db, "FooHasManyRoo").c_str(), fooInstanceId1.GetValue(), rooInstanceId1.GetValue(), fooClassId.GetValue(), rooClassId.GetValue());
    fooHasManyRooInstanceId1 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "FooHasManyRoo").c_str(), fooInstanceId1.GetValue(), rooInstanceId2.GetValue(), fooClassId.GetValue(), rooClassId.GetValue());
    fooHasManyRooInstanceId2 = InsertTestInstance(db, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(db, "FooHasManyRoo").c_str(), fooHasManyRooInstanceId1.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), fooInstanceId1, rooInstanceId1, fooClassId, rooClassId);

    ecsql.Sprintf(ToSelectECSql(db, "FooHasManyRoo").c_str(), fooHasManyRooInstanceId2.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), fooInstanceId1, rooInstanceId2, fooClassId, rooClassId);
    }

    //Insert N-N Relationship
    {
    EC::ECInstanceId manyFooHasManyRooInstanceId1;
    EC::ECInstanceId manyFooHasManyRooInstanceId2;
    EC::ECInstanceId manyFooHasManyRooInstanceId3;
    EC::ECInstanceId manyFooHasManyRooInstanceId4;
    Savepoint savePoint(db, "N-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(db, "ManyFooHasManyRoo").c_str(), fooInstanceId1.GetValue(), rooInstanceId1.GetValue(), fooClassId.GetValue(), rooClassId.GetValue());
    manyFooHasManyRooInstanceId1 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "ManyFooHasManyRoo").c_str(), fooInstanceId1.GetValue(), rooInstanceId2.GetValue(), fooClassId.GetValue(), rooClassId.GetValue());
    manyFooHasManyRooInstanceId2 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "ManyFooHasManyRoo").c_str(), fooInstanceId2.GetValue(), rooInstanceId1.GetValue(), fooClassId.GetValue(), rooClassId.GetValue());
    manyFooHasManyRooInstanceId3 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "ManyFooHasManyRoo").c_str(), fooInstanceId2.GetValue(), rooInstanceId2.GetValue(), fooClassId.GetValue(), rooClassId.GetValue());
    manyFooHasManyRooInstanceId4 = InsertTestInstance(db, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(db, "ManyFooHasManyRoo").c_str(), manyFooHasManyRooInstanceId1.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), fooInstanceId1, rooInstanceId1, fooClassId, rooClassId);

    ecsql.Sprintf(ToSelectECSql(db, "ManyFooHasManyRoo").c_str(), manyFooHasManyRooInstanceId2.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), fooInstanceId1, rooInstanceId2, fooClassId, rooClassId);

    ecsql.Sprintf(ToSelectECSql(db, "ManyFooHasManyRoo").c_str(), manyFooHasManyRooInstanceId3.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), fooInstanceId2, rooInstanceId1, fooClassId, rooClassId);

    ecsql.Sprintf(ToSelectECSql(db, "ManyFooHasManyRoo").c_str(), manyFooHasManyRooInstanceId4.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), fooInstanceId2, rooInstanceId2, fooClassId, rooClassId);
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
TEST_F(JoinedTableTestFixture, RelationshipWithStandAloneClass1)
    {
    SchemaItem testSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' alias='dgn' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "    <ECEntityClass typeName='Foo' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "        <ECNavigationProperty propertyName='Roo1' relationshipName='RooHasFoo' direction='Backward'/>"
        "        <ECNavigationProperty propertyName='Roo2' relationshipName='RooHasManyFoo' direction='Backward'/>"
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
        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Roo' />"
        "    </Source>"
        "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Foo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='RooHasManyFoo' strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Roo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='referenced by'>"
        "      <Class class = 'Foo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='ManyRooHasManyFoo' strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Roo' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Foo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("JoinedTableTest.ecdb", testSchema));
    ECDbR db = m_ecdb;

    ECClassId fooClassId = db.Schemas().GetClassId("JoinedTableTest", "Foo");
    ECClassId rooClassId = db.Schemas().GetClassId("JoinedTableTest", "Roo");

    EC::ECInstanceId fooInstanceId1;
    EC::ECInstanceId fooInstanceId2;
    EC::ECInstanceId rooInstanceId1;
    EC::ECInstanceId rooInstanceId2;

    //Insert Instances for Constraint classes of Relationships
    {
    fooInstanceId1 = InsertTestInstance(db, "INSERT INTO dgn.Foo (A, B) VALUES(100001, 'Class Foo Instance 1')");
    fooInstanceId2 = InsertTestInstance(db, "INSERT INTO dgn.Foo (A, B) VALUES(100002, 'Class Foo Instance 2')");
    rooInstanceId1 = InsertTestInstance(db, "INSERT INTO dgn.Roo (E, F) VALUES(200001, 'Class Roo Instance 1')");
    rooInstanceId2 = InsertTestInstance(db, "INSERT INTO dgn.Roo (E, F) VALUES(200002, 'Class Roo Instance 2')");

    if (!fooInstanceId1.IsValid() || !fooInstanceId2.IsValid() || !rooInstanceId1.IsValid() || !rooInstanceId2.IsValid())
        ASSERT_TRUE(false) << "Instance Id's not valid";
    }

    //Insert 1-1 Relationship
    {
    EC::ECInstanceId rooHasFooInstanceId1;
    Savepoint savePoint(db, "1-1 Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(db, "RooHasFoo").c_str(), rooInstanceId1.GetValue(), fooInstanceId1.GetValue(), rooClassId.GetValue(), fooClassId.GetValue());
    rooHasFooInstanceId1 = InsertTestInstance(db, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(db, "RooHasFoo").c_str(), rooHasFooInstanceId1.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), rooInstanceId1, fooInstanceId1, rooClassId, fooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId rooHasManyFooInstanceId1;
    EC::ECInstanceId rooHasManyFooInstanceId2;
    Savepoint savePoint(db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(db, "RooHasManyFoo").c_str(), rooInstanceId1.GetValue(), fooInstanceId1.GetValue(), rooClassId.GetValue(), fooClassId.GetValue());
    rooHasManyFooInstanceId1 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "RooHasManyFoo").c_str(), rooInstanceId1.GetValue(), fooInstanceId2.GetValue(), rooClassId.GetValue(), fooClassId.GetValue());
    rooHasManyFooInstanceId2 = InsertTestInstance(db, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(db, "RooHasManyFoo").c_str(), rooHasManyFooInstanceId1.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), rooInstanceId1, fooInstanceId1, rooClassId, fooClassId);

    ecsql.Sprintf(ToSelectECSql(db, "RooHasManyFoo").c_str(), rooHasManyFooInstanceId2.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), rooInstanceId1, fooInstanceId2, rooClassId, fooClassId);
    }

    //Insert N-N Relationship
    {
    EC::ECInstanceId manyRooHasManyFooInstanceId1;
    EC::ECInstanceId manyRooHasManyFooInstanceId2;
    EC::ECInstanceId manyRooHasManyFooInstanceId3;
    EC::ECInstanceId manyRooHasManyFooInstanceId4;
    Savepoint savePoint(db, "N-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(db, "ManyRooHasManyFoo").c_str(), rooInstanceId1.GetValue(), fooInstanceId1.GetValue(), rooClassId.GetValue(), fooClassId.GetValue());
    manyRooHasManyFooInstanceId1 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "ManyRooHasManyFoo").c_str(), rooInstanceId1.GetValue(), fooInstanceId2.GetValue(), rooClassId.GetValue(), fooClassId.GetValue());
    manyRooHasManyFooInstanceId2 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "ManyRooHasManyFoo").c_str(), rooInstanceId2.GetValue(), fooInstanceId1.GetValue(), rooClassId.GetValue(), fooClassId.GetValue());
    manyRooHasManyFooInstanceId3 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "ManyRooHasManyFoo").c_str(), rooInstanceId2.GetValue(), fooInstanceId2.GetValue(), rooClassId.GetValue(), fooClassId.GetValue());
    manyRooHasManyFooInstanceId4 = InsertTestInstance(db, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(db, "ManyRooHasManyFoo").c_str(), manyRooHasManyFooInstanceId1.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), rooInstanceId1, fooInstanceId1, rooClassId, fooClassId);

    ecsql.Sprintf(ToSelectECSql(db, "ManyRooHasManyFoo").c_str(), manyRooHasManyFooInstanceId2.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), rooInstanceId1, fooInstanceId2, rooClassId, fooClassId);

    ecsql.Sprintf(ToSelectECSql(db, "ManyRooHasManyFoo").c_str(), manyRooHasManyFooInstanceId3.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), rooInstanceId2, fooInstanceId1, rooClassId, fooClassId);

    ecsql.Sprintf(ToSelectECSql(db, "ManyRooHasManyFoo").c_str(), manyRooHasManyFooInstanceId4.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), rooInstanceId2, fooInstanceId2, rooClassId, fooClassId);
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
TEST_F(JoinedTableTestFixture, PolymorphicRelationshipWithStandAloneClass)
    {
    SchemaItem testSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' alias='dgn' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "   <ECEntityClass typeName='IFace' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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
        "        <ECNavigationProperty propertyName='Face1' relationshipName='IFaceHasBody' direction='Backward'/>"
        "        <ECNavigationProperty propertyName='Face2' relationshipName='IFaceHasManyBody' direction='Backward'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='IFaceHasBody' strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'IFace' />"
        "    </Source>"
        "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'Body' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='IFaceHasManyBody' strength='referencing' modifier='Sealed'>"
        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
        "      <Class class = 'IFace' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='referenced by'>"
        "      <Class class = 'Body' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ManyIFaceHaveManyBody' strength='referencing' modifier='Sealed'>"
        "       <Source multiplicity='(0..*)' polymorphic='True' roleLabel='references'>"
        "           <Class class = 'IFace' />"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='references'>"
        "           <Class class = 'Body' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("JoinedTableTest.ecdb", testSchema));
    ECDbR db = m_ecdb;

    ECClassId booClassId = db.Schemas().GetClassId("JoinedTableTest", "Boo");
    ECClassId gooClassId = db.Schemas().GetClassId("JoinedTableTest", "Goo");
    ECClassId bodyClassId = db.Schemas().GetClassId("JoinedTableTest", "Body");

    EC::ECInstanceId gooInstanceId1;
    EC::ECInstanceId gooInstanceId2;
    EC::ECInstanceId booInstanceId1;
    EC::ECInstanceId booInstanceId2;
    EC::ECInstanceId bodyInstanceId1;
    EC::ECInstanceId bodyInstanceId2;

    //Insert Instances for Constraint classes of Relationships
    {
    gooInstanceId1 = InsertTestInstance(db, "INSERT INTO dgn.Goo(Foo_L, Foo_S, Goo_L, Goo_S) VALUES(101, '::101', 102, '::102')");
    gooInstanceId2 = InsertTestInstance(db, "INSERT INTO dgn.Goo(Foo_L, Foo_S, Goo_L, Goo_S) VALUES(102, '::102', 103, '::103')");
    booInstanceId1 = InsertTestInstance(db, "INSERT INTO dgn.Boo(Foo_L, Foo_S, Boo_L, Boo_S) VALUES(201, '::201', 202, '::202')");
    booInstanceId2 = InsertTestInstance(db, "INSERT INTO dgn.Boo(Foo_L, Foo_S, Boo_L, Boo_S) VALUES(202, '::202', 203, '::203')");
    bodyInstanceId1 = InsertTestInstance(db, "INSERT INTO dgn.Body(Body_L, Body_S) VALUES(301, '::302')");
    bodyInstanceId2 = InsertTestInstance(db, "INSERT INTO dgn.Body(Body_L, Body_S) VALUES(302, '::303')");

    if (!gooInstanceId1.IsValid() || !gooInstanceId2.IsValid() || !booInstanceId1.IsValid() || !booInstanceId2.IsValid())
        ASSERT_TRUE(false) << "Instance Id's not valid";
    }

    //Insert 1-1 Relationship
    {
    EC::ECInstanceId IFaceHasBodyInstanceId1;
    EC::ECInstanceId IFaceHasBodyInstanceId2;
    Savepoint savePoint(db, "1-1 Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(db, "IFaceHasBody").c_str(), gooInstanceId1.GetValue(), bodyInstanceId1.GetValue(), gooClassId.GetValue(), bodyClassId.GetValue());
    IFaceHasBodyInstanceId1 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "IFaceHasBody").c_str(), booInstanceId1.GetValue(), bodyInstanceId2.GetValue(), booClassId.GetValue(), bodyClassId.GetValue());
    IFaceHasBodyInstanceId2 = InsertTestInstance(db, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(db, "IFaceHasBody").c_str(), IFaceHasBodyInstanceId1.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), gooInstanceId1, bodyInstanceId1, gooClassId, bodyClassId);

    ecsql.Sprintf(ToSelectECSql(db, "IFaceHasBody").c_str(), IFaceHasBodyInstanceId2.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), booInstanceId1, bodyInstanceId2, booClassId, bodyClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId IFaceHasManyBodyInstanceId1;
    EC::ECInstanceId IFaceHasManyBodyInstanceId2;
    Savepoint savePoint(db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(db, "IFaceHasManyBody").c_str(), gooInstanceId1.GetValue(), bodyInstanceId1.GetValue(), gooClassId.GetValue(), bodyClassId.GetValue());
    IFaceHasManyBodyInstanceId1 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "IFaceHasManyBody").c_str(), booInstanceId1.GetValue(), bodyInstanceId2.GetValue(), booClassId.GetValue(), bodyClassId.GetValue());
    IFaceHasManyBodyInstanceId2 = InsertTestInstance(db, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(db, "IFaceHasManyBody").c_str(), IFaceHasManyBodyInstanceId1.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), gooInstanceId1, bodyInstanceId1, gooClassId, bodyClassId);

    ecsql.Sprintf(ToSelectECSql(db, "IFaceHasManyBody").c_str(), IFaceHasManyBodyInstanceId2.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), booInstanceId1, bodyInstanceId2, booClassId, bodyClassId);
    }

    //Insert N-N Relationship
    {
    EC::ECInstanceId ManyIFaceHaveManyBodyInstanceId1;
    EC::ECInstanceId ManyIFaceHaveManyBodyInstanceId2;
    EC::ECInstanceId ManyIFaceHaveManyBodyInstanceId3;
    EC::ECInstanceId ManyIFaceHaveManyBodyInstanceId4;
    Savepoint savePoint(db, "N-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf(ToInsertECSql(db, "ManyIFaceHaveManyBody").c_str(), gooInstanceId1.GetValue(), bodyInstanceId1.GetValue(), gooClassId.GetValue(), bodyClassId.GetValue());
    ManyIFaceHaveManyBodyInstanceId1 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "ManyIFaceHaveManyBody").c_str(), gooInstanceId2.GetValue(), bodyInstanceId2.GetValue(), gooClassId.GetValue(), bodyClassId.GetValue());
    ManyIFaceHaveManyBodyInstanceId2 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "ManyIFaceHaveManyBody").c_str(), booInstanceId1.GetValue(), bodyInstanceId1.GetValue(), booClassId.GetValue(), bodyClassId.GetValue());
    ManyIFaceHaveManyBodyInstanceId3 = InsertTestInstance(db, ecsql.c_str());

    ecsql.Sprintf(ToInsertECSql(db, "ManyIFaceHaveManyBody").c_str(), booInstanceId2.GetValue(), bodyInstanceId2.GetValue(), booClassId.GetValue(), bodyClassId.GetValue());
    ManyIFaceHaveManyBodyInstanceId4 = InsertTestInstance(db, ecsql.c_str());

    savePoint.Commit();

    ecsql.Sprintf(ToSelectECSql(db, "ManyIFaceHaveManyBody").c_str(), ManyIFaceHaveManyBodyInstanceId1.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), gooInstanceId1, bodyInstanceId1, gooClassId, bodyClassId);

    ecsql.Sprintf(ToSelectECSql(db, "ManyIFaceHaveManyBody").c_str(), ManyIFaceHaveManyBodyInstanceId2.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), gooInstanceId2, bodyInstanceId2, gooClassId, bodyClassId);

    ecsql.Sprintf(ToSelectECSql(db, "ManyIFaceHaveManyBody").c_str(), ManyIFaceHaveManyBodyInstanceId3.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), booInstanceId1, bodyInstanceId1, booClassId, bodyClassId);

    ecsql.Sprintf(ToSelectECSql(db, "ManyIFaceHaveManyBody").c_str(), ManyIFaceHaveManyBodyInstanceId4.GetValue());
    VerifyInsertedInstance(db, ecsql.c_str(), booInstanceId2, bodyInstanceId2, booClassId, bodyClassId);
    }
    db.Schemas().CreateClassViewsInDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableTestFixture, DropFKConstraintForSharedColumnForSubClasses)
    {
    SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='A'>"
                        "        <ECProperty propertyName='AName' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B'>"
                        "     <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                        "        </ClassMap>"
                        "        <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                        "     </ECCustomAttributes>"
                        "        <ECProperty propertyName='BName' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B1Sub'>"
                        "     <ECCustomAttributes>"
                        "            <ShareColumns xmlns='ECDbMap.02.00'>"
                        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                        "            </ShareColumns>"
                        "     </ECCustomAttributes>"
                        "        <BaseClass>B</BaseClass>"
                        "        <ECProperty propertyName='B1SubName' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B1SubSub'>"
                        "        <BaseClass>B1Sub</BaseClass>"
                        "        <ECProperty propertyName='B1SubSubName' typeName='string' />"
                        "        <ECProperty propertyName='AId' typeName='long' />"
                        "        <ECNavigationProperty propertyName='A' relationshipName='Rel1' direction='Backward'>"
                        "           <ECCustomAttributes>"
                        "                <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                        "                   <OnDeleteAction>Restrict</OnDeleteAction>"
                        "                </ForeignKeyConstraint>"
                        "           </ECCustomAttributes>"
                        "         </ECNavigationProperty>"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='Rel1' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B1SubSub'/>"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("fkconstraintsonsharedcolumnsforsubclasses.ecdb", testItem));
    ECDbR ecdb = m_ecdb;

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
    statement.BindId(1, sourceKey.GetInstanceId());
    statement.BindId(2, targetKey.GetInstanceId());
    statement.BindId(3, sourceKey.GetClassId());
    statement.BindId(4, targetKey.GetClassId());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step(relKey));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "SELECT SourceECInstanceId, TargetECInstanceId FROM ts.Rel1 WHERE ECInstanceId = ?"));
    statement.BindId(1, relKey.GetInstanceId());
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(sourceKey.GetInstanceId().GetValue(), statement.GetValueInt64(0));
    ASSERT_EQ(targetKey.GetInstanceId().GetValue(), statement.GetValueInt64(1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableTestFixture, VerifyONDeleteRestrictWithJoinedTable)
    {
    SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='A'>"
                        "        <ECProperty propertyName='A_prop' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B'>"
                        "      <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                        "        </ClassMap>"
                        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                        "     </ECCustomAttributes>"
                        "        <ECProperty propertyName='B_prop' typeName='string' />"
                        "        <ECNavigationProperty propertyName='A1' relationshipName='AOwnsB' direction='Backward'>"
                        "           <ECCustomAttributes>"
                        "                <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                        "                   <OnDeleteAction>Cascade</OnDeleteAction>"
                        "                </ForeignKeyConstraint>"
                        "           </ECCustomAttributes>"
                        "         </ECNavigationProperty>"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B1'>"
                        "        <BaseClass>B</BaseClass>"
                        "        <ECProperty propertyName='B1_prop' typeName='string' />"
                        "        <ECNavigationProperty propertyName='A2' relationshipName='AOwnsB1' direction='Backward'>"
                        "           <ECCustomAttributes>"
                        "                <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                        "                   <OnDeleteAction>Restrict</OnDeleteAction>"
                        "                </ForeignKeyConstraint>"
                        "           </ECCustomAttributes>"
                        "         </ECNavigationProperty>"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='AOwnsB' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B'/>"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  <ECRelationshipClass typeName='AOwnsB1' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B1'/>"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  </ECSchema>");

    ASSERT_EQ(SUCCESS, SetupECDb("ondeleterestrictforjoinedtable.ecdb", testItem));
    ECDbR ecdb = m_ecdb;

    ECInstanceKey sourceKey;
    ECInstanceKey targetKey;

    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.A VALUES('A1')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step(sourceKey));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.B(B_Prop) VALUES('B1')"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step(targetKey));
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.AOwnsB(SourceECInstanceId, TargetECInstanceId, SourceECClassId, TargetECClassId) VALUES(?,?,?,?)"));
    statement.BindId(1, sourceKey.GetInstanceId());
    statement.BindId(2, targetKey.GetInstanceId());
    statement.BindId(3, sourceKey.GetClassId());
    statement.BindId(4, targetKey.GetClassId());
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
    statement.BindId(1, sourceKey1.GetInstanceId());
    statement.BindId(2, targetKey1.GetInstanceId());
    statement.BindId(3, sourceKey1.GetClassId());
    statement.BindId(4, targetKey1.GetClassId());
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "DELETE FROM ts.A WHERE ECInstanceId = ?"));
    statement.BindId(1, sourceKey.GetInstanceId());
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Reset();
    statement.ClearBindings();

    statement.BindId(1, sourceKey1.GetInstanceId());
    ASSERT_EQ(DbResult::BE_SQLITE_CONSTRAINT_TRIGGER, statement.Step());
    statement.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
struct JoinedTableECSqlStatementsTests : DbMappingTestFixture
    {
    void ImportSchemaWithCA(ECSchemaPtr& ecSchema, Utf8CP className);

    void SetUpECSqlStatementTestsDb();

    void SetUpNestedStructArrayDb();
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
void JoinedTableECSqlStatementsTests::ImportSchemaWithCA(ECSchemaPtr& ecSchema, Utf8CP className)
    {
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    SchemaKey ecdbmapKey = SchemaKey("ECDbMap", 2, 0);
    ECSchemaPtr ecdbmapSchema = readContext->LocateSchema(ecdbmapKey, SchemaMatchType::LatestWriteCompatible);
    ASSERT_TRUE(ecdbmapSchema.IsValid());
    readContext->AddSchema(*ecSchema);
    ecSchema->AddReferencedSchema(*ecdbmapSchema);

    ECClassP personClass = ecSchema->GetClassP(className);
    ASSERT_TRUE(personClass != nullptr);

    ECClassCP ca = ecdbmapSchema->GetClassCP("ClassMap");
    EXPECT_TRUE(ca != nullptr);
    auto customAttribute = ca->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_TRUE(customAttribute != nullptr);
    ASSERT_TRUE(customAttribute->SetValue("MapStrategy", ECValue("TablePerHierarchy")) == ECObjectsStatus::Success);
    ASSERT_TRUE(personClass->SetCustomAttribute(*customAttribute) == ECObjectsStatus::Success);

    ca = ecdbmapSchema->GetClassCP("JoinedTablePerDirectSubclass");
    EXPECT_TRUE(ca != nullptr);
    customAttribute = ca->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_TRUE(customAttribute != nullptr);
    ASSERT_TRUE(personClass->SetCustomAttribute(*customAttribute) == ECObjectsStatus::Success);

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
void JoinedTableECSqlStatementsTests::SetUpECSqlStatementTestsDb()
    {
    SetupECDb("JoinedTableECSqlStatementTests.ecdb");

    ECSchemaReadContextPtr context = nullptr;
    ECSchemaPtr schemaPtr = ReadECSchemaFromDisk(context, BeFileName(L"ECSqlStatementTests.01.00.ecschema.xml"));
    ASSERT_TRUE(schemaPtr != NULL);

    ImportSchemaWithCA(schemaPtr, "Person");

    NestedStructArrayTestSchemaHelper::PopulateECSqlStatementTestsDb(m_ecdb);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
void JoinedTableECSqlStatementsTests::SetUpNestedStructArrayDb()
    {
    SetupECDb("JoinedTableECSqlStatementTests.ecdb");

    ECSchemaReadContextPtr context = nullptr;
    ECSchemaPtr schemaPtr = ReadECSchemaFromDisk(context, BeFileName(L"NestedStructArrayTest.01.00.ecschema.xml"));
    ASSERT_TRUE(schemaPtr != NULL);

    ImportSchemaWithCA(schemaPtr, "ClassA");

    NestedStructArrayTestSchemaHelper::PopulateNestedStructArrayDb(m_ecdb, true);
    }

struct PowSqlFunction : ScalarFunction
    {
    private:

        void _ComputeScalar(Context& ctx, int nArgs, DbValue* args) override
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
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "UPDATE ECST.Customer Set Phone=10000, PersonName.FirstName='Jones' WHERE PersonName.FirstName='Charles' AND PersonName.LastName='Baron'"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    //verify Updated Instance
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT Phone FROM ECST.Person WHERE PersonName.FirstName='Jones' AND PersonName.LastName='Baron'"));
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
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM ECST.Person WHERE PersonName.FirstName='Charles' AND PersonName.LastName='Baron'"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    //Verify Delete
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "SELECT * FROM ECST.Person WHERE PersonName.FirstName='Charles' AND PersonName.LastName='Baron'"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECSqlStatementsTests, PersistSqlForQueryOnAbstractBaseClass)
    {
    SetUpECSqlStatementTestsDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM ECST.Person"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_STREQ("SELECT [Person].[ECInstanceId] FROM (SELECT [Id] ECInstanceId,[ECClassId] FROM [ECST_Person]) [Person]", stmt.GetNativeSql());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECSqlStatementsTests, UnionTests)
    {
    SetUpECSqlStatementTestsDb();
    m_ecdb.SaveChanges();
    int rowCount;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM (SELECT CompanyName FROM ECST.Supplier UNION ALL SELECT CompanyName FROM ECST.Shipper)"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
    int count = stmt.GetValueInt(0);
    EXPECT_EQ(6, count);
    stmt.Finalize();

    //Select Statement containing Union All Clause and also Order By clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CompanyName, Phone FROM ECST.Supplier UNION ALL SELECT CompanyName, Phone FROM ECST.Shipper ORDER BY Phone"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT City FROM ECST.Supplier UNION SELECT City FROM ECST.Customer ORDER BY City"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT City FROM ECST.Supplier UNION ALL SELECT City FROM ECST.Customer ORDER BY City"));
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
    ASSERT_EQ(0, m_ecdb.AddFunction(func));
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "Select POW(ECInstanceId, 2), ECClassId, ECInstanceId From ECST.Supplier UNION ALL Select POW(ECInstanceId, 2), ECClassId, ECInstanceId From ECST.Customer ORDER BY ECInstanceId"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Count(*), AVG(Phone) FROM (SELECT Phone FROM ECST.Supplier UNION ALL SELECT Phone FROM ECST.Customer)"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_ROW);
    ASSERT_EQ(7, stmt.GetValueInt(0));
    ASSERT_EQ(1400, stmt.GetValueInt(1));
    stmt.Finalize();

    //Use GROUP BY clause in Union Query
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*), Phone FROM (SELECT ECClassId, Phone FROM ECST.Supplier UNION ALL SELECT ECClassId, Phone FROM ECST.Customer) GROUP BY ECClassId ORDER BY Phone"));

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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE nsat.ClassA SET T='UpdatedValue', I=2"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, I,T FROM nsat.ClassA"));
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

    ASSERT_TRUE(m_ecdb.TableExists("nsat_DerivedA"));
    ASSERT_TRUE(m_ecdb.TableExists("nsat_DerivedB"));
    ASSERT_FALSE(m_ecdb.TableExists("nsat_DoubleDerivedA"));
    ASSERT_FALSE(m_ecdb.TableExists("nsat_DoubleDerivedC"));

    //Delete all Instances of the base class, all the structArrays and relationships should also be deleted.
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "DELETE FROM nsat.ClassA"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();
    m_ecdb.SaveChanges();

    bvector<Utf8String> tableNames = {"ClassA", "DerivedA", "DerivedB", "BaseHasDerivedA", "DerivedBHasChildren"};

    for (Utf8StringCR tableName : tableNames)
        {
        Utf8String selectSql = "SELECT count(*) FROM nsat_";
        selectSql.append(tableName);
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, selectSql.c_str())) << "Prepare failed for " << selectSql.c_str();
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CompanyName FROM ECST.Supplier EXCEPT SELECT CompanyName FROM ECST.Shipper"));
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

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ContactTitle FROM ECST.Customer EXCEPT SELECT ContactTitle FROM ECST.Supplier"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CompanyName FROM ECST.Supplier INTERSECT SELECT CompanyName FROM ECST.Shipper ORDER BY CompanyName"));
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

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ContactTitle FROM ECST.Supplier INTERSECT SELECT ContactTitle FROM ECST.Customer ORDER BY ContactTitle"));
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
TEST_F(JoinedTableTestFixture, JoinedTableForClassesWithoutBusinessProperties)
    {
    SchemaItem testSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='BaseClass' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
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

    ASSERT_EQ(SUCCESS, SetupECDb("ClassesWithoutBusinessProperties.ecdb", testSchema));

    //Verify that only one base table and one Secondary Table have been created, rest of the tables will not be mapped as joined table because they don't have business properties.
    Statement statement;
    ASSERT_EQ(BE_SQLITE_OK, statement.Prepare(m_ecdb, "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name LIKE 'dgn_%'"));
    ASSERT_EQ(BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(4, statement.GetValueInt(0));
    }

END_ECDBUNITTESTS_NAMESPACE

