/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbJoinedTable_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "SchemaImportTestFixture.h"
#include "ECSqlStatementTestsSchemaHelper.h"
#include <initializer_list>
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
        Utf8CP ToInsertECSql (ECDbCR ecdb, Utf8CP className);
        Utf8CP ToSelectECSql (ECDbCR ecdb, Utf8CP className);
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
    std::vector<SchemaItem> testSchemas {
            SchemaItem("JoinedTablePerDirectSubclass on Root",
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
                "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
                "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='Foo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                "    </ECClass>"
                "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                "        <BaseClass>Foo</BaseClass>"
                "        <ECProperty propertyName='C' typeName='long'/>"
                "        <ECProperty propertyName='D' typeName='string'/>"
                "    </ECClass>"
                "   <ECClass typeName='Boo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                "        <BaseClass>Foo</BaseClass>"
                "        <ECProperty propertyName='E' typeName='long'/>"
                "        <ECProperty propertyName='F' typeName='string'/>"
                "    </ECClass>"
                "   <ECClass typeName='Roo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                "        <BaseClass>Boo</BaseClass>"
                "        <ECProperty propertyName='G' typeName='long'/>"
                "        <ECProperty propertyName='H' typeName='string'/>"
                "    </ECClass>"
                "</ECSchema>"),

    SchemaItem("JoinedTablePerDirectSubclass on Root and SharedColumnsForSubclasses",
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
                "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
                "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='Foo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                "    </ECClass>"
                "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                "        <BaseClass>Foo</BaseClass>"
                "        <ECProperty propertyName='C' typeName='long'/>"
                "        <ECProperty propertyName='D' typeName='string'/>"
                "    </ECClass>"
                "   <ECClass typeName='Boo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                "        <BaseClass>Foo</BaseClass>"
                "        <ECProperty propertyName='E' typeName='long'/>"
                "        <ECProperty propertyName='F' typeName='string'/>"
                "    </ECClass>"
                "   <ECClass typeName='Roo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                "        <BaseClass>Boo</BaseClass>"
                "        <ECProperty propertyName='G' typeName='long'/>"
                "        <ECProperty propertyName='H' typeName='string'/>"
                "    </ECClass>"
                "</ECSchema>"),

            SchemaItem("JoinedTablePerDirectSubclass on single direct subclass",
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
                "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
                "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='Foo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                "    </ECClass>"
                "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                "        <BaseClass>Foo</BaseClass>"
                "        <ECProperty propertyName='C' typeName='long'/>"
                "        <ECProperty propertyName='D' typeName='string'/>"
                "    </ECClass>"
                "   <ECClass typeName='Boo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                "    </ECClass>"
                "   <ECClass typeName='Roo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                "        <BaseClass>Boo</BaseClass>"
                "        <ECProperty propertyName='G' typeName='long'/>"
                "        <ECProperty propertyName='H' typeName='string'/>"
                "    </ECClass>"
                "</ECSchema>"),

    SchemaItem("JoinedTablePerDirectSubclass on single direct subclass and SharedColumnsForSubclasses on Root",
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
                "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
                "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='Foo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                "    </ECClass>"
                "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                "        <BaseClass>Foo</BaseClass>"
                "        <ECProperty propertyName='C' typeName='long'/>"
                "        <ECProperty propertyName='D' typeName='string'/>"
                "    </ECClass>"
                "   <ECClass typeName='Boo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                "    </ECClass>"
                "   <ECClass typeName='Roo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                "        <BaseClass>Boo</BaseClass>"
                "        <ECProperty propertyName='G' typeName='long'/>"
                "        <ECProperty propertyName='H' typeName='string'/>"
                "    </ECClass>"
                "</ECSchema>"),

            SchemaItem("JoinedTablePerDirectSubclass on both subclasses",
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
                "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
                "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='Foo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                "    </ECClass>"
                "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                "    </ECClass>"
                "   <ECClass typeName='Boo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                "    </ECClass>"
                "   <ECClass typeName='Roo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                "        <BaseClass>Boo</BaseClass>"
                "        <ECProperty propertyName='G' typeName='long'/>"
                "        <ECProperty propertyName='H' typeName='string'/>"
                "    </ECClass>"
                "</ECSchema>") ,

            SchemaItem("JoinedTablePerDirectSubclass on both subclasses and shared columns",
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
                "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
                "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='Foo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                "    </ECClass>"
                "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                "    </ECClass>"
                "   <ECClass typeName='Boo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                "    </ECClass>"
                "   <ECClass typeName='Roo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                "        <BaseClass>Boo</BaseClass>"
                "        <ECProperty propertyName='G' typeName='long'/>"
                "        <ECProperty propertyName='H' typeName='string'/>"
                "    </ECClass>"
                "</ECSchema>")
        };

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
        "<ECSchema schemaName='ReferredSchema' nameSpacePrefix='rs' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Base' isDomainClass='False'>"
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
        "    </ECClass>"
        "    <ECClass typeName='Sub1' isDomainClass='True'>"
        "         <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECClass>"
        "</ECSchema>", true, "Mapstrategy Option JoinedTablePerDirectSubclass (applied to subclasses) is expected to succeed");

    SchemaItem secondTestItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECSchemaReference name='ReferredSchema' version='01.00' prefix='rs' />"
        "    <ECClass typeName='Sub2' isDomainClass='True'>"
        "         <BaseClass>rs:Base</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='Sub11' isDomainClass='True'>"
        "         <BaseClass>rs:Sub1</BaseClass>"
        "        <ECProperty propertyName='P11' typeName='int' />"
        "    </ECClass>"
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
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Foo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
        "    </ECClass>"
        "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECClass>"
        "   <ECClass typeName='Boo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECClass>"
        "   <ECClass typeName='Roo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Boo</BaseClass>"
        "        <ECProperty propertyName='G' typeName='long'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "    </ECClass>"
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
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo'>"
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
        "   <ECEntityClass typeName='Goo'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Boo'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Roo'>"
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
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Foo' isDomainClass='False' isStruct='False' isCustomAttributeClass='False'>"
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
        "    </ECClass>"
        "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Foo</BaseClass>"
        "    </ECClass>"
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
            if (actualColName.Equals("ecinstanceid") ||
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
Utf8CP JoinedTableECDbMapStrategyTests::ToInsertECSql (ECDbCR ecdb, Utf8CP className)
    {
    ECN::ECClassCP ecClass = ecdb.Schemas ().GetECClass ("JoinedTableTest", className);
    EXPECT_TRUE (ecClass != nullptr);

    Utf8String insertECSql = "INSERT INTO ";
    insertECSql.append (ecClass->GetECSqlName()).append (" (SourceECInstanceId, TargetECInstanceId, SourceECClassId, TargetECClassId) VALUES (%lld, %lld, %lld, %lld)");

    return insertECSql.c_str ();
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
Utf8CP JoinedTableECDbMapStrategyTests::ToSelectECSql (ECDbCR ecdb, Utf8CP className)
    {
    ECN::ECClassCP ecClass = ecdb.Schemas ().GetECClass ("JoinedTableTest", className);
    EXPECT_TRUE (ecClass != nullptr);

    Utf8String selecteECSql = "SELECT ";
    selecteECSql.append (className).append (".* FROM ").append (ecClass->GetECSqlName()).append (" WHERE ECInstanceId = %lld");

    return selecteECSql.c_str ();
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
    ASSERT_EQ (sourceClassId, stmt.GetValueInt64 (2)) << "Get SourceClassId failed : " << ecsql;
    ASSERT_EQ (targetInstanceId.GetValue (), stmt.GetValueInt64 (3)) << "Get TargetInstanceId failed : " << ecsql;
    ASSERT_EQ (targetClassId, stmt.GetValueInt64 (4)) << "Get TargetClassId failed : " << ecsql;
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
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Foo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
        "    </ECClass>"
        "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECClass>"
        "    <ECRelationshipClass typeName='FooHasFoo' isDomainClass='True' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(1,1)' polymorphic='True'>"
        "      <Class class = 'Foo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='FooHasManyFoo' isDomainClass='True' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Foo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='ManyFooHasManyFoo' isDomainClass='True' strength='referencing'>"
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
    ecsql.Sprintf (ToInsertECSql (db, "FooHasFoo"), fooInstanceId1.GetValue (), fooInstanceId1.GetValue (), fooClassId, fooClassId);
    fooHasFooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasFoo"), fooHasFooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, fooInstanceId1, fooClassId, fooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId fooHasManyFooInstanceId1;
    EC::ECInstanceId fooHasManyFooInstanceId2;
    Savepoint savePoint (db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyFoo"), fooInstanceId1.GetValue (), fooInstanceId1.GetValue (), fooClassId, fooClassId);
    fooHasManyFooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyFoo"), fooInstanceId1.GetValue (), fooInstanceId2.GetValue (), fooClassId, fooClassId);
    fooHasManyFooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyFoo"), fooHasManyFooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, fooInstanceId1, fooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyFoo"), fooHasManyFooInstanceId2.GetValue ());
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
    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyFoo"), fooInstanceId1.GetValue (), fooInstanceId1.GetValue (), fooClassId, fooClassId);
    manyFooHasManyFooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyFoo"), fooInstanceId1.GetValue (), fooInstanceId2.GetValue (), fooClassId, fooClassId);
    manyFooHasManyFooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyFoo"), fooInstanceId2.GetValue (), fooInstanceId1.GetValue (), fooClassId, fooClassId);
    manyFooHasManyFooInstanceId3 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyFoo"), fooInstanceId2.GetValue (), fooInstanceId2.GetValue (), fooClassId, fooClassId);
    manyFooHasManyFooInstanceId4 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyFoo"), manyFooHasManyFooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, fooInstanceId1, fooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyFoo"), manyFooHasManyFooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, fooInstanceId2, fooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyFoo"), manyFooHasManyFooInstanceId3.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId2, fooInstanceId1, fooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyFoo"), manyFooHasManyFooInstanceId4.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId2, fooInstanceId2, fooClassId, fooClassId);
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
TEST_F (JoinedTableECDbMapStrategyTests, BaseAndDirectDerivedClassRelationship)
    {
    SchemaItem testSchema (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Foo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
        "    </ECClass>"
        "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECClass>"
        "    <ECRelationshipClass typeName='FooHasGoo' isDomainClass='True' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(1,1)' polymorphic='True'>"
        "      <Class class = 'Goo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='FooHasManyGoo' isDomainClass='True' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Goo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='ManyFooHasManyGoo' isDomainClass='True' strength='referencing'>"
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
    ecsql.Sprintf (ToInsertECSql (db, "FooHasGoo"), fooInstanceId1.GetValue (), gooInstanceId1.GetValue (), fooClassId, gooClassId);
    fooHasGooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasGoo"), fooHasGooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, gooInstanceId1, fooClassId, gooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId fooHasManyGooInstanceId1;
    EC::ECInstanceId fooHasManyGooInstanceId2;
    Savepoint savePoint (db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyGoo"), fooInstanceId1.GetValue (), gooInstanceId1.GetValue (), fooClassId, gooClassId);
    fooHasManyGooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyGoo"), fooInstanceId1.GetValue (), gooInstanceId2.GetValue (), fooClassId, gooClassId);
    fooHasManyGooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyGoo"), fooHasManyGooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, gooInstanceId1, fooClassId, gooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyGoo"), fooHasManyGooInstanceId2.GetValue ());
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
    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyGoo"), fooInstanceId1.GetValue (), gooInstanceId1.GetValue (), fooClassId, gooClassId);
    manyFooHasManyGooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyGoo"), fooInstanceId1.GetValue (), gooInstanceId2.GetValue (), fooClassId, gooClassId);
    manyFooHasManyGooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyGoo"), fooInstanceId2.GetValue (), gooInstanceId1.GetValue (), fooClassId, gooClassId);
    manyFooHasManyGooInstanceId3 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyGoo"), fooInstanceId2.GetValue (), gooInstanceId2.GetValue (), fooClassId, gooClassId);
    manyFooHasManyGooInstanceId4 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyGoo"), manyFooHasManyGooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, gooInstanceId1, fooClassId, gooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyGoo"), manyFooHasManyGooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, gooInstanceId2, fooClassId, gooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyGoo"), manyFooHasManyGooInstanceId3.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId2, gooInstanceId1, fooClassId, gooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyGoo"), manyFooHasManyGooInstanceId4.GetValue ());
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
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Foo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
        "    </ECClass>"
        "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECClass>"
        "    <ECRelationshipClass typeName='FooHasGooWithKeyProp' isDomainClass='True' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(1,1)' polymorphic='True'>"
        "      <Class class = 'Goo' >"
        "        <Key><Property name='A'/></Key>"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='FooHasManyGooWithKeyProp' isDomainClass='True' strength='referencing'>"
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
    ecsql.Sprintf (ToInsertECSql (db, "FooHasGooWithKeyProp"), fooInstanceId1.GetValue (), gooInstanceId1.GetValue (), fooClassId, gooClassId);
    fooHasGooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();
    db.SaveChanges();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasGooWithKeyProp"), fooHasGooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, gooInstanceId1, fooClassId, gooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId fooHasManyGooInstanceId1;
    EC::ECInstanceId fooHasManyGooInstanceId2;
    Savepoint savePoint (db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyGooWithKeyProp"), fooInstanceId1.GetValue (), gooInstanceId1.GetValue (), fooClassId, gooClassId);
    fooHasManyGooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyGooWithKeyProp"), fooInstanceId1.GetValue (), gooInstanceId2.GetValue (), fooClassId, gooClassId);
    fooHasManyGooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();
    db.SaveChanges();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyGooWithKeyProp"), fooHasManyGooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, gooInstanceId1, fooClassId, gooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyGooWithKeyProp"), fooHasManyGooInstanceId2.GetValue ());
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
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Foo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
        "    </ECClass>"
        "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECClass>"
        "   <ECClass typeName='Boo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECClass>"
        "   <ECClass typeName='Roo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Boo</BaseClass>"
        "        <ECProperty propertyName='G' typeName='long'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "    </ECClass>"
        "    <ECRelationshipClass typeName='GooHasRoo' isDomainClass='True' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Source>"
        "    <Target cardinality='(1,1)' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='GooHasManyRoo' isDomainClass='True' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='ManyGooHasManyRoo' isDomainClass='True' strength='referencing'>"
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
    ecsql.Sprintf (ToInsertECSql (db, "GooHasRoo"), gooInstanceId1.GetValue (), rooInstanceId1.GetValue (), gooClassId, rooClassId);
    gooHasRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "GooHasRoo"), gooHasRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, rooInstanceId1, gooClassId, rooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId gooHasManyRooInstanceId1;
    EC::ECInstanceId gooHasManyRooInstanceId2;
    Savepoint savePoint (db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "GooHasManyRoo"), gooInstanceId1.GetValue (), rooInstanceId1.GetValue (), gooClassId, rooClassId);
    gooHasManyRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "GooHasManyRoo"), gooInstanceId1.GetValue (), rooInstanceId2.GetValue (), gooClassId, rooClassId);
    gooHasManyRooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "GooHasManyRoo"), gooHasManyRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, rooInstanceId1, gooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "GooHasManyRoo"), gooHasManyRooInstanceId2.GetValue ());
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
    ecsql.Sprintf (ToInsertECSql (db, "ManyGooHasManyRoo"), gooInstanceId1.GetValue (), rooInstanceId1.GetValue (), gooClassId, rooClassId);
    manyGooHasManyRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyGooHasManyRoo"), gooInstanceId1.GetValue (), rooInstanceId2.GetValue (), gooClassId, rooClassId);
    manyGooHasManyRooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyGooHasManyRoo"), gooInstanceId2.GetValue (), rooInstanceId1.GetValue (), gooClassId, rooClassId);
    manyGooHasManyRooInstanceId3 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyGooHasManyRoo"), gooInstanceId2.GetValue (), rooInstanceId2.GetValue (), gooClassId, rooClassId);
    manyGooHasManyRooInstanceId4 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "ManyGooHasManyRoo"), manyGooHasManyRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, rooInstanceId1, gooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyGooHasManyRoo"), manyGooHasManyRooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, rooInstanceId2, gooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyGooHasManyRoo"), manyGooHasManyRooInstanceId3.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId2, rooInstanceId1, gooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyGooHasManyRoo"), manyGooHasManyRooInstanceId4.GetValue ());
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
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Foo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
        "    </ECClass>"
        "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECClass>"
        "   <ECClass typeName='Boo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='E1' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECClass>"
        "   <ECClass typeName='Roo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Boo</BaseClass>"
        "        <ECProperty propertyName='G' typeName='long'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "    </ECClass>"
        "    <ECRelationshipClass typeName='GooHasRooWithKeyProp' isDomainClass='True' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Source>"
        "    <Target cardinality='(1,1)' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "           <Key>"
        "              <Property name='E'/>"
        "           </Key>"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='GooHasManyRooWithKeyProp' isDomainClass='True' strength='referencing'>"
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
    ecsql.Sprintf (ToInsertECSql (db, "GooHasRooWithKeyProp"), gooInstanceId1.GetValue (), rooInstanceId1.GetValue (), gooClassId, rooClassId);
    gooHasRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "GooHasRooWithKeyProp"), gooHasRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, rooInstanceId1, gooClassId, rooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId gooHasManyRooInstanceId1;
    EC::ECInstanceId gooHasManyRooInstanceId2;
    Savepoint savePoint (db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "GooHasManyRooWithKeyProp"), gooInstanceId1.GetValue (), rooInstanceId1.GetValue (), gooClassId, rooClassId);
    gooHasManyRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "GooHasManyRooWithKeyProp"), gooInstanceId1.GetValue (), rooInstanceId2.GetValue (), gooClassId, rooClassId);
    gooHasManyRooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "GooHasManyRooWithKeyProp"), gooHasManyRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), gooInstanceId1, rooInstanceId1, gooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "GooHasManyRooWithKeyProp"), gooHasManyRooInstanceId2.GetValue ());
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
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Foo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
        "    </ECClass>"
        "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECClass>"
        "   <ECClass typeName='Roo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECClass>"
        "    <ECRelationshipClass typeName='FooHasRoo' isDomainClass='True' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(1,1)' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='FooHasManyRoo' isDomainClass='True' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Roo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='ManyFooHasManyRoo' isDomainClass='True' strength='referencing'>"
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
    ecsql.Sprintf (ToInsertECSql (db, "FooHasRoo"), fooInstanceId1.GetValue (), rooInstanceId1.GetValue (), fooClassId, rooClassId);
    fooHasRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());
    
    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasRoo"), fooHasRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, rooInstanceId1, fooClassId, rooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId fooHasManyRooInstanceId1;
    EC::ECInstanceId fooHasManyRooInstanceId2;
    Savepoint savePoint (db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyRoo"), fooInstanceId1.GetValue (), rooInstanceId1.GetValue (), fooClassId, rooClassId);
    fooHasManyRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "FooHasManyRoo"), fooInstanceId1.GetValue (), rooInstanceId2.GetValue (), fooClassId, rooClassId);
    fooHasManyRooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyRoo"), fooHasManyRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, rooInstanceId1, fooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "FooHasManyRoo"), fooHasManyRooInstanceId2.GetValue ());
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
    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyRoo"), fooInstanceId1.GetValue (), rooInstanceId1.GetValue (), fooClassId, rooClassId);
    manyFooHasManyRooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyRoo"), fooInstanceId1.GetValue (), rooInstanceId2.GetValue (), fooClassId, rooClassId);
    manyFooHasManyRooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyRoo"), fooInstanceId2.GetValue (), rooInstanceId1.GetValue (), fooClassId, rooClassId);
    manyFooHasManyRooInstanceId3 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyFooHasManyRoo"), fooInstanceId2.GetValue (), rooInstanceId2.GetValue (), fooClassId, rooClassId);
    manyFooHasManyRooInstanceId4 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyRoo"), manyFooHasManyRooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, rooInstanceId1, fooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyRoo"), manyFooHasManyRooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId1, rooInstanceId2, fooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyRoo"), manyFooHasManyRooInstanceId3.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), fooInstanceId2, rooInstanceId1, fooClassId, rooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyFooHasManyRoo"), manyFooHasManyRooInstanceId4.GetValue ());
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
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Foo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
        "    </ECClass>"
        "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECClass>"
        "   <ECClass typeName='Roo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECClass>"
        "    <ECRelationshipClass typeName='RooHasFoo' isDomainClass='True' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Roo' />"
        "    </Source>"
        "    <Target cardinality='(1,1)' polymorphic='True'>"
        "      <Class class = 'Foo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='RooHasManyFoo' isDomainClass='True' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Roo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Foo' >"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='ManyRooHasManyFoo' isDomainClass='True' strength='referencing'>"
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
    ecsql.Sprintf (ToInsertECSql (db, "RooHasFoo"), rooInstanceId1.GetValue (),fooInstanceId1.GetValue (), rooClassId, fooClassId);
    rooHasFooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "RooHasFoo"), rooHasFooInstanceId1.GetValue());
    VerifyInsertedInstance (db, ecsql.c_str (), rooInstanceId1, fooInstanceId1, rooClassId, fooClassId);
    }

    //Insert 1-N Relationship
    {
    EC::ECInstanceId rooHasManyFooInstanceId1;
    EC::ECInstanceId rooHasManyFooInstanceId2;
    Savepoint savePoint (db, "1-N Relationship Instances");

    Utf8String ecsql;
    ecsql.Sprintf (ToInsertECSql (db, "RooHasManyFoo"), rooInstanceId1.GetValue (), fooInstanceId1.GetValue (), rooClassId, fooClassId);
    rooHasManyFooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "RooHasManyFoo"), rooInstanceId1.GetValue (), fooInstanceId2.GetValue (), rooClassId, fooClassId);
    rooHasManyFooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "RooHasManyFoo"), rooHasManyFooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), rooInstanceId1, fooInstanceId1, rooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "RooHasManyFoo"), rooHasManyFooInstanceId2.GetValue ());
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
    ecsql.Sprintf (ToInsertECSql (db, "ManyRooHasManyFoo"), rooInstanceId1.GetValue (), fooInstanceId1.GetValue (), rooClassId, fooClassId);
    manyRooHasManyFooInstanceId1 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyRooHasManyFoo"), rooInstanceId1.GetValue (), fooInstanceId2.GetValue (), rooClassId, fooClassId);
    manyRooHasManyFooInstanceId2 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyRooHasManyFoo"), rooInstanceId2.GetValue (), fooInstanceId1.GetValue (), rooClassId, fooClassId);
    manyRooHasManyFooInstanceId3 = InsertTestInstance (db, ecsql.c_str ());

    ecsql.Sprintf (ToInsertECSql (db, "ManyRooHasManyFoo"), rooInstanceId2.GetValue (), fooInstanceId2.GetValue (), rooClassId, fooClassId);
    manyRooHasManyFooInstanceId4 = InsertTestInstance (db, ecsql.c_str ());

    savePoint.Commit ();

    ecsql.Sprintf (ToSelectECSql (db, "ManyRooHasManyFoo"), manyRooHasManyFooInstanceId1.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), rooInstanceId1, fooInstanceId1, rooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyRooHasManyFoo"), manyRooHasManyFooInstanceId2.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), rooInstanceId1, fooInstanceId2, rooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyRooHasManyFoo"), manyRooHasManyFooInstanceId3.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), rooInstanceId2, fooInstanceId1, rooClassId, fooClassId);

    ecsql.Sprintf (ToSelectECSql (db, "ManyRooHasManyFoo"), manyRooHasManyFooInstanceId4.GetValue ());
    VerifyInsertedInstance (db, ecsql.c_str (), rooInstanceId2, fooInstanceId2, rooClassId, fooClassId);
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
void ApplyCustomAttributeAndImportSchema (ECDbR ecdb, ECSchemaPtr ecSchema)
    {
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext ();
    readContext->AddSchemaLocater (ecdb.GetSchemaLocater ());
    SchemaKey ecdbmapKey = SchemaKey ("ECDbMap", 1, 0);
    ECSchemaPtr ecdbmapSchema = readContext->LocateSchema (ecdbmapKey, SchemaMatchType::SCHEMAMATCHTYPE_LatestCompatible);
    ASSERT_TRUE (ecdbmapSchema.IsValid ());
    readContext->AddSchema (*ecSchema);
    ecSchema->AddReferencedSchema (*ecdbmapSchema);

    ECClassP personClass = ecSchema->GetClassP ("Person");
    ASSERT_TRUE (personClass != nullptr);

    ECClassCP ca = ecdbmapSchema->GetClassCP ("ClassMap");
    EXPECT_TRUE (ca != nullptr);
    auto customAttribute = ca->GetDefaultStandaloneEnabler ()->CreateInstance ();
    EXPECT_TRUE (customAttribute != nullptr);
    ASSERT_TRUE (customAttribute->SetValue ("MapStrategy.Strategy", ECValue ("SharedTable")) == ECObjectsStatus::Success);
    ASSERT_TRUE (customAttribute->SetValue ("MapStrategy.Options", ECValue ("JoinedTablePerDirectSubclass")) == ECObjectsStatus::Success);
    ASSERT_TRUE (customAttribute->SetValue ("MapStrategy.AppliesToSubclasses", ECValue (true)) == ECObjectsStatus::Success);
    ASSERT_TRUE (personClass->SetCustomAttribute (*customAttribute) == ECObjectsStatus::Success);

    ASSERT_EQ (SUCCESS, ecdb.Schemas ().ImportECSchemas (readContext->GetCache ()));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (JoinedTableECDbMapStrategyTests, PopulateECSql_TestDbWithTestData)
    {
    ECDbR ecdb = SetupECDb ("JoinedTableECSqlStatementTests.ecdb");

    ECSchemaPtr schemaPtr = ECDbTestUtility::ReadECSchemaFromDisk (L"ECSqlStatementTests.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE (schemaPtr != NULL);
    
    ApplyCustomAttributeAndImportSchema (ecdb, schemaPtr);

    ECSqlStatementTestsSchemaHelper::Populate (ecdb);
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (JoinedTableECDbMapStrategyTests, VerifyECSqlOnAbstractBaseClass)
    {
    ECDbR ecdb = SetupECDb ("JoinedTableECSqlStatementTests.ecdb");

    ECSchemaPtr schemaPtr = ECDbTestUtility::ReadECSchemaFromDisk (L"ECSqlStatementTests.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE (schemaPtr != NULL);

    ApplyCustomAttributeAndImportSchema (ecdb, schemaPtr);
    Utf8CP expectedGeneratedECSql = "SELECT [Person].[ECInstanceId] FROM (SELECT [ECST_Person].ECClassId, [ECST_Person].[ECInstanceId] FROM [ECST_Person]) [Person]";
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT ECInstanceId FROM ECST.Person"));
    ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ());
    ASSERT_STREQ (expectedGeneratedECSql, stmt.GetNativeSql ());
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(JoinedTableECDbMapStrategyTests, MultiInheritence1)
    {
    SchemaItem testSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='dgn' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Foo'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                    <Strategy>SharedTable</Strategy>"
        "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                    <Options>JoinedTablePerDirectSubclass</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Foo_L' typeName='long'/>"
        "        <ECProperty propertyName='Foo_S' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='IFace' modifier='Abstract' />"
        "   <ECEntityClass typeName='Goo'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <BaseClass>IFace</BaseClass>"
        "        <ECProperty propertyName='Goo_L' typeName='long'/>"
        "        <ECProperty propertyName='Goo_S' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Boo'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <BaseClass>IFace</BaseClass>"
        "        <ECProperty propertyName='Boo_L' typeName='long'/>"
        "        <ECProperty propertyName='Boo_S' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Moo'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='Moo_L' typeName='long'/>"
        "        <ECProperty propertyName='Moo_S' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECEntityClass typeName='Body'>"
        "        <ECProperty propertyName='Body_L' typeName='long'/>"
        "        <ECProperty propertyName='Body_S' typeName='string'/>"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='IFaceHasBody' isDomainClass='True' strength='referencing'>"
        "       <Source cardinality='(1,1)' polymorphic='True'>"
        "           <Class class = 'IFace' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class = 'Body' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>");

    ECDbR db = SetupECDb("JoinedTableTest101.ecdb", testSchema);
    ASSERT_TRUE(db.IsDbOpen());

    ECClassId gooId = db.Schemas().GetECClassId("dgn", "Goo", ResolveSchema::BySchemaNamespacePrefix);
    ECClassId booId = db.Schemas().GetECClassId("dgn", "Boo", ResolveSchema::BySchemaNamespacePrefix);
    //ECClassId mooId = db.Schemas().GetECClassId("dgn", "Moo", ResolveSchema::BySchemaNamespacePrefix);
    ECClassId bodyId = db.Schemas().GetECClassId("dgn", "Body", ResolveSchema::BySchemaNamespacePrefix);
    ECClassId ifaceHasBodyId = db.Schemas().GetECClassId("dgn", "IFaceHasBody", ResolveSchema::BySchemaNamespacePrefix);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO dgn.Goo(ECInstanceId, Foo_L, Foo_S, Goo_L, Goo_S) VALUES(1, 101, '::101', 102, '::102')"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO dgn.Boo(ECInstanceId, Foo_L, Foo_S, Boo_L, Boo_S) VALUES(2, 201, '::201', 202, '::202')"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO dgn.Moo(ECInstanceId, Foo_L, Foo_S, Moo_L, Moo_S) VALUES(3, 301, '::301', 302, '::302')"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO dgn.Body(ECInstanceId, Body_L, Body_S) VALUES(4, 401, '::401')"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO dgn.Body(ECInstanceId, Body_L, Body_S) VALUES(5, 401, '::401')"));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db,SqlPrintfString("INSERT INTO dgn.IFaceHasBody(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(%d,%lld,%d,%lld)", 1, gooId, 4, bodyId).GetUtf8CP()));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, SqlPrintfString("INSERT INTO dgn.IFaceHasBody(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(%d,%lld,%d,%lld)", 2, booId, 5, bodyId).GetUtf8CP()));
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT ECInstanceId, GetECClassId(), SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM dgn.IFaceHasBody ORDER BY ECInstanceId"));
    int count = 0;
    while(stmt.Step() == BE_SQLITE_ROW)
        {
        auto ecInstanceId = stmt.GetValueInt64(0);
        auto ecClassId = stmt.GetValueInt64(1);
        auto sourceECInstanceId = stmt.GetValueInt64(2);
        auto sourceECClassId = stmt.GetValueInt64(3);
        auto targetECInstanceId = stmt.GetValueInt64(4);
        auto targetECClassId = stmt.GetValueInt64(5);
        if (count == 0)
            {
            //    5	135	2	131	5	130
            ASSERT_EQ(4, ecInstanceId);
            ASSERT_EQ(ifaceHasBodyId, ecClassId);
            ASSERT_EQ(1, sourceECInstanceId);
            ASSERT_EQ(gooId, sourceECClassId);
            ASSERT_EQ(4, targetECInstanceId);
            ASSERT_EQ(bodyId, targetECClassId);
            }
        else if (count == 1)
            {
            //    5	135	2	131	5	130
            ASSERT_EQ(5, ecInstanceId);
            ASSERT_EQ(ifaceHasBodyId, ecClassId);
            ASSERT_EQ(2, sourceECInstanceId);
            ASSERT_EQ(booId, sourceECClassId);
            ASSERT_EQ(5, targetECInstanceId);
            ASSERT_EQ(bodyId, targetECClassId);
            }

        count++;
        }
    stmt.Finalize();

    db.Schemas().CreateECClassViewsInDb();

    }

TEST_F (JoinedTableECDbMapStrategyTests, JoinedTableForClassesWithoutBusinessProperties)
    {
    SchemaItem testSchema (
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

    ECDbR db = SetupECDb ("ClassesWithoutBusinessProperties.ecdb", testSchema);
    ASSERT_TRUE (db.IsDbOpen ());

    //Verify that only one base table and one Secondary Table have been created, rest of the tables will not be mapped as joined table because they don't have business properties.
    Statement statement;
    ASSERT_EQ (DbResult::BE_SQLITE_OK, statement.Prepare (db, "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name LIKE 'dgn_%'"));
    ASSERT_EQ (DbResult::BE_SQLITE_ROW, statement.Step ());
    ASSERT_EQ (2, statement.GetValueInt (0));
    }
END_ECDBUNITTESTS_NAMESPACE
