/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbMapping_Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "SchemaImportTestFixture.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, InvalidMapStrategyCATests)
    {
    std::vector<SchemaItem> testItems;
    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>bla</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ClassAB'>"
        "        <BaseClass>ClassA</BaseClass>"
        "    </ECEntityClass>"
        "</ECSchema>", false, "Invalid MapStrategy"));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA'>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "Option SharedColumns cannot be used without a strategy"));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>None</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "MapStrategy None not allowed"));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>", true, ""));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA'>"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "JoinedTablePerDirectSubclass cannot be used without a strategy"));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "sharedColumns not allowed with Strategy NotMapped"));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>None</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "sharedTable only allowed with SharedTable strategy"));

    AssertSchemaImport(testItems, "invalidmapstrategycatests.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                                   Krischan.Eberle   02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, UnsupportedNavigationPropertyCases)
    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "        <ECNavigationProperty propertyName='BIds' relationshipName='AHasB' direction='Forward' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='AHasB' strength='Referencing' modifier='Sealed'>"
        "      <Source cardinality='(0,1)' polymorphic='False'>"
        "          <Class class ='A' />"
        "      </Source>"
        "      <Target cardinality='(0,N)' polymorphic='False'>"
        "          <Class class ='B' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>", false, "NavigationProperty to 'Many' end of relationship is not supported"));

    testSchemas.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "        <ECNavigationProperty propertyName='AId' relationshipName='AHasB' direction='Backward' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='AHasB' strength='Referencing' modifier='Sealed'>"
        "      <Source cardinality='(0,N)' polymorphic='False'>"
        "          <Class class ='A' />"
        "      </Source>"
        "      <Target cardinality='(0,N)' polymorphic='False'>"
        "          <Class class ='B' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>", false, "NavigationProperty for link table relationships is not supported"));

    AssertSchemaImport(testSchemas, "invalidnavprops.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, OwnTableCATests)
    {
    std::vector<SchemaItem> testItems;
    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>OwnTable</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <ShareColumns xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", false, "Option SharedColumns can only be used with strategy SharedTable"));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>OwnTable</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", false, "Option JoinedTablePerDirectSubclass can only be used with strategy SharedTable (applied to subclasses)"));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='Class' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>OwnTable</MapStrategy>"
                                   "                <TableName>bla</TableName>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", false, "MapStrategy OwnTable doesn't allow TableName to be set."));

   
    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>OwnTable</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>", true, "NotMapped within Class Hierarchy is supported"));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ParentA' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>OwnTable</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ParentB' modifier='None'>"
        "        <BaseClass>ParentA</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>", true, "OwnTable allows a child class to have it's own strategy."));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>OwnTable</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Child' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>SharedTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>", true, "OwnTable allows a child class to have it's own strategy."));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>OwnTable</MapStrategy>"
        "                <TableName>bla</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "Own Table doesn't allows a user Defined table name."));

    AssertSchemaImport(testItems, "owntablecatests.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, TablePerHierarchyCATests)
    {
    std::vector<SchemaItem> testItems;
    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='Class' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "                <TableName>bla</TableName>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", false, "MapStrategy TablePerHierarchy doesn't allow TableName to be set."));


    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='Base' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='P0' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='Sub' modifier='None'>"
                                   "        <BaseClass>Base</BaseClass>"
                                   "        <ECProperty propertyName='P1' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='SubSub' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <BaseClass>Sub</BaseClass>"
                                   "        <ECProperty propertyName='P2' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", false, "MapStrategy TablePerHierarchy on child class where base has TablePerHierarchy is not supported."));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='Base1' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='P1' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='Base2' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='P2' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='Sub' modifier='None'>"
                                   "        <BaseClass>Base1</BaseClass>"
                                   "        <BaseClass>Base2</BaseClass>"
                                   "        <ECProperty propertyName='P3' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", false, "Child class has two base classes which both have MapStrategy TablePerHierarchy. This is not expected to be supported."));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSubSub' Modifier='None'>"
        "        <BaseClass>SubSub</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>", true, "MapStrategy NotMapped on child class where base has TablePerHierarchy is supported."));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TeststructClassInPolymorphicSharedTable' nameSpacePrefix='tph' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='BaseClass' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='p1' typeName='string' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='AbstractClass' modifier='Abstract'>"
                                   "        <BaseClass>BaseClass</BaseClass>"
                                   "        <ECProperty propertyName='p2' typeName='string' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", true, "Abstract Class in a Hierarchy with TablePerHierarchy CA is expected to be not supported."));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>", true, "TablePerHierarchy within Class Hierarchy is expected to be supported where Root class has default MapStrategy"));

    AssertSchemaImport(testItems, "tableperhierarchycatests.ecdb");

    {
    SchemaItem testSchema("<?xml version='1.0' encoding='utf-8'?>"
                          "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                          "    <ECEntityClass typeName='Parent' modifier='None'>"
                          "        <ECCustomAttributes>"
                          "            <ClassMap xmlns='ECDbMap.02.00'>"
                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                          "            </ClassMap>"
                          "        </ECCustomAttributes>"
                          "        <ECProperty propertyName='P1' typeName='int' />"
                          "    </ECEntityClass>"
                          "    <ECEntityClass typeName='Child1'>"
                          "        <BaseClass>Parent</BaseClass>"
                          "        <ECProperty propertyName='Price' typeName='double' />"
                          "    </ECEntityClass>"
                          "    <ECEntityClass typeName='Child2'>"
                          "        <BaseClass>Parent</BaseClass>"
                          "        <ECProperty propertyName='Price' typeName='double' />"
                          "    </ECEntityClass>"
                          "</ECSchema>");
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "tableperhierarchycatests.ecdb");
    ASSERT_FALSE(asserted);

    Utf8CP tableName = "ts_Parent";
    bvector<Utf8String> columnNames;
    ecdb.GetColumns(columnNames, tableName);
    ASSERT_EQ(4, columnNames.size()) << "Table " << tableName;
    ASSERT_TRUE(std::find(columnNames.begin(), columnNames.end(), "ECInstanceId") != columnNames.end()) << "Table " << tableName;
    ASSERT_TRUE(std::find(columnNames.begin(), columnNames.end(), "ECClassId") != columnNames.end()) << "Table " << tableName;
    ASSERT_TRUE(std::find(columnNames.begin(), columnNames.end(), "P1") != columnNames.end()) << "Table " << tableName;
    //As property type is the same, both Child1.Price and Child2.Price share the same column
    ASSERT_TRUE(std::find(columnNames.begin(), columnNames.end(), "Price") != columnNames.end()) << "Table " << tableName;
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, SharedTableCATests)
    {
    std::vector<SchemaItem> testItems;
    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='Class' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>SharedTable</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", false, "MapStrategy SharedTable expects TableName to be set."));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='Class' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>SharedTable</MapStrategy>"
                                   "                <TableName>idontexist</TableName>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", true, "MapStrategy SharedTable doesn't expect table specified in TableName to be set."));

    AssertSchemaImport(testItems, "sharedtablecatests.ecdb");
    }
//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, ExistingTableCATests)
    {
    std::vector<SchemaItem> testItems;
    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='Class' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>ExistingTable</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", false, "MapStrategy ExistingTable expects TableName to be set"));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>ExistingTable</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", false, "Option JoinedTablePerDirectSubclass can only be used with strategy SharedTable (applied to subclasses)"));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='Class' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>ExistingTable</MapStrategy>"
                                   "                <TableName>idontexist</TableName>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", false, "MapStrategy ExistingTable expects table specified by TableName to preexist"));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='BePropInfo' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>be_Prop</TableName>"
        "                <ECInstanceIdColumn>Id</ECInstanceIdColumn>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Namespace' typeName='string' />"
        "        <ECProperty propertyName='PropNotMappedToAnExistingCol' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "Cannot add new column to existing table"));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='BePropInfo' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>be_Prop</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Namespace' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "Cannot add ECInstanceId column to existing table"));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDb_FileInfo' version='02.00' prefix='ecdbf' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "   <ECRelationshipClass typeName='ParentHasEmbeddedFile' strength='Referencing' modifier='Sealed'>"
        "      <Source cardinality='(0,1)' polymorphic='False'>"
        "          <Class class ='Parent' />"
        "      </Source>"
        "      <Target cardinality='(0,N)' polymorphic='False'>"
        "          <Class class ='ecdb.EmbeddedFileInfo' />"
        "      </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>", false, "Cannot add FK column to existing table"));

    AssertSchemaImport(testItems, "existingtablecatests.ecdb");


    {
    ECDbCR ecdb = SetupECDb("existingtablecatests.ecdb");

    bmap<Utf8String, bool> testDataset;
    testDataset["SELECT * FROM ecdbf.ExternalFileInfo"] = true;
    testDataset["INSERT INTO ecdbf.ExternalFileInfo(Name) VALUES('Foo')"] = true;
    testDataset["UPDATE ecdbf.ExternalFileInfo SET Name='Foo' WHERE ECInstanceId=1"] = true;
    testDataset["DELETE FROM ecdbf.ExternalFileInfo"] = true;

    testDataset["SELECT * FROM ecdbf.EmbeddedFileInfo"] = true;
    testDataset["INSERT INTO ecdbf.EmbeddedFileInfo(Name) VALUES('Foo')"] = false;
    testDataset["UPDATE ecdbf.EmbeddedFileInfo SET Name='Foo' WHERE ECInstanceId=1"] = false;
    testDataset["DELETE FROM ecdbf.EmbeddedFileInfo"] = false;

    testDataset["SELECT * FROM ec.ECClassDef"] = true;
    testDataset["INSERT INTO ec.ECClassDef(SchemaId, Name, DisplayLabel) VALUES(1, 'Foo', 'Foo')"] = false;
    testDataset["UPDATE ec.ECClassDef SET DisplayLabel='Foo' WHERE ECInstanceId=1"] = false;
    testDataset["DELETE FROM ec.ECClassDef"] = false;

    for (bpair<Utf8String, bool> const& testItem : testDataset)
        {
        const bool expectedSuccess = testItem.second;
        Utf8CP ecsql = testItem.first.c_str();
        ECSqlStatement stmt;
        const ECSqlStatus stat = stmt.Prepare(ecdb, ecsql);

        if (expectedSuccess)
            ASSERT_EQ(ECSqlStatus::Success, stat) << ecsql;
        else
            ASSERT_EQ(ECSqlStatus::InvalidECSql, stat) << ecsql;
        }

    ECClassCP testClass = ecdb.Schemas().GetECClass("MetaSchema", "ECClassDef");
    ASSERT_TRUE(testClass != nullptr);

    {
    ECInstanceInserter inserter(ecdb, *testClass);
    ASSERT_FALSE(inserter.IsValid());
    ECInstanceUpdater updater(ecdb, *testClass);
    ASSERT_FALSE(updater.IsValid());
    ECInstanceDeleter deleter(ecdb, *testClass);
    ASSERT_FALSE(deleter.IsValid());
    }

    {
    JsonInserter inserter(ecdb, *testClass);
    ASSERT_FALSE(inserter.IsValid());
    JsonUpdater updater(ecdb, *testClass);
    ASSERT_FALSE(updater.IsValid());
    JsonDeleter deleter(ecdb, *testClass);
    ASSERT_FALSE(deleter.IsValid());
    }
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, NotMappedCATests)
    {
    std::vector<SchemaItem> invalidSchemas;
    invalidSchemas.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                        "    <ECEntityClass typeName='Class' modifier='None'>"
                                        "        <ECCustomAttributes>"
                                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                                        "                <MapStrategy>NotMapped</MapStrategy>"
                                        "                <TableName>bla</TableName>"
                                        "            </ClassMap>"
                                        "        </ECCustomAttributes>"
                                        "        <ECProperty propertyName='Price' typeName='double' />"
                                        "    </ECEntityClass>"
                                        "</ECSchema>", false, "MapStrategy NotMapped doesn't allow TableName to be set."));

    invalidSchemas.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "Conflicting MapStrategy TablePerHierarchy within Class Hierarchy not supported where Root has Strategy NotMapped"));

    invalidSchemas.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>OwnTable</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "Conflicting mapStrategies OwnTable within Class Hierarchy not supported where Root has MapStrategy NotMapped"));

    invalidSchemas.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='A' modifier='None'>"
        "        <ECProperty propertyName='AProp' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B' modifier='None'>"
        "        <ECProperty propertyName='BProp' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "               <MapStrategy>OwnTable</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='B' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>", false, "ECRelationshipClass with FK mapping must not have a ClassMap CA unless it has MapStrategy NotMapped"));

    AssertSchemaImport(invalidSchemas, "notmappedcatests.ecdb");

    {
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSubSub' modifier='None'>"
        "        <BaseClass>SubSub</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>", true, "NotMapped on subclass"), "notmappedcatests.ecdb");
    ASSERT_FALSE(asserted);

    MapStrategyInfo mapStrategy;
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "Sub")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::OwnTable, mapStrategy.m_strategy);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "SubSub")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "SubSubSub")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

    }

    {
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>", true, "NotMapped within Class Hierarchy is expected to be supported where Root class has default MapStrategy"),
        "notmappedcatests.ecdb");
    ASSERT_FALSE(asserted);
    }

    {
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                  "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                  "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                  "    <ECEntityClass typeName='Base' modifier='None'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>NotMapped</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <ECProperty propertyName='P0' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='Sub' modifier='None'>"
                                                  "        <BaseClass>Base</BaseClass>"
                                                  "        <ECProperty propertyName='P1' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='SubSub' modifier='None'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>NotMapped</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <BaseClass>Sub</BaseClass>"
                                                  "        <ECProperty propertyName='P2' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='SubSubSub' modifier='None'>"
                                                  "        <BaseClass>SubSub</BaseClass>"
                                                  "        <ECProperty propertyName='P3' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "</ECSchema>", false, "NotMapped cannot be set on subclass if base class already defined it"),
                       "notmappedcatests.ecdb");
    ASSERT_FALSE(asserted);
    }

    {
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                  "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                  "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                  "    <ECEntityClass typeName='Base' modifier='None'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>NotMapped</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <ECProperty propertyName='P0' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='Sub' modifier='None'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <BaseClass>Base</BaseClass>"
                                                  "        <ECProperty propertyName='P1' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='SubSub' modifier='None'>"
                                                  "        <BaseClass>Sub</BaseClass>"
                                                  "        <ECProperty propertyName='P2' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "</ECSchema>", false, "SharedTable(polymorphic) within Class Hierarchy is not supported where Root has Strategy NotMapped"),
                       "notmappedcatests.ecdb");
    ASSERT_FALSE(asserted);
    }

    {
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                  "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                  "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                  "    <ECEntityClass typeName='Base' modifier='None'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>NotMapped</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <ECProperty propertyName='P0' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='Sub' modifier='None'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>OwnTable</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <BaseClass>Base</BaseClass>"
                                                  "        <ECProperty propertyName='P1' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='SubSub' modifier='None'>"
                                                  "        <BaseClass>Sub</BaseClass>"
                                                  "        <ECProperty propertyName='P2' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "</ECSchema>", false, "OwnTable) within Class Hierarchy is not supported where Root has Strategy NotMapped"),
                       "notmappedcatests.ecdb");
    ASSERT_FALSE(asserted);
    }

    {
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                  "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                  "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                  "    <ECEntityClass typeName='Base' modifier='None'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>NotMapped</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <ECProperty propertyName='P0' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='Sub' modifier='None'>"
                                                  "        <BaseClass>Base</BaseClass>"
                                                  "        <ECProperty propertyName='P1' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='SubSub' modifier='None'>"
                                                  "        <BaseClass>Sub</BaseClass>"
                                                  "        <ECProperty propertyName='P2' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='SubSubSub' modifier='None'>"
                                                  "        <BaseClass>SubSub</BaseClass>"
                                                  "        <ECProperty propertyName='P3' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "</ECSchema>", true, "NotMapped applied to non-sealed classes means to apply to subclasses, too"),
                       "notmappedcatests.ecdb");
    ASSERT_FALSE(asserted);

    MapStrategyInfo mapStrategy;
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "Base")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "Sub")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "SubSub")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "SubSubSub")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);

    }

    {
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                  "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                  "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                  "    <ECEntityClass typeName='Base' modifier='None'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>NotMapped</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <ECProperty propertyName='P0' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='Sub' modifier='None'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>SharedTable</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <BaseClass>Base</BaseClass>"
                                                  "        <ECProperty propertyName='P1' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='SubSub' modifier='None'>"
                                                  "        <BaseClass>Sub</BaseClass>"
                                                  "        <ECProperty propertyName='P2' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "</ECSchema>", false, "SharedTable cannot be applied to subclass if base class has NotMapped"),
                       "notmappedcatests.ecdb");
    ASSERT_FALSE(asserted);
    }

    {
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                  "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                  "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                  "    <ECEntityClass typeName='Base' modifier='Abstract'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <ECProperty propertyName='P0' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='Sub' modifier='Sealed'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>NotMapped</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "        <BaseClass>Base</BaseClass>"
                                                  "        <ECProperty propertyName='P1' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "</ECSchema>", true),
                       "notmappedcatests.ecdb");
    ASSERT_FALSE(asserted);

    MapStrategyInfo mapStrategy;
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "Base")->GetId()));
    ASSERT_EQ((int) MapStrategyInfo::Strategy::TablePerHierarchy, (int) mapStrategy.m_strategy);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "Sub")->GetId()));
    ASSERT_EQ((int) MapStrategyInfo::Strategy::NotMapped, (int) mapStrategy.m_strategy);
    }

    {
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                  "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                  "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                  "    <ECEntityClass typeName='A' modifier='None'>"
                                                  "        <ECProperty propertyName='AProp' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='B' modifier='None'>"
                                                  "        <ECProperty propertyName='BProp' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>NotMapped</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "       <Source cardinality='(0,1)' polymorphic='True'>"
                                                  "           <Class class='A' />"
                                                  "       </Source>"
                                                  "       <Target cardinality='(0,N)' polymorphic='True'>"
                                                  "           <Class class='B' />"
                                                  "       </Target>"
                                                  "     </ECRelationshipClass>"
                                                  "</ECSchema>", true, "ECRelationshipClass with FK mapping can have a ClassMap CA with MapStrategy NotMapped"),
                       "notmappedcatests.ecdb");
    ASSERT_FALSE(asserted);

    MapStrategyInfo mapStrategy;
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "Rel")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);
    }

    {
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                  "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                  "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                  "    <ECEntityClass typeName='A' modifier='None'>"
                                                  "        <ECProperty propertyName='AProp' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='B' modifier='None'>"
                                                  "        <ECProperty propertyName='BProp' typeName='int' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                                                  "        <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>NotMapped</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "        </ECCustomAttributes>"
                                                  "       <Source cardinality='(0,1)' polymorphic='True'>"
                                                  "           <Class class='A' />"
                                                  "       </Source>"
                                                  "       <Target cardinality='(0,N)' polymorphic='True'>"
                                                  "           <Class class='B' />"
                                                  "       </Target>"
                                                  "     </ECRelationshipClass>"
                                                  "</ECSchema>", true, "ECRelationshipClass with FK mapping can have a ClassMap CA with MapStrategy NotMapped"),
                       "notmappedcatests.ecdb");
    ASSERT_FALSE(asserted);
    MapStrategyInfo mapStrategy;
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetECClass("Test", "Rel")->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);
    }

    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, JoinedTableCATests)
    {
    std::vector<SchemaItem> testItems;
    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", true, "Option JoinedTablePerDirectSubclass is expected to work with strategy SharedTable (applied to subclasses)"));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <ShareColumns xmlns='ECDbMap.02.00'>"
                                   "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                                   "            </ShareColumns>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", true, "Combination of options JoinedTablePerDirectSubclass and SharedColumnsForSubclasses is expected to work with strategy SharedTable (applied to subclasses)"));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <ShareColumns xmlns='ECDbMap.02.00'/>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", true, "Combination of options JoinedTablePerDirectSubclass and SharedColumns is expected to work with strategy SharedTable (applied to subclasses)"));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <ShareColumns xmlns='ECDbMap.02.00'>"
                                   "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                                   "            </ShareColumns>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", true, "Combination of options SharedColumnsForSubclasses and JoinedTablePerDirectSubclass is expected to work with strategy SharedTable (applied to subclasses) and with SharedColumnsForSubclasses"));

    std::vector<Utf8String> testSchemas;
    testSchemas.push_back("<?xml version='1.0' encoding='utf-8'?>"
                          "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                          "    <ECEntityClass typeName='ClassA' modifier='None'>"
                          "        <ECCustomAttributes>"
                          "            <ClassMap xmlns='ECDbMap.02.00'>"
                          "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                          "            </ClassMap>"
                          "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                          "        </ECCustomAttributes>"
                          "        <ECProperty propertyName='Price' typeName='double' />"
                          "    </ECEntityClass>"
                          "    <ECEntityClass typeName='ClassB' modifier='None'>"
                          "        <BaseClass>ClassA</BaseClass>"
                          "        <ECProperty propertyName='Cost' typeName='double' />"
                          "    </ECEntityClass>"
                          "</ECSchema>");

    testSchemas.push_back("<?xml version='1.0' encoding='utf-8'?>"
                          "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                          "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts' />"
                          "    <ECEntityClass typeName='ClassC' modifier='None'>"
                          "        <ECCustomAttributes>"
                          "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                          "        </ECCustomAttributes>"
                          "        <BaseClass>ts:ClassB</BaseClass>"
                          "        <ECProperty propertyName='Name' typeName='string' />"
                          "    </ECEntityClass>"
                          "</ECSchema>");
    testItems.push_back(SchemaItem(testSchemas, false, "JoinedTablePerDirectSubclass cannot be applied if it was already specified higher up in the hierarchy"));
    testSchemas.clear();

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>SharedTable</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Price' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", false, "Option JoinedTablePerDirectSubclass can only be used with strategy SharedTable"));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSub' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSubSub' modifier='None'>"
        "        <BaseClass>SubSub</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>", true, "Option JoinedTablePerDirectSubclass can be applied to subclass where base has TablePerHierarchy."));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='ClassA' modifier='None'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                   "        </ECCustomAttributes>"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='ClassB' modifier='None'>"
                                   "        <BaseClass>ClassA</BaseClass>"
                                   "        <ECProperty propertyName='Cost' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>", true, "JoinedTable on a class without any property is expected to be successful"));

    AssertSchemaImport(testItems, "joinedtablecatests.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, ForeignKeyMapCATests)
    {
    std::vector<SchemaItem> testItems;
    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='A' modifier='None'>"
                                   "        <ECProperty propertyName='AA' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='B' modifier='None'>"
                                   "        <ECProperty propertyName='BB' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                                   "        <ECCustomAttributes>"
                                   "            <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
                                   "            </ForeignKeyRelationshipMap>"
                                   "        </ECCustomAttributes>"
                                   "       <Source cardinality='(0,N)' polymorphic='True'>"
                                   "           <Class class='A' />"
                                   "       </Source>"
                                   "       <Target cardinality='(0,N)' polymorphic='True'>"
                                   "           <Class class='B' />"
                                   "       </Target>"
                                   "     </ECRelationshipClass>"
                                   "</ECSchema>", false, "ForeignKeyRelationshipMap on N:N relationship is not supported"));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='A' modifier='None'>"
                                   "        <ECProperty propertyName='AA' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='B' modifier='None'>"
                                   "        <ECProperty propertyName='BB' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='embedding'>"
                                   "        <ECCustomAttributes>"
                                   "            <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
                                   "               <OnDeleteAction>Cascade</OnDeleteAction>"
                                   "            </ForeignKeyRelationshipMap>"
                                   "        </ECCustomAttributes>"
                                   "       <Source cardinality='(0,1)' polymorphic='True'>"
                                   "           <Class class='A' />"
                                   "       </Source>"
                                   "       <Target cardinality='(0,N)' polymorphic='True'>"
                                   "           <Class class='B' />"
                                   "       </Target>"
                                   "     </ECRelationshipClass>"
                                   "</ECSchema>", true, "Cascading delete only supported for embedding relationships"));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='A' modifier='None'>"
                                   "        <ECProperty propertyName='AA' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='B' modifier='None'>"
                                   "        <ECProperty propertyName='BB' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                                   "        <ECCustomAttributes>"
                                   "            <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
                                   "               <OnDeleteAction>Cascade</OnDeleteAction>"
                                   "            </ForeignKeyRelationshipMap>"
                                   "        </ECCustomAttributes>"
                                   "       <Source cardinality='(0,1)' polymorphic='True'>"
                                   "           <Class class='A' />"
                                   "       </Source>"
                                   "       <Target cardinality='(0,N)' polymorphic='True'>"
                                   "           <Class class='B' />"
                                   "       </Target>"
                                   "     </ECRelationshipClass>"
                                   "</ECSchema>", false, "Cascading delete only supported for embedding relationships"));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='A' modifier='None'>"
                                   "        <ECProperty propertyName='AA' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "    <ECEntityClass typeName='B' modifier='None'>"
                                   "        <ECProperty propertyName='BB' typeName='double' />"
                                   "    </ECEntityClass>"
                                   "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='holding'>"
                                   "        <ECCustomAttributes>"
                                   "            <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
                                   "               <OnDeleteAction>Cascade</OnDeleteAction>"
                                   "            </ForeignKeyRelationshipMap>"
                                   "        </ECCustomAttributes>"
                                   "       <Source cardinality='(0,1)' polymorphic='True'>"
                                   "           <Class class='A' />"
                                   "       </Source>"
                                   "       <Target cardinality='(0,N)' polymorphic='True'>"
                                   "           <Class class='B' />"
                                   "       </Target>"
                                   "     </ECRelationshipClass>"
                                   "</ECSchema>", false, "Cascading delete only supported for embedding relationships"));

    AssertSchemaImport(testItems, "sharedtablecatests.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, SharedColumnCA)
    {
    std::vector<SchemaItem> testItems;
    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "Defining ShareColumns multiple times in class hierarchy is expected to fail"));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "Defining ShareColumns multiple times in class hierarchy is expected to fail"));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "Defining ShareColumns multiple times in class hierarchy is expected to fail"));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>100</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>", true, ""));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>100</SharedColumnCount>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "SharedColumnCount can only be defined on first occurrence of SharedColumn option in a hierarchy"));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>100</SharedColumnCount>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "SharedColumnCount can only be defined on first occurrence of SharedColumn option in a hierarchy"));

    testItems.push_back(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>100</SharedColumnCount>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>", false, "SharedColumnCount can only be defined on first occurrence of SharedColumn option in a hierarchy"));

    AssertSchemaImport(testItems, "sharedtablecatests.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Krischan.Eberle                  05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, SharedColumnAndPerColumnConstraints)
    {
    SchemaItem testSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Child' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='NotNullableProp' typeName='double'>"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "                <IsNullable>false</IsNullable>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECProperty>"
        "        <ECProperty propertyName='NullableProp' typeName='string'>"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "                <IsNullable>true</IsNullable>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECProperty>"
        "        <ECProperty propertyName='UniqueProp' typeName='string'>"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "                <IsUnique>true</IsUnique>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECProperty>"
        "        <ECProperty propertyName='NotUniqueProp' typeName='string'>"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "                <IsUnique>false</IsUnique>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECProperty>"
        "        <ECProperty propertyName='CollationBinaryProp' typeName='string'>"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "                <Collation>Binary</Collation>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECProperty>"
        "        <ECProperty propertyName='CollationNoCaseProp' typeName='string'>"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "                <Collation>NoCase</Collation>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECProperty>"
        "        <ECProperty propertyName='CollationRTrimProp' typeName='string'>"
        "           <ECCustomAttributes>"
        "            <PropertyMap xmlns='ECDbMap.02.00'>"
        "                <Collation>RTrim</Collation>"
        "            </PropertyMap>"
        "           </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "</ECSchema>", true, "Column constraints on property that maps to shared column");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchema, "sharedcolumnandcolumnconstraints.ecdb");
    ASSERT_FALSE(asserted);

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT sql FROM sqlite_master WHERE name='ts_Parent'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Did not find table ts_Parent";

    bvector<Utf8String> columnDdlList;
    BeStringUtilities::Split(stmt.GetValueText(0), ",", columnDdlList);
    stmt.Finalize();

    for (Utf8StringR columnDdl : columnDdlList)
        {
        columnDdl.ToLower();

        if (!columnDdl.StartsWithI("[sc") || columnDdl.StartsWithI("sc"))
            continue;

        ASSERT_TRUE(columnDdl.find("not null") == columnDdl.npos) << columnDdl.c_str();
        ASSERT_TRUE(columnDdl.find("unique") == columnDdl.npos) << columnDdl.c_str();
        ASSERT_TRUE(columnDdl.find("collat") == columnDdl.npos) << columnDdl.c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, SharedColumnCount)
    {
            {
            ECDbR ecdb = SetupECDb("sharedcolumncount.ecdb", SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                "   <ECEntityClass typeName='Parent' modifier='None' >"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "            </ClassMap>"
                "            <ShareColumns xmlns='ECDbMap.02.00'>"
                "              <SharedColumnCount>5</SharedColumnCount>"
                "            </ShareColumns>"
                "        </ECCustomAttributes>"
                "       <ECProperty propertyName='P1' typeName='int' />"
                "   </ECEntityClass>"
                "</ECSchema>"));
            ASSERT_TRUE(ecdb.IsDbOpen());
            ecdb.SaveChanges();

            std::vector<std::pair<Utf8String, int>> testItems;
            testItems.push_back(std::make_pair("ts_Parent", 7));
            AssertColumnCount(ecdb, testItems, "SharedColumnCount");
            }

            {
            ECDbR ecdb = SetupECDb("sharedcolumncount.ecdb", SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "   <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                "   <ECEntityClass typeName='Parent' modifier='None' >"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "            </ClassMap>"
                "            <ShareColumns xmlns='ECDbMap.02.00'>"
                "              <SharedColumnCount>5</SharedColumnCount>"
                "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                "            </ShareColumns>"
                "        </ECCustomAttributes>"
                "       <ECProperty propertyName='P1' typeName='int' />"
                "   </ECEntityClass>"
                "</ECSchema>"));
            ASSERT_TRUE(ecdb.IsDbOpen());
            ecdb.SaveChanges();

            std::vector<std::pair<Utf8String, int>> testItems;
            testItems.push_back(std::make_pair("ts_Parent", 3));
            AssertColumnCount(ecdb, testItems, "SharedColumnCount");
            }

            {
            ECDbR ecdb = SetupECDb("sharedcolumncount.ecdb", SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='Parent' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "            </ClassMap>"
                "            <ShareColumns xmlns='ECDbMap.02.00'>"
                "              <SharedColumnCount>100</SharedColumnCount>"
                "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                "            </ShareColumns>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='Price' typeName='double' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='Sub1' modifier='None'>"
                "        <BaseClass>Parent</BaseClass>"
                "        <ECProperty propertyName='Cost' typeName='double' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='Sub2' modifier='None'>"
                "        <BaseClass>Parent</BaseClass>"
                "        <ECProperty propertyName='DoubleProp' typeName='double' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='Sub11' modifier='None'>"
                "        <BaseClass>Sub1</BaseClass>"
                "        <ECProperty propertyName='Diameter' typeName='double' />"
                "    </ECEntityClass>"
                "</ECSchema>"));

            ASSERT_TRUE(ecdb.IsDbOpen());
            ecdb.SaveChanges();

            std::vector<std::pair<Utf8String, int>> testItems;
            testItems.push_back(std::make_pair("ts_Parent", 103));
            AssertColumnCount(ecdb, testItems, "After first schema import");

            SchemaItem secondSchema(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts' />"
                "    <ECEntityClass typeName='Sub3'>"
                "        <BaseClass>ts:Parent</BaseClass>"
                "        <ECProperty propertyName='Prop3' typeName='double' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='Sub111'>"
                "        <BaseClass>ts:Sub11</BaseClass>"
                "        <ECProperty propertyName='Prop111' typeName='double' />"
                "    </ECEntityClass>"
                "</ECSchema>");

            bool asserted = false;
            AssertSchemaImport(asserted, ecdb, secondSchema);
            ASSERT_FALSE(asserted);

            AssertColumnCount(ecdb, testItems, "After second schema import");
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, SharedColumnCountWithJoinedTable)
    {
    ECDbR ecdb = SetupECDb("sharedcolumncount.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>100</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='Cost' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECProperty propertyName='DoubleProp' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='Diameter' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ecdb.SaveChanges();

    std::vector<std::pair<Utf8String, int>> testItems;
    testItems.push_back(std::make_pair("ts_Parent", 3));
    testItems.push_back(std::make_pair("ts_Sub1", 102));
    testItems.push_back(std::make_pair("ts_Sub2", 102));
    AssertColumnCount(ecdb, testItems, "After first schema import");

    SchemaItem secondSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts' />"
        "    <ECEntityClass typeName='Sub3'>"
        "        <BaseClass>ts:Parent</BaseClass>"
        "        <ECProperty propertyName='Prop3' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub111'>"
        "        <BaseClass>ts:Sub11</BaseClass>"
        "        <ECProperty propertyName='Prop111' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, secondSchema);
    ASSERT_FALSE(asserted);

    testItems.push_back(std::make_pair("ts2_Sub3", 102));
    AssertColumnCount(ecdb, testItems, "After second schema import");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, SharedColumnCountBisScenario)
    {
    ECDbR ecdb = SetupECDb("minimumsharedcolumncount.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='InformationElement' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='DefinitionElement' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>50</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <BaseClass>InformationElement</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='GeometricElement' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Element</BaseClass>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='GeometricElement2d' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <BaseClass>GeometricElement</BaseClass>"
        "        <ECProperty propertyName='GeometryStream' typeName='binary' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='GeometricElement3d' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>5</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <BaseClass>GeometricElement</BaseClass>"
        "        <ECProperty propertyName='GeometryStream' typeName='binary' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub1' modifier='None'>"
        "        <BaseClass>DefinitionElement</BaseClass>"
        "        <ECProperty propertyName='Sub1Prop1' typeName='double' />"
        "        <ECProperty propertyName='Sub1Prop2' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub11' modifier='None'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='Sub11Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub2' modifier='None'>"
        "        <BaseClass>GeometricElement2d</BaseClass>"
        "        <ECProperty propertyName='Sub2Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub3' modifier='None'>"
        "        <BaseClass>GeometricElement3d</BaseClass>"
        "        <ECProperty propertyName='Sub3Prop1' typeName='double' />"
        "        <ECProperty propertyName='Sub3Prop2' typeName='double' />"
        "        <ECProperty propertyName='Sub3Prop3' typeName='double' />"
        "        <ECProperty propertyName='Sub3Prop4' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ecdb.SaveChanges();

    const int elementExpectedColCount = 3;
    const int definitionElementExpectedColCount = 52;
    int geometricElement2dExpectedColCount = 4;
    int geometricElement3dExpectedColCount = 8;

    std::vector<std::pair<Utf8String, int>> testItems;
    testItems.push_back(std::make_pair("ts_Element", elementExpectedColCount));
    testItems.push_back(std::make_pair("ts_DefinitionElement", definitionElementExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement2d", geometricElement2dExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement3d", geometricElement3dExpectedColCount));

    AssertColumnCount(ecdb, testItems, "after first schema import");

    SchemaItem secondSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts' />"
        "    <ECEntityClass typeName='Sub4'>"
        "        <BaseClass>ts:InformationElement</BaseClass>"
        "        <ECProperty propertyName='Sub4Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub21'>"
        "        <BaseClass>ts:Sub2</BaseClass>"
        "        <ECProperty propertyName='Sub21Prop' typeName='double' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub31'>"
        "        <BaseClass>ts:Sub3</BaseClass>"
        "        <ECProperty propertyName='Sub31Prop1' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, secondSchema);
    ASSERT_FALSE(asserted);

    geometricElement2dExpectedColCount++;
    const int sub4ExpectedColCount = 3;

    testItems.clear();
    testItems.push_back(std::make_pair("ts_Element", elementExpectedColCount));
    testItems.push_back(std::make_pair("ts_DefinitionElement", definitionElementExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement2d", geometricElement2dExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement3d", geometricElement3dExpectedColCount));
    testItems.push_back(std::make_pair("ts2_Sub4", sub4ExpectedColCount));

    AssertColumnCount(ecdb, testItems, "after second schema import");

    SchemaItem thirdSchema(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema3' nameSpacePrefix='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts' />"
        "    <ECEntityClass typeName='Sub32'>"
        "        <BaseClass>ts:Sub3</BaseClass>"
        "        <ECProperty propertyName='Sub32Prop1' typeName='double' />"
        "        <ECProperty propertyName='Sub32Prop2' typeName='double' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    asserted = false;
    AssertSchemaImport(asserted, ecdb, thirdSchema);
    ASSERT_FALSE(asserted);

    geometricElement3dExpectedColCount++;

    testItems.clear();
    testItems.push_back(std::make_pair("ts_Element", elementExpectedColCount));
    testItems.push_back(std::make_pair("ts_DefinitionElement", definitionElementExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement2d", geometricElement2dExpectedColCount));
    testItems.push_back(std::make_pair("ts_GeometricElement3d", geometricElement3dExpectedColCount));
    testItems.push_back(std::make_pair("ts2_Sub4", sub4ExpectedColCount));

    AssertColumnCount(ecdb, testItems, "after third schema import");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, TablePrefix)
    {
            {
            SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                                "<ECSchema schemaName='test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                "        <ECCustomAttributes>"
                                "            <SchemaMap xmlns='ECDbMap.02.00'>"
                                "                <TablePrefix>myownprefix</TablePrefix>"
                                "            </SchemaMap>"
                                "        </ECCustomAttributes>"
                                "    <ECEntityClass typeName='A' modifier='None'>"
                                "        <ECProperty propertyName='P1' typeName='string' />"
                                "    </ECEntityClass>"
                                "    <ECEntityClass typeName='B' modifier='None'>"
                                "        <ECProperty propertyName='P2' typeName='string' />"
                                "    </ECEntityClass>"
                                "</ECSchema>",
                                true, "");

            ECDb db;
            bool asserted = false;
            AssertSchemaImport(db, asserted, testItem, "tableprefix.ecdb");
            ASSERT_FALSE(asserted);

            //verify tables
            ASSERT_TRUE(db.TableExists("myownprefix_A"));
            ASSERT_TRUE(db.TableExists("myownprefix_B"));
            ASSERT_FALSE(db.TableExists("ts_A"));
            ASSERT_FALSE(db.TableExists("ts_B"));
            }

            {
            SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                                "<ECSchema schemaName='test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                "        <ECCustomAttributes>"
                                "            <SchemaMap xmlns='ECDbMap.02.00'>"
                                "            </SchemaMap>"
                                "        </ECCustomAttributes>"
                                "    <ECEntityClass typeName='A' modifier='None'>"
                                "        <ECProperty propertyName='P1' typeName='string' />"
                                "    </ECEntityClass>"
                                "    <ECEntityClass typeName='B' modifier='None'>"
                                "        <ECProperty propertyName='P2' typeName='string' />"
                                "    </ECEntityClass>"
                                "</ECSchema>",
                                true, "");

            ECDb db;
            bool asserted = false;
            AssertSchemaImport(db, asserted, testItem, "tableprefix.ecdb");
            ASSERT_FALSE(asserted);

            //verify tables
            ASSERT_TRUE(db.TableExists("ts_A"));
            ASSERT_TRUE(db.TableExists("ts_B"));
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, SharedColumnsAcrossMultipleSchemaImports)
    {
    SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='ReferredSchema' nameSpacePrefix='rs' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='Base' modifier='None'>"
                        "        <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                        "            </ClassMap>"
                        "            <ShareColumns xmlns='ECDbMap.02.00'>"
                        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                        "            </ShareColumns>"
                        "        </ECCustomAttributes>"
                        "        <ECProperty propertyName='P0' typeName='int' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='Sub1' modifier='None'>"
                        "         <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='P1' typeName='int' />"
                        "    </ECEntityClass>"
                        "</ECSchema>", true, "Mapstrategy Option SharedColumnForSubClasses (applied to subclasses) is expected to succeed");

    SchemaItem secondTestItem("<?xml version='1.0' encoding='utf-8'?>"
                              "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                              "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                              "    <ECSchemaReference name='ReferredSchema' version='01.00' prefix='rs' />"
                              "    <ECEntityClass typeName='Sub2' modifier='None'>"
                              "         <BaseClass>rs:Sub1</BaseClass>"
                              "        <ECProperty propertyName='P2' typeName='int' />"
                              "    </ECEntityClass>"
                              "</ECSchema>", true, "Mapstrategy Option SharedColumnForSubClasses (applied to subclasses) is expected to be honored from base Class of Refered schema");

    Utf8String ecdbFilePath;
    {
    ECDb testDb;
    bool asserted = false;
    AssertSchemaImport(testDb, asserted, testItem, "SharedColumnsForSubclasses.ecdb");
    ASSERT_FALSE(asserted);
    testDb.SaveChanges();
    ecdbFilePath = testDb.GetDbFileName();
    }

    ECDb testDb;
    ASSERT_EQ(BE_SQLITE_OK, testDb.OpenBeSQLiteDb(ecdbFilePath.c_str(), ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    bool asserted = false;
    AssertSchemaImport(asserted, testDb, secondTestItem);
    ASSERT_FALSE(asserted);

    //verify Number and Names of Columns in BaseClass
    BeSQLite::Statement statement;
    const int expectedColCount = 5;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, statement.Prepare(testDb, "SELECT * FROM rs_Base"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    ASSERT_EQ(expectedColCount, statement.GetColumnCount());

    //verify that the columns generated are same as expected
    Utf8String expectedColumnNames = "ECInstanceIdECClassIdP0sc01sc02";
    Utf8String actualColumnNames;
    for (int i = 0; i < expectedColCount; i++)
        {
        actualColumnNames.append(statement.GetColumnName(i));
        }
    ASSERT_STREQ(expectedColumnNames.c_str(), actualColumnNames.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, AbstractClass)
    {
    std::vector<SchemaItem> testItems;
    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='AbstractClassTest' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='AbstractClass' modifier='Abstract'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Prop' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>",
                                   true, "Abstract class with TablePerHierarchy MapStrategy"));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='AbstractClassTest' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='AbstractClass' modifier='Abstract'>"
                                   "        <ECProperty propertyName='Prop' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>",
                                   true, "Abstract class with no MapStrategy specified"));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='AbstractClassTest' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='AbstractClass' modifier='Abstract'>"
                        "        <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>SharedTable</MapStrategy>"
                        "                <TableName>MyName</TableName>"
                        "            </ClassMap>"
                        "        </ECCustomAttributes>"
                        "        <ECProperty propertyName='Prop' typeName='int' />"
                        "    </ECEntityClass>"
                        "</ECSchema>",
                        false, "Abstract class can only have TablePerHierarchy or NotMapped MapStrategy"));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='AbstractClassTest' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='AbstractClass' modifier='Abstract'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>ExistingTable</MapStrategy>"
                                   "                <TableName>MyName</TableName>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Prop' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>",
                                   false, "Abstract class can only have TablePerHierarchy MapStrategy"));

    testItems.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                   "<ECSchema schemaName='AbstractClassTest' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                   "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                   "    <ECEntityClass typeName='AbstractClass' modifier='Abstract'>"
                                   "        <ECCustomAttributes>"
                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                   "                <MapStrategy>OwnTable</MapStrategy>"
                                   "            </ClassMap>"
                                   "        </ECCustomAttributes>"
                                   "        <ECProperty propertyName='Prop' typeName='int' />"
                                   "    </ECEntityClass>"
                                   "</ECSchema>",
                                   false, "Abstract class can only have TablePerHierarchy MapStrategy"));

    AssertSchemaImport(testItems, "abstractclass_mapstrategy.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, ForeignKeyMapCreateIndex)
    {
    SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                        "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                        "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                        "  <ECEntityClass typeName='Parent' modifier='None'>"
                        "    <ECProperty propertyName='Name' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECEntityClass typeName='Child' modifier='None'>"
                        "    <ECProperty propertyName='ParentId' typeName='long' />"
                        "    <ECProperty propertyName='ChildName' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECRelationshipClass typeName='RelCreateIndexTrue' modifier='Sealed' strength='referencing'>"
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00' />"
                        "    </ECCustomAttributes>"
                        "    <Source cardinality='(1,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'Child' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  <ECRelationshipClass typeName='RelCreateIndexFalse' modifier='Sealed' strength='referencing'>"
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00' >"
                        "           <CreateIndex>False</CreateIndex>"
                        "        </ForeignKeyRelationshipMap>"
                        "    </ECCustomAttributes>"
                        "    <Source cardinality='(1,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'Child' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  <ECRelationshipClass typeName='RelCreateIndexDefaultValue' modifier='Sealed' strength='referencing'>"
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00' />"
                        "    </ECCustomAttributes>"
                        "    <Source cardinality='(1,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'Child' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>", true, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "foreignkeymapcreateindex.ecdb");
    ASSERT_FALSE(asserted);

    AssertIndex(ecdb, "ix_ts_Child_fk_ts_RelCreateIndexTrue_target", false, "ts_Child", {"ForeignECInstanceId_ts_RelCreateIndexTrue"});
    AssertIndex(ecdb, "ix_ts_Child_fk_ts_RelCreateIndexDefaultValue_target", false, "ts_Child", {"ForeignECInstanceId_ts_RelCreateIndexDefaultValue"});
    AssertIndexExists(ecdb, "ix_ts_Child_fk_ts_RelCreateIndexFalse_target", false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, PropertyWithSameNameAsStructMemberColumn)
    {
            {
            SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                                "  <ECStructClass typeName='ElementCode' modifier='None'>"
                                "    <ECProperty propertyName='Name' typeName='string' />"
                                "  </ECStructClass>"
                                "  <ECEntityClass typeName='Foo' modifier='None'>"
                                "    <ECProperty propertyName='Code_Name' typeName='string' />"
                                "    <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                                "  </ECEntityClass>"
                                "</ECSchema>", true, "");

            ECDb ecdb;
            bool asserted = false;
            AssertSchemaImport(ecdb, asserted, testItem, "propertywithsamenameasstructmembercol.ecdb");
            ASSERT_FALSE(asserted);

            ASSERT_TRUE(ecdb.ColumnExists("ts_Foo", "Code_Name"));

            ECClassId fooClassId = ecdb.Schemas().GetECClassId("TestSchema", "Foo");
            ASSERT_TRUE(fooClassId.IsValid());

            Utf8String expectedColumnName;
            expectedColumnName.Sprintf("c%s_Code_Name", fooClassId.ToString().c_str());
            ASSERT_TRUE(ecdb.ColumnExists("ts_Foo", expectedColumnName.c_str()));
            }

            {
            //now flip order of struct prop and prim prop
            SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                                "  <ECStructClass typeName='ElementCode' modifier='None'>"
                                "    <ECProperty propertyName='Name' typeName='string' />"
                                "  </ECStructClass>"
                                "  <ECEntityClass typeName='Foo' modifier='None'>"
                                "    <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                                "    <ECProperty propertyName='Code_Name' typeName='string' />"
                                "  </ECEntityClass>"
                                "</ECSchema>", true, "");

            ECDb ecdb;
            bool asserted = false;
            AssertSchemaImport(ecdb, asserted, testItem, "propertywithsamenameasstructmembercol.ecdb");
            ASSERT_FALSE(asserted);

            ASSERT_TRUE(ecdb.ColumnExists("ts_Foo", "Code_Name"));
            ECClassId fooClassId = ecdb.Schemas().GetECClassId("TestSchema", "Foo");
            ASSERT_TRUE(fooClassId.IsValid());

            Utf8String expectedColumnName;
            expectedColumnName.Sprintf("c%s_Code_Name", fooClassId.ToString().c_str());
            ASSERT_TRUE(ecdb.ColumnExists("ts_Foo", expectedColumnName.c_str()));
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, InstanceInsertionForClassMappedToExistingTable)
    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ECDbTestUtility::CreateECDb(ecdb, nullptr, L"VerifyInstanceInsertionForClassMappedToExistingTable.ecdb"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ASSERT_FALSE(ecdb.TableExists("TestTable"));
    ecdb.CreateTable("TestTable", "ECInstanceId INTEGER PRIMARY KEY, Name TEXT, Date INTEGER");
    ASSERT_TRUE(ecdb.TableExists("TestTable"));
    ecdb.SaveChanges();

    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "  <ECProperty propertyName='Name' typeName='string'/>"
        "  <ECProperty propertyName='Date' typeName='int'/>"
        "</ECEntityClass>"
        "</ECSchema>", true);

    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, testItem);
    ASSERT_FALSE(asserted);

    //Verifying that the class is not mapped to any table other than the Existing Table.
    ASSERT_FALSE(ecdb.TableExists("t_Class"));

    ECClassCP testClass = ecdb.Schemas().GetECClass("TestSchema", "TestClass");
    ASSERT_TRUE(testClass != nullptr);

    ECInstanceInserter inserter(ecdb, *testClass);
    ASSERT_FALSE(inserter.IsValid());

    ECInstanceUpdater updater(ecdb, *testClass);
    ASSERT_FALSE(updater.IsValid());

    ECInstanceDeleter deleter(ecdb, *testClass);
    ASSERT_FALSE(deleter.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, MapRelationshipsToExistingTable)
    {
    auto assertECSql = [] (Utf8CP ecsql, ECDbR ecdb, ECSqlStatus sqlStatus = ECSqlStatus::InvalidECSql, DbResult dbResult = DbResult::BE_SQLITE_ERROR)
        {
        ECSqlStatement statement;
        ASSERT_EQ(sqlStatus, statement.Prepare(ecdb, ecsql));
        ASSERT_EQ(dbResult, statement.Step());
        };

    /*-----------------------------------------------
    LinkTable relationship against existing link table
    ------------------------------------------------*/
    {
    SetupECDb("linktablerelationshipmappedtoexistinglinktable.ecdb");

    GetECDb().CreateTable("TestTable", "ECInstanceId INTEGER PRIMARY KEY, relProp INTEGER, SourceId INTEGER, TargetId INTEGER");
    ASSERT_TRUE(GetECDb().TableExists("TestTable"));
    GetECDb().SaveChanges();

    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='Foo' modifier='None' >"
        "   <ECProperty propertyName='FooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='Goo' modifier='None' >"
        "   <ECProperty propertyName='GooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='FooHasGoo' modifier='Sealed' strength='referencing'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "            <LinkTableRelationshipMap xmlns = 'ECDbMap.02.00'>"
        "               <SourceECInstanceIdColumn>SourceId</SourceECInstanceIdColumn>"
        "               <TargetECInstanceIdColumn>TargetId</TargetECInstanceIdColumn>"
        "            </LinkTableRelationshipMap>"
        "        </ECCustomAttributes>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Target>"
        "   <ECProperty propertyName='relProp' typeName='int' />"
        "</ECRelationshipClass>"
        "</ECSchema>", true, "relationship link table mapping to existing table is expected to succeed");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), testItem);
    ASSERT_FALSE(asserted);

    //Verify ECSql
    assertECSql("INSERT INTO t.Foo(ECInstanceId, FooProp) VALUES(1, 1)", GetECDb(), ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assertECSql("INSERT INTO t.Goo(ECInstanceId, GooProp) VALUES(1, 1)", GetECDb(), ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);

    assertECSql("SELECT count(*) FROM t.FooHasGoo", GetECDb(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);
    assertECSql("INSERT INTO t.FooHasGoo (SourceECInstanceId, TargetECInstanceId) VALUES(1, 1)", GetECDb());
    assertECSql("UPDATE t.FooHasGoo set relProp=10 WHERE ECInstanceId=1", GetECDb());
    assertECSql("DELETE FROM t.FooHasGoo", GetECDb());
    }

    /*----------------------------------------------------------------------------------
    Existing Table already containing columns named SourceInstanceId and TargetInstanceId
    -----------------------------------------------------------------------------------*/
    {
    SetupECDb("linktablerelationshipmappedtoexistinglinktable.ecdb");

    GetECDb().CreateTable("TestTable", "ECInstanceId INTEGER PRIMARY KEY, relProp INTEGER, SourceECInstanceId INTEGER, TargetECInstanceId INTEGER");
    ASSERT_TRUE(GetECDb().TableExists("TestTable"));
    GetECDb().SaveChanges();

    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='Foo' modifier='None' >"
        "   <ECProperty propertyName='FooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='Goo' modifier='None' >"
        "   <ECProperty propertyName='GooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='FooHasGoo' modifier='Sealed' strength='referencing'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Target>"
        "   <ECProperty propertyName='relProp' typeName='int' />"
        "</ECRelationshipClass>"
        "</ECSchema>", true, "linktable relationship map to existing table is expected to succeed if existing column already contains required named columns i.e SourceECInstanceId and TargetECInstanceId");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), testItem);
    ASSERT_FALSE(asserted);

    //Verify ECSql
    assertECSql("INSERT INTO t.Foo(ECInstanceId, FooProp) VALUES(1, 1)", GetECDb(), ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);
    assertECSql("INSERT INTO t.Goo(ECInstanceId, GooProp) VALUES(1, 1)", GetECDb(), ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);

    assertECSql("SELECT count(*) FROM t.FooHasGoo", GetECDb(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);
    assertECSql("INSERT INTO t.FooHasGoo (SourceECInstanceId, TargetECInstanceId) VALUES(1, 1)", GetECDb());
    assertECSql("UPDATE t.FooHasGoo set relProp=10 WHERE ECInstanceId=1", GetECDb());
    assertECSql("DELETE FROM t.FooHasGoo", GetECDb());
    }

    /*----------------------------------------------------------------------------------------------------------------------------
    CA linkTableRelationshipMap not applied nor the ExistingTable Contains Columns named SourceECInstanceId and TargetECInstanceId
    -----------------------------------------------------------------------------------------------------------------------------*/
    {
    SetupECDb("linktablerelationshipmappedtoexistinglinktable.ecdb");

    GetECDb().CreateTable("TestTable", "ECInstanceId INTEGER PRIMARY KEY, relProp INTEGER, SourceId INTEGER, TargetId INTEGER");
    ASSERT_TRUE(GetECDb().TableExists("TestTable"));
    GetECDb().SaveChanges();

    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='Foo' modifier='None' >"
        "   <ECProperty propertyName='FooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='Goo' modifier='None' >"
        "   <ECProperty propertyName='GooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='FooHasGoo' modifier='Sealed' strength='referencing'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Target>"
        "   <ECProperty propertyName='relProp' typeName='int' />"
        "</ECRelationshipClass>"
        "</ECSchema>", false, "CA LinkTableRelationshipMap should be applied to specify source and target class Columns in existing table.");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), testItem);
    ASSERT_FALSE(asserted);
    }

    /*---------------------------------------------
    FK relationship class mapping to existing table
    ----------------------------------------------*/
    {
    SetupECDb("fkrelationshipclassmappedtoexistingtable.ecdb");

    GetECDb().CreateTable("TestTable", "ECInstanceId INTEGER PRIMARY KEY, GooProp INTEGER, ForeignKeyId INTEGER");
    ASSERT_TRUE(GetECDb().TableExists("TestTable"));
    GetECDb().SaveChanges();

    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='Foo' modifier='None' >"
        "   <ECProperty propertyName='FooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "   <ECProperty propertyName='GooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='FooHasGoo' modifier='Sealed' strength='referencing'>"
        "        <ECCustomAttributes>"
        "            <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
        "                   <ForeignKeyColumn>ForeignKeyId</ForeignKeyColumn>"
        "            </ForeignKeyRelationshipMap>"
        "        </ECCustomAttributes>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Target>"
        "</ECRelationshipClass>"
        "</ECSchema>", true, "FK relationship mapping to existing table is expected to succeed");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), testItem);
    ASSERT_FALSE(asserted);

    //verify ECSql
    assertECSql("INSERT INTO t.Foo(ECInstanceId, FooProp) VALUES(1, 1)", GetECDb(), ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);

    //ECSql on Classes mapped to existing table
    assertECSql("SELECT COUNT(*) FROM t.Goo", GetECDb(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);
    assertECSql("INSERT INTO t.Goo(ECInstanceId, GooProp) VALUES(2, 2)", GetECDb());
    assertECSql("UPDATE t.Goo SET GooProp=3 WHERE GooProp=2 AND ECInstanceId=2", GetECDb());
    assertECSql("DELETE FROM t.Goo", GetECDb());

    //ECSql on FK relationship mapped to existing table
    assertECSql("SELECT COUNT(*) FROM t.FooHasGoo", GetECDb(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);
    assertECSql("INSERT INTO t.FooHasGoo(SourceECInstanceId, TargetECInstanceId) VALUES(1, 1)", GetECDb());
    assertECSql("DELETE FROM t.FooHasGoo", GetECDb());
    }

    /*----------------------------------------------------------------------------------------------------------------------------------
    Existing table already contains column with appropriate name required by the relationship to map i.e ForeignECInstanceId_t_FooHasGoo
    ----------------------------------------------------------------------------------------------------------------------------------*/
    {
    SetupECDb("fkrelationshipclassmappedtoexistingtable.ecdb");

    GetECDb().CreateTable("TestTable", "ECInstanceId INTEGER PRIMARY KEY, GooProp INTEGER, ForeignECInstanceId_t_FooHasGoo INTEGER");
    ASSERT_TRUE(GetECDb().TableExists("TestTable"));
    GetECDb().SaveChanges();

    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='Foo' modifier='None' >"
        "   <ECProperty propertyName='FooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "   <ECProperty propertyName='GooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='FooHasGoo' modifier='Sealed' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Target>"
        "</ECRelationshipClass>"
        "</ECSchema>", true, "FK relationship mapping to existing table is expected to succeed");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), testItem);
    ASSERT_FALSE(asserted);

    //verify ECSql
    assertECSql("INSERT INTO t.Foo(ECInstanceId, FooProp) VALUES(1, 1)", GetECDb(), ECSqlStatus::Success, DbResult::BE_SQLITE_DONE);

    //ECSql on Classes mapped to existing table
    assertECSql("SELECT COUNT(*) FROM t.Goo", GetECDb(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);
    assertECSql("INSERT INTO t.Goo(ECInstanceId, GooProp) VALUES(2, 2)", GetECDb());
    assertECSql("UPDATE t.Goo SET GooProp=3 WHERE GooProp=2 AND ECInstanceId=2", GetECDb());
    assertECSql("DELETE FROM t.Goo", GetECDb());

    //ECSql on FK relationship mapped to existing table
    assertECSql("SELECT COUNT(*) FROM t.FooHasGoo", GetECDb(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);
    assertECSql("INSERT INTO t.FooHasGoo(SourceECInstanceId, TargetECInstanceId) VALUES(1, 1)", GetECDb());
    assertECSql("DELETE FROM t.FooHasGoo", GetECDb());
    }

    /*--------------------------------------------------------------------------------------------------------------------
    CA ForiegnKeyRelationshipMap not applied to relationshp class nor the Existing table contains column with required name
    ---------------------------------------------------------------------------------------------------------------------*/
    {
    SetupECDb("fkrelationshipclassmappedtoexistingtable.ecdb");
    GetECDb().CreateTable("TestTable", "ECInstanceId INTEGER PRIMARY KEY, GooProp INTEGER, ForeignKeyId INTEGER");
    ASSERT_TRUE(GetECDb().TableExists("TestTable"));
    GetECDb().SaveChanges();

    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='Foo' modifier='None' >"
        "   <ECProperty propertyName='FooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "   <ECProperty propertyName='GooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='FooHasGoo' modifier='Sealed' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Goo' />"
        "    </Target>"
        "</ECRelationshipClass>"
        "</ECSchema>", false, "ForiegnKeyRelationshipMap CA not found nor the Existing table contains column with required name");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), testItem);
    ASSERT_FALSE(asserted);
    }

    //Mapping Class containing Navigation property to existing table
    {
    SetupECDb("existingtablenavproperty.ecdb");

    GetECDb().CreateTable("TestTable", "ECInstanceId INTEGER PRIMARY KEY, GooProp INTEGER, navProp INTEGER");
    ASSERT_TRUE(GetECDb().TableExists("TestTable"));
    GetECDb().SaveChanges();

    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='Foo' modifier='None' >"
        "   <ECProperty propertyName='FooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "   <ECProperty propertyName='GooProp' typeName='int' />"
        "   <ECNavigationProperty propertyName = 'navProp' relationshipName = 'FooHasManyGoo' direction = 'backward' />"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='FooHasManyGoo' modifier='Sealed' strength='referencing'>"
        "    <Source cardinality='(0,1)' polymorphic='false'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='false'>"
        "      <Class class = 'Goo' />"
        "    </Target>"
        "</ECRelationshipClass>"
        "</ECSchema>", true, "reusing existing column for navigation property is expected to succeed");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), testItem);
    ASSERT_FALSE(asserted);

    assertECSql("SELECT COUNT(*) FROM t.Goo", GetECDb(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);
    assertECSql("INSERT INTO t.Goo (GooProp, navProp) VALUES(1, 1)", GetECDb());
    assertECSql("UPDATE ONLY t.Goo SET navProp=2", GetECDb());
    assertECSql("DELETE FROM t.Goo", GetECDb());
    }

    //Cardinality implying NotNull or FK Column should not be allowed to map to existing table.
    {
    SetupECDb("existingtablenavproperty.ecdb");

    GetECDb().CreateTable("TestTable", "ECInstanceId INTEGER PRIMARY KEY, GooProp INTEGER, navProp INTEGER");
    ASSERT_TRUE(GetECDb().TableExists("TestTable"));
    GetECDb().SaveChanges();

    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='Foo' modifier='None' >"
        "   <ECProperty propertyName='FooProp' typeName='int' />"
        "</ECEntityClass>"
        "<ECEntityClass typeName='Goo' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "   <ECProperty propertyName='GooProp' typeName='int' />"
        "   <ECNavigationProperty propertyName = 'navProp' relationshipName = 'FooHasManyGoo' direction = 'backward' />"
        "</ECEntityClass>"
        "<ECRelationshipClass typeName='FooHasManyGoo' modifier='Sealed' strength='embedding'>"
        "    <Source cardinality='(1,1)' polymorphic='false'>"
        "      <Class class = 'Foo' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='true'>"
        "      <Class class = 'Goo' />"
        "    </Target>"
        "</ECRelationshipClass>"
        "</ECSchema>", false, "Cardinality implying NotNull or FK Column is expected to be not supported");

    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), testItem);
    ASSERT_FALSE(asserted);
    }
    }

//---------------------------------------------------------------------------------------
//*Test to verify the CRUD operations for a schema having similar Class and Property name
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, ClassAndPropertyWithSameName)
    {
    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Product' modifier='None'>"
        "        <ECProperty propertyName='Product' typeName='string' />"
        "        <ECProperty propertyName='Price' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>", true, "");

    ECDb db;
    bool asserted = false;
    AssertSchemaImport(db, asserted, testItem, "InstanceCRUD.ecdb");
    ASSERT_FALSE(asserted);

    //Inserts Instances.
    ECSqlStatement stmt, s1, s2, s3, s4;
    ASSERT_EQ(ECSqlStatus::Success, s1.Prepare(db, "INSERT INTO ts.Product (Product,Price) VALUES('Book',100)"));
    ASSERT_EQ(BE_SQLITE_DONE, s1.Step());
    ASSERT_EQ(ECSqlStatus::Success, s2.Prepare(db, "INSERT INTO ts.Product (Product,Price) VALUES('E-Reader',200)"));
    ASSERT_EQ(BE_SQLITE_DONE, s2.Step());
    ASSERT_EQ(ECSqlStatus::Success, s3.Prepare(db, "INSERT INTO ts.Product (Product,Price) VALUES('I-Pod',700)"));
    ASSERT_EQ(BE_SQLITE_DONE, s3.Step());
    ASSERT_EQ(ECSqlStatus::Success, s4.Prepare(db, "INSERT INTO ts.Product (Product,Price) VALUES('Goggles',500)"));
    ASSERT_EQ(BE_SQLITE_DONE, s4.Step());

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT COUNT(*) FROM ts.Product"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(4, stmt.GetValueInt(0));
    stmt.Finalize();

    //Deletes the instance
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "DELETE FROM ts.Product WHERE Price=200"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT COUNT(*) FROM ts.Product"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    ASSERT_EQ(3, stmt.GetValueInt(0));
    stmt.Finalize();

    //Updates the instance
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "UPDATE ts.Product SET Product='Watch' WHERE Price=500"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    //Select the instance matching the query.
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT Product.Product FROM ts.Product WHERE Price=700"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    ASSERT_STREQ("I-Pod", stmt.GetValueText(0));
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, MismatchDataTypesInExistingTable)
    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ECDbTestUtility::CreateECDb(ecdb, nullptr, L"DataTypeMismatchInExistingTableTest.ecdb"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ASSERT_FALSE(ecdb.TableExists("TestTable"));
    ecdb.CreateTable("TestTable", "ECInstanceId INTEGER PRIMARY KEY, Name TEXT, Date INTEGER");
    ASSERT_TRUE(ecdb.TableExists("TestTable"));
    ecdb.SaveChanges();

    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='Class' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "  <ECProperty propertyName='Name' typeName='string'/>"
        "  <ECProperty propertyName='Date' typeName='double'/>"
        "</ECEntityClass>"
        "</ECSchema>", false);

    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, testItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, ExistingTableWithOutECInstanceIdColumn)
    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ECDbTestUtility::CreateECDb(ecdb, nullptr, L"InvalidPrimaryKeyInExistingTable.ecdb"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ASSERT_FALSE(ecdb.TableExists("TestTable"));

    ecdb.CreateTable("TestTable", "Id INTEGER PRIMARY KEY, Name TEXT, Date INTEGER");
    ASSERT_TRUE(ecdb.TableExists("TestTable"));

    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='TestClass' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>TestTable</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "  <ECProperty propertyName='Name' typeName='string'/>"
        "  <ECProperty propertyName='Date' typeName='int'/>"
        "</ECEntityClass>"
        "</ECSchema>", false);

    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, testItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle   12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, PropertiesWithoutColumnsInExistingTable)
    {
    ECDbR ecdb = SetupECDb("existingtablemapstrategy.ecdb");
    ASSERT_TRUE(ecdb.IsDbOpen());

    ASSERT_EQ(BE_SQLITE_OK, ecdb.CreateTable("Foo", "ECInstanceId INTEGER PRIMARY KEY, P1 TEXT, P2 INTEGER"));
    ecdb.SaveChanges();

    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "<ECEntityClass typeName='Foo'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>ExistingTable</MapStrategy>"
        "                <TableName>Foo</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "  <ECProperty propertyName='P1' typeName='string'/>"
        "  <ECProperty propertyName='P2' typeName='int'/>"
        "  <ECProperty propertyName='P3' typeName='int'/>"
        "  <ECProperty propertyName='P4' typeName='string'/>"
        "</ECEntityClass>"
        "</ECSchema>", false); //schema import should ideally fail for properties without columns in existing table. TFS#268976

    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, testItem);
    ASSERT_FALSE(asserted);

    ASSERT_FALSE(ecdb.ColumnExists("Foo", "P3"));
    ASSERT_FALSE(ecdb.ColumnExists("Foo", "P4"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, SharedTableInstanceInsertionAndDeletion)
    {
    SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' displayLabel='Table Per Hierarchy' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "   <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "   <ECEntityClass typeName='ClassA' Modifier='None'>"
                        "       <ECCustomAttributes>"
                        "           <ClassMap xmlns='ECDbMap.02.00'>"
                        "               <MapStrategy>SharedTable</MapStrategy>"
                        "               <TableName>TestTable</TableName>"
                        "           </ClassMap>"
                        "       </ECCustomAttributes>"
                        "       <ECProperty propertyName='P1' typeName='string' />"
                        "   </ECEntityClass>"
                        "   <ECEntityClass typeName='ClassB' modifier='None'>"
                        "<ECCustomAttributes>"
                        "           <ClassMap xmlns='ECDbMap.02.00'>"
                        "               <MapStrategy>SharedTable</MapStrategy>"
                        "               <TableName>TestTable</TableName>"
                        "           </ClassMap>"
                        "       </ECCustomAttributes>"
                        "       <ECProperty propertyName='P2' typeName='string' />"
                        "   </ECEntityClass>"
                        "   <ECEntityClass typeName='ClassC' modifier='None'>"
                        "       <ECCustomAttributes>"
                        "           <ClassMap xmlns='ECDbMap.02.00'>"
                        "               <MapStrategy>SharedTable</MapStrategy>"
                        "           <TableName>TestTable</TableName>"
                        "           </ClassMap>"
                        "       </ECCustomAttributes>"
                        "       <ECProperty propertyName='P3' typeName='string' />"
                        "   </ECEntityClass>"
                        "</ECSchema>", true);

    ECDb ecdb;
    bool asserted = false;
    ECSqlStatement statment;

    AssertSchemaImport(ecdb, asserted, testItem, "SharedTableTest.ecdb");
    ASSERT_FALSE(asserted);

    ECSchemaCP testSchema = ecdb.Schemas().GetECSchema("TestSchema");
    EXPECT_TRUE(testSchema != nullptr);

    //Inserts values in Class A,B and C.
    EXPECT_EQ(ECSqlStatus::Success, statment.Prepare(ecdb, "INSERT INTO t.ClassA(P1) VALUES ('Testval1')"));
    EXPECT_TRUE(BE_SQLITE_DONE == statment.Step());
    statment.Finalize();

    EXPECT_EQ(ECSqlStatus::Success, statment.Prepare(ecdb, "INSERT INTO t.ClassB(P2) VALUES ('Testval2')"));
    EXPECT_TRUE(BE_SQLITE_DONE == statment.Step());
    statment.Finalize();

    EXPECT_EQ(ECSqlStatus::Success, statment.Prepare(ecdb, "INSERT INTO t.ClassC(P3) VALUES ('Testval3')"));
    EXPECT_TRUE(BE_SQLITE_DONE == statment.Step());
    statment.Finalize();

    //Deletes the instance of ClassA.
    EXPECT_EQ(ECSqlStatus::Success, statment.Prepare(ecdb, "DELETE FROM t.ClassA"));
    EXPECT_TRUE(BE_SQLITE_DONE == statment.Step());
    statment.Finalize();

    BeSQLite::Statement stmt;
    EXPECT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM TestTable"));
    ASSERT_TRUE(DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(2, stmt.GetValueInt(0));

    //Updates the instance of ClassB.
    EXPECT_EQ(ECSqlStatus::Success, statment.Prepare(ecdb, "UPDATE t.ClassB SET P2='UpdatedValue'"));
    EXPECT_TRUE(BE_SQLITE_DONE == statment.Step());
    statment.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, SharedTableAppliesToSubclasses_SharedColumns)
    {
    SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='SchemaWithSharedColumns' nameSpacePrefix='rc' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
                        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='BaseClass' modifier='None'>"
                        "        <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                        "            </ClassMap>"
                        "            <ShareColumns xmlns='ECDbMap.02.00'>"
                        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                        "            </ShareColumns>"
                        "        </ECCustomAttributes>"
                        "        <ECProperty propertyName='P1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='ChildDomainClassA' modifier='None'>"
                        "        <BaseClass>BaseClass</BaseClass>"
                        "        <ECProperty propertyName='P2' typeName='double' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='ChildDomainClassB' modifier='None'>"
                        "        <BaseClass>BaseClass</BaseClass>"
                        "        <ECProperty propertyName='P3' typeName='int' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='DerivedA' modifier='None'>"
                        "        <BaseClass>ChildDomainClassA</BaseClass>"
                        "        <ECProperty propertyName='P4' typeName='double' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='DerivedB' modifier='None'>"
                        "        <BaseClass>ChildDomainClassA</BaseClass>"
                        "        <ECProperty propertyName='P5' typeName='string' />"
                        "    </ECEntityClass>"
                        "</ECSchema>", true, "");

    ECDb db;
    bool asserted = false;
    AssertSchemaImport(db, asserted, testItem, "sharedcolumns.ecdb");
    ASSERT_FALSE(asserted);

    //verify tables
    ASSERT_TRUE(db.TableExists("rc_BaseClass"));
    ASSERT_FALSE(db.TableExists("rc_ChildDomainClassA"));
    ASSERT_FALSE(db.TableExists("rc_ChildDomainClassB"));
    ASSERT_FALSE(db.TableExists("rc_DerivedA"));
    ASSERT_FALSE(db.TableExists("rc_DerivedB"));

    //verify ECSqlStatments
    ECSqlStatement s1, s2, s3, s4, s5;
    ASSERT_EQ(s1.Prepare(db, "INSERT INTO rc.BaseClass (P1) VALUES('HelloWorld')"), ECSqlStatus::Success);
    ASSERT_EQ(s1.Step(), BE_SQLITE_DONE);
    ASSERT_EQ(s2.Prepare(db, "INSERT INTO rc.ChildDomainClassA (P1, P2) VALUES('ChildClassA', 10.002)"), ECSqlStatus::Success);
    ASSERT_EQ(s2.Step(), BE_SQLITE_DONE);
    ASSERT_EQ(s3.Prepare(db, "INSERT INTO rc.ChildDomainClassB (P1, P3) VALUES('ChildClassB', 2)"), ECSqlStatus::Success);
    ASSERT_EQ(s3.Step(), BE_SQLITE_DONE);
    ASSERT_EQ(s4.Prepare(db, "INSERT INTO rc.DerivedA (P1, P2, P4) VALUES('DerivedA', 11.003, 12.004)"), ECSqlStatus::Success);
    ASSERT_EQ(s4.Step(), BE_SQLITE_DONE);
    ASSERT_EQ(s5.Prepare(db, "INSERT INTO rc.DerivedB (P1, P2, P5) VALUES('DerivedB', 11.003, 'DerivedB')"), ECSqlStatus::Success);
    ASSERT_EQ(s5.Step(), BE_SQLITE_DONE);

    //verify No of Columns in BaseClass
    Statement statement;
    ASSERT_EQ(statement.Prepare(db, "SELECT * FROM rc_BaseClass"), DbResult::BE_SQLITE_OK);
    ASSERT_EQ(statement.Step(), DbResult::BE_SQLITE_ROW);
    ASSERT_EQ(5, statement.GetColumnCount());

    //verify that the columns generated are same as expected
    Utf8String expectedColumnNames = "ECInstanceIdECClassIdP1sc01sc02";
    Utf8String actualColumnNames;
    for (int i = 0; i < 5; i++)
        {
        actualColumnNames.append(statement.GetColumnName(i));
        }
    ASSERT_STREQ(expectedColumnNames.c_str(), actualColumnNames.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, IndexGenerationOnClassId)
    {
    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='ClassA' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ClassB' modifier='None' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>", true);

    ECDb db;
    bool asserted = false;
    AssertSchemaImport(db, asserted, testItem, "IndexGenerationOnClassId.ecdb");
    ASSERT_FALSE(asserted);

    Statement sqlstmt;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, sqlstmt.Prepare(db, "SELECT Name FROM ec_Index WHERE Id=(SELECT IndexId FROM ec_IndexColumn WHERE ColumnId=(SELECT Id FROM ec_Column WHERE Name='ECClassId' AND TableId=(SELECT Id FROM ec_Table WHERE Name='ts_ClassA')))"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, sqlstmt.Step());
    ASSERT_STREQ("ix_ts_ClassA_ecclassid", sqlstmt.GetValueText(0));
    sqlstmt.Finalize();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, sqlstmt.Prepare(db, "SELECT Name FROM ec_Index WHERE Id=(SELECT IndexId FROM ec_IndexColumn WHERE ColumnId=(SELECT Id FROM ec_Column WHERE Name='ECClassId' AND TableId=(SELECT Id FROM ec_Table WHERE Name='ts_ClassB')))"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, sqlstmt.Step());
    ASSERT_STREQ("ix_ts_ClassB_ecclassid", sqlstmt.GetValueText(0));
    sqlstmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, NotMappedWithinClassHierarchy)
    {
    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Base' modifier='None'>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Sub' modifier='None' >"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='SubSub' modifier='Sealed' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>NotMapped</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECEntityClass>"
        "</ECSchema>", true);

    ECDb db;
    bool asserted = false;
    AssertSchemaImport(db, asserted, testItem, "NotMappedWithinClassHierarchy.ecdb");
    ASSERT_FALSE(asserted);

    //verify tables
    ASSERT_TRUE(db.TableExists("ts_Base"));
    ASSERT_TRUE(db.TableExists("ts_Sub"));
    ASSERT_FALSE(db.TableExists("ts_SubSub"));

    //verify ECSQL
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(db, "INSERT INTO ts.SubSub (P1, P2) VALUES(1,2)")) << "INSERT not possible against unmapped class";
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(db, "SELECT * FROM ts.SubSub")) << "SELECT not possible against unmapped class";
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, CascadeDeletion)
    {
    SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='ClassA' modifier='None'>"
                        "        <ECProperty propertyName='AA' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='ClassB' modifier='None'>"
                        "        <ECProperty propertyName='BB' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECRelationshipClass typeName='AHasB' modifier='Sealed' strength='embedding'>"
                        "        <ECCustomAttributes>"
                        "            <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
                        "               <OnDeleteAction>Cascade</OnDeleteAction>"
                        "            </ForeignKeyRelationshipMap>"
                        "        </ECCustomAttributes>"
                        "       <Source cardinality='(0,1)' polymorphic='True'>"
                        "           <Class class='ClassA' />"
                        "       </Source>"
                        "       <Target cardinality='(0,N)' polymorphic='True'>"
                        "           <Class class='ClassB' />"
                        "       </Target>"
                        "     </ECRelationshipClass>"
                        "    <ECEntityClass typeName='ClassC' modifier='None'>"
                        "        <ECProperty propertyName='CC' typeName='string' />"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='BHasC' modifier='Sealed' strength='embedding'>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'ClassB' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'ClassC' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>", true, "");

    ECDb db;
    bool asserted = false;
    AssertSchemaImport(db, asserted, testItem, "CascadeDeletion.ecdb");
    ASSERT_FALSE(asserted);

    ECSchemaCP schema = db.Schemas().GetECSchema("TestSchema");
    EXPECT_TRUE(schema != nullptr);

    ECClassCP ClassA = schema->GetClassCP("ClassA");
    EXPECT_TRUE(ClassA != nullptr);

    ECClassCP ClassB = schema->GetClassCP("ClassB");
    EXPECT_TRUE(ClassB != nullptr);

    ECClassCP ClassC = schema->GetClassCP("ClassC");
    EXPECT_TRUE(ClassC != nullptr);

    //Instance of ClassA
    StandaloneECInstancePtr ClassA_Instance = ClassA->GetDefaultStandaloneEnabler()->CreateInstance();
    ClassA_Instance->SetValue("AA", ECValue("val1"));

    //Instance of ClassB
    StandaloneECInstancePtr ClassB_Instance = ClassB->GetDefaultStandaloneEnabler()->CreateInstance();
    ClassB_Instance->SetValue("BB", ECValue("val3"));

    //Inserter of ClassA
    ECInstanceInserter ClassA_Inserter(db, *ClassA);
    ASSERT_TRUE(ClassA_Inserter.IsValid());
    ClassA_Inserter.Insert(*ClassA_Instance);

    //Inserter of ClassB
    ECInstanceInserter ClassB_Inserter(db, *ClassB);
    ASSERT_TRUE(ClassB_Inserter.IsValid());
    ClassB_Inserter.Insert(*ClassB_Instance);

    ECRelationshipClassCP AHasB = db.Schemas().GetECClass("TestSchema", "AHasB")->GetRelationshipClassCP();
    ECRelationshipClassCP BHasC = db.Schemas().GetECClass("TestSchema", "BHasC")->GetRelationshipClassCP();

    //Inserting relationship instance.
    ECN::StandaloneECRelationshipInstancePtr ClassAHasB_relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*AHasB)->CreateRelationshipInstance();
    ClassAHasB_relationshipInstance->SetSource(ClassA_Instance.get());
    ClassAHasB_relationshipInstance->SetTarget(ClassB_Instance.get());

    ECInstanceInserter AHasB_relationshipInserter(db, *AHasB);
    AHasB_relationshipInserter.Insert(*ClassAHasB_relationshipInstance);


    //Inserting instances of ClassC
    StandaloneECInstancePtr ClassC_Instance = ClassC->GetDefaultStandaloneEnabler()->CreateInstance();
    ClassC_Instance->SetValue("CC", ECValue("val5"));

    //Inserter of ClassC
    ECInstanceInserter ClassC_Inserter(db, *ClassC);
    ASSERT_TRUE(ClassC_Inserter.IsValid());
    ClassC_Inserter.Insert(*ClassC_Instance);

    //Inserting relationship instances.
    ECN::StandaloneECRelationshipInstancePtr ClassBHasC_relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*BHasC)->CreateRelationshipInstance();
    ClassBHasC_relationshipInstance->SetSource(ClassB_Instance.get());
    ClassBHasC_relationshipInstance->SetTarget(ClassC_Instance.get());

    ECInstanceInserter BHasC_relationshipInserter(db, *BHasC);
    BHasC_relationshipInserter.Insert(*ClassBHasC_relationshipInstance);

    //Deletes instance of ClassA. Instances of ClassB and ClassC are also deleted.
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "DELETE FROM ts.ClassA WHERE ECInstanceId=1"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT AA FROM ts.ClassA"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    ASSERT_EQ(NULL, stmt.GetValueText(0));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT BB FROM ts.ClassB"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    ASSERT_EQ(NULL, stmt.GetValueText(0));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT CC FROM ts.ClassC"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    ASSERT_EQ(NULL, stmt.GetValueText(0));
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle         10/15
//+---------------+---------------+---------------+---------------+---------------+------
void AssertECInstanceIdAutoGeneration(ECDbCR ecdb, bool expectedToSucceed, Utf8CP fullyQualifiedTestClass, Utf8CP prop, Utf8CP val)
    {
    if (!expectedToSucceed)
        BeTest::SetFailOnAssert(false);

    //different ways to let ECDb auto-generated (if allowed)
    {
    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO %s (%s) VALUES(%s)", fullyQualifiedTestClass, prop, val);
    ECSqlStatement stmt;

    ECSqlStatus expectedStat = expectedToSucceed ? ECSqlStatus::Success : ECSqlStatus::InvalidECSql;
    ASSERT_EQ(expectedStat, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();

    if (expectedToSucceed)
        {
        ECInstanceKey newKey;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));
        ASSERT_TRUE(newKey.IsValid());
        }
    }

    {
    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO %s (ECInstanceId, %s) VALUES(NULL, %s)", fullyQualifiedTestClass, prop, val);
    ECSqlStatement stmt;

    //only fails at step time
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();

    if (expectedToSucceed)
        {
        ECInstanceKey newKey;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey));
        ASSERT_TRUE(newKey.IsValid());
        }
    }

    ECInstanceId id;
    {
    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO %s (ECInstanceId, %s) VALUES(?, %s)", fullyQualifiedTestClass, prop, val);
    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << "Prepare should always succeed if ECInstanceId is bound via parameters: " << ecsql.c_str();

    DbResult expectedStat = expectedToSucceed ? BE_SQLITE_DONE : BE_SQLITE_ERROR;

    ECInstanceKey newKey;
    ASSERT_EQ(expectedStat, stmt.Step(newKey));
    ASSERT_EQ(expectedToSucceed, newKey.IsValid());

    id = newKey.GetECInstanceId();
    }

    //now test when ECInstanceId is specified
    {
    id = ECInstanceId(id.GetValue() + 1);
    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO %s (ECInstanceId, %s) VALUES(%llu, %s)", fullyQualifiedTestClass, prop, id.GetValue(), val);
    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();
    ECInstanceKey newKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << "Disable flag does not affect case when ECInstanceId is specified";
    ASSERT_EQ(id.GetValue(), newKey.GetECInstanceId().GetValue());
    }

    {
    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO %s (ECInstanceId, %s) VALUES(?, %s)", fullyQualifiedTestClass, prop, val);
    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str())) << ecsql.c_str();

    id = ECInstanceId(id.GetValue() + 1);
    stmt.BindId(1, id);
    ECInstanceKey newKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(newKey)) << "Disable flag does not affect case when ECInstanceId is specified";
    ASSERT_EQ(id.GetValue(), newKey.GetECInstanceId().GetValue());
    }

    BeTest::SetFailOnAssert(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, ECInstanceIdAutoGeneration)
    {
    //CASE 1
            {
            // DisableECInstanceIdAutogeneration CA not present.Should generate Id's automatically.
            SchemaItem testItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='TestClass' modifier='None' >"
                "    <ECProperty propertyName='P0' typeName='string' />"
                "    </ECEntityClass>"
                "</ECSchema>", true);

            ECDb db;
            bool asserted = false;
            AssertSchemaImport(db, asserted, testItem, "ECInstanceIdAutoGeneration.ecdb");
            ASSERT_FALSE(asserted);

            AssertECInstanceIdAutoGeneration(db, true, "ts.TestClass", "P0", "'val'");
            }

            //CASE 2
            {
            // DisableECInstanceIdAutogeneration custom attribute present on a class. 
            SchemaItem testItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='TestClass' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <DisableECInstanceIdAutogeneration xmlns='ECDbMap.02.00'>"
                "            </DisableECInstanceIdAutogeneration>"
                "        </ECCustomAttributes>"
                "    <ECProperty propertyName='P0' typeName='string' />"
                "    </ECEntityClass>"
                "</ECSchema>", true);

            ECDb db;
            bool asserted = false;
            AssertSchemaImport(db, asserted, testItem, "ECInstanceIdAutoGeneration.ecdb");
            ASSERT_FALSE(asserted);

            AssertECInstanceIdAutoGeneration(db, false, "ts.TestClass", "P0", "'val'");
            }

            //CASE 3
            {
            //DisableECInstanceIdAutogenerationCA true for sub classes.
            SchemaItem testItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='Parent' modifier='None' >"
                "        <ECCustomAttributes>"
                "            <DisableECInstanceIdAutogeneration xmlns='ECDbMap.02.00'>"
                "                  <AppliesToSubclasses>True</AppliesToSubclasses>"
                "            </DisableECInstanceIdAutogeneration>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='P0' typeName='string' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='Child' modifier='None' >"
                "    <BaseClass>Parent</BaseClass>"
                "        <ECProperty propertyName='P1' typeName='string' />"
                "    </ECEntityClass>"
                "</ECSchema>", true);

            ECDb db;
            bool asserted = false;
            AssertSchemaImport(db, asserted, testItem, "ECInstanceIdAutoGeneration.ecdb");
            ASSERT_FALSE(asserted);

            AssertECInstanceIdAutoGeneration(db, false, "ts.Parent", "P0", "'val'");
            AssertECInstanceIdAutoGeneration(db, false, "ts.Child", "P1", "'val'");
            }

            //CASE 4
            {
            //DisableECInstanceIdAutogenerationCA False for sub classes.
            SchemaItem testItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='Parent' modifier='None' >"
                "        <ECCustomAttributes>"
                "            <DisableECInstanceIdAutogeneration xmlns='ECDbMap.02.00'>"
                "                  <AppliesToSubclasses>False</AppliesToSubclasses>"
                "            </DisableECInstanceIdAutogeneration>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='P0' typeName='string' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='Child' modifier='None' >"
                "    <BaseClass>Parent</BaseClass>"
                "        <ECProperty propertyName='P1' typeName='string' />"
                "    </ECEntityClass>"
                "</ECSchema>", true);

            ECDb db;
            bool asserted = false;
            AssertSchemaImport(db, asserted, testItem, "ECInstanceIdAutoGeneration.ecdb");
            ASSERT_FALSE(asserted);

            AssertECInstanceIdAutoGeneration(db, false, "ts.Parent", "P0", "'val'");
            AssertECInstanceIdAutoGeneration(db, true, "ts.Child", "P1", "'val'");
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, EnforceLinkTableMapping)
    {
    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='A' modifier='None'>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B' modifier='None'>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='AHasB' modifier='Sealed' strength='referencing'>"
        "        <ECCustomAttributes>"
        "            <LinkTableRelationshipMap xmlns='ECDbMap.02.00' />"
        "        </ECCustomAttributes>"
        "       <Source cardinality='(0, 1)' polymorphic='True'>"
        "        <Class class = 'A' />"
        "       </Source>"
        "       <Target cardinality='(0, 1)' polymorphic='True'>"
        "         <Class class='B' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>", true);

    ECDb db;
    bool asserted = false;
    AssertSchemaImport(db, asserted, testItem, "enforcelinktablemapping.ecdb");
    ASSERT_FALSE(asserted);

    //verify tables
    ASSERT_TRUE(db.TableExists("ts_AHasB"));
    ASSERT_TRUE(db.ColumnExists("ts_AHasB", "SourceECInstanceId"));
    ASSERT_TRUE(db.ColumnExists("ts_AHasB", "TargetECInstanceId"));
    bvector<Utf8String> columns;
    ASSERT_TRUE(db.GetColumns(columns, "ts_AHasB"));
    ASSERT_EQ(3, columns.size());

    columns.clear();
    ASSERT_TRUE(db.GetColumns(columns, "ts_A"));
    ASSERT_EQ(2, columns.size());

    columns.clear();
    ASSERT_TRUE(db.GetColumns(columns, "ts_B"));
    ASSERT_EQ(2, columns.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, UserDefinedIndexTest)
    {
            {
            std::vector<SchemaItem> testItems;
            testItems.push_back(SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='Element' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                 <Indexes>"
                "                   <DbIndex>"
                "                       <IsUnique>True</IsUnique>"
                "                       <Name>uix_element_code</Name>"
                "                       <Properties>"
                "                          <string>Bla</string>"
                "                       </Properties>"
                "                   </DbIndex>"
                "                 </Indexes>"
                "            </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='Code' typeName='int' />"
                "    </ECEntityClass>"
                "</ECSchema>", false, "Property in index does not exist"));

            testItems.push_back(SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECStructClass typeName='ElementCode' modifier='None'>"
                "        <ECProperty propertyName='AuthorityId' typeName='long' />"
                "        <ECProperty propertyName='Namespace' typeName='string' />"
                "        <ECProperty propertyName='Val' typeName='string' />"
                "    </ECStructClass>"
                "    <ECEntityClass typeName='Element' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "                 <Indexes>"
                "                   <DbIndex>"
                "                       <IsUnique>True</IsUnique>"
                "                       <Name>uix_element_code</Name>"
                "                       <Properties>"
                "                          <string>Code</string>"
                "                       </Properties>"
                "                   </DbIndex>"
                "                 </Indexes>"
                "            </ClassMap>"
                "            <ShareColumns xmlns='ECDbMap.02.00'>"
                "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                "            </ShareColumns>"
                "        </ECCustomAttributes>"
                "        <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                "    </ECEntityClass>"
                "</ECSchema>", false, "Cannot define index on struct prop"));

            testItems.push_back(SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECStructClass typeName='ElementCode' modifier='None'>"
                "        <ECProperty propertyName='AuthorityId' typeName='long' />"
                "        <ECProperty propertyName='Namespace' typeName='string' />"
                "        <ECProperty propertyName='Val' typeName='string' />"
                "    </ECStructClass>"
                "    <ECEntityClass typeName='Element' modifier='None' >"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "                 <Indexes>"
                "                   <DbIndex>"
                "                       <IsUnique>True</IsUnique>"
                "                       <Name>uix_element_code</Name>"
                "                       <Properties>"
                "                          <string>Codes</string>"
                "                       </Properties>"
                "                   </DbIndex>"
                "                 </Indexes>"
                "            </ClassMap>"
                "            <ShareColumns xmlns='ECDbMap.02.00'>"
                "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                "            </ShareColumns>"
                "        </ECCustomAttributes>"
                "        <ECStructArrayProperty propertyName='Codes' typeName='ElementCode' minOccurs='0' maxOccurs='unbounded' />"
                "    </ECEntityClass>"
                "</ECSchema>", false, "Cannot define index on struct array prop"));

            testItems.push_back(SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='Element' modifier='None' >"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "                 <Indexes>"
                "                   <DbIndex>"
                "                       <IsUnique>True</IsUnique>"
                "                       <Name>uix_element_code</Name>"
                "                       <Properties>"
                "                          <string>Codes</string>"
                "                       </Properties>"
                "                   </DbIndex>"
                "                 </Indexes>"
                "            </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECArrayProperty propertyName='Codes' typeName='string' minOccurs='0' maxOccurs='unbounded' />"
                "    </ECEntityClass>"
                "</ECSchema>", false, "Cannot define index on primitive array prop"));

            testItems.push_back(SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='A' modifier='None' >"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                 <Indexes>"
                "                   <DbIndex>"
                "                       <IsUnique>True</IsUnique>"
                "                       <Name>mypoorlynamedindex</Name>"
                "                       <Properties>"
                "                          <string>Code</string>"
                "                       </Properties>"
                "                   </DbIndex>"
                "                 </Indexes>"
                "            </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='Code' typeName='string'/>"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B' modifier='None' >"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                 <Indexes>"
                "                   <DbIndex>"
                "                       <IsUnique>False</IsUnique>"
                "                       <Name>mypoorlynamedindex</Name>"
                "                       <Properties>"
                "                          <string>BB</string>"
                "                       </Properties>"
                "                   </DbIndex>"
                "                 </Indexes>"
                "            </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='BB' typeName='string'/>"
                "    </ECEntityClass>"
                "</ECSchema>", false, "Duplicate indexes"));

            testItems.push_back(SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='A' modifier='None' >"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "            </ClassMap>"
                "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='AProp' typeName='string'/>"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B' modifier='None' >"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                 <Indexes>"
                "                   <DbIndex>"
                "                       <IsUnique>False</IsUnique>"
                "                       <Name>MyIndex</Name>"
                "                       <Properties>"
                "                          <string>AProp</string>"
                "                          <string>BProp</string>"
                "                       </Properties>"
                "                   </DbIndex>"
                "                 </Indexes>"
                "            </ClassMap>"
                "        </ECCustomAttributes>"
                "        <BaseClass>A</BaseClass>"
                "        <ECProperty propertyName='BProp' typeName='string'/>"
                "    </ECEntityClass>"
                "</ECSchema>", false, "Index with properties that map to different tables is not supported"));


            AssertSchemaImport(testItems, "userdefinedindextest.ecdb");
            }

                {
                SchemaItem testItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECEntityClass typeName='Base' modifier='Abstract'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "                 <Indexes>"
                    "                   <DbIndex>"
                    "                       <Name>ix_base_code</Name>"
                    "                       <Properties>"
                    "                          <string>Code</string>"
                    "                       </Properties>"
                    "                   </DbIndex>"
                    "                 </Indexes>"
                    "            </ClassMap>"
                    "        </ECCustomAttributes>"
                    "        <ECProperty propertyName='Code' typeName='string' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub1' modifier='None'>"
                    "        <BaseClass>Base</BaseClass>"
                    "        <ECProperty propertyName='Sub1_Prop' typeName='double' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub2' modifier='None'>"
                    "        <BaseClass>Base</BaseClass>"
                    "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
                    "    </ECEntityClass>"
                    "</ECSchema>", true, "");

                ECDb db;
                bool asserted = false;
                AssertSchemaImport(db, asserted, testItem, "userdefinedindextest.ecdb");
                ASSERT_FALSE(asserted);

                AssertIndex(db, "ix_base_code", false, "ts_Base", {"Code"});
                }

                {
                SchemaItem testItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECEntityClass typeName='Base' modifier='Abstract'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "                 <Indexes>"
                    "                   <DbIndex>"
                    "                       <Name>ix_base_code</Name>"
                    "                       <Properties>"
                    "                          <string>Code</string>"
                    "                       </Properties>"
                    "                   </DbIndex>"
                    "                 </Indexes>"
                    "            </ClassMap>"
                    "        </ECCustomAttributes>"
                    "        <ECProperty propertyName='Code' typeName='string' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub1' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                 <Indexes>"
                    "                   <DbIndex>"
                    "                       <Name>ix_sub1_prop</Name>"
                    "                       <Properties>"
                    "                          <string>Sub1_Prop</string>"
                    "                       </Properties>"
                    "                   </DbIndex>"
                    "                 </Indexes>"
                    "            </ClassMap>"
                    "        </ECCustomAttributes>"
                    "        <BaseClass>Base</BaseClass>"
                    "        <ECProperty propertyName='Sub1_Prop' typeName='double' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub2' modifier='None'>"
                    "        <BaseClass>Base</BaseClass>"
                    "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
                    "    </ECEntityClass>"
                    "</ECSchema>", true, "");

                ECDb db;
                bool asserted = false;
                AssertSchemaImport(db, asserted, testItem, "userdefinedindextest.ecdb");
                ASSERT_FALSE(asserted);

                AssertIndex(db, "ix_sub1_prop", false, "ts_Base", {"Sub1_Prop"});
                }

                {
                SchemaItem testItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECEntityClass typeName='Base' modifier='Abstract'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "            </ClassMap>"
                    "        </ECCustomAttributes>"
                    "        <ECProperty propertyName='Code' typeName='string' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub1' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                 <Indexes>"
                    "                   <DbIndex>"
                    "                       <Name>uix_sub1_code</Name>"
                    "                       <IsUnique>true</IsUnique>"
                    "                       <Properties>"
                    "                          <string>Code</string>"
                    "                       </Properties>"
                    "                   </DbIndex>"
                    "                 </Indexes>"
                    "            </ClassMap>"
                    "        </ECCustomAttributes>"
                    "        <BaseClass>Base</BaseClass>"
                    "        <ECProperty propertyName='Sub1_Prop' typeName='double' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub2' modifier='None'>"
                    "        <BaseClass>Sub1</BaseClass>"
                    "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
                    "    </ECEntityClass>"
                    "</ECSchema>", true, "");

                ECDb db;
                bool asserted = false;
                AssertSchemaImport(db, asserted, testItem, "userdefinedindextest.ecdb");
                ASSERT_FALSE(asserted);

                ECClassId baseClassId = db.Schemas().GetECClassId("TestSchema", "Base");
                Utf8String indexWhereClause;
                indexWhereClause.Sprintf("ECClassId<>%llu", baseClassId.GetValue());
                AssertIndex(db, "uix_sub1_code", true, "ts_Base", {"Code"}, indexWhereClause.c_str());
                }

                {
                SchemaItem testItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECEntityClass typeName='Base' modifier='Abstract'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "                 <Indexes>"
                    "                   <DbIndex>"
                    "                       <Name>uix_base_code</Name>"
                    "                       <IsUnique>True</IsUnique>"
                    "                       <Properties>"
                    "                          <string>Code</string>"
                    "                       </Properties>"
                    "                   </DbIndex>"
                    "                 </Indexes>"
                    "            </ClassMap>"
                    "        </ECCustomAttributes>"
                    "        <ECProperty propertyName='Code' typeName='string' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub1' modifier='None'>"
                    "        <BaseClass>Base</BaseClass>"
                    "        <ECProperty propertyName='Sub1_Prop' typeName='double' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub2' modifier='None'>"
                    "        <BaseClass>Sub1</BaseClass>"
                    "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub3' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                 <Indexes>"
                    "                   <DbIndex>"
                    "                       <Name>uix_sub3_prop</Name>"
                    "                       <IsUnique>True</IsUnique>"
                    "                       <Properties>"
                    "                          <string>Sub3_Prop</string>"
                    "                       </Properties>"
                    "                   </DbIndex>"
                    "                 </Indexes>"
                    "            </ClassMap>"
                    "        </ECCustomAttributes>"
                    "        <BaseClass>Base</BaseClass>"
                    "        <ECProperty propertyName='Sub3_Prop' typeName='double' />"
                    "    </ECEntityClass>"
                    "</ECSchema>", true, "");

                SchemaItem secondSchemaTestItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts' />"
                    "    <ECEntityClass typeName='Sub4' modifier='None'>"
                    "        <BaseClass>ts:Sub3</BaseClass>"
                    "        <ECProperty propertyName='Sub4_Prop' typeName='double' />"
                    "    </ECEntityClass>"
                    "</ECSchema>", true, "");

                ECClassId sub3ClassId;
                Utf8String ecdbFilePath;

                {
                ECDb ecdb;
                bool asserted = false;
                AssertSchemaImport(ecdb, asserted, testItem, "userdefinedindextest.ecdb");
                ASSERT_FALSE(asserted);
                ecdb.SaveChanges();
                ecdbFilePath = ecdb.GetDbFileName();
                sub3ClassId = ecdb.Schemas().GetECClassId("TestSchema", "Sub3");

                AssertIndex(ecdb, "uix_base_code", true, "ts_Base", {"Code"});

                Utf8String indexWhereClause;
                indexWhereClause.Sprintf("ECClassId=%llu", sub3ClassId.GetValue());
                AssertIndex(ecdb, "uix_sub3_prop", true, "ts_Base", {"Sub3_Prop"}, indexWhereClause.c_str());
                }

                //after second import new subclass in hierarchy must be reflected by indices
                ECDb ecdb;
                ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbFilePath.c_str(), ECDb::OpenParams(Db::OpenMode::ReadWrite)));

                bool asserted = false;
                AssertSchemaImport(asserted, ecdb, secondSchemaTestItem);
                ASSERT_FALSE(asserted);

                //This index is not affected as index is still applying to entire hierarchy
                AssertIndex(ecdb, "uix_base_code", true, "ts_Base", {"Code"});

                //This index must include the new subclass Sub4
                ECClassId sub4ClassId = ecdb.Schemas().GetECClassId("TestSchema2", "Sub4");
                Utf8String indexWhereClause;
                indexWhereClause.Sprintf("ECClassId=%llu OR ECClassId=%llu", sub3ClassId.GetValue(), sub4ClassId.GetValue());
                AssertIndex(ecdb, "uix_sub3_prop", true, "ts_Base", {"Sub3_Prop"}, indexWhereClause.c_str());
                }

                {
                SchemaItem testItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECEntityClass typeName='Base' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "            </ClassMap>"
                    "            <ShareColumns xmlns='ECDbMap.02.00'>"
                    "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                    "            </ShareColumns>"
                    "        </ECCustomAttributes>"
                    "        <ECProperty propertyName='Code' typeName='string' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub1' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                 <Indexes>"
                    "                   <DbIndex>"
                    "                       <Name>ix_sub1_aid</Name>"
                    "                       <Properties>"
                    "                          <string>AId</string>"
                    "                       </Properties>"
                    "                   </DbIndex>"
                    "                 </Indexes>"
                    "            </ClassMap>"
                    "        </ECCustomAttributes>"
                    "        <BaseClass>Base</BaseClass>"
                    "        <ECProperty propertyName='AId' typeName='long' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub2' modifier='None'>"
                    "        <BaseClass>Base</BaseClass>"
                    "        <ECProperty propertyName='Name' typeName='string' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub1_1' modifier='None'>"
                    "        <BaseClass>Sub1</BaseClass>"
                    "        <ECProperty propertyName='Cost' typeName='double' />"
                    "    </ECEntityClass>"
                    "</ECSchema>", true, "");

                ECDb db;
                bool asserted = false;
                AssertSchemaImport(db, asserted, testItem, "userdefinedindextest.ecdb");
                ASSERT_FALSE(asserted);

                AssertIndex(db, "ix_sub1_aid", false, "ts_Base", {"sc01"});
                }

                {
                SchemaItem testItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECEntityClass typeName='Base' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "            </ClassMap>"
                    "            <ShareColumns xmlns='ECDbMap.02.00'>"
                    "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                    "            </ShareColumns>"
                    "        </ECCustomAttributes>"
                    "        <ECProperty propertyName='Code' typeName='string' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub1' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                 <Indexes>"
                    "                   <DbIndex>"
                    "                       <IsUnique>True</IsUnique>"
                    "                       <Name>uix_sub1_aid</Name>"
                    "                       <Properties>"
                    "                          <string>AId</string>"
                    "                       </Properties>"
                    "                   </DbIndex>"
                    "                 </Indexes>"
                    "            </ClassMap>"
                    "        </ECCustomAttributes>"
                    "        <BaseClass>Base</BaseClass>"
                    "        <ECProperty propertyName='AId' typeName='long' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub2' modifier='None'>"
                    "        <BaseClass>Base</BaseClass>"
                    "        <ECProperty propertyName='Name' typeName='string' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub3' modifier='None'>"
                    "        <BaseClass>Base</BaseClass>"
                    "        <ECProperty propertyName='Name2' typeName='string' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='Sub11' modifier='None'>"
                    "        <BaseClass>Sub1</BaseClass>"
                    "        <ECProperty propertyName='Cost' typeName='double' />"
                    "    </ECEntityClass>"
                    "</ECSchema>", true, "Unique indices on shared columns are supported.");

                ECDb db;
                bool asserted = false;
                AssertSchemaImport(db, asserted, testItem, "userdefinedindextest.ecdb");
                ASSERT_FALSE(asserted);

                ECClassId sub1ClassId = db.Schemas().GetECClassId("TestSchema", "Sub1");
                ECClassId sub11ClassId = db.Schemas().GetECClassId("TestSchema", "Sub11");
                Utf8String indexWhereClause;
                indexWhereClause.Sprintf("ECClassId=%llu OR ECClassId=%llu", sub1ClassId.GetValue(), sub11ClassId.GetValue());
                AssertIndex(db, "uix_sub1_aid", true, "ts_Base", {"sc01"}, indexWhereClause.c_str());
                }

                {
                SchemaItem testItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECStructClass typeName='ElementCode' modifier='None'>"
                    "        <ECProperty propertyName='AuthorityId' typeName='long' />"
                    "        <ECProperty propertyName='Namespace' typeName='string' />"
                    "        <ECProperty propertyName='Val' typeName='string' />"
                    "    </ECStructClass>"
                    "    <ECEntityClass typeName='Element' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "                 <Indexes>"
                    "                   <DbIndex>"
                    "                       <IsUnique>True</IsUnique>"
                    "                       <Name>uix_element_code</Name>"
                    "                       <Properties>"
                    "                          <string>Code.AuthorityId</string>"
                    "                          <string>Code.Namespace</string>"
                    "                          <string>Code.Val</string>"
                    "                       </Properties>"
                    "                   </DbIndex>"
                    "                 </Indexes>"
                    "            </ClassMap>"
                    "        </ECCustomAttributes>"
                    "        <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                    "    </ECEntityClass>"
                    "</ECSchema>", true, "");

                ECDb db;
                bool asserted = false;
                AssertSchemaImport(db, asserted, testItem, "userdefinedindextest.ecdb");
                ASSERT_FALSE(asserted);

                AssertIndex(db, "uix_element_code", true, "ts_Element", {"Code_AuthorityId", "Code_Namespace", "Code_Val"});
                }

                {
                SchemaItem testItem("Index on abstract classes - Schema 1",
                                    "<?xml version='1.0' encoding='utf-8'?>"
                                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                    "    <ECEntityClass typeName='Root' modifier='Abstract'>"
                                    "        <ECCustomAttributes>"
                                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                    "                 <Indexes>"
                                    "                   <DbIndex>"
                                    "                       <IsUnique>True</IsUnique>"
                                    "                       <Name>uix_root</Name>"
                                    "                       <Properties>"
                                    "                          <string>RootProp</string>"
                                    "                       </Properties>"
                                    "                   </DbIndex>"
                                    "                 </Indexes>"
                                    "            </ClassMap>"
                                    "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                    "        </ECCustomAttributes>"
                                    "        <ECProperty propertyName='RootProp' typeName='int' />"
                                    "    </ECEntityClass>"
                                    "    <ECEntityClass typeName='Interface' modifier='Abstract'>"
                                    "        <ECCustomAttributes>"
                                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                                    "                 <Indexes>"
                                    "                   <DbIndex>"
                                    "                       <IsUnique>True</IsUnique>"
                                    "                       <Name>uix_interface</Name>"
                                    "                       <Properties>"
                                    "                          <string>InterfaceProp</string>"
                                    "                       </Properties>"
                                    "                   </DbIndex>"
                                    "                 </Indexes>"
                                    "            </ClassMap>"
                                    "        </ECCustomAttributes>"
                                    "        <ECProperty propertyName='InterfaceProp' typeName='int' />"
                                    "    </ECEntityClass>"
                                    "    <ECEntityClass typeName='Sub'>"
                                    "        <ECCustomAttributes>"
                                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                                    "                 <Indexes>"
                                    "                   <DbIndex>"
                                    "                       <IsUnique>True</IsUnique>"
                                    "                       <Name>uix_sub</Name>"
                                    "                       <Properties>"
                                    "                          <string>SubProp</string>"
                                    "                       </Properties>"
                                    "                   </DbIndex>"
                                    "                 </Indexes>"
                                    "            </ClassMap>"
                                    "        </ECCustomAttributes>"
                                    "       <BaseClass>Root</BaseClass>"
                                    "       <BaseClass>Interface</BaseClass>"
                                    "        <ECProperty propertyName='SubProp' typeName='int' />"
                                    "    </ECEntityClass>"
                                    "    <ECEntityClass typeName='SubSub'>"
                                    "        <ECCustomAttributes>"
                                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                                    "                 <Indexes>"
                                    "                   <DbIndex>"
                                    "                       <IsUnique>True</IsUnique>"
                                    "                       <Name>uix_subsub</Name>"
                                    "                       <Properties>"
                                    "                          <string>SubSubProp</string>"
                                    "                       </Properties>"
                                    "                   </DbIndex>"
                                    "                 </Indexes>"
                                    "            </ClassMap>"
                                    "        </ECCustomAttributes>"
                                    "       <BaseClass>Sub</BaseClass>"
                                    "        <ECProperty propertyName='SubSubProp' typeName='int' />"
                                    "    </ECEntityClass>"
                                    "    <ECEntityClass typeName='Sub2'>"
                                    "        <ECCustomAttributes>"
                                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                                    "                 <Indexes>"
                                    "                   <DbIndex>"
                                    "                       <IsUnique>True</IsUnique>"
                                    "                       <Name>uix_sub2</Name>"
                                    "                       <Properties>"
                                    "                          <string>Sub2Prop</string>"
                                    "                       </Properties>"
                                    "                   </DbIndex>"
                                    "                 </Indexes>"
                                    "            </ClassMap>"
                                    "        </ECCustomAttributes>"
                                    "       <BaseClass>Root</BaseClass>"
                                    "       <BaseClass>Interface</BaseClass>"
                                    "        <ECProperty propertyName='Sub2Prop' typeName='int' />"
                                    "    </ECEntityClass>"
                                    "    <ECEntityClass typeName='RootUnshared' modifier='Abstract'>"
                                    "        <ECCustomAttributes>"
                                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                                    "                 <Indexes>"
                                    "                   <DbIndex>"
                                    "                       <IsUnique>True</IsUnique>"
                                    "                       <Name>uix_rootunshared</Name>"
                                    "                       <Properties>"
                                    "                          <string>RootUnsharedProp</string>"
                                    "                       </Properties>"
                                    "                   </DbIndex>"
                                    "                 </Indexes>"
                                    "            </ClassMap>"
                                    "        </ECCustomAttributes>"
                                    "        <ECProperty propertyName='RootUnsharedProp' typeName='int' />"
                                    "    </ECEntityClass>"
                                    "    <ECEntityClass typeName='SubUnshared'>"
                                    "        <ECCustomAttributes>"
                                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                                    "                 <Indexes>"
                                    "                   <DbIndex>"
                                    "                       <IsUnique>True</IsUnique>"
                                    "                       <Name>uix_subunshared</Name>"
                                    "                       <Properties>"
                                    "                          <string>SubUnsharedProp</string>"
                                    "                       </Properties>"
                                    "                   </DbIndex>"
                                    "                 </Indexes>"
                                    "            </ClassMap>"
                                    "        </ECCustomAttributes>"
                                    "       <BaseClass>RootUnshared</BaseClass>"
                                    "        <ECProperty propertyName='SubUnsharedProp' typeName='int' />"
                                    "    </ECEntityClass>"
                                    "</ECSchema>");

                ECDb db;
                bool asserted = false;
                AssertSchemaImport(db, asserted, testItem, "userdefinedindextest.ecdb");
                ASSERT_FALSE(asserted);

                SchemaItem secondSchema("Index on abstract classes - Schema 2",
                                        "<?xml version='1.0' encoding='utf-8'?>"
                                        "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                        "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts' />"
                                        "    <ECEntityClass typeName='Sub3'>"
                                        "        <ECCustomAttributes>"
                                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                                        "                 <Indexes>"
                                        "                   <DbIndex>"
                                        "                       <IsUnique>True</IsUnique>"
                                        "                       <Name>uix_sub3</Name>"
                                        "                       <Properties>"
                                        "                          <string>Sub3Prop</string>"
                                        "                       </Properties>"
                                        "                   </DbIndex>"
                                        "                 </Indexes>"
                                        "            </ClassMap>"
                                        "        </ECCustomAttributes>"
                                        "       <BaseClass>ts:Root</BaseClass>"
                                        "       <BaseClass>ts:Interface</BaseClass>"
                                        "        <ECProperty propertyName='Sub3Prop' typeName='int' />"
                                        "    </ECEntityClass>"
                                        "    <ECEntityClass typeName='Sub2Unshared'>"
                                        "        <ECCustomAttributes>"
                                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                                        "                 <Indexes>"
                                        "                   <DbIndex>"
                                        "                       <IsUnique>True</IsUnique>"
                                        "                       <Name>uix_sub2unshared</Name>"
                                        "                       <Properties>"
                                        "                          <string>Sub2UnsharedProp</string>"
                                        "                       </Properties>"
                                        "                   </DbIndex>"
                                        "                 </Indexes>"
                                        "            </ClassMap>"
                                        "        </ECCustomAttributes>"
                                        "       <BaseClass>ts:RootUnshared</BaseClass>"
                                        "        <ECProperty propertyName='Sub2UnsharedProp' typeName='int' />"
                                        "    </ECEntityClass>"
                                        " </ECSchema>");

                asserted = false;
                AssertSchemaImport(asserted, db, secondSchema);
                ASSERT_FALSE(asserted);

                //class hierarchy with shared table
                AssertIndex(db, "uix_root", true, "ts_Root", {"RootProp"});

                //index from Interface class is applied to Sub and Sub2 which are stored in joined tables
                AssertIndex(db, "uix_interface_ts_Sub", true, "ts_Sub", {"InterfaceProp"});
                AssertIndex(db, "uix_interface_ts_Sub2", true, "ts_Sub2", {"InterfaceProp"});
                AssertIndex(db, "uix_interface_ts2_Sub3", true, "ts2_Sub3", {"InterfaceProp"});

                AssertIndex(db, "uix_sub", true, "ts_Sub", {"SubProp"});
                AssertIndex(db, "uix_sub2", true, "ts_Sub2", {"Sub2Prop"});
                AssertIndex(db, "uix_sub3", true, "ts2_Sub3", {"Sub3Prop"});

                ECClassCP subSubClass = db.Schemas().GetECClass("TestSchema", "SubSub");
                ASSERT_TRUE(subSubClass != nullptr);
                Utf8String indexWhere;
                indexWhere.Sprintf("ECClassId=%llu", subSubClass->GetId().GetValue());
                AssertIndex(db, "uix_subsub", true, "ts_Sub", {"SubSubProp"}, indexWhere.c_str());

                //class hierarchy without shared table
                AssertIndex(db, "uix_rootunshared_ts_SubUnshared", true, "ts_SubUnshared", {"RootUnsharedProp"});
                AssertIndex(db, "uix_rootunshared_ts2_Sub2Unshared", true, "ts2_Sub2Unshared", {"RootUnsharedProp"});
                AssertIndex(db, "uix_subunshared", true, "ts_SubUnshared", {"SubUnsharedProp"});
                AssertIndex(db, "uix_sub2unshared", true, "ts2_Sub2Unshared", {"Sub2UnsharedProp"});
                }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, IdSpecificationCustomAttributes)
    {
    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.13' prefix='bsca' />"
        "    <ECEntityClass typeName='ClassWithBusinessKey' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <BusinessKeySpecification xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "               <PropertyName>Name</PropertyName>"
        "            </BusinessKeySpecification>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Id' typeName='long' />"
        "        <ECProperty propertyName='Name' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ClassWithSyncId' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <SyncIDSpecification xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "               <Property>Name</Property>"
        "            </SyncIDSpecification>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Id' typeName='long' />"
        "        <ECProperty propertyName='Name' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='ClassWithGlobalId' modifier='None'>"
        "        <ECCustomAttributes>"
        "            <GlobalIdSpecification xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "               <PropertyName>Name</PropertyName>"
        "            </GlobalIdSpecification>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Id' typeName='long' />"
        "        <ECProperty propertyName='Name' typeName='string' />"
        "    </ECEntityClass>"
        "</ECSchema>");

    ECDb db;
    bool asserted = false;
    AssertSchemaImport(db, asserted, testItem, "idspectests.ecdb");
    ASSERT_FALSE(asserted);

    AssertIndex(db, "ix_ts_ClassWithBusinessKey_BusinessKeySpecification_Name", false, "ts_ClassWithBusinessKey", {"Name"});
    AssertIndex(db, "ix_ts_ClassWithSyncId_SyncIDSpecification_Name", false, "ts_ClassWithSyncId", {"Name"});
    AssertIndex(db, "ix_ts_ClassWithGlobalId_GlobalIdSpecification_Name", false, "ts_ClassWithGlobalId", {"Name"});
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, NotNullableProperty)
    {
            {
            SchemaItem testItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "               <Indexes>"
                "                   <DbIndex>"
                "                       <Name>ix_b_id</Name>"
                "                       <Properties>"
                "                           <string>Id</string>"
                "                       </Properties>"
                "                       <Where>IndexedColumnsAreNotNull</Where>"
                "                   </DbIndex>"
                "               </Indexes>"
                "            </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='Id' typeName='long' >"
                "           <ECCustomAttributes>"
                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                "               <IsNullable>false</IsNullable>"
                "            </PropertyMap>"
                "           </ECCustomAttributes>"
                "        </ECProperty>"
                "        <ECProperty propertyName='Name' typeName='string' />"
                "    </ECEntityClass>"
                "</ECSchema>", true, "");

            ECDb db;
            bool asserted = false;
            AssertSchemaImport(db, asserted, testItem, "notnullableproptest1.ecdb");
            ASSERT_FALSE(asserted);
            AssertIndex(db, "ix_b_id", false, "ts1_B", {"Id"});
            }

            {
            SchemaItem testItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "               <Indexes>"
                "                   <DbIndex>"
                "                       <Name>ix_b_id_name</Name>"
                "                       <Properties>"
                "                           <string>Id</string>"
                "                           <string>Name</string>"
                "                       </Properties>"
                "                       <Where>IndexedColumnsAreNotNull</Where>"
                "                   </DbIndex>"
                "               </Indexes>"
                "            </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='Id' typeName='long' >"
                "           <ECCustomAttributes>"
                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                "               <IsNullable>false</IsNullable>"
                "            </PropertyMap>"
                "           </ECCustomAttributes>"
                "        </ECProperty>"
                "        <ECProperty propertyName='Name' typeName='string' />"
                "    </ECEntityClass>"
                "</ECSchema>", true, "");

            ECDb db;
            bool asserted = false;
            AssertSchemaImport(db, asserted, testItem, "notnullableproptest2.ecdb");
            ASSERT_FALSE(asserted);
            AssertIndex(db, "ix_b_id_name", false, "ts2_B", {"Id","Name"}, "([Name] IS NOT NULL)");
            }

            {
            SchemaItem testItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "               <Indexes>"
                "                   <DbIndex>"
                "                       <Name>ix_b_id_name</Name>"
                "                       <Properties>"
                "                           <string>Id</string>"
                "                           <string>Name</string>"
                "                       </Properties>"
                "                       <Where>IndexedColumnsAreNotNull</Where>"
                "                   </DbIndex>"
                "               </Indexes>"
                "            </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='Id' typeName='long' >"
                "           <ECCustomAttributes>"
                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                "               <IsNullable>false</IsNullable>"
                "            </PropertyMap>"
                "           </ECCustomAttributes>"
                "        </ECProperty>"
                "        <ECProperty propertyName='Name' typeName='string'>"
                "           <ECCustomAttributes>"
                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                "               <IsNullable>false</IsNullable>"
                "            </PropertyMap>"
                "           </ECCustomAttributes>"
                "        </ECProperty>"
                "    </ECEntityClass>"
                "</ECSchema>", true, "");

            ECDb db;
            bool asserted = false;
            AssertSchemaImport(db, asserted, testItem, "notnullableproptest3.ecdb");
            ASSERT_FALSE(asserted);
            AssertIndex(db, "ix_b_id_name", false, "ts3_B", {"Id", "Name"});
            }

            {
            SchemaItem testItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts4' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='A' modifier='None'>"
                "        <ECProperty propertyName='Code' typeName='string' />"
                "        <ECProperty propertyName='Id' typeName='long' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECProperty propertyName='AId' typeName='long'>"
                "           <ECCustomAttributes>"
                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                "               <IsNullable>false</IsNullable>"
                "            </PropertyMap>"
                "           </ECCustomAttributes>"
                "        </ECProperty>"
                "        <ECProperty propertyName='Name' typeName='string' />"
                "    </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='embedding'>"
                "    <Source cardinality='(1,1)' polymorphic='True'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target cardinality='(0,N)' polymorphic='True'>"
                "      <Class class='B'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>", true, "");

            ECDb db;
            bool asserted = false;
            AssertSchemaImport(db, asserted, testItem, "notnullableproptest4.ecdb");
            ASSERT_FALSE(asserted);

            AssertIndex(db, "ix_ts4_B_fk_ts4_Rel_target", false, "ts4_B", {"ForeignECInstanceId_ts4_Rel"});

            }

            {
            std::vector<SchemaItem> testItems {
            SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='A' modifier='None'>"
                "        <ECProperty propertyName='Id' typeName='long' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECNavigationProperty propertyName='AId' relationshipName='Rel' direction='Backward'>"
                "           <ECCustomAttributes>"
                "            <PropertyMap xmlns='ECDbMap.02.00'>"
                "            </PropertyMap>"
                "           </ECCustomAttributes>"
                "        </ECNavigationProperty>"
                "        <ECProperty propertyName='Name' typeName='string' />"
                "    </ECEntityClass>"
                "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='embedding'>"
                "    <Source cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='A'/>"
                "    </Source>"
                "    <Target cardinality='(0,N)' polymorphic='True'>"
                "      <Class class='B' />"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>", false, "PropertyMap CA not allowed on navigation property"),

                SchemaItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECEntityClass typeName='A' modifier='None'>"
                    "        <ECProperty propertyName='Id' typeName='long' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='B' modifier='None'>"
                    "        <ECNavigationProperty propertyName='AId' relationshipName='Rel' direction='Backward'>"
                    "           <ECCustomAttributes>"
                    "            <PropertyMap xmlns='ECDbMap.02.00'>"
                    "               <IsNullable>false</IsNullable>"
                    "            </PropertyMap>"
                    "           </ECCustomAttributes>"
                    "        </ECNavigationProperty>"
                    "        <ECProperty propertyName='Name' typeName='string' />"
                    "    </ECEntityClass>"
                    "  <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='embedding'>"
                    "    <Source cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='A'/>"
                    "    </Source>"
                    "    <Target cardinality='(0,N)' polymorphic='True'>"
                    "      <Class class='B' />"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>", false, "PropertyMap CA not allowed on navigation property")
                };

            AssertSchemaImport(testItems, "notnullableproptest.ecdb");
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, IndexCreationForRelationships)
    {
            {
            SchemaItem testItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='A' modifier='None' >"
                "        <ECProperty propertyName='AId' typeName='string' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "            </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECNavigationProperty propertyName='AId' relationshipName='Rel' direction='Backward'/>"
                "        <ECProperty propertyName='BId' typeName='long' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='BB' modifier='None'>"
                "        <BaseClass>B</BaseClass>"
                "        <ECProperty propertyName='BBId' typeName='long' />"
                "    </ECEntityClass>"
                "   <ECRelationshipClass typeName='Rel' strength='embedding' modifier='Sealed'>"
                "    <Source cardinality='(1,1)' polymorphic='True'>"
                "      <Class class='A' />"
                "    </Source>"
                "    <Target cardinality='(0,N)' polymorphic='True'>"
                "      <Class class='B' />"
                "    </Target>"
                "  </ECRelationshipClass>"
                "   <ECRelationshipClass typeName='Rel11' strength='embedding' modifier='Sealed'>"
                "    <Source cardinality='(1,1)' polymorphic='True'>"
                "      <Class class='A' />"
                "    </Source>"
                "    <Target cardinality='(1,1)' polymorphic='True'>"
                "      <Class class='B' />"
                "    </Target>"
                "  </ECRelationshipClass>"
                "   <ECRelationshipClass typeName='Rel11Backwards' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                "    <Source cardinality='(1,1)' polymorphic='True'>"
                "      <Class class='A' />"
                "    </Source>"
                "    <Target cardinality='(1,1)' polymorphic='True'>"
                "      <Class class='B' />"
                "    </Target>"
                "  </ECRelationshipClass>"
                "   <ECRelationshipClass typeName='RelNN' strength='referencing' modifier='Sealed'>"
                "    <Source cardinality='(1,N)' polymorphic='True'>"
                "      <Class class='A' />"
                "    </Source>"
                "    <Target cardinality='(1,N)' polymorphic='True'>"
                "      <Class class='B' />"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>", true, "");

            ECDb ecdb;
            bool asserted = false;
            AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships1.ecdb");
            ASSERT_FALSE(asserted);

            AssertIndex(ecdb, "ix_ts1_B_fk_ts1_Rel_target", false, "ts1_B", {"AId"});
            AssertIndex(ecdb, "uix_ts1_B_fk_ts1_Rel11_target", true, "ts1_B", {"ForeignECInstanceId_ts1_Rel11"});
            AssertIndex(ecdb, "uix_ts1_A_fk_ts1_Rel11Backwards_source", true, "ts1_A", {"ForeignECInstanceId_ts1_Rel11Backwards"});

            AssertIndex(ecdb, "ix_ts1_RelNN_source", false, "ts1_RelNN", {"SourceECInstanceId"});
            AssertIndex(ecdb, "ix_ts1_RelNN_target", false, "ts1_RelNN", {"TargetECInstanceId"});
            AssertIndex(ecdb, "uix_ts1_RelNN_sourcetarget", true, "ts1_RelNN", {"SourceECInstanceId", "TargetECInstanceId"});
            }

                {
                SchemaItem testItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECEntityClass typeName='A' modifier='None' >"
                    "        <ECProperty propertyName='AId' typeName='string' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='B' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "            </ClassMap>"
                    "            <ShareColumns xmlns='ECDbMap.02.00'/>"
                    "        </ECCustomAttributes>"
                    "        <ECProperty propertyName='AId' typeName='long' />"
                    "        <ECProperty propertyName='BId' typeName='long' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='BB' modifier='None'>"
                    "        <BaseClass>B</BaseClass>"
                    "        <ECProperty propertyName='BBId' typeName='long' />"
                    "    </ECEntityClass>"
                    "   <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='embedding'>"
                    "    <Source cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='A' />"
                    "    </Source>"
                    "    <Target cardinality='(0,N)' polymorphic='True'>"
                    "      <Class class='B' />"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>", true, "");

                ECDb ecdb;
                bool asserted = false;
                AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships2.ecdb");
                ASSERT_FALSE(asserted);
                AssertIndex(ecdb, "ix_ts2_B_fk_ts2_Rel_target", false, "ts2_B", {"ForeignECInstanceId_ts2_Rel"}, "([ForeignECInstanceId_ts2_Rel] IS NOT NULL)");
                }

                {
                SchemaItem testItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECEntityClass typeName='A' modifier='None' >"
                    "        <ECProperty propertyName='Id' typeName='string' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='B' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "            </ClassMap>"
                    "            <ShareColumns xmlns='ECDbMap.02.00'/>"
                    "        </ECCustomAttributes>"
                    "        <ECProperty propertyName='BId' typeName='long' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='BB' modifier='None'>"
                    "        <BaseClass>B</BaseClass>"
                    "        <ECProperty propertyName='BBId' typeName='long' />"
                    "        <ECNavigationProperty propertyName='AId' relationshipName='Rel' direction='Backward' />"
                    "    </ECEntityClass>"
                    "   <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='embedding'>"
                    "    <Source cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='A' />"
                    "    </Source>"
                    "    <Target cardinality='(0,N)' polymorphic='True'>"
                    "      <Class class='BB'/>"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>", true);

                ECDb ecdb;
                bool asserted = false;
                AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships3.ecdb");
                ASSERT_FALSE(asserted);

                AssertIndex(ecdb, "ix_ts3_B_fk_ts3_Rel_target", false, "ts3_B", {"AId"}, "([AId] IS NOT NULL)");
                }

                {
                SchemaItem testItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts4' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECEntityClass typeName='A' modifier='None' >"
                    "        <ECProperty propertyName='Id' typeName='string' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='B' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "            </ClassMap>"
                    "            <ShareColumns xmlns='ECDbMap.02.00'/>"
                    "        </ECCustomAttributes>"
                    "        <ECNavigationProperty propertyName='AId' relationshipName='Rel11' direction='Backward' />"
                    "        <ECProperty propertyName='BId' typeName='long' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='BB' modifier='None'>"
                    "        <BaseClass>B</BaseClass>"
                    "        <ECProperty propertyName='BBId' typeName='long' />"
                    "    </ECEntityClass>"
                    "   <ECRelationshipClass typeName='Rel11' modifier='Sealed' >"
                    "    <Source cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='A' />"
                    "    </Source>"
                    "    <Target cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='B'/>"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>", true, "");

                ECDb ecdb;
                bool asserted = false;
                AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships4.ecdb");
                ASSERT_FALSE(asserted);

                AssertIndex(ecdb, "uix_ts4_B_fk_ts4_Rel11_target", true, "ts4_B", {"AId"}, "([AId] IS NOT NULL)");
                }

                {
                SchemaItem testItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts50' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECEntityClass typeName='A' modifier='None'>"
                    "        <ECProperty propertyName='Id' typeName='string' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='B' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "             </ClassMap>"
                    "        </ECCustomAttributes>"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='B1' modifier='None'>"
                    "        <BaseClass>B</BaseClass>"
                    "        <ECProperty propertyName='B1Id' typeName='long' />"
                    "    </ECEntityClass>"
                    "   <ECRelationshipClass typeName='RelBase' modifier='Abstract' strength='referencing'>"
                    "    <Source cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='A'/>"
                    "    </Source>"
                    "    <Target cardinality='(1,N)' polymorphic='True'>"
                    "      <Class class='B'/>"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "   <ECRelationshipClass typeName='RelSub1' modifier='Sealed' strength='referencing'>"
                    "    <BaseClass>RelBase</BaseClass>"
                    "    <Source cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='A' />"
                    "    </Source>"
                    "    <Target cardinality='(1,1)' polymorphic='True'>"
                    "      <Class class='B1'/>"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>", true, "");

                ECDb ecdb;
                bool asserted = false;
                AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships50.ecdb");
                ASSERT_FALSE(asserted);

                ASSERT_EQ(3, (int) RetrieveIndicesForTable(ecdb, "ts50_B").size()) << "Expected indices: class id index, user defined index; no indexes for the relationship constraints";

                AssertIndex(ecdb, "ix_ts50_B_fk_ts50_RelBase_target", false, "ts50_B", {"ForeignECInstanceId_ts50_RelBase"}, "([ForeignECInstanceId_ts50_RelBase] IS NOT NULL)");
                AssertIndex(ecdb, "ix_ts50_B_RelECClassId_ts50_RelBase", false, "ts50_B", {"RelECClassId_ts50_RelBase"}, "([RelECClassId_ts50_RelBase] IS NOT NULL)");
                AssertIndexExists(ecdb, "uix_ts50_B_fk_ts50_RelSub1_target", false);
                }

                {
                SchemaItem testItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts5' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECEntityClass typeName='A' modifier='None'>"
                    "        <ECProperty propertyName='Id' typeName='string' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='B' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "             </ClassMap>"
                    "        </ECCustomAttributes>"
                    "        <ECNavigationProperty propertyName='AId' relationshipName='RelBase' direction='Backward' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='B1' modifier='None'>"
                    "        <BaseClass>B</BaseClass>"
                    "        <ECProperty propertyName='B1Id' typeName='long' />"
                    "    </ECEntityClass>"
                    "   <ECRelationshipClass typeName='RelBase' modifier='Abstract' strength='referencing'>"
                    "    <Source cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='A'/>"
                    "    </Source>"
                    "    <Target cardinality='(1,N)' polymorphic='True'>"
                    "      <Class class='B'/>"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "   <ECRelationshipClass typeName='RelSub1' modifier='Sealed' strength='referencing'>"
                    "    <BaseClass>RelBase</BaseClass>"
                    "    <Source cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='A' />"
                    "    </Source>"
                    "    <Target cardinality='(1,1)' polymorphic='True'>"
                    "      <Class class='B1'/>"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>", true, "");

                ECDb ecdb;
                bool asserted = false;
                AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships5.ecdb");
                ASSERT_FALSE(asserted);

                ASSERT_EQ(3, (int) RetrieveIndicesForTable(ecdb, "ts5_B").size()) << "Expected indices: class id index, user defined index; no indexes for the relationship constraints";

                AssertIndex(ecdb, "ix_ts5_B_fk_ts5_RelBase_target", false, "ts5_B", {"AId"}, "([AId] IS NOT NULL)");
                AssertIndex(ecdb, "ix_ts5_B_ARelECClassId", false, "ts5_B", {"ARelECClassId"}, "([ARelECClassId] IS NOT NULL)");
                AssertIndexExists(ecdb, "uix_ts5_B_fk_ts5_RelSub1_target", false);
                }

                {
                SchemaItem testItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts6' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECEntityClass typeName='A' modifier='None'>"
                    "        <ECProperty propertyName='Id' typeName='string' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='B' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "             </ClassMap>"
                    "        </ECCustomAttributes>"
                    "        <ECNavigationProperty propertyName='AInstance' relationshipName='RelBase' direction='Backward' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='B1' modifier='None'>"
                    "        <BaseClass>B</BaseClass>"
                    "        <ECProperty propertyName='B1Id' typeName='long' />"
                    "    </ECEntityClass>"
                    "   <ECRelationshipClass typeName='RelBase' modifier='Abstract' strength='referencing'>"
                    "    <Source cardinality='(1,1)' polymorphic='True'>"
                    "      <Class class='A'/>"
                    "    </Source>"
                    "    <Target cardinality='(1,N)' polymorphic='True'>"
                    "      <Class class='B'/>"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "   <ECRelationshipClass typeName='RelSub1' modifier='Sealed' strength='referencing'>"
                    "    <BaseClass>RelBase</BaseClass>"
                    "    <Source cardinality='(1,1)' polymorphic='True'>"
                    "      <Class class='A' />"
                    "    </Source>"
                    "    <Target cardinality='(1,1)' polymorphic='True'>"
                    "      <Class class='B1'/>"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>", true, "");

                ECDb ecdb;
                bool asserted = false;
                AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships6.ecdb");
                ASSERT_FALSE(asserted);

                ASSERT_EQ(3, (int) RetrieveIndicesForTable(ecdb, "ts6_B").size()) << "Expected indices: class id index, user defined index; no indexes for the relationship constraints";

                AssertIndex(ecdb, "ix_ts6_B_AInstanceRelECClassId", false, "ts6_B", {"AInstanceRelECClassId"});
                AssertIndex(ecdb, "ix_ts6_B_fk_ts6_RelBase_target", false, "ts6_B", {"AInstance"});
                AssertIndexExists(ecdb, "uix_ts6_B_fk_ts6_RelSub1_target", false);
                }


                {
                SchemaItem testItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts7' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECEntityClass typeName='B' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "             </ClassMap>"
                    "        </ECCustomAttributes>"
                    "        <ECProperty propertyName='Id' typeName='long' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='B1' modifier='None'>"
                    "        <BaseClass>B</BaseClass>"
                    "        <ECProperty propertyName='B1Id' typeName='long' />"
                    "    </ECEntityClass>"
                    "   <ECRelationshipClass typeName='RelBase' modifier='Abstract' strength='referencing'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "             </ClassMap>"
                    "            <ShareColumns xmlns='ECDbMap.02.00'/>"
                    "        </ECCustomAttributes>"
                    "    <Source cardinality='(0,N)' polymorphic='True'>"
                    "      <Class class='B'/>"
                    "    </Source>"
                    "    <Target cardinality='(1,N)' polymorphic='True'>"
                    "      <Class class='B' />"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "   <ECRelationshipClass typeName='RelSub11' modifier='Sealed' strength='referencing'>"
                    "    <BaseClass>RelBase</BaseClass>"
                    "    <Source cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='B' />"
                    "    </Source>"
                    "    <Target cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='B1' />"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "   <ECRelationshipClass typeName='RelSub1N' modifier='Sealed' strength='referencing'>"
                    "    <BaseClass>RelBase</BaseClass>"
                    "    <Source cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='B1' />"
                    "    </Source>"
                    "    <Target cardinality='(1,N)' polymorphic='True'>"
                    "      <Class class='B1' />"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>", true, "");

                ECDb ecdb;
                bool asserted = false;
                AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships7.ecdb");
                ASSERT_FALSE(asserted);

                ASSERT_EQ(9, (int) RetrieveIndicesForTable(ecdb, "ts7_RelBase").size());
                }

                {
                SchemaItem testItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts8' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                    "    <ECEntityClass typeName='A' modifier='None'>"
                    "        <ECProperty propertyName='AId' typeName='long' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='B' modifier='None'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.02.00'>"
                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                    "             </ClassMap>"
                    "        </ECCustomAttributes>"
                    "        <ECProperty propertyName='Id' typeName='long' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='B1' modifier='None'>"
                    "        <BaseClass>B</BaseClass>"
                    "        <ECProperty propertyName='B1Id' typeName='long' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='B11' modifier='None'>"
                    "        <BaseClass>B1</BaseClass>"
                    "        <ECProperty propertyName='B11Id' typeName='long' />"
                    "    </ECEntityClass>"
                    "    <ECEntityClass typeName='B2' modifier='None'>"
                    "        <BaseClass>B</BaseClass>"
                    "        <ECProperty propertyName='B2Id' typeName='long' />"
                    "    </ECEntityClass>"
                    "   <ECRelationshipClass typeName='RelNonPoly' modifier='Sealed' strength='referencing'>"
                    "    <Source cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='A' />"
                    "    </Source>"
                    "    <Target cardinality='(0,1)' polymorphic='False'>"
                    "      <Class class='B1' />"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "   <ECRelationshipClass typeName='RelPoly' modifier='Sealed' strength='referencing'>"
                    "    <Source cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='A' />"
                    "    </Source>"
                    "    <Target cardinality='(0,1)' polymorphic='True'>"
                    "      <Class class='B1' />"
                    "    </Target>"
                    "  </ECRelationshipClass>"
                    "</ECSchema>", true, "");

                ECDb ecdb;
                bool asserted = false;
                AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships8.ecdb");
                ASSERT_FALSE(asserted);

                ASSERT_EQ(3, (int) RetrieveIndicesForTable(ecdb, "ts8_B").size());

                ECClassId b1ClassId = ecdb.Schemas().GetECClassId("TestSchema", "B1");
                ECClassId b11ClassId = ecdb.Schemas().GetECClassId("TestSchema", "B11");

                //RelNonPoly must exclude index on B11 as the constraint is non-polymorphic
                Utf8String indexWhereClause;
                indexWhereClause.Sprintf("([ForeignECInstanceId_ts8_RelNonPoly] IS NOT NULL) AND (ECClassId=%s)", b1ClassId.ToString().c_str());
                AssertIndex(ecdb, "uix_ts8_B_fk_ts8_RelNonPoly_target", true, "ts8_B", {"ForeignECInstanceId_ts8_RelNonPoly"}, indexWhereClause.c_str());

                //RelPoly must include index on B11 as the constraint is polymorphic
                indexWhereClause.Sprintf("([ForeignECInstanceId_ts8_RelPoly] IS NOT NULL) AND (ECClassId=%s OR ECClassId=%s)", b1ClassId.ToString().c_str(), b11ClassId.ToString().c_str());
                AssertIndex(ecdb, "uix_ts8_B_fk_ts8_RelPoly_target", true, "ts8_B", {"ForeignECInstanceId_ts8_RelPoly"}, indexWhereClause.c_str());
                }

                {
                //Tests that AllowDuplicateRelationships Flag from LinkTableRelationshipMap CA is applied to subclasses
                SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts9\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                                    "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                                    "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                                    "  <ECEntityClass typeName='A' modifier='None'>"
                                    "    <ECProperty propertyName='Name' typeName='string' />"
                                    "  </ECEntityClass>"
                                    "  <ECEntityClass typeName='B' modifier='None'>"
                                    "    <ECProperty propertyName='BName' typeName='string' />"
                                    "  </ECEntityClass>"
                                    "  <ECEntityClass typeName='C' modifier='None'>"
                                    "    <ECProperty propertyName='CName' typeName='string' />"
                                    "  </ECEntityClass>"
                                    "  <ECRelationshipClass typeName='ARelB' modifier='Abstract' strength='referencing'>"
                                    "    <ECCustomAttributes>"
                                    "        <ClassMap xmlns='ECDbMap.02.00'>"
                                    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                    "        </ClassMap>"
                                    "        <LinkTableRelationshipMap xmlns='ECDbMap.02.00'>"
                                    "             <AllowDuplicateRelationships>True</AllowDuplicateRelationships>"
                                    "        </LinkTableRelationshipMap>"
                                    "    </ECCustomAttributes>"
                                    "    <Source cardinality='(0,N)' polymorphic='True'>"
                                    "      <Class class = 'A' />"
                                    "    </Source>"
                                    "    <Target cardinality='(0,N)' polymorphic='True'>"
                                    "      <Class class = 'B' />"
                                    "    </Target>"
                                    "  </ECRelationshipClass>"
                                    "  <ECRelationshipClass typeName='ARelC' modifier='Sealed' strength='referencing'>"
                                    "    <BaseClass>ARelB</BaseClass>"
                                    "    <Source cardinality='(0,1)' polymorphic='True'>"
                                    "      <Class class = 'A' />"
                                    "    </Source>"
                                    "    <Target cardinality='(0,N)' polymorphic='True'>"
                                    "      <Class class = 'C' />"
                                    "    </Target>"
                                    "  </ECRelationshipClass>"
                                    "</ECSchema>", true, "");

                ECDb ecdb;
                bool asserted = false;
                AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships9.ecdb");
                ASSERT_FALSE(asserted);

                ASSERT_TRUE(ecdb.TableExists("ts9_ARelB"));
                ASSERT_FALSE(ecdb.TableExists("ts9_ARelC")) << "ARelC is expected to be persisted in ts9_ARelB as well (SharedTable strategy)";

                //ARelB must not have a unique index on source and target as it as AllowDuplicateRelationship set to true.
                //ARelC must not have the unique index either, as AllowDuplicateRelationship is applied to subclasses
                std::vector<IndexInfo> indexes = RetrieveIndicesForTable(ecdb, "ts9_ARelB");
                ASSERT_EQ(3, (int) indexes.size()) << "Indexes on ts9_ARelB";
                ASSERT_STREQ("ix_ts9_ARelB_ecclassid", indexes[0].m_name.c_str());
                ASSERT_STREQ("ix_ts9_ARelB_source", indexes[1].m_name.c_str());
                ASSERT_STREQ("ix_ts9_ARelB_target", indexes[2].m_name.c_str());
                }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, NotNullConstraintsOnFkColumns)
    {
    auto getDdl = [] (Utf8StringR ddl, ECDbCR ecdb, Utf8CP tableName)
        {
        CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT sql FROM sqlite_master WHERE name=?");
        ASSERT_TRUE(stmt != nullptr);

        ASSERT_EQ(BE_SQLITE_OK, stmt->BindText(1, tableName, Statement::MakeCopy::No)) << stmt->GetSql();
        ASSERT_EQ(BE_SQLITE_ROW, stmt->Step()) << stmt->GetSql();
        ddl.assign(stmt->GetValueText(0));
        };

    {
    SchemaItem testItem("Plain relationship classes",
                        "<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "    <ECEntityClass typeName='A'>"
                        "        <ECProperty propertyName='AName' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B'>"
                        "        <ECProperty propertyName='BName' typeName='string' />"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='Rel1N' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(1,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  <ECRelationshipClass typeName='RelN1' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                        "    <Source cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Source>"
                        "    <Target cardinality='(1,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  <ECRelationshipClass typeName='Rel0N' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  <ECRelationshipClass typeName='RelN0' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                        "    <Source cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Source>"
                        "    <Target cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "notnullconstraintsonfkcolumns.ecdb");
    ASSERT_FALSE(asserted);

    Utf8String ddl;
    getDdl(ddl, ecdb, "ts_B");
    ASSERT_FALSE(ddl.empty());

    ASSERT_TRUE(ddl.ContainsI("[ForeignECInstanceId_ts_Rel0N] INTEGER,"));
    ASSERT_TRUE(ddl.ContainsI("[ForeignECInstanceId_ts_Rel1N] INTEGER NOT NULL,"));
    ASSERT_TRUE(ddl.ContainsI("[ForeignECInstanceId_ts_RelN0] INTEGER,"));
    ASSERT_TRUE(ddl.ContainsI("[ForeignECInstanceId_ts_RelN1] INTEGER NOT NULL,"));
    }

    {
    SchemaItem testItem("relationship classes with nav props",
                        "<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "    <ECEntityClass typeName='A'>"
                        "        <ECProperty propertyName='AName' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B'>"
                        "        <ECProperty propertyName='BName' typeName='string' />"
                        "        <ECNavigationProperty propertyName='AId_Rel1N' relationshipName='Rel1N' direction='Backward' />"
                        "        <ECNavigationProperty propertyName='AId_Rel0N' relationshipName='Rel0N' direction='Backward' />"
                        "        <ECNavigationProperty propertyName='AId_RelN1' relationshipName='RelN1' direction='Forward' />"
                        "        <ECNavigationProperty propertyName='AId_RelN0' relationshipName='RelN0' direction='Forward' />"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='Rel1N' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(1,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  <ECRelationshipClass typeName='RelN1' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                        "    <Source cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Source>"
                        "    <Target cardinality='(1,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  <ECRelationshipClass typeName='Rel0N' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  <ECRelationshipClass typeName='RelN0' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                        "    <Source cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Source>"
                        "    <Target cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "notnullconstraintsonfkcolumns.ecdb");
    ASSERT_FALSE(asserted);

    Utf8String ddl;
    getDdl(ddl, ecdb, "ts_B");
    ASSERT_FALSE(ddl.empty());

    ASSERT_TRUE(ddl.ContainsI("[AId_Rel0N] INTEGER,"));
    ASSERT_TRUE(ddl.ContainsI("[AId_Rel1N] INTEGER NOT NULL,"));
    ASSERT_TRUE(ddl.ContainsI("[AId_RelN0] INTEGER,"));
    ASSERT_TRUE(ddl.ContainsI("[AId_RelN1] INTEGER NOT NULL,"));
    }

    {
    SchemaItem testItem("relationship classes with custom fk names",
                        "<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='A'>"
                        "        <ECProperty propertyName='AName' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B'>"
                        "        <ECProperty propertyName='BName' typeName='string' />"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='Rel1N' strength='embedding' modifier='Sealed'>"
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
                        "             <ForeignKeyColumn>AId_Rel1N</ForeignKeyColumn>"
                        "        </ForeignKeyRelationshipMap>"
                        "    </ECCustomAttributes>"
                        "    <Source cardinality='(1,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  <ECRelationshipClass typeName='RelN1' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
                        "             <ForeignKeyColumn>AId_RelN1</ForeignKeyColumn>"
                        "        </ForeignKeyRelationshipMap>"
                        "    </ECCustomAttributes>"
                        "    <Source cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Source>"
                        "    <Target cardinality='(1,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  <ECRelationshipClass typeName='Rel0N' strength='embedding' modifier='Sealed'>"
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
                        "             <ForeignKeyColumn>AId_Rel0N</ForeignKeyColumn>"
                        "        </ForeignKeyRelationshipMap>"
                        "    </ECCustomAttributes>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  <ECRelationshipClass typeName='RelN0' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
                        "             <ForeignKeyColumn>AId_RelN0</ForeignKeyColumn>"
                        "        </ForeignKeyRelationshipMap>"
                        "    </ECCustomAttributes>"
                        "    <Source cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Source>"
                        "    <Target cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "notnullconstraintsonfkcolumns.ecdb");
    ASSERT_FALSE(asserted);

    Utf8String ddl;
    getDdl(ddl, ecdb, "ts_B");
    ASSERT_FALSE(ddl.empty());

    ASSERT_TRUE(ddl.ContainsI("[AId_Rel0N] INTEGER,"));
    ASSERT_TRUE(ddl.ContainsI("[AId_Rel1N] INTEGER NOT NULL,"));
    ASSERT_TRUE(ddl.ContainsI("[AId_RelN0] INTEGER,"));
    ASSERT_TRUE(ddl.ContainsI("[AId_RelN1] INTEGER NOT NULL,"));
    }


    {
    SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='A'>"
                        "        <ECProperty propertyName='AName' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B'>"
                        "      <ECCustomAttributes>"
                        "        <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                        "        </ClassMap>"
                        "      </ECCustomAttributes>"
                        "       <ECProperty propertyName='BName' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='BSub'>"
                        "        <BaseClass>B</BaseClass>"
                        "        <ECProperty propertyName='BSubName' typeName='string' />"
                        "        <ECNavigationProperty propertyName='AId' relationshipName='Rel' direction='Backward' />"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='Rel' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(1,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'BSub'/>"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>", true, "(1,1) rel with dropped NOT NULL constraint");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "notnullconstraintsonfkcolumns.ecdb");
    ASSERT_FALSE(asserted);

    Utf8String ddl;
    getDdl(ddl, ecdb, "ts_B");
    ASSERT_FALSE(ddl.empty());

    ASSERT_TRUE(ddl.ContainsI("[AId] INTEGER,"));
    }

    {
    SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='A'>"
                        "        <ECProperty propertyName='AName' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
                        "      <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                        "        </ClassMap>"
                        "      </ECCustomAttributes>"
                        "       <ECProperty propertyName='BaseName' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='Sub' modifier='Abstract'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='SubName' typeName='string' />"
                        "        <ECNavigationProperty propertyName='AId' relationshipName='Rel' direction='Backward'/>"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='SubSub'>"
                        "        <BaseClass>Sub</BaseClass>"
                        "        <ECProperty propertyName='SubSubName' typeName='string' />"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='Rel' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(1,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'Sub' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "notnullconstraintsonfk.ecdb");
    ASSERT_FALSE(asserted);

    Utf8String ddl;
    getDdl(ddl, ecdb, "ts_Base");
    ASSERT_FALSE(ddl.empty());
    ASSERT_TRUE(ddl.ContainsI("[AId] INTEGER,")) << "Actual DDL: " << ddl.c_str();
    }

    {
    SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='A'>"
                        "        <ECProperty propertyName='AName' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='Base' modifier='None'>"
                        "      <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                        "        </ClassMap>"
                        "      </ECCustomAttributes>"
                        "       <ECProperty propertyName='BaseName' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='Sub' modifier='Abstract'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='SubName' typeName='string' />"
                        "        <ECNavigationProperty propertyName='AId' relationshipName='Rel' direction='Backward'/>"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='SubSub'>"
                        "        <BaseClass>Sub</BaseClass>"
                        "        <ECProperty propertyName='SubSubName' typeName='string' />"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='Rel' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(1,1)' polymorphic='True'>"
                        "      <Class class = 'A' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'Sub' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "notnullconstraintsonfk.ecdb");
    ASSERT_FALSE(asserted);

    Utf8String ddl;
    getDdl(ddl, ecdb, "ts_Base");
    ASSERT_FALSE(ddl.empty());
    ASSERT_TRUE(ddl.ContainsI("[AId] INTEGER,")) << "Actual DDL: " << ddl.c_str();
    }

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, ForeignKeyColumnPosition)
    {
    auto assertColumnPosition = [] (ECDbCR ecdb, Utf8CP tableName, Utf8CP columnName, int expectedPosition, Utf8CP scenario)
        {
        bvector<Utf8String> colNames;
        ASSERT_TRUE(ecdb.GetColumns(colNames, tableName)) << "Scenario: " << scenario << " Table: " << tableName << " Column: " << columnName << " Expected position: " << expectedPosition;

        const int colCount = (int) colNames.size();
        if (expectedPosition < 0)
            expectedPosition = colCount - 1;
        else
            ASSERT_LE(expectedPosition, colCount - 1) << "Scenario: " << scenario << " Table: " << tableName << " Column: " << columnName << " Expected position: " << expectedPosition;

        int actualPosition = 0;
        for (Utf8StringCR colName : colNames)
            {
            if (colName.EqualsI(columnName))
                break;

            actualPosition++;
            }

        ASSERT_EQ(expectedPosition, actualPosition) << "Scenario: " << scenario << " Table: " << tableName << " Column: " << columnName << " Expected position: " << expectedPosition;
        };

    {
    SchemaItem testItem("No NavProp", "<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='Parent'>"
                        "        <ECProperty propertyName='ParentProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
                        "      <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                        "        </ClassMap>"
                        "      </ECCustomAttributes>"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='A'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='AProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='AA'>"
                        "        <BaseClass>A</BaseClass>"
                        "        <ECProperty propertyName='AAProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='BProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='C'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='CProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='Rel' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "fkcolumnposition.ecdb");
    ASSERT_FALSE(asserted);

    AssertForeignKey(true, ecdb, "ts_Base", "ForeignECInstanceId_ts_Rel");
    assertColumnPosition(ecdb, "ts_Base", "ForeignECInstanceId_ts_Rel", -1, testItem.m_name.c_str());
    }


    {
    SchemaItem testItem("Nav Prop as first prop", "<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='Parent'>"
                        "        <ECProperty propertyName='ParentProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
                        "      <ECCustomAttributes>"
                        "         <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                        "        </ClassMap>"
                        "      </ECCustomAttributes>"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='A'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='AProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECNavigationProperty propertyName='ParentId' relationshipName='Rel' direction='Backward' />"
                        "        <ECProperty propertyName='BProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='AA'>"
                        "        <BaseClass>A</BaseClass>"
                        "        <ECProperty propertyName='AAProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='C'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='CProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='Rel' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "fkcolumnposition.ecdb");
    ASSERT_FALSE(asserted);

    AssertForeignKey(true, ecdb, "ts_Base", "ParentId");
    //Subsubclasses come before sibling classes, therefore parent id is after AAProp1
    assertColumnPosition(ecdb, "ts_Base", "ParentId", 4, testItem.m_name.c_str());
    }

    {
    SchemaItem testItem("Nav prop as last property", "<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='Parent'>"
                        "        <ECProperty propertyName='ParentProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
                        "      <ECCustomAttributes>"
                        "        <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                        "        </ClassMap>"
                        "      </ECCustomAttributes>"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='A'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='AProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='AA'>"
                        "        <BaseClass>A</BaseClass>"
                        "        <ECProperty propertyName='AAProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='BProp1' typeName='string' />"
                        "        <ECNavigationProperty propertyName='ParentId' relationshipName='Rel' direction='Backward' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='C'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='CProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='Rel' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "fkcolumnposition.ecdb");
    ASSERT_FALSE(asserted);

    AssertForeignKey(true, ecdb, "ts_Base", "ParentId");
    assertColumnPosition(ecdb, "ts_Base", "ParentId", 5, testItem.m_name.c_str());
    }

    {
    SchemaItem testItem("Nav prop is only prop", "<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='Parent'>"
                        "        <ECProperty propertyName='ParentProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
                        "      <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                        "        </ClassMap>"
                        "      </ECCustomAttributes>"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='A'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='AProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='AA'>"
                        "        <BaseClass>A</BaseClass>"
                        "        <ECProperty propertyName='AAProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECNavigationProperty propertyName='ParentId' relationshipName='Rel' direction='Backward' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='C'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='CProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='Rel' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "fkcolumnposition.ecdb");
    ASSERT_FALSE(asserted);

    AssertForeignKey(true, ecdb, "ts_Base", "ParentId");
    assertColumnPosition(ecdb, "ts_Base", "ParentId", 2, testItem.m_name.c_str());
    }

    {
    SchemaItem testItem("Nav Prop in class with shared columns", "<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "    <ECEntityClass typeName='Parent'>"
                        "        <ECProperty propertyName='ParentProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='Base' modifier='Abstract'>"
                        "      <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                        "        </ClassMap>"
                        "            <ShareColumns xmlns='ECDbMap.02.00'>"
                        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                        "            </ShareColumns>"
                        "      </ECCustomAttributes>"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='A'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='AProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='AA'>"
                        "        <BaseClass>A</BaseClass>"
                        "        <ECProperty propertyName='AAProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='B'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECNavigationProperty propertyName='ParentId' relationshipName='Rel' direction='Backward' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='C'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='CProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='Rel' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "fkcolumnposition.ecdb");
    ASSERT_FALSE(asserted);

    AssertForeignKey(true, ecdb, "ts_Base", "ParentId");
    assertColumnPosition(ecdb, "ts_Base", "ParentId", -1, testItem.m_name.c_str());
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, OneToOneRelationshipMapping)
    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem("embedding relationships", "<?xml version='1.0' encoding='utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "    <ECEntityClass typeName='A'>"
                                     "        <ECProperty propertyName='AName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B'>"
                                     "        <ECProperty propertyName='BName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "  <ECRelationshipClass typeName='Rel11' strength='embedding' modifier='Sealed'>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel01' strength='embedding' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel10' strength='embedding' modifier='Sealed'>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel00' strength='embedding' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel11back' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel01back' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel10back' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel00back' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "</ECSchema>"));

    testSchemas.push_back(SchemaItem("holding relationships", "<?xml version='1.0' encoding='utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "    <ECEntityClass typeName='A'>"
                                     "        <ECProperty propertyName='AName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B'>"
                                     "        <ECProperty propertyName='BName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "  <ECRelationshipClass typeName='Rel11' strength='holding' modifier='Sealed'>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel01' strength='holding' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel10' strength='holding' modifier='Sealed'>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel00' strength='holding' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel11back' strength='holding' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel01back' strength='holding' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel10back' strength='holding' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel00back' strength='holding' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "</ECSchema>"));

    testSchemas.push_back(SchemaItem("referencing relationships", "<?xml version='1.0' encoding='utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "    <ECEntityClass typeName='A'>"
                                     "        <ECProperty propertyName='AName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B'>"
                                     "        <ECProperty propertyName='BName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "  <ECRelationshipClass typeName='Rel11' strength='referencing' modifier='Sealed'>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel01' strength='referencing' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel10' strength='referencing' modifier='Sealed'>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel00' strength='referencing' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel11back' strength='referencing' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel01back' strength='referencing' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel10back' strength='referencing' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel00back' strength='referencing' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "</ECSchema>"));

    for (SchemaItem const& testSchema : testSchemas)
        {
        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testSchema, "onetoonerelationshipmappings.ecdb");
        ASSERT_FALSE(asserted);

        AssertForeignKey(true, ecdb, "ts_b", "ForeignECInstanceId_ts_Rel11");
        AssertForeignKey(false, ecdb, "ts_a", "ForeignECInstanceId_ts_Rel11");
        AssertForeignKey(true, ecdb, "ts_b", "ForeignECInstanceId_ts_Rel10");
        AssertForeignKey(false, ecdb, "ts_a", "ForeignECInstanceId_ts_Rel10");
        AssertForeignKey(true, ecdb, "ts_b", "ForeignECInstanceId_ts_Rel01");
        AssertForeignKey(false, ecdb, "ts_a", "ForeignECInstanceId_ts_Rel01");
        AssertForeignKey(true, ecdb, "ts_b", "ForeignECInstanceId_ts_Rel00");
        AssertForeignKey(false, ecdb, "ts_a", "ForeignECInstanceId_ts_Rel00");

        AssertForeignKey(false, ecdb, "ts_b", "ForeignECInstanceId_ts_Rel11back");
        AssertForeignKey(true, ecdb, "ts_a", "ForeignECInstanceId_ts_Rel11back");
        AssertForeignKey(false, ecdb, "ts_b", "ForeignECInstanceId_ts_Rel10back");
        AssertForeignKey(true, ecdb, "ts_a", "ForeignECInstanceId_ts_Rel10back");
        AssertForeignKey(false, ecdb, "ts_b", "ForeignECInstanceId_ts_Rel01back");
        AssertForeignKey(true, ecdb, "ts_a", "ForeignECInstanceId_ts_Rel01back");
        AssertForeignKey(false, ecdb, "ts_b", "ForeignECInstanceId_ts_Rel00back");
        AssertForeignKey(true, ecdb, "ts_a", "ForeignECInstanceId_ts_Rel00back");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, DisallowCascadingDeleteOnJoinedTable)
    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                     "    <ECEntityClass typeName='A'>"
                                     "        <ECProperty propertyName='AName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B'>"
                                     "      <ECCustomAttributes>"
                                     "            <ClassMap xmlns='ECDbMap.02.00'>"
                                     "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                     "        </ClassMap>"
                                     "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                     "     </ECCustomAttributes>"
                                     "        <ECProperty propertyName='BName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B1'>"
                                     "        <BaseClass>B</BaseClass>"
                                     "        <ECProperty propertyName='B1Name' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B2'>"
                                     "        <BaseClass>B</BaseClass>"
                                     "        <ECProperty propertyName='B2Name' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "  <ECRelationshipClass typeName='Rel1' strength='embedding' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,N)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel2' strength='embedding' modifier='Sealed'>"
                                     "      <ECCustomAttributes>"
                                     "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
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
                                     "  <ECRelationshipClass typeName='Rel3' strength='embedding' modifier='Sealed'>"
                                     "      <ECCustomAttributes>"
                                     "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
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
                                     true, "Supported cases"));

    testSchemas.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                     "    <ECEntityClass typeName='A'>"
                                     "        <ECProperty propertyName='AName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B'>"
                                     "      <ECCustomAttributes>"
                                     "            <ClassMap xmlns='ECDbMap.02.00'>"
                                     "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                     "        </ClassMap>"
                                     "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                     "     </ECCustomAttributes>"
                                     "        <ECProperty propertyName='BName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B1'>"
                                     "        <BaseClass>B</BaseClass>"
                                     "        <ECProperty propertyName='B1Name' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B2'>"
                                     "        <BaseClass>B</BaseClass>"
                                     "        <ECProperty propertyName='B2Name' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "  <ECRelationshipClass typeName='Rel' strength='embedding' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,N)' polymorphic='True'>"
                                     "      <Class class = 'B1'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  </ECSchema>",
                                     false, "Embedding relationship against subclass (joined table)"));

    testSchemas.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                     "    <ECEntityClass typeName='A'>"
                                     "        <ECProperty propertyName='AName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B'>"
                                     "      <ECCustomAttributes>"
                                     "            <ClassMap xmlns='ECDbMap.02.00'>"
                                     "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                     "        </ClassMap>"
                                     "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                     "     </ECCustomAttributes>"
                                     "        <ECProperty propertyName='BName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B1'>"
                                     "        <BaseClass>B</BaseClass>"
                                     "        <ECProperty propertyName='B1Name' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B2'>"
                                     "        <BaseClass>B</BaseClass>"
                                     "        <ECProperty propertyName='B2Name' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "  <ECRelationshipClass typeName='Rel' strength='embedding' modifier='Sealed'>"
                                     "      <ECCustomAttributes>"
                                     "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
                                     "          <OnDeleteAction>Cascade</OnDeleteAction>"
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
                                     false, "relationship with Cascade Delete against subclass (joined table)"));

    AssertSchemaImport(testSchemas, "nocascadingdeleteonjoinedtables.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, ForeignKeyMapWhereLinkTableIsRequired)
    {
    SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                        "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                        "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                        "  <ECEntityClass typeName='Parent' >"
                        "    <ECProperty propertyName='Name' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECEntityClass typeName='Child' >"
                        "    <ECProperty propertyName='ParentId' typeName='long' />"
                        "    <ECProperty propertyName='ChildName' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECEntityClass typeName='Child2' >"
                        "    <ECProperty propertyName='ParentId' typeName='long' />"
                        "    <ECProperty propertyName='ChildName' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECRelationshipClass typeName='ParentHasChildren' strength='referencing' modifier='Sealed'>"
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'/>"
                        "    </ECCustomAttributes>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'Child' />"
                        "    </Target>"
                        "    <ECProperty propertyName='ForcingToLinkTable' typeName='string' />"
                        "  </ECRelationshipClass>"
                        "</ECSchema>", false, "Cannot apply ForeignKeyRelationshipMap when a link table is required.");

    AssertSchemaImport(testItem, "ForeignKeyMapWhereLinkTableIsRequired.ecdb");
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, ForeignKeyRelationshipMap_Misc)
    {
    Utf8CP ecdbName = "ForeignKeyRelationshipMap.ecdb";
    Utf8CP childTableName = "ts_Child";


    {
    SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                        "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                        "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                        "  <ECEntityClass typeName='Parent' >"
                        "    <ECProperty propertyName='Name' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECEntityClass typeName='Child' >"
                        "    <ECProperty propertyName='ParentId' typeName='long' />"
                        "    <ECProperty propertyName='ChildName' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECRelationshipClass typeName='ParentHasChildren' strength='referencing' modifier='Sealed'>"
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
                        "            <ForeignKeyColumn>ParentId</ForeignKeyColumn>"
                        "        </ForeignKeyRelationshipMap>"
                        "    </ECCustomAttributes>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'Child' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>", false, "ForeignKeyColumn specifies a value already used by a property");
    AssertSchemaImport(testItem, ecdbName);
    }

    {
    SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                        "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                        "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                        "  <ECEntityClass typeName='Parent' >"
                        "    <ECProperty propertyName='Name' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECEntityClass typeName='Child' >"
                        "    <ECProperty propertyName='ParentId' typeName='long' />"
                        "    <ECProperty propertyName='ChildName' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECRelationshipClass typeName='ParentHasChildren' strength='referencing' modifier='Sealed'>"
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
                        "            <ForeignKeyColumn>MyOwnParentId</ForeignKeyColumn>"
                        "        </ForeignKeyRelationshipMap>"
                        "    </ECCustomAttributes>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'Child' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>", true, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, ecdbName);
    ASSERT_FALSE(asserted);

    ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
    ASSERT_TRUE(ecdb.ColumnExists(childTableName, "MyOwnParentId"));
    bvector<Utf8String> columns;
    ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
    ASSERT_EQ(4, columns.size()) << childTableName << " table should contain an extra foreign key column as the relationship map specifies a value for ForeignKeyColumn";

    auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
    auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
    ASSERT_TRUE(it == columns.end()) << childTableName << " table should contain an extra foreign key column as the relationship map specifies a value for ForeignKeyColumn";

    AssertForeignKey(true, ecdb, childTableName);
    }

    {
    SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                        "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                        "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                        "  <ECEntityClass typeName='Parent' >"
                        "    <ECProperty propertyName='Name' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECEntityClass typeName='Child' >"
                        "    <ECProperty propertyName='ParentId' typeName='long' />"
                        "    <ECProperty propertyName='ChildName' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECRelationshipClass typeName='ParentHasChildren' strength='referencing' modifier='Sealed'>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'Child' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>", true, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, ecdbName);
    ASSERT_FALSE(asserted);

    ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
    bvector<Utf8String> columns;
    ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
    ASSERT_EQ(4, columns.size()) << childTableName << " table should contain a default-name extra foreign key column as there is no relationship map CA";

    auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
    auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
    ASSERT_TRUE(it != columns.end()) << "ts_child table should contain a default-name extra foreign key column as there is no relationship map CA";

    //  ---------<
    //  In following we expected 'false' but with trigger replaced with FK. We create FK constraint automatically so now it set to 'true'
    AssertForeignKey(true, ecdb, childTableName);
    //  --------->
    }

    {
    SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                        "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                        "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                        "  <ECEntityClass typeName='Parent' >"
                        "    <ECProperty propertyName='Name' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECEntityClass typeName='Child' >"
                        "    <ECProperty propertyName='ParentId' typeName='long' />"
                        "    <ECProperty propertyName='ChildName' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECRelationshipClass typeName='ParentHasChildren' strength='referencing' modifier='Sealed'>"
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00'>"
                        "        </ForeignKeyRelationshipMap>"
                        "    </ECCustomAttributes>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'Child' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>", true, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, ecdbName);
    ASSERT_FALSE(asserted);

    ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
    bvector<Utf8String> columns;
    ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
    ASSERT_EQ(4, columns.size()) << childTableName << " table should contain a default-name extra foreign key column as there is the relationship map CA doesn't specify a value for ForeignKeyColumn";

    auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
    auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
    ASSERT_TRUE(it != columns.end()) << childTableName << " table should contain a default-name extra foreign key column as there is the relationship map CA doesn't specify a value for ForeignKeyColumn";

    AssertForeignKey(true, ecdb, childTableName);
    }

    {
    SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                        "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                        "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                        "  <ECEntityClass typeName='Parent' >"
                        "    <ECProperty propertyName='Name' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECEntityClass typeName='Child' >"
                        "    <ECProperty propertyName='ParentId' typeName='long' />"
                        "    <ECProperty propertyName='ChildName' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECRelationshipClass typeName='ParentHasChildren' strength='referencing' modifier='Sealed'>"
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00' />"
                        "    </ECCustomAttributes>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'Child' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>", true, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, ecdbName);
    ASSERT_FALSE(asserted);

    ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
    bvector<Utf8String> columns;
    ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
    ASSERT_EQ(4, columns.size()) << childTableName << " table should contain a default-name extra foreign key column as there is the relationship map CA doesn't specify a value for ForeignKeyColumn";

    auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
    auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
    ASSERT_TRUE(it != columns.end()) << childTableName << " table should contain a default-name extra foreign key column as there is the relationship map CA doesn't specify a value for ForeignKeyColumn";

    AssertForeignKey(true, ecdb, childTableName, "ForeignECInstanceId_ts_ParentHasChildren");
    }

    {
    SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                        "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                        "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                        "  <ECEntityClass typeName='Parent' >"
                        "    <ECProperty propertyName='Name' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECEntityClass typeName='Child' >"
                        "    <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.02.00'>"
                        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                        "        </ClassMap>"
                        "    </ECCustomAttributes>"
                        "    <ECProperty propertyName='ParentId' typeName='long' />"
                        "    <ECProperty propertyName='ChildName' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECEntityClass typeName='Child2' >"
                        "    <BaseClass>Child</BaseClass>"
                        "    <ECProperty propertyName='Child2Name' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECRelationshipClass typeName='ParentHasChildren' strength='referencing' modifier='Sealed'>"
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.02.00' />"
                        "    </ECCustomAttributes>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'Child' />"
                        "      <Class class = 'Child2' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>", true, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, ecdbName);
    ASSERT_FALSE(asserted);

    ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
    bvector<Utf8String> columns;
    ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
    ASSERT_EQ(6, columns.size()) << childTableName << " table should contain a default-name extra foreign key column as there is the relationship map CA doesn't specify a value for ForeignKeyColumn";

    auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
    auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
    ASSERT_TRUE(it != columns.end()) << childTableName << " table should contain a default-name extra foreign key column as there is the relationship map CA doesn't specify a value for ForeignKeyColumn";

    AssertForeignKey(true, ecdb, childTableName, "ForeignECInstanceId_ts_ParentHasChildren");
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, RelationshipWithAbstractConstraintClassAndNoSubclasses)
    {
    auto getGeometrySourceHasGeometryRowCount = [] (ECDbCR ecdb)
        {
        ECSqlStatement selectStmt;
        if (ECSqlStatus::Success != selectStmt.Prepare(ecdb, "SELECT count(*) FROM ts.GeometrySourceHasGeometry"))
            return -1;

        if (BE_SQLITE_ROW != selectStmt.Step())
            return -1;

        return selectStmt.GetValueInt(0);
        };

    {
    SchemaItem testItem(
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "  <ECEntityClass typeName='Element' modifier='Abstract'>"
        "    <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "        </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "    </ECCustomAttributes>"
        "    <ECProperty propertyName='Code' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='GeometrySource' modifier='Abstract' />"
        "  <ECEntityClass typeName='ElementGeometry'>"
        "    <ECProperty propertyName='Geom' typeName='binary' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='ExtendedElement'>"
        "    <BaseClass>Element</BaseClass>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='GeometrySourceHasGeometry' strength='embedding' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class='GeometrySource' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='ElementGeometry' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>", false, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipWithAbstractBaseClass.ecdb");
    ASSERT_FALSE(asserted);
    }

    {
    SchemaItem testItem(
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "  <ECEntityClass typeName='Element' modifier='Abstract'>"
        "    <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "        </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "    </ECCustomAttributes>"
        "    <ECProperty propertyName='Code' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='GeometrySource' modifier='Abstract' />"
        "  <ECEntityClass typeName='GeometrySource3d'>"
        "    <BaseClass>GeometrySource</BaseClass>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='ElementGeometry'>"
        "    <ECProperty propertyName='Geom' typeName='binary' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='ExtendedElement'>"
        "    <BaseClass>Element</BaseClass>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='GeometrySourceHasGeometry' strength='embedding' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='False'>"
        "      <Class class='GeometrySource' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='ElementGeometry' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>", false, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipWithAbstractBaseClass.ecdb");
    ASSERT_FALSE(asserted);
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, RelationshipWithAbstractConstraintClass)
    {
    auto getGeometrySourceHasGeometryRowCount = [] (ECDbCR ecdb)
        {
        ECSqlStatement selectStmt;
        if (ECSqlStatus::Success != selectStmt.Prepare(ecdb, "SELECT count(*) FROM ts.GeometrySourceHasGeometry"))
            return -1;

        if (BE_SQLITE_ROW != selectStmt.Step())
            return -1;

        return selectStmt.GetValueInt(0);
        };

    {
    //Abstract base class has subclasses in different tables
    SchemaItem testItem(
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "  <ECEntityClass typeName='Element' modifier='Abstract'>"
        "    <ECCustomAttributes>"
        "        <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "        </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "    </ECCustomAttributes>"
        "    <ECProperty propertyName='Code' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='GeometrySource' modifier='Abstract' />"
        "  <ECEntityClass typeName='ElementGeometry'>"
        "    <ECProperty propertyName='Geom' typeName='binary' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='GeometricElement'>"
        "    <BaseClass>Element</BaseClass>"
        "    <BaseClass>GeometrySource</BaseClass>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='LooseGeometry'>"
        "    <BaseClass>GeometrySource</BaseClass>"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='GeometrySourceHasGeometry' strength='embedding' modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class='GeometrySource' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='ElementGeometry' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>", false, "Multiple Tables on either end of the relationship are not supported");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipWithAbstractBaseClass.ecdb");
    ASSERT_FALSE(asserted);
    }

    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem("GeometrySource is abstract and has concrete subclasses pointing to a single table",
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                     "  <ECEntityClass typeName='Element' modifier='Abstract' >"
                                     "    <ECProperty propertyName='Code' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='GeometrySource' modifier='Abstract'>"
                                     "    <ECCustomAttributes>"
                                     "        <ClassMap xmlns='ECDbMap.02.00'>"
                                     "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                     "        </ClassMap>"
                                     "            <ShareColumns xmlns='ECDbMap.02.00'>"
                                     "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                                     "            </ShareColumns>"
                                     "    </ECCustomAttributes>"
                                     "    <BaseClass>Element</BaseClass>"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='ElementGeometry'>"
                                     "    <ECProperty propertyName='Geom' typeName='binary' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='ExtendedElement'>"
                                     "    <BaseClass>GeometrySource</BaseClass>"
                                     "    <ECProperty propertyName='Name' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='GeometricElement'>"
                                     "    <BaseClass>ExtendedElement</BaseClass>"
                                     "  </ECEntityClass>"
                                     "  <ECRelationshipClass typeName='GeometrySourceHasGeometry' strength='embedding' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class='GeometrySource' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,N)' polymorphic='True'>"
                                     "      <Class class='ElementGeometry' />"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "</ECSchema>"));

    testSchemas.push_back(SchemaItem("GeometrySource is abstract and subClass of Element and Element has JoinedTable CA",
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                     "  <ECEntityClass typeName='Element' modifier='Abstract'>"
                                     "    <ECCustomAttributes>"
                                     "        <ClassMap xmlns='ECDbMap.02.00'>"
                                     "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                     "        </ClassMap>"
                                     "            <ShareColumns xmlns='ECDbMap.02.00'>"
                                     "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                                     "            </ShareColumns>"
                                     "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                     "    </ECCustomAttributes>"
                                     "    <ECProperty propertyName='Code' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='GeometrySource' modifier='Abstract'>"
                                     "   <BaseClass>Element</BaseClass>"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='ElementGeometry'>"
                                     "    <ECProperty propertyName='Geom' typeName='binary' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='ExtendedElement'>"
                                     "    <BaseClass>GeometrySource</BaseClass>"
                                     "    <ECProperty propertyName='Name' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='GeometricElement'>"
                                     "    <BaseClass>ExtendedElement</BaseClass>"
                                     "  </ECEntityClass>"
                                     "  <ECRelationshipClass typeName='GeometrySourceHasGeometry' strength='embedding' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class='GeometrySource' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,N)' polymorphic='True'>"
                                     "      <Class class='ElementGeometry' />"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "</ECSchema>"));

    for (SchemaItem const& testSchema : testSchemas)
        {
        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testSchema, "RelationshipWithAbstractBaseClass.ecdb");
        ASSERT_FALSE(asserted);

        ecdb.Schemas().CreateECClassViewsInDb();

        ECInstanceKey elem1Key, elem2Key, geomElem1Key, geomElem2Key, geom1Key, geom2Key;
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.ExtendedElement (Code,Name) VALUES('0001','NonGeom1')"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(elem1Key));
        stmt.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.ExtendedElement (Code,Name) VALUES('0002','NonGeom2')"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(elem2Key));
        stmt.Finalize();

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.GeometricElement (Code,Name) VALUES('0003','Geom1')"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomElem1Key));
        stmt.Finalize();

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.GeometricElement (Code,Name) VALUES('0004','Geom2')"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomElem2Key));
        stmt.Finalize();

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.ElementGeometry (Geom) VALUES('0x13124')"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geom1Key));
        stmt.Finalize();

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.ElementGeometry (Geom) VALUES('0x42343')"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geom2Key));
        stmt.Finalize();
        ecdb.SaveChanges();

        //now do actual tests with relationship
        ASSERT_EQ(0, getGeometrySourceHasGeometryRowCount(ecdb)) << "Before inserting one relationship [Scenario: " << testSchema.m_name << "]";

        ECSqlStatement insertStmt;
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.Prepare(ecdb, "INSERT INTO ts.GeometrySourceHasGeometry(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(?,?,?,?)"));

        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(1, geomElem1Key.GetECInstanceId()));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(2, geomElem1Key.GetECClassId()));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(3, geom1Key.GetECInstanceId()));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(4, geom1Key.GetECClassId()));

        ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step()) << "Inserting GeometrySourceHasGeometry against GeometricElement is expected to succeed";
        insertStmt.Reset();
        insertStmt.ClearBindings();

        ASSERT_EQ(1, getGeometrySourceHasGeometryRowCount(ecdb)) << "After inserting one relationship [Scenario: " << testSchema.m_name << "]";

        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(1, elem1Key.GetECInstanceId()));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(2, elem1Key.GetECClassId()));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(3, geom2Key.GetECInstanceId()));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(4, geom2Key.GetECClassId()));
        ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step()) << "Inserting GeometrySourceHasGeometry against ExtendedElement is also expected to succeed";
        insertStmt.Reset();
        insertStmt.ClearBindings();
        }
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, RelationshipWithAbstractClassAsConstraintOnChildEnd)
    {
    SchemaItem testItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                        "  <ECEntityClass typeName='Solid'>"
                        "    <ECProperty propertyName='Name' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECEntityClass typeName='Face' modifier='Abstract'>"
                        "    <ECProperty propertyName='FaceName' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECRelationshipClass typeName='SolidHasFaces' strength='embedding' modifier='Sealed'>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class='Solid' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class='Face' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>", true, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipWithAbstractClassAsConstraintOnChildEnd.ecdb");
    ASSERT_FALSE(asserted);

    ECClassId faceClassId = ecdb.Schemas().GetECClassId("TestSchema", "Face");
    ASSERT_TRUE(faceClassId.IsValid());
    ECInstanceKey solidKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Solid(Name) VALUES('MySolid')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(solidKey));
    }

    ecdb.SaveChanges();

    {
    ECSqlStatement selectStmt;
    ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(ecdb, "SELECT count(*) FROM ts.SolidHasFaces"));
    ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
    ASSERT_EQ(0, selectStmt.GetValueInt(0)) << "SELECT count(*) FROM ts.SolidHasFaces";
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "INSERT INTO ts.SolidHasFaces(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(?,?,4444,?)"));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                  06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, RelationshipWithNotMappedClassAsConstraint)
    {
            {
            SchemaItem testItem(
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "  <ECEntityClass typeName='Element' modifier='Sealed'>"
                "    <ECCustomAttributes>"
                "        <ClassMap xmlns='ECDbMap.02.00'>"
                "            <MapStrategy>NotMapped</MapStrategy>"
                "        </ClassMap>"
                "    </ECCustomAttributes>"
                "    <ECProperty propertyName='Code' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='ElementGeometry' modifier='Sealed'>"
                "    <ECCustomAttributes>"
                "        <ClassMap xmlns='ECDbMap.02.00'>"
                "            <MapStrategy>NotMapped</MapStrategy>"
                "        </ClassMap>"
                "    </ECCustomAttributes>"
                "    <ECProperty propertyName='Geom' typeName='binary' />"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='ElementHasGeometry' strength='embedding' modifier='Sealed'>"
                "    <Source cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='Element' />"
                "    </Source>"
                "    <Target cardinality='(0,N)' polymorphic='True'>"
                "      <Class class='ElementGeometry' />"
                "    </Target>"
                "    <ECProperty propertyName='RelProp' typeName='string' />"
                "  </ECRelationshipClass>"
                "</ECSchema>", true, "1:N Relationship having NotMapped constraint class on both sides of relationship are not supported");
                bool asserted = false;
                ECDb ecdb;
                AssertSchemaImport(ecdb, asserted, testItem, "relationshipwithnotmappedclassacsconstraint.ecdb");
                ASSERT_FALSE(asserted);

                ECRelationshipClassCP relClass = ecdb.Schemas().GetECSchema("TestSchema")->GetClassCP("ElementHasGeometry")->GetRelationshipClassCP();
                MapStrategyInfo mapStrategy;
                ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, relClass->GetId()));
                ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);
            }

            {
            SchemaItem testItem(
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "  <ECEntityClass typeName='Element' modifier='None'>"
                "    <ECCustomAttributes>"
                "        <ClassMap xmlns='ECDbMap.02.00'>"
                "            <MapStrategy>NotMapped</MapStrategy>"
                "        </ClassMap>"
                "    </ECCustomAttributes>"
                "    <ECProperty propertyName='Code' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='ElementGeometry'>"
                "    <ECCustomAttributes>"
                "        <ClassMap xmlns='ECDbMap.02.00'>"
                "            <MapStrategy>NotMapped</MapStrategy>"
                "        </ClassMap>"
                "    </ECCustomAttributes>"
                "    <ECProperty propertyName='Geom' typeName='binary' />"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='ElementHasGeometry' strength='referencing' modifier='Sealed'>"
                "    <Source cardinality='(0,N)' polymorphic='True'>"
                "      <Class class='Element' />"
                "    </Source>"
                "    <Target cardinality='(0,N)' polymorphic='True'>"
                "      <Class class='ElementGeometry' />"
                "    </Target>"
                "    <ECProperty propertyName='RelProp' typeName='string' />"
                "  </ECRelationshipClass>"
                "</ECSchema>", true, "N:N Relationship having not mapped constraint class on both sides of relationship are not supported");

                bool asserted = false;
                ECDb ecdb;
                AssertSchemaImport(ecdb, asserted, testItem, "relationshipwithnotmappedclassacsconstraint.ecdb");
                ASSERT_FALSE(asserted);

                ECRelationshipClassCP relClass = ecdb.Schemas().GetECSchema("TestSchema")->GetClassCP("ElementHasGeometry")->GetRelationshipClassCP();
                MapStrategyInfo mapStrategy;
                ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, relClass->GetId()));
                ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);
            }

            {
            SchemaItem testItem(
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "  <ECEntityClass typeName='Element' modifier='None'>"
                "    <ECProperty propertyName='Code' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='PhysicalElement' modifier='None'>"
                "    <ECCustomAttributes>"
                "        <ClassMap xmlns='ECDbMap.02.00'>"
                "            <MapStrategy>NotMapped</MapStrategy>"
                "        </ClassMap>"
                "    </ECCustomAttributes>"
                "    <BaseClass>Element</BaseClass>"
                "    <ECProperty propertyName='NameSpace' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='ElementGeometry' modifier='None'>"
                "    <ECProperty propertyName='Geom' typeName='binary' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='ElementGeometry3D'>"
                "    <ECCustomAttributes>"
                "        <ClassMap xmlns='ECDbMap.02.00'>"
                "            <MapStrategy>NotMapped</MapStrategy>"
                "        </ClassMap>"
                "    </ECCustomAttributes>"
                "    <BaseClass>ElementGeometry</BaseClass>"
                "    <ECProperty propertyName='Category3D' typeName='binary' />"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='ElementHasGeometry' strength='referencing' modifier='Sealed'>"
                "    <Source cardinality='(0,N)' polymorphic='True'>"
                "      <Class class='Element' />"
                "    </Source>"
                "    <Target cardinality='(0,N)' polymorphic='True'>"
                "      <Class class='ElementGeometry' />"
                "    </Target>"
                "    <ECProperty propertyName='RelProp' typeName='string' />"
                "  </ECRelationshipClass>"
                "</ECSchema>", true, "N:N Relationship having at least one NotMapped constraint class on both sides of relationship are not supported");

                bool asserted = false;
                ECDb ecdb;
                AssertSchemaImport(ecdb, asserted, testItem, "relationshipwithnotmappedclassacsconstraint.ecdb");
                ASSERT_FALSE(asserted);

                ECRelationshipClassCP relClass = ecdb.Schemas().GetECSchema("TestSchema")->GetClassCP("ElementHasGeometry")->GetRelationshipClassCP();
                MapStrategyInfo mapStrategy;
                ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, relClass->GetId()));
                ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);
            }

            {
            SchemaItem testItem(
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "  <ECEntityClass typeName='Element' modifier='None'>"
                "    <ECProperty propertyName='Code' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='PhysicalElement' modifier='None'>"
                "    <BaseClass>Element</BaseClass>"
                "    <ECProperty propertyName='NameSpace' typeName='string' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='ElementGeometry' modifier='None'>"
                "    <ECProperty propertyName='Geom' typeName='binary' />"
                "  </ECEntityClass>"
                "  <ECEntityClass typeName='ElementGeometry3D'>"
                "    <ECCustomAttributes>"
                "        <ClassMap xmlns='ECDbMap.02.00'>"
                "            <MapStrategy>NotMapped</MapStrategy>"
                "        </ClassMap>"
                "    </ECCustomAttributes>"
                "    <BaseClass>ElementGeometry</BaseClass>"
                "    <ECProperty propertyName='Category3D' typeName='binary' />"
                "  </ECEntityClass>"
                "  <ECRelationshipClass typeName='ElementHasGeometry' strength='referencing' modifier='Sealed'>"
                "    <Source cardinality='(0,N)' polymorphic='True'>"
                "      <Class class='Element' />"
                "    </Source>"
                "    <Target cardinality='(0,N)' polymorphic='True'>"
                "      <Class class='ElementGeometry' />"
                "    </Target>"
                "    <ECProperty propertyName='RelProp' typeName='string' />"
                "  </ECRelationshipClass>"
                "</ECSchema>", true, "N:N Relationships having at least one NotMapped constraint class on one sides of relationship are not supported");

                bool asserted = false;
                ECDb ecdb;
                AssertSchemaImport(ecdb, asserted, testItem, "relationshipwithnotmappedclassacsconstraint.ecdb");
                ASSERT_FALSE(asserted);

                ECRelationshipClassCP relClass = ecdb.Schemas().GetECSchema("TestSchema")->GetClassCP("ElementHasGeometry")->GetRelationshipClassCP();
                MapStrategyInfo mapStrategy;
                ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, relClass->GetId()));
                ASSERT_EQ(MapStrategyInfo::Strategy::NotMapped, mapStrategy.m_strategy);
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
void AssertAndExecuteECSql(ECDbCR ecdb, Utf8CP ecsql, ECSqlStatus prepareStatus = ECSqlStatus::Success, DbResult stepStatus = BE_SQLITE_DONE)
    {
    ECSqlStatement stmt;
    ASSERT_EQ(stmt.Prepare(ecdb, ecsql), prepareStatus) << "Prepare failed for: " << ecsql;
    if (stmt.IsPrepared())
        {
        ASSERT_EQ(stmt.Step(), stepStatus) << "Step failed for: " << ecsql;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, AddDerivedClassOfConstraintOnNsideOf1NRelationship)
    {
    SchemaItem opSchema(
        "<?xml version = '1.0' encoding = 'utf-8'?>"
        "<ECSchema schemaName='OpenPlant' nameSpacePrefix='op' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "  <ECEntityClass typeName='ITEM' >"
        "    <ECProperty propertyName='op_ITEM_prop' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='UNIT' >"
        "    <ECProperty propertyName='op_UNIT_prop' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='UNIT_HAS_ITEM' strength='referencing' strengthDirection='forward'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class='UNIT' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='ITEM' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    bool asserted = false;
    ECDb ecdb;
    AssertSchemaImport(ecdb, asserted, opSchema, "addderivedclassonNsideof1Nrelationship.ecdb");
    ASSERT_FALSE(asserted);

    ECClassCP item = ecdb.Schemas().GetECClass("OpenPlant", "ITEM");
    ECClassCP unit = ecdb.Schemas().GetECClass("OpenPlant", "UNIT");

    Savepoint sp(ecdb, "CRUD Operations");
    //Insert Statements
    {
    //relationship between UNIT and ITEM
    AssertAndExecuteECSql(ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')");
    AssertAndExecuteECSql(ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')");

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401, 201, %llu, 101, %llu)", unit->GetId().GetValue(), item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    }

    //Select statements
    {
    AssertAndExecuteECSql(ecdb, "SELECT * FROM op.UNIT_HAS_ITEM", ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    }
    sp.Cancel();

    ecdb.SaveChanges();
    SchemaItem op3dSchema(
        "<?xml version = '1.0' encoding = 'utf-8'?>"
        "<ECSchema schemaName='OpenPlant_3D' nameSpacePrefix='op3d' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='OpenPlant' version='01.00' prefix='op' />"
        "  <ECEntityClass typeName='ITEM_3D' >"
        "    <BaseClass>op:ITEM</BaseClass>"
        "    <ECProperty propertyName='op3d_ITEM_prop' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    asserted = false;
    AssertSchemaImport(asserted, ecdb, op3dSchema);
    ASSERT_FALSE(asserted);

    item = ecdb.Schemas().GetECClass("OpenPlant", "ITEM");
    unit = ecdb.Schemas().GetECClass("OpenPlant", "UNIT");
    ECClassCP item_3D = ecdb.Schemas().GetECClass("OpenPlant_3D", "ITEM_3D");

    //Insert Statements
    {
    Utf8String ecsql;
    //relationship between UNIT and ITEM
    AssertAndExecuteECSql(ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')");
    AssertAndExecuteECSql(ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')");

    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401, 201, %llu, 101, %llu)", unit->GetId().GetValue(), item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());

    //relationship between UNIT and ITEM_3D(new derived Class)
    AssertAndExecuteECSql(ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(202, 'unitString2')");
    AssertAndExecuteECSql(ecdb, "INSERT INTO op3d.ITEM_3D(ECInstanceId, op_ITEM_prop, op3d_ITEM_prop) VALUES(301, 'itemString1', 'item3dString1')");

    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(402, 202, %llu, 301, %llu)", unit->GetId().GetValue(), item_3D->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    }

    //Select statements
    {
    Utf8String ecsql;
    AssertAndExecuteECSql(ecdb, "SELECT * FROM op.UNIT_HAS_ITEM", ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);

    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);

    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item_3D->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());

    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE ECInstanceId = 402 AND TargetECClassId = %llu", item_3D->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    //verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item_3D->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, AddDerivedClassOfConstraintOn1sideOf1NRelationship)
    {
    SchemaItem opSchema(
        "<?xml version = '1.0' encoding = 'utf-8'?>"
        "<ECSchema schemaName='OpenPlant' nameSpacePrefix='op' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "  <ECEntityClass typeName='ITEM' >"
        "    <ECProperty propertyName='op_ITEM_prop' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='UNIT' >"
        "    <ECProperty propertyName='op_UNIT_prop' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='UNIT_HAS_ITEM' strength='referencing' strengthDirection='forward'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class='UNIT' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='ITEM' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    bool asserted = false;
    ECDb ecdb;
    AssertSchemaImport(ecdb, asserted, opSchema, "addderivedclasson1sideof1Nrelationship.ecdb");
    ASSERT_FALSE(asserted);

    ECClassCP unit = ecdb.Schemas().GetECClass("OpenPlant", "UNIT");
    ECClassCP item = ecdb.Schemas().GetECClass("OpenPlant", "ITEM");

    Savepoint sp(ecdb, "CRUD operations");
    //Insert Statements
    {
    //relationship between UNIT and ITEM
    AssertAndExecuteECSql(ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')");
    AssertAndExecuteECSql(ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')");

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401, 201, %llu, 101, %llu)", unit->GetId().GetValue(), item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    }

    //Select Statements
    {
    Utf8String ecsql;
    AssertAndExecuteECSql(ecdb, "SELECT * FROM op.UNIT_HAS_ITEM", ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);

    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    }
    sp.Cancel();

    ecdb.SaveChanges();
    SchemaItem op3dSchema(
        "<?xml version = '1.0' encoding = 'utf-8'?>"
        "<ECSchema schemaName='OpenPlant_3D' nameSpacePrefix='op3d' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='OpenPlant' version='01.00' prefix='op' />"
        "  <ECEntityClass typeName='UNIT_3D' >"
        "    <BaseClass>op:UNIT</BaseClass>"
        "    <ECProperty propertyName='op3d_UNIT_prop' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    asserted = false;
    AssertSchemaImport(asserted, ecdb, op3dSchema);
    ASSERT_FALSE(asserted);

    item = ecdb.Schemas().GetECClass("OpenPlant", "ITEM");
    unit = ecdb.Schemas().GetECClass("OpenPlant", "UNIT");
    ECClassCP unit_3D = ecdb.Schemas().GetECClass("OpenPlant_3D", "UNIT_3D");

    //Insert Statements
    {
    Utf8String ecsql;
    //relationship between UNIT and ITEM
    AssertAndExecuteECSql(ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')");
    AssertAndExecuteECSql(ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')");

    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401, 201, %llu, 101, %llu)", unit->GetId().GetValue(), item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());

    //relationship between UNIT_3D(new derived Class) and ITEM
    AssertAndExecuteECSql(ecdb, "INSERT INTO op3d.UNIT_3D(ECInstanceId, op_UNIT_prop, op3d_UNIT_prop) VALUES(301, 'unitString2', 'unit3dString2')");
    AssertAndExecuteECSql(ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(102, 'itemString2')");

    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(402, 301, %llu, 102, %llu)", unit_3D->GetId().GetValue(), item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    }

    //Select Statements
    {
    Utf8String ecsql;
    AssertAndExecuteECSql(ecdb, "SELECT * FROM op.UNIT_HAS_ITEM", ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);

    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);

    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit_3D->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());

    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE ECInstanceId = 402 AND SourceECClassId=%llu", unit_3D->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit_3D->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, AddDerivedClassOfConstraintsForNNRelationship)
    {
    SchemaItem opSchema(
        "<?xml version = '1.0' encoding = 'utf-8'?>"
        "<ECSchema schemaName='OpenPlant' nameSpacePrefix='op' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "  <ECEntityClass typeName='ITEM' >"
        "    <ECProperty propertyName='op_ITEM_prop' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='UNIT' >"
        "    <ECProperty propertyName='op_UNIT_prop' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='UNIT_HAS_ITEM' strength='referencing' strengthDirection='forward'>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='UNIT' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='ITEM' />"
        "    </Target>"
        "    <ECProperty propertyName='relProp' typeName='string' />"
        "  </ECRelationshipClass>"
        "</ECSchema>");

    bool asserted = false;
    ECDb ecdb;
    AssertSchemaImport(ecdb, asserted, opSchema, "addderivedclassofconstraintsforNNrelationship.ecdb");
    ASSERT_FALSE(asserted);

    ECClassCP item = ecdb.Schemas().GetECClass("OpenPlant", "ITEM");
    ECClassCP unit = ecdb.Schemas().GetECClass("OpenPlant", "UNIT");

    Savepoint sp(ecdb, "CRUD Operations");
    //Insert Statements
    {
    //relationship between UNIT and ITEM
    AssertAndExecuteECSql(ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')");
    AssertAndExecuteECSql(ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')");

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, relProp) VALUES(401, 201, %llu, 101, %llu, 'relPropString1')", unit->GetId().GetValue(), item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    }

    //Select statements
    {
    AssertAndExecuteECSql(ecdb, "SELECT * FROM op.UNIT_HAS_ITEM", ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu AND TargetECClassId = %llu", unit->GetId().GetValue(), item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);
    }

    //update Statement
    {
    AssertAndExecuteECSql(ecdb, "UPDATE op.UNIT_HAS_ITEM SET relProp='relPropUpdatedString1' WHERE ECInstanceId=401");
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %lld AND TargetECClassId = %llu", unit->GetId().GetValue(), item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    //verify Deltion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu AND TargetECClassId = %llu", unit->GetId().GetValue(), item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    }
    sp.Cancel();

    ecdb.SaveChanges();
    SchemaItem op3dSchema(
        "<?xml version = '1.0' encoding = 'utf-8'?>"
        "<ECSchema schemaName='OpenPlant_3D' nameSpacePrefix='op3d' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='OpenPlant' version='01.00' prefix='op' />"
        "  <ECEntityClass typeName='ITEM_3D' >"
        "    <BaseClass>op:ITEM</BaseClass>"
        "    <ECProperty propertyName='op3d_ITEM_prop' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='UNIT_3D' >"
        "    <BaseClass>op:UNIT</BaseClass>"
        "    <ECProperty propertyName='op3d_UNIT_prop' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>");

    asserted = false;
    AssertSchemaImport(asserted, ecdb, op3dSchema);
    ASSERT_FALSE(asserted);

    item = ecdb.Schemas().GetECClass("OpenPlant", "ITEM");
    unit = ecdb.Schemas().GetECClass("OpenPlant", "UNIT");
    ECClassCP item_3D = ecdb.Schemas().GetECClass("OpenPlant_3D", "ITEM_3D");
    ECClassCP unit_3D = ecdb.Schemas().GetECClass("OpenPlant_3D", "UNIT_3D");

    //Insert Statements
    {
    //relationship between UNIT and ITEM
    AssertAndExecuteECSql(ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')");
    AssertAndExecuteECSql(ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')");

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, relProp) VALUES(501, 201, %llu, 101, %llu, 'relPropString1')", unit->GetId().GetValue(), item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());

    //relationship between UNIT_3D and ITEM_3D newly added derived classes
    AssertAndExecuteECSql(ecdb, "INSERT INTO op3d.UNIT_3D(ECInstanceId, op_UNIT_prop, op3d_UNIT_prop) VALUES(401, 'unitString2', 'unit3dString2')");
    AssertAndExecuteECSql(ecdb, "INSERT INTO op3d.ITEM_3D(ECInstanceId, op_ITEM_prop, op3d_ITEM_prop) VALUES(301, 'itemString2', 'item3dString2')");

    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, relProp) VALUES(502, 401, %llu, 301, %llu, 'relPropString2')", unit_3D->GetId().GetValue(), item_3D->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    }

    //Select statements
    {
    AssertAndExecuteECSql(ecdb, "SELECT * FROM op.UNIT_HAS_ITEM", ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu AND TargetECClassId = %llu", unit->GetId().GetValue(), item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);

    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu AND TargetECClassId = %llu", unit_3D->GetId().GetValue(), item_3D->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str(), ECSqlStatus::Success, DbResult::BE_SQLITE_ROW);
    }

    //update Statement
    {
    AssertAndExecuteECSql(ecdb, "UPDATE op.UNIT_HAS_ITEM SET relProp='relPropUpdatedString1' WHERE ECInstanceId=501");

    //update relationship between newly added derived classes
    AssertAndExecuteECSql(ecdb, "UPDATE op.UNIT_HAS_ITEM SET relProp='relPropUpdatedString2' WHERE ECInstanceId=502");
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %lld AND TargetECClassId = %llu", unit->GetId().GetValue(), item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu AND TargetECClassId = %llu", unit->GetId().GetValue(), item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());

    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %lld AND TargetECClassId = %llu", unit_3D->GetId().GetValue(), item_3D->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu AND TargetECClassId = %llu", unit->GetId().GetValue(), item->GetId().GetValue());
    AssertAndExecuteECSql(ecdb, ecsql.c_str());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
struct ECDbHoldingRelationshipStrengthTestFixture : ECDbMappingTestFixture
    {
    protected:
        bool InstanceExists(Utf8CP classExp, ECInstanceKey const& key) const
            {
            Utf8String ecsql;
            ecsql.Sprintf("SELECT NULL FROM %s WHERE ECInstanceId=?", classExp);
            ECSqlStatement stmt;
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), ecsql.c_str())) << ecsql.c_str();
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetECInstanceId()));

            DbResult stat = stmt.Step();
            EXPECT_TRUE(stat == BE_SQLITE_ROW || stat == BE_SQLITE_DONE);
            return stat == BE_SQLITE_ROW;
            };

        bool RelationshipExists(Utf8CP relClassExp, ECInstanceKey const& sourceKey, ECInstanceKey const& targetKey) const
            {
            Utf8String ecsql;
            ecsql.Sprintf("SELECT NULL FROM %s WHERE SourceECInstanceId=? AND SourceECClassId=? AND TargetECInstanceId=? AND TargetECClassId=?", relClassExp);
            ECSqlStatement stmt;
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), ecsql.c_str())) << ecsql.c_str();
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(1, sourceKey.GetECInstanceId()));
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(2, sourceKey.GetECClassId()));
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(3, targetKey.GetECInstanceId()));
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(4, targetKey.GetECClassId()));

            DbResult stat = stmt.Step();
            EXPECT_TRUE(stat == BE_SQLITE_ROW || stat == BE_SQLITE_DONE);
            return stat == BE_SQLITE_ROW;
            };
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, OneToOneForward)
    {
    SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
              SchemaItem("1:N and holding",
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "  <ECEntityClass typeName='Geometry' >"
                         "    <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='GeometryPart' >"
                         "    <ECProperty propertyName='Stream' typeName='binary' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='GeometryHoldsParts' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                         "     <Source cardinality='(0,1)' polymorphic='True'>"
                         "         <Class class='Geometry' />"
                         "     </Source>"
                         "    <Target cardinality='(0,1)' polymorphic='True'>"
                         "        <Class class='GeometryPart' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Geometry(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Polygon", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey1));
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Solid", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey2));
    }

    ECInstanceKey partKey1;
    ECInstanceKey partKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey1));
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey2));
    }

    {
    //Create relationships:
    //Geom-Part
    //1-1
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryHoldsParts(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey1.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey1.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    }

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());
    delGeomStmt.Reset();
    delGeomStmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_FALSE(RelationshipExists("ts.GeometryHoldsParts", geomKey1, partKey1)) << "ECSQL DELETE deletes affected relationships";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, OneToOneBackward)
    {
    SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
              SchemaItem("1:N and holding",
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "  <ECEntityClass typeName='Geometry' >"
                         "    <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='GeometryPart' >"
                         "    <ECProperty propertyName='Stream' typeName='binary' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='PartHeldByGeometry' strength='holding' strengthDirection='Backward' modifier='Sealed'>"
                         "    <Source cardinality='(0,1)' polymorphic='True'>"
                         "        <Class class='GeometryPart' />"
                         "     </Source>"
                         "     <Target cardinality='(0,1)' polymorphic='True'>"
                         "         <Class class='Geometry' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Geometry(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Polygon", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey1));
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Solid", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey2));
    }

    ECInstanceKey partKey1;
    ECInstanceKey partKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey1));
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey2));
    }

    {
    //Create relationships:
    //Geom-Part
    //1-1
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.PartHeldByGeometry(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey1.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey1.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    }

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());
    delGeomStmt.Reset();
    delGeomStmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_FALSE(RelationshipExists("ts.PartHeldByGeometry", partKey1, geomKey1)) << "ECSQL DELETE deletes affected relationships";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, OneToManyForward)
    {
    SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
              SchemaItem("1:N and holding",
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "  <ECEntityClass typeName='Geometry' >"
                         "    <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='GeometryPart' >"
                         "    <ECProperty propertyName='Stream' typeName='binary' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='GeometryHoldsParts' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                         "     <Source cardinality='(0,N)' polymorphic='True'>"
                         "         <Class class='Geometry' />"
                         "     </Source>"
                         "    <Target cardinality='(0,1)' polymorphic='True'>"
                         "        <Class class='GeometryPart' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Geometry(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Polygon", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey1));
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Solid", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey2));
    }

    ECInstanceKey partKey1;
    ECInstanceKey partKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey1));
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey2));
    }

    {
    //Create relationships:
    //Geom-Part
    //1-1
    //2-1
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryHoldsParts(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey1.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey1.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey2.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey2.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey1.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());
    delGeomStmt.Reset();
    delGeomStmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_FALSE(RelationshipExists("ts.GeometryHoldsParts", geomKey1, partKey1)) << "ECSQL DELETE deletes affected relationships";
    ASSERT_TRUE(RelationshipExists("ts.GeometryHoldsParts", geomKey2, partKey1));

    //delete Geom2
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey2.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1)) << "Part 1 is not held anymore, but will only be deleted by Purge";
    ASSERT_FALSE(RelationshipExists("ts.GeometryHoldsParts", geomKey2, partKey1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, OneToManyBackward)
    {
    SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
              SchemaItem("1:N and holding",
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "  <ECEntityClass typeName='Geometry' >"
                         "    <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='GeometryPart' >"
                         "    <ECProperty propertyName='Stream' typeName='binary' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='PartIsHeldByGeometry' strength='holding' strengthDirection='Backward' modifier='Sealed'>"
                         "    <Source cardinality='(0,1)' polymorphic='True'>"
                         "        <Class class='GeometryPart' />"
                         "     </Source>"
                         "     <Target cardinality='(0,N)' polymorphic='True'>"
                         "         <Class class='Geometry' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Geometry(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Polygon", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey1));
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Solid", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey2));
    }

    ECInstanceKey partKey1;
    ECInstanceKey partKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey1));
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey2));
    }

    {
    //Create relationships:
    //Part-Geom
    //1-1
    //1-2
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.PartIsHeldByGeometry(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey1.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey1.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey1.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey2.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey2.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());
    delGeomStmt.Reset();
    delGeomStmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_FALSE(RelationshipExists("ts.PartIsHeldByGeometry", partKey1, geomKey1)) << "ECSQL DELETE deletes affected relationships";
    ASSERT_TRUE(RelationshipExists("ts.PartIsHeldByGeometry", partKey1, geomKey2));

    //delete Geom2
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey2.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1)) << "Part 1 is not held anymore, but will only be deleted by Purge";
    ASSERT_FALSE(RelationshipExists("ts.PartIsHeldByGeometry", partKey1, geomKey2));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, ManyToManyForward)
    {
    SetupECDb("ecdbrelationshipmappingrules_manytomanyandholding.ecdb",
              SchemaItem("N:N and holding",
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "  <ECEntityClass typeName='Geometry' >"
                         "    <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='GeometryPart' >"
                         "    <ECProperty propertyName='Stream' typeName='binary' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='GeometryHasParts' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                         "     <Source cardinality='(0,N)' polymorphic='True'>"
                         "         <Class class='Geometry' />"
                         "     </Source>"
                         "    <Target cardinality='(0,N)' polymorphic='True'>"
                         "        <Class class='GeometryPart' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Geometry(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Polygon", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey1));
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Solid", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey2));
    }

    ECInstanceKey partKey1;
    ECInstanceKey partKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey1));
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey2));
    }

    {
    //Create relationships:
         //Geom-Part
         //1-1
         //1-2
         //2-2
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryHasParts(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey1.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey1.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey1.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey2.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey2.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey2.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey2.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey2.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey2.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_FALSE(RelationshipExists("ts.GeometryHasParts", geomKey1, partKey1));
    ASSERT_FALSE(RelationshipExists("ts.GeometryHasParts", geomKey1, partKey2));
    ASSERT_TRUE(RelationshipExists("ts.GeometryHasParts", geomKey2, partKey2));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, ManyToManyBackward)
    {
    SetupECDb("ecdbrelationshipmappingrules_manytomanyandholding.ecdb",
              SchemaItem("N:N and holding",
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "  <ECEntityClass typeName='Geometry' >"
                         "    <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='GeometryPart' >"
                         "    <ECProperty propertyName='Stream' typeName='binary' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='PartsHeldByGeometry' strength='holding' strengthDirection='Backward' modifier='Sealed'>"
                         "    <Source cardinality='(0,N)' polymorphic='True'>"
                         "        <Class class='GeometryPart' />"
                         "     </Source>"
                         "     <Target cardinality='(0,N)' polymorphic='True'>"
                         "         <Class class='Geometry' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Geometry(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Polygon", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey1));
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Solid", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey2));
    }

    ECInstanceKey partKey1;
    ECInstanceKey partKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey1));
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey2));
    }

    {
    //Create relationships:
    //Geom-Part
        //1-1
        //1-2
        //2-2
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.PartsHeldByGeometry(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey1.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey1.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey2.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey2.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey1.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey1.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey2.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey2.GetECClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey2.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey2.GetECClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_FALSE(RelationshipExists("ts.PartsHeldByGeometry", partKey1, geomKey1));
    ASSERT_FALSE(RelationshipExists("ts.PartsHeldByGeometry", partKey2, geomKey1));
    ASSERT_TRUE(RelationshipExists("ts.PartsHeldByGeometry", partKey2, geomKey2));
    }

//=======================================================================================    
// @bsiclass                                   Muhammad Hassan                     05/15
//=======================================================================================    
struct RelationshipsAndSharedTablesTestFixture : ECDbMappingTestFixture
    {
    protected:
        static Utf8CP const SCHEMA_XML;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const RelationshipsAndSharedTablesTestFixture::SCHEMA_XML =
"<?xml version='1.0' encoding='utf-8'?>"
"<ECSchema schemaName='test' nameSpacePrefix='t' version='1.0' description='Schema covers all the cases in which base class is OwnTable(Polymorphic)' displayLabel='Table Per Hierarchy' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
    "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
    "<ECEntityClass typeName='Base'>"
        "<ECCustomAttributes>"
        "        <ClassMap xmlns='ECDbMap.02.00'>"
    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
    "</ClassMap>"
        "</ECCustomAttributes>"
        "<ECProperty propertyName='P0' typeName='string' />"
    "</ECEntityClass>"
    "<ECEntityClass typeName='ClassA' >"
        "<BaseClass>Base</BaseClass>"
        "<ECProperty propertyName='P1' typeName='string' />"
    "</ECEntityClass>"
    "<ECEntityClass typeName='ClassB' >"
        "<BaseClass>ClassA</BaseClass>"
            "<ECProperty propertyName='P2' typeName='string' />"
    "</ECEntityClass>"
    "<ECRelationshipClass typeName='BaseOwnsBase' strength='referencing' strengthDirection='forward' modifier='Abstract'>"
        "<ECCustomAttributes>"
    "        <ClassMap xmlns='ECDbMap.02.00'>"
    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
    "</ClassMap>"
        "</ECCustomAttributes>"
        "<Source cardinality='(0,N)' polymorphic='True'>"
            "<Class class='Base' />"
        "</Source>"
        "<Target cardinality='(0,N)' polymorphic='True'>"
            "<Class class='Base' />"
        "</Target>"
    "</ECRelationshipClass>"
    "<ECRelationshipClass typeName='BaseHasClassA' strength='referencing' strengthDirection='forward' modifier='Sealed'>"
        "<BaseClass>BaseOwnsBase</BaseClass>"
        "<Source cardinality='(0,1)' polymorphic='True'>"
            "<Class class='Base' />"
        "</Source>"
        "<Target cardinality='(0,1)' polymorphic='True'>"
            "<Class class='ClassA' />"
        "</Target>"
    "</ECRelationshipClass>"
    "<ECRelationshipClass typeName='BaseHasClassB' strength='referencing' strengthDirection='forward' modifier='Sealed'>"
        "<BaseClass>BaseOwnsBase</BaseClass>"
        "<Source cardinality='(0,1)' polymorphic='True'>"
            "<Class class='Base' />"
        "</Source>"
        "<Target cardinality='(0,1)' polymorphic='True'>"
            "<Class class='ClassB' />"
        "</Target>"
    "</ECRelationshipClass>"
"</ECSchema>";

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipsAndSharedTablesTestFixture, UniqueIndexesSupportFor1to1Relationship)
    {
    SchemaItem testItem(SCHEMA_XML, true);
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipsAndSharedTables.ecdb");
    ASSERT_FALSE(asserted);

    BeSQLite::Statement stmt;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT Id from ec_Class where ec_Class.Name = 'BaseHasClassA'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, stmt.Step());
    ECClassId classId = stmt.GetValueId<ECClassId>(0);
    stmt.Finalize();

    //verify that entry in the ec_Index table exists for relationship table BaseHasClassA
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT Name, IsUnique from ec_Index where ClassId = ?"));
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.BindId(1, classId));
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        ASSERT_EQ(1, stmt.GetValueInt(1)) << "Index value for 1:1 Relationship is not Unique";
        Utf8String indexName = stmt.GetValueText(0);
        ASSERT_TRUE(indexName == "idx_ECRel_Source_Unique_t_BaseOwnsBase" || "idx_ECRel_Target_Unique_t_BaseOwnsBase");
        }
    stmt.Finalize();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT Id from ec_Class where ec_Class.Name = 'BaseHasClassB'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, stmt.Step());
    classId = stmt.GetValueId<ECClassId>(0);
    stmt.Finalize();

    //verify that entry in ec_Index table also exists for relationship table BaseHasClassB
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT Name, IsUnique from ec_Index where ClassId = ?"));
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.BindId(1, classId));
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        ASSERT_EQ(1, stmt.GetValueInt(1)) << "Index value for 1:1 Relationship is not Unique";
        Utf8String indexName = stmt.GetValueText(0);
        ASSERT_TRUE(indexName == "uix_unique_t_BaseHasClassB_Source" || "uix_unique_t_BaseHasClassB_Target");
        }
    stmt.Finalize();

    ecdb.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipsAndSharedTablesTestFixture, InstanceDeletionFromPolymorphicRelationships)
    {
    SchemaItem testItem(SCHEMA_XML, true);
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipsAndSharedTables.ecdb");
    ASSERT_FALSE(asserted);

    ASSERT_TRUE(ecdb.TableExists("t_BaseOwnsBase"));
    ASSERT_FALSE(ecdb.TableExists("t_BaseHasClassA"));
    ASSERT_FALSE(ecdb.TableExists("t_BaseHasClassB"));

    ECSchemaCP schema = ecdb.Schemas().GetECSchema("test", true);
    ASSERT_TRUE(schema != nullptr) << "Couldn't locate test schema";

    ECClassCP baseClass = schema->GetClassCP("Base");
    ASSERT_TRUE(baseClass != nullptr) << "Couldn't locate class Base from schema";
    ECClassCP classA = schema->GetClassCP("ClassA");
    ASSERT_TRUE(classA != nullptr) << "Couldn't locate classA from Schema";
    ECClassCP classB = schema->GetClassCP("ClassB");
    ASSERT_TRUE(classB != nullptr) << "Couldn't locate classB from Schema";

    //Insert Instances for class Base
    ECN::StandaloneECInstancePtr baseInstance1 = baseClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECN::StandaloneECInstancePtr baseInstance2 = baseClass->GetDefaultStandaloneEnabler()->CreateInstance();

    baseInstance1->SetValue("P0", ECValue("string1"));
    baseInstance2->SetValue("P0", ECValue("string2"));

    ECInstanceInserter inserter(ecdb, *baseClass);
    ASSERT_TRUE(inserter.IsValid());

    auto stat = inserter.Insert(*baseInstance1, true);
    ASSERT_TRUE(stat == SUCCESS);
    stat = inserter.Insert(*baseInstance2, true);
    ASSERT_TRUE(stat == SUCCESS);

    //Insert Instances for ClassA
    ECN::StandaloneECInstancePtr classAInstance1 = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECN::StandaloneECInstancePtr classAInstance2 = classA->GetDefaultStandaloneEnabler()->CreateInstance();

    classAInstance1->SetValue("P1", ECValue("string1"));
    classAInstance2->SetValue("P1", ECValue("string2"));

    ECInstanceInserter classAinserter(ecdb, *classA);
    ASSERT_TRUE(classAinserter.IsValid());

    stat = classAinserter.Insert(*classAInstance1, true);
    ASSERT_TRUE(stat == SUCCESS);
    stat = classAinserter.Insert(*classAInstance2, true);
    ASSERT_TRUE(stat == SUCCESS);

    //Insert Instances for ClassB
    ECN::StandaloneECInstancePtr classBInstance1 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECN::StandaloneECInstancePtr classBInstance2 = classB->GetDefaultStandaloneEnabler()->CreateInstance();

    classBInstance1->SetValue("P2", ECValue("string1"));
    classBInstance2->SetValue("P2", ECValue("string2"));

    ECInstanceInserter classBinserter(ecdb, *classB);
    ASSERT_TRUE(classBinserter.IsValid());

    stat = classBinserter.Insert(*classBInstance1, true);
    ASSERT_TRUE(stat == SUCCESS);
    stat = classBinserter.Insert(*classBInstance2, true);
    ASSERT_TRUE(stat == SUCCESS);

    //Get Relationship Classes
    ECRelationshipClassCP baseHasClassAClass = schema->GetClassCP("BaseHasClassA")->GetRelationshipClassCP();
    ASSERT_TRUE(baseHasClassAClass != nullptr);
    ECRelationshipClassCP baseHasClassBClass = schema->GetClassCP("BaseHasClassB")->GetRelationshipClassCP();
    ASSERT_TRUE(baseHasClassBClass != nullptr);

    {//Insert Instances for Relationship TPHhasClassA
    ECN::StandaloneECRelationshipInstancePtr relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*baseHasClassAClass)->CreateRelationshipInstance();
    ECInstanceInserter relationshipinserter(ecdb, *baseHasClassAClass);
    ASSERT_TRUE(relationshipinserter.IsValid());

    {//Inserting 1st Instance
    relationshipInstance->SetSource(baseInstance1.get());
    relationshipInstance->SetTarget(classAInstance1.get());
    relationshipInstance->SetInstanceId("source->target");
    ASSERT_EQ(SUCCESS, relationshipinserter.Insert(*relationshipInstance));
    }
    {//Inserting 2nd Instance
    relationshipInstance->SetSource(baseInstance2.get());
    relationshipInstance->SetTarget(classAInstance2.get());
    relationshipInstance->SetInstanceId("source->target");
    ASSERT_EQ(SUCCESS, relationshipinserter.Insert(*relationshipInstance));
    }
    }

    {//Insert Instances for Relationship TPHhasClassB
    ECN::StandaloneECRelationshipInstancePtr relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*baseHasClassBClass)->CreateRelationshipInstance();
    ECInstanceInserter relationshipinserter(ecdb, *baseHasClassBClass);
    ASSERT_TRUE(relationshipinserter.IsValid());

    {//Inserting 1st Instance
    relationshipInstance->SetSource(baseInstance1.get());
    relationshipInstance->SetTarget(classBInstance1.get());
    relationshipInstance->SetInstanceId("source->target");
    ASSERT_EQ(SUCCESS, relationshipinserter.Insert(*relationshipInstance));
    }
    {//Inserting 2nd Instance
    relationshipInstance->SetSource(baseInstance2.get());
    relationshipInstance->SetTarget(classBInstance2.get());
    relationshipInstance->SetInstanceId("source->target");
    ASSERT_EQ(SUCCESS, relationshipinserter.Insert(*relationshipInstance));
    }
    }
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM t.Base"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(6, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(4, stmt.GetValueInt(0));
    stmt.Finalize();

    //Deletes the instances of BaseOwnsBase class..
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ONLY t.BaseOwnsBase"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(4, stmt.GetValueInt(0));
    stmt.Finalize();

    //Deletes the instances of BaseHasClassA class..
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ONLY t.BaseHasClassA"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(2, stmt.GetValueInt(0));
    stmt.Finalize();

    //Deletes the instances of BaseHasClassB class..
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ONLY t.BaseHasClassB"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0));
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipsAndSharedTablesTestFixture, RetrieveConstraintClassInstanceBeforeAfterInsertingRelationshipInstance)
    {
    SchemaItem testItem(SCHEMA_XML, true);
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipsAndSharedTables.ecdb");
    ASSERT_FALSE(asserted);


    ASSERT_TRUE(ecdb.TableExists("t_BaseOwnsBase"));
    ASSERT_FALSE(ecdb.TableExists("t_BaseHasClassA"));
    ASSERT_FALSE(ecdb.TableExists("t_BaseHasClassB"));

    ECSqlStatement insertStatement;
    ECInstanceKey TPHKey1;
    ECInstanceKey TPHKey2;
    ASSERT_EQ(ECSqlStatus::Success, insertStatement.Prepare(ecdb, "INSERT INTO t.Base (P0) VALUES ('string1')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStatement.Step(TPHKey1));
    ASSERT_TRUE(TPHKey1.IsValid());
    insertStatement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, insertStatement.Prepare(ecdb, "INSERT INTO t.Base (P0) VALUES ('string2')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStatement.Step(TPHKey2));
    ASSERT_TRUE(TPHKey2.IsValid());
    insertStatement.Finalize();

    ECInstanceKey classAKey1;
    ECInstanceKey classAKey2;
    ASSERT_EQ(ECSqlStatus::Success, insertStatement.Prepare(ecdb, "INSERT INTO t.ClassA (P1) VALUES ('string1')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStatement.Step(classAKey1));
    ASSERT_TRUE(classAKey1.IsValid());
    insertStatement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, insertStatement.Prepare(ecdb, "INSERT INTO t.ClassA (P1) VALUES ('string2')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStatement.Step(classAKey2));
    ASSERT_TRUE(classAKey2.IsValid());
    insertStatement.Finalize();

    //retrieve ECInstance from Db before inserting Relationship Instance, based on ECInstanceId, verify ECInstance is valid
    ECSqlStatement selectStmt;
    ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(ecdb, "SELECT * FROM t.Base WHERE ECInstanceId = ?"));
    selectStmt.BindId(1, TPHKey1.GetECInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
    ECInstanceECSqlSelectAdapter TPHadapter(selectStmt);
    IECInstancePtr readInstance = TPHadapter.GetInstance();
    ASSERT_TRUE(readInstance.IsValid());
    selectStmt.Finalize();

    ECSqlStatement relationStmt;
    ASSERT_EQ(relationStmt.Prepare(ecdb, "INSERT INTO t.BaseHasClassA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    relationStmt.BindId(1, TPHKey1.GetECInstanceId());
    relationStmt.BindId(2, TPHKey1.GetECClassId());
    relationStmt.BindId(3, classAKey1.GetECInstanceId());
    relationStmt.BindId(4, classAKey1.GetECClassId());
    ASSERT_EQ(BE_SQLITE_DONE, relationStmt.Step());
    relationStmt.Finalize();

    //try to insert Duplicate relationship step() should return error
    ASSERT_EQ(relationStmt.Prepare(ecdb, "INSERT INTO t.BaseHasClassA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    relationStmt.BindId(1, TPHKey1.GetECInstanceId());
    relationStmt.BindId(2, TPHKey1.GetECClassId());
    relationStmt.BindId(3, classAKey1.GetECInstanceId());
    relationStmt.BindId(4, classAKey1.GetECClassId());
    ASSERT_TRUE((BE_SQLITE_CONSTRAINT_BASE & relationStmt.Step()) == BE_SQLITE_CONSTRAINT_BASE);
    relationStmt.Finalize();

    //retrieve ECInstance from Db After Inserting Relationship Instance, based on ECInstanceId, verify ECInstance is valid
    ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(ecdb, "SELECT * FROM t.ClassA WHERE ECInstanceId = ?"));
    selectStmt.BindId(1, classAKey1.GetECInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
    ECInstanceECSqlSelectAdapter ClassAadapter(selectStmt);
    readInstance = ClassAadapter.GetInstance();
    ASSERT_TRUE(readInstance.IsValid());
    selectStmt.Finalize();

    ecdb.CloseDb();
    }

//=======================================================================================    
// @bsiclass                                   Muhammad Hassan                     05/15
//=======================================================================================    
struct ReferentialIntegrityTestFixture : ECDbMappingTestFixture
    {
    private:
        void VerifyRelationshipInsertionIntegrity(ECDbCR ecdb, Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<DbResult> const& expected, size_t& rowInserted) const;
        size_t GetRelationshipInstanceCount(ECDbCR ecdb, Utf8CP relationshipClass) const;

    protected:
        void ExecuteRelationshipInsertionIntegrityTest(ECDbR ecdb, bool allowDuplicateRelationships, bool allowForeignKeyConstraint, bool schemaImportExpectedToSucceed) const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ReferentialIntegrityTestFixture, ForeignKeyRelationshipMap_EnforceReferentialIntegrity)
    {
    ECDbR ecdb = SetupECDb("ForeignKeyRelationshipMap_EnforceReferentialIntegrity.ecdb");
    ExecuteRelationshipInsertionIntegrityTest(ecdb, false, true, true);
    //when AllowDuplicate is turned of, OneFooHasManyGoo will also be mapped as endtable therefore ReferentialIntegrityCheck will be performed for it, so there will be two rows in the ForeignKey table
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasManyGoo"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ReferentialIntegrityTestFixture, ForeignKeyConstraint_EnforceReferentialIntegrityCheck_AllowDuplicateRelation)
    {
    ECDbR ecdb = SetupECDb("ForeignKeyConstraint_EnforceReferentialIntegrityCheck_AllowDuplicateRelation.ecdb");
    ExecuteRelationshipInsertionIntegrityTest(ecdb, true, true, true);
    //when AllowDuplicate is turned on, OneFooHasManyGoo will also be mapped as endtable therefore there will be only one row in the ForeignKey table
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasManyGoo"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         02/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ReferentialIntegrityTestFixture, DoNotAllowDuplicateRelationships)
    {
    ECDbR ecdb = SetupECDb("RelationshipCardinalityTest.ecdb");
    ExecuteRelationshipInsertionIntegrityTest(ecdb, false, false, true);
    ASSERT_TRUE(ecdb.TableExists("ts_Foo"));
    ASSERT_TRUE(ecdb.TableExists("ts_Goo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasManyGoo"));
    ASSERT_TRUE(ecdb.TableExists("ts_ManyFooHasManyGoo"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         02/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ReferentialIntegrityTestFixture, AllowDuplicateRelationships)
    {
    ECDbR ecdb = SetupECDb("RelationshipCardinalityTest_AllowDuplicateRelationships.ecdb");
    ExecuteRelationshipInsertionIntegrityTest(ecdb, true, false, true);
    ASSERT_TRUE(ecdb.TableExists("ts_Foo"));
    ASSERT_TRUE(ecdb.TableExists("ts_Goo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasManyGoo"));
    ASSERT_TRUE(ecdb.TableExists("ts_ManyFooHasManyGoo"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ReferentialIntegrityTestFixture::VerifyRelationshipInsertionIntegrity(ECDbCR ecdb, Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<DbResult> const& expected, size_t& rowInserted) const
    {
    ECSqlStatement stmt;
    auto sql = SqlPrintfString("INSERT INTO %s (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(?,?,?,?)", relationshipClass);
    ASSERT_EQ(stmt.Prepare(ecdb, sql.GetUtf8CP()), ECSqlStatus::Success);
    ASSERT_EQ(expected.size(), sourceKeys.size() * targetKeys.size());
    int n = 0;
    for (auto& fooKey : sourceKeys)
        {
        for (auto& gooKey : targetKeys)
            {
            stmt.Reset();
            ASSERT_EQ(ECSqlStatus::Success, stmt.ClearBindings());
            stmt.BindId(1, fooKey.GetECInstanceId());
            stmt.BindId(2, fooKey.GetECClassId());
            stmt.BindId(3, gooKey.GetECInstanceId());
            stmt.BindId(4, gooKey.GetECClassId());
            if (expected[n] != BE_SQLITE_DONE)
                ASSERT_NE(BE_SQLITE_DONE, stmt.Step());
            else
                {
                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
                rowInserted++;
                }

            n = n + 1;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         02/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ReferentialIntegrityTestFixture::GetRelationshipInstanceCount(ECDbCR ecdb, Utf8CP relationshipClass) const
    {
    ECSqlStatement stmt;
    auto sql = SqlPrintfString("SELECT COUNT(*) FROM ONLY ts.Foo JOIN ts.Goo USING %s", relationshipClass);
    if (stmt.Prepare(ecdb, sql.GetUtf8CP()) == ECSqlStatus::Success)
        {
        if (stmt.Step() == BE_SQLITE_ROW)
            return static_cast<size_t>(stmt.GetValueInt(0));
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ReferentialIntegrityTestFixture::ExecuteRelationshipInsertionIntegrityTest(ECDbR ecdb, bool allowDuplicateRelationships, bool allowForeignKeyConstraint, bool schemaImportExpectedToSucceed) const
    {
    ECSchemaPtr testSchema;
    ECEntityClassP foo = nullptr, goo = nullptr;
    ECRelationshipClassP oneFooHasOneGoo = nullptr, oneFooHasManyGoo = nullptr, manyFooHasManyGoo = nullptr;
    PrimitiveECPropertyP prim;
    auto readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(ecdb.GetSchemaLocater());
    auto ecdbmapKey = SchemaKey("ECDbMap", 2, 0);
    auto ecdbmapSchema = readContext->LocateSchema(ecdbmapKey, SchemaMatchType::LatestCompatible);
    ASSERT_TRUE(ecdbmapSchema.IsValid());

    ECSchema::CreateSchema(testSchema, "TestSchema", 1, 0);
    ASSERT_TRUE(testSchema.IsValid());

    testSchema->SetNamespacePrefix("ts");
    testSchema->AddReferencedSchema(*ecdbmapSchema);

    testSchema->CreateEntityClass(foo, "Foo");
    testSchema->CreateEntityClass(goo, "Goo");

    testSchema->CreateRelationshipClass(oneFooHasOneGoo, "OneFooHasOneGoo");
    testSchema->CreateRelationshipClass(oneFooHasManyGoo, "OneFooHasManyGoo");
    testSchema->CreateRelationshipClass(manyFooHasManyGoo, "ManyFooHasManyGoo");

    ASSERT_TRUE(foo != nullptr);
    ASSERT_TRUE(foo != nullptr);
    ASSERT_TRUE(oneFooHasOneGoo != nullptr);
    ASSERT_TRUE(oneFooHasManyGoo != nullptr);
    ASSERT_TRUE(manyFooHasManyGoo != nullptr);

    prim = nullptr;
    foo->CreatePrimitiveProperty(prim, "fooProp");
    prim->SetType(PrimitiveType::PRIMITIVETYPE_String);
    ASSERT_TRUE(prim != nullptr);

    prim = nullptr;
    goo->CreatePrimitiveProperty(prim, "gooProp");
    prim->SetType(PrimitiveType::PRIMITIVETYPE_String);
    ASSERT_TRUE(prim != nullptr);

    oneFooHasOneGoo->GetSource().AddClass(*foo);
    oneFooHasOneGoo->GetSource().SetCardinality(RelationshipCardinality::ZeroOne());
    oneFooHasOneGoo->GetTarget().AddClass(*goo);
    oneFooHasOneGoo->GetTarget().SetCardinality(RelationshipCardinality::ZeroOne());

    oneFooHasManyGoo->GetSource().AddClass(*foo);
    oneFooHasManyGoo->GetSource().SetCardinality(RelationshipCardinality::ZeroOne());
    oneFooHasManyGoo->GetTarget().AddClass(*goo);
    oneFooHasManyGoo->GetTarget().SetCardinality(RelationshipCardinality::OneMany());

    manyFooHasManyGoo->GetSource().AddClass(*foo);
    manyFooHasManyGoo->GetSource().SetCardinality(RelationshipCardinality::OneMany());
    manyFooHasManyGoo->GetTarget().AddClass(*goo);
    manyFooHasManyGoo->GetTarget().SetCardinality(RelationshipCardinality::OneMany());
    BackDoor::ECObjects::ECSchemaReadContext::AddSchema(*readContext, *testSchema);

    if (allowDuplicateRelationships)
        {
        auto caInstClass = ecdbmapSchema->GetClassCP("LinkTableRelationshipMap");
        ASSERT_TRUE(caInstClass != nullptr);
        auto caInst = caInstClass->GetDefaultStandaloneEnabler()->CreateInstance();
        ASSERT_TRUE(caInst != nullptr);
        ASSERT_TRUE(caInst->SetValue("AllowDuplicateRelationships", ECValue(true)) == ECObjectsStatus::Success);
        ASSERT_TRUE(manyFooHasManyGoo->SetCustomAttribute(*caInst) == ECObjectsStatus::Success);
        }

    if (allowForeignKeyConstraint)
        {
        auto fkMapClass = ecdbmapSchema->GetClassCP("ForeignKeyRelationshipMap");
        ASSERT_TRUE(fkMapClass != nullptr);
        auto caInst = fkMapClass->GetDefaultStandaloneEnabler()->CreateInstance();
        ASSERT_TRUE(caInst != nullptr);
        ASSERT_TRUE(oneFooHasOneGoo->SetCustomAttribute(*caInst) == ECObjectsStatus::Success);
        ASSERT_TRUE(oneFooHasManyGoo->SetCustomAttribute(*caInst) == ECObjectsStatus::Success);
        }

    if (schemaImportExpectedToSucceed)
        ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(readContext->GetCache()));
    else
        {
        ASSERT_EQ(ERROR, ecdb.Schemas().ImportECSchemas(readContext->GetCache()));
        return;
        }

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;

    ECSqlStatement fooStmt;
    ASSERT_EQ(fooStmt.Prepare(ecdb, "INSERT INTO ts.Foo(fooProp) VALUES(?)"), ECSqlStatus::Success);
    for (auto i = 0; i < maxFooInstances; i++)
        {
        ECInstanceKey out;
        ASSERT_EQ(fooStmt.Reset(), ECSqlStatus::Success);
        ASSERT_EQ(fooStmt.ClearBindings(), ECSqlStatus::Success);
        ASSERT_EQ(fooStmt.BindText(1, SqlPrintfString("foo_%d", i), IECSqlBinder::MakeCopy::Yes), ECSqlStatus::Success);
        ASSERT_EQ(fooStmt.Step(out), BE_SQLITE_DONE);
        fooKeys.push_back(out);
        }

    ECSqlStatement gooStmt;
    ASSERT_EQ(gooStmt.Prepare(ecdb, "INSERT INTO ts.Goo(gooProp) VALUES(?)"), ECSqlStatus::Success);
    for (auto i = 0; i < maxGooInstances; i++)
        {
        ECInstanceKey out;
        ASSERT_EQ(gooStmt.Reset(), ECSqlStatus::Success);
        ASSERT_EQ(gooStmt.ClearBindings(), ECSqlStatus::Success);
        ASSERT_EQ(gooStmt.BindText(1, SqlPrintfString("goo_%d", i), IECSqlBinder::MakeCopy::Yes), ECSqlStatus::Success);
        ASSERT_EQ(gooStmt.Step(out), BE_SQLITE_DONE);
        gooKeys.push_back(out);
        }

    //Compute what are the right valid permutation
    std::vector<DbResult> oneFooHasOneGooResult;
    std::vector<DbResult> oneFooHasManyGooResult;
    std::vector<DbResult> manyFooHasManyGooResult;
    std::vector<DbResult> reinsertResultError;
    std::vector<DbResult> reinsertResultDone;
    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //1:1 is not effected with AllowDuplicateRelationships
            if (f == g)
                oneFooHasOneGooResult.push_back(BE_SQLITE_DONE);
            else
                oneFooHasOneGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //1:N is effected with AllowDuplicateRelationships
            if (f == 0)
                oneFooHasManyGooResult.push_back(BE_SQLITE_DONE);
            else
                oneFooHasManyGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            manyFooHasManyGooResult.push_back(BE_SQLITE_DONE);
            reinsertResultError.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);
            reinsertResultDone.push_back(BE_SQLITE_DONE);
            }
        }

    //1:1--------------------------------
    size_t count_OneFooHasOneGoo = 0;
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.OneFooHasOneGoo", fooKeys, gooKeys, oneFooHasOneGooResult, count_OneFooHasOneGoo);
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.OneFooHasOneGoo", fooKeys, gooKeys, reinsertResultError, count_OneFooHasOneGoo);

    MapStrategyInfo mapStrategy;
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, oneFooHasOneGoo->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);
    ASSERT_EQ(count_OneFooHasOneGoo, GetRelationshipInstanceCount(ecdb, "ts.OneFooHasOneGoo"));

    //1:N--------------------------------
    size_t count_OneFooHasManyGoo = 0;
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.OneFooHasManyGoo", fooKeys, gooKeys, oneFooHasManyGooResult, count_OneFooHasManyGoo);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, oneFooHasManyGoo->GetId()));
    ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);
    ASSERT_EQ(count_OneFooHasManyGoo, GetRelationshipInstanceCount(ecdb, "ts.OneFooHasManyGoo"));

    //N:N--------------------------------
    size_t count_ManyFooHasManyGoo = 0;
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, manyFooHasManyGooResult, count_ManyFooHasManyGoo);
    if (allowDuplicateRelationships)
        VerifyRelationshipInsertionIntegrity(ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, reinsertResultDone, count_ManyFooHasManyGoo);
    else
        VerifyRelationshipInsertionIntegrity(ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, reinsertResultError, count_ManyFooHasManyGoo);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, manyFooHasManyGoo->GetId()));

    ASSERT_EQ(MapStrategyInfo::Strategy::OwnTable, mapStrategy.m_strategy);
    ASSERT_TRUE(mapStrategy.m_tphInfo.IsUnset());
    ASSERT_EQ(count_ManyFooHasManyGoo, GetRelationshipInstanceCount(ecdb, "ts.ManyFooHasManyGoo"));
    }

END_ECDBUNITTESTS_NAMESPACE