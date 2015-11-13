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
    {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         10/05
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableECDbMapStrategyTests, TableLayout)
    {
    struct TestItem
        {
        SchemaItem m_testSchema;
        std::set<Utf8String> m_classesWithJoinedTable;

        TestItem(SchemaItem const& testSchema, std::set<Utf8String> const& classesWithJoinedTable) : m_testSchema(testSchema), m_classesWithJoinedTable(classesWithJoinedTable) {}
        };

    std::vector<TestItem> testItems
        {
        TestItem(SchemaItem("Join on C0",
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
                                "                    <Options>JoinedTableForSubclasses</Options>"
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
                                "</ECSchema>"), {"C0"}),

                      TestItem(SchemaItem("Join on C1",
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
                                       "                    <Options>JoinedTableForSubclasses</Options>"
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
                                       "</ECSchema>"),  {"C1"}),

               TestItem( SchemaItem("Join on C1 and C2",
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
                            "                    <Options>JoinedTableForSubclasses</Options>"
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
                           "                    <Options>JoinedTableForSubclasses</Options>"
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
                            "</ECSchema>"), {"C1", "C2"})
        };


    for (TestItem const& testItem : testItems)
        {
        ECDbR ecdb = SetupECDb("joinedtablemapstrategy.ecdb", testItem.m_testSchema);
        ASSERT_TRUE(ecdb.IsDbOpen());

        ECSchemaCP schema = ecdb.Schemas().GetECSchema("JoinedTableTest");
        for (ECClassCP ecclass : schema->GetClasses())
            {
            const bool expectedToHaveJoinedTable = testItem.m_classesWithJoinedTable.find(ecclass->GetName()) != testItem.m_classesWithJoinedTable.end();
            
            Utf8String joinedTableName;
            if (ecclass->GetName().EqualsI("C0"))
                joinedTableName.assign("ts_C0_joined");
            else
                joinedTableName.Sprintf("ts_C0_%s_joined", ecclass->GetName().c_str());

            ASSERT_EQ(expectedToHaveJoinedTable, ecdb.TableExists(joinedTableName.c_str())) << testItem.m_testSchema.m_name.c_str() << " ECClass " << ecclass->GetName().c_str();
            }

        ecdb.CloseDb();
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         10/05
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableECDbMapStrategyTests, BasicCRUD)
    {
    std::vector<SchemaItem> testSchemas {
    SchemaItem("Join on Root",
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
        "                    <Options>JoinedTableForSubclasses</Options>"
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

        SchemaItem("Join on Root and SharedColumnsForSubclasses",
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
                   "                    <Options>JoinedTableForSubclasses, SharedColumnsForSubclasses</Options>"
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

        SchemaItem("Join on single direct subclass",
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
            "                    <Options>JoinedTableForSubclasses</Options>"
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

    /*        SchemaItem("Join on single direct subclass and SharedColumnsForSubclasses on Root",
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
                       "                    <Options>JoinedTableForSubclasses</Options>"
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
                       "</ECSchema>", true),*/

        SchemaItem("Join on both subclasses",
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
            "                    <Options>JoinedTableForSubclasses</Options>"
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
            "                    <Options>JoinedTableForSubclasses</Options>"
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
            "</ECSchema>") /*,
            SchemaItem("Join on both subclasses and shared columns",
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
                "                    <Options>JoinedTableForSubclasses</Options>"
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
                "                    <Options>JoinedTableForSubclasses</Options>"
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
                "</ECSchema>")*/
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
// @bsimethod                                   Affan.Khan                         10/05
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableECDbMapStrategyTests, MappingAcrossTwoSchemaImportSession)
    {
    auto const baseSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='BaseSchema' nameSpacePrefix='bs' version='1.0'"
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
        "                    <Options>JoinedTableForSubclasses</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='long'/>"
        "        <ECProperty propertyName='B' typeName='string'/>"
        "    </ECClass>"
        "   <ECClass typeName='BaseGoo' isDomainClass='false' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='C' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='string'/>"
        "    </ECClass>"
        "   <ECClass typeName='BaseBoo' isDomainClass='false' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>Foo</BaseClass>"
        "        <ECProperty propertyName='E' typeName='long'/>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "    </ECClass>"
        "   <ECClass typeName='BaseRoo' isDomainClass='false' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>BaseBoo</BaseClass>"
        "        <ECProperty propertyName='G' typeName='long'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "    </ECClass>"
        "</ECSchema>";

    auto const childSchemaXml =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='ChildSchema' nameSpacePrefix='cs' version='1.0'"
        "   xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:ecschema='http://www.bentley.com/schemas/Bentley.ECXML.2.0'"
        "   xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'"
        "   xsi:schemaLocation='ecschema ECSchema.xsd' >"
        "    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.12' prefix='bsca' />"
        "    <ECSchemaReference name='BaseSchema' version='01.00' prefix='bs' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "   <ECClass typeName='Goo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>bs:BaseGoo</BaseClass>"
        "    </ECClass>"
        "   <ECClass typeName='Boo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>bs:BaseBoo</BaseClass>"
        "    </ECClass>"
        "   <ECClass typeName='Roo' isDomainClass='True' isStruct='False' isCustomAttributeClass='False'>"
        "        <BaseClass>bs:BaseRoo</BaseClass>"
        "    </ECClass>"
        "</ECSchema>";


    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create("JoinedTableTest2.ecdb");
    ECSchemaPtr baseSchema, childSchema;
    auto readContext = ECSchemaReadContext::CreateContext();
    //First import base schema
    ECSchema::ReadFromXmlString(baseSchema, baseSchemaXml, *readContext);
    ASSERT_TRUE(baseSchema != nullptr);
    auto importStatus = db.Schemas().ImportECSchemas(readContext->GetCache());
    ASSERT_TRUE(importStatus == BentleyStatus::SUCCESS);
    //Then import child schema along with base schema
    ECSchema::ReadFromXmlString(childSchema, childSchemaXml, *readContext);
    ASSERT_TRUE(childSchema != nullptr);
    importStatus = db.Schemas().ImportECSchemas(readContext->GetCache());
    ASSERT_TRUE(importStatus == BentleyStatus::SUCCESS);


    db.SaveChanges();
    auto assert_ecsql = [&db] (Utf8CP sql, ECSqlStatus expectedStatus, DbResult expectedStepStatus)
        {
        ECSqlStatement stmt;
        LOG.infov("Executing : %s", sql);
        ASSERT_EQ(stmt.Prepare(db, sql), expectedStatus);
        if (stmt.IsPrepared())
            {
            LOG.infov("NativeSQL : %s", stmt.GetNativeSql());
            if (expectedStatus == ECSqlStatus::Success)
                {
                ASSERT_EQ(stmt.Step(), expectedStepStatus);
                }
            }
        };
    auto assert_ecsql2 = [&db] (Utf8CP sql, ECSqlStatus expectedStatus, int columnCountExpected, int rowCountExpected)
        {
        ECSqlStatement stmt;
        LOG.infov("Executing : %s", sql);
        ASSERT_EQ(stmt.Prepare(db, sql), expectedStatus);
        if (stmt.IsPrepared())
            {
            LOG.infov("NativeSQL : %s", stmt.GetNativeSql());
            ASSERT_EQ(stmt.GetColumnCount(), columnCountExpected);

            int realRowCount = 0;
            while (stmt.Step() == BE_SQLITE_ROW)
                realRowCount++;

            ASSERT_EQ(realRowCount, rowCountExpected);
            }
        };


    assert_ecsql("UPDATE cs.Goo SET A = ?, B = 'bb1', C = :c1, D = 'dd1' WHERE  A = ? AND B = :b1;", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("UPDATE cs.Goo SET A = ?, B = 'bb1' WHERE  A = ? AND B = :b1;", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("UPDATE cs.Goo SET C = :c1, D = 'dd1' WHERE  A = ? AND B = :b1;", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("UPDATE bs.Foo SET A = 2, B = 'bb1' WHERE  A = 101 AND B = 'b1';", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);

    assert_ecsql("INSERT INTO cs.Goo(A, B, C, D) VALUES(:a,'b1',:c,'d1');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO cs.Goo(ECInstanceId, A, B, C, D) VALUES(120, 102,'b2',202,'d2');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO cs.Goo(A, B, C, D) VALUES(103,'b3',203,'d3');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO cs.Goo(A, B) VALUES(104,'b4');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO bs.Foo(A, B) VALUES(105,'b5');", ECSqlStatus::InvalidECSql, DbResult::BE_SQLITE_DONE);

    assert_ecsql("INSERT INTO cs.Boo(A, B, E, F) VALUES(:a,'b6',:c,'f1');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO cs.Boo(ECInstanceId, A, B, E, F) VALUES(130, 102,'b8',202,'f2');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO cs.Boo(A, B, E, F) VALUES(103,'b9',203,'f3');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO cs.Boo(A, B) VALUES(105,'b10');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO bs.Foo(A, B) VALUES(104,'b11');", ECSqlStatus::InvalidECSql, DbResult::BE_SQLITE_DONE);


    assert_ecsql("INSERT INTO cs.Roo(A, B, G, H) VALUES(:a,'b12',:c,'h1');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO cs.Roo(ECInstanceId, A, B, G, H) VALUES(140, 102,'b13',202,'h2');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO cs.Roo(A, B, G, H) VALUES(103,'b14',203,'h3');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO cs.Roo(A, B) VALUES(105,'b15');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO bs.Foo(A, B) VALUES(104,'b16');", ECSqlStatus::InvalidECSql, DbResult::BE_SQLITE_DONE);
    assert_ecsql("INSERT INTO bs.Foo(A, B) VALUES(104,'b17');", ECSqlStatus::InvalidECSql, DbResult::BE_SQLITE_DONE);


    assert_ecsql2("SELECT ECInstanceId, A, B FROM bs.Foo", ECSqlStatus::Success, 3, 12);
    assert_ecsql2("SELECT ECInstanceId, A, B FROM ONLY bs.Foo", ECSqlStatus::Success, 3, 0);
    assert_ecsql2("SELECT ECInstanceId, A, B FROM bs.Foo WHERE A = 102 AND B = 'b2'", ECSqlStatus::Success, 3, 1);
    assert_ecsql2("SELECT ECInstanceId, A, B FROM ONLY bs.Foo WHERE A = 102 AND B = 'b2'", ECSqlStatus::Success, 3, 0);
    assert_ecsql2("SELECT ECInstanceId, A, B FROM ONLY bs.Foo  WHERE A = 104 AND B = 'b17'", ECSqlStatus::Success, 3, 0);


    assert_ecsql2("SELECT ECInstanceId, A, B, C, D FROM cs.Goo", ECSqlStatus::Success, 5, 4);
    assert_ecsql2("SELECT ECInstanceId, A, B FROM cs.Goo", ECSqlStatus::Success, 3, 4);
    assert_ecsql2("SELECT ECInstanceId, C, D FROM cs.Goo", ECSqlStatus::Success, 3, 4);
    assert_ecsql2("SELECT ECInstanceId, A, B, C, D FROM ONLY cs.Goo", ECSqlStatus::Success, 5, 4);
    assert_ecsql2("SELECT ECInstanceId, A, B, C, D FROM cs.Goo WHERE A = 102 AND B ='b2' AND C = 202 AND D ='d2'", ECSqlStatus::Success, 5, 1);
    assert_ecsql2("SELECT ECInstanceId, A, B, C, D FROM ONLY cs.Goo WHERE A = 102 AND B ='b2' AND C = 202 AND D ='d2'", ECSqlStatus::Success, 5, 1);


    assert_ecsql2("SELECT ECInstanceId, A, B, E, F FROM cs.Boo", ECSqlStatus::Success, 5, 4);
    assert_ecsql2("SELECT ECInstanceId, A, B FROM cs.Boo", ECSqlStatus::Success, 3, 4);
    assert_ecsql2("SELECT ECInstanceId, E, F FROM cs.Boo", ECSqlStatus::Success, 3, 4);
    assert_ecsql2("SELECT ECInstanceId, A, B, E, F FROM ONLY cs.Boo", ECSqlStatus::Success, 5, 4);
    assert_ecsql2("SELECT ECInstanceId, A, B, E, F FROM cs.Boo WHERE A = 102 AND B ='b8' AND E = 202 AND F ='f2'", ECSqlStatus::Success, 5, 1);
    assert_ecsql2("SELECT ECInstanceId, A, B, E, F FROM ONLY cs.Boo WHERE A = 102 AND B ='b8' AND E = 202 AND F ='f2'", ECSqlStatus::Success, 5, 1);


    assert_ecsql2("SELECT ECInstanceId, A, B, G, H FROM cs.Roo", ECSqlStatus::Success, 5, 4);
    assert_ecsql2("SELECT ECInstanceId, A, B FROM cs.Roo", ECSqlStatus::Success, 3, 4);
    assert_ecsql2("SELECT ECInstanceId, G, H FROM cs.Roo", ECSqlStatus::Success, 3, 4);
    assert_ecsql2("SELECT ECInstanceId, A, B, G, H FROM ONLY cs.Roo", ECSqlStatus::Success, 5, 4);
    assert_ecsql2("SELECT ECInstanceId, A, B, G, H FROM cs.Roo WHERE A = 102 AND B ='b13' AND G = 202 AND H ='h2'", ECSqlStatus::Success, 5, 1);
    assert_ecsql2("SELECT ECInstanceId, A, B, G, H FROM ONLY cs.Roo WHERE A = 102 AND B ='b13' AND G = 202 AND H ='h2'", ECSqlStatus::Success, 5, 1);



    assert_ecsql("DELETE FROM bs.Foo WHERE ECInstanceId = 1 AND A = 101 AND B = 'b1';", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("DELETE FROM cs.Goo WHERE ECInstanceId = 1 AND A = 101 AND B = 'b1';", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assert_ecsql("DELETE FROM cs.Goo WHERE ECInstanceId = 1 AND (A = 101 AND B = 'b1') AND (C = 101 AND D = 'd1');", ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         10/05
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(JoinedTableECDbMapStrategyTests, InsertWithParameterBinding)
    {
    auto const schema =
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
        "                    <Options>JoinedTableForSubclasses</Options>"
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
        "</ECSchema>";

    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create("JoinedTableTest.ecdb");
    ECSchemaPtr joinedTableTestSchema;
    auto readContext = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(joinedTableTestSchema, schema, *readContext);
    ASSERT_TRUE(joinedTableTestSchema != nullptr);
    auto importStatus = db.Schemas().ImportECSchemas(readContext->GetCache());
    ASSERT_TRUE(importStatus == BentleyStatus::SUCCESS);

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
    auto const schema =
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
        "                    <Options>JoinedTableForSubclasses</Options>"
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
        "</ECSchema>";

    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create("JoinedTableTest.ecdb");
    ECSchemaPtr joinedTableTestSchema;
    auto readContext = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(joinedTableTestSchema, schema, *readContext);
    ASSERT_TRUE(joinedTableTestSchema != nullptr);
    auto importStatus = db.Schemas().ImportECSchemas(readContext->GetCache());
    ASSERT_TRUE(importStatus == BentleyStatus::SUCCESS);

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
    auto const schema =
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
        "                    <Options>JoinedTableForSubclasses</Options>"
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
        "</ECSchema>";

    ECDbTestProject saveTestProject;
    ECDbR db = saveTestProject.Create("JoinedTableTest.ecdb");
    ECSchemaPtr joinedTableTestSchema;
    auto readContext = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(joinedTableTestSchema, schema, *readContext);
    ASSERT_TRUE(joinedTableTestSchema != nullptr);
    auto importStatus = db.Schemas().ImportECSchemas(readContext->GetCache());
    ASSERT_TRUE(importStatus == BentleyStatus::SUCCESS);

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

END_ECDBUNITTESTS_NAMESPACE
