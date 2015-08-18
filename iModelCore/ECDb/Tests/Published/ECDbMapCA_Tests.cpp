/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbMapCA_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (SchemaImportTestFixture, ECDbMapTests)
    {
    std::vector<TestItem> testItems{

    TestItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='ParentA' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>OwnTable</Strategy>"
    "                   <IsPolymorphic>True</IsPolymorphic>"
    "                 </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "    <ECClass typeName='ParentB' isDomainClass='True'>"
    "        <BaseClass>ParentA</BaseClass>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <IsPolymorphic>True</IsPolymorphic>"
    "                 </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "    <ECClass typeName='Child' isDomainClass='True'>"
    "        <BaseClass>ParentB</BaseClass>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <IsPolymorphic>True</IsPolymorphic>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "        <ECProperty propertyName='Price_child' typeName='double' />"
    "    </ECClass>"
    "</ECSchema>", false, ""),

    TestItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Parent' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <Options>SharedColumns</Options>"
    "                   <IsPolymorphic>True</IsPolymorphic>"
    "                 </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "    <ECClass typeName='Child' isDomainClass='True'>"
    "        <BaseClass>Parent</BaseClass>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Options>DisableSharedColumns</Options>"
    "                   <Strategy>OwnTable</Strategy>"
    "                 </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "</ECSchema>", false , "Option 'DisableSharedColumn' doesn't allow strategy to be set."),

    TestItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='ParentA' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>ExistingTable</Strategy>"
    "                   <IsPolymorphic>True</IsPolymorphic>"
    "                 </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price_parentA' typeName='double' />"
    "    </ECClass>"
    "</ECSchema>", false, ""),

    TestItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Parent' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <Options>ReadOnly</Options>"
    "                 </MapStrategy>"
    "                <TableName>TestTable</TableName>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "</ECSchema>", false, "Option ReadOnly, can only be used with the Strategy ExistingTable"),

    TestItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Parent' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>ExistingTable</Strategy>"
    "                   <Options>abc</Options>"
    "                 </MapStrategy>"
    "                <TableName>TestTable</TableName>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "</ECSchema>", false, "abc is not a valid MapStarategy Option"),

    TestItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Parent' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>OwnTable</Strategy>"
    "                   <IsPolymorphic>True</IsPolymorphic>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "    <ECClass typeName='Child' isDomainClass='True'>"
    "        <BaseClass>Parent</BaseClass>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                </MapStrategy>"
    "                <TableName>TestTable</TableName>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='P3' typeName='int' />"
    "    </ECClass>"
    "</ECSchema>", false, "Polymorphic parent class (with any strategy) doesn't allow a child class to have it's own strategy."),

    TestItem ("<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Parent' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <IsPolymorphic>True</IsPolymorphic>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='P1' typeName='int' />"
    "    </ECClass>"
    "    <ECClass typeName='Child1' isDomainClass='True'>"
    "        <BaseClass>Parent</BaseClass>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>NotMapped</Strategy>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "</ECSchema>", false, "Strategy NotMapped-polymorphic is not supported within a  class heirarchy"),

    TestItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='ClassA' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>OwnTable</Strategy>"
    "                </MapStrategy>"
    "                <TableName>bla</TableName>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "</ECSchema>", false, "Own Table does'nt allows a user Defined table name.")
    };

    for (TestItem const& item : testItems)
        {
        AssertSchemaImport (item, "ecdbmapcatests.ecdb");
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaImportTestFixture, ECDbMapCATests)
    {
    std::vector<TestItem> testItems {

    TestItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='ClassA' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy><Strategy>bla</Strategy></MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "    <ECClass typeName='ClassAB' isDomainClass='True'>"
    "        <BaseClass>ClassA</BaseClass>"
    "    </ECClass>"
    "</ECSchema>", false, ""),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='ClassA' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>OwnTable</Strategy>"
        "                   <Options>SharedColumns</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, ""),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>ExistingTable</Strategy>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy ExistingTable expects TableName to be set"),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>ExistingTable</Strategy>"
        "                </MapStrategy>"
        "                <TableName>idontexist</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy ExistingTable expects table specified by TableName to preexist"),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>OwnTable</Strategy>"
        "                   <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "                <TableName>bla</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy OwnTable doesn't allow TableName to be set."),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>OwnTable</Strategy>"
        "                   <IsPolymorphic>False</IsPolymorphic>"
        "                </MapStrategy>"
        "                <TableName>bla</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy OwnTable doesn't allow TableName to be set."),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "                <TableName>bla</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy SharedTable, polymorphic doesn't allow TableName to be set."),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <IsPolymorphic>False</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy SharedTable, non-polymorphic expects TableName to be set."),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <IsPolymorphic>False</IsPolymorphic>"
        "                </MapStrategy>"
        "                <TableName>idontexistyet</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", true, "MapStrategy SharedTable, non-polymorphic doesn't expect table specified in TableName to be set."),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>NotMapped</Strategy>"
        "                   <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "                <TableName>bla</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy NotMapped, polymorphic doesn't allow TableName to be set."),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Class' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>NotMapped</Strategy>"
        "                   <IsPolymorphic>False</IsPolymorphic>"
        "                </MapStrategy>"
        "                <TableName>bla</TableName>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Price' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy NotMapped, non-polymorphic doesn't allow TableName to be set."),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Base' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='Sub' isDomainClass='True'>"
        "        <BaseClass>Base</BaseClass>" 
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='SubSub' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy SharedTable (polymorphic) on child class where base has SharedTable (polymorphic) is not supported."),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Base1' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='Base2' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='Sub' isDomainClass='True'>"
        "        <BaseClass>Base1</BaseClass>"
        "        <BaseClass>Base2</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECClass>"
        "</ECSchema>", false, "Child class has two base classes which both have MapStrategy SharedTable (polymorphic). This is not expected to be supported."),

        TestItem (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Base' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='Sub' isDomainClass='True'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='SubSub' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>NotMapped</Strategy>"
        "                  <IsPolymorphic>False</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='SubSubSub' isDomainClass='True'>"
        "        <BaseClass>SubSub</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECClass>"
        "</ECSchema>", false, "MapStrategy NotMapped on child class where base has SharedTable (polymorphic) is not supported."),

        TestItem (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Base' isDomainClass='True'>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='Sub' isDomainClass='True'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='SubSub' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>NotMapped</Strategy>"
        "                  <IsPolymorphic>False</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='SubSubSub' isDomainClass='True'>"
        "        <BaseClass>SubSub</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECClass>"
        "</ECSchema>", true, ""),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='A' isDomainClass='True'>"
        "        <ECProperty propertyName='AA' typeName='double' />"
        "    </ECClass>"
        "    <ECClass typeName='B' isDomainClass='True'>"
        "        <ECProperty propertyName='BB' typeName='double' />"
        "    </ECClass>"
        "    <ECRelationshipClass typeName='Rel' isDomainClass='True' strength='referencing'>"
        "        <ECCustomAttributes>"
        "            <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
        "            </ForeignKeyRelationshipMap>"
        "        </ECCustomAttributes>"
        "       <Source cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='B' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>", false, "ForeignKeyRelationshipMap on N:N relationship is not supported"),

        TestItem("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TeststructClassInPolymorphicSharedTable' nameSpacePrefix='tph' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='BaseClass' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='p1' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='CustomAttributeClass' isDomainClass='False' isCustomAttributeClass='True'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECProperty propertyName='p2' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='isStructClass' isStruct='True' isDomainClass='True'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECProperty propertyName='p3' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='NonDomainClass' isDomainClass='False'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECProperty propertyName='p4' typeName='string' />"
        "    </ECClass>"
        "</ECSchema>", false, "Struct in class hierarchy with SharedTable (polymorphic) map strategy is expected to be not supported."),
        
        TestItem (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='ClassA' isDomainClass='True'>"
        "    </ECClass>"
        "</ECSchema>", true, " A class expects a property to be set.")
        };

    for (TestItem const& item : testItems)
        {
        AssertSchemaImport(item, "ecdbmapcatests.ecdb");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaImportTestFixture, AbstractClassWithPolymorphicAndNonPolymorphicSharedTable)
    {
    TestItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestAbstractClasses' nameSpacePrefix='tac' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
                        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                        "    <ECClass typeName='AbstractBaseClass' isDomainClass='False'>"
                        "        <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.01.00'>"
                        "                <MapStrategy>"
                        "                  <Strategy>SharedTable</Strategy>"
                        "                  <IsPolymorphic>True</IsPolymorphic>"
                        "                </MapStrategy>"
                        "            </ClassMap>"
                        "        </ECCustomAttributes>"
                        "        <ECProperty propertyName='P1' typeName='string' />"
                        "    </ECClass>"
                        "    <ECClass typeName='ChildDomainClassA' isDomainClass='True'>"
                        "        <BaseClass>AbstractBaseClass</BaseClass>"
                        "        <ECProperty propertyName='P2' typeName='string' />"
                        "    </ECClass>"
                        "    <ECClass typeName='ChildDomainClassB' isDomainClass='True'>"
                        "        <BaseClass>AbstractBaseClass</BaseClass>"
                        "        <ECProperty propertyName='P3' typeName='string' />"
                        "    </ECClass>"
                        "    <ECClass typeName='SharedTable' isDomainClass='False'>"
                        "        <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.01.00'>"
                        "                <MapStrategy>"
                        "                  <Strategy>SharedTable</Strategy>"
                        "                  <IsPolymorphic>False</IsPolymorphic>"
                        "                </MapStrategy>"
                        "                <TableName>SharedTable</TableName>"
                        "            </ClassMap>"
                        "        </ECCustomAttributes>"
                        "        <ECProperty propertyName='P1' typeName='string' />"
                        "    </ECClass>"
                        "    <ECClass typeName='SharedTable1' isDomainClass='True'>"
                        "        <ECCustomAttributes>"
                        "            <ClassMap xmlns='ECDbMap.01.00'>"
                        "                <MapStrategy>"
                        "                  <Strategy>SharedTable</Strategy>"
                        "                  <IsPolymorphic>False</IsPolymorphic>"
                        "                </MapStrategy>"
                        "                <TableName>SharedTable</TableName>"
                        "            </ClassMap>"
                        "        </ECCustomAttributes>"
                        "        <ECProperty propertyName='P2' typeName='string' />"
                        "    </ECClass>"
                        "</ECSchema>",
                        true, "");

    ECDb db;
    bool asserted = false;
    AssertSchemaImport(db, asserted, testItem, "abstractclass.ecdb");
    ASSERT_FALSE(asserted);

    //verify tables
    ASSERT_TRUE(db.TableExists("SharedTable"));
    ASSERT_TRUE(db.TableExists("tac_AbstractBaseClass"));
    ASSERT_FALSE(db.TableExists("tac_ChildDomainClassA"));
    ASSERT_FALSE(db.TableExists("tac_ChildDomainClassB"));
    ASSERT_FALSE(db.TableExists("tac_SharedTable"));
    ASSERT_FALSE(db.TableExists("tac_SharedTable1"));

    //verify ECSqlStatement
    ECSqlStatement s1, s2, s3, s4, s5;
    ASSERT_EQ(s1.Prepare(db, "INSERT INTO tac.AbstractBaseClass (P1) VALUES('Hello')"), ECSqlStatus::InvalidECSql);
    ASSERT_EQ(s4.Prepare(db, "INSERT INTO tac.SharedTable (P1) VALUES('Hello')"), ECSqlStatus::InvalidECSql);
    //Noabstract classes
    ASSERT_EQ(s2.Prepare(db, "INSERT INTO tac.ChildDomainClassA (P1, P2) VALUES('Hello', 'World')"), ECSqlStatus::Success);
    ASSERT_EQ(s2.Step(), ECSqlStepStatus::Done);
    ASSERT_EQ(s3.Prepare(db, "INSERT INTO tac.ChildDomainClassB (P1, P3) VALUES('Hello', 'World')"), ECSqlStatus::Success);
    ASSERT_EQ(s3.Step(), ECSqlStepStatus::Done);
    ASSERT_EQ(s5.Prepare(db, "INSERT INTO tac.SharedTable1 (P2) VALUES('Hello')"), ECSqlStatus::Success);
    ASSERT_EQ(s5.Step(), ECSqlStepStatus::Done);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaImportTestFixture, PolymorphicSharedTable_SharedColumns)
    {
    TestItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='SchemaWithSharedColumns' nameSpacePrefix='rc' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='BaseClass' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <Options>SharedColumnsForSubclasses</Options>"
        "                  <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P1' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='ChildDomainClassA' isDomainClass='True'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='double' />"
        "    </ECClass>"
        "    <ECClass typeName='ChildDomainClassB' isDomainClass='True'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='DerivedA' isDomainClass='True'>"
        "        <BaseClass>ChildDomainClassA</BaseClass>"
        "        <ECProperty propertyName='P4' typeName='double' />"
        "    </ECClass>"
        "    <ECClass typeName='DerivedB' isDomainClass='True'>"
        "        <BaseClass>ChildDomainClassA</BaseClass>"
        "        <ECProperty propertyName='P5' typeName='string' />"
        "    </ECClass>"
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
    ASSERT_EQ(s1.Step(), ECSqlStepStatus::Done);
    ASSERT_EQ(s2.Prepare(db, "INSERT INTO rc.ChildDomainClassA (P1, P2) VALUES('ChildClassA', 10.002)"), ECSqlStatus::Success);
    ASSERT_EQ(s2.Step(), ECSqlStepStatus::Done);
    ASSERT_EQ(s3.Prepare(db, "INSERT INTO rc.ChildDomainClassB (P1, P3) VALUES('ChildClassB', 2)"), ECSqlStatus::Success);
    ASSERT_EQ(s3.Step(), ECSqlStepStatus::Done);
    ASSERT_EQ(s4.Prepare(db, "INSERT INTO rc.DerivedA (P1, P2, P4) VALUES('DerivedA', 11.003, 12.004)"), ECSqlStatus::Success);
    ASSERT_EQ(s4.Step(), ECSqlStepStatus::Done);
    ASSERT_EQ(s5.Prepare(db, "INSERT INTO rc.DerivedB (P1, P2, P5) VALUES('DerivedB', 11.003, 'DerivedB')"), ECSqlStatus::Success);
    ASSERT_EQ(s5.Step(), ECSqlStepStatus::Done);

    //verify No of Columns in BaseClass
    Statement statement;
    ASSERT_EQ(statement.Prepare(db, "SELECT * FROM rc_BaseClass"), DbResult::BE_SQLITE_OK);
    ASSERT_EQ(statement.Step(), DbResult::BE_SQLITE_ROW);
    size_t columnCount = statement.GetColumnCount();
    ASSERT_EQ(columnCount, 5);

    //verify that the columns generated are same as expected
    Utf8String expectedColumnNames = "ECInstanceIdECClassIdP1x01x02";
    Utf8String actualColumnNames;
    for (int i = 0; i < 5; i++)
        {
        actualColumnNames.append(statement.GetColumnName(i));
        }
    ASSERT_EQ(expectedColumnNames, actualColumnNames);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaImportTestFixture, PolymorphicSharedTable_SharedColumns_DisableSharedColumns)
    {
    TestItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='SchemaWithSharedColumns' nameSpacePrefix='rc' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='BaseClass' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <Options>SharedColumnsForSubclasses</Options>"
        "                  <IsPolymorphic>True</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P1' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='ChildDomainClassA' isDomainClass='True'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Options>DisableSharedColumns</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P2' typeName='double' />"
        "    </ECClass>"
        "    <ECClass typeName='ChildDomainClassB' isDomainClass='True'>"
        "        <BaseClass>BaseClass</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Options>DisableSharedColumns</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='DerivedA' isDomainClass='True'>"
        "        <BaseClass>ChildDomainClassA</BaseClass>"
        "        <ECProperty propertyName='P4' typeName='double' />"
        "    </ECClass>"
        "    <ECClass typeName='DerivedB' isDomainClass='True'>"
        "        <BaseClass>ChildDomainClassA</BaseClass>"
        "        <ECProperty propertyName='P5' typeName='string' />"
        "    </ECClass>"
        "</ECSchema>", true, "");

    ECDb db;
    bool asserted = false;
    AssertSchemaImport(db, asserted, testItem, "sharedcolumnstest.ecdb");
    ASSERT_FALSE(asserted);

    //verify tables
    ASSERT_TRUE(db.TableExists("rc_BaseClass"));
    ASSERT_FALSE(db.TableExists("rc_ChildDomainClassA"));
    ASSERT_FALSE(db.TableExists("rc_ChildDomainClassB"));
    ASSERT_FALSE(db.TableExists("rc_DerivedA"));
    ASSERT_FALSE(db.TableExists("rc_DerivedB"));

    //verify ECSqlStatments
    ECSqlStatement s1, s2, s3, s4, s5;
    ASSERT_EQ(ECSqlStatus::Success, s1.Prepare(db, "INSERT INTO rc.BaseClass (P1) VALUES('HelloWorld')"));
    ASSERT_EQ(ECSqlStepStatus::Done, s1.Step());
    ASSERT_EQ(ECSqlStatus::Success, s2.Prepare(db, "INSERT INTO rc.ChildDomainClassA (P1, P2) VALUES('ChildClassA', 10.002)"));
    ASSERT_EQ(ECSqlStepStatus::Done, s2.Step());
    ASSERT_EQ(ECSqlStatus::Success, s3.Prepare(db, "INSERT INTO rc.ChildDomainClassB (P1, P3) VALUES('ChildClassB', 2)"));
    ASSERT_EQ(ECSqlStepStatus::Done, s3.Step());
    ASSERT_EQ(ECSqlStatus::Success, s4.Prepare(db, "INSERT INTO rc.DerivedA (P1, P2, P4) VALUES('DerivedA', 11.003, 12.004)"));
    ASSERT_EQ(ECSqlStepStatus::Done, s4.Step());
    ASSERT_EQ(ECSqlStatus::Success, s5.Prepare(db, "INSERT INTO rc.DerivedB (P1, P2, P5) VALUES('DerivedB', 11.003, 'DerivedB')"));
    ASSERT_EQ(ECSqlStepStatus::Done, s5.Step());

    //verify No of Columns in BaseClass
    const int expectedColCount = 6;
    Statement statement;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, statement.Prepare(db, "SELECT * FROM rc_BaseClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(expectedColCount, statement.GetColumnCount());

    //verify that the columns generated are same as expected
    Utf8String expectedColumnNames = "ECInstanceIdECClassIdP1P2x01P3";
    Utf8String actualColumnNames;
    for (int i = 0; i < expectedColCount; i++)
        {
        actualColumnNames.append(statement.GetColumnName(i));
        }
    ASSERT_STREQ(expectedColumnNames.c_str(), actualColumnNames.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaImportTestFixture, NotMappedWithinClassHierarchy)
    {
    TestItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Base' isDomainClass='True'>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='Sub' isDomainClass='True'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='SubSub' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>NotMapped</Strategy>"
        "                  <IsPolymorphic>False</IsPolymorphic>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <BaseClass>Sub</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='SubSubSub' isDomainClass='True'>"
        "        <BaseClass>SubSub</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECClass>"
        "</ECSchema>", true, "");

    ECDb db;
    bool asserted = false;
    AssertSchemaImport(db, asserted, testItem, "NotMappedWithinClassHierarchy.ecdb");
    ASSERT_FALSE(asserted);

    //verify tables
    ASSERT_TRUE(db.TableExists("ts_Base"));
    ASSERT_TRUE(db.TableExists("ts_Sub"));
    ASSERT_FALSE(db.TableExists("ts_SubSub"));
    ASSERT_TRUE(db.TableExists("ts_SubSubSub"));
    ASSERT_TRUE(db.ColumnExists("ts_SubSubSub", "P0"));
    ASSERT_TRUE(db.ColumnExists("ts_SubSubSub", "P1"));
    ASSERT_TRUE(db.ColumnExists("ts_SubSubSub", "P2"));
    ASSERT_TRUE(db.ColumnExists("ts_SubSubSub", "P3"));

    //verify ECSQL
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(db, "INSERT INTO ts.SubSub (P1, P2) VALUES(1,2)")) << "INSERT not possible against unmapped class";
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(db, "SELECT * FROM ts.SubSub")) << "SELECT not possible against unmapped class";
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.SubSubSub (P1, P2, P3) VALUES(1,2,3)")) << "INSERT should be possible even if base class is not mapped";
        ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step()) << "INSERT should be possible even if base class is not mapped";
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT P1,P2,P3 FROM ts.SubSubSub")) << "SELECT should be possible even if base class is not mapped";
        ASSERT_EQ(ECSqlStepStatus::HasRow, stmt.Step()) << "SELECT should be possible even if base class is not mapped";
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(2, stmt.GetValueInt(1));
        ASSERT_EQ(3, stmt.GetValueInt(2));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaImportTestFixture, EnforceLinkTableMapping)
    {
    TestItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='A' isDomainClass='True'>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='B' isDomainClass='True'>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "    </ECClass>"
        "    <ECRelationshipClass typeName='AHasB' isDomainClass='True' strength='referencing'>"
        "        <ECCustomAttributes>"
        "            <LinkTableRelationshipMap xmlns='ECDbMap.01.00' />"
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
TEST_F(SchemaImportTestFixture, UserDefinedIndexTest)
    {
            {
            TestItem testItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='Base' isDomainClass='False' isStruct='False' isCustomAttribute='False'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.01.00'>"
                "                <MapStrategy>"
                "                   <Strategy>SharedTable</Strategy>"
                "                   <IsPolymorphic>True</IsPolymorphic>"
                "                 </MapStrategy>"
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
                "    </ECClass>"
                "    <ECClass typeName='Sub1' isDomainClass='True'>"
                "        <BaseClass>Base</BaseClass>"
                "        <ECProperty propertyName='Sub1_Prop' typeName='double' />"
                "    </ECClass>"
                "    <ECClass typeName='Sub2' isDomainClass='True'>"
                "        <BaseClass>Base</BaseClass>"
                "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
                "    </ECClass>"
                "</ECSchema>", true, "");

            ECDb db;
            bool asserted = false;
            AssertSchemaImport(db, asserted, testItem, "userdefinedindextest.ecdb");
            ASSERT_FALSE(asserted);

            AssertIndex(db, "ix_base_code", false, "ts_Base", {"Code"});
            }

            {
            TestItem testItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='Base' isDomainClass='False' isStruct='False' isCustomAttribute='False'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.01.00'>"
                "                <MapStrategy>"
                "                   <Strategy>SharedTable</Strategy>"
                "                   <IsPolymorphic>True</IsPolymorphic>"
                "                 </MapStrategy>"
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
                "    </ECClass>"
                "    <ECClass typeName='Sub1' isDomainClass='True'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.01.00'>"
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
                "    </ECClass>"
                "    <ECClass typeName='Sub2' isDomainClass='True'>"
                "        <BaseClass>Base</BaseClass>"
                "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
                "    </ECClass>"
                "</ECSchema>", true, "");

            ECDb db;
            bool asserted = false;
            AssertSchemaImport(db, asserted, testItem, "userdefinedindextest.ecdb");
            ASSERT_FALSE(asserted);

            ECClassId classId = db.Schemas().GetECClassId("ts", "Sub1", ResolveSchema::BySchemaNamespacePrefix);
            AssertIndex(db, "ix_sub1_prop", false, "ts_Base", {"Sub1_Prop"}, {classId});
            }

        {
        TestItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='Base' isDomainClass='False' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <IsPolymorphic>True</IsPolymorphic>"
            "                 </MapStrategy>"
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
            "    </ECClass>"
            "    <ECClass typeName='Sub1' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
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
            "    </ECClass>"
            "    <ECClass typeName='Sub2' isDomainClass='True'>"
            "        <BaseClass>Sub1</BaseClass>"
            "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
            "    </ECClass>"
            "</ECSchema>", true, "");

        ECDb db;
        bool asserted = false;
        AssertSchemaImport(db, asserted, testItem, "userdefinedindextest.ecdb");
        ASSERT_FALSE(asserted);

        ECClassId baseClassId = db.Schemas().GetECClassId("ts", "Base", ResolveSchema::BySchemaNamespacePrefix);
        AssertIndex(db, "ix_sub1_prop", false, "ts_Base", {"Sub1_Prop"}, {baseClassId}, true);
        }

        {
        TestItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='Base' isDomainClass='False' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <IsPolymorphic>True</IsPolymorphic>"
            "                 </MapStrategy>"
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
            "    </ECClass>"
            "    <ECClass typeName='Sub1' isDomainClass='True'>"
            "        <BaseClass>Base</BaseClass>"
            "        <ECProperty propertyName='Sub1_Prop' typeName='double' />"
            "    </ECClass>"
            "    <ECClass typeName='Sub2' isDomainClass='True'>"
            "        <BaseClass>Sub1</BaseClass>"
            "        <ECProperty propertyName='Sub2_Prop' typeName='double' />"
            "    </ECClass>"
            "    <ECClass typeName='Sub3' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                 <Indexes>"
            "                   <DbIndex>"
            "                       <Name>ix_sub3_prop</Name>"
            "                       <Properties>"
            "                          <string>Sub3_Prop</string>"
            "                       </Properties>"
            "                   </DbIndex>"
            "                 </Indexes>"
            "            </ClassMap>"
            "        </ECCustomAttributes>"
            "        <BaseClass>Base</BaseClass>"
            "        <ECProperty propertyName='Sub3_Prop' typeName='double' />"
            "    </ECClass>"
            "</ECSchema>", true, "");

        TestItem secondSchemaTestItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts' />"
            "    <ECClass typeName='Sub4' isDomainClass='True'>"
            "        <BaseClass>ts:Sub3</BaseClass>"
            "        <ECProperty propertyName='Sub4_Prop' typeName='double' />"
            "    </ECClass>"
            "</ECSchema>", true, "");
        
        ECDb db;
        bool asserted = false;
        AssertSchemaImport(db, asserted, testItem, "userdefinedindextest.ecdb");
        ASSERT_FALSE(asserted);

        ECClassId sub3ClassId = db.Schemas().GetECClassId("ts", "Sub3", ResolveSchema::BySchemaNamespacePrefix);
        AssertIndex(db, "ix_base_code", false, "ts_Base", {"Code"});
        AssertIndex(db, "ix_sub3_prop", false, "ts_Base", {"Sub3_Prop"}, {sub3ClassId});

        db.SaveChanges();
        db.ClearECDbCache();

        asserted = false;
        AssertSchemaImport(asserted, db, secondSchemaTestItem);
        ASSERT_FALSE(asserted);

        ECClassId sub4ClassId = db.Schemas().GetECClassId("ts2", "Sub4", ResolveSchema::BySchemaNamespacePrefix);
        AssertIndex(db, "ix_base_code", false, "ts_Base", {"Code"});
        AssertIndex(db, "ix_sub3_prop", false, "ts_Base", {"Sub3_Prop"}, {sub3ClassId, sub4ClassId});
        }

        {
        TestItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='Base' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <Options>SharedColumnsForSubclasses</Options>"  
            "                   <IsPolymorphic>True</IsPolymorphic>"
            "                 </MapStrategy>"
            "            </ClassMap>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='Code' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='Sub1' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
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
            "    </ECClass>"
            "    <ECClass typeName='Sub2' isDomainClass='True'>"
            "        <BaseClass>Base</BaseClass>"
            "        <ECProperty propertyName='Name' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='Sub1_1' isDomainClass='True'>"
            "        <BaseClass>Sub1</BaseClass>"
            "        <ECProperty propertyName='Cost' typeName='double' />"
            "    </ECClass>"
            "    <ECClass typeName='A' isDomainClass='True'>"
            "        <ECProperty propertyName='Id' typeName='long' />"
            "    </ECClass>"
            "   <ECRelationshipClass typeName='Rel' isDomainClass='True' strength='embedding'>"
            "    <Source cardinality='(0,1)' polymorphic='True'>"
            "      <Class class='A'>"
            "         <Key>"
            "           <Property name='Id'/>"
            "         </Key>"
            "       </Class>"
            "    </Source>"
            "    <Target cardinality='(0,1)' polymorphic='True'>"
            "      <Class class='Sub1'>"
            "         <Key>"
            "           <Property name='AId'/>"
            "         </Key>"
            "       </Class>"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", true, "");

        ECDb db;
        bool asserted = false;
        AssertSchemaImport(db, asserted, testItem, "userdefinedindextest.ecdb");
        ASSERT_FALSE(asserted);

        ECClassId sub1ClassId = db.Schemas().GetECClassId("ts", "Sub1", ResolveSchema::BySchemaNamespacePrefix);
        ECClassId sub11ClassId = db.Schemas().GetECClassId("ts", "Sub1_1", ResolveSchema::BySchemaNamespacePrefix);
        AssertIndex(db, "ix_sub1_aid", false, "ts_Base", {"x01"}, {sub1ClassId, sub11ClassId});

        //now assert that the automatically generated index on the relationship foreign key column is correct
        AssertIndex(db, "uix_ts_Base_fk_ts_Rel_target", true, "ts_Base", {"x01"}, "[x01] IS NOT NULL");

        //insert a few objects to make sure the unique index' where clause is set up correctly
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ts.A (Id) VALUES(?)"));
        for (int id = 100; id <= 1000; id += 100)
            {
            ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, id));
            ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step());
            stmt.Reset();
            stmt.ClearBindings();
            }

        stmt.Finalize();

        Utf8CP ecsql = "INSERT INTO ts.Base (Code) VALUES(?)";
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, ecsql));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "base:1", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step()) << "First execution of " << ecsql << " Error: " << db.GetLastError();
        stmt.Reset();
        stmt.ClearBindings();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "base:2", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step()) << "Second execution of " << ecsql << " Error: " << db.GetLastError();
        stmt.Finalize();

        ecsql = "INSERT INTO ts.Sub1 (Code,AId) VALUES(?,?)";
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, ecsql));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "sub1:1", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, 100));
        ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step()) << "First execution of " << ecsql << " Error: " << db.GetLastError();
        stmt.Reset();
        stmt.ClearBindings();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "sub1:2", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, 200));
        ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step()) << "Second execution of " << ecsql << " Error: " << db.GetLastError();
        stmt.Finalize();

        ecsql = "INSERT INTO ts.Sub1_1 (Code,AId,Cost) VALUES(?,?,9.99)";
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, ecsql));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "sub1_1:1", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, 300));
        ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step()) << "First execution of " << ecsql << " Error: " << db.GetLastError();
        stmt.Reset();
        stmt.ClearBindings();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "sub1_1:2", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, 400));
        ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step()) << "Second execution of " << ecsql << " Error: " << db.GetLastError();
        stmt.Finalize();

        //use same name for all Sub2 instances to test that the partial unique index on this column is excluding Sub2 rows.
        ecsql = "INSERT INTO ts.Sub2 (Code,Name) VALUES(?,'A sub2 instance')";
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, ecsql));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "sub2:1", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStepStatus::Done, stmt.Step()) << "First execution of " << ecsql << " Error: " << db.GetLastError();
        stmt.Reset();
        stmt.ClearBindings();
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "sub2:2", IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStepStatus::Error, stmt.Step()) << "Second execution of " << ecsql << " Error: " << db.GetLastError ();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaImportTestFixture, NotNullablePropertyTest)
    {
        {
        TestItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='B' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "               <Indexes>"
            "                   <DbIndex>"
            "                       <Name>ix_b_id</Name>"
            "                       <Properties>"
            "                           <string>Id</string>"
            "                       </Properties>"
            "                       <Where>ECDB_NOTNULL</Where>"
            "                   </DbIndex>"
            "               </Indexes>"
            "            </ClassMap>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='Id' typeName='long' >"
            "           <ECCustomAttributes>"
            "            <PropertyMap xmlns='ECDbMap.01.00'>"
            "               <IsNullable>false</IsNullable>"
            "            </PropertyMap>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"
            "        <ECProperty propertyName='Name' typeName='string' />"
            "    </ECClass>"
            "</ECSchema>", true, "");

        ECDb db;
        bool asserted = false;
        AssertSchemaImport(db, asserted, testItem, "notnullableproptest.ecdb");
        ASSERT_FALSE(asserted);
        AssertIndex(db, "ix_b_id", false, "ts_B", {"Id"});
        }

        {
        TestItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='B' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "               <Indexes>"
            "                   <DbIndex>"
            "                       <Name>ix_b_id_name</Name>"
            "                       <Properties>"
            "                           <string>Id</string>"
            "                           <string>Name</string>"
            "                       </Properties>"
            "                       <Where>ECDB_NOTNULL</Where>"
            "                   </DbIndex>"
            "               </Indexes>"
            "            </ClassMap>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='Id' typeName='long' >"
            "           <ECCustomAttributes>"
            "            <PropertyMap xmlns='ECDbMap.01.00'>"
            "               <IsNullable>false</IsNullable>"
            "            </PropertyMap>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"
            "        <ECProperty propertyName='Name' typeName='string' />"
            "    </ECClass>"
            "</ECSchema>", true, "");

        ECDb db;
        bool asserted = false;
        AssertSchemaImport(db, asserted, testItem, "notnullableproptest.ecdb");
        ASSERT_FALSE(asserted);
        AssertIndex(db, "ix_b_id_name", false, "ts_B", {"Id","Name"}, "[Name] IS NOT NULL");
        }

        {
        TestItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='B' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "               <Indexes>"
            "                   <DbIndex>"
            "                       <Name>ix_b_id_name</Name>"
            "                       <Properties>"
            "                           <string>Id</string>"
            "                           <string>Name</string>"
            "                       </Properties>"
            "                       <Where>ECDB_NOTNULL</Where>"
            "                   </DbIndex>"
            "               </Indexes>"
            "            </ClassMap>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='Id' typeName='long' >"
            "           <ECCustomAttributes>"
            "            <PropertyMap xmlns='ECDbMap.01.00'>"
            "               <IsNullable>false</IsNullable>"
            "            </PropertyMap>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"
            "        <ECProperty propertyName='Name' typeName='string'>"
            "           <ECCustomAttributes>"
            "            <PropertyMap xmlns='ECDbMap.01.00'>"
            "               <IsNullable>false</IsNullable>"
            "            </PropertyMap>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"
            "    </ECClass>"
            "</ECSchema>", true, "");

        ECDb db;
        bool asserted = false;
        AssertSchemaImport(db, asserted, testItem, "notnullableproptest.ecdb");
        ASSERT_FALSE(asserted);
        AssertIndex(db, "ix_b_id_name", false, "ts_B", {"Id", "Name"});
        }

        {
        TestItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='A' isDomainClass='True'>"
            "        <ECProperty propertyName='Code' typeName='string' />"
            "        <ECProperty propertyName='Id' typeName='long' />"
            "    </ECClass>"
            "    <ECClass typeName='B' isDomainClass='True'>"
            "        <ECProperty propertyName='AId' typeName='long'/>"
            "        <ECProperty propertyName='Name' typeName='string' />"
            "    </ECClass>"
            "  <ECRelationshipClass typeName='Rel' isDomainClass='True' strength='embedding'>"
            "    <Source cardinality='(0,1)' polymorphic='True'>"
            "      <Class class='A'>"
            "         <Key>"
            "           <Property name='Id'/>"
            "         </Key>"
            "       </Class>"
            "    </Source>"
            "    <Target cardinality='(0,N)' polymorphic='True'>"
            "      <Class class='B'>"
            "         <Key>"
            "           <Property name='AId'/>"
            "         </Key>"
            "       </Class>"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", true, "");

        ECDb db;
        bool asserted = false;
        AssertSchemaImport(db, asserted, testItem, "notnullableproptest.ecdb");
        ASSERT_FALSE(asserted);

        AssertIndex(db, "ix_ts_B_fk_ts_Rel_target", false, "ts_B", {"AId"}, "[AId] IS NOT NULL");
        }

        {
        TestItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='A' isDomainClass='True'>"
            "        <ECProperty propertyName='Code' typeName='string' />"
            "        <ECProperty propertyName='Id' typeName='long' />"
            "    </ECClass>"
            "    <ECClass typeName='B' isDomainClass='True'>"
            "        <ECProperty propertyName='AId' typeName='long'>"
            "           <ECCustomAttributes>"
            "            <PropertyMap xmlns='ECDbMap.01.00'>"
            "               <IsNullable>false</IsNullable>"
            "            </PropertyMap>"
            "           </ECCustomAttributes>"
            "        </ECProperty>"
            "        <ECProperty propertyName='Name' typeName='string' />"
            "    </ECClass>"
            "  <ECRelationshipClass typeName='Rel' isDomainClass='True' strength='embedding'>"
            "    <Source cardinality='(0,1)' polymorphic='True'>"
            "      <Class class='A'>"
            "         <Key>"
            "           <Property name='Id'/>"
            "         </Key>"
            "       </Class>"
            "    </Source>"
            "    <Target cardinality='(0,N)' polymorphic='True'>"
            "      <Class class='B'>"
            "         <Key>"
            "           <Property name='AId'/>"
            "         </Key>"
            "       </Class>"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", true, "");

        ECDb db;
        bool asserted = false;
        AssertSchemaImport(db, asserted, testItem, "notnullableproptest.ecdb");
        ASSERT_FALSE(asserted);

        AssertIndex(db, "ix_ts_B_fk_ts_Rel_target", false, "ts_B", {"AId"});
        }
    }
//=======================================================================================    
// @bsiclass                                   Muhammad Hassan                     05/15
//=======================================================================================    
struct RelationshipsAndSharedTablesTestFixture : SchemaImportTestFixture
    {
protected:
    static Utf8CP const SCHEMA_XML;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipsAndSharedTablesTestFixture, UniqueIndexesSupportFor1to1Relationship)
    {
    TestItem testItem(SCHEMA_XML, true);
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipsAndSharedTables.ecdb");
    ASSERT_FALSE(asserted);

    BeSQLite::Statement stmt;
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (ecdb, "SELECT Id from ec_Class where ec_Class.Name = 'BaseHasClassA'"));
    ASSERT_EQ (DbResult::BE_SQLITE_ROW, stmt.Step ());
    ECClassId classId = stmt.GetValueInt64(0);
    stmt.Finalize ();

    //verify that entry in the ec_Index table exists for relationship table BaseHasClassA
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (ecdb, "SELECT Name, IsUnique from ec_Index where ClassId = ?"));
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.BindInt64 (1, classId));
    while (DbResult::BE_SQLITE_ROW == stmt.Step ())
        {
        ASSERT_EQ (1, stmt.GetValueInt (1)) << "Index value for 1:1 Relationship is not Unique";
        Utf8String indexName = stmt.GetValueText (0);
        ASSERT_TRUE (indexName == "idx_ECRel_Source_Unique_t_BaseOwnsBase" || "idx_ECRel_Target_Unique_t_BaseOwnsBase");
        }
    stmt.Finalize ();

    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (ecdb, "SELECT Id from ec_Class where ec_Class.Name = 'BaseHasClassB'"));
    ASSERT_EQ (DbResult::BE_SQLITE_ROW, stmt.Step ());
    classId = stmt.GetValueInt64(0);
    stmt.Finalize();

    //verify that entry in ec_Index table also exists for relationship table BaseHasClassB
    ASSERT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (ecdb, "SELECT Name, IsUnique from ec_Index where ClassId = ?"));
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.BindInt64(1, classId));
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        ASSERT_EQ (1, stmt.GetValueInt (1)) << "Index value for 1:1 Relationship is not Unique";
        Utf8String indexName = stmt.GetValueText (0);
        ASSERT_TRUE (indexName == "uix_unique_t_BaseHasClassB_Source" || "uix_unique_t_BaseHasClassB_Target");
        }
    stmt.Finalize ();

    ecdb.CloseDb ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipsAndSharedTablesTestFixture, InstanceDeletionFromPolymorphicRelationships)
    {
    TestItem testItem(SCHEMA_XML, true);
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipsAndSharedTables.ecdb");
    ASSERT_FALSE(asserted);

    ASSERT_TRUE (ecdb.TableExists ("t_BaseOwnsBase"));
    ASSERT_FALSE (ecdb.TableExists ("t_BaseHasClassA"));
    ASSERT_FALSE (ecdb.TableExists ("t_BaseHasClassB"));

    ECSchemaCP schema = ecdb.Schemas ().GetECSchema ("test", true);
    ASSERT_TRUE (schema != nullptr) << "Couldn't locate test schema";

    ECClassCP baseClass = schema->GetClassCP ("Base");
    ASSERT_TRUE(baseClass != nullptr) << "Couldn't locate class Base from schema";
    ECClassCP classA = schema->GetClassCP ("ClassA");
    ASSERT_TRUE (classA != nullptr) << "Couldn't locate classA from Schema";
    ECClassCP classB = schema->GetClassCP ("ClassB");
    ASSERT_TRUE (classB != nullptr) << "Couldn't locate classB from Schema";

    //Insert Instances for class Base
    ECN::StandaloneECInstancePtr baseInstance1 = baseClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ECN::StandaloneECInstancePtr baseInstance2 = baseClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    baseInstance1->SetValue("P0", ECValue("string1"));
    baseInstance2->SetValue("P0", ECValue("string2"));

    ECInstanceInserter inserter(ecdb, *baseClass);
    ASSERT_TRUE (inserter.IsValid ());

    auto stat = inserter.Insert(*baseInstance1, true);
    ASSERT_TRUE (stat == SUCCESS);
    stat = inserter.Insert(*baseInstance2, true);
    ASSERT_TRUE (stat == SUCCESS);

    //Insert Instances for ClassA
    ECN::StandaloneECInstancePtr classAInstance1 = classA->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ECN::StandaloneECInstancePtr classAInstance2 = classA->GetDefaultStandaloneEnabler ()->CreateInstance ();

    classAInstance1->SetValue ("P1", ECValue ("string1"));
    classAInstance2->SetValue ("P1", ECValue ("string2"));

    ECInstanceInserter classAinserter (ecdb, *classA);
    ASSERT_TRUE (classAinserter.IsValid ());

    stat = classAinserter.Insert (*classAInstance1, true);
    ASSERT_TRUE (stat == SUCCESS);
    stat = classAinserter.Insert (*classAInstance2, true);
    ASSERT_TRUE (stat == SUCCESS);

    //Insert Instances for ClassB
    ECN::StandaloneECInstancePtr classBInstance1 = classB->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ECN::StandaloneECInstancePtr classBInstance2 = classB->GetDefaultStandaloneEnabler ()->CreateInstance ();

    classBInstance1->SetValue ("P2", ECValue ("string1"));
    classBInstance2->SetValue ("P2", ECValue ("string2"));

    ECInstanceInserter classBinserter (ecdb, *classB);
    ASSERT_TRUE (classBinserter.IsValid ());

    stat = classBinserter.Insert (*classBInstance1, true);
    ASSERT_TRUE (stat == SUCCESS);
    stat = classBinserter.Insert (*classBInstance2, true);
    ASSERT_TRUE (stat == SUCCESS);

    //Get Relationship Classes
    ECRelationshipClassCP baseOwnsBaseClass = schema->GetClassCP ("BaseOwnsBase")->GetRelationshipClassCP ();
    ASSERT_TRUE(baseOwnsBaseClass != nullptr);
    ECRelationshipClassCP baseHasClassAClass = schema->GetClassCP ("BaseHasClassA")->GetRelationshipClassCP ();
    ASSERT_TRUE(baseHasClassAClass != nullptr);
    ECRelationshipClassCP baseHasClassBClass = schema->GetClassCP ("BaseHasClassB")->GetRelationshipClassCP ();
    ASSERT_TRUE(baseHasClassBClass != nullptr);

        {//Insert Instances for Relationship TPHOwnsTPH
        ECN::StandaloneECRelationshipInstancePtr relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*baseOwnsBaseClass)->CreateRelationshipInstance();
        ECInstanceInserter relationshipinserter(ecdb, *baseOwnsBaseClass);
        ASSERT_TRUE (relationshipinserter.IsValid ());

            {//Inserting 1st Instance
            relationshipInstance->SetSource (baseInstance1.get ());
            relationshipInstance->SetTarget (baseInstance2.get ());
            relationshipInstance->SetInstanceId ("source->target");
            ASSERT_EQ (SUCCESS, relationshipinserter.Insert (*relationshipInstance));
            }
                {//Inserting 2nd Instance
                relationshipInstance->SetSource (baseInstance2.get ());
                relationshipInstance->SetTarget (baseInstance1.get ());
                relationshipInstance->SetInstanceId ("source->target");
                ASSERT_EQ (SUCCESS, relationshipinserter.Insert (*relationshipInstance));
                    }
        }

            {//Insert Instances for Relationship TPHhasClassA
            ECN::StandaloneECRelationshipInstancePtr relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*baseHasClassAClass)->CreateRelationshipInstance();
            ECInstanceInserter relationshipinserter(ecdb, *baseHasClassAClass);
            ASSERT_TRUE (relationshipinserter.IsValid ());

                {//Inserting 1st Instance
                relationshipInstance->SetSource (baseInstance1.get ());
                relationshipInstance->SetTarget (classAInstance1.get ());
                relationshipInstance->SetInstanceId ("source->target");
                ASSERT_EQ (SUCCESS, relationshipinserter.Insert (*relationshipInstance));
                }
                    {//Inserting 2nd Instance
                    relationshipInstance->SetSource (baseInstance2.get ());
                    relationshipInstance->SetTarget (classAInstance2.get ());
                    relationshipInstance->SetInstanceId ("source->target");
                    ASSERT_EQ (SUCCESS, relationshipinserter.Insert (*relationshipInstance));
                        }
                }

                {//Insert Instances for Relationship TPHhasClassB
                ECN::StandaloneECRelationshipInstancePtr relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*baseHasClassBClass)->CreateRelationshipInstance();
                ECInstanceInserter relationshipinserter(ecdb, *baseHasClassBClass);
                ASSERT_TRUE (relationshipinserter.IsValid ());

                    {//Inserting 1st Instance
                    relationshipInstance->SetSource(baseInstance1.get());
                    relationshipInstance->SetTarget (classBInstance1.get ());
                    relationshipInstance->SetInstanceId ("source->target");
                    ASSERT_EQ (SUCCESS, relationshipinserter.Insert (*relationshipInstance));
                    }
                        {//Inserting 2nd Instance
                        relationshipInstance->SetSource(baseInstance2.get());
                        relationshipInstance->SetTarget (classBInstance2.get ());
                        relationshipInstance->SetInstanceId ("source->target");
                        ASSERT_EQ (SUCCESS, relationshipinserter.Insert (*relationshipInstance));
                            }
            }
    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT COUNT(*) FROM t.Base"));
    ASSERT_TRUE (ECSqlStepStatus::HasRow == stmt.Step ());
    EXPECT_EQ (6, stmt.GetValueInt (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE (ECSqlStepStatus::HasRow == stmt.Step ());
    EXPECT_EQ (6, stmt.GetValueInt (0));
    stmt.Finalize ();

    //Deletes the instances of BaseOwnsBase class..
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "DELETE FROM ONLY t.BaseOwnsBase"));
    ASSERT_TRUE (ECSqlStepStatus::Done == stmt.Step ());
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE (ECSqlStepStatus::HasRow == stmt.Step ());
    EXPECT_EQ (4, stmt.GetValueInt (0));
    stmt.Finalize ();

    //Deletes the instances of BaseHasClassA class..
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "DELETE FROM ONLY t.BaseHasClassA"));
    ASSERT_TRUE (ECSqlStepStatus::Done == stmt.Step ());
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE (ECSqlStepStatus::HasRow == stmt.Step ());
    EXPECT_EQ (2, stmt.GetValueInt (0));
    stmt.Finalize ();

    //Deletes the instances of BaseHasClassB class..
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "DELETE FROM ONLY t.BaseHasClassB"));
    ASSERT_TRUE (ECSqlStepStatus::Done == stmt.Step ());
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE (ECSqlStepStatus::HasRow == stmt.Step ());
    EXPECT_EQ (0, stmt.GetValueInt (0));
    stmt.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipsAndSharedTablesTestFixture, RetrieveConstraintClassInstanceBeforeAfterInsertingRelationshipInstance)
    {
    TestItem testItem(SCHEMA_XML, true);
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipsAndSharedTables.ecdb");
    ASSERT_FALSE(asserted);


    ASSERT_TRUE(ecdb.TableExists("t_BaseOwnsBase"));
    ASSERT_FALSE (ecdb.TableExists ("t_BaseHasClassA"));
    ASSERT_FALSE (ecdb.TableExists ("t_BaseHasClassB"));

    ECSqlStatement insertStatement;
    ECInstanceKey TPHKey1;
    ECInstanceKey TPHKey2;
    ASSERT_EQ (ECSqlStatus::Success, insertStatement.Prepare (ecdb, "INSERT INTO t.Base (P0) VALUES ('string1')"));
    ASSERT_EQ (ECSqlStepStatus::Done, insertStatement.Step (TPHKey1));
    ASSERT_TRUE (TPHKey1.IsValid ());
    insertStatement.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, insertStatement.Prepare (ecdb, "INSERT INTO t.Base (P0) VALUES ('string2')"));
    ASSERT_EQ (ECSqlStepStatus::Done, insertStatement.Step (TPHKey2));
    ASSERT_TRUE (TPHKey2.IsValid ());
    insertStatement.Finalize ();

    ECInstanceKey classAKey1;
    ECInstanceKey classAKey2;
    ASSERT_EQ (ECSqlStatus::Success, insertStatement.Prepare (ecdb, "INSERT INTO t.ClassA (P1) VALUES ('string1')"));
    ASSERT_EQ (ECSqlStepStatus::Done, insertStatement.Step (classAKey1));
    ASSERT_TRUE (classAKey1.IsValid ());
    insertStatement.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, insertStatement.Prepare (ecdb, "INSERT INTO t.ClassA (P1) VALUES ('string2')"));
    ASSERT_EQ (ECSqlStepStatus::Done, insertStatement.Step (classAKey2));
    ASSERT_TRUE (classAKey2.IsValid ());
    insertStatement.Finalize ();

    //retrieve ECInstance from Db before inserting Relationship Instance, based on ECInstanceId, verify ECInstance is valid
    ECSqlStatement selectStmt;
    ASSERT_EQ (ECSqlStatus::Success, selectStmt.Prepare (ecdb, "SELECT * FROM t.Base WHERE ECInstanceId = ?"));
    selectStmt.BindId (1, TPHKey1.GetECInstanceId ());
    ASSERT_EQ (ECSqlStepStatus::HasRow, selectStmt.Step ());
    ECInstanceECSqlSelectAdapter TPHadapter (selectStmt);
    IECInstancePtr readInstance = TPHadapter.GetInstance ();
    ASSERT_TRUE (readInstance.IsValid ());
    selectStmt.Finalize ();

    ECSqlStatement relationStmt;
    ASSERT_EQ (relationStmt.Prepare (ecdb, "INSERT INTO t.BaseHasClassA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    relationStmt.BindId (1, TPHKey1.GetECInstanceId ());
    relationStmt.BindInt64 (2, TPHKey1.GetECClassId ());
    relationStmt.BindId (3, classAKey1.GetECInstanceId ());
    relationStmt.BindInt64 (4, classAKey1.GetECClassId ());
    ASSERT_EQ (relationStmt.Step (), ECSqlStepStatus::Done);
    relationStmt.Finalize ();

    //try to insert Duplicate relationship step() should return error
    ASSERT_EQ (relationStmt.Prepare (ecdb, "INSERT INTO t.BaseHasClassA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    relationStmt.BindId (1, TPHKey1.GetECInstanceId ());
    relationStmt.BindInt64 (2, TPHKey1.GetECClassId ());
    relationStmt.BindId (3, classAKey1.GetECInstanceId ());
    relationStmt.BindInt64 (4, classAKey1.GetECClassId ());
    ASSERT_EQ (relationStmt.Step (), ECSqlStepStatus::Error);
    relationStmt.Finalize ();

    //retrieve ECInstance from Db After Inserting Relationship Instance, based on ECInstanceId, verify ECInstance is valid
    ASSERT_EQ (ECSqlStatus::Success, selectStmt.Prepare (ecdb, "SELECT * FROM t.ClassA WHERE ECInstanceId = ?"));
    selectStmt.BindId (1, classAKey1.GetECInstanceId ());
    ASSERT_EQ (ECSqlStepStatus::HasRow, selectStmt.Step ());
    ECInstanceECSqlSelectAdapter ClassAadapter (selectStmt);
    readInstance = ClassAadapter.GetInstance ();
    ASSERT_TRUE (readInstance.IsValid ());
    selectStmt.Finalize ();

    ecdb.CloseDb ();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const RelationshipsAndSharedTablesTestFixture::SCHEMA_XML =
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='test' nameSpacePrefix='t' version='1.0' description='Schema covers all the cases in which base class is TablePerHierarchy' displayLabel='Table Per Hierarchy' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "<ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "<ECClass typeName='Base' isDomainClass='True'>"
                "<ECCustomAttributes>"
                    "<ClassMap xmlns='ECDbMap.01.00'>"
                    "<MapStrategy>"
                        "<Strategy>SharedTable</Strategy>"
                        "<IsPolymorphic>True</IsPolymorphic>"
                    "</MapStrategy>"
                    "</ClassMap>"
                "</ECCustomAttributes>"
                "<ECProperty propertyName='P0' typeName='string' />"
            "</ECClass>"
            "<ECClass typeName='ClassA' isDomainClass='True'>"
                "<BaseClass>Base</BaseClass>"
                "<ECProperty propertyName='P1' typeName='string' />"
            "</ECClass>"
            "<ECClass typeName='ClassB' isDomainClass='True'>"
                "<BaseClass>ClassA</BaseClass>"
                "<ECProperty propertyName='P2' typeName='string' />"
            "</ECClass>"
            "<ECRelationshipClass typeName='BaseOwnsBase' isDomainClass='True' strength='referencing' strengthDirection='forward'>"
                "<ECCustomAttributes>"
                    "<ClassMap xmlns='ECDbMap.01.00'>"
                        "<MapStrategy>"
                        "<Strategy>SharedTable</Strategy>"
                        "<IsPolymorphic>True</IsPolymorphic>"
                        "</MapStrategy>"
                    "</ClassMap>"
                "</ECCustomAttributes>"
                "<Source cardinality='(0,N)' polymorphic='True'>"
                    "<Class class='Base' />"
                "</Source>"
                "<Target cardinality='(0,N)' polymorphic='True'>"
                    "<Class class='Base' />"
                "</Target>"
            "</ECRelationshipClass>"
            "<ECRelationshipClass typeName='BaseHasClassA' isDomainClass='True' strength='referencing' strengthDirection='forward'>"
                "<BaseClass>BaseOwnsBase</BaseClass>"
                "<Source cardinality='(0,1)' polymorphic='True'>"
                    "<Class class='Base' />"
                "</Source>"
                "<Target cardinality='(0,1)' polymorphic='True'>"
                    "<Class class='ClassA' />"
                "</Target>"
            "</ECRelationshipClass>"
            "<ECRelationshipClass typeName='BaseHasClassB' isDomainClass='True' strength='referencing' strengthDirection='forward'>"
                "<BaseClass>BaseOwnsBase</BaseClass>"
                "<Source cardinality='(0,1)' polymorphic='True'>"
                    "<Class class='Base' />"
                "</Source>"
                "<Target cardinality='(0,1)' polymorphic='True'>"
                    "<Class class='ClassB' />"
                "</Target>"
            "</ECRelationshipClass>"
        "</ECSchema>";

//=======================================================================================    
// @bsiclass                                   Muhammad Hassan                     05/15
//=======================================================================================    
struct ReferentialIntegrityTestFixture : SchemaImportTestFixture
    {
protected:
    //This is a mirror of the internal MapStrategy used by ECDb and persisted in the DB.
    //The values can change, so in that case this struct needs to be updated accordingly.
    struct ResolvedMapStrategy
        {
        enum class Strategy
            {
            NotMapped,
            OwnTable,
            SharedTable,
            ExistingTable,

            ForeignKeyRelationshipInTargetTable = 100,
            ForeignKeyRelationshipInSourceTable = 101
            };

        enum class Options
            {
            None = 0,
            Readonly = 1,
            SharedColumns = 2
            };

        Strategy m_strategy;
        Options m_options;
        bool m_isPolymorphic;

        ResolvedMapStrategy() : m_strategy(Strategy::NotMapped), m_options(Options::None), m_isPolymorphic(false) {}
        ResolvedMapStrategy(Strategy strategy, Options options, bool isPolymorphic) : m_strategy(strategy), m_options(options), m_isPolymorphic(isPolymorphic) {}
        };

private:
    void VerifyRelationshipInsertionIntegrity(ECDbCR ecdb, Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<ECSqlStepStatus> const& expected, size_t& rowInserted) const;
    size_t GetRelationshipInstanceCount(ECDbCR ecdb, Utf8CP relationshipClass) const;
    ResolvedMapStrategy GetMapStrategy(ECDbR ecdb, ECClassId ClassId) const;

protected:
    void ExecuteRelationshipInsertionIntegrityTest(ECDbR ecdb, bool allowDuplicateRelationships, bool allowForeignKeyConstraint, bool schemaImportExpectedToSucceed) const;

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ReferentialIntegrityTestFixture, ForeignKeyRelationshipMap_EnforceReferentialIntegrity)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("ForeignKeyRelationshipMap_EnforceReferentialIntegrity.ecdb");
    ExecuteRelationshipInsertionIntegrityTest (ecdb, false, true, true);
    //when AllowDuplicate is turned of, OneFooHasManyGoo will also be mapped as endtable therefore ReferentialIntegrityCheck will be performed for it, so there will be two rows in the ForeignKey table
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasOneGoo"));
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasManyGoo"));

    BeSQLite::Statement sqlStatment;
    auto stat = sqlStatment.Prepare (ecdb, "SELECT ec_Column.Name FROM ec_Column JOIN ec_ForeignKey ON ec_ForeignKey.TableId = ec_Column.[TableId] JOIN ec_ForeignKeyColumn ON ec_ForeignKeyColumn.ColumnId = ec_Column.Id WHERE ec_ForeignKey.Id = 1");
    ASSERT_EQ (stat, DbResult::BE_SQLITE_OK);
    size_t rowCount = 0;
    while (sqlStatment.Step () != DbResult::BE_SQLITE_DONE)
        {
        rowCount++;
        }
    ASSERT_EQ (2, rowCount);

    sqlStatment.Finalize ();
    ecdb.CloseDb ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         04/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ReferentialIntegrityTestFixture, ForeignKeyConstraint_EnforceReferentialIntegrityCheck_AllowDuplicateRelation)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("ForeignKeyConstraint_EnforceReferentialIntegrityCheck_AllowDuplicateRelation.ecdb");
    ExecuteRelationshipInsertionIntegrityTest (ecdb, true, true, true);
    //when AllowDuplicate is turned on, OneFooHasManyGoo will also be mapped as endtable therefore there will be only one row in the ForeignKey table
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasOneGoo"));
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasManyGoo"));

    BeSQLite::Statement sqlStatment;
    auto stat = sqlStatment.Prepare (ecdb, "SELECT ec_Column.[Name] FROM ec_Column JOIN ec_ForeignKey ON ec_ForeignKey.[TableId] = ec_Column.[TableId] JOIN ec_ForeignKeyColumn ON ec_ForeignKeyColumn.[ColumnId] = ec_Column.[Id] WHERE ec_ForeignKey.[Id] = 1");
    ASSERT_EQ (stat, DbResult::BE_SQLITE_OK);
    size_t rowCount = 0;
    while (sqlStatment.Step () != DbResult::BE_SQLITE_DONE)
        {
        rowCount++;
        }
    ASSERT_EQ (2, rowCount);

    sqlStatment.Finalize ();
    ecdb.CloseDb ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         02/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ReferentialIntegrityTestFixture, RelationshipTest_DoNotAllowDuplicateRelationships)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("RelationshipCardinalityTest.ecdb");
    ExecuteRelationshipInsertionIntegrityTest (ecdb, false, false, true);
    ASSERT_TRUE (ecdb.TableExists ("ts_Foo"));
    ASSERT_TRUE (ecdb.TableExists ("ts_Goo"));
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasOneGoo"));
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasManyGoo"));
    ASSERT_TRUE (ecdb.TableExists ("ts_ManyFooHasManyGoo"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         02/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ReferentialIntegrityTestFixture, RelationshipTest_AllowDuplicateRelationships)
    {
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("RelationshipCardinalityTest_AllowDuplicateRelationships.ecdb");
    ExecuteRelationshipInsertionIntegrityTest (ecdb, true, false, true);
    ASSERT_TRUE (ecdb.TableExists ("ts_Foo"));
    ASSERT_TRUE (ecdb.TableExists ("ts_Goo"));
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasOneGoo"));
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasManyGoo"));
    ASSERT_TRUE (ecdb.TableExists ("ts_ManyFooHasManyGoo"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ReferentialIntegrityTestFixture::VerifyRelationshipInsertionIntegrity(ECDbCR ecdb, Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<ECSqlStepStatus> const& expected, size_t& rowInserted) const
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
            ASSERT_EQ(static_cast<int>(stmt.ClearBindings()), static_cast<int>(ECSqlStatus::Success));
            stmt.BindId(1, fooKey.GetECInstanceId());
            stmt.BindInt64(2, fooKey.GetECClassId());
            stmt.BindId(3, gooKey.GetECInstanceId());
            stmt.BindInt64(4, gooKey.GetECClassId());
            ASSERT_EQ(static_cast<int>(stmt.Step()), static_cast<int>(expected[n]));
            if (expected[n] == ECSqlStepStatus::Done)
                rowInserted++;

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
        if (stmt.Step() == ECSqlStepStatus::HasRow)
            return static_cast<size_t>(stmt.GetValueInt(0));
        }

    return 0;
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         02/15
+---------------+---------------+---------------+---------------+---------------+------*/
ReferentialIntegrityTestFixture::ResolvedMapStrategy ReferentialIntegrityTestFixture::GetMapStrategy(ECDbR ecdb, ECClassId ClassId) const
        {
        Statement stmt;
        stmt.Prepare (ecdb, "SELECT MapStrategy, MapStrategyOptions, IsMapStrategyPolymorphic FROM ec_ClassMap WHERE ClassId = ?");
        stmt.BindInt64 (1, ClassId);
        if (stmt.Step() == BE_SQLITE_ROW)
            {
            ResolvedMapStrategy::Strategy strat = (ResolvedMapStrategy::Strategy) stmt.GetValueInt(0);
            ResolvedMapStrategy::Options options = stmt.IsColumnNull(1) ? ResolvedMapStrategy::Options::None : (ResolvedMapStrategy::Options) stmt.GetValueInt(1);
            bool isPolymorphic = stmt.GetValueInt(2) != 0;
            return ResolvedMapStrategy(strat, options, isPolymorphic);
            }

        return ResolvedMapStrategy();
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                         02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ReferentialIntegrityTestFixture::ExecuteRelationshipInsertionIntegrityTest(ECDbR ecdb, bool allowDuplicateRelationships, bool allowForeignKeyConstraint, bool schemaImportExpectedToSucceed) const
    {
    ECSchemaPtr testSchema;
    ECClassP foo = nullptr, goo = nullptr;
    ECRelationshipClassP oneFooHasOneGoo = nullptr, oneFooHasManyGoo = nullptr, manyFooHasManyGoo = nullptr;
    PrimitiveECPropertyP prim;
    auto readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(ecdb.GetSchemaLocater());
    auto ecdbmapKey = SchemaKey("ECDbMap", 1, 0);
    auto ecdbmapSchema = readContext->LocateSchema(ecdbmapKey, SchemaMatchType::SCHEMAMATCHTYPE_LatestCompatible);
    ASSERT_TRUE(ecdbmapSchema.IsValid());

    ECSchema::CreateSchema(testSchema, "TestSchema", 1, 0);
    ASSERT_TRUE(testSchema.IsValid());

    testSchema->SetNamespacePrefix("ts");
    testSchema->AddReferencedSchema(*ecdbmapSchema);

    testSchema->CreateClass(foo, "Foo");
    testSchema->CreateClass(goo, "Goo");

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
    oneFooHasOneGoo->GetSource().SetCardinality("1");
    oneFooHasOneGoo->GetTarget().AddClass(*goo);
    oneFooHasOneGoo->GetTarget().SetCardinality("1");

    oneFooHasManyGoo->GetSource().AddClass(*foo);
    oneFooHasManyGoo->GetSource().SetCardinality("1");
    oneFooHasManyGoo->GetTarget().AddClass(*goo);
    oneFooHasManyGoo->GetTarget().SetCardinality("N");

    manyFooHasManyGoo->GetSource().AddClass(*foo);
    manyFooHasManyGoo->GetSource().SetCardinality("N");
    manyFooHasManyGoo->GetTarget().AddClass(*goo);
    manyFooHasManyGoo->GetTarget().SetCardinality("N");
    Backdoor::ECObjects::ECSchemaReadContext::AddSchema(*readContext, *testSchema);

    if (allowDuplicateRelationships)
        {
        auto caInstClass = ecdbmapSchema->GetClassCP("LinkTableRelationshipMap");
        ASSERT_TRUE(caInstClass != nullptr);
        auto caInst = caInstClass->GetDefaultStandaloneEnabler()->CreateInstance();
        ASSERT_TRUE(caInst != nullptr);
        ASSERT_TRUE(caInst->SetValue("AllowDuplicateRelationships", ECValue(true)) == ECOBJECTS_STATUS_Success);
        ASSERT_TRUE(manyFooHasManyGoo->SetCustomAttribute(*caInst) == ECOBJECTS_STATUS_Success);
        }

    if (allowForeignKeyConstraint)
        {
        auto fkMapClass = ecdbmapSchema->GetClassCP("ForeignKeyRelationshipMap");
        ASSERT_TRUE(fkMapClass != nullptr);
        auto caInst = fkMapClass->GetDefaultStandaloneEnabler()->CreateInstance();
        ASSERT_TRUE(caInst != nullptr);
        const Utf8CP enforceReferentialIntegrityProperty = "CreateConstraint";
        ASSERT_TRUE(caInst->SetValue(enforceReferentialIntegrityProperty, ECValue(true)) == ECOBJECTS_STATUS_Success);
        ASSERT_TRUE(oneFooHasOneGoo->SetCustomAttribute(*caInst) == ECOBJECTS_STATUS_Success);
        ASSERT_TRUE(oneFooHasManyGoo->SetCustomAttribute(*caInst) == ECOBJECTS_STATUS_Success);
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
        ASSERT_EQ(fooStmt.Step(out), ECSqlStepStatus::Done);
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
        ASSERT_EQ(gooStmt.Step(out), ECSqlStepStatus::Done);
        gooKeys.push_back(out);
        }

    //Compute what are the right valid permutation
    std::vector<ECSqlStepStatus> oneFooHasOneGooResult;
    std::vector<ECSqlStepStatus> oneFooHasManyGooResult;
    std::vector<ECSqlStepStatus> manyFooHasManyGooResult;
    std::vector<ECSqlStepStatus> reinsertResultError;
    std::vector<ECSqlStepStatus> reinsertResultDone;
    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //1:1 is not effected with AllowDuplicateRelationships
            if (f == g)
                oneFooHasOneGooResult.push_back(ECSqlStepStatus::Done);
            else
                oneFooHasOneGooResult.push_back(ECSqlStepStatus::Error);

            //1:N is effected with AllowDuplicateRelationships
            if (f == 0)
                oneFooHasManyGooResult.push_back(ECSqlStepStatus::Done);
            else
                oneFooHasManyGooResult.push_back(ECSqlStepStatus::Error);

            manyFooHasManyGooResult.push_back(ECSqlStepStatus::Done);
            reinsertResultError.push_back(ECSqlStepStatus::Error);
            reinsertResultDone.push_back(ECSqlStepStatus::Done);
            }
        }

    //1:1--------------------------------
    size_t count_OneFooHasOneGoo = 0;
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.OneFooHasOneGoo", fooKeys, gooKeys, oneFooHasOneGooResult, count_OneFooHasOneGoo);
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.OneFooHasOneGoo", fooKeys, gooKeys, reinsertResultError, count_OneFooHasOneGoo);

    ResolvedMapStrategy mapStrategy = GetMapStrategy(ecdb, oneFooHasOneGoo->GetId());
    ASSERT_EQ(ResolvedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);
    ASSERT_EQ(count_OneFooHasOneGoo, GetRelationshipInstanceCount(ecdb, "ts.OneFooHasOneGoo"));

    //1:N--------------------------------
    size_t count_OneFooHasManyGoo = 0;
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.OneFooHasManyGoo", fooKeys, gooKeys, oneFooHasManyGooResult, count_OneFooHasManyGoo);

    mapStrategy = GetMapStrategy(ecdb, oneFooHasManyGoo->GetId());
    ASSERT_EQ(ResolvedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);
    ASSERT_EQ(count_OneFooHasManyGoo, GetRelationshipInstanceCount(ecdb, "ts.OneFooHasManyGoo"));

    //N:N--------------------------------
    size_t count_ManyFooHasManyGoo = 0;
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, manyFooHasManyGooResult, count_ManyFooHasManyGoo);
    if (allowDuplicateRelationships)
        VerifyRelationshipInsertionIntegrity(ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, reinsertResultDone, count_ManyFooHasManyGoo);
    else
        VerifyRelationshipInsertionIntegrity(ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, reinsertResultError, count_ManyFooHasManyGoo);

    mapStrategy = GetMapStrategy(ecdb, manyFooHasManyGoo->GetId());

    ASSERT_EQ(ResolvedMapStrategy::Strategy::OwnTable, mapStrategy.m_strategy);
    ASSERT_EQ(ResolvedMapStrategy::Options::None, mapStrategy.m_options);
    ASSERT_FALSE(mapStrategy.m_isPolymorphic);
    ASSERT_EQ(count_ManyFooHasManyGoo, GetRelationshipInstanceCount(ecdb, "ts.ManyFooHasManyGoo"));
    }

END_ECDBUNITTESTS_NAMESPACE