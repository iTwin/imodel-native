/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbMapping_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "SchemaImportTestFixture.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================    
// @bsiclass                                   Krischan.Eberle                  10/15
//=======================================================================================    
struct ECDbMappingTestFixture : SchemaImportTestFixture
    {
protected:
    //This is a mirror of the internal MapStrategy used by ECDb and persisted in the DB.
    //The values can change, so in that case this struct needs to be updated accordingly.
    struct PersistedMapStrategy
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
            SharedColumns = 1,
            ParentOfJoinedTable = 2,
            JoinedTable = 4
            };

        Strategy m_strategy;
        Options m_options;
        bool m_appliesToSubclasses;

        PersistedMapStrategy() : m_strategy(Strategy::NotMapped), m_options(Options::None), m_appliesToSubclasses(false) {}
        PersistedMapStrategy(Strategy strategy, Options options, bool appliesToSubclasses) : m_strategy(strategy), m_options(options), m_appliesToSubclasses(appliesToSubclasses) {}
        };

    //---------------------------------------------------------------------------------
    // @bsimethod                                   Affan.Khan                         02/15
    //+---------------+---------------+---------------+---------------+---------------+------
    bool TryGetPersistedMapStrategy(PersistedMapStrategy& strategy, ECDbCR ecdb, ECClassId classId) const
        {
        CachedStatementPtr stmt = ecdb.GetCachedStatement("SELECT MapStrategy, MapStrategyOptions, MapStrategyAppliesToSubclasses FROM ec_ClassMap WHERE ClassId = ?");
        EXPECT_TRUE(stmt != nullptr);

        stmt->BindInt64(1, classId);
        if (BE_SQLITE_ROW == stmt->Step())
            {
            const PersistedMapStrategy::Strategy strat = (PersistedMapStrategy::Strategy) stmt->GetValueInt(0);
            const PersistedMapStrategy::Options options = (PersistedMapStrategy::Options) stmt->GetValueInt(1);
            const bool appliesToSubclasses = stmt->GetValueInt(2) == 1;
            strategy = PersistedMapStrategy(strat, options, appliesToSubclasses);
            return true;
            }

        return false;
        }

public:
    virtual ~ECDbMappingTestFixture() {}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, ECDbMapTests)
    {
    std::vector<SchemaItem> testItems {

    SchemaItem (
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
    "</ECSchema>", false, "Invalid MapStrategy"),

    SchemaItem ("<?xml version='1.0' encoding='utf-8'?>"
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
    "</ECSchema>", false, "Option SharedColumns can only be used with strategy SharedTable"),

        SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                 "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                 "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                 "    <ECClass typeName='ClassA' isDomainClass='True'>"
                 "        <ECCustomAttributes>"
                 "            <ClassMap xmlns='ECDbMap.01.00'>"
                 "                <MapStrategy>"
                 "                   <Options>SharedColumns</Options>"
                 "                </MapStrategy>"
                 "            </ClassMap>"
                 "        </ECCustomAttributes>"
                 "        <ECProperty propertyName='Price' typeName='double' />"
                 "    </ECClass>"
                 "</ECSchema>", false, "Option SharedColumns cannot be used without a strategy"),

        SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                 "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                 "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                 "    <ECClass typeName='ClassA' isDomainClass='True'>"
                 "        <ECCustomAttributes>"
                 "            <ClassMap xmlns='ECDbMap.01.00'>"
                 "                <MapStrategy>"
                 "                   <Options>JoinedTableForSubclasses</Options>"
                 "                </MapStrategy>"
                 "            </ClassMap>"
                 "        </ECCustomAttributes>"
                 "        <ECProperty propertyName='Price' typeName='double' />"
                 "    </ECClass>"
                 "</ECSchema>", false, "Option JoinedTableForSubclasses cannot be used without a strategy"),

        SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                 "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                 "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                 "    <ECClass typeName='ClassA' isDomainClass='True'>"
                 "        <ECCustomAttributes>"
                 "            <ClassMap xmlns='ECDbMap.01.00'>"
                 "                <MapStrategy>"
                 "                   <Strategy>SharedTable</Strategy>"
                 "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
                 "                   <Options>JoinedTableForSubclasses</Options>"
                 "                </MapStrategy>"
                 "            </ClassMap>"
                 "        </ECCustomAttributes>"
                 "        <ECProperty propertyName='Price' typeName='double' />"
                 "    </ECClass>"
                 "</ECSchema>", true, "Option JoinedTableForSubclasses is expected to work with strategy SharedTable (applied to subclasses)"),

        SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                 "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                 "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                 "    <ECClass typeName='ClassA' isDomainClass='True'>"
                 "        <ECCustomAttributes>"
                 "            <ClassMap xmlns='ECDbMap.01.00'>"
                 "                <MapStrategy>"
                 "                   <Strategy>SharedTable</Strategy>"
                 "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
                 "                   <Options>SharedColumnsForSubclasses, JoinedTableForSubclasses</Options>"
                 "                </MapStrategy>"
                 "            </ClassMap>"
                 "        </ECCustomAttributes>"
                 "        <ECProperty propertyName='Price' typeName='double' />"
                 "    </ECClass>"
                 "</ECSchema>", true, "Option JoinedTableForSubclasses is expected to work with strategy SharedTable (applied to subclasses) and with SharedColumnsForSubclasses"),

    SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='ClassA' isDomainClass='True'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.01.00'>"
                "                <MapStrategy>"
                "                   <Strategy>SharedTable</Strategy>"
                "                   <AppliesToSubclasses>False</AppliesToSubclasses>"
                "                   <Options>JoinedTableForSubclasses</Options>"
                "                </MapStrategy>"
                "            </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='Price' typeName='double' />"
                "    </ECClass>"
                "</ECSchema>", false, "Option JoinedTableForSubclasses can only be used with strategy SharedTable (applied to subclasses)"),

    SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='ClassA' isDomainClass='True'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.01.00'>"
                "                <MapStrategy>"
                "                   <Strategy>OwnTable</Strategy>"
                "                   <Options>JoinedTableForSubclasses</Options>"
                "                </MapStrategy>"
                "            </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='Price' typeName='double' />"
                "    </ECClass>"
                "</ECSchema>", false, "Option JoinedTableForSubclasses can only be used with strategy SharedTable (applied to subclasses)"),

    SchemaItem ("<?xml version='1.0' encoding='utf-8'?>"
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

    SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='ClassA' isDomainClass='True'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.01.00'>"
                "                <MapStrategy>"
                "                   <Strategy>ExistingTable</Strategy>"
                "                   <Options>JoinedTableForSubclasses</Options>"
                "                </MapStrategy>"
                "            </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='Price' typeName='double' />"
                "    </ECClass>"
                "</ECSchema>", false, "Option JoinedTableForSubclasses can only be used with strategy SharedTable (applied to subclasses)"),

    SchemaItem ("<?xml version='1.0' encoding='utf-8'?>"
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

    SchemaItem ("<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Class' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>OwnTable</Strategy>"
    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
    "                </MapStrategy>"
    "                <TableName>bla</TableName>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "</ECSchema>", false, "MapStrategy OwnTable doesn't allow TableName to be set."),

    SchemaItem ("<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Class' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>OwnTable</Strategy>"
    "                   <AppliesToSubclasses>False</AppliesToSubclasses>"
    "                </MapStrategy>"
    "                <TableName>bla</TableName>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "</ECSchema>", false, "MapStrategy OwnTable doesn't allow TableName to be set."),

    SchemaItem ("<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Class' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
    "                </MapStrategy>"
    "                <TableName>bla</TableName>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "</ECSchema>", false, "MapStrategy SharedTable (applies to subclasses) doesn't allow TableName to be set."),

    SchemaItem ("<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Class' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <AppliesToSubclasses>False</AppliesToSubclasses>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "</ECSchema>", false, "MapStrategy SharedTable, non-polymorphic expects TableName to be set."),

    SchemaItem ("<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Class' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <AppliesToSubclasses>False</AppliesToSubclasses>"
    "                </MapStrategy>"
    "                <TableName>idontexistyet</TableName>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "</ECSchema>", true, "MapStrategy SharedTable, non-polymorphic doesn't expect table specified in TableName to be set."),

    SchemaItem ("<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Class' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>NotMapped</Strategy>"
    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
    "                </MapStrategy>"
    "                <TableName>bla</TableName>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "</ECSchema>", false, "MapStrategy NotMapped, polymorphic doesn't allow TableName to be set."),

    SchemaItem ("<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Class' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>NotMapped</Strategy>"
    "                   <AppliesToSubclasses>False</AppliesToSubclasses>"
    "                </MapStrategy>"
    "                <TableName>bla</TableName>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "</ECSchema>", false, "MapStrategy NotMapped, non-polymorphic doesn't allow TableName to be set."),

    SchemaItem ("<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Base' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
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
    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <BaseClass>Sub</BaseClass>"
    "        <ECProperty propertyName='P2' typeName='int' />"
    "    </ECClass>"
    "</ECSchema>", false, "MapStrategy SharedTable (polymorphic) on child class where base has SharedTable (polymorphic) is not supported."),

    SchemaItem ("<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Base1' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
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
    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
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

    SchemaItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Base' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>SharedTable</Strategy>"
    "                  <AppliesToSubclasses>True</AppliesToSubclasses>"
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
    "                  <AppliesToSubclasses>False</AppliesToSubclasses>"
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

        SchemaItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='Base' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                  <Strategy>SharedTable</Strategy>"
            "                  <AppliesToSubclasses>True</AppliesToSubclasses>"
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
            "                <Options>JoinedTableForSubclasses</Options>"
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
            "</ECSchema>", false, "Option JoinedTableForSubclasses cannot be applied to subclass where base has SharedTable (applies to subclasses)."),

    SchemaItem (
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
    "                  <AppliesToSubclasses>False</AppliesToSubclasses>"
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


    SchemaItem ("<?xml version='1.0' encoding='utf-8'?>"
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

    SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='A' isDomainClass='True'>"
                "        <ECProperty propertyName='AA' typeName='double' />"
                "    </ECClass>"
                "    <ECClass typeName='B' isDomainClass='True'>"
                "        <ECProperty propertyName='BB' typeName='double' />"
                "    </ECClass>"
                "    <ECRelationshipClass typeName='Rel' isDomainClass='True' strength='embedding'>"
                "        <ECCustomAttributes>"
                "            <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
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
                "</ECSchema>", true, "Cascading delete only supported for embedding relationships"),

    SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                "</ECSchema>", false, "Cascading delete only supported for embedding relationships"),

    SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='A' isDomainClass='True'>"
                "        <ECProperty propertyName='AA' typeName='double' />"
                "    </ECClass>"
                "    <ECClass typeName='B' isDomainClass='True'>"
                "        <ECProperty propertyName='BB' typeName='double' />"
                "    </ECClass>"
                "    <ECRelationshipClass typeName='Rel' isDomainClass='True' strength='holding'>"
                "        <ECCustomAttributes>"
                "            <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
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
                "</ECSchema>", false, "Cascading delete only supported for embedding relationships"),

    SchemaItem ("<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TeststructClassInPolymorphicSharedTable' nameSpacePrefix='tph' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='BaseClass' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
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

    SchemaItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='ClassA' isDomainClass='True'>"
    "    </ECClass>"
    "</ECSchema>", true, " A class expects a property to be set."),

    SchemaItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Parent' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                   <Strategy>SharedTable</Strategy>"
    "                   <Options>SharedColumns</Options>"
    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
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
    "                 </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='Price' typeName='double' />"
    "    </ECClass>"
    "</ECSchema>", true, ""),

    SchemaItem (
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
    "                  <AppliesToSubclasses>true</AppliesToSubclasses>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <BaseClass>Sub</BaseClass>"
    "        <ECProperty propertyName='P2' typeName='int' />"
    "    </ECClass>"
    "</ECSchema>", true, "NotMapped(polymorphic) within Class Hierarchy is expected to be supported where Root class has default MapStrategy"),

    SchemaItem (
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
    "                  <Strategy>SharedTable</Strategy>"
    "                  <AppliesToSubclasses>true</AppliesToSubclasses>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <BaseClass>Sub</BaseClass>"
    "        <ECProperty propertyName='P2' typeName='int' />"
    "    </ECClass>"
    "</ECSchema>", true, "SharedTable(polymorphic) within Class Hierarchy is expected to be supported where Root class has default MapStrategy"),

    SchemaItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Base' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>NotMapped</Strategy>"
    "                  <AppliesToSubclasses>true</AppliesToSubclasses>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='P0' typeName='int' />"
    "    </ECClass>"
    "    <ECClass typeName='Sub' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>SharedTable</Strategy>"
    "                  <AppliesToSubclasses>true</AppliesToSubclasses>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <BaseClass>Base</BaseClass>"
    "        <ECProperty propertyName='P1' typeName='int' />"
    "    </ECClass>"
    "</ECSchema>", false, "Conflicting mapStrategies SharedTable(polymorphic) within Class Hierarchy not supported where Root has Strategy NotMapped(Polymorphic)"),

    SchemaItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Base' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>NotMapped</Strategy>"
    "                  <AppliesToSubclasses>true</AppliesToSubclasses>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='P0' typeName='int' />"
    "    </ECClass>"
    "    <ECClass typeName='Sub' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>OwnTable</Strategy>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <BaseClass>Base</BaseClass>"
    "        <ECProperty propertyName='P1' typeName='int' />"
    "    </ECClass>"
    "</ECSchema>", false, "Conflicting mapStrategies OwnTable within Class Hierarchy not supported where Root has MapStrategy NotMapped(Polymorphic)"),

    SchemaItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Base' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>NotMapped</Strategy>"
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
    "                  <AppliesToSubclasses>true</AppliesToSubclasses>"
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
    "</ECSchema>", true, "NotMapped (polymorphic) within Class Hierarchy is expected to be supported where Root has Strategy NotMapped"),

    SchemaItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Base' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>NotMapped</Strategy>"
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
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <BaseClass>Sub</BaseClass>"
    "        <ECProperty propertyName='P2' typeName='int' />"
    "    </ECClass>"
    "    <ECClass typeName='SubSubSub' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>NotMapped</Strategy>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <BaseClass>SubSub</BaseClass>"
    "        <ECProperty propertyName='P3' typeName='int' />"
    "    </ECClass>"
    "</ECSchema>", true, "Using MapStrategy NotMapped multiple times within Hierarchy is expected to be supported"),

    SchemaItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Base' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>NotMapped</Strategy>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='P0' typeName='int' />"
    "    </ECClass>"
    "    <ECClass typeName='Sub' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>SharedTable</Strategy>"
    "                  <AppliesToSubclasses>true</AppliesToSubclasses>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <BaseClass>Base</BaseClass>"
    "        <ECProperty propertyName='P1' typeName='int' />"
    "    </ECClass>"
    "    <ECClass typeName='SubSub' isDomainClass='True'>"
    "        <BaseClass>Sub</BaseClass>"
    "        <ECProperty propertyName='P2' typeName='int' />"
    "    </ECClass>"
    "</ECSchema>", true, "SharedTable(polymorphic) within Class Hierarchy is expected to be supported where Root has Strategy NotMapped(non-Polymorphic)"),

    SchemaItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Base' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>NotMapped</Strategy>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='P0' typeName='int' />"
    "    </ECClass>"
    "    <ECClass typeName='Sub' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>OwnTable</Strategy>"
    "                  <AppliesToSubclasses>true</AppliesToSubclasses>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <BaseClass>Base</BaseClass>"
    "        <ECProperty propertyName='P1' typeName='int' />"
    "    </ECClass>"
    "    <ECClass typeName='SubSub' isDomainClass='True'>"
    "        <BaseClass>Sub</BaseClass>"
    "        <ECProperty propertyName='P2' typeName='int' />"
    "    </ECClass>"
    "</ECSchema>", true, "OwnTable(polymorphic) within Class Hierarchy is expected to be supported where Root has Strategy NotMapped(non-Polymorphic)"),

    SchemaItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Base' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>NotMapped</Strategy>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='P0' typeName='int' />"
    "    </ECClass>"
    "    <ECClass typeName='Sub' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>OwnTable</Strategy>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <BaseClass>Base</BaseClass>"
    "        <ECProperty propertyName='P1' typeName='int' />"
    "    </ECClass>"
    "    <ECClass typeName='SubSub' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>SharedTable</Strategy>"
    "                  <AppliesToSubclasses>true</AppliesToSubclasses>"
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
    "</ECSchema>", true, "Polymorphic Strategies can be used in class Hierarchy where Root has non-polymorphic Strategies"),

    SchemaItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Base' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>OwnTable</Strategy>"
    "                  <AppliesToSubclasses>true</AppliesToSubclasses>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='P0' typeName='int' />"
    "    </ECClass>"
    "    <ECClass typeName='Sub' isDomainClass='True'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>NotMapped</Strategy>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <BaseClass>Base</BaseClass>"
    "        <ECProperty propertyName='P1' typeName='int' />"
    "    </ECClass>"
    "</ECSchema>", false, "NotMapped within Class Hierarchy is not Supported"),

        SchemaItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='Parent' isDomainClass='True'>"
            "        <ECProperty propertyName='Price' typeName='double' />"
            "    </ECClass>"
            "    <ECClass typeName='Base' isDomainClass='False' isStruct='True'>"
            "        <BaseClass>Parent</BaseClass>"
            "        <ECProperty propertyName='Item' typeName='double' />"
            "    </ECClass>"
            "</ECSchema>", false, "Struct is not supported within a class heirarchy."),

        SchemaItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='ParentA' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>OwnTable</Strategy>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
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
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
            "            </ClassMap>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='Price' typeName='double' />"
            "    </ECClass>"
            "</ECSchema>", false, "Conflicting Map Strategies SharedTable(Polymorphic) for derived Class where base class has strategy OwnTable(Polymorphic)"),

        SchemaItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='Parent' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <Options>SharedColumns</Options>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
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
            "</ECSchema>", false, "Option 'DisableSharedColumn' doesn't allow strategy to be set."),

        SchemaItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='ParentA' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>ExistingTable</Strategy>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
            "            </ClassMap>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='Price_parentA' typeName='double' />"
            "    </ECClass>"
            "</ECSchema>", false, ""),

        SchemaItem(
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
            "</ECSchema>", false, "abc is not a valid MapStrategy Option"),

        SchemaItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='Parent' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>OwnTable</Strategy>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
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

        SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                 "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                 "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                 "    <ECClass typeName='Parent' isDomainClass='True'>"
                 "        <ECCustomAttributes>"
                 "            <ClassMap xmlns='ECDbMap.01.00'>"
                 "                <MapStrategy>"
                 "                   <Strategy>SharedTable</Strategy>"
                 "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
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
                 "</ECSchema>", false, "Strategy NotMapped-polymorphic is not supported within a  class hierarchy"),

        SchemaItem(
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
            "</ECSchema>", false, "Own Table doesn't allows a user Defined table name.")
    };

    for (SchemaItem const& item : testItems)
        {
        AssertSchemaImport (item, "ecdbmapcatests.ecdb");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, TablePrefix)
    {
     {
     SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "        <ECCustomAttributes>"
            "            <SchemaMap xmlns='ECDbMap.01.00'>"
            "                <TablePrefix>myownprefix</TablePrefix>"
            "            </SchemaMap>"
            "        </ECCustomAttributes>"
            "    <ECClass typeName='A' isDomainClass='True'>"
            "        <ECProperty propertyName='P1' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='B' isDomainClass='True'>"
            "        <ECProperty propertyName='P2' typeName='string' />"
            "    </ECClass>"
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
                       "<ECSchema schemaName='test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                       "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                       "        <ECCustomAttributes>"
                       "            <SchemaMap xmlns='ECDbMap.01.00'>"
                       "            </SchemaMap>"
                       "        </ECCustomAttributes>"
                       "    <ECClass typeName='A' isDomainClass='True'>"
                       "        <ECProperty propertyName='P1' typeName='string' />"
                       "    </ECClass>"
                       "    <ECClass typeName='B' isDomainClass='True'>"
                       "        <ECProperty propertyName='P2' typeName='string' />"
                       "    </ECClass>"
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
TEST_F (ECDbMappingTestFixture, SharedColumnsForSubclassesOnHierarchyAccrossMultipleSchemas)
    {
    SchemaItem testItem
        ("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='ReferedSchema' nameSpacePrefix='rs' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Base' isDomainClass='False'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <Options>SharedColumnsForSubClasses</Options>"
        "                  <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='Sub1' isDomainClass='True'>"
        "         <BaseClass>Base</BaseClass>"
        "    </ECClass>"
        "    <ECClass typeName='Sub2' isDomainClass='True'>"
        "         <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECClass>"
        "</ECSchema>", true, "Mapstrategy Option SharedColumnForSubClasses (applied to subclasses) is expected to succeed");

    ECDb testDb;
    ASSERT_EQ (DbResult::BE_SQLITE_OK, ECDbTestFixture::CreateECDb (testDb, "SharedColumnsForSubclasses.ecdb"));
    bool asserted = false;
    AssertSchemaImport (asserted, testDb, testItem);
    ASSERT_FALSE (asserted);

    SchemaItem derivedItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ReferedSchema' version='01.00' prefix='rs' />"
        "    <ECClass typeName='Sub3' isDomainClass='True'>"
        "         <BaseClass>rs:Sub2</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='Sub4' isDomainClass='True'>"
        "         <BaseClass>Sub3</BaseClass>"
        "        <ECProperty propertyName='P4' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='DerivedSub4A' isDomainClass='True'>"
        "        <BaseClass>Sub3</BaseClass>"
        "        <ECProperty propertyName='P5' typeName='double' />"
        "    </ECClass>"
        "    <ECClass typeName='DerivedSub4B' isDomainClass='True'>"
        "        <BaseClass>Sub3</BaseClass>"
        "        <ECProperty propertyName='P6' typeName='string' />"
        "    </ECClass>"
        "</ECSchema>", true, "Mapstrategy Option SharedColumnForSubClasses (applied to subclasses) is expected to be honored from base Class of Refered schema");

    AssertSchemaImport (asserted, testDb, derivedItem);
    ASSERT_FALSE (asserted);

    //verify Number and Names of Columns in BaseClass
    BeSQLite::Statement statement;
    const int expectedColCount = 8;
    ASSERT_EQ (DbResult::BE_SQLITE_OK, statement.Prepare (testDb, "SELECT * FROM rs_Base"));
    ASSERT_EQ (DbResult::BE_SQLITE_DONE, statement.Step ());
    ASSERT_EQ (expectedColCount, statement.GetColumnCount ());

    //verify that the columns generated are same as expected
    Utf8String expectedColumnNames = "ECInstanceIdECClassIdP0sc01sc02sc03sc04sc05";
    Utf8String actualColumnNames;
    for (int i = 0; i < expectedColCount; i++)
        {
        actualColumnNames.append (statement.GetColumnName (i));
        }
    ASSERT_STREQ (expectedColumnNames.c_str (), actualColumnNames.c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbMappingTestFixture, DisableSharedColumnsOnHierarchyAccrossMultipleSchemas)
    {
    SchemaItem testItem
        ("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='ReferedSchema' nameSpacePrefix='rs' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Base' isDomainClass='False'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <Options>SharedColumnsForSubClasses</Options>"
        "                  <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='Sub1' isDomainClass='True'>"
        "         <BaseClass>Base</BaseClass>"
        "    </ECClass>"
        "    <ECClass typeName='Sub2' isDomainClass='True'>"
        "         <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='int' />"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECClass>"
        "</ECSchema>", true, "Mapstrategy Option SharedColumnForSubClasses (applied to subclasses) is expected to succeed");

    ECDb testDb;
    ASSERT_EQ (DbResult::BE_SQLITE_OK, ECDbTestFixture::CreateECDb (testDb, "DisableSharedColumns.ecdb"));
    bool asserted = false;
    AssertSchemaImport (asserted, testDb, testItem);
    ASSERT_FALSE (asserted);

    SchemaItem derivedItem (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECSchemaReference name='ReferedSchema' version='01.00' prefix='rs' />"
        "    <ECClass typeName='Sub3' isDomainClass='True'>"
        "         <BaseClass>rs:Sub2</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='Sub4' isDomainClass='True'>"
        "         <BaseClass>Sub3</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Options>DisableSharedColumns</Options>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P4' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='DerivedSub4A' isDomainClass='True'>"
        "        <BaseClass>Sub4</BaseClass>"
        "        <ECProperty propertyName='P5' typeName='double' />"
        "    </ECClass>"
        "    <ECClass typeName='DerivedSub4B' isDomainClass='True'>"
        "        <BaseClass>Sub4</BaseClass>"
        "        <ECProperty propertyName='P6' typeName='string' />"
        "    </ECClass>"
        "</ECSchema>", true, "Mapstrategy Option SharedColumnForSubClasses (applied to subclasses) is expected to be honored from base Class of Refered schema");

    AssertSchemaImport (asserted, testDb, derivedItem);
    ASSERT_FALSE (asserted);

    //verify Number and Names of Columns in BaseClass
    BeSQLite::Statement statement;
    const int expectedColCount = 8;
    ASSERT_EQ (DbResult::BE_SQLITE_OK, statement.Prepare (testDb, "SELECT * FROM rs_Base"));
    ASSERT_EQ (DbResult::BE_SQLITE_DONE, statement.Step ());
    ASSERT_EQ (expectedColCount, statement.GetColumnCount ());

    //verify that the columns generated are same as expected
    Utf8String expectedColumnNames = "ECInstanceIdECClassIdP0sc01sc02sc03P4sc04";
    Utf8String actualColumnNames;
    for (int i = 0; i < expectedColCount; i++)
        {
        actualColumnNames.append (statement.GetColumnName (i));
        }
    ASSERT_STREQ (expectedColumnNames.c_str (), actualColumnNames.c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbMappingTestFixture, JoinedTableForSubclassesOnHierarchyAccrossMultipleSchemas)
    {
    SchemaItem testItem (
    "<?xml version='1.0' encoding='utf-8'?>"
    "<ECSchema schemaName='ReferedSchema' nameSpacePrefix='rs' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
    "    <ECClass typeName='Base' isDomainClass='False'>"
    "        <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.01.00'>"
    "                <MapStrategy>"
    "                  <Strategy>SharedTable</Strategy>"
    "                  <Options>JoinedTableForSubclasses</Options>"
    "                  <AppliesToSubclasses>True</AppliesToSubclasses>"
    "                </MapStrategy>"
    "            </ClassMap>"
    "        </ECCustomAttributes>"
    "        <ECProperty propertyName='P0' typeName='int' />"
    "    </ECClass>"
    "    <ECClass typeName='Sub1' isDomainClass='True'>"
    "         <BaseClass>Base</BaseClass>"
    "    </ECClass>"
    "    <ECClass typeName='Sub2' isDomainClass='True'>"
    "         <BaseClass>Sub1</BaseClass>"
    "        <ECProperty propertyName='P1' typeName='int' />"
    "        <ECProperty propertyName='P2' typeName='int' />"
    "    </ECClass>"
    "</ECSchema>", true, "Mapstrategy Option JoinedTableForSubclasses (applied to subclasses) is expected to succeed");

    ECDb testDb;
    bool asserted = false;
    AssertSchemaImport (testDb, asserted, testItem, "joinedtableforsubclasses.ecdb");
    ASSERT_FALSE (asserted);

    SchemaItem derivedItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ReferedSchema' version='01.00' prefix='rs' />"
        "    <ECClass typeName='Sub3' isDomainClass='True'>"
        "         <BaseClass>rs:Sub2</BaseClass>"
        "        <ECProperty propertyName='P3' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='Sub4' isDomainClass='True'>"
        "         <BaseClass>Sub3</BaseClass>"
        "        <ECProperty propertyName='P4' typeName='int' />"
        "    </ECClass>"
        "</ECSchema>", true, "Mapstrategy Option JoinedTableForSubclasses (applied to subclasses) is expected to be honored from base Class of Refered schema");

    AssertSchemaImport (asserted, testDb, derivedItem);
    ASSERT_FALSE (asserted);
    ASSERT_EQ(DbResult::BE_SQLITE_OK, testDb.SaveChanges ());

    //verify tables
    std::vector<Utf8String> tableNames;
        {
        Statement stmt;
        ASSERT_EQ (BE_SQLITE_OK, stmt.Prepare (testDb, "SELECT name FROM sqlite_master WHERE name Like 'rs_%' and type='table'"));
        while (BE_SQLITE_ROW == stmt.Step ())
            {
            tableNames.push_back (stmt.GetValueText (0));
            }
        }
        ASSERT_EQ (2, tableNames.size ());

        auto it = std::find (tableNames.begin (), tableNames.end (), "rs_Base");
        ASSERT_TRUE (it != tableNames.end ()) << "Table ts_Base is expected to exist";

        it = std::find (tableNames.begin (), tableNames.end (), "rs_Base_Joined");
        ASSERT_TRUE (it != tableNames.end ()) << "Table ts_Base_Joined is expected to exist";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, AbstractClassWithSharedTable)
    {
    SchemaItem testItem ("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestAbstractClasses' nameSpacePrefix='tac' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='AbstractBaseClass' isDomainClass='False'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <AppliesToSubclasses>True</AppliesToSubclasses>"
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
        "                  <AppliesToSubclasses>False</AppliesToSubclasses>"
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
        "                  <AppliesToSubclasses>False</AppliesToSubclasses>"
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
    ASSERT_EQ(s2.Step(), BE_SQLITE_DONE);
    ASSERT_EQ(s3.Prepare(db, "INSERT INTO tac.ChildDomainClassB (P1, P3) VALUES('Hello', 'World')"), ECSqlStatus::Success);
    ASSERT_EQ(s3.Step(), BE_SQLITE_DONE);
    ASSERT_EQ(s5.Prepare(db, "INSERT INTO tac.SharedTable1 (P2) VALUES('Hello')"), ECSqlStatus::Success);
    ASSERT_EQ(s5.Step(), BE_SQLITE_DONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  09/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, ForeignKeyMapCreateIndex)
    {
    SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
        "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "  <ECClass typeName='Parent' >"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECClass>"
        "  <ECClass typeName='Child' >"
        "    <ECProperty propertyName='ParentId' typeName='long' />"
        "    <ECProperty propertyName='ChildName' typeName='string' />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName='RelCreateIndexTrue' isDomainClass='True' strength='referencing'>"
        "    <ECCustomAttributes>"
        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00' />"
        "    </ECCustomAttributes>"
        "    <Source cardinality='(1,1)' polymorphic='True'>"
        "      <Class class = 'Parent' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Child' >"
        "           <Key>"
        "              <Property name='ParentId'/>"
        "           </Key>"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "  <ECRelationshipClass typeName='RelCreateIndexFalse' isDomainClass='True' strength='referencing'>"
        "    <ECCustomAttributes>"
        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00' >"
        "           <CreateIndex>False</CreateIndex>"
        "        </ForeignKeyRelationshipMap>"
        "    </ECCustomAttributes>"
        "    <Source cardinality='(1,1)' polymorphic='True'>"
        "      <Class class = 'Parent' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Child' >"
        "           <Key>"
        "              <Property name='ParentId'/>"
        "           </Key>"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "  <ECRelationshipClass typeName='RelCreateIndexDefaultValue' isDomainClass='True' strength='referencing'>"
        "    <ECCustomAttributes>"
        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00' />"
        "    </ECCustomAttributes>"
        "    <Source cardinality='(1,1)' polymorphic='True'>"
        "      <Class class = 'Parent' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Child' >"
        "           <Key>"
        "              <Property name='ParentId'/>"
        "           </Key>"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>", true, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "foreignkeymapcreateindex.ecdb");
    ASSERT_FALSE(asserted);

    AssertIndexExists(ecdb, "ix_ts_Child_fk_ts_RelCreateIndexTrue_target", false);
    AssertIndexExists(ecdb, "ix_ts_Child_fk_ts_RelCreateIndexDefaultValue_target", false);
    AssertIndexExists(ecdb, "ix_ts_Child_fk_ts_RelCreateIndexFalse_target", false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, PropertyWithSameNameAsStructMemberColumn)
    {
        {
        SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                            "  <ECClass typeName='ElementCode' isStruct='True' isDomainClass='False'>"
                            "    <ECProperty propertyName='Name' typeName='string' />"
                            "  </ECClass>"
                            "  <ECClass typeName='Foo' >"
                            "    <ECProperty propertyName='Code_Name' typeName='string' />"
                            "    <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                            "  </ECClass>"
                            "</ECSchema>", true, "");

        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testItem, "propertywithsamenameasstructmembercol.ecdb");
        ASSERT_FALSE(asserted);

        ASSERT_TRUE(ecdb.ColumnExists("ts_Foo", "Code_Name"));
        ASSERT_TRUE(ecdb.ColumnExists("ts_Foo", "c131__43_ode__4e_ame"));
            }

        {
        //now flip order of struct prop and prim prop
        SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                          "  <ECClass typeName='ElementCode' isStruct='True' isDomainClass='False'>"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECClass>"
                          "  <ECClass typeName='Foo' >"
                          "    <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                          "    <ECProperty propertyName='Code_Name' typeName='string' />"
                          "  </ECClass>"
                          "</ECSchema>", true, "");

        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testItem, "propertywithsamenameasstructmembercol.ecdb");
        ASSERT_FALSE(asserted);

        ASSERT_TRUE(ecdb.ColumnExists("ts_Foo", "Code_Name"));
        ASSERT_TRUE(ecdb.ColumnExists("ts_Foo", "c131__43_ode_5f__4e_ame"));
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbMappingTestFixture, InstanceInsertionInExistingTable)
    {
    ECDb ecdb;
    EXPECT_EQ (BE_SQLITE_OK, ECDbTestUtility::CreateECDb (ecdb, nullptr, L"Test.ecdb"));
    EXPECT_TRUE (ecdb.IsDbOpen ());

    EXPECT_FALSE (ecdb.TableExists ("TestTable"));

    AString ddl = "ECInstanceId INTEGER PRIMARY KEY";
    ddl.append (", Name TEXT");
    ddl.append (", Date INTEGER");

    ecdb.CreateTable ("TestTable", ddl.c_str ());
    EXPECT_TRUE (ecdb.TableExists ("TestTable"));

    //TODO: Use TEstItem and AssertSchemaImport(bool,ECDbCR,SchemaItem const&) instead
    Utf8CP testSchemaXML = "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "<ECClass typeName='Class' isDomainClass='True'>"
        "<ECCustomAttributes>"
        "<ClassMap xmlns='ECDbMap.01.00'>"
        "<MapStrategy>"
        "<Strategy>ExistingTable</Strategy>"
        "</MapStrategy>"
        "<TableName>TestTable</TableName>"
        "</ClassMap>"
        "</ECCustomAttributes>"
        "<ECProperty propertyName='Name' typeName='string'/>"
        "<ECProperty propertyName='Date' typeName='int'/>"
        "</ECClass>"
        "</ECSchema>";

    auto schemaCache = ECDbTestUtility::ReadECSchemaFromString (testSchemaXML);
    ASSERT_TRUE (schemaCache != nullptr);

    BentleyStatus status = ecdb.Schemas ().ImportECSchemas (*schemaCache);
    ASSERT_TRUE (status == 0);

    ECN::ECSchemaCP schema = ecdb.Schemas ().GetECSchema ("TestSchema");
    ASSERT_TRUE (schema != nullptr);

    ECClassCP Class = schema->GetClassCP ("Class");
    ASSERT_TRUE (Class != nullptr) << "Couldn't locate class Base from schema";

    //Insert Instances in the class.
    ECN::StandaloneECInstancePtr Instance1 = Class->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ECN::StandaloneECInstancePtr Instance2 = Class->GetDefaultStandaloneEnabler ()->CreateInstance ();

    Instance1->SetValue ("Name", ECValue ("Abc"));
    Instance1->SetValue ("Date", ECValue (123));

    Instance2->SetValue ("Name", ECValue ("Foo"));
    Instance2->SetValue ("Date", ECValue (321));

    ECInstanceInserter inserter (ecdb, *Class);
    ASSERT_TRUE (inserter.IsValid ());

    auto stat = inserter.Insert (*Instance1, true);
    ASSERT_TRUE (stat == SUCCESS);

    stat = inserter.Insert (*Instance2, true);
    ASSERT_TRUE (stat == SUCCESS);

    //Verifying that the class is not mapped to any table other than the Existing Table.
    ASSERT_FALSE (ecdb.TableExists ("t_Class"));
    }

//---------------------------------------------------------------------------------------
//*Test to verify the CRUD operations for a schema having similar Class and Property name
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbMappingTestFixture, ClassAndPropertyWithSameName)
    {
    SchemaItem testItem (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Product' isDomainClass='True'>"
        "        <ECProperty propertyName='Product' typeName='string' />"
        "        <ECProperty propertyName='Price' typeName='int' />"
        "    </ECClass>"
        "</ECSchema>", true, "");

    ECDb db;
    bool asserted = false;
    AssertSchemaImport (db, asserted, testItem, "InstanceCRUD.ecdb");
    ASSERT_FALSE (asserted);

    //Inserts Instances.
    ECSqlStatement stmt, s1, s2, s3, s4;
    ASSERT_EQ (ECSqlStatus::Success, s1.Prepare (db, "INSERT INTO ts.Product (Product,Price) VALUES('Book',100)"));
    ASSERT_EQ (BE_SQLITE_DONE, s1.Step ());
    ASSERT_EQ (ECSqlStatus::Success, s2.Prepare (db, "INSERT INTO ts.Product (Product,Price) VALUES('E-Reader',200)"));
    ASSERT_EQ (BE_SQLITE_DONE, s2.Step ());
    ASSERT_EQ (ECSqlStatus::Success, s3.Prepare (db, "INSERT INTO ts.Product (Product,Price) VALUES('I-Pod',700)"));
    ASSERT_EQ (BE_SQLITE_DONE, s3.Step ());
    ASSERT_EQ (ECSqlStatus::Success, s4.Prepare (db, "INSERT INTO ts.Product (Product,Price) VALUES('Goggles',500)"));
    ASSERT_EQ (BE_SQLITE_DONE, s4.Step ());

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (db, "SELECT COUNT(*) FROM ts.Product"));
    ASSERT_TRUE (BE_SQLITE_ROW == stmt.Step ());
    ASSERT_EQ (4, stmt.GetValueInt (0));
    stmt.Finalize ();

    //Deletes the instance
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (db, "DELETE FROM ts.Product WHERE Price=200"));
    ASSERT_TRUE (BE_SQLITE_DONE == stmt.Step ());
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (db, "SELECT COUNT(*) FROM ts.Product"));
    ASSERT_TRUE (BE_SQLITE_ROW == stmt.Step ());
    ASSERT_EQ (3, stmt.GetValueInt (0));
    stmt.Finalize ();

    //Updates the instance
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (db, "UPDATE ts.Product SET Product='Watch' WHERE Price=500"));
    ASSERT_TRUE (BE_SQLITE_DONE == stmt.Step ());
    stmt.Finalize ();

    //Select the instance matching the query.
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (db, "SELECT Product.Product FROM ts.Product WHERE Price=700"));
    ASSERT_TRUE (BE_SQLITE_ROW == stmt.Step ());
    ASSERT_STREQ ("I-Pod", stmt.GetValueText (0));
    stmt.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbMappingTestFixture, MismatchDataTypesInExistingTable)
    {
    ECDb ecdb;
    ASSERT_EQ (BE_SQLITE_OK, ECDbTestUtility::CreateECDb (ecdb, nullptr, L"Test.ecdb"));
    ASSERT_TRUE (ecdb.IsDbOpen ());

    ASSERT_FALSE (ecdb.TableExists ("TestTable"));
    ecdb.CreateTable("TestTable", "ECInstanceId INTEGER PRIMARY KEY, Name TEXT, Date INTEGER");
    ASSERT_TRUE(ecdb.TableExists("TestTable"));

    SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                      "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                      "<ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                      "<ECClass typeName='Class' isDomainClass='True'>"
                      "  <ECCustomAttributes>"
                      "   <ClassMap xmlns='ECDbMap.01.00'>"
                      "    <MapStrategy>"
                      "      <Strategy>ExistingTable</Strategy>"
                      "    </MapStrategy>"
                      "    <TableName>TestTable</TableName>"
                      "   </ClassMap>"
                      "  </ECCustomAttributes>"
                      "  <ECProperty propertyName='Name' typeName='string'/>"
                      "  <ECProperty propertyName='Date' typeName='double'/>"
                      "</ECClass>"
                      "</ECSchema>", false);

    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, testItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbMappingTestFixture, InvalidPrimaryKeyInExistingTable)
    {
    ECDb ecdb;
    ASSERT_EQ (BE_SQLITE_OK, ECDbTestUtility::CreateECDb (ecdb, nullptr, L"InvalidPrimaryKeyInExistingTable.ecdb"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ASSERT_FALSE(ecdb.TableExists("TestTable"));

    ecdb.CreateTable("TestTable", "Id INTEGER PRIMARY KEY, Name TEXT, Date INTEGER");
    ASSERT_TRUE(ecdb.TableExists("TestTable"));

    SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                      "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                      "<ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                      "<ECClass typeName='TestClass' isDomainClass='True'>"
                      "<ECCustomAttributes>"
                      "<ClassMap xmlns='ECDbMap.01.00'>"
                      "<MapStrategy>"
                      "<Strategy>ExistingTable</Strategy>"
                      "</MapStrategy>"
                      "<TableName>TestTable</TableName>"
                      "</ClassMap>"
                      "</ECCustomAttributes>"
                      "<ECProperty propertyName='Name' typeName='string'/>"
                      "<ECProperty propertyName='Date' typeName='double'/>"
                      "</ECClass>"
                      "</ECSchema>", false);

    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, testItem);
    ASSERT_FALSE(asserted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbMappingTestFixture, SharedTableInstanceInsertionAndDeletion)
    {
    SchemaItem testItem ("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='t' version='1.0' description='Handles the insertion and deletion from standalone classes.' displayLabel='Table Per Hierarchy' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "<ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "<ECClass typeName='ClassA' isDomainClass='True'>"
        "<ECCustomAttributes>"
        "<ClassMap xmlns='ECDbMap.01.00'>"
        "<MapStrategy>"
        "<Strategy>SharedTable</Strategy>"
        "<AppliesToSubclasses>False</AppliesToSubclasses>"
        "</MapStrategy>"
        "<TableName>TestTable</TableName>"
        "</ClassMap>"
        "</ECCustomAttributes>"
        "<ECProperty propertyName='P1' typeName='string' />"
        "</ECClass>"
        "<ECClass typeName='ClassB' isDomainClass='True'>"
        "<ECCustomAttributes>"
        "<ClassMap xmlns='ECDbMap.01.00'>"
        "<MapStrategy>"
        "<Strategy>SharedTable</Strategy>"
        "<AppliesToSubclasses>False</AppliesToSubclasses>"
        "</MapStrategy>"
        "<TableName>TestTable</TableName>"
        "</ClassMap>"
        "</ECCustomAttributes>"
        "<ECProperty propertyName='P2' typeName='string' />"
        "</ECClass>"
        "<ECClass typeName='ClassC' isDomainClass='True'>"
        "<ECCustomAttributes>"
        "<ClassMap xmlns='ECDbMap.01.00'>"
        "<MapStrategy>"
        "<Strategy>SharedTable</Strategy>"
        "<AppliesToSubclasses>False</AppliesToSubclasses>"
        "</MapStrategy>"
        "<TableName>TestTable</TableName>"
        "</ClassMap>"
        "</ECCustomAttributes>"
        "<ECProperty propertyName='P3' typeName='string' />"
        "</ECClass>"
        "</ECSchema>", true);

    ECDb ecdb;
    bool asserted = false;
    ECSqlStatement statment;

    AssertSchemaImport (ecdb, asserted, testItem, "SharedTableTest.ecdb");
    ASSERT_FALSE (asserted);

    ECSchemaCP testSchema = ecdb.Schemas ().GetECSchema ("TestSchema");
    EXPECT_TRUE (testSchema != nullptr);

    //Inserts values in Class A,B and C.
    EXPECT_EQ (ECSqlStatus::Success, statment.Prepare (ecdb, "INSERT INTO t.ClassA(P1) VALUES ('Testval1')"));
    EXPECT_TRUE (BE_SQLITE_DONE == statment.Step ());
    statment.Finalize ();

    EXPECT_EQ (ECSqlStatus::Success, statment.Prepare (ecdb, "INSERT INTO t.ClassB(P2) VALUES ('Testval2')"));
    EXPECT_TRUE (BE_SQLITE_DONE == statment.Step ());
    statment.Finalize ();

    EXPECT_EQ (ECSqlStatus::Success, statment.Prepare (ecdb, "INSERT INTO t.ClassC(P3) VALUES ('Testval3')"));
    EXPECT_TRUE (BE_SQLITE_DONE == statment.Step ());
    statment.Finalize ();

    //Deletes the instance of ClassA.
    EXPECT_EQ (ECSqlStatus::Success, statment.Prepare (ecdb, "DELETE FROM t.ClassA"));
    EXPECT_TRUE (BE_SQLITE_DONE == statment.Step ());
    statment.Finalize ();
    
    BeSQLite::Statement stmt;
    EXPECT_EQ (DbResult::BE_SQLITE_OK, stmt.Prepare (ecdb, "SELECT COUNT(*) FROM TestTable"));
    ASSERT_TRUE (DbResult::BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ (2, stmt.GetValueInt (0));

    //Updates the instance of ClassB.
    EXPECT_EQ (ECSqlStatus::Success, statment.Prepare (ecdb, "UPDATE t.ClassB SET P2='UpdatedValue'"));
    EXPECT_TRUE (BE_SQLITE_DONE == statment.Step ());
    statment.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, SharedTableAppliesToSubclasses_SharedColumns)
    {
    SchemaItem testItem(
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
        "                  <AppliesToSubclasses>True</AppliesToSubclasses>"
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
    size_t columnCount = statement.GetColumnCount();
    ASSERT_EQ(columnCount, 5);

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
// @bsimethod                                   Muhammad Hassan                     04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, SharedTableAppliesToSubclasses_SharedColumns_DisableSharedColumns)
    {
    SchemaItem testItem(
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
        "                  <AppliesToSubclasses>True</AppliesToSubclasses>"
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
    ASSERT_EQ(BE_SQLITE_DONE, s1.Step());
    ASSERT_EQ(ECSqlStatus::Success, s2.Prepare(db, "INSERT INTO rc.ChildDomainClassA (P1, P2) VALUES('ChildClassA', 10.002)"));
    ASSERT_EQ(BE_SQLITE_DONE, s2.Step());
    ASSERT_EQ(ECSqlStatus::Success, s3.Prepare(db, "INSERT INTO rc.ChildDomainClassB (P1, P3) VALUES('ChildClassB', 2)"));
    ASSERT_EQ(BE_SQLITE_DONE, s3.Step());
    ASSERT_EQ(ECSqlStatus::Success, s4.Prepare(db, "INSERT INTO rc.DerivedA (P1, P2, P4) VALUES('DerivedA', 11.003, 12.004)"));
    ASSERT_EQ(BE_SQLITE_DONE, s4.Step());
    ASSERT_EQ(ECSqlStatus::Success, s5.Prepare(db, "INSERT INTO rc.DerivedB (P1, P2, P5) VALUES('DerivedB', 11.003, 'DerivedB')"));
    ASSERT_EQ(BE_SQLITE_DONE, s5.Step());

    //verify No of Columns in BaseClass
    const int expectedColCount = 6;
    Statement statement;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, statement.Prepare(db, "SELECT * FROM rc_BaseClass"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, statement.Step());
    ASSERT_EQ(expectedColCount, statement.GetColumnCount());

    //verify that the columns generated are same as expected
    Utf8String expectedColumnNames = "ECInstanceIdECClassIdP1P2sc01P3";
    Utf8String actualColumnNames;
    for (int i = 0; i < expectedColCount; i++)
        {
        actualColumnNames.append(statement.GetColumnName(i));
        }
    ASSERT_STREQ(expectedColumnNames.c_str(), actualColumnNames.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, SharedTableAppliesToSubclasses_JoinedTableForSubclasses)
    {
    SchemaItem testItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Base' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <Options>JoinedTableForSubclasses</Options>"
        "                  <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='Sub1' isDomainClass='True'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P1' typeName='double' />"
        "    </ECClass>"
        "    <ECClass typeName='Sub2' isDomainClass='True'>"
        "        <BaseClass>Base</BaseClass>"
        "        <ECProperty propertyName='P2' typeName='int' />"
        "    </ECClass>"
        "    <ECClass typeName='Sub11' isDomainClass='True'>"
        "        <BaseClass>Sub1</BaseClass>"
        "        <ECProperty propertyName='P11' typeName='double' />"
        "    </ECClass>"
        "</ECSchema>", true, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "joinedtableforsubclasses.ecdb");
    ASSERT_FALSE(asserted);

    //verify tables
    std::vector<Utf8String> tableNames;
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT name FROM sqlite_master WHERE name Like 'ts_%' and type='table'"));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        tableNames.push_back(stmt.GetValueText(0));
        }
    }
    ASSERT_EQ(2, tableNames.size());

    auto it = std::find(tableNames.begin(), tableNames.end(), "ts_Base");
    ASSERT_TRUE(it != tableNames.end()) << "Table ts_Base is expected to exist";

    it = std::find(tableNames.begin(), tableNames.end(), "ts_Base_Joined");
    ASSERT_TRUE(it != tableNames.end()) << "Table ts_Base_Joined is expected to exist";

    //verify that joined table option was resolved correctly. Need to look at the ec_ClassMap table directly to check that.
    std::map<ECClassId, PersistedMapStrategy> expectedResults {
        {ecdb.Schemas().GetECClassId("TestSchema","Base"), PersistedMapStrategy(PersistedMapStrategy::Strategy::SharedTable, PersistedMapStrategy::Options::ParentOfJoinedTable, true)},
        {ecdb.Schemas().GetECClassId("TestSchema","Sub1"), PersistedMapStrategy(PersistedMapStrategy::Strategy::SharedTable, PersistedMapStrategy::Options::JoinedTable, true)},
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
// @bsimethod                                   Maha Nasir                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbMappingTestFixture, IndexGenerationOnClassId)
    {
    SchemaItem testItem (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='Test' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='ClassA' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='ClassB' isDomainClass='True'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                  <Strategy>SharedTable</Strategy>"
        "                  <AppliesToSubclasses>True</AppliesToSubclasses>"
        "                </MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='P0' typeName='string' />"
        "    </ECClass>"
        "</ECSchema>", true);

    ECDb db;
    bool asserted = false;
    AssertSchemaImport (db, asserted, testItem, "IndexGenerationOnClassId.ecdb");
    ASSERT_FALSE (asserted);

    Statement sqlstmt;
    ASSERT_EQ (DbResult::BE_SQLITE_OK, sqlstmt.Prepare (db, "SELECT Name FROM ec_Index WHERE Id=(SELECT IndexId FROM ec_IndexColumn WHERE ColumnId=(SELECT Id FROM ec_Column WHERE Name='ECClassId' AND TableId=(SELECT Id FROM ec_Table WHERE Name='ts_ClassA')))"));
    ASSERT_EQ (DbResult::BE_SQLITE_ROW, sqlstmt.Step ());
    ASSERT_STREQ ("ix_ts_ClassA_ecclassid", sqlstmt.GetValueText (0));
    sqlstmt.Finalize ();

    ASSERT_EQ (DbResult::BE_SQLITE_OK, sqlstmt.Prepare (db, "SELECT Name FROM ec_Index WHERE Id=(SELECT IndexId FROM ec_IndexColumn WHERE ColumnId=(SELECT Id FROM ec_Column WHERE Name='ECClassId' AND TableId=(SELECT Id FROM ec_Table WHERE Name='ts_ClassB')))"));
    ASSERT_EQ (DbResult::BE_SQLITE_ROW, sqlstmt.Step ());
    ASSERT_STREQ ("ix_ts_ClassB_ecclassid", sqlstmt.GetValueText (0));
    sqlstmt.Finalize ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, NotMappedWithinClassHierarchy)
    {
    SchemaItem testItem(
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
        "                  <AppliesToSubclasses>False</AppliesToSubclasses>"
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
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "INSERT should be possible even if base class is not mapped";
        }

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "SELECT P1,P2,P3 FROM ts.SubSubSub")) << "SELECT should be possible even if base class is not mapped";
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "SELECT should be possible even if base class is not mapped";
        ASSERT_EQ(1, stmt.GetValueInt(0));
        ASSERT_EQ(2, stmt.GetValueInt(1));
        ASSERT_EQ(3, stmt.GetValueInt(2));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (ECDbMappingTestFixture, CascadeDeletion)
    {
    SchemaItem testItem ("<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='ClassA' isDomainClass='True'>"
        "        <ECProperty propertyName='AA' typeName='string' />"
        "    </ECClass>"
        "    <ECClass typeName='ClassB' isDomainClass='True'>"
        "        <ECProperty propertyName='BB' typeName='string' />"
        "    </ECClass>"
        "    <ECRelationshipClass typeName='AHasB' isDomainClass='True' strength='embedding'>"
        "        <ECCustomAttributes>"
        "            <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
        "               <CreateConstraint>True</CreateConstraint>"
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
        "    <ECClass typeName='ClassC' isDomainClass='True'>"
        "        <ECProperty propertyName='CC' typeName='string' />"
        "    </ECClass>"
        "  <ECRelationshipClass typeName='BHasC' isDomainClass='True' strength='embedding'>"
        "    <Source cardinality='(1,1)' polymorphic='True'>"
        "      <Class class = 'ClassB' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'ClassC' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>", true, "");

    ECDb db;
    bool asserted = false;
    AssertSchemaImport (db, asserted, testItem, "CascadeDeletion.ecdb");
    ASSERT_FALSE (asserted);

    ECSchemaCP schema = db.Schemas ().GetECSchema ("TestSchema");
    EXPECT_TRUE (schema != nullptr);

    ECClassCP ClassA = schema->GetClassCP ("ClassA");
    EXPECT_TRUE (ClassA != nullptr);

    ECClassCP ClassB = schema->GetClassCP ("ClassB");
    EXPECT_TRUE (ClassB != nullptr);

    ECClassCP ClassC = schema->GetClassCP ("ClassC");
    EXPECT_TRUE (ClassC != nullptr);

    //Instance of ClassA
    StandaloneECInstancePtr ClassA_Instance = ClassA->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ClassA_Instance->SetValue ("AA", ECValue ("val1"));

    //Instance of ClassB
    StandaloneECInstancePtr ClassB_Instance = ClassB->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ClassB_Instance->SetValue ("BB", ECValue ("val3"));

    //Inserter of ClassA
    ECInstanceInserter ClassA_Inserter (db, *ClassA);
    ASSERT_TRUE (ClassA_Inserter.IsValid ());
    ClassA_Inserter.Insert (*ClassA_Instance);

    //Inserter of ClassB
    ECInstanceInserter ClassB_Inserter (db, *ClassB);
    ASSERT_TRUE (ClassB_Inserter.IsValid ());
    ClassB_Inserter.Insert (*ClassB_Instance);

    ECRelationshipClassCP AHasB = db.Schemas ().GetECClass ("TestSchema", "AHasB")->GetRelationshipClassCP ();
    ECRelationshipClassCP BHasC = db.Schemas ().GetECClass ("TestSchema", "BHasC")->GetRelationshipClassCP ();

    //Inserting relationship instance.
    ECN::StandaloneECRelationshipInstancePtr ClassAHasB_relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*AHasB)->CreateRelationshipInstance ();
    ClassAHasB_relationshipInstance->SetSource (ClassA_Instance.get ());
    ClassAHasB_relationshipInstance->SetTarget (ClassB_Instance.get ());

    ECInstanceInserter AHasB_relationshipInserter (db, *AHasB);
    AHasB_relationshipInserter.Insert (*ClassAHasB_relationshipInstance);


    //Inserting instances of ClassC
    StandaloneECInstancePtr ClassC_Instance = ClassC->GetDefaultStandaloneEnabler ()->CreateInstance ();
    ClassC_Instance->SetValue ("CC", ECValue ("val5"));

    //Inserter of ClassC
    ECInstanceInserter ClassC_Inserter (db, *ClassC);
    ASSERT_TRUE (ClassC_Inserter.IsValid ());
    ClassC_Inserter.Insert (*ClassC_Instance);

    //Inserting relationship instances.
    ECN::StandaloneECRelationshipInstancePtr ClassBHasC_relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*BHasC)->CreateRelationshipInstance ();
    ClassBHasC_relationshipInstance->SetSource (ClassB_Instance.get ());
    ClassBHasC_relationshipInstance->SetTarget (ClassC_Instance.get ());

    ECInstanceInserter BHasC_relationshipInserter (db, *BHasC);
    BHasC_relationshipInserter.Insert (*ClassBHasC_relationshipInstance);

    //Deletes instance of ClassA. Instances of ClassB and ClassC are also deleted.
    ECSqlStatement stmt;
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (db, "DELETE FROM ts.ClassA WHERE ECInstanceId=1"));
    ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ());
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (db, "SELECT AA FROM ts.ClassA"));
    ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ());
    ASSERT_EQ (NULL, stmt.GetValueText (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (db, "SELECT BB FROM ts.ClassB"));
    ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ());
    ASSERT_EQ (NULL, stmt.GetValueText (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (db, "SELECT CC FROM ts.ClassC"));
    ASSERT_EQ (DbResult::BE_SQLITE_DONE, stmt.Step ());
    ASSERT_EQ (NULL, stmt.GetValueText (0));
    stmt.Finalize ();
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
            ecsql.Sprintf("INSERT INTO %s (ECInstanceId, %s) VALUES(%lld, %s)", fullyQualifiedTestClass, prop, id.GetValue() , val);
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

            ECInstanceId id(id.GetValue() + 1);
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
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='TestClass' isDomainClass='True'>"
                "    <ECProperty propertyName='P0' typeName='string' />"
                "    </ECClass>"
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
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='TestClass' isDomainClass='True'>"
                "        <ECCustomAttributes>"
                "            <DisableECInstanceIdAutogeneration xmlns='ECDbMap.01.00'>"
                "            </DisableECInstanceIdAutogeneration>"
                "        </ECCustomAttributes>"
                "    <ECProperty propertyName='P0' typeName='string' />"
                "    </ECClass>"
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
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='Parent' isDomainClass='True'>"
                "        <ECCustomAttributes>"
                "            <DisableECInstanceIdAutogeneration xmlns='ECDbMap.01.00'>"
                "                  <AppliesToSubclasses>True</AppliesToSubclasses>"
                "            </DisableECInstanceIdAutogeneration>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='P0' typeName='String' />"
                "    </ECClass>"
                "    <ECClass typeName='Child' isDomainClass='True'>"
                "    <BaseClass>Parent</BaseClass>"
                "        <ECProperty propertyName='P1' typeName='String' />"
                "    </ECClass>"
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
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                "    <ECClass typeName='Parent' isDomainClass='True'>"
                "        <ECCustomAttributes>"
                "            <DisableECInstanceIdAutogeneration xmlns='ECDbMap.01.00'>"
                "                  <AppliesToSubclasses>False</AppliesToSubclasses>"
                "            </DisableECInstanceIdAutogeneration>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='P0' typeName='String' />"
                "    </ECClass>"
                "    <ECClass typeName='Child' isDomainClass='True'>"
                "    <BaseClass>Parent</BaseClass>"
                "        <ECProperty propertyName='P1' typeName='String' />"
                "    </ECClass>"
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
TEST_F (ECDbMappingTestFixture, UserDefinedIndexTest)
    {
        {
        std::vector<SchemaItem> testItems {SchemaItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='Element' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
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
            "    </ECClass>"
            "</ECSchema>", false, "Property in index does not exist"),

                SchemaItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                    "    <ECClass typeName='ElementCode' isDomainClass='False' isStruct='True' isCustomAttribute='False'>"
                    "        <ECProperty propertyName='AuthorityId' typeName='long' />"
                    "        <ECProperty propertyName='Namespace' typeName='string' />"
                    "        <ECProperty propertyName='Val' typeName='string' />"
                    "    </ECClass>"
                    "    <ECClass typeName='Element' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.01.00'>"
                    "                <MapStrategy>"
                    "                   <Strategy>SharedTable</Strategy>"
                    "                   <Options>SharedColumnsForSubclasses</Options>"
                    "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
                    "                 </MapStrategy>"
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
                    "        </ECCustomAttributes>"
                    "        <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                    "    </ECClass>"
                    "</ECSchema>", false, "Cannot define index on struct prop"),

                SchemaItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='ElementCode' isDomainClass='False' isStruct='True' isCustomAttribute='False'>"
            "        <ECProperty propertyName='AuthorityId' typeName='long' />"
            "        <ECProperty propertyName='Namespace' typeName='string' />"
            "        <ECProperty propertyName='Val' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='Element' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <Options>SharedColumnsForSubclasses</Options>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
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
            "        <ECArrayProperty propertyName='Codes' typeName='ElementCode' isStruct='True' minOccurs='0' maxOccurs='unbounded' />"
            "    </ECClass>"
            "</ECSchema>", false, "Cannot define index on struct array prop"),

            SchemaItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='Element' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <Options>SharedColumnsForSubclasses</Options>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
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
            "    </ECClass>"
                "</ECSchema>", false, "Cannot define index on primitive array prop"),

                SchemaItem(
                    "<?xml version='1.0' encoding='utf-8'?>"
                    "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                    "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                    "    <ECClass typeName='A' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.01.00'>"
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
                    "    </ECClass>"
                    "    <ECClass typeName='B' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
                    "        <ECCustomAttributes>"
                    "            <ClassMap xmlns='ECDbMap.01.00'>"
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
                    "    </ECClass>"
                    "</ECSchema>", false, "Duplicate indexes")};

        AssertSchemaImport(testItems, "userdefinedindextest.ecdb");
        }

        {
        SchemaItem testItem (
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
        "    <ECClass typeName='Base' isDomainClass='False' isStruct='False' isCustomAttribute='False'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.01.00'>"
        "                <MapStrategy>"
        "                   <Strategy>SharedTable</Strategy>"
        "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
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
        AssertSchemaImport (db, asserted, testItem, "userdefinedindextest.ecdb");
        ASSERT_FALSE (asserted);

        AssertIndex (db, "ix_base_code", false, "ts_Base", { "Code" });
        }

        {
        SchemaItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='Base' isDomainClass='False' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
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

        AssertIndex(db, "ix_sub1_prop", false, "ts_Base", {"Sub1_Prop"});
        }

            {
        SchemaItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='Base' isDomainClass='False' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
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

        ECClassId baseClassId = db.Schemas().GetECClassId("TestSchema", "Base");
        Utf8String indexWhereClause;
        indexWhereClause.Sprintf("ECClassId<>%lld", baseClassId);
        AssertIndex(db, "uix_sub1_code", true, "ts_Base", {"Code"}, indexWhereClause.c_str());
        }

        {
        SchemaItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='Base' isDomainClass='False' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
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
            "    </ECClass>"
            "</ECSchema>", true, "");

        SchemaItem secondSchemaTestItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='TestSchema' version='01.00' prefix='ts' />"
            "    <ECClass typeName='Sub4' isDomainClass='True'>"
            "        <BaseClass>ts:Sub3</BaseClass>"
            "        <ECProperty propertyName='Sub4_Prop' typeName='double' />"
            "    </ECClass>"
            "</ECSchema>", true, "");
        
        ECClassId sub3ClassId = ECClass::UNSET_ECCLASSID;
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
        indexWhereClause.Sprintf("ECClassId=%lld", sub3ClassId);
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
        indexWhereClause.Sprintf("ECClassId=%lld OR ECClassId=%lld", sub3ClassId, sub4ClassId);
        AssertIndex(ecdb, "uix_sub3_prop", true, "ts_Base", {"Sub3_Prop"}, indexWhereClause.c_str());
        }

        {
        SchemaItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='Base' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <Options>SharedColumnsForSubclasses</Options>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
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
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='Base' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <Options>SharedColumnsForSubclasses</Options>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
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
            "    </ECClass>"
            "    <ECClass typeName='Sub2' isDomainClass='True'>"
            "        <BaseClass>Base</BaseClass>"
            "        <ECProperty propertyName='Name' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='Sub3' isDomainClass='True'>"
            "        <BaseClass>Base</BaseClass>"
            "        <ECProperty propertyName='Name2' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='Sub11' isDomainClass='True'>"
            "        <BaseClass>Sub1</BaseClass>"
            "        <ECProperty propertyName='Cost' typeName='double' />"
            "    </ECClass>"
            "</ECSchema>", true, "Unique indices on shared columns are supported.");

        ECDb db;
        bool asserted = false;
        AssertSchemaImport(db, asserted, testItem, "userdefinedindextest.ecdb");
        ASSERT_FALSE(asserted);

        ECClassId sub1ClassId = db.Schemas().GetECClassId("TestSchema", "Sub1");
        ECClassId sub11ClassId = db.Schemas().GetECClassId("TestSchema", "Sub11");
        Utf8String indexWhereClause;
        indexWhereClause.Sprintf("ECClassId=%lld OR ECClassId=%lld", sub1ClassId, sub11ClassId);
        AssertIndex(db, "uix_sub1_aid", true, "ts_Base", {"sc01"}, indexWhereClause.c_str());
        }

        {
        SchemaItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='Base' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <Options>SharedColumnsForSubclasses</Options>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
            "            </ClassMap>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='Code' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='Sub1' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                 <MapStrategy>"
            "                   <Options>DisableSharedColumns</Options>"
            "                 </MapStrategy>"
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
            "    </ECClass>"
            "    <ECClass typeName='Sub2' isDomainClass='True'>"
            "        <BaseClass>Base</BaseClass>"
            "        <ECProperty propertyName='Name' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='Sub3' isDomainClass='True'>"
            "        <BaseClass>Base</BaseClass>"
            "        <ECProperty propertyName='Name' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='Sub11' isDomainClass='True'>"
            "        <BaseClass>Sub1</BaseClass>"
            "        <ECProperty propertyName='Cost' typeName='double' />"
            "    </ECClass>"
            "</ECSchema>", true, "");

        ECDb db;
        bool asserted = false;
        AssertSchemaImport(db, asserted, testItem, "userdefinedindextest.ecdb");
        ASSERT_FALSE(asserted);

        ECClassId sub1ClassId = db.Schemas().GetECClassId("TestSchema", "Sub1");
        ECClassId sub11ClassId = db.Schemas().GetECClassId("TestSchema", "Sub11");
        Utf8String indexWhereClause;
        indexWhereClause.Sprintf("ECClassId=%lld OR ECClassId=%lld", sub1ClassId, sub11ClassId);
        AssertIndex(db, "uix_sub1_aid", true, "ts_Base", {"AId"}, indexWhereClause.c_str());
        }

        {
        SchemaItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='ElementCode' isDomainClass='False' isStruct='True' isCustomAttribute='False'>"
            "        <ECProperty propertyName='AuthorityId' typeName='long' />"
            "        <ECProperty propertyName='Namespace' typeName='string' />"
            "        <ECProperty propertyName='Val' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='Element' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
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
            "    </ECClass>"
            "</ECSchema>", true, "");

        ECDb db;
        bool asserted = false;
        AssertSchemaImport(db, asserted, testItem, "userdefinedindextest.ecdb");
        ASSERT_FALSE(asserted);

        AssertIndex(db, "uix_element_code", true, "ts_Element", {"Code_AuthorityId", "Code_Namespace", "Code_Val"});
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, NotNullablePropertyTest)
    {
        {
        SchemaItem testItem(
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
        SchemaItem testItem(
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
        AssertIndex(db, "ix_b_id_name", false, "ts_B", {"Id","Name"}, "([Name] IS NOT NULL)");
        }

        {
        SchemaItem testItem(
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
        SchemaItem testItem(
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

        AssertIndexExists(db, "ix_ts_B_fk_ts_Rel_target", false);
        }

        {
        SchemaItem testItem(
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

        AssertIndexExists(db, "ix_ts_B_fk_ts_Rel_target", false);
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
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='A' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECProperty propertyName='AId' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='B' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
            "            </ClassMap>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='AId' typeName='long' />"
            "        <ECProperty propertyName='BId' typeName='long' />"
            "    </ECClass>"
            "    <ECClass typeName='BB' isDomainClass='True'>"
            "        <BaseClass>B</BaseClass>"
            "        <ECProperty propertyName='BBId' typeName='long' />"
            "    </ECClass>"
            "   <ECRelationshipClass typeName='Rel' isDomainClass='True' strength='embedding'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='A' />"
            "    </Source>"
            "    <Target cardinality='(0,N)' polymorphic='True'>"
            "      <Class class='B' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "   <ECRelationshipClass typeName='Rel11' isDomainClass='True' strength='embedding'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='A' />"
            "    </Source>"
            "    <Target cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='B' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "   <ECRelationshipClass typeName='RelWithKeyProp' isDomainClass='True' strength='embedding'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='A' />"
            "    </Source>"
            "    <Target cardinality='(0,N)' polymorphic='True'>"
            "      <Class class='B'>"
            "        <Key><Property name='AId'/></Key>"
            "      </Class>"
            "    </Target>"
            "  </ECRelationshipClass>"
            "   <ECRelationshipClass typeName='RelWithKeyProp11' isDomainClass='True' strength='embedding'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='A' />"
            "    </Source>"
            "    <Target cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='B'>"
            "        <Key><Property name='AId'/></Key>"
            "      </Class>"
            "    </Target>"
            "  </ECRelationshipClass>"
            "   <ECRelationshipClass typeName='RelNN' isDomainClass='True' strength='embedding'>"
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
        AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships.ecdb");
        ASSERT_FALSE(asserted);

        AssertIndex(ecdb, "ix_ts_B_fk_ts_Rel_target", false, "ts_B", {"ForeignECInstanceId_Rel"}, "([ForeignECInstanceId_Rel] IS NOT NULL)");
        AssertIndex(ecdb, "uix_ts_B_fk_ts_Rel11_target", true, "ts_B", {"ForeignECInstanceId_Rel11"}, "([ForeignECInstanceId_Rel11] IS NOT NULL)");
        
        //For relationships with key property, index is created if unique (as this is to enforce cardinality
        AssertIndexExists(ecdb, "ix_ts_B_fk_ts_RelWithKeyProp_target", false);
        AssertIndex(ecdb, "uix_ts_B_fk_ts_RelWithKeyProp11_target", true, "ts_B", {"AId"}, "([AId] IS NOT NULL)");

        AssertIndex(ecdb, "ix_ts_RelNN_source", false, "ts_RelNN", {"SourceECInstanceId"});
        AssertIndex(ecdb, "ix_ts_RelNN_target", false, "ts_RelNN", {"TargetECInstanceId"});
        AssertIndex(ecdb, "uix_ts_RelNN_sourcetarget", true, "ts_RelNN", {"SourceECInstanceId", "TargetECInstanceId"});
        }

        {
        SchemaItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='A' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECProperty propertyName='AId' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='B' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <Options>SharedColumns</Options>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
            "            </ClassMap>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='AId' typeName='long' />"
            "        <ECProperty propertyName='BId' typeName='long' />"
            "    </ECClass>"
            "    <ECClass typeName='BB' isDomainClass='True'>"
            "        <BaseClass>B</BaseClass>"
            "        <ECProperty propertyName='BBId' typeName='long' />"
            "    </ECClass>"
            "   <ECRelationshipClass typeName='Rel' isDomainClass='True' strength='embedding'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='A' />"
            "    </Source>"
            "    <Target cardinality='(0,N)' polymorphic='True'>"
            "      <Class class='B' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", true, "");

        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships.ecdb");
        ASSERT_FALSE(asserted);
        AssertIndex(ecdb, "ix_ts_B_fk_ts_Rel_target", false, "ts_B", {"ForeignECInstanceId_Rel"}, "([ForeignECInstanceId_Rel] IS NOT NULL)");
        }

        {
        SchemaItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='A' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECProperty propertyName='Id' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='B' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <Options>SharedColumns</Options>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
            "            </ClassMap>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='AId' typeName='long' />"
            "        <ECProperty propertyName='BId' typeName='long' />"
            "    </ECClass>"
            "    <ECClass typeName='BB' isDomainClass='True'>"
            "        <BaseClass>B</BaseClass>"
            "        <ECProperty propertyName='BBId' typeName='long' />"
            "    </ECClass>"
            "   <ECRelationshipClass typeName='Rel' isDomainClass='True' strength='embedding'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='A' />"
            "    </Source>"
            "    <Target cardinality='(0,N)' polymorphic='True'>"
            "      <Class class='B'>"
            "        <Key><Property name='AId'/></Key>"
            "      </Class>"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", true);

        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships.ecdb");
        ASSERT_FALSE(asserted);

        AssertIndexExists(ecdb, "ix_ts_B_fk_ts_Rel_target", false);
        }

        {
        SchemaItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='A' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECProperty propertyName='Id' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='B' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <Options>SharedColumns</Options>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
            "            </ClassMap>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='AId' typeName='long' />"
            "        <ECProperty propertyName='BId' typeName='long' />"
            "    </ECClass>"
            "    <ECClass typeName='BB' isDomainClass='True'>"
            "        <BaseClass>B</BaseClass>"
            "        <ECProperty propertyName='BBId' typeName='long' />"
            "    </ECClass>"
            "   <ECRelationshipClass typeName='Rel11' isDomainClass='True' strength='embedding'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='A' />"
            "    </Source>"
            "    <Target cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='B'>"
            "        <Key><Property name='AId'/></Key>"
            "      </Class>"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", true, "");

        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships.ecdb");
        ASSERT_FALSE(asserted);

        AssertIndex(ecdb, "uix_ts_B_fk_ts_Rel11_target", true, "ts_B", {"sc01"}, "([sc01] IS NOT NULL)");
        }

        {
        SchemaItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='A' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECProperty propertyName='Id' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='B' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <Options>SharedColumns</Options>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
            "            </ClassMap>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='AId' typeName='long' />"
            "        <ECProperty propertyName='BId' typeName='long' />"
            "    </ECClass>"
            "    <ECClass typeName='B1' isDomainClass='True'>"
            "        <BaseClass>B</BaseClass>"
            "        <ECProperty propertyName='B1Id' typeName='long' />"
            "    </ECClass>"
            "    <ECClass typeName='B2' isDomainClass='True'>"
            "        <BaseClass>B</BaseClass>"
            "        <ECProperty propertyName='B2Id' typeName='long' />"
            "    </ECClass>"
            "   <ECRelationshipClass typeName='Rel11' isDomainClass='True' strength='embedding'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='A' />"
            "    </Source>"
            "    <Target cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='B1'>"
            "        <Key><Property name='B1Id'/></Key>"
            "      </Class>"
            "    </Target>"
            "  </ECRelationshipClass>"
            "   <ECRelationshipClass typeName='AnotherRel11' isDomainClass='True' strength='embedding'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='A' />"
            "    </Source>"
            "    <Target cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='B1'>"
            "        <Key><Property name='B1Id'/></Key>"
            "      </Class>"
            "    </Target>"
            "  </ECRelationshipClass>"
            "   <ECRelationshipClass typeName='Rel1N' isDomainClass='True' strength='embedding'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='A' />"
            "    </Source>"
            "    <Target cardinality='(1,N)' polymorphic='True'>"
            "      <Class class='B1'>"
            "        <Key><Property name='B1Id'/></Key>"
            "      </Class>"
            "    </Target>"
            "  </ECRelationshipClass>"
            "   <ECRelationshipClass typeName='Rel1NNoKeyProp' isDomainClass='True' strength='embedding'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='A' />"
            "    </Source>"
            "    <Target cardinality='(1,N)' polymorphic='True'>"
            "      <Class class='B1' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", true, "");

        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships1.ecdb");
        ASSERT_FALSE(asserted);

        ECClassId b1ClassId = ecdb.Schemas().GetECClassId("TestSchema", "B1");
        Utf8String indexWhereClause;
        indexWhereClause.Sprintf("([sc03] IS NOT NULL) AND (ECClassId=%lld)", b1ClassId);
        AssertIndexExists(ecdb, "ix_ts_B_fk_ts_Rel1N_target", false);
        AssertIndex(ecdb, "ix_ts_B_fk_ts_Rel1NNoKeyProp_target", false, "ts_B", {"ForeignECInstanceId_Rel1NNoKeyProp"}, "([ForeignECInstanceId_Rel1NNoKeyProp] IS NOT NULL)");
        AssertIndex(ecdb, "ix_ts_B_ecclassid", false, "ts_B", {"ECClassId"});

        //Unique indexes on FK for Rel11 and AnotherRel11 are the same, therefore one is dropped
        ASSERT_EQ(3, (int) RetrieveIndicesForTable(ecdb, "ts_B").size());
        }

        {
        SchemaItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='A' isDomainClass='True' isStruct='False' isCustomAttribute='False'>"
            "        <ECProperty propertyName='Id' typeName='string' />"
            "    </ECClass>"
            "    <ECClass typeName='B' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
            "                 <Indexes>"
            "                 <DbIndex>"
            "                   <Name>ix_B_AId</Name>"
            "                   <IsUnique>False</IsUnique>"
            "                   <Properties>"
            "                      <string>AId</string>"
            "                   </Properties>"
            "                 </DbIndex>"
            "                 </Indexes>"
            "             </ClassMap>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='AId' typeName='long' />"
            "    </ECClass>"
            "    <ECClass typeName='B1' isDomainClass='True'>"
            "        <BaseClass>B</BaseClass>"
            "        <ECProperty propertyName='B1Id' typeName='long' />"
            "    </ECClass>"
            "   <ECRelationshipClass typeName='RelBase' isDomainClass='True' strength='referencing'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='A'/>"
            "    </Source>"
            "    <Target cardinality='(1,N)' polymorphic='True'>"
            "      <Class class='B'>"
            "        <Key><Property name='AId'/></Key>"
            "      </Class>"
            "    </Target>"
            "  </ECRelationshipClass>"
            "   <ECRelationshipClass typeName='RelSub1' isDomainClass='True' strength='referencing'>"
            "    <BaseClass>RelBase</BaseClass>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='A' />"
            "    </Source>"
            "    <Target cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='B1'>"
            "        <Key><Property name='AId'/></Key>"
            "      </Class>"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", true, "");

        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships.ecdb");
        ASSERT_FALSE(asserted);

        ASSERT_EQ(3, (int) RetrieveIndicesForTable(ecdb, "ts_B").size()) << "Expected indices: class id index, user defined index, unique index to enforce cardinality of RelSub1";

        AssertIndex(ecdb, "ix_B_AId", false, "ts_B", {"AId"});

        AssertIndexExists(ecdb, "ix_ts_B_fk_ts_RelBase_target", false);

        ECClassId b1ClassId = ecdb.Schemas().GetECClassId("TestSchema", "B1");
        Utf8String indexWhereClause;
        indexWhereClause.Sprintf("([AId] IS NOT NULL) AND (ECClassId=%lld)", b1ClassId);

        AssertIndex(ecdb, "uix_ts_B_fk_ts_RelSub1_target", true, "ts_B", {"Aid"}, indexWhereClause.c_str());
        }

        {
        SchemaItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='B' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
            "             </ClassMap>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='Id' typeName='long' />"
            "    </ECClass>"
            "    <ECClass typeName='B1' isDomainClass='True'>"
            "        <BaseClass>B</BaseClass>"
            "        <ECProperty propertyName='B1Id' typeName='long' />"
            "    </ECClass>"
            "   <ECRelationshipClass typeName='RelBase' isDomainClass='True' strength='referencing'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <Options>SharedColumns</Options>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
            "             </ClassMap>"
            "        </ECCustomAttributes>"
            "    <Source cardinality='(1,N)' polymorphic='True'>"
            "      <Class class='B'/>"
            "    </Source>"
            "    <Target cardinality='(1,N)' polymorphic='True'>"
            "      <Class class='B' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "   <ECRelationshipClass typeName='RelSub11' isDomainClass='True' strength='referencing'>"
            "    <BaseClass>RelBase</BaseClass>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='B' />"
            "    </Source>"
            "    <Target cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='B1' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "   <ECRelationshipClass typeName='RelSub1N' isDomainClass='True' strength='referencing'>"
            "    <BaseClass>RelBase</BaseClass>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='B1' />"
            "    </Source>"
            "    <Target cardinality='(1,N)' polymorphic='True'>"
            "      <Class class='B1' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", true, "");

        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships.ecdb");
        ASSERT_FALSE(asserted);

        ASSERT_EQ(9, (int) RetrieveIndicesForTable(ecdb, "ts_RelBase").size());
        }

        {
        SchemaItem testItem(
            "<?xml version='1.0' encoding='utf-8'?>"
            "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
            "    <ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
            "    <ECClass typeName='A' isDomainClass='True'>"
            "        <ECProperty propertyName='AId' typeName='long' />"
            "    </ECClass>"
            "    <ECClass typeName='B' isDomainClass='True'>"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                 </MapStrategy>"
            "             </ClassMap>"
            "        </ECCustomAttributes>"
            "        <ECProperty propertyName='Id' typeName='long' />"
            "    </ECClass>"
            "    <ECClass typeName='B1' isDomainClass='True'>"
            "        <BaseClass>B</BaseClass>"
            "        <ECProperty propertyName='B1Id' typeName='long' />"
            "    </ECClass>"
            "    <ECClass typeName='B11' isDomainClass='True'>"
            "        <BaseClass>B1</BaseClass>"
            "        <ECProperty propertyName='B11Id' typeName='long' />"
            "    </ECClass>"
            "    <ECClass typeName='B2' isDomainClass='True'>"
            "        <BaseClass>B</BaseClass>"
            "        <ECProperty propertyName='B2Id' typeName='long' />"
            "    </ECClass>"
            "   <ECRelationshipClass typeName='RelNonPoly' isDomainClass='True' strength='referencing'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='A' />"
            "    </Source>"
            "    <Target cardinality='(1,1)' polymorphic='False'>"
            "      <Class class='B1' />"
            "      <Class class='B2' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "   <ECRelationshipClass typeName='RelPoly' isDomainClass='True' strength='referencing'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='A' />"
            "    </Source>"
            "    <Target cardinality='(1,1)' polymorphic='True'>"
            "      <Class class='B1' />"
            "      <Class class='B2' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", true, "");

        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships.ecdb");
        ASSERT_FALSE(asserted);

        ASSERT_EQ(3, (int) RetrieveIndicesForTable(ecdb, "ts_B").size());

        ECClassId bClassId = ecdb.Schemas().GetECClassId("TestSchema", "B");
        ECClassId b1ClassId = ecdb.Schemas().GetECClassId("TestSchema", "B1");
        ECClassId b2ClassId = ecdb.Schemas().GetECClassId("TestSchema", "B2");
        
        //RelNonPoly must exclude index on B11 as the constraint is non-polymorphic
        Utf8String indexWhereClause;
        indexWhereClause.Sprintf("([ForeignECInstanceId_RelNonPoly] IS NOT NULL) AND (ECClassId=%lld OR ECClassId=%lld)", b1ClassId, b2ClassId);
        AssertIndex(ecdb, "uix_ts_B_fk_ts_RelNonPoly_target", true, "ts_B", {"ForeignECInstanceId_RelNonPoly"}, indexWhereClause.c_str());

        //RelPoly must include index on B11 as the constraint is polymorphic
        indexWhereClause.Sprintf("([ForeignECInstanceId_RelPoly] IS NOT NULL) AND (ECClassId<>%lld)", bClassId);
        AssertIndex(ecdb, "uix_ts_B_fk_ts_RelPoly_target", true, "ts_B", {"ForeignECInstanceId_RelPoly"}, indexWhereClause.c_str());
        }

        {
        //Tests that AllowDuplicateRelationships Flag from LinkTableRelationshipMap CA is not applied to subclasses
        SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                          "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                          "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
                          "  <ECClass typeName='A' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECClass>"
                          "  <ECClass typeName='B' >"
                          "    <ECProperty propertyName='BName' typeName='string' />"
                          "  </ECClass>"
                          "  <ECClass typeName='C' >"
                          "    <ECProperty propertyName='CName' typeName='string' />"
                          "  </ECClass>"
                          "  <ECRelationshipClass typeName='ARelB' isDomainClass='True' strength='referencing'>"
                          "    <ECCustomAttributes>"
                          "        <ClassMap xmlns='ECDbMap.01.00'>"
                          "                <MapStrategy>"
                          "                   <Strategy>SharedTable</Strategy>"
                          "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
                          "                </MapStrategy>"
                          "        </ClassMap>"
                          "        <LinkTableRelationshipMap xmlns='ECDbMap.01.00'>"
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
                          "  <ECRelationshipClass typeName='ARelC' isDomainClass='True' strength='referencing'>"
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
        AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships.ecdb");
        ASSERT_FALSE(asserted);

        ASSERT_TRUE(ecdb.TableExists("ts_ARelB"));
        ASSERT_FALSE(ecdb.TableExists("ts_ARelC")) << "ARelC is expected to be persisted in ts_ARelB as well (SharedTable strategy)";

        ASSERT_EQ(5, (int) RetrieveIndicesForTable(ecdb, "ts_ARelB").size());

        ECClassId aRelCClassId = ecdb.Schemas().GetECClassId("TestSchema", "ARelC");
        ASSERT_TRUE(aRelCClassId != ECClass::UNSET_ECCLASSID);

        Utf8String indexWhereClause;
        indexWhereClause.Sprintf("ECClassId=%lld", aRelCClassId);

        AssertIndex(ecdb, "ix_ts_ARelB_source", false, "ts_ARelB", {"SourceECInstanceId"});
        AssertIndex(ecdb, "ix_ts_ARelB_target", false, "ts_ARelB", {"TargetECInstanceId"});

        AssertIndex(ecdb, "uix_ts_ARelC_target", true, "ts_ARelB", {"TargetECInstanceId"}, indexWhereClause.c_str());

        //ARelB must not have a unique index on source and target as it as AllowDuplicateRelationship set to true.
        //ARelC must have the unique index, as AllowDuplicateRelationship is not applied to subclasses
        AssertIndexExists(ecdb, "uix_ts_ARelB_sourcetarget", false);
        AssertIndex(ecdb, "uix_ts_ARelC_sourcetarget", true, "ts_ARelB", {"SourceECInstanceId", "TargetECInstanceId"}, indexWhereClause.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, ForeignKeyMapWhereLinkTableIsRequired)
    {
    SchemaItem testItem ("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
        "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "  <ECClass typeName='Parent' >"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECClass>"
        "  <ECClass typeName='Child' >"
        "    <ECProperty propertyName='ParentId' typeName='long' />"
        "    <ECProperty propertyName='ChildName' typeName='string' />"
        "  </ECClass>"
        "  <ECClass typeName='Child2' >"
        "    <ECProperty propertyName='ParentId' typeName='long' />"
        "    <ECProperty propertyName='ChildName' typeName='string' />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
        "    <ECCustomAttributes>"
        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'/>"
        "    </ECCustomAttributes>"
        "    <Source cardinality='(1,1)' polymorphic='True'>"
        "      <Class class = 'Parent' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Child' />"
        "      <Class class = 'Child2' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>", false,"Cannot apply ForeignKeyRelationshipMap when a link table is required.");

    AssertSchemaImport(testItem, "ForeignKeyMapWhereLinkTableIsRequired.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, ForeignKeyMapWithKeyProperty)
    {
    Utf8CP ecdbName = "ForeignKeyMapWithKeyProp.ecdb";

    {
    std::vector<SchemaItem> testItems {
        SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
                "  <ECClass typeName='Parent' >"
                "    <ECProperty propertyName='Name' typeName='string' />"
                "  </ECClass>"
                "  <ECClass typeName='Child' >"
                "    <ECProperty propertyName='ParentId' typeName='long' />"
                "    <ECProperty propertyName='ChildName' typeName='string' />"
                "  </ECClass>"
                "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
                "    <ECCustomAttributes>"
                "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
                "            <ForeignKeyColumn>ParentId</ForeignKeyColumn>"
                "        </ForeignKeyRelationshipMap>"
                "    </ECCustomAttributes>"
                "    <Source cardinality='(1,1)' polymorphic='True'>"
                "      <Class class = 'Parent' />"
                "    </Source>"
                "    <Target cardinality='(0,N)' polymorphic='True'>"
                "      <Class class = 'Child' >"
                "           <Key>"
                "              <Property name='ParentId'/>"
                "           </Key>"
                "      </Class>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>", false, "ForeignKeyColumn should not be specified if Key property is defined."),

        SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                 "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
                 "  <ECClass typeName='Parent' >"
                 "    <ECProperty propertyName='Name' typeName='string' />"
                 "  </ECClass>"
                 "  <ECClass typeName='Child' >"
                 "    <ECProperty propertyName='ParentId' typeName='long' />"
                 "    <ECProperty propertyName='ChildName' typeName='string' />"
                 "  </ECClass>"
                 "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
                 "    <ECCustomAttributes>"
                 "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
                 "            <ForeignKeyColumn>MyOwnParentId</ForeignKeyColumn>"
                 "        </ForeignKeyRelationshipMap>"
                 "    </ECCustomAttributes>"
                 "    <Source cardinality='(1,1)' polymorphic='True'>"
                 "      <Class class = 'Parent' />"
                 "    </Source>"
                 "    <Target cardinality='(0,N)' polymorphic='True'>"
                 "      <Class class = 'Child' >"
                 "           <Key>"
                 "              <Property name='ParentId'/>"
                 "           </Key>"
                 "      </Class>"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "</ECSchema>", false, "ForeignKeyColumn should not be specified if Key property is defined."),

        SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                 "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
                 "  <ECClass typeName='Parent' >"
                 "    <ECProperty propertyName='Name' typeName='string' />"
                 "  </ECClass>"
                 "  <ECClass typeName='Child' >"
                 "    <ECCustomAttributes>"
                 "        <ClassMap xmlns='ECDbMap.01.00'>"
                 "                <MapStrategy>"
                 "                   <Strategy>SharedTable</Strategy>"
                 "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
                 "                </MapStrategy>"
                 "        </ClassMap>"
                 "    </ECCustomAttributes>"
                 "    <ECProperty propertyName='ParentId' typeName='long' />"
                 "    <ECProperty propertyName='ChildName' typeName='string' />"
                 "  </ECClass>"
                 "  <ECClass typeName='Child2' >"
                 "    <BaseClass>Child</BaseClass>"
                 "    <ECProperty propertyName='Child2Name' typeName='string' />"
                 "  </ECClass>"
                 "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
                 "    <Source cardinality='(1,1)' polymorphic='True'>"
                 "      <Class class = 'Parent' />"
                 "    </Source>"
                 "    <Target cardinality='(0,N)' polymorphic='True'>"
                 "      <Class class = 'Child' >"
                 "           <Key>"
                 "              <Property name='ParentId'/>"
                 "           </Key>"
                 "      </Class>"
                 "      <Class class = 'Child2' />"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "</ECSchema>", false, "Only one constraint class supported by ECDb if key properties are defined."),

        SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                 "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                 "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
                 "  <ECClass typeName='Parent' >"
                 "    <ECProperty propertyName='Name' typeName='string' />"
                 "  </ECClass>"
                 "  <ECClass typeName='Child' >"
                 "    <ECProperty propertyName='ParentId' typeName='long' />"
                 "    <ECProperty propertyName='ChildName' typeName='string' />"
                 "  </ECClass>"
                 "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
                 "    <Source cardinality='(1,1)' polymorphic='True'>"
                 "      <Class class = 'Parent' />"
                 "    </Source>"
                 "    <Target cardinality='(0,N)' polymorphic='True'>"
                 "      <Class class = 'Child' >"
                 "           <Key>"
                 "              <Property name='ParentId'/>"
                 "              <Property name='ChildName'/>"
                 "           </Key>"
                 "      </Class>"
                 "    </Target>"
                 "  </ECRelationshipClass>"
                 "</ECSchema>", false, "Only one key property is supported by ECDb.")};

    AssertSchemaImport(testItems, ecdbName);
    }

    Utf8CP childTableName = "ts_Child";

        {
        SchemaItem testItem ("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
            "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
            "  <ECClass typeName='Parent' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECClass>"
            "  <ECClass typeName='Child' >"
            "    <ECProperty propertyName='ParentId' typeName='long' />"
            "    <ECProperty propertyName='ChildName' typeName='string' />"
            "  </ECClass>"
            "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class = 'Parent' />"
            "    </Source>"
            "    <Target cardinality='(0,N)' polymorphic='True'>"
            "      <Class class = 'Child' >"
            "           <Key>"
            "              <Property name='ParentId'/>"
            "           </Key>"
            "      </Class>"
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
        ASSERT_EQ(3, columns.size()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it == columns.end()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";
        AssertForeignKey(false, ecdb, childTableName);
        }

        {
        SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
            "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
            "  <ECClass typeName='Parent' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECClass>"
            "  <ECClass typeName='Child' >"
            "    <ECProperty propertyName='ParentId' typeName='long'>"
            "       <ECCustomAttributes>"
            "          <PropertyMap xmlns='ECDbMap.01.00'>"
            "            <ColumnName>parent_id</ColumnName>"
            "          </PropertyMap>"
            "       </ECCustomAttributes>"
            "   </ECProperty>"
            "    <ECProperty propertyName='ChildName' typeName='string' />"
            "  </ECClass>"
            "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class = 'Parent' />"
            "    </Source>"
            "    <Target cardinality='(0,N)' polymorphic='True'>"
            "      <Class class = 'Child' >"
            "           <Key>"
            "              <Property name='ParentId'/>"
            "           </Key>"
            "      </Class>"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", true, "");

        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testItem, ecdbName);
        ASSERT_FALSE(asserted);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "parent_id"));
        ASSERT_FALSE(ecdb.ColumnExists(childTableName, "ParentId"));
        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(3, columns.size()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it == columns.end()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

        AssertForeignKey(false, ecdb, childTableName);
        }

        {
        SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
            "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
            "  <ECClass typeName='Parent' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECClass>"
            "  <ECClass typeName='Child' >"
            "    <ECProperty propertyName='ParentId' typeName='long' />"
            "    <ECProperty propertyName='ChildName' typeName='string' />"
            "  </ECClass>"
            "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
            "    <ECCustomAttributes>"
            "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00' />"
            "    </ECCustomAttributes>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class = 'Parent' />"
            "    </Source>"
            "    <Target cardinality='(0,N)' polymorphic='True'>"
            "      <Class class = 'Child' >"
            "           <Key>"
            "              <Property name='ParentId'/>"
            "           </Key>"
            "      </Class>"
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
        ASSERT_EQ(3, columns.size()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it == columns.end()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

        AssertForeignKey(true, ecdb, childTableName, "ParentId");
        }

        {
        SchemaItem testItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
                          "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                          "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
                          "  <ECClass typeName='Authority' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECClass>"
                          "  <ECClass typeName='ElementCode' isDomainClass='False' isStruct='True'>"
                          "    <ECProperty propertyName='AuthorityId' typeName='int' />"
                          "    <ECProperty propertyName='Namespace' typeName='string' />"
                          "    <ECProperty propertyName='Code' typeName='string' />"
                          "  </ECClass>"
                          "  <ECClass typeName='Element' >"
                          "    <ECProperty propertyName='ModelId' typeName='long' />"
                          "    <ECStructProperty propertyName='Code' typeName='ElementCode' />"
                          "  </ECClass>"
                          "  <ECRelationshipClass typeName='AuthorityIssuesCode' isDomainClass='True' strength='referencing'>"
                          "    <ECCustomAttributes>"
                          "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
                          "             <OnDeleteAction>NoAction</OnDeleteAction>"
                          "        </ForeignKeyRelationshipMap>"
                          "    </ECCustomAttributes>"
                          "    <Source cardinality='(1,1)' polymorphic='False'>"
                          "        <Class class='Authority' />"
                          "     </Source>"
                          "     <Target cardinality='(0,N)' polymorphic='True'>"
                          "         <Class class='Element'>"
                          "             <Key>"
                          "                 <Property name='Code.AuthorityId'/>"
                          "             </Key>"
                          "         </Class>"
                          "     </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>", true, "");

        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testItem, ecdbName);
        ASSERT_FALSE(asserted);

        ASSERT_TRUE(ecdb.ColumnExists("ts_Element", "Code_AuthorityId"));
        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, "ts_Element"));
        ASSERT_EQ(5, columns.size()) << " ts_Element table should not contain an extra foreign key column as the relationship specifies a Key property";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it == columns.end()) << " ts_Element table should not contain an extra foreign key column as the relationship specifies a Key property";

        AssertForeignKey(true, ecdb, "ts_Element", "Code_AuthorityId");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, ForeignKeyMapWithECInstanceIdKeyProperty)
    {
    Utf8CP ecdbName = "ForeignKeyMapWithECInstanceIdKeyProp.ecdb";


    {
    std::vector<SchemaItem> testItems {SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
        "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "  <ECClass typeName='Parent' >"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECClass>"
        "  <ECClass typeName='Child' >"
        "    <ECProperty propertyName='ChildName' typeName='string' />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
        "    <ECCustomAttributes>"
        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
        "            <ForeignKeyColumn>ECInstanceId</ForeignKeyColumn>"
        "        </ForeignKeyRelationshipMap>"
        "    </ECCustomAttributes>"
        "    <Source cardinality='(1,1)' polymorphic='True'>"
        "      <Class class = 'Parent' />"
        "    </Source>"
        "    <Target cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Child' >"
        "           <Key>"
        "              <Property name='ECInstanceId'/>"
        "           </Key>"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>", false, ""),

        SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
        "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "  <ECClass typeName='Parent' >"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECClass>"
        "  <ECClass typeName='Child' >"
        "    <ECProperty propertyName='ChildName' typeName='string' />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
        "    <ECCustomAttributes>"
        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
        "            <ForeignKeyColumn>blabla</ForeignKeyColumn>"
        "        </ForeignKeyRelationshipMap>"
        "    </ECCustomAttributes>"
        "    <Source cardinality='(1,1)' polymorphic='True'>"
        "      <Class class = 'Parent' />"
        "    </Source>"
        "    <Target cardinality='(0,1)' polymorphic='True'>"
        "      <Class class = 'Child' >"
        "           <Key>"
        "              <Property name='ECInstanceId'/>"
        "           </Key>"
        "      </Class>"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>", false, "Value in ForeignKeyColumn property does not exist")};

    AssertSchemaImport(testItems, ecdbName);
    }

    Utf8CP childTableName = "ts_Child";

        {
        SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                          "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                          "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
                          "  <ECClass typeName='Parent' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECClass>"
                          "  <ECClass typeName='Child' >"
                          "    <ECProperty propertyName='ChildName' typeName='string' />"
                          "  </ECClass>"
                          "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
                          "    <Source cardinality='(1,1)' polymorphic='True'>"
                          "      <Class class = 'Parent' />"
                          "    </Source>"
                          "    <Target cardinality='(0,1)' polymorphic='True'>"
                          "      <Class class = 'Child' >"
                          "           <Key>"
                          "              <Property name='ECInstanceId'/>"
                          "           </Key>"
                          "      </Class>"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>", true, "");

        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testItem, ecdbName);
        ASSERT_FALSE(asserted);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ECInstanceId"));
        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(2, columns.size()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies that the ECInstanceId is the foreign key";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it == columns.end()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

        AssertForeignKey(false, ecdb, childTableName);
        }

        {
        SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                          "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                          "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
                          "  <ECClass typeName='Parent' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECClass>"
                          "  <ECClass typeName='Child' >"
                          "    <ECCustomAttributes>"
                          "        <ClassMap xmlns='ECDbMap.01.00'>"
                          "            <ECInstanceIdColumn>Id</ECInstanceIdColumn>"
                          "        </ClassMap>"
                          "    </ECCustomAttributes>"
                          "    <ECProperty propertyName='ChildName' typeName='string' />"
                          "  </ECClass>"
                          "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
                          "    <Source cardinality='(1,1)' polymorphic='True'>"
                          "      <Class class = 'Parent' />"
                          "    </Source>"
                          "    <Target cardinality='(0,1)' polymorphic='True'>"
                          "      <Class class = 'Child' >"
                          "           <Key>"
                          "              <Property name='ECInstanceId'/>"
                          "           </Key>"
                          "      </Class>"
                          "    </Target>"
                          "  </ECRelationshipClass>"
                          "</ECSchema>", true, "");

        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testItem, ecdbName);
        ASSERT_FALSE(asserted);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "Id"));
        ASSERT_FALSE(ecdb.ColumnExists(childTableName, "ECInstanceId"));

        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(2, columns.size()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies that the ECInstanceId is the foreign key";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it == columns.end()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

        AssertForeignKey(false, ecdb, childTableName);
        }

        {
        SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
            "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
            "  <ECClass typeName='Parent' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECClass>"
            "  <ECClass typeName='Child' >"
            "    <ECCustomAttributes>"
            "        <ClassMap xmlns='ECDbMap.01.00'>"
            "            <ECInstanceIdColumn>Id</ECInstanceIdColumn>"
            "        </ClassMap>"
            "    </ECCustomAttributes>"
            "    <ECProperty propertyName='ChildName' typeName='string' />"
            "  </ECClass>"
            "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
            "    <ECCustomAttributes>"
            "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00' />"
            "    </ECCustomAttributes>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class = 'Parent' />"
            "    </Source>"
            "    <Target cardinality='(0,1)' polymorphic='True'>"
            "      <Class class = 'Child' >"
            "           <Key>"
            "              <Property name='ECInstanceId'/>"
            "           </Key>"
            "      </Class>"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", true, "");

        ECDb ecdb;
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testItem, ecdbName);
        ASSERT_FALSE(asserted);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "Id"));
        ASSERT_FALSE(ecdb.ColumnExists(childTableName, "ECInstanceId"));

        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(2, columns.size()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies that the ECInstanceId is the foreign key";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it == columns.end()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

        AssertForeignKey(true, ecdb, childTableName, "Id");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, ForeignKeyMapWithoutKeyProperty)
    {
    Utf8CP ecdbName = "ForeignKeyMapWithoutKeyProp.ecdb";
    Utf8CP childTableName = "ts_Child";

        {
        SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                          "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                          "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
                          "  <ECClass typeName='Parent' >"
                          "    <ECProperty propertyName='Name' typeName='string' />"
                          "  </ECClass>"
                          "  <ECClass typeName='Child' >"
                          "    <ECProperty propertyName='ParentId' typeName='long' />"
                          "    <ECProperty propertyName='ChildName' typeName='string' />"
                          "  </ECClass>"
                          "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
                          "    <ECCustomAttributes>"
                          "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
                          "            <ForeignKeyColumn>ParentId</ForeignKeyColumn>"
                          "        </ForeignKeyRelationshipMap>"
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
        AssertSchemaImport(ecdb, asserted, testItem, ecdbName);
        ASSERT_FALSE(asserted);
        
        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(3, columns.size()) << childTableName << " table should not contain an extra foreign key column as the relationship map specifies an existing column name";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it == columns.end()) << childTableName << " table should not contain an extra foreign key column as the relationship map specifies an existing column name";

        AssertForeignKey(true, ecdb, childTableName);
        }

        {
        SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
            "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
            "  <ECClass typeName='Parent' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECClass>"
            "  <ECClass typeName='Child' >"
            "    <ECProperty propertyName='ParentId' typeName='long' />"
            "    <ECProperty propertyName='ChildName' typeName='string' />"
            "  </ECClass>"
            "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
            "    <ECCustomAttributes>"
            "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
            "            <ForeignKeyColumn>MyOwnParentId</ForeignKeyColumn>"
            "        </ForeignKeyRelationshipMap>"
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
        SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
            "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
            "  <ECClass typeName='Parent' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECClass>"
            "  <ECClass typeName='Child' >"
            "    <ECProperty propertyName='ParentId' typeName='long' />"
            "    <ECProperty propertyName='ChildName' typeName='string' />"
            "  </ECClass>"
            "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
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
        AssertSchemaImport(ecdb, asserted, testItem, ecdbName);
        ASSERT_FALSE(asserted);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(4, columns.size()) << childTableName << " table should contain a default-name extra foreign key column as there is no relationship map CA";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it != columns.end()) << "ts_child table should contain a default-name extra foreign key column as there is no relationship map CA";

        AssertForeignKey(false, ecdb, childTableName);
        }

        {
        SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
            "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
            "  <ECClass typeName='Parent' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECClass>"
            "  <ECClass typeName='Child' >"
            "    <ECProperty propertyName='ParentId' typeName='long' />"
            "    <ECProperty propertyName='ChildName' typeName='string' />"
            "  </ECClass>"
            "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
            "    <ECCustomAttributes>"
            "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
            "        </ForeignKeyRelationshipMap>"
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
        SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
            "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
            "  <ECClass typeName='Parent' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECClass>"
            "  <ECClass typeName='Child' >"
            "    <ECProperty propertyName='ParentId' typeName='long' />"
            "    <ECProperty propertyName='ChildName' typeName='string' />"
            "  </ECClass>"
            "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
            "    <ECCustomAttributes>"
            "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00' />"
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
        AssertSchemaImport(ecdb, asserted, testItem, ecdbName);
        ASSERT_FALSE(asserted);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(4, columns.size()) << childTableName << " table should contain a default-name extra foreign key column as there is the relationship map CA doesn't specify a value for ForeignKeyColumn";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it != columns.end()) << childTableName << " table should contain a default-name extra foreign key column as there is the relationship map CA doesn't specify a value for ForeignKeyColumn";

        AssertForeignKey(true, ecdb, childTableName, "ForeignEC");
        }

        {
        SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
            "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
            "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
            "  <ECClass typeName='Parent' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECClass>"
            "  <ECClass typeName='Child' >"
            "    <ECCustomAttributes>"
            "        <ClassMap xmlns='ECDbMap.01.00'>"
            "                <MapStrategy>"
            "                   <Strategy>SharedTable</Strategy>"
            "                   <AppliesToSubclasses>True</AppliesToSubclasses>"
            "                </MapStrategy>"
            "        </ClassMap>"
            "    </ECCustomAttributes>"
            "    <ECProperty propertyName='ParentId' typeName='long' />"
            "    <ECProperty propertyName='ChildName' typeName='string' />"
            "  </ECClass>"
            "  <ECClass typeName='Child2' >"
            "    <BaseClass>Child</BaseClass>"
            "    <ECProperty propertyName='Child2Name' typeName='string' />"
            "  </ECClass>"
            "  <ECRelationshipClass typeName='ParentHasChildren' isDomainClass='True' strength='referencing'>"
            "    <ECCustomAttributes>"
            "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00' />"
            "    </ECCustomAttributes>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
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

        AssertForeignKey(true, ecdb, childTableName, "ForeignEC");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbMappingTestFixture, RelationshipMapCAOnSubclasses)
    {
    SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                        "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
                        "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
                        "  <ECClass typeName='Element' >"
                        "    <ECCustomAttributes>"
                        "        <ClassMap xmlns='ECDbMap.01.00'>"
                        "            <MapStrategy>"
                        "               <Strategy>SharedTable</Strategy>"
                        "               <AppliesToSubclasses>True</AppliesToSubclasses>"
                        "               <Options>SharedColumnsForSubclasses</Options>"
                        "            </MapStrategy>"
                        "        </ClassMap>"
                        "    </ECCustomAttributes>"
                        "    <ECProperty propertyName='Code' typeName='string' />"
                        "  </ECClass>"
                        "  <ECClass typeName='MyElement' >"
                        "    <BaseClass>Element</BaseClass>"
                        "    <ECProperty propertyName='MyName' typeName='string' />"
                        "  </ECClass>"
                        "  <ECClass typeName='YourElement' >"
                        "    <BaseClass>Element</BaseClass>"
                        "    <ECProperty propertyName='YourName' typeName='string' />"
                        "  </ECClass>"
                        "  <ECRelationshipClass typeName='ElementOwnsChildElements' isDomainClass='False' />"
                        "  <ECRelationshipClass typeName='MyElementHasYourElements' isDomainClass='True' strength='embedding'>"
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
                        "            <ForeignKeyColumn>ParentId</ForeignKeyColumn>"
                        "        </ForeignKeyRelationshipMap>"
                        "    </ECCustomAttributes>"
                        "   <BaseClass>ElementOwnsChildElements</BaseClass>"
                        "    <Source cardinality='(1,1)' polymorphic='True'>"
                        "      <Class class = 'MyElement' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'YourElement' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>", true, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipMapCAOnSubclasses.ecdb");
    ASSERT_FALSE(asserted);

    bvector<Utf8String> columns;
    ASSERT_TRUE(ecdb.GetColumns(columns, "ts_Element"));
    auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
    auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
    ASSERT_TRUE(it == columns.end()) << "ts_Element table should not contain an extra foreign key column as the relationship map specifies to use the ParentId column";

    AssertForeignKey(true, ecdb, "ts_Element", "ParentId");
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
TEST_F(RelationshipsAndSharedTablesTestFixture, UniqueIndexesSupportFor1to1Relationship)
    {
    SchemaItem testItem(SCHEMA_XML, true);
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
    SchemaItem testItem(SCHEMA_XML, true);
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
    ASSERT_TRUE (BE_SQLITE_ROW == stmt.Step ());
    EXPECT_EQ (6, stmt.GetValueInt (0));
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE (BE_SQLITE_ROW == stmt.Step ());
    EXPECT_EQ (6, stmt.GetValueInt (0));
    stmt.Finalize ();

    //Deletes the instances of BaseOwnsBase class..
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "DELETE FROM ONLY t.BaseOwnsBase"));
    ASSERT_TRUE (BE_SQLITE_DONE == stmt.Step ());
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE (BE_SQLITE_ROW == stmt.Step ());
    EXPECT_EQ (4, stmt.GetValueInt (0));
    stmt.Finalize ();

    //Deletes the instances of BaseHasClassA class..
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "DELETE FROM ONLY t.BaseHasClassA"));
    ASSERT_TRUE (BE_SQLITE_DONE == stmt.Step ());
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE (BE_SQLITE_ROW == stmt.Step ());
    EXPECT_EQ (2, stmt.GetValueInt (0));
    stmt.Finalize ();

    //Deletes the instances of BaseHasClassB class..
    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "DELETE FROM ONLY t.BaseHasClassB"));
    ASSERT_TRUE (BE_SQLITE_DONE == stmt.Step ());
    stmt.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, stmt.Prepare (ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE (BE_SQLITE_ROW == stmt.Step ());
    EXPECT_EQ (0, stmt.GetValueInt (0));
    stmt.Finalize ();
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
    ASSERT_FALSE (ecdb.TableExists ("t_BaseHasClassA"));
    ASSERT_FALSE (ecdb.TableExists ("t_BaseHasClassB"));

    ECSqlStatement insertStatement;
    ECInstanceKey TPHKey1;
    ECInstanceKey TPHKey2;
    ASSERT_EQ (ECSqlStatus::Success, insertStatement.Prepare (ecdb, "INSERT INTO t.Base (P0) VALUES ('string1')"));
    ASSERT_EQ (BE_SQLITE_DONE, insertStatement.Step (TPHKey1));
    ASSERT_TRUE (TPHKey1.IsValid ());
    insertStatement.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, insertStatement.Prepare (ecdb, "INSERT INTO t.Base (P0) VALUES ('string2')"));
    ASSERT_EQ (BE_SQLITE_DONE, insertStatement.Step (TPHKey2));
    ASSERT_TRUE (TPHKey2.IsValid ());
    insertStatement.Finalize ();

    ECInstanceKey classAKey1;
    ECInstanceKey classAKey2;
    ASSERT_EQ (ECSqlStatus::Success, insertStatement.Prepare (ecdb, "INSERT INTO t.ClassA (P1) VALUES ('string1')"));
    ASSERT_EQ (BE_SQLITE_DONE, insertStatement.Step (classAKey1));
    ASSERT_TRUE (classAKey1.IsValid ());
    insertStatement.Finalize ();

    ASSERT_EQ (ECSqlStatus::Success, insertStatement.Prepare (ecdb, "INSERT INTO t.ClassA (P1) VALUES ('string2')"));
    ASSERT_EQ (BE_SQLITE_DONE, insertStatement.Step (classAKey2));
    ASSERT_TRUE (classAKey2.IsValid ());
    insertStatement.Finalize ();

    //retrieve ECInstance from Db before inserting Relationship Instance, based on ECInstanceId, verify ECInstance is valid
    ECSqlStatement selectStmt;
    ASSERT_EQ (ECSqlStatus::Success, selectStmt.Prepare (ecdb, "SELECT * FROM t.Base WHERE ECInstanceId = ?"));
    selectStmt.BindId (1, TPHKey1.GetECInstanceId ());
    ASSERT_EQ (BE_SQLITE_ROW, selectStmt.Step ());
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
    ASSERT_EQ (BE_SQLITE_DONE, relationStmt.Step ());
    relationStmt.Finalize ();

    //try to insert Duplicate relationship step() should return error
    ASSERT_EQ (relationStmt.Prepare (ecdb, "INSERT INTO t.BaseHasClassA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    relationStmt.BindId (1, TPHKey1.GetECInstanceId ());
    relationStmt.BindInt64 (2, TPHKey1.GetECClassId ());
    relationStmt.BindId (3, classAKey1.GetECInstanceId ());
    relationStmt.BindInt64 (4, classAKey1.GetECClassId ());
    ASSERT_TRUE ((BE_SQLITE_CONSTRAINT_BASE & relationStmt.Step ()) == BE_SQLITE_CONSTRAINT_BASE);
    relationStmt.Finalize ();

    //retrieve ECInstance from Db After Inserting Relationship Instance, based on ECInstanceId, verify ECInstance is valid
    ASSERT_EQ (ECSqlStatus::Success, selectStmt.Prepare (ecdb, "SELECT * FROM t.ClassA WHERE ECInstanceId = ?"));
    selectStmt.BindId (1, classAKey1.GetECInstanceId ());
    ASSERT_EQ (BE_SQLITE_ROW, selectStmt.Step ());
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
                        "<AppliesToSubclasses>True</AppliesToSubclasses>"
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
                        "<AppliesToSubclasses>True</AppliesToSubclasses>"
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
    ECDbTestProject test;
    ECDbR ecdb = test.Create ("ForeignKeyRelationshipMap_EnforceReferentialIntegrity.ecdb");
    ExecuteRelationshipInsertionIntegrityTest (ecdb, false, true, true);
    //when AllowDuplicate is turned of, OneFooHasManyGoo will also be mapped as endtable therefore ReferentialIntegrityCheck will be performed for it, so there will be two rows in the ForeignKey table
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasOneGoo"));
    ASSERT_FALSE (ecdb.TableExists ("ts_OneFooHasManyGoo"));

    BeSQLite::Statement sqlStatment;
    ASSERT_EQ(BE_SQLITE_OK, sqlStatment.Prepare (ecdb, "SELECT ec_Column.Name FROM ec_Column JOIN ec_ForeignKey ON ec_ForeignKey.TableId = ec_Column.[TableId] JOIN ec_ForeignKeyColumn ON ec_ForeignKeyColumn.ColumnId = ec_Column.Id WHERE ec_ForeignKey.Id = 1"));
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
            stmt.BindInt64(2, fooKey.GetECClassId());
            stmt.BindId(3, gooKey.GetECInstanceId());
            stmt.BindInt64(4, gooKey.GetECClassId());
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
    BackDoor::ECObjects::ECSchemaReadContext::AddSchema(*readContext, *testSchema);

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

    PersistedMapStrategy mapStrategy;
    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, ecdb, oneFooHasOneGoo->GetId()));
    ASSERT_EQ(PersistedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);
    ASSERT_EQ(count_OneFooHasOneGoo, GetRelationshipInstanceCount(ecdb, "ts.OneFooHasOneGoo"));

    //1:N--------------------------------
    size_t count_OneFooHasManyGoo = 0;
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.OneFooHasManyGoo", fooKeys, gooKeys, oneFooHasManyGooResult, count_OneFooHasManyGoo);

    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, ecdb, oneFooHasManyGoo->GetId()));
    ASSERT_EQ(PersistedMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);
    ASSERT_EQ(count_OneFooHasManyGoo, GetRelationshipInstanceCount(ecdb, "ts.OneFooHasManyGoo"));

    //N:N--------------------------------
    size_t count_ManyFooHasManyGoo = 0;
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, manyFooHasManyGooResult, count_ManyFooHasManyGoo);
    if (allowDuplicateRelationships)
        VerifyRelationshipInsertionIntegrity(ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, reinsertResultDone, count_ManyFooHasManyGoo);
    else
        VerifyRelationshipInsertionIntegrity(ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, reinsertResultError, count_ManyFooHasManyGoo);

    ASSERT_TRUE(TryGetPersistedMapStrategy(mapStrategy, ecdb, manyFooHasManyGoo->GetId()));

    ASSERT_EQ(PersistedMapStrategy::Strategy::OwnTable, mapStrategy.m_strategy);
    ASSERT_EQ(PersistedMapStrategy::Options::None, mapStrategy.m_options);
    ASSERT_FALSE(mapStrategy.m_appliesToSubclasses);
    ASSERT_EQ(count_ManyFooHasManyGoo, GetRelationshipInstanceCount(ecdb, "ts.ManyFooHasManyGoo"));
    }

END_ECDBUNITTESTS_NAMESPACE