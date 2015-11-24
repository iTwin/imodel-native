/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbJoinedTable_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "SchemaImportTestFixture.h"

#include <initializer_list>
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         10/05
//---------------+---------------+---------------+---------------+---------------+-------
struct JoinedTableECDbMapStrategyTests: ECDbMappingTestFixture
    {
    protected:
        void AssertTableLayouts(ECDbCR, bmap<Utf8String, Utf8String> const& tableLayouts, Utf8CP scenario) const;
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
                                 "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                                 "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                                 "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
                                 "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
                                 "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                                 "    <ECClass typeName='C0' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                                 "   <ECClass typeName='C1' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                 "        <BaseClass>C0</BaseClass>"
                                 "        <ECProperty propertyName='C' typeName='long'/>"
                                 "        <ECProperty propertyName='D' typeName='string'/>"
                                 "    </ECClass>"
                                 "   <ECClass typeName='C2' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                 "        <BaseClass>C0</BaseClass>"
                                 "        <ECProperty propertyName='E' typeName='long'/>"
                                 "        <ECProperty propertyName='F' typeName='string'/>"
                                 "    </ECClass>"
                                 "   <ECClass typeName='C11' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                 "        <BaseClass>C1</BaseClass>"
                                 "        <ECProperty propertyName='G' typeName='long'/>"
                                 "        <ECProperty propertyName='H' typeName='string'/>"
                                 "    </ECClass>"
                                 "   <ECClass typeName='C21' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                 "        <BaseClass>C2</BaseClass>"
                                 "        <ECProperty propertyName='I' typeName='long'/>"
                                 "        <ECProperty propertyName='J' typeName='string'/>"
                                 "    </ECClass>"
                                 "</ECSchema>"));
    testItem.AddTableLayout("ts_C0", "a b");
    testItem.AddTableLayout("ts_C0_C1_joined", "c d g h");
    testItem.AddTableLayout("ts_C0_C2_joined", "e f i j");
    testItems.push_back(testItem);

    testItem = TestItem(SchemaItem("JoinedTablePerDirectSubclass_on_c1",
                                   "<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='ts' version='1.0'"
                                   "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                                   "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                                   "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
                                   "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
                                   "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                                   "    <ECClass typeName='C0' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                                   "    </ECClass>"
                                   "   <ECClass typeName='C1' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                                   "    </ECClass>"
                                   "   <ECClass typeName='C2' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                   "        <BaseClass>C0</BaseClass>"
                                   "        <ECProperty propertyName='C2_A' typeName='long'/>"
                                   "        <ECProperty propertyName='C2_B' typeName='string'/>"
                                   "    </ECClass>"
                                   "   <ECClass typeName='C11' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                   "        <BaseClass>C1</BaseClass>"
                                   "        <ECProperty propertyName='C11_A' typeName='long'/>"
                                   "        <ECProperty propertyName='C11_B' typeName='string'/>"
                                   "    </ECClass>"
                                   "   <ECClass typeName='C12' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                   "        <BaseClass>C1</BaseClass>"
                                   "        <ECProperty propertyName='C12_A' typeName='long'/>"
                                   "        <ECProperty propertyName='C12_B' typeName='string'/>"
                                   "    </ECClass>"
                                   "   <ECClass typeName='C111' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                   "        <BaseClass>C11</BaseClass>"
                                   "        <ECProperty propertyName='C111_A' typeName='long'/>"
                                   "        <ECProperty propertyName='C111_B' typeName='string'/>"
                                   "    </ECClass>"
                                   "   <ECClass typeName='C21' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                   "        <BaseClass>C2</BaseClass>"
                                   "        <ECProperty propertyName='C21_A' typeName='long'/>"
                                   "        <ECProperty propertyName='C21_B' typeName='string'/>"
                                   "    </ECClass>"
                                   "</ECSchema>"));

    testItem.AddTableLayout("ts_C0", "c0_a c0_b c1_a c1_b c21_a c21_b c2_a c2_b");
    testItem.AddTableLayout("ts_C0_C11_joined", "c111_a c111_b c11_a c11_b");
    testItem.AddTableLayout("ts_C0_C12_joined", "c12_a c12_b");
    testItems.push_back(testItem);

    testItem = TestItem(SchemaItem("JoinedTablePerDirectSubclass_on_c1_and_c2",
                                   "<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='ts' version='1.0'"
                                   "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                                   "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                                   "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
                                   "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
                                   "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                                   "    <ECClass typeName='C0' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                                   "   <ECClass typeName='C1' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                                   "    </ECClass>"
                                   "   <ECClass typeName='C2' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                                   "    </ECClass>"
                                   "   <ECClass typeName='C11' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                   "        <BaseClass>C1</BaseClass>"
                                   "        <ECProperty propertyName='G' typeName='long'/>"
                                   "        <ECProperty propertyName='H' typeName='string'/>"
                                   "    </ECClass>"
                                   "   <ECClass typeName='C21' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                   "        <BaseClass>C2</BaseClass>"
                                   "        <ECProperty propertyName='I' typeName='long'/>"
                                   "        <ECProperty propertyName='J' typeName='string'/>"
                                   "    </ECClass>"
                                   "</ECSchema>"));

    testItem.AddTableLayout("ts_C0", "a b c d e f");
    testItem.AddTableLayout("ts_C0_C11_joined", "g h");
    testItem.AddTableLayout("ts_C0_C21_joined", "i j");
    testItems.push_back(testItem);

    //SingleJoinedTableForSubclasses tests

    testItem = TestItem(SchemaItem("SingleJoinedTableForSubclasses_on_c0",
                                 "<?xml version='1.0' encoding='utf-8'?>"
                                 "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='ts' version='1.0'"
                                 "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                                 "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                                 "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
                                 "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
                                 "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                                 "    <ECClass typeName='C0' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                 "        <ECCustomAttributes>"
                                 "            <ClassMap xmlns='ECDbMap.01.00'>"
                                 "                <MapStrategy>"
                                 "                    <Strategy>SharedTable</Strategy>"
                                 "                    <AppliesToSubclasses>True</AppliesToSubclasses>"
                                 "                    <Options>SingleJoinedTableForSubclasses</Options>"
                                 "                </MapStrategy>"
                                 "            </ClassMap>"
                                 "        </ECCustomAttributes>"
                                 "        <ECProperty propertyName='A' typeName='long'/>"
                                 "        <ECProperty propertyName='B' typeName='string'/>"
                                 "    </ECClass>"
                                 "   <ECClass typeName='C1' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                 "        <BaseClass>C0</BaseClass>"
                                 "        <ECProperty propertyName='C' typeName='long'/>"
                                 "        <ECProperty propertyName='D' typeName='string'/>"
                                 "    </ECClass>"
                                 "   <ECClass typeName='C2' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                 "        <BaseClass>C0</BaseClass>"
                                 "        <ECProperty propertyName='E' typeName='long'/>"
                                 "        <ECProperty propertyName='F' typeName='string'/>"
                                 "    </ECClass>"
                                 "   <ECClass typeName='C11' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                 "        <BaseClass>C1</BaseClass>"
                                 "        <ECProperty propertyName='G' typeName='long'/>"
                                 "        <ECProperty propertyName='H' typeName='string'/>"
                                 "    </ECClass>"
                                 "   <ECClass typeName='C21' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                 "        <BaseClass>C2</BaseClass>"
                                 "        <ECProperty propertyName='I' typeName='long'/>"
                                 "        <ECProperty propertyName='J' typeName='string'/>"
                                 "    </ECClass>"
                                 "</ECSchema>"));
    testItem.AddTableLayout("ts_C0", "a b");
    testItem.AddTableLayout("ts_C0_joined", "c d e f g h i j");
    testItems.push_back(testItem);

    testItem = TestItem(SchemaItem("SingleJoinedTableForSubclasses_on_c1",
                                       "<?xml version='1.0' encoding='utf-8'?>"
                                       "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='ts' version='1.0'"
                                       "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                                       "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                                       "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
                                       "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
                                       "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                                       "    <ECClass typeName='C0' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                                       "   <ECClass typeName='C1' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                       "        <ECCustomAttributes>"
                                       "            <ClassMap xmlns='ECDbMap.01.00'>"
                                       "                <MapStrategy>"
                                       "                    <Options>SingleJoinedTableForSubclasses</Options>"
                                       "                </MapStrategy>"
                                       "            </ClassMap>"
                                       "        </ECCustomAttributes>"
                                       "        <BaseClass>C0</BaseClass>"
                                       "        <ECProperty propertyName='C' typeName='long'/>"
                                       "        <ECProperty propertyName='D' typeName='string'/>"
                                       "    </ECClass>"
                                       "   <ECClass typeName='C2' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                       "        <BaseClass>C0</BaseClass>"
                                       "        <ECProperty propertyName='E' typeName='long'/>"
                                       "        <ECProperty propertyName='F' typeName='string'/>"
                                       "    </ECClass>"
                                       "   <ECClass typeName='C11' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                       "        <BaseClass>C1</BaseClass>"
                                       "        <ECProperty propertyName='G' typeName='long'/>"
                                       "        <ECProperty propertyName='H' typeName='string'/>"
                                       "    </ECClass>"
                                        "   <ECClass typeName='C21' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                                        "        <BaseClass>C2</BaseClass>"
                                        "        <ECProperty propertyName='I' typeName='long'/>"
                                        "        <ECProperty propertyName='J' typeName='string'/>"
                                        "    </ECClass>"
                                       "</ECSchema>"));

    testItem.AddTableLayout("ts_C0", "a b c d e f i j");
    testItem.AddTableLayout("ts_C0_C1_joined", "g h");
    testItems.push_back(testItem);

    testItem = TestItem( SchemaItem("SingleJoinedTableForSubclasses_on_c1_and_c2",
                            "<?xml version='1.0' encoding='utf-8'?>"
                            "<ECSchema schemaName='JoinedTableTest' nameSpacePrefix='ts' version='1.0'"
                            "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                            "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
                            "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
                            "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
                            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                            "    <ECClass typeName='C0' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
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
                            "   <ECClass typeName='C1' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                            "        <ECCustomAttributes>"
                            "            <ClassMap xmlns='ECDbMap.01.00'>"
                            "                <MapStrategy>"
                            "                    <Options>SingleJoinedTableForSubclasses</Options>"
                            "                </MapStrategy>"
                            "            </ClassMap>"
                            "        </ECCustomAttributes>"
                            "        <BaseClass>C0</BaseClass>"
                            "        <ECProperty propertyName='C' typeName='long'/>"
                            "        <ECProperty propertyName='D' typeName='string'/>"
                            "    </ECClass>"
                            "   <ECClass typeName='C2' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                           "        <ECCustomAttributes>"
                           "            <ClassMap xmlns='ECDbMap.01.00'>"
                           "                <MapStrategy>"
                           "                    <Options>SingleJoinedTableForSubclasses</Options>"
                           "                </MapStrategy>"
                           "            </ClassMap>"
                           "        </ECCustomAttributes>"
                           "        <BaseClass>C0</BaseClass>"
                            "        <ECProperty propertyName='E' typeName='long'/>"
                            "        <ECProperty propertyName='F' typeName='string'/>"
                            "    </ECClass>"
                            "   <ECClass typeName='C11' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                            "        <BaseClass>C1</BaseClass>"
                            "        <ECProperty propertyName='G' typeName='long'/>"
                            "        <ECProperty propertyName='H' typeName='string'/>"
                            "    </ECClass>"
                            "   <ECClass typeName='C21' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
                            "        <BaseClass>C2</BaseClass>"
                            "        <ECProperty propertyName='I' typeName='long'/>"
                            "        <ECProperty propertyName='J' typeName='string'/>"
                            "    </ECClass>"
                            "</ECSchema>"));

    testItem.AddTableLayout("ts_C0", "a b c d e f");
    testItem.AddTableLayout("ts_C0_C1_joined", "g h");
    testItem.AddTableLayout("ts_C0_C2_joined", "i j");
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
    expectedTableLayouts["rs_Base_Sub1_joined"] = "p1 p11";
    expectedTableLayouts["rs_Base_ts_Sub2_joined"] = "p2";
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
// @bsimethod                                   Affan.Khan                         10/05
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
// @bsimethod                                   Affan.Khan                         10/05
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableECDbMapStrategyTests, InsertWithUnnamedParameterBinding)
    {
    SchemaItem testSchema("<?xml version='1.0' encoding='utf-8'?>"
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
    ASSERT_EQ(stmt.Prepare(db, "INSERT INTO dgn.Goo (ECInstanceId, A, B, C, D ) VALUES ( ?, ?, ?, ?, ?)"), ECSqlStatus::Success);



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
    ASSERT_EQ(stmt.Prepare(db, "SELECT A, B, C, D FROM dgn.Goo WHERE ECInstanceId = ?"), ECSqlStatus::Success);;
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

    ASSERT_EQ(stmt.Prepare(db, "DELETE FROM dgn.Goo WHERE ECInstanceId = ?"), ECSqlStatus::Success);
    auto bindR = stmt.BindInt64(1, 101);
    ASSERT_EQ(ECSqlStatus::Success, bindR);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();

    ASSERT_EQ(stmt.Prepare(db, "SELECT A, B, C, D FROM dgn.Goo WHERE ECInstanceId = 101"), ECSqlStatus::Success);
    ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         10/05
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
END_ECDBUNITTESTS_NAMESPACE
