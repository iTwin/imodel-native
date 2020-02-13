/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

struct RelationshipMappingTestFixture : ECDbTestFixture {};


//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                  02/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, InvalidCases)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='Price' typeName='double'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='Name' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='AHasB' modifier='None' strength='referencing' >"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>OwnTable</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "       <Source multiplicity='(0,1)' polymorphic='True' roleLabel='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0,N)' polymorphic='True' roleLabel='B'>"
        "           <Class class='B' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "    <ECRelationshipClass typeName='AHasB2' modifier='None' strength='referencing' >"
        "       <BaseClass>AHasB</BaseClass>"
        "       <Source multiplicity='(0,1)' polymorphic='True' roleLabel='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0,N)' polymorphic='True' roleLabel='B'>"
        "           <Class class='B' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>"))) << "BaseRelationshipClass having OwnTable mapping strategy is not supported in ECRelationshipClassHeirarchy.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "  <ECEntityClass typeName='Model' >"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Element' >"
        "    <ECProperty propertyName='Code' typeName='string' />"
        "    <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward'/>"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
        "    <ECCustomAttributes>"
        "        <LinkTableRelationshipMap xmlns='ECDbMap.02.00'/>"
        "    </ECCustomAttributes>"
        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
        "      <Class class='Model' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
        "      <Class class='Element' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot define a nav prop for a link table relationship class";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "  <ECEntityClass typeName='Model' >"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Element' >"
        "    <ECProperty propertyName='Code' typeName='string' />"
        "    <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward'>"
        "       <ECCustomAttributes>"
        "         <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
        "       </ECCustomAttributes>"
        "    </ECNavigationProperty>"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
        "    <ECCustomAttributes>"
        "        <LinkTableRelationshipMap xmlns='ECDbMap.02.00'/>"
        "    </ECCustomAttributes>"
        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
        "      <Class class='Model' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
        "      <Class class='Element' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot define a nav prop (with a ForeignKeyConstraint) for a link table relationship";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "  <ECEntityClass typeName='Parent' >"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Child' >"
        "    <ECProperty propertyName='ParentId' typeName='long' />"
        "    <ECProperty propertyName='ChildName' typeName='string' />"
        "    <ECNavigationProperty propertyName='Parent' relationshipName='ParentHasChildren' direction='Backward'/>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Child2' >"
        "    <ECProperty propertyName='ParentId' typeName='long' />"
        "    <ECProperty propertyName='ChildName' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='ParentHasChildren' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Parent' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Child' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot define a nav prop when a link table (implied by cardinality) is required.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "  <ECEntityClass typeName='Parent' >"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Child' >"
        "    <ECProperty propertyName='ParentId' typeName='long' />"
        "    <ECProperty propertyName='ChildName' typeName='string' />"
        "    <ECNavigationProperty propertyName='Parent' relationshipName='ParentHasChildren' direction='Backward'>"
        "      <ECCustomAttributes>"
        "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
        "      </ECCustomAttributes>"
        "    </ECNavigationProperty>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Child2' >"
        "    <ECProperty propertyName='ParentId' typeName='long' />"
        "    <ECProperty propertyName='ChildName' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='ParentHasChildren' strength='referencing' modifier='Sealed'>"
        "    <Source cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Parent' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'Child' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot define a nav prop (with ForeignKeyConstraint) when a link table (implied by cardinality) is required";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "  <ECEntityClass typeName='Parent' >"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Child' >"
        "    <ECProperty propertyName='ParentId' typeName='long' />"
        "    <ECProperty propertyName='ChildName' typeName='string' />"
        "    <ECNavigationProperty propertyName='Parent' relationshipName='ParentHasChildren' direction='Backward'/>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Child2' >"
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
        "    <ECProperty propertyName='ForcingToLinkTable' typeName='string' />"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot define a nav prop when a link table (implied by additional prop.) is required.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
        "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
        "  <ECEntityClass typeName='Parent' >"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Child' >"
        "    <ECProperty propertyName='ParentId' typeName='long' />"
        "    <ECProperty propertyName='ChildName' typeName='string' />"
        "    <ECNavigationProperty propertyName='Parent' relationshipName='ParentHasChildren' direction='Backward'>"
        "      <ECCustomAttributes>"
        "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
        "      </ECCustomAttributes>"
        "    </ECNavigationProperty>"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Child2' >"
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
        "    <ECProperty propertyName='ForcingToLinkTable' typeName='string' />"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "Cannot define a nav prop (with ForeignKeyConstraint) when a link table (implied by additional prop.) is required.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
        "  <ECEntityClass typeName='Model' >"
        "    <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Element' >"
        "    <ECProperty propertyName='Code' typeName='string' />"
        "    <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward'/>"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
        "    <ECCustomAttributes>"
        "        <ClassMap xmlns='ECDbMap.02.00'>"
        "            <MapStrategy>OwnTable</MapStrategy>"
        "        </ClassMap>"
        "    </ECCustomAttributes>"
        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
        "      <Class class='Model' />"
        "    </Source>"
        "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
        "      <Class class='Element' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>"))) << "ForeignKey mapping may not have the ClassMap CA.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                            "<ECSchema schemaName='Test' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                            "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                                            "  <ECEntityClass typeName='Model' >"
                                                            "    <ECProperty propertyName='Name' typeName='string' />"
                                                            "  </ECEntityClass>"
                                                            "  <ECEntityClass typeName='Element' modifier='Abstract' >"
                                                            "    <ECCustomAttributes>"
                                                            "         <ClassMap xmlns='ECDbMap.02.00'>"
                                                            "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                                            "        </ClassMap>"
                                                            "    </ECCustomAttributes>"
                                                            "    <ECProperty propertyName='Code' typeName='string' />"
                                                            "    <ECNavigationProperty propertyName='ModelId' relationshipName='ModelHasElements' direction='Backward' />"
                                                            "  </ECEntityClass>"
                                                            "  <ECEntityClass typeName='PhysicalElement'>"
                                                            "    <BaseClass>Element</BaseClass>"
                                                            "    <ECProperty propertyName='Geometry' typeName='Bentley.Geometry.Common.IGeometry' />"
                                                            "  </ECEntityClass>"
                                                            "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
                                                            "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
                                                            "      <Class class='Model' />"
                                                            "    </Source>"
                                                            "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
                                                            "      <Class class='Element' />"
                                                            "    </Target>"
                                                            "  </ECRelationshipClass>"
                                                            "  <ECRelationshipClass typeName='ModelHasPhysicalElements' strength='embedding' modifier='Sealed'>"
                                                            "    <ECCustomAttributes>"
                                                            "        <ClassMap xmlns='ECDbMap.02.00'>"
                                                            "                <MapStrategy>NotMapped</MapStrategy>"
                                                            "        </ClassMap>"
                                                            "    </ECCustomAttributes>"
                                                            "   <BaseClass>ModelHasElements</BaseClass>"
                                                            "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
                                                            "      <Class class='Model' />"
                                                            "    </Source>"
                                                            "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements (Reversed)'>"
                                                            "      <Class class='PhysicalElement' />"
                                                            "    </Target>"
                                                            "  </ECRelationshipClass>"
                                                            "  <ECRelationshipClass typeName='ModelHasPhysicalElements2' strength='embedding' modifier='Sealed'>"
                                                            "    <ECCustomAttributes>"
                                                            "        <ClassMap xmlns='ECDbMap.02.00'>"
                                                            "                <MapStrategy>NotMapped</MapStrategy>"
                                                            "        </ClassMap>"
                                                            "    </ECCustomAttributes>"
                                                            "   <BaseClass>ModelHasElements</BaseClass>"
                                                            "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Physical Elements'>"
                                                            "      <Class class='Model' />"
                                                            "    </Source>"
                                                            "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Physical Elements (Reversed)'>"
                                                            "      <Class class='PhysicalElement' />"
                                                            "    </Target>"
                                                            "  </ECRelationshipClass>"
                                                            "</ECSchema>"))) << "Subclasses of FK rels cannot have NotMapped map strategy";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECEntityClass typeName='A' modifier='Abstract'>"
        "        <ECProperty propertyName='Price' typeName='double'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='Name' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECRelationshipClass typeName='AHasB' modifier='None' strength='referencing' >"
        "       <Source multiplicity='(0,1)' polymorphic='True' roleLabel='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0,N)' polymorphic='True' roleLabel='B'>"
        "           <Class class='B' />"
        "       </Target>"
        "     </ECRelationshipClass>"
        "</ECSchema>"))) << "Source or target constraint classes are abstract without subclasses. Apply the MapStrategy 'TablePerHierarchy' to the abstract constraint class";

      }


//---------------------------------------------------------------------------------------
// @bsiMethod                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, NotMappedCATests)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                            "<ECSchema schemaName='Test1' alias='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                            "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                                            "    <ECEntityClass typeName='A' modifier='None'>"
                                                            "        <ECProperty propertyName='AProp' typeName='int' />"
                                                            "    </ECEntityClass>"
                                                            "    <ECEntityClass typeName='B' modifier='None'>"
                                                            "        <ECProperty propertyName='BProp' typeName='int' />"
                                                            "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward'/>"
                                                            "    </ECEntityClass>"
                                                            "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                                                            "        <ECCustomAttributes>"
                                                            "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                            "                <MapStrategy>NotMapped</MapStrategy>"
                                                            "            </ClassMap>"
                                                            "        </ECCustomAttributes>"
                                                            "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
                                                            "           <Class class='A' />"
                                                            "       </Source>"
                                                            "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='is referenced by'>"
                                                            "           <Class class='B' />"
                                                            "       </Target>"
                                                            "     </ECRelationshipClass>"
                                                            "</ECSchema>"))) << "ECRelationshipClass with FK mapping cannot have a ClassMap CA with MapStrategy NotMapped. The Navigation Property class must have NotMapped instead";

    {
    ASSERT_EQ(SUCCESS, SetupECDb("notmappedcatests.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                     "<ECSchema schemaName='Test2' alias='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                                     "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                                                     "    <ECEntityClass typeName='A' modifier='None'>"
                                                                     "        <ECProperty propertyName='AProp' typeName='int' />"
                                                                     "    </ECEntityClass>"
                                                                     "    <ECEntityClass typeName='B' modifier='None'>"
                                                                     "        <ECCustomAttributes>"
                                                                     "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                                     "                <MapStrategy>NotMapped</MapStrategy>"
                                                                     "            </ClassMap>"
                                                                     "        </ECCustomAttributes>"
                                                                     "        <ECProperty propertyName='BProp' typeName='int' />"
                                                                     "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward'/>"
                                                                     "    </ECEntityClass>"
                                                                     "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                                                                     "        <ECCustomAttributes>"
                                                                     "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                                     "                <MapStrategy>NotMapped</MapStrategy>"
                                                                     "            </ClassMap>"
                                                                     "        </ECCustomAttributes>"
                                                                     "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
                                                                     "           <Class class='A' />"
                                                                     "       </Source>"
                                                                     "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='is referenced by'>"
                                                                     "           <Class class='B' />"
                                                                     "       </Target>"
                                                                     "     </ECRelationshipClass>"
                                                                     "</ECSchema>"))) << "Nav prop class has NotMapped strategy, rel has it";
    EXPECT_EQ(MapStrategy::OwnTable, GetHelper().GetMapStrategy(m_ecdb.Schemas().GetClassId("Test2", "A")).GetStrategy());
    EXPECT_EQ(MapStrategy::NotMapped, GetHelper().GetMapStrategy(m_ecdb.Schemas().GetClassId("Test2", "B")).GetStrategy());
    EXPECT_EQ(MapStrategy::NotMapped, GetHelper().GetMapStrategy(m_ecdb.Schemas().GetClassId("Test2", "Rel")).GetStrategy());
    }

    ASSERT_EQ(ERROR, SetupECDb("notmappedcatests.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                     "<ECSchema schemaName='Test3' alias='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                                     "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                                                     "    <ECEntityClass typeName='A' modifier='None'>"
                                                                     "        <ECProperty propertyName='AProp' typeName='int' />"
                                                                     "    </ECEntityClass>"
                                                                     "    <ECEntityClass typeName='B' modifier='None'>"
                                                                     "        <ECCustomAttributes>"
                                                                     "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                                     "                <MapStrategy>NotMapped</MapStrategy>"
                                                                     "            </ClassMap>"
                                                                     "        </ECCustomAttributes>"
                                                                     "        <ECProperty propertyName='BProp' typeName='int' />"
                                                                     "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward'/>"
                                                                     "    </ECEntityClass>"
                                                                     "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                                                                     "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
                                                                     "           <Class class='A' />"
                                                                     "       </Source>"
                                                                     "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='is referenced by'>"
                                                                     "           <Class class='B' />"
                                                                     "       </Target>"
                                                                     "     </ECRelationshipClass>"
                                                                     "</ECSchema>"))) << "Nav prop class has NotMapped strategy, rel doesn't";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                            "<ECSchema schemaName='Test4' alias='ts4' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                            "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                                            "    <ECEntityClass typeName='A' modifier='None'>"
                                                            "        <ECCustomAttributes>"
                                                            "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                            "                <MapStrategy>NotMapped</MapStrategy>"
                                                            "            </ClassMap>"
                                                            "        </ECCustomAttributes>"
                                                            "        <ECProperty propertyName='AProp' typeName='int' />"
                                                            "    </ECEntityClass>"
                                                            "    <ECEntityClass typeName='B' modifier='None'>"
                                                            "        <ECProperty propertyName='BProp' typeName='int' />"
                                                            "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward'/>"
                                                            "    </ECEntityClass>"
                                                            "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                                                            "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
                                                            "           <Class class='A' />"
                                                            "       </Source>"
                                                            "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='is referenced by'>"
                                                            "           <Class class='B' />"
                                                            "       </Target>"
                                                            "     </ECRelationshipClass>"
                                                            "</ECSchema>"))) << "Parent class has NotMapped strategy, rel doesn't.";

    ASSERT_EQ(ERROR, SetupECDb("notmappedcatests.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                     "<ECSchema schemaName='Test5' alias='ts5' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                                     "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                                                     "    <ECEntityClass typeName='A' modifier='None'>"
                                                                     "        <ECProperty propertyName='AProp' typeName='int' />"
                                                                     "    </ECEntityClass>"
                                                                     "    <ECEntityClass typeName='B' modifier='None'>"
                                                                     "        <ECCustomAttributes>"
                                                                     "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                                     "                <MapStrategy>NotMapped</MapStrategy>"
                                                                     "            </ClassMap>"
                                                                     "        </ECCustomAttributes>"
                                                                     "        <ECProperty propertyName='BProp' typeName='int' />"
                                                                     "    </ECEntityClass>"
                                                                     "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                                                                     "       <Source multiplicity='(0..*)' polymorphic='True' roleLabel='references'>"
                                                                     "           <Class class='A' />"
                                                                     "       </Source>"
                                                                     "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='is referenced by'>"
                                                                     "           <Class class='B' />"
                                                                     "       </Target>"
                                                                     "     </ECRelationshipClass>"
                                                                     "</ECSchema>"))) << "Target constraint class of Link Table rel has NotMapped";

    ASSERT_EQ(ERROR, SetupECDb("notmappedcatests.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                   "<ECSchema schemaName='Test6' alias='ts6' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                                   "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                                                                   "    <ECEntityClass typeName='A' modifier='None'>"
                                                                   "        <ECProperty propertyName='AProp' typeName='int' />"
                                                                   "    </ECEntityClass>"
                                                                   "    <ECEntityClass typeName='BaseB' modifier='None'>"
                                                                   "        <ECCustomAttributes>"
                                                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                                   "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                                                   "            </ClassMap>"
                                                                   "        </ECCustomAttributes>"
                                                                   "        <ECProperty propertyName='AProp' typeName='int' />"
                                                                   "    </ECEntityClass>"
                                                                   "    <ECEntityClass typeName='B1' modifier='None'>"
                                                                   "        <ECCustomAttributes>"
                                                                   "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                                   "                <MapStrategy>NotMapped</MapStrategy>"
                                                                   "            </ClassMap>"
                                                                   "        </ECCustomAttributes>"
                                                                   "        <BaseClass>BaseB</BaseClass>"
                                                                   "        <ECProperty propertyName='B1Prop' typeName='int' />"
                                                                   "    </ECEntityClass>"
                                                                   "    <ECEntityClass typeName='B2' modifier='None'>"
                                                                   "        <BaseClass>BaseB</BaseClass>"
                                                                   "        <ECProperty propertyName='B2Prop' typeName='int' />"
                                                                   "    </ECEntityClass>"
                                                                   "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                                                                   "       <Source multiplicity='(0..*)' polymorphic='True' roleLabel='references'>"
                                                                   "           <Class class='A' />"
                                                                   "       </Source>"
                                                                   "       <Target multiplicity='(0..*)' polymorphic='True' abstractConstraintClass='BaseB' roleLabel='is referenced by'>"
                                                                   "           <Class class='B1' />"
                                                                   "           <Class class='B2' />"
                                                                   "       </Target>"
                                                                   "     </ECRelationshipClass>"
                                                                   "</ECSchema>"))) << "One of the two Target constraint class of Link Table rel has NotMapped";

    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("notmappedcatests.ecdb"));

    ASSERT_EQ(SUCCESS, GetHelper().ImportSchemas({SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                                              <ECSchema schemaName="Test7" alias="ts7" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                               <ECEntityClass typeName="A" modifier="None">
                                                                    <ECProperty propertyName="AProp" typeName="int" />
                                                               </ECEntityClass>
                                                               <ECEntityClass typeName="B" modifier="None">
                                                                   <ECProperty propertyName="BProp" typeName="int" />
                                                                   <ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward"/>
                                                                </ECEntityClass>
                                                                <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="referencing">
                                                                  <Source multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                                                                    <Class class="A" />
                                                                  </Source>
                                                                  <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is referenced by">
                                                                    <Class class="B" />
                                                                  </Target>
                                                                 </ECRelationshipClass>
                                                             </ECSchema>)xml"),
                                                        SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                                              <ECSchema schemaName="Test7_Supp" alias="ts7_supp" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                                                               <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />

                                                               <ECCustomAttributes>
                                                                    <SupplementalSchema xmlns="CoreCustomAttributes.01.00">
                                                                        <PrimarySchemaReference>
                                                                            <SchemaName>Test7</SchemaName>
                                                                            <MajorVersion>1</MajorVersion>
                                                                            <WriteVersion>0</WriteVersion>
                                                                            <MinorVersion>0</MinorVersion>
                                                                        </PrimarySchemaReference>
                                                                        <Precedence>9900</Precedence>
                                                                        <Purpose>Localization</Purpose>                                                             
                                                                    </SupplementalSchema>
                                                               </ECCustomAttributes>

                                                               <ECEntityClass typeName="A" modifier="None">
                                                                    <ECProperty propertyName="AProp" typeName="int" />
                                                               </ECEntityClass>
                                                               <ECEntityClass typeName="B" modifier="None">
                                                                   <ECCustomAttributes>
                                                                       <ClassMap xmlns="ECDbMap.02.00">
                                                                           <MapStrategy>NotMapped</MapStrategy>
                                                                       </ClassMap>
                                                                   </ECCustomAttributes>
                                                                   <ECProperty propertyName="BProp" typeName="int" />
                                                                   <ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward"/>
                                                                </ECEntityClass>
                                                                <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="referencing">
                                                                   <ECCustomAttributes>
                                                                       <ClassMap xmlns="ECDbMap.02.00">
                                                                           <MapStrategy>NotMapped</MapStrategy>
                                                                       </ClassMap>
                                                                   </ECCustomAttributes>
                                                                  <Source multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                                                                    <Class class="A" />
                                                                  </Source>
                                                                  <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is referenced by">
                                                                    <Class class="B" />
                                                                  </Target>
                                                                 </ECRelationshipClass>
                                                             </ECSchema>)xml")})) << "Schema import where NotMapped is applied via supplementation";
    
    EXPECT_EQ(MapStrategy::OwnTable, GetHelper().GetMapStrategy(m_ecdb.Schemas().GetClassId("Test7", "A")).GetStrategy());
    EXPECT_EQ(MapStrategy::NotMapped, GetHelper().GetMapStrategy(m_ecdb.Schemas().GetClassId("Test7", "B")).GetStrategy());
    EXPECT_EQ(MapStrategy::NotMapped, GetHelper().GetMapStrategy(m_ecdb.Schemas().GetClassId("Test7", "Rel")).GetStrategy());
    }

    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("notmappedcatests.ecdb"));

    ASSERT_EQ(ERROR, GetHelper().ImportSchemas({SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                                              <ECSchema schemaName="Test8" alias="ts8" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                               <ECEntityClass typeName="A" modifier="None">
                                                                    <ECProperty propertyName="AProp" typeName="int" />
                                                               </ECEntityClass>
                                                               <ECEntityClass typeName="B" modifier="None">
                                                                   <ECProperty propertyName="BProp" typeName="int" />
                                                                   <ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward"/>
                                                                </ECEntityClass>
                                                                <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="referencing">
                                                                  <Source multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                                                                    <Class class="A" />
                                                                  </Source>
                                                                  <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is referenced by">
                                                                    <Class class="B" />
                                                                  </Target>
                                                                 </ECRelationshipClass>
                                                             </ECSchema>)xml"),
                                                 SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                                              <ECSchema schemaName="Test8_Supp" alias="ts8_supp" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                                                               <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />

                                                               <ECCustomAttributes>
                                                                    <SupplementalSchema xmlns="CoreCustomAttributes.01.00">
                                                                        <PrimarySchemaReference>
                                                                            <SchemaName>Test8</SchemaName>
                                                                            <MajorVersion>1</MajorVersion>
                                                                            <WriteVersion>0</WriteVersion>
                                                                            <MinorVersion>0</MinorVersion>
                                                                        </PrimarySchemaReference>
                                                                        <Precedence>9900</Precedence>
                                                                        <Purpose>Localization</Purpose>                                                             
                                                                    </SupplementalSchema>
                                                               </ECCustomAttributes>

                                                               <ECEntityClass typeName="A" modifier="None">
                                                                    <ECProperty propertyName="AProp" typeName="int" />
                                                               </ECEntityClass>
                                                               <ECEntityClass typeName="B" modifier="None">
                                                                   <ECProperty propertyName="BProp" typeName="int" />
                                                                   <ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward"/>
                                                                </ECEntityClass>
                                                                <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="referencing">
                                                                   <ECCustomAttributes>
                                                                       <ClassMap xmlns="ECDbMap.02.00">
                                                                           <MapStrategy>NotMapped</MapStrategy>
                                                                       </ClassMap>
                                                                   </ECCustomAttributes>
                                                                  <Source multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                                                                    <Class class="A" />
                                                                  </Source>
                                                                  <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is referenced by">
                                                                    <Class class="B" />
                                                                  </Target>
                                                                 </ECRelationshipClass>
                                                             </ECSchema>)xml")})) << "NotMapped is applied via supplementation only to rel, not to nav prop class";
    }

    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("notmappedcatests.ecdb"));

    ASSERT_EQ(ERROR, GetHelper().ImportSchemas({SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                                              <ECSchema schemaName="Test9" alias="ts9" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                               <ECEntityClass typeName="A" modifier="None">
                                                                    <ECProperty propertyName="AProp" typeName="int" />
                                                               </ECEntityClass>
                                                               <ECEntityClass typeName="B" modifier="None">
                                                                   <ECProperty propertyName="BProp" typeName="int" />
                                                                   <ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward"/>
                                                                </ECEntityClass>
                                                                <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="referencing">
                                                                  <Source multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                                                                    <Class class="A" />
                                                                  </Source>
                                                                  <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is referenced by">
                                                                    <Class class="B" />
                                                                  </Target>
                                                                 </ECRelationshipClass>
                                                             </ECSchema>)xml"),
                                               SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                                              <ECSchema schemaName="Test9_Supp" alias="ts9_supp" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                                                               <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />

                                                               <ECCustomAttributes>
                                                                    <SupplementalSchema xmlns="CoreCustomAttributes.01.00">
                                                                        <PrimarySchemaReference>
                                                                            <SchemaName>Test9</SchemaName>
                                                                            <MajorVersion>1</MajorVersion>
                                                                            <WriteVersion>0</WriteVersion>
                                                                            <MinorVersion>0</MinorVersion>
                                                                        </PrimarySchemaReference>
                                                                        <Precedence>9900</Precedence>
                                                                        <Purpose>Localization</Purpose>                                                             
                                                                    </SupplementalSchema>
                                                               </ECCustomAttributes>

                                                               <ECEntityClass typeName="A" modifier="None">
                                                                    <ECProperty propertyName="AProp" typeName="int" />
                                                               </ECEntityClass>
                                                               <ECEntityClass typeName="B" modifier="None">
                                                                   <ECCustomAttributes>
                                                                       <ClassMap xmlns="ECDbMap.02.00">
                                                                           <MapStrategy>NotMapped</MapStrategy>
                                                                       </ClassMap>
                                                                   </ECCustomAttributes>
                                                                   <ECProperty propertyName="BProp" typeName="int" />
                                                                   <ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward"/>
                                                                </ECEntityClass>
                                                                <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="referencing">
                                                                  <Source multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                                                                    <Class class="A" />
                                                                  </Source>
                                                                  <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is referenced by">
                                                                    <Class class="B" />
                                                                  </Target>
                                                                 </ECRelationshipClass>
                                                             </ECSchema>)xml")})) << "NotMapped is applied via supplementation only to nav prop class, not to rel class";
    }
    {
    ASSERT_EQ(SUCCESS, SetupECDb("notmappedcatests.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                     "<ECSchema schemaName='Test10' alias='ts10' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                                     "    <ECEntityClass typeName='A' modifier='None'>"
                                                                     "        <ECProperty propertyName='AProp' typeName='int' />"
                                                                     "    </ECEntityClass>"
                                                                     "    <ECEntityClass typeName='B' modifier='None'>"
                                                                     "        <ECProperty propertyName='BProp' typeName='int' />"
                                                                     "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward'/>"
                                                                     "    </ECEntityClass>"
                                                                     "    <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='referencing'>"
                                                                     "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='references'>"
                                                                     "           <Class class='A' />"
                                                                     "       </Source>"
                                                                     "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='is referenced by'>"
                                                                     "           <Class class='B' />"
                                                                     "       </Target>"
                                                                     "     </ECRelationshipClass>"
                                                                     "</ECSchema>")));

    ASSERT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                                              <ECSchema schemaName="Test10" alias="ts10" version="1.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                               <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                                                               <ECEntityClass typeName="A" modifier="None">
                                                                    <ECProperty propertyName="AProp" typeName="int" />
                                                               </ECEntityClass>
                                                               <ECEntityClass typeName="B" modifier="None">
                                                                   <ECCustomAttributes>
                                                                       <ClassMap xmlns="ECDbMap.02.00">
                                                                           <MapStrategy>NotMapped</MapStrategy>
                                                                       </ClassMap>
                                                                   </ECCustomAttributes>
                                                                   <ECProperty propertyName="BProp" typeName="int" />
                                                                   <ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward"/>
                                                                </ECEntityClass>
                                                                <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="referencing">
                                                                   <ECCustomAttributes>
                                                                       <ClassMap xmlns="ECDbMap.02.00">
                                                                           <MapStrategy>NotMapped</MapStrategy>
                                                                       </ClassMap>
                                                                   </ECCustomAttributes>
                                                                  <Source multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                                                                    <Class class="A" />
                                                                  </Source>
                                                                  <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is referenced by">
                                                                    <Class class="B" />
                                                                  </Target>
                                                                 </ECRelationshipClass>
                                                             </ECSchema>)xml"))) << "Changing map strategy to NotMapped in schema update is not supported";
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                     Krischan.Eberle                  10/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, LinkTableConstraintClassIdCols)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("LinkTableConstraintClassIdCols.ecdb",SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                        <ECSchema schemaName="Test" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                                            <ECEntityClass typeName="BaseA" modifier="None">
                                                 <ECCustomAttributes>
                                                      <ClassMap xmlns="ECDbMap.02.00">
                                                          <MapStrategy>TablePerHierarchy</MapStrategy>
                                                       </ClassMap>
                                                 </ECCustomAttributes>
                                                <ECProperty propertyName="BaseAProp1" typeName="int" />
                                            </ECEntityClass>
                                            <ECEntityClass typeName="SubA" modifier="None">
                                                <BaseClass>BaseA</BaseClass>
                                                <ECProperty propertyName="SubAProp1" typeName="int" />
                                            </ECEntityClass>
                                            <ECEntityClass typeName="BaseB" modifier="None">
                                                 <ECCustomAttributes>
                                                      <ClassMap xmlns="ECDbMap.02.00">
                                                          <MapStrategy>TablePerHierarchy</MapStrategy>
                                                       </ClassMap>
                                                 </ECCustomAttributes>
                                                <ECProperty propertyName="BaseBProp1" typeName="int" />
                                            </ECEntityClass>
                                            <ECEntityClass typeName="SubB" modifier="None">
                                                <BaseClass>BaseB</BaseClass>
                                                <ECProperty propertyName="SubBProp1" typeName="int" />
                                            </ECEntityClass>
                                            <ECRelationshipClass typeName="Rel" modifier="None" strength="referencing">
                                               <Source multiplicity="(0..*)" polymorphic="True" roleLabel="references">
                                                   <Class class="BaseA" />
                                               </Source>
                                               <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is referenced by">
                                                   <Class class="BaseB" />
                                               </Target>
                                             </ECRelationshipClass>
                                        </ECSchema>)xml")));

    ASSERT_EQ(std::vector<Utf8String>({"Id","ECClassId","SourceId","TargetId"}), GetHelper().GetColumnNames("ts_Rel")) << "Link Table is not expected to have constraint class id columns";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, CascadeDeletion)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("CascadeDeletion.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                        "    <ECEntityClass typeName='ClassA' modifier='None'>"
                        "        <ECProperty propertyName='AA' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='ClassB' modifier='None'>"
                        "        <ECProperty propertyName='BB' typeName='string' />"
                        "        <ECNavigationProperty propertyName='A' relationshipName='AHasB' direction='Backward'>"
                        "          <ECCustomAttributes>"
                        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                        "               <OnDeleteAction>Cascade</OnDeleteAction>"
                        "            </ForeignKeyConstraint>"
                        "          </ECCustomAttributes>"
                        "        </ECNavigationProperty>"
                        "    </ECEntityClass>"
                        "    <ECRelationshipClass typeName='AHasB' modifier='Sealed' strength='embedding'>"
                        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                        "           <Class class='ClassA' />"
                        "       </Source>"
                        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target'>"
                        "           <Class class='ClassB' />"
                        "       </Target>"
                        "     </ECRelationshipClass>"
                        "    <ECEntityClass typeName='ClassC' modifier='None'>"
                        "        <ECProperty propertyName='CC' typeName='string' />"
                        "        <ECNavigationProperty propertyName='B' relationshipName='BHasC' direction='Backward'>"
                        "          <ECCustomAttributes>"
                        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                        "               <OnDeleteAction>Cascade</OnDeleteAction>"
                        "            </ForeignKeyConstraint>"
                        "          </ECCustomAttributes>"
                        "        </ECNavigationProperty>"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='BHasC' modifier='Sealed' strength='embedding'>"
                        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                        "      <Class class = 'ClassB' />"
                        "    </Source>"
                        "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target'>"
                        "      <Class class = 'ClassC' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>")));

    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("TestSchema");
    EXPECT_TRUE(schema != nullptr);

    ECClassCP ClassA = schema->GetClassCP("ClassA");
    EXPECT_TRUE(ClassA != nullptr);

    ECClassCP ClassB = schema->GetClassCP("ClassB");
    EXPECT_TRUE(ClassB != nullptr);

    ECClassCP ClassC = schema->GetClassCP("ClassC");
    EXPECT_TRUE(ClassC != nullptr);

    //Instance of ClassA
    ECInstanceKey aKey, bKey, cKey;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.ClassA(AA) VALUES('val1')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(aKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.ClassB(BB, A.Id) VALUES('val3',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, aKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(bKey));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.ClassC(CC, B.Id) VALUES('val5',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, bKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(cKey));
    stmt.Finalize();

    //Deletes instance of ClassA. Instances of ClassB and ClassC are also deleted.
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ts.ClassA WHERE ECInstanceId=1"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT AA FROM ts.ClassA"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    ASSERT_EQ(NULL, stmt.GetValueText(0));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT BB FROM ts.ClassB"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    ASSERT_EQ(NULL, stmt.GetValueText(0));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT CC FROM ts.ClassC"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, stmt.Step());
    ASSERT_EQ(NULL, stmt.GetValueText(0));
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MultipleFkEndTables)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("multiplefkendtables.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                            <ECEntityClass typeName="Parent">
                              <ECProperty propertyName="Code" typeName="int" />
                            </ECEntityClass>
                            <ECEntityClass typeName="Child">
                              <ECProperty propertyName="Name" typeName="string" />
                              <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward">
                                <ECCustomAttributes>
                                    <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                                </ECCustomAttributes>
                              </ECNavigationProperty>
                            </ECEntityClass>
                            <ECEntityClass typeName="SpecialChild">
                              <BaseClass>Child</BaseClass>
                              <ECProperty propertyName="SpecialName" typeName="string" />
                            </ECEntityClass>
                            <ECRelationshipClass typeName="Rel" strength="Referencing" modifier="Sealed">
                                <Source multiplicity="(0..1)" polymorphic="False" roleLabel="Parent">
                                   <Class class ="Parent" />
                                </Source>
                                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Children">
                                   <Class class ="Child" />
                                </Target>
                            </ECRelationshipClass>
                        </ECSchema>)xml")));

    ECInstanceKey parentKey, childKey, specialChildKey;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Parent(Code) VALUES(1)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(parentKey)) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Child(Name, MyParent.Id) VALUES('Child1',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, parentKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(childKey)) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.SpecialChild(Name,SpecialName, MyParent.Id) VALUES('Child2','I am special',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, parentKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(specialChildKey)) << stmt.GetECSql();
    stmt.Finalize();
    
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.Rel(SourceECInstanceId, TargetECInstanceId) VALUES(?,?)"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, Name FROM ts.Child ORDER BY ECInstanceId"));
    int rowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        rowCount++;
        switch (rowCount)
            {
                case 1:
                {
                ASSERT_EQ(childKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue()) << "Child 1";
                ASSERT_EQ(childKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue()) << "Child 1";
                ASSERT_STREQ("Child1", stmt.GetValueText(2)) << "Child 1";
                break;
                }
                case 2:
                {
                ASSERT_EQ(specialChildKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue()) << "Child 2, SpecialChild";
                ASSERT_EQ(specialChildKey.GetClassId().GetValue(), stmt.GetValueId<ECClassId>(1).GetValue()) << "Child 2, SpecialChild";
                ASSERT_STREQ("Child2", stmt.GetValueText(2)) << "Child 2, SpecialChild";
                break;
                }

                default:
                    FAIL();
                    return;
            }
        }
    ASSERT_EQ(2, rowCount);
    stmt.Finalize();
    m_ecdb.SaveChanges();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId, TargetECInstanceId, TargetECClassId FROM ts.Rel ORDER BY TargetECClassId"));
    rowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        rowCount++;
        ASSERT_EQ(parentKey.GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue()) << stmt.GetECSql();
        ECInstanceKey const* expectedChildKey = rowCount == 1 ? &childKey : &specialChildKey;
        ASSERT_EQ(expectedChildKey->GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(1).GetValue()) << stmt.GetECSql();
        ASSERT_EQ(expectedChildKey->GetClassId().GetValue(), stmt.GetValueId<ECClassId>(2).GetValue()) << stmt.GetECSql();
        }
    ASSERT_EQ(2, rowCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MultipleConstraintClasses)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("multipleconstraintclasses.ecdb",
                        SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                            <ECEntityClass typeName="Parent">
                              <ECProperty propertyName="Code" typeName="int" />
                            </ECEntityClass>
                            <ECEntityClass typeName="Child">
                                <ECCustomAttributes>
                                    <ClassMap xmlns="ECDbMap.02.00">
                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                                </ECCustomAttributes>
                                <ECProperty propertyName="ChildProp" typeName="int" />
                             </ECEntityClass>
                             <ECEntityClass typeName="GrandchildA" >
                                <BaseClass>Child</BaseClass>
                                <ECProperty propertyName="GrandchildAProp" typeName="int" />
                                <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                             </ECEntityClass>
                             <ECEntityClass typeName="GrandchildB" >
                                <BaseClass>Child</BaseClass>
                                <ECProperty propertyName="GrandchildBProp" typeName="int" />
                                <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                            </ECEntityClass>
                            <ECRelationshipClass typeName="Rel" strength="referencing" modifier="Sealed">
                                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="Parent Has Grandchildren">
                                    <Class class="Parent" />
                                </Source>
                                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Parent Has Grandchildren (Reversed)" abstractConstraint="Child">
                                    <Class class="GrandchildA" />
                                    <Class class="GrandchildB" />
                                </Target>
                             </ECRelationshipClass>
                        </ECSchema>)xml")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, LogicalForeignKeyRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("LogicalForeignKeyRelationship.ecdb",
              SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                         "  <ECCustomAttributeClass typeName='Interface' appliesTo='EntityClass' modifier='Sealed' />"
                         "  <ECEntityClass typeName='PrimaryClassA'>"
                         "      <ECCustomAttributes>"
                         "          <ClassMap xmlns='ECDbMap.02.00'>"
                         "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "          </ClassMap>"
                         "          <ShareColumns xmlns='ECDbMap.02.00'>"
                         "              <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>" //
                         "              <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
                         "          </ShareColumns>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='P1' typeName='long' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='SecondaryClassA'>"
                         "      <ECCustomAttributes>"
                         "          <ClassMap xmlns='ECDbMap.02.00'>"
                         "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "          </ClassMap>"
                         "          <ShareColumns xmlns='ECDbMap.02.00'>"
                         "              <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>" //
                         "              <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
                         "          </ShareColumns>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='T1' typeName='long' />"
                         "      <ECNavigationProperty propertyName='PrimaryClassA' relationshipName='PrimaryClassAHasSecondaryClassA' direction='Backward' />"
                         "  </ECEntityClass>"
                         "   <ECRelationshipClass typeName='PrimaryClassAHasSecondaryClassA' strength='Referencing' modifier='Abstract'>"
                         "      <Source cardinality='(0,1)' polymorphic='False'>"
                         "          <Class class ='PrimaryClassA' />"
                         "      </Source>"
                         "      <Target cardinality='(0,N)' polymorphic='False'>"
                         "          <Class class ='SecondaryClassA' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "   <ECRelationshipClass typeName='PrimaryClassAHasSecondaryClassB' strength='Referencing' modifier='Sealed'>"
                         "       <BaseClass>PrimaryClassAHasSecondaryClassA</BaseClass> "
                         "      <Source cardinality='(0,1)' polymorphic='False'>"
                         "          <Class class ='PrimaryClassA' />"
                         "      </Source>"
                         "      <Target cardinality='(0,N)' polymorphic='False'>"
                         "          <Class class ='SecondaryClassA' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "</ECSchema>")));
    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();
    ECClassId primaryClassAHasSecondaryClassAId = m_ecdb.Schemas().GetClassId("TestSchema", "PrimaryClassAHasSecondaryClassA");
    ECClassId primaryClassAHasSecondaryClassBId = m_ecdb.Schemas().GetClassId("TestSchema", "PrimaryClassAHasSecondaryClassB");

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(101, 10000)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(102, 20000)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(103, 30000)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(104, 40000)"));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(Utf8PrintfString("INSERT INTO ts.SecondaryClassA(ECInstanceId, T1, PrimaryClassA.Id, PrimaryClassA.RelECClassId) VALUES(201, 10000, 101, %ld)", primaryClassAHasSecondaryClassBId.GetValue()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SecondaryClassA(ECInstanceId, T1, PrimaryClassA.Id) VALUES(202, 20000, 102)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SecondaryClassA(ECInstanceId, T1) VALUES(203, 30000)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.SecondaryClassA(ECInstanceId, T1) VALUES(204, 40000)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(Utf8PrintfString("UPDATE ts.SecondaryClassA SET PrimaryClassA.Id = 103, T1=300002, PrimaryClassA.RelECClassId = %ld  WHERE ECInstanceId = 203", primaryClassAHasSecondaryClassBId.GetValue()).c_str()));
    ASSERT_EQ(ECSqlStatus::InvalidECSql, GetHelper().PrepareECSql("INSERT INTO ts.PrimaryClassAHasSecondaryClassB(SourceECInstanceId, TargetECInstanceId) VALUES(104, 204)"));
    m_ecdb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, LogicalForeignKeyRelationshipMappedToSharedColumnWithMixin)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("logicalfk_sharedcol.ecdb",
              SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                         "  <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' />"
                         "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>"
                         "  <ECEntityClass typeName='Equipment'  modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <ClassMap xmlns='ECDbMap.02.00'>"
                         "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "          </ClassMap>"
                         "          <ShareColumns xmlns='ECDbMap.02.00'>"
                         "              <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>"
                         "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                         "          </ShareColumns>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='Code' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='IP2' modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                         "              <AppliesToEntityClass>Equipment</AppliesToEntityClass>"
                         "          </IsMixin>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='Wip' typeName='long' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='IEndPoint' modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                         "              <AppliesToEntityClass>Equipment</AppliesToEntityClass>"
                         "          </IsMixin>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='www' typeName='long' />"
                         "      <ECNavigationProperty propertyName='Car' relationshipName='BaseRelationship' direction='Backward' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='BaseRelationship' strength='holding' strengthDirection='Forward' modifier='Abstract'>"
                         "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
                         "         <Class class='Car' />"
                         "     </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
                         "        <Class class='IEndPoint' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "  <ECRelationshipClass typeName='CarHasEndPoint' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                         "      <BaseClass>BaseRelationship</BaseClass>"
                         "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
                         "         <Class class='Car' />"
                         "     </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
                         "        <Class class='IEndPoint' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "  <ECEntityClass typeName='Car'>"
                         "      <ECProperty propertyName='Name' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Engine'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <BaseClass>IP2</BaseClass>"
                         "      <BaseClass>IEndPoint</BaseClass>"
                         "      <ECProperty propertyName='Volumn1' typeName='double' />"
                         "      <ECProperty propertyName='Volumn' typeName='double' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Sterring'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <BaseClass>IEndPoint</BaseClass>"
                         "      <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Tire'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <ECProperty propertyName='Diameter' typeName='double' />"
                         "  </ECEntityClass>"
                         "</ECSchema>")));
    ECClassId relId = m_ecdb.Schemas().GetClassId("TestSchema", "CarHasEndPoint");
    ECClassId carId = m_ecdb.Schemas().GetClassId("TestSchema", "Car");
    ECClassId engineId = m_ecdb.Schemas().GetClassId("TestSchema", "Engine");
    ECClassId sterringId = m_ecdb.Schemas().GetClassId("TestSchema", "Sterring");

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Car            (Name                                      ) VALUES ('BMW-S')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(SqlPrintfString("INSERT INTO ts.Engine         (Code, www, Volumn,Car.Id,Car.RelECClassId ) VALUES ('CODE-1','www1', 2000.0,1,%d )", relId.GetValue())));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(SqlPrintfString("INSERT INTO ts.Sterring       (Code, www, Type,Car.Id,Car.RelECClassId   ) VALUES ('CODE-2','www2', 'S-Type',1,%d)", relId.GetValue())));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Tire           (Code, Diameter                            ) VALUES ('CODE-3', 15.0)"));


    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.CarHasEndPoint"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(2, stmt.GetValueInt64(0));
    ASSERT_EQ(relId.GetValue(), stmt.GetValueInt64(1));
    ASSERT_EQ(1, stmt.GetValueInt64(2));
    ASSERT_EQ(carId.GetValue(), stmt.GetValueInt64(3));
    ASSERT_EQ(2, stmt.GetValueInt64(4));
    ASSERT_EQ(engineId.GetValue(), stmt.GetValueInt64(5));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(3, stmt.GetValueInt64(0));
    ASSERT_EQ(relId.GetValue(), stmt.GetValueInt64(1));
    ASSERT_EQ(1, stmt.GetValueInt64(2));
    ASSERT_EQ(carId.GetValue(), stmt.GetValueInt64(3));
    ASSERT_EQ(3, stmt.GetValueInt64(4));
    ASSERT_EQ(sterringId.GetValue(), stmt.GetValueInt64(5));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Car.Id,Car.RelECClassId FROM ts.Engine"));
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Car.Id,Car.RelECClassId FROM ts.Sterring"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, LogicalForeignKeyRelationshipMappedToSharedColumn)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("logicalfk_sharedcol.ecdb",
              SchemaItem(
                         "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                         "  <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' />"
                         "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>"
                         "  <ECEntityClass typeName='Equipment'  modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <ClassMap xmlns='ECDbMap.02.00'>"
                         "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "          </ClassMap>"
                         "          <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                         "          <ShareColumns xmlns='ECDbMap.02.00'>"
                         "              <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>"
                         "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                         "          </ShareColumns>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='Code' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='IEndPoint' modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                         "              <AppliesToEntityClass>Equipment</AppliesToEntityClass>"
                         "          </IsMixin>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='www' typeName='long' />"
                         "      <ECNavigationProperty propertyName='Car' relationshipName='BaseRelationship' direction='Backward' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='BaseRelationship' strength='holding' strengthDirection='Forward' modifier='Abstract'>"
                         "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
                         "         <Class class='Car' />"
                         "     </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
                         "        <Class class='IEndPoint' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "  <ECRelationshipClass typeName='CarHasEndPoint' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                         "      <BaseClass>BaseRelationship</BaseClass>"
                         "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
                         "         <Class class='Car' />"
                         "     </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
                         "        <Class class='IEndPoint' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "  <ECEntityClass typeName='Car'>"
                         "      <ECProperty propertyName='Name' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Engine'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <BaseClass>IEndPoint</BaseClass>"
                         "      <ECProperty propertyName='Volumn' typeName='double' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Sterring'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <BaseClass>IEndPoint</BaseClass>"
                         "      <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Tire'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <ECProperty propertyName='Diameter' typeName='double' />"
                         "  </ECEntityClass>"
                         "</ECSchema>")));
    ECClassId relId = m_ecdb.Schemas().GetClassId("TestSchema", "CarHasEndPoint");
    ECClassId carId = m_ecdb.Schemas().GetClassId("TestSchema", "Car");
    ECClassId engineId = m_ecdb.Schemas().GetClassId("TestSchema", "Engine");
    ECClassId sterringId = m_ecdb.Schemas().GetClassId("TestSchema", "Sterring");

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(                "INSERT INTO ts.Car            (Name                                      ) VALUES ('BMW-S')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(SqlPrintfString("INSERT INTO ts.Engine         (Code, www, Volumn,Car.Id,Car.RelECClassId ) VALUES ('CODE-1','www1', 2000.0,1,%s)", relId.ToString().c_str())));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(SqlPrintfString("INSERT INTO ts.Sterring       (Code, www, Type,Car.Id,Car.RelECClassId   ) VALUES ('CODE-2','www2', 'S-Type',1,%s)", relId.ToString().c_str())));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(                "INSERT INTO ts.Tire           (Code, Diameter                            ) VALUES ('CODE-3', 15.0)"));


    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.CarHasEndPoint ORDER BY ECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(2, stmt.GetValueInt64(0));
    ASSERT_EQ(relId, stmt.GetValueId<ECClassId>(1));
    ASSERT_EQ(1, stmt.GetValueInt64(2));
    ASSERT_EQ(carId, stmt.GetValueId<ECClassId>(3));
    ASSERT_EQ(2, stmt.GetValueInt64(4));
    ASSERT_EQ(engineId, stmt.GetValueId<ECClassId>(5));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(3, stmt.GetValueInt64(0));
    ASSERT_EQ(relId, stmt.GetValueId<ECClassId>(1));
    ASSERT_EQ(1, stmt.GetValueInt64(2));
    ASSERT_EQ(carId, stmt.GetValueId<ECClassId>(3));
    ASSERT_EQ(3, stmt.GetValueInt64(4));
    ASSERT_EQ(sterringId, stmt.GetValueId<ECClassId>(5));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT Car.Id,Car.RelECClassId FROM ts.Engine"));
    ASSERT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT Car.Id,Car.RelECClassId FROM ts.Sterring"));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, LogicalForeignKeyRelationshipMappedToUnsharedColumn)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("logicalfk_unsharedcol.ecdb",
              SchemaItem(
                         "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                         "  <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' />"
                         "  <ECEntityClass typeName='Model' modifier='None'>"
                         "      <ECProperty propertyName='Name' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Element'  modifier='None'>"
                         "      <ECCustomAttributes>"
                         "          <ClassMap xmlns='ECDbMap.02.00'>"
                         "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "          </ClassMap>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='Code' typeName='string' />"
                         "      <ECNavigationProperty propertyName='Model' relationshipName='Rel' direction='Backward' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='Rel' strength='referencing' strengthDirection='Forward' modifier='Abstract'>"
                         "      <Source multiplicity='(1..1)' polymorphic='False' roleLabel='Model'>"
                         "         <Class class='Model' />"
                         "     </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Element'>"
                         "        <Class class='Element' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "</ECSchema>")));

    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();

    ECClassCP elementClass = m_ecdb.Schemas().GetClass("TestSchema", "Element");
    ASSERT_TRUE(elementClass != nullptr);

    ASSERT_FALSE(GetHelper().IsForeignKeyColumn("ts_Element", "ModelId"));
    ASSERT_FALSE(GetHelper().IndexExists("x_ts_Element_fk_ts_Rel_target"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MixinAsRelationshipEnd)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("diamond_problem3.ecdb",
              SchemaItem(
                         "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                         "  <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' />"
                         "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>"
                         "  <ECEntityClass typeName='Equipment'  modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <ClassMap xmlns='ECDbMap.02.00'>"
                         "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "          </ClassMap>"
                         "          <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                         "          <ShareColumns xmlns='ECDbMap.02.00'>"
                         "              <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>"
                         "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                         "          </ShareColumns>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='Code' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='IEndPoint' modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                         "              <AppliesToEntityClass>Equipment</AppliesToEntityClass>"
                         "          </IsMixin>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='www' typeName='long' />"
                         "      <ECNavigationProperty propertyName='Car' relationshipName='CarHasEndPoint' direction='Backward'>"
                        "          <ECCustomAttributes>"
                        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                        "          </ECCustomAttributes>"
                        "        </ECNavigationProperty>"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='CarHasEndPoint' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                         "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
                         "         <Class class='Car' />"
                         "     </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
                         "        <Class class='IEndPoint' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "  <ECEntityClass typeName='Car'>"
                         "      <ECProperty propertyName='Name' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Engine'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <BaseClass>IEndPoint</BaseClass>"
                         "      <ECProperty propertyName='Volumn' typeName='double' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Sterring'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <BaseClass>IEndPoint</BaseClass>"
                         "      <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Tire'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <ECProperty propertyName='Diameter' typeName='double' />"
                         "  </ECEntityClass>"
                         "</ECSchema>")));

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Car(Name) VALUES ('BMW-S')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Engine(Code, www, Volumn,Car.Id) VALUES ('CODE-1','www1', 2000.0,1 )"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Sterring(Code, www, Type,Car.Id) VALUES ('CODE-2','www2', 'S-Type',1)"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.Tire(Code, Diameter) VALUES ('CODE-3', 15.0)"));


    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.CarHasEndPoint ORDER BY TargetECInstanceId"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(ECInstanceId(UINT64_C(1)), stmt.GetValueId<ECInstanceId>(0));
    ASSERT_EQ(m_ecdb.Schemas().GetClassId("TestSchema","Car"), stmt.GetValueId<ECClassId>(1));

    ASSERT_EQ(ECInstanceId(UINT64_C(2)), stmt.GetValueId<ECInstanceId>(2));
    ASSERT_EQ(m_ecdb.Schemas().GetClassId("TestSchema", "Engine"), stmt.GetValueId<ECClassId>(3));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(ECInstanceId(UINT64_C(1)), stmt.GetValueId<ECInstanceId>(0));
    ASSERT_EQ(m_ecdb.Schemas().GetClassId("TestSchema", "Car"), stmt.GetValueId<ECClassId>(1));
    ASSERT_EQ(ECInstanceId(UINT64_C(3)), stmt.GetValueId<ECInstanceId>(2));
    ASSERT_EQ(m_ecdb.Schemas().GetClassId("TestSchema", "Sterring"), stmt.GetValueId<ECClassId>(3));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MixinAsRelationshipEnd2)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("diamond_problem3.ecdb",
              SchemaItem(
                         "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                         "  <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' />"
                         "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>"
                         "  <ECEntityClass typeName='Equipment'  modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <ClassMap xmlns='ECDbMap.02.00'>"
                         "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "          </ClassMap>"
                         "          <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                         "          <ShareColumns xmlns='ECDbMap.02.00'>"
                         "              <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>"
                         "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                         "          </ShareColumns>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='Code' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='IEndPoint' modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                         "              <AppliesToEntityClass>Equipment</AppliesToEntityClass>"
                         "          </IsMixin>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='www' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='CarHasEndPoint' strength='holding' strengthDirection='Forward' modifier='Abstract'>"
                         "      <LinkTableRelationshipMap xmlns='ECDbMap.02.00'/>"
                         "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
                         "         <Class class='Car' />"
                         "     </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
                         "        <Class class='IEndPoint' />"
                         "     </Target>"
                         "      <ECProperty propertyName='Tag' typeName='string' />"
                         "  </ECRelationshipClass>"
                         "  <ECRelationshipClass typeName='CarHasEndPoint2' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                         "      <BaseClass>CarHasEndPoint</BaseClass>"
                         "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
                         "         <Class class='Car' />"
                         "     </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
                         "        <Class class='IEndPoint' />"
                         "     </Target>"
                         "      <ECProperty propertyName='Rule' typeName='string' />"
                         "  </ECRelationshipClass>"
                         "  <ECEntityClass typeName='Car'>"
                         "      <ECProperty propertyName='Name' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Engine'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <BaseClass>IEndPoint</BaseClass>"
                         "      <ECProperty propertyName='Volumn' typeName='double' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Sterring'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <BaseClass>IEndPoint</BaseClass>"
                         "      <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Tire'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <ECProperty propertyName='Diameter' typeName='double' />"
                         "  </ECEntityClass>"
                         "</ECSchema>")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Car            (Name              ) VALUES ('BMW-S')")) << m_ecdb.GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << ": " << m_ecdb.GetLastError().c_str();
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Engine         (Code, www, Volumn ) VALUES ('CODE-1','www1', 2000.0 )")) << m_ecdb.GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << ": " << m_ecdb.GetLastError().c_str();
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Sterring       (Code, www, Type   ) VALUES ('CODE-2','www2', 'S-Type')")) << m_ecdb.GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << ": " << m_ecdb.GetLastError().c_str();
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Tire           (Code, Diameter    ) VALUES ('CODE-3', 15.0)")) << m_ecdb.GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << ": " << m_ecdb.GetLastError().c_str();
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.CarHasEndPoint2 (SourceECInstanceId, TargetECInstanceId, TargetECClassId, Tag, Rule) VALUES (1,2,54,'tag1','Rule1')")) << m_ecdb.GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << ": " << m_ecdb.GetLastError().c_str();
    stmt.Finalize();
    m_ecdb.SaveChanges();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.CarHasEndPoint2 (SourceECInstanceId, TargetECInstanceId, TargetECClassId, Tag, Rule) VALUES (1,3,56,'tag2','Rule2')")) << m_ecdb.GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << ": " << m_ecdb.GetLastError().c_str();
    stmt.Finalize();
    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MixinAsRelationshipEnd3)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("diamond_problem3.ecdb",
              SchemaItem(
                         "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                         "  <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' />"
                         "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>"
                         "  <ECEntityClass typeName='Equipment'  modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <ClassMap xmlns='ECDbMap.02.00'>"
                         "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "          </ClassMap>"
                         "          <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                         "          <ShareColumns xmlns='ECDbMap.02.00'>"
                         "              <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>"
                         "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                         "          </ShareColumns>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='Code' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='IEndPoint' modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                         "              <AppliesToEntityClass>Equipment</AppliesToEntityClass>"
                         "          </IsMixin>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='www' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='CarHasEndPoint' strength='holding' strengthDirection='Forward' modifier='Abstract'>"
                         "      <LinkTableRelationshipMap xmlns='ECDbMap.02.00'/>"
                         "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
                         "         <Class class='Car' />"
                         "     </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
                         "        <Class class='IEndPoint' />"
                         "     </Target>"
                         "      <ECProperty propertyName='Tag' typeName='string' />"
                         "  </ECRelationshipClass>"
                         "  <ECEntityClass typeName='Car'>"
                         "      <ECProperty propertyName='Name' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Engine'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <BaseClass>IEndPoint</BaseClass>"
                         "      <ECProperty propertyName='Volumn' typeName='double' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Sterring'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <BaseClass>IEndPoint</BaseClass>"
                         "      <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Tire'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <ECProperty propertyName='Diameter' typeName='double' />"
                         "  </ECEntityClass>"
                         "</ECSchema>")));
    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();

    ReopenECDb();

    TestHelper test(m_ecdb);
    ASSERT_EQ(BE_SQLITE_DONE, test.ExecuteECSql("INSERT INTO ts.Car            (Name              ) VALUES ('BMW-S')"));
    ASSERT_EQ(BE_SQLITE_DONE, test.ExecuteECSql("INSERT INTO ts.Engine         (Code, www, Volumn ) VALUES ('CODE-1','www1', 2000.0 )"));
    ASSERT_EQ(BE_SQLITE_DONE, test.ExecuteECSql("INSERT INTO ts.Sterring       (Code, www, Type   ) VALUES ('CODE-2','www2', 'S-Type')"));
    ASSERT_EQ(BE_SQLITE_DONE, test.ExecuteECSql("INSERT INTO ts.Tire           (Code, Diameter    ) VALUES ('CODE-3', 15.0)"));

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                       "  <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' />"
                                                       "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>"
                                                       "  <ECEntityClass typeName='Equipment'  modifier='Abstract'>"
                                                       "      <ECCustomAttributes>"
                                                       "          <ClassMap xmlns='ECDbMap.02.00'>"
                                                       "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                                                       "          </ClassMap>"
                                                       "          <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                                       "          <ShareColumns xmlns='ECDbMap.02.00'>"
                                                       "              <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>"
                                                       "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                                                       "          </ShareColumns>"
                                                       "      </ECCustomAttributes>"
                                                       "      <ECProperty propertyName='Code' typeName='string' />"
                                                       "  </ECEntityClass>"
                                                       "  <ECEntityClass typeName='IEndPoint' modifier='Abstract'>"
                                                       "      <ECCustomAttributes>"
                                                       "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                                                       "              <AppliesToEntityClass>Equipment</AppliesToEntityClass>"
                                                       "          </IsMixin>"
                                                       "      </ECCustomAttributes>"
                                                       "      <ECProperty propertyName='www' typeName='string' />"
                                                       "  </ECEntityClass>"
                                                       "  <ECRelationshipClass typeName='CarHasEndPoint' strength='holding' strengthDirection='Forward' modifier='Abstract'>"
                                                       "      <LinkTableRelationshipMap xmlns='ECDbMap.02.00'/>"
                                                       "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
                                                       "         <Class class='Car' />"
                                                       "     </Source>"
                                                       "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
                                                       "        <Class class='IEndPoint' />"
                                                       "     </Target>"
                                                       "      <ECProperty propertyName='Tag' typeName='string' />"
                                                       "  </ECRelationshipClass>"
                                                       "  <ECRelationshipClass typeName='CarHasEndPoint2' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                                                       "      <BaseClass>CarHasEndPoint</BaseClass>"
                                                       "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
                                                       "         <Class class='Car' />"
                                                       "     </Source>"
                                                       "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
                                                       "        <Class class='IEndPoint' />"
                                                       "     </Target>"
                                                       "      <ECProperty propertyName='Rule' typeName='string' />"
                                                       "  </ECRelationshipClass>"
                                                       "  <ECEntityClass typeName='Car'>"
                                                       "      <ECProperty propertyName='Name' typeName='string' />"
                                                       "  </ECEntityClass>"
                                                       "  <ECEntityClass typeName='Engine'>"
                                                       "      <BaseClass>Equipment</BaseClass>"
                                                       "      <BaseClass>IEndPoint</BaseClass>"
                                                       "      <ECProperty propertyName='Volumn' typeName='double' />"
                                                       "  </ECEntityClass>"
                                                       "  <ECEntityClass typeName='Sterring'>"
                                                       "      <BaseClass>Equipment</BaseClass>"
                                                       "      <BaseClass>IEndPoint</BaseClass>"
                                                       "      <ECProperty propertyName='Type' typeName='string' />"
                                                       "  </ECEntityClass>"
                                                       "  <ECEntityClass typeName='Tire'>"
                                                       "      <BaseClass>Equipment</BaseClass>"
                                                       "      <ECProperty propertyName='Diameter' typeName='double' />"
                                                       "  </ECEntityClass>"
                                                       "</ECSchema>"))) << "Diamond Problem2";
    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.CarHasEndPoint2 (SourceECInstanceId, TargetECInstanceId, TargetECClassId, Tag, Rule) VALUES (1,2,54,'tag1','Rule1')"));
    m_ecdb.SaveChanges();
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.CarHasEndPoint2 (SourceECInstanceId, TargetECInstanceId, TargetECClassId, Tag, Rule) VALUES (1,3,56,'tag2','Rule2')"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MixinsMappedToMultipleJoinedTablesOnReferencedEnd)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("multijoinedtablemixinonreferencedend.ecdb",
                            SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                          <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                          <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
                                          <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                                              <ECEntityClass typeName="Model">
                                                 <ECCustomAttributes>
                                                   <ClassMap xmlns="ECDbMap.02.00">
                                                          <MapStrategy>TablePerHierarchy</MapStrategy>
                                                   </ClassMap>
                                                   <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                                                 </ECCustomAttributes>
                                                  <ECProperty propertyName="Name" typeName="string" />
                                              </ECEntityClass>
                                              <ECEntityClass typeName="IIsGeometricModel" modifier="Abstract">
                                                 <ECCustomAttributes>
                                                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                                                        <AppliesToEntityClass>Model</AppliesToEntityClass>
                                                    </IsMixin>"
                                                 </ECCustomAttributes>
                                                  <ECProperty propertyName="Is2d" typeName="boolean"/>
                                                  <ECProperty propertyName="SupportedGeometryType" typeName="string" />
                                              </ECEntityClass>
                                              <ECEntityClass typeName="Model2d">
                                                    <BaseClass>Model</BaseClass>
                                                    <BaseClass>IIsGeometricModel</BaseClass>
                                                  <ECProperty propertyName="Origin2d" typeName="Point2d" />
                                              </ECEntityClass>
                                              <ECEntityClass typeName="Model3d">
                                                    <BaseClass>Model</BaseClass>
                                                    <BaseClass>IIsGeometricModel</BaseClass>
                                                  <ECProperty propertyName="Origin3d" typeName="Point3d" />
                                              </ECEntityClass>
                                              <ECEntityClass typeName="Element">
                                                 <ECCustomAttributes>
                                                   <ClassMap xmlns="ECDbMap.02.00">
                                                          <MapStrategy>TablePerHierarchy</MapStrategy>
                                                   </ClassMap>
                                                 </ECCustomAttributes>
                                                <ECProperty propertyName="Code" typeName="string" />
                                                <ECNavigationProperty propertyName="Model" relationshipName="GeometricModelHasElements" direction="Backward"/>
                                              </ECEntityClass>
                                             <ECRelationshipClass typeName="GeometricModelHasElements" strength="Referencing" modifier="Sealed">
                                                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="GeometricModel">
                                                    <Class class ="IIsGeometricModel" />
                                                </Source>
                                                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Element">
                                                    <Class class ="Element" />
                                                </Target>
                                             </ECRelationshipClass>
                                          </ECSchema>)xml")));
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());
    m_ecdb.SaveChanges();

    std::vector<ECInstanceKey> model2dKeys, model3dKeys, element2dKeys, element3dKeys;
    //INSERT
    {
    ECInstanceKey key;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Model2d(Name,Is2d,SupportedGeometryType,Origin2d) VALUES(?,true,'Line',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Main2d", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(2, DPoint2d::From(1.0, 1.0)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    model2dKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Detail2d", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(2, DPoint2d::From(1.5, 1.5)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    model2dKeys.push_back(key);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Model3d(Name,Is2d,SupportedGeometryType,Origin3d) VALUES(?,false,'Solid',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Main3d", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(2, DPoint3d::From(2.0, 2.0, 2.0)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    model3dKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Detail3d", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(2, DPoint3d::From(2.5, 2.5, 2.5)));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    model3dKeys.push_back(key);
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Element(Code, Model.Id) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "E-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, model2dKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    element2dKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "E-2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, model2dKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    element2dKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "E-3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, model3dKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    element3dKeys.push_back(key);
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "E-4", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, model3dKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_TRUE(key.IsValid());
    element3dKeys.push_back(key);
    }

    //SELECT Elements
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, Code, Model.Id FROM ts.Element"));
    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;

        ECInstanceId currentId = stmt.GetValueId<ECInstanceId>(0);
        if (currentId == element2dKeys[0].GetInstanceId() || currentId == element2dKeys[1].GetInstanceId())
            ASSERT_EQ(model2dKeys[0].GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(2).GetValue()) << stmt.GetECSql();
        else if (currentId == element3dKeys[0].GetInstanceId() || currentId == element3dKeys[1].GetInstanceId())
            ASSERT_EQ(model3dKeys[0].GetInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(2).GetValue()) << stmt.GetECSql();
        }

    ASSERT_EQ(4, rowCount) << stmt.GetECSql();
    stmt.Finalize();

    //Select models via mixin
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, ECClassId, Is2d, SupportedGeometryType FROM ts.IIsGeometricModel"));
    int rowCount2d = 0;
    int rowCount3d = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ECClassId actualClassId = stmt.GetValueId<ECClassId>(1);
        if (actualClassId == model2dKeys[0].GetClassId())
            {
            rowCount2d++;
            ASSERT_TRUE(stmt.GetValueBoolean(2)) << stmt.GetECSql();
            ASSERT_STREQ("Line", stmt.GetValueText(3)) << stmt.GetECSql();
            }
        else if (actualClassId == model3dKeys[0].GetClassId())
            {
            rowCount3d++;
            ASSERT_FALSE(stmt.GetValueBoolean(2)) << stmt.GetECSql();
            ASSERT_STREQ("Solid", stmt.GetValueText(3)) << stmt.GetECSql();
            }
        }

    ASSERT_EQ(2, rowCount2d) << stmt.GetECSql();
    ASSERT_EQ(2, rowCount3d) << stmt.GetECSql();
    stmt.Finalize();

    //Select concrete models
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, Name, SupportedGeometryType, Origin2d FROM ts.Model2d"));
    rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        ASSERT_STREQ("Line", stmt.GetValueText(2));

        const ECInstanceId actualModelId = stmt.GetValueId<ECInstanceId>(0);
        Utf8CP actualModelName = stmt.GetValueText(1);
        if (actualModelId == model2dKeys[0].GetInstanceId())
            ASSERT_STREQ("Main2d", actualModelName) << stmt.GetECSql();
        else if (actualModelId == model2dKeys[1].GetInstanceId())
            ASSERT_STREQ("Detail2d", actualModelName) << stmt.GetECSql();
        }

    ASSERT_EQ(2, rowCount) << stmt.GetECSql();

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, Name, SupportedGeometryType, Origin3d FROM ts.Model3d"));
    rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        ASSERT_STREQ("Solid", stmt.GetValueText(2));

        const ECInstanceId actualModelId = stmt.GetValueId<ECInstanceId>(0);
        Utf8CP actualModelName = stmt.GetValueText(1);
        if (actualModelId == model3dKeys[0].GetInstanceId())
            ASSERT_STREQ("Main3d", actualModelName) << stmt.GetECSql();
        else if (actualModelId == model3dKeys[1].GetInstanceId())
            ASSERT_STREQ("Detail3d", actualModelName) << stmt.GetECSql();
        }

    ASSERT_EQ(2, rowCount) << stmt.GetECSql();
    }

    //UPDATE NavProp
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Element SET Model.Id=? WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, model2dKeys[1].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, element2dKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(1, m_ecdb.GetModifiedRowCount()) << stmt.GetECSql();
    stmt.ClearBindings();
    stmt.Reset();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, model3dKeys[1].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, element3dKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(1, m_ecdb.GetModifiedRowCount()) << stmt.GetECSql();
    stmt.Finalize();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT e.ECInstanceId, m.ECInstanceId, m.ECClassId, m.Is2d, m.SupportedGeometryType FROM ts.Element e JOIN ts.IIsGeometricModel m USING ts.GeometricModelHasElements ORDER BY m.ECInstanceId"));
    int rowCount = 0;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        const ECInstanceId actualElementId = stmt.GetValueId<ECInstanceId>(0);
        if (actualElementId == element2dKeys[0].GetInstanceId())
            {
            ASSERT_EQ(model2dKeys[1].GetInstanceId().GetValue(), stmt.GetValueUInt64(1)) << stmt.GetECSql();
            ASSERT_EQ(model2dKeys[1].GetClassId().GetValue(), stmt.GetValueUInt64(2)) << stmt.GetECSql();
            ASSERT_TRUE(stmt.GetValueBoolean(3)) << stmt.GetECSql();
            ASSERT_STREQ("Line", stmt.GetValueText(4)) << stmt.GetECSql();
            }
        else if (actualElementId == element2dKeys[1].GetInstanceId())
            {
            ASSERT_EQ(model2dKeys[0].GetInstanceId().GetValue(), stmt.GetValueUInt64(1)) << stmt.GetECSql();
            ASSERT_EQ(model2dKeys[0].GetClassId().GetValue(), stmt.GetValueUInt64(2)) << stmt.GetECSql();
            ASSERT_TRUE(stmt.GetValueBoolean(3)) << stmt.GetECSql();
            ASSERT_STREQ("Line", stmt.GetValueText(4)) << stmt.GetECSql();
            }
        else if (actualElementId == element3dKeys[0].GetInstanceId())
            {
            ASSERT_EQ(model3dKeys[1].GetInstanceId().GetValue(), stmt.GetValueUInt64(1)) << stmt.GetECSql();
            ASSERT_EQ(model3dKeys[1].GetClassId().GetValue(), stmt.GetValueUInt64(2)) << stmt.GetECSql();
            ASSERT_FALSE(stmt.GetValueBoolean(3)) << stmt.GetECSql();
            ASSERT_STREQ("Solid", stmt.GetValueText(4)) << stmt.GetECSql();
            }
        else if (actualElementId == element3dKeys[1].GetInstanceId())
            {
            ASSERT_EQ(model3dKeys[0].GetInstanceId().GetValue(), stmt.GetValueUInt64(1)) << stmt.GetECSql();
            ASSERT_EQ(model3dKeys[0].GetClassId().GetValue(), stmt.GetValueUInt64(2)) << stmt.GetECSql();
            ASSERT_FALSE(stmt.GetValueBoolean(3)) << stmt.GetECSql();
            ASSERT_STREQ("Solid", stmt.GetValueText(4)) << stmt.GetECSql();
            }
        else
            FAIL() << "Unexpected row returned from " << stmt.GetECSql();
        }
    ASSERT_EQ(4, rowCount);
    }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, EnforceLinkTableMapping)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("enforcelinktablemapping.ecdb", SchemaItem(
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
        "</ECSchema>")));

    //verify tables
    ASSERT_TRUE(GetHelper().TableExists("ts_AHasB"));
    ASSERT_TRUE(m_ecdb.ColumnExists("ts_AHasB", "SourceId"));
    ASSERT_TRUE(m_ecdb.ColumnExists("ts_AHasB", "TargetId"));
    bvector<Utf8String> columns;
    ASSERT_TRUE(m_ecdb.GetColumns(columns, "ts_AHasB"));
    ASSERT_EQ(3, columns.size());

    columns.clear();
    ASSERT_TRUE(m_ecdb.GetColumns(columns, "ts_A"));
    ASSERT_EQ(2, columns.size());

    columns.clear();
    ASSERT_TRUE(m_ecdb.GetColumns(columns, "ts_B"));
    ASSERT_EQ(2, columns.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, FKConstraintsOnLinkTables)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("fkconstraintsonlinktables.ecdb", SchemaItem(
            R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                      <ECEntityClass typeName="A" >
                        <ECProperty propertyName="Prop1" typeName="string" />
                      </ECEntityClass>
                      <ECEntityClass typeName="B" >
                        <ECProperty propertyName="Prop2" typeName="string" />
                      </ECEntityClass>
                      <ECRelationshipClass typeName="LinkTableWithFk1" modifier="Sealed">
                            <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                              <Class class="A"/>
                            </Source>
                            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                              <Class class="B"/>
                            </Target>
                      </ECRelationshipClass>
                      <ECRelationshipClass typeName="LinkTableWithFk2" modifier="Sealed">
                            <ECCustomAttributes>
                                <LinkTableRelationshipMap xmlns="ECDbMap.02.00"/>
                            </ECCustomAttributes>
                            <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                              <Class class="A"/>
                            </Source>
                            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                              <Class class="B"/>
                            </Target>
                      </ECRelationshipClass>
                      <ECRelationshipClass typeName="LinkTableWithFk3" modifier="Sealed">
                            <ECCustomAttributes>
                                <LinkTableRelationshipMap xmlns="ECDbMap.02.00">
                                    <CreateForeignKeyConstraints>True</CreateForeignKeyConstraints>
                                </LinkTableRelationshipMap>
                            </ECCustomAttributes>
                            <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                              <Class class="A"/>
                            </Source>
                            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                              <Class class="B"/>
                            </Target>
                        </ECRelationshipClass>
                        <ECRelationshipClass typeName="LinkTableWithoutFk" modifier="Sealed">
                           <ECCustomAttributes>
                                <LinkTableRelationshipMap xmlns="ECDbMap.02.00">
                                    <CreateForeignKeyConstraints>False</CreateForeignKeyConstraints>
                                </LinkTableRelationshipMap>
                            </ECCustomAttributes>
                            <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                              <Class class="A"/>
                            </Source>
                            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                              <Class class="B"/>
                            </Target>
                       </ECRelationshipClass>
                 </ECSchema>)xml")));

    ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_LinkTableWithFk1", "SourceId", "CASCADE", nullptr)) << GetHelper().GetForeignKeyConstraintDdl("ts_LinkTableWithFk1", "SourceId");
    ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_LinkTableWithFk1", "TargetId", "CASCADE", nullptr)) << GetHelper().GetForeignKeyConstraintDdl("ts_LinkTableWithFk1", "TargetId");
    ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_LinkTableWithFk2", "SourceId", "CASCADE", nullptr)) << GetHelper().GetForeignKeyConstraintDdl("ts_LinkTableWithFk2", "SourceId");
    ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_LinkTableWithFk2", "TargetId", "CASCADE", nullptr)) << GetHelper().GetForeignKeyConstraintDdl("ts_LinkTableWithFk2", "TargetId");
    ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_LinkTableWithFk3", "SourceId", "CASCADE", nullptr)) << GetHelper().GetForeignKeyConstraintDdl("ts_LinkTableWithFk3", "SourceId");
    ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_LinkTableWithFk3", "TargetId", "CASCADE", nullptr)) << GetHelper().GetForeignKeyConstraintDdl("ts_LinkTableWithFk3", "TargetId");
    ASSERT_FALSE(GetHelper().GetDdl("ts_LinkTableWithoutFk").ContainsI("foreign key"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, LinkTablesAndSharedColumns)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("LinkTablesAndSharedColumns.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                   <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                      <ECEntityClass typeName="A" >
                        <ECProperty propertyName="Prop1" typeName="string" />
                      </ECEntityClass>
                      <ECEntityClass typeName="B" >
                        <ECProperty propertyName="Prop2" typeName="string" />
                      </ECEntityClass>
                      <ECRelationshipClass typeName="LinkTable" modifier="None">
                            <ECCustomAttributes>
                                <ClassMap xmlns="ECDbMap.02.00">
                                  <MapStrategy>TablePerHierarchy</MapStrategy>
                                </ClassMap>
                                <ShareColumns xmlns="ECDbMap.02.00">
                                    <ApplyToSubclassesOnly>false</ApplyToSubclassesOnly>
                                </ShareColumns>
                            </ECCustomAttributes>
                            <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                              <Class class="A"/>
                            </Source>
                            <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
                              <Class class="B"/>
                            </Target>
                      </ECRelationshipClass>
                      <ECRelationshipClass typeName="LinkTableSub" modifier="None">
                            <BaseClass>LinkTable</BaseClass>
                            <Source multiplicity="(1..*)" polymorphic="True" roleLabel="has">
                              <Class class="A"/>
                            </Source>
                            <Target multiplicity="(1..1)" polymorphic="True" roleLabel="has">
                              <Class class="B"/>
                            </Target>
                            <ECProperty propertyName="Order" typeName="int" />
                      </ECRelationshipClass>
                 </ECSchema>)xml")));

    EXPECT_EQ(ExpectedColumn("ts_LinkTable", "SourceId"), GetHelper().GetPropertyMapColumn(AccessString("ts", "LinkTable", "SourceECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_A", "ECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("ts", "LinkTable", "SourceECClassId")));
    EXPECT_EQ(ExpectedColumn("ts_LinkTable", "TargetId"), GetHelper().GetPropertyMapColumn(AccessString("ts", "LinkTable", "TargetECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_B", "ECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("ts", "LinkTable", "TargetECClassId")));

    EXPECT_FALSE(GetHelper().TableExists("ts_LinkTableSub"));
    EXPECT_EQ(ExpectedColumn("ts_LinkTable", "SourceId"), GetHelper().GetPropertyMapColumn(AccessString("ts", "LinkTableSub", "SourceECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_A", "ECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("ts", "LinkTableSub", "SourceECClassId")));
    EXPECT_EQ(ExpectedColumn("ts_LinkTable", "TargetId"), GetHelper().GetPropertyMapColumn(AccessString("ts", "LinkTableSub", "TargetECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_B", "ECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("ts", "LinkTableSub", "TargetECClassId")));
    EXPECT_EQ(ExpectedColumn("ts_LinkTable", "ps1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "LinkTableSub", "Order")));

    ECClassId linkTableClassId = m_ecdb.Schemas().GetClassId("TestSchema", "LinkTable");
    ASSERT_TRUE(linkTableClassId.IsValid());
    ECClassId linkTableSubClassId = m_ecdb.Schemas().GetClassId("TestSchema", "LinkTableSub");
    ASSERT_TRUE(linkTableSubClassId.IsValid());
    EXPECT_EQ(3, GetHelper().GetIndexNamesForTable("ts_LinkTable").size());
    EXPECT_STRCASEEQ(IndexInfo("ix_ts_LinkTable_ecclassid", false, "ts_LinkTable", "ECClassId").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_ts_LinkTable_ecclassid").c_str());
    EXPECT_STRCASEEQ(IndexInfo("ix_ts_LinkTable_target", false, "ts_LinkTable", "TargetId").ToDdl().c_str(), GetHelper().GetIndexDdl("ix_ts_LinkTable_target").c_str());
    EXPECT_STRCASEEQ(IndexInfo("uix_ts_LinkTable_sourcetargetclassid", true, "ts_LinkTable", std::vector<Utf8String>{"SourceId","TargetId", "ECClassId"}).ToDdl().c_str(), GetHelper().GetIndexDdl("uix_ts_LinkTable_sourcetargetclassid").c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, AmbigousRelationshipProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ambigousRelationshipProperty.ecdb",
                                 SchemaItem(
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                     "  <ECEntityClass typeName='Geometry' >"
                                     "        <ECCustomAttributes>"
                                     "            <ClassMap xmlns='ECDbMap.02.00'>"
                                     "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                     "            </ClassMap>"
                                     "        </ECCustomAttributes>"
                                     "    <ECProperty propertyName='P1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECEntityClass typeName='GeometryPart' >"
                                     "        <ECCustomAttributes>"
                                     "            <ClassMap xmlns='ECDbMap.02.00'>"
                                     "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                     "            </ClassMap>"
                                     "        </ECCustomAttributes>"
                                     "    <ECProperty propertyName='P1' typeName='string' />"
                                     "  </ECEntityClass>"
                                     "  <ECRelationshipClass typeName='GeometryHoldsParts' strength='referencing' strengthDirection='Forward' modifier='Sealed'>"
                                     "     <Source cardinality='(0,N)' polymorphic='True'>"
                                     "         <Class class='Geometry' />"
                                     "     </Source>"
                                     "    <Target cardinality='(0,N)' polymorphic='True'>"
                                     "        <Class class='GeometryPart' />"
                                     "     </Target>"
                                     "    <ECProperty propertyName='P1' typeName='string' />"
                                     "  </ECRelationshipClass>"
                                     "</ECSchema>"))) << "N:N and holding";
    ECInstanceKey geometryKey, geometryPartKey;
    {//INSERT Geometry
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Geometry(P1) VALUES('G1')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geometryKey));
    m_ecdb.SaveChanges();
    }//===============

    {//INSERT GeometryPart
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GeometryPart(P1) VALUES('GP1')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geometryPartKey));
    m_ecdb.SaveChanges();
    }//===============

    {//INSERT GeometryHoldsParts
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GeometryHoldsParts(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, P1) VALUES(?,?,?,?, 'GHP1')"));
    stmt.BindId(1, geometryKey.GetInstanceId());
    stmt.BindId(2, geometryKey.GetClassId());
    stmt.BindId(3, geometryPartKey.GetInstanceId());
    stmt.BindId(4, geometryPartKey.GetClassId());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    m_ecdb.SaveChanges();
    }//===============

    {//Verify
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, P1 FROM ts.GeometryHoldsParts WHERE P1 = 'GHP1'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(geometryKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0));
    ASSERT_EQ(geometryKey.GetClassId(), stmt.GetValueId<ECClassId>(1));
    ASSERT_EQ(geometryPartKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(2));
    ASSERT_EQ(geometryPartKey.GetClassId(), stmt.GetValueId<ECClassId>(3));
    ASSERT_STREQ("GHP1", stmt.GetValueText(4));
    }//===============
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, NotNullConstraintsOnFkColumns)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("notnullconstraintsonfkcolumns.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                                                                  "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                                                                  "   <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                                                                  "    <ECEntityClass typeName='A'>"
                                                                                  "        <ECProperty propertyName='AName' typeName='string' />"
                                                                                  "    </ECEntityClass>"
                                                                                  "    <ECEntityClass typeName='B'>"
                                                                                  "        <ECProperty propertyName='BName' typeName='string' />"
                                                                                  "        <ECNavigationProperty propertyName='AId_Rel1N' relationshipName='Rel1N' direction='Backward' >"
                                                                                  "          <ECCustomAttributes>"
                                                                                  "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                                                                  "               <OnDeleteAction>Cascade</OnDeleteAction>"
                                                                                  "            </ForeignKeyConstraint>"
                                                                                  "          </ECCustomAttributes>"
                                                                                  "        </ECNavigationProperty>"
                                                                                  "        <ECNavigationProperty propertyName='AId_Rel0N' relationshipName='Rel0N' direction='Backward' >"
                                                                                  "          <ECCustomAttributes>"
                                                                                  "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                                                                  "               <OnDeleteAction>Cascade</OnDeleteAction>"
                                                                                  "            </ForeignKeyConstraint>"
                                                                                  "          </ECCustomAttributes>"
                                                                                  "        </ECNavigationProperty>"
                                                                                  "        <ECNavigationProperty propertyName='AId_RelN1' relationshipName='RelN1' direction='Forward' >"
                                                                                  "          <ECCustomAttributes>"
                                                                                  "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                                                                  "               <OnDeleteAction>Cascade</OnDeleteAction>"
                                                                                  "            </ForeignKeyConstraint>"
                                                                                  "          </ECCustomAttributes>"
                                                                                  "        </ECNavigationProperty>"
                                                                                  "        <ECNavigationProperty propertyName='AId_RelN0' relationshipName='RelN0' direction='Forward' >"
                                                                                  "          <ECCustomAttributes>"
                                                                                  "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                                                                  "               <OnDeleteAction>Cascade</OnDeleteAction>"
                                                                                  "            </ForeignKeyConstraint>"
                                                                                  "          </ECCustomAttributes>"
                                                                                  "        </ECNavigationProperty>"
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
                                                                                  "</ECSchema>"))) << "relationship classes with nav props";

    Utf8String ddl = GetHelper().GetDdl("ts_B");
    ASSERT_FALSE(ddl.empty());

    ASSERT_TRUE(ddl.ContainsI("[AId_Rel0NId] INTEGER,")) << "relationship classes with nav props> Actual DDL: " << ddl.c_str();
    ASSERT_TRUE(ddl.ContainsI("[AId_Rel1NId] INTEGER NOT NULL,")) << "relationship classes with nav props> Actual DDL: " << ddl.c_str();
    ASSERT_TRUE(ddl.ContainsI("[AId_RelN0Id] INTEGER,")) << "relationship classes with nav props> Actual DDL: " << ddl.c_str();
    ASSERT_TRUE(ddl.ContainsI("[AId_RelN1Id] INTEGER NOT NULL,")) << "relationship classes with nav props> Actual DDL: " << ddl.c_str();


    ASSERT_EQ(SUCCESS, SetupECDb("notnullconstraintsonfkcolumns.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='A'>"
        "        <ECProperty propertyName='AName' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='B'>"
        "        <ECProperty propertyName='BName' typeName='string' />"
        "        <ECNavigationProperty propertyName='A1' relationshipName='Rel1N' direction='Backward' >"
        "          <ECCustomAttributes>"
        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
        "          </ECCustomAttributes>"
        "        </ECNavigationProperty>"
        "        <ECNavigationProperty propertyName='A2' relationshipName='Rel0N' direction='Backward' >"
        "          <ECCustomAttributes>"
        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
        "          </ECCustomAttributes>"
        "        </ECNavigationProperty>"
        "        <ECNavigationProperty propertyName='A3' relationshipName='RelN0' direction='Forward' >"
        "          <ECCustomAttributes>"
        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
        "          </ECCustomAttributes>"
        "        </ECNavigationProperty>"
        "        <ECNavigationProperty propertyName='A4' relationshipName='RelN1' direction='Forward' >"
        "          <ECCustomAttributes>"
        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
        "          </ECCustomAttributes>"
        "        </ECNavigationProperty>"
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
        "</ECSchema>"))) << "relationship classes with custom fk names";

    ddl = GetHelper().GetDdl("ts_B");
    ASSERT_FALSE(ddl.empty());

    ASSERT_TRUE(ddl.ContainsI("[A1Id] INTEGER NOT NULL,")) << "relationship classes with custom fk names> Actual DDL: " << ddl.c_str();
    ASSERT_TRUE(ddl.ContainsI("[A2Id] INTEGER,")) << "relationship classes with custom fk names> Actual DDL: " << ddl.c_str();
    ASSERT_TRUE(ddl.ContainsI("[A3Id] INTEGER,")) << "relationship classes with custom fk names> Actual DDL: " << ddl.c_str();
    ASSERT_TRUE(ddl.ContainsI("[A4Id] INTEGER NOT NULL,")) << "relationship classes with custom fk names> Actual DDL: " << ddl.c_str();



    ASSERT_EQ(SUCCESS, SetupECDb("notnullconstraintsonfkcolumns.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                                                                  "        <ECNavigationProperty propertyName='AId' relationshipName='Rel' direction='Backward' >"
                                                                                  "          <ECCustomAttributes>"
                                                                                  "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                                                                  "          </ECCustomAttributes>"
                                                                                  "        </ECNavigationProperty>"
                                                                                  "    </ECEntityClass>"
                                                                                  "  <ECRelationshipClass typeName='Rel' strength='embedding' modifier='Sealed'>"
                                                                                  "    <Source cardinality='(1,1)' polymorphic='True'>"
                                                                                  "      <Class class = 'A' />"
                                                                                  "    </Source>"
                                                                                  "    <Target cardinality='(0,N)' polymorphic='True'>"
                                                                                  "      <Class class = 'BSub'/>"
                                                                                  "    </Target>"
                                                                                  "  </ECRelationshipClass>"
                                                                                  "</ECSchema>"))) << "(1,1) rel with dropped NOT NULL constraint";

    ddl = GetHelper().GetDdl("ts_B");
    ASSERT_FALSE(ddl.empty());

    ASSERT_TRUE(ddl.ContainsI("[AId] INTEGER,")) << "(1,1) rel with dropped NOT NULL constraint> Actual DDL: " << ddl.c_str();


    ASSERT_EQ(SUCCESS, SetupECDb("notnullconstraintsonfkcolumns.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                                                                  "        <ECNavigationProperty propertyName='AId' relationshipName='Rel' direction='Backward'/>"
                                                                                  "    </ECEntityClass>"
                                                                                  "    <ECEntityClass typeName='Sub' modifier='Abstract'>"
                                                                                  "        <BaseClass>Base</BaseClass>"
                                                                                  "        <ECProperty propertyName='SubName' typeName='string' />"
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
                                                                                  "      <Class class = 'Base' />"
                                                                                  "    </Target>"
                                                                                  "  </ECRelationshipClass>"
                                                                                  "</ECSchema>"))) << "Logical FK";

    ddl = GetHelper().GetDdl("ts_Base");
    ASSERT_FALSE(ddl.empty());
    ASSERT_TRUE(ddl.ContainsI("[AId] INTEGER,")) << "Logical FK> Actual DDL: " << ddl.c_str();


    ASSERT_EQ(SUCCESS, SetupECDb("notnullconstraintsonfkcolumns.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                                                                 "</ECSchema>")));

    ddl = GetHelper().GetDdl("ts_Base");
    ASSERT_FALSE(ddl.empty());
    ASSERT_TRUE(ddl.ContainsI("[AId] INTEGER,")) << "Logical FK with dropped not null constraint> Actual DDL: " << ddl.c_str();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, ForeignKeyColumnPosition)
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


    ASSERT_EQ(SUCCESS, SetupECDb("fkcolumnposition.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                                                     "        <ECNavigationProperty propertyName='ParentId' relationshipName='Rel' direction='Backward' >"
                                                                     "          <ECCustomAttributes>"
                                                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                                                     "          </ECCustomAttributes>"
                                                                     "        </ECNavigationProperty>"
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
                                                                     "</ECSchema>"))) << "Nav Prop as first prop";

    ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_Base", "ParentId", "CASCADE", nullptr));
    //Subsubclasses come before sibling classes, therefore parent id is after AAProp1
    assertColumnPosition(m_ecdb, "ts_Base", "ParentId", 4, "Nav Prop as first prop");


    ASSERT_EQ(SUCCESS, SetupECDb("fkcolumnposition.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                                                     "        <ECNavigationProperty propertyName='ParentId' relationshipName='Rel' direction='Backward'>"
                                                                     "          <ECCustomAttributes>"
                                                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                                                     "          </ECCustomAttributes>"
                                                                     "        </ECNavigationProperty>"
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
                                                                     "</ECSchema>"))) << "Nav prop as last property";

    ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_Base", "ParentId", "CASCADE", nullptr));
    assertColumnPosition(m_ecdb, "ts_Base", "ParentId", 5, "Nav prop as last property");


    ASSERT_EQ(SUCCESS, SetupECDb("fkcolumnposition.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                                                     "        <ECNavigationProperty propertyName='Parent1' relationshipName='Rel1' direction='Backward' >"
                                                                     "          <ECCustomAttributes>"
                                                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                                                     "          </ECCustomAttributes>"
                                                                     "        </ECNavigationProperty>"
                                                                     "        <ECNavigationProperty propertyName='Parent2' relationshipName='Rel2' direction='Backward' >"
                                                                     "          <ECCustomAttributes>"
                                                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                                                     "          </ECCustomAttributes>"
                                                                     "        </ECNavigationProperty>"
                                                                     "    </ECEntityClass>"
                                                                     "    <ECEntityClass typeName='C'>"
                                                                     "        <BaseClass>Base</BaseClass>"
                                                                     "        <ECProperty propertyName='CProp1' typeName='string' />"
                                                                     "    </ECEntityClass>"
                                                                     "  <ECRelationshipClass typeName='Rel1' strength='embedding' modifier='Sealed'>"
                                                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                                                     "      <Class class = 'Parent' />"
                                                                     "    </Source>"
                                                                     "    <Target cardinality='(0,N)' polymorphic='True'>"
                                                                     "      <Class class = 'B' />"
                                                                     "    </Target>"
                                                                     "  </ECRelationshipClass>"
                                                                     "  <ECRelationshipClass typeName='Rel2' strength='embedding' modifier='Sealed'>"
                                                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                                                     "      <Class class = 'Parent' />"
                                                                     "    </Source>"
                                                                     "    <Target cardinality='(0,N)' polymorphic='True'>"
                                                                     "      <Class class = 'B' />"
                                                                     "    </Target>"
                                                                     "  </ECRelationshipClass>"
                                                                     "</ECSchema>"))) << "Two Nav props in a row";

    ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_Base", "Parent1Id", "CASCADE", nullptr));
    ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_Base", "Parent2Id", "CASCADE", nullptr));
    //WIP: Column order for two nav props in a row is not correct yet. Once fixed, flip positions in the below calls.
    assertColumnPosition(m_ecdb, "ts_Base", "Parent1Id", 5, "Two Nav props in a row");
    assertColumnPosition(m_ecdb, "ts_Base", "Parent2Id", 6, "Two Nav props in a row");


    ASSERT_EQ(SUCCESS, SetupECDb("fkcolumnposition.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                                                     "        <ECNavigationProperty propertyName='ParentId' relationshipName='Rel' direction='Backward' >"
                                                                     "          <ECCustomAttributes>"
                                                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                                                     "          </ECCustomAttributes>"
                                                                     "        </ECNavigationProperty>"
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
                                                                     "</ECSchema>"))) << "Nav prop is only prop";

    ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_Base", "ParentId", "CASCADE", nullptr));
    assertColumnPosition(m_ecdb, "ts_Base", "ParentId", 4, "Nav prop is only prop");


    ASSERT_EQ(SUCCESS, SetupECDb("fkcolumnposition.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                                                     "        <ECNavigationProperty propertyName='ParentId' relationshipName='Rel' direction='Backward' >"
                                                                     "          <ECCustomAttributes>"
                                                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                                                     "          </ECCustomAttributes>"
                                                                     "        </ECNavigationProperty>"
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
                                                                     "</ECSchema>"))) << "Nav Prop in class with shared columns";

    ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_Base", "ParentId", "CASCADE", nullptr));
    assertColumnPosition(m_ecdb, "ts_Base", "ParentId", -1, "Nav Prop in class with shared columns");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, OneToOneRelationshipMapping)
    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                     "    <ECEntityClass typeName='A'>"
                                     "        <ECProperty propertyName='AName' typeName='string' />"
                                     "        <ECNavigationProperty propertyName='B1' relationshipName='Rel11back' direction='Forward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='B2' relationshipName='Rel01back' direction='Forward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='B3' relationshipName='Rel10back' direction='Forward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='B4' relationshipName='Rel00back' direction='Forward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B'>"
                                     "        <ECProperty propertyName='BName' typeName='string' />"
                                     "        <ECNavigationProperty propertyName='A1' relationshipName='Rel11' direction='Backward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='A2' relationshipName='Rel01' direction='Backward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='A3' relationshipName='Rel10' direction='Backward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='A4' relationshipName='Rel00' direction='Backward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
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

    testSchemas.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                     "    <ECEntityClass typeName='A'>"
                                     "        <ECProperty propertyName='AName' typeName='string' />"
                                     "        <ECNavigationProperty propertyName='B1' relationshipName='Rel11back' direction='Forward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='B2' relationshipName='Rel01back' direction='Forward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='B3' relationshipName='Rel10back' direction='Forward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='B4' relationshipName='Rel00back' direction='Forward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B'>"
                                     "        <ECProperty propertyName='BName' typeName='string' />"
                                     "        <ECNavigationProperty propertyName='A1' relationshipName='Rel11' direction='Backward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='A2' relationshipName='Rel01' direction='Backward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='A3' relationshipName='Rel10' direction='Backward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='A4' relationshipName='Rel00' direction='Backward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
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

    testSchemas.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                     "    <ECEntityClass typeName='A'>"
                                     "        <ECProperty propertyName='AName' typeName='string' />"
                                     "        <ECNavigationProperty propertyName='B1' relationshipName='Rel11back' direction='Forward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='B2' relationshipName='Rel01back' direction='Forward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='B3' relationshipName='Rel10back' direction='Forward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='B4' relationshipName='Rel00back' direction='Forward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B'>"
                                     "        <ECProperty propertyName='BName' typeName='string' />"
                                     "        <ECNavigationProperty propertyName='A1' relationshipName='Rel11' direction='Backward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='A2' relationshipName='Rel01' direction='Backward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='A3' relationshipName='Rel10' direction='Backward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "        <ECNavigationProperty propertyName='A4' relationshipName='Rel00' direction='Backward' >"
                                     "          <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
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
        ASSERT_EQ(SUCCESS, SetupECDb("onetoonerelationshipmappings.ecdb", testSchema));

        ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_b", "A1Id")) << GetHelper().GetForeignKeyConstraintDdl("ts_b","A1Id");
        ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_b", "A2Id")) << GetHelper().GetForeignKeyConstraintDdl("ts_b", "A2Id");
        ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_b", "A3Id")) << GetHelper().GetForeignKeyConstraintDdl("ts_b", "A3Id");
        ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_b", "A4Id")) << GetHelper().GetForeignKeyConstraintDdl("ts_b", "A4Id");

        ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_a", "B1Id")) << GetHelper().GetForeignKeyConstraintDdl("ts_a", "B1Id");
        ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_a", "B2Id")) << GetHelper().GetForeignKeyConstraintDdl("ts_a", "B2Id");
        ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_a", "B3Id")) << GetHelper().GetForeignKeyConstraintDdl("ts_a", "B3Id");
        ASSERT_TRUE(GetHelper().IsForeignKeyColumn("ts_a", "B4Id")) << GetHelper().GetForeignKeyConstraintDdl("ts_a", "B4Id");

        ASSERT_FALSE(GetHelper().IsForeignKeyColumn("ts_a", "A1Id"));
        ASSERT_FALSE(GetHelper().IsForeignKeyColumn("ts_a", "A2Id"));
        ASSERT_FALSE(GetHelper().IsForeignKeyColumn("ts_a", "A3Id"));
        ASSERT_FALSE(GetHelper().IsForeignKeyColumn("ts_a", "A4Id"));

        ASSERT_FALSE(GetHelper().IsForeignKeyColumn("ts_b", "B1Id"));
        ASSERT_FALSE(GetHelper().IsForeignKeyColumn("ts_b", "B2Id"));
        ASSERT_FALSE(GetHelper().IsForeignKeyColumn("ts_b", "B3Id"));
        ASSERT_FALSE(GetHelper().IsForeignKeyColumn("ts_b", "B4Id"));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, DisallowCascadingDeleteOnJoinedTable)
    {
    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                     "    <ECEntityClass typeName='A'>"
                                     "        <ECProperty propertyName='AName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B'>"
                                     "      <ECCustomAttributes>"
                                     "        <ClassMap xmlns='ECDbMap.02.00'>"
                                     "            <MapStrategy>TablePerHierarchy</MapStrategy>"
                                     "        </ClassMap>"
                                     "        <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                     "     </ECCustomAttributes>"
                                     "        <ECProperty propertyName='BName' typeName='string' />"
                                     "        <ECNavigationProperty propertyName='A1' relationshipName='Rel1' direction='Backward' />"
                                     "        <ECNavigationProperty propertyName='A2' relationshipName='Rel2' direction='Backward' >"
                                     "          <ECCustomAttributes>"
                                     "             <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                     "               <OnDeleteAction>Cascade</OnDeleteAction>"
                                     "             </ForeignKeyConstraint>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B1'>"
                                     "        <BaseClass>B</BaseClass>"
                                     "        <ECProperty propertyName='B1Name' typeName='string' />"
                                     "        <ECNavigationProperty propertyName='A3' relationshipName='Rel3' direction='Backward' >"
                                     "          <ECCustomAttributes>"
                                     "             <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                     "               <OnDeleteAction>Restrict</OnDeleteAction>"
                                     "             </ForeignKeyConstraint>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
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
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,N)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel3' strength='embedding' modifier='Sealed'>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,N)' polymorphic='True'>"
                                     "      <Class class = 'B1'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  </ECSchema>"))) << "Supported cases";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                     "    <ECEntityClass typeName='A'>"
                                     "        <ECProperty propertyName='AName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B'>"
                                     "      <ECCustomAttributes>"
                                     "            <ClassMap xmlns='ECDbMap.02.00'>"
                                     "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                     "            </ClassMap>"
                                     "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                     "     </ECCustomAttributes>"
                                     "        <ECProperty propertyName='BName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B1'>"
                                     "        <BaseClass>B</BaseClass>"
                                     "        <ECProperty propertyName='B1Name' typeName='string' />"
                                     "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward' />"
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
                                     "  </ECSchema>"))) << "Logical Foreign key supports embedding relationship against subclass (joined table)";


    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                     "    <ECEntityClass typeName='A'>"
                                     "        <ECProperty propertyName='AName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B'>"
                                     "      <ECCustomAttributes>"
                                     "            <ClassMap xmlns='ECDbMap.02.00'>"
                                     "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                     "            </ClassMap>"
                                     "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                     "     </ECCustomAttributes>"
                                     "        <ECProperty propertyName='BName' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B1'>"
                                     "        <BaseClass>B</BaseClass>"
                                     "        <ECProperty propertyName='B1Name' typeName='string' />"
                                     "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward' >"
                                     "              <ECCustomAttributes>"
                                     "                  <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "              </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
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
                                     "  </ECSchema>"))) << "Embedding relationship against subclass (joined table)";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                     "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward' >"
                                     "          <ECCustomAttributes>"
                                     "             <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                     "               <OnDeleteAction>Cascade</OnDeleteAction>"
                                     "             </ForeignKeyConstraint>"
                                     "          </ECCustomAttributes>"
                                     "        </ECNavigationProperty>"
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
                                     "  </ECSchema>"))) << "relationship with Cascade Delete against subclass (joined table)";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, RelationshipWithAbstractConstraintClassAndNoSubclasses)
    {
    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.0">
          <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
          <ECEntityClass typeName="Element" modifier="Abstract">
            <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00">
                      <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Code" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="GeometrySource" modifier="Abstract" />
          <ECEntityClass typeName="ElementGeometry">
            <ECProperty propertyName="Geom" typeName="binary" />
            <ECNavigationProperty propertyName="Source" relationshipName="GeometrySourceHasGeometry" direction="Backward" />                 
          </ECEntityClass>
          <ECEntityClass typeName="ExtendedElement">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
          <ECRelationshipClass typeName="GeometrySourceHasGeometry" strength="embedding" modifier="Sealed">
            <Source cardinality="(0,1)" polymorphic="True">
              <Class class="GeometrySource" />
            </Source>
            <Target cardinality="(0,N)" polymorphic="True">
              <Class class="ElementGeometry" />
            </Target>
          </ECRelationshipClass>
        </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.0">
          <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
          <ECEntityClass typeName="Element" modifier="Abstract">
            <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00">
                      <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="Code" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="GeometrySource" modifier="Abstract" />
          <ECEntityClass typeName="GeometrySource3d">
            <BaseClass>GeometrySource</BaseClass>
            <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
          <ECEntityClass typeName="ElementGeometry">
            <ECProperty propertyName="Geom" typeName="binary" />
            <ECNavigationProperty propertyName="Source" relationshipName="GeometrySourceHasGeometry" direction="Backward" />                 
          </ECEntityClass>
          <ECEntityClass typeName="ExtendedElement">
            <BaseClass>Element</BaseClass>
            <ECProperty propertyName="Name" typeName="string" />
          </ECEntityClass>
          <ECRelationshipClass typeName="GeometrySourceHasGeometry" strength="embedding" modifier="Sealed">
            <Source cardinality="(0,1)" polymorphic="False">
              <Class class="GeometrySource" />
            </Source>
            <Target cardinality="(0,N)" polymorphic="True">
              <Class class="ElementGeometry" />
            </Target>
          </ECRelationshipClass>
        </ECSchema>)xml")));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, RelationshipWithAbstractConstraintClass)
    {
    
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
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
        "    <ECNavigationProperty propertyName='Source' relationshipName='GeometrySourceHasGeometry' direction='Backward' />"
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
        "</ECSchema>"))) << "Multiple Tables on either end of the relationship are not supported";

    auto getGeometrySourceHasGeometryRowCount = [] (ECDbCR ecdb)
        {
        ECSqlStatement selectStmt;
        if (ECSqlStatus::Success != selectStmt.Prepare(ecdb, "SELECT count(*) FROM ts.GeometrySourceHasGeometry"))
            return -1;

        if (BE_SQLITE_ROW != selectStmt.Step())
            return -1;

        return selectStmt.GetValueInt(0);
        };

    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem(
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
                                     "    <ECNavigationProperty propertyName='Source' relationshipName='GeometrySourceHasGeometry' direction='Backward' />"
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

    testSchemas.push_back(SchemaItem("<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
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
                                     "    <ECNavigationProperty propertyName='Source' relationshipName='GeometrySourceHasGeometry' direction='Backward' />"
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
        ASSERT_EQ(SUCCESS, SetupECDb("RelationshipWithAbstractBaseClass.ecdb", testSchema));

        ECInstanceKey elem1Key, elem2Key, geomElem1Key, geomElem2Key, geom1Key, geom2Key;
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.ExtendedElement(Code,Name) VALUES('0001','NonGeom1')"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(elem1Key));
        stmt.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.ExtendedElement(Code,Name) VALUES('0002','NonGeom2')"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(elem2Key));
        stmt.Finalize();

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GeometricElement(Code,Name) VALUES('0003','Geom1')"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomElem1Key));
        stmt.Finalize();

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GeometricElement(Code,Name) VALUES('0004','Geom2')"));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomElem2Key));
        stmt.Finalize();

        //now do actual tests with relationship
        ASSERT_EQ(0, getGeometrySourceHasGeometryRowCount(m_ecdb)) << "Before inserting nav prop";

        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.ElementGeometry(Source.Id) VALUES(?)"));
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomElem1Key.GetInstanceId()));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geom1Key));
        stmt.Reset();
        stmt.ClearBindings();

        ASSERT_EQ(1, getGeometrySourceHasGeometryRowCount(m_ecdb)) << "After inserting one nav prop";

        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elem1Key.GetInstanceId()));
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geom1Key));

        ASSERT_EQ(2, getGeometrySourceHasGeometryRowCount(m_ecdb)) << "After inserting second nav prop";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, RelationshipWithAbstractClassAsConstraintOnChildEnd)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("RelationshipWithAbstractClassAsConstraintOnChildEnd.ecdb", SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                          <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                          <ECEntityClass typeName="Solid">
                            <ECProperty propertyName="Name" typeName="string" />
                          </ECEntityClass>
                          <ECEntityClass typeName="Face" modifier="Abstract">
                            <ECProperty propertyName="FaceName" typeName="string" />
                            <ECNavigationProperty propertyName="Solid" relationshipName="SolidHasFaces" direction="Backward" />
                          </ECEntityClass>
                          <ECRelationshipClass typeName="SolidHasFaces" strength="embedding" modifier="Sealed">
                            <Source multiplicity="(0..1)" roleLabel="has" polymorphic="True">
                              <Class class="Solid" />
                            </Source>
                            <Target multiplicity="(0..*)" roleLabel="belongs to" polymorphic="True">
                              <Class class="Face" />
                            </Target>
                          </ECRelationshipClass>
                        </ECSchema>)xml")));

    ECClassId faceClassId = m_ecdb.Schemas().GetClassId("TestSchema", "Face");
    ASSERT_TRUE(faceClassId.IsValid());
    ECInstanceKey solidKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Solid(Name) VALUES('MySolid')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(solidKey));
    }

    m_ecdb.SaveChanges();

    {
    ECSqlStatement selectStmt;
    ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(m_ecdb, "SELECT count(*) FROM ts.SolidHasFaces"));
    ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
    ASSERT_EQ(0, selectStmt.GetValueInt(0)) << "SELECT count(*) FROM ts.SolidHasFaces";
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.SolidHasFaces(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(?,?,4444,?)"));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                  06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, RelationshipWithNotMappedClassAsConstraint)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                  <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                  <ECEntityClass typeName="Element" modifier="Sealed">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>NotMapped</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Code" typeName="string" />
                  </ECEntityClass>
                  <ECEntityClass typeName="ElementGeometry" modifier="Sealed">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>NotMapped</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Geom" typeName="binary" />
                  </ECEntityClass>
                  <ECRelationshipClass typeName="ElementHasGeometry" strength="embedding" modifier="Sealed">
                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="owns">
                      <Class class="Element" />
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                      <Class class="ElementGeometry" />
                    </Target>
                    <ECProperty propertyName="RelProp" typeName="string" />
                  </ECRelationshipClass>
                </ECSchema>)xml"))) << "1:N Relationship having NotMapped constraint class on both sides of relationship are not supported";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                  <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                  <ECEntityClass typeName="Element" modifier="Sealed">
                    <ECProperty propertyName="Code" typeName="string" />
                  </ECEntityClass>
                  <ECEntityClass typeName="ElementGeometry" modifier="Sealed">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>NotMapped</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Geom" typeName="binary" />
                  </ECEntityClass>
                  <ECRelationshipClass typeName="ElementHasGeometry" strength="embedding" modifier="Sealed">
                    <Source multiplicity="(0..*)" polymorphic="True" roleLabel="owns">
                      <Class class="Element" />
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                      <Class class="ElementGeometry" />
                    </Target>
                    <ECProperty propertyName="RelProp" typeName="string" />
                  </ECRelationshipClass>
                </ECSchema>)xml"))) << "N:N Relationship having not mapped constraint class on one side of relationship are not supported";

    {
    ASSERT_EQ(SUCCESS, SetupECDb("RelationshipConstraint_WithNotMappedSubclass.ecdb", SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                  <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                  <ECEntityClass typeName="Element" modifier="None">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Code" typeName="string" />
                  </ECEntityClass>
                  <ECEntityClass typeName="PhysicalElement" modifier="Sealed">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>NotMapped</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <BaseClass>Element</BaseClass>
                    <ECProperty propertyName="Code" typeName="string" />
                  </ECEntityClass>
                  <ECEntityClass typeName="ElementGeometry" modifier="None">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Geom" typeName="binary" />
                  </ECEntityClass>
                  <ECEntityClass typeName="ElementGeometry3D" modifier="Sealed">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>NotMapped</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <BaseClass>Element</BaseClass>
                    <ECProperty propertyName="Dimension" typeName="integer" />
                  </ECEntityClass>
                  <ECRelationshipClass typeName="ElementHasGeometry" strength="embedding" modifier="Sealed">
                    <Source multiplicity="(0..*)" polymorphic="True" roleLabel="owns">
                      <Class class="Element" />
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                      <Class class="ElementGeometry" />
                    </Target>
                    <ECProperty propertyName="RelProp" typeName="string" />
                  </ECRelationshipClass>
                </ECSchema>)xml")));

    EXPECT_EQ(Table::Type::Primary, GetHelper().GetMappedTable("ts_Element").GetType());
    EXPECT_EQ(Table::Type::Primary, GetHelper().GetMappedTable("ts_ElementGeometry").GetType());
    EXPECT_EQ(Table::Type::Primary, GetHelper().GetMappedTable("ts_ElementHasGeometry").GetType());
    }

    {
    ASSERT_EQ(SUCCESS, SetupECDb("RelationshipConstraint_WithNotMappedSubclass.ecdb", 
                  SchemaItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                  <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                  <ECEntityClass typeName="Element" modifier="None">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                     <ECProperty propertyName="Code" typeName="string" />
                  </ECEntityClass>
                  <ECEntityClass typeName="PhysicalElement" modifier="Sealed">
                    <BaseClass>Element</BaseClass>
                    <ECProperty propertyName="Name" typeName="string" />
                  </ECEntityClass>
                  <ECEntityClass typeName="ElementGeometry" modifier="None">
                      <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                      </ECCustomAttributes>
                     <ECProperty propertyName="Geom" typeName="binary" />
                  </ECEntityClass>
                  <ECEntityClass typeName="ElementGeometry3D" modifier="Sealed">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>NotMapped</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <BaseClass>ElementGeometry</BaseClass>
                    <ECProperty propertyName="Dimension" typeName="integer" />
                  </ECEntityClass>
                  <ECRelationshipClass typeName="ElementHasGeometry" strength="referencing" modifier="Sealed">
                    <Source multiplicity="(0..*)" polymorphic="True" roleLabel="owns">
                      <Class class="Element" />
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                      <Class class="ElementGeometry" />
                    </Target>
                    <ECProperty propertyName="RelProp" typeName="string" />
                  </ECRelationshipClass>
                </ECSchema>)xml")));

    EXPECT_EQ(Table::Type::Primary, GetHelper().GetMappedTable("ts_Element").GetType());
    EXPECT_EQ(Table::Type::Primary, GetHelper().GetMappedTable("ts_ElementGeometry").GetType());
    EXPECT_EQ(Table::Type::Primary, GetHelper().GetMappedTable("ts_ElementHasGeometry").GetType());
    }

    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, AddDerivedClassOfConstraintOnNsideOf1NRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("addderivedclassonNsideof1Nrelationship.ecdb", SchemaItem(
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
        "</ECSchema>")));

    ECClassCP item = m_ecdb.Schemas().GetClass("OpenPlant", "ITEM");
    ECClassCP unit = m_ecdb.Schemas().GetClass("OpenPlant", "UNIT");

    Savepoint sp(m_ecdb, "CRUD Operations");
    //Insert Statements
    {
    //relationship between UNIT and ITEM
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')"));

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401, 201, %llu, 101, %llu)", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(ecsql.c_str()));
    }

    //Select statements
    {
    ASSERT_EQ(BE_SQLITE_ROW, GetHelper().ExecuteECSql("SELECT * FROM op.UNIT_HAS_ITEM"));

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId=%s", item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_ROW, GetHelper().ExecuteECSql(ecsql.c_str()));
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %s", item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(ecsql.c_str()));
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId=%s", item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(ecsql.c_str()));
    }
    sp.Cancel();

    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(
        "<?xml version = '1.0' encoding = 'utf-8'?>"
        "<ECSchema schemaName='OpenPlant_3D' nameSpacePrefix='op3d' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='OpenPlant' version='01.00' prefix='op' />"
        "  <ECEntityClass typeName='ITEM_3D' >"
        "    <BaseClass>op:ITEM</BaseClass>"
        "    <ECProperty propertyName='op3d_ITEM_prop' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>")));

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, AddDerivedClassOfConstraintOn1sideOf1NRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("addderivedclasson1sideof1Nrelationship.ecdb", SchemaItem(
        "<?xml version = '1.0' encoding = 'utf-8'?>"
        "<ECSchema schemaName='OpenPlant' nameSpacePrefix='op' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "  <ECEntityClass typeName='ITEM' >"
        "    <ECProperty propertyName='op_ITEM_prop' typeName='string' />"
        "        <ECNavigationProperty propertyName='UNIT' relationshipName='UNIT_HAS_ITEM' direction='Backward' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='UNIT' >"
        "    <ECProperty propertyName='op_UNIT_prop' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='UNIT_HAS_ITEM' strength='referencing' strengthDirection='forward'  modifier='Sealed'>"
        "    <Source cardinality='(0,1)' polymorphic='True'>"
        "      <Class class='UNIT' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class='ITEM' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>")));

    ECClassCP unit = m_ecdb.Schemas().GetClass("OpenPlant", "UNIT");
    //ECClassCP item = m_ecdb.Schemas().GetClass("OpenPlant", "ITEM");
    m_ecdb.SaveChanges();
    Savepoint sp(m_ecdb, "CRUD operations");
    //Insert Statements
    {
    //relationship between UNIT and ITEM
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop,UNIT.Id) VALUES(101, 'itemString1',201)"));
    }

    //Select Statements
    {
    Utf8String ecsql;
    ASSERT_EQ(BE_SQLITE_ROW, GetHelper().ExecuteECSql("SELECT * FROM op.UNIT_HAS_ITEM"));
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, GetHelper().ExecuteECSql(ecsql.c_str()));
    }

    //Delete Statements
    {
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE op.ITEM SET UNIT.Id = NULL WHERE ECInstanceId = 101"));
    //Verify Deletion

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(ecsql.c_str()));
    }
    sp.Cancel();

    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(
        "<?xml version = '1.0' encoding = 'utf-8'?>"
        "<ECSchema schemaName='OpenPlant_3D' nameSpacePrefix='op3d' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='OpenPlant' version='01.00' prefix='op' />"
        "  <ECEntityClass typeName='UNIT_3D' >"
        "    <BaseClass>op:UNIT</BaseClass>"
        "    <ECProperty propertyName='op3d_UNIT_prop' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>")));

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, AddDerivedClassOfConstraintsForNNRelationship)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("addderivedclassofconstraintsforNNrelationship.ecdb", SchemaItem(
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
        "</ECSchema>")));

    ECClassCP item = m_ecdb.Schemas().GetClass("OpenPlant", "ITEM");
    ECClassCP unit = m_ecdb.Schemas().GetClass("OpenPlant", "UNIT");

    Savepoint sp(m_ecdb, "CRUD Operations");
    //Insert Statements
    {
    //relationship between UNIT and ITEM
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')"));

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, relProp) VALUES(401, 201, %llu, 101, %llu, 'relPropString1')", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(ecsql.c_str()));
    }

    //Select statements
    {
    ASSERT_EQ(BE_SQLITE_ROW, GetHelper().ExecuteECSql("SELECT * FROM op.UNIT_HAS_ITEM"));

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu AND TargetECClassId = %llu", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, GetHelper().ExecuteECSql(ecsql.c_str()));
    }

    //update Statement
    {
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("UPDATE op.UNIT_HAS_ITEM SET relProp='relPropUpdatedString1' WHERE ECInstanceId=401"));
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %s AND TargetECClassId = %s", unit->GetId().ToString().c_str(),
                  item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(ecsql.c_str()));
    //verify Deltion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %s AND TargetECClassId = %s", unit->GetId().ToString().c_str(), item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql(ecsql.c_str()));
    }
    sp.Cancel();

    m_ecdb.SaveChanges();
    ASSERT_EQ(ERROR, ImportSchema(SchemaItem(
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
        "</ECSchema>")));

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                  02/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, StrengthDirectionValidityOnEndTableRelationship)
    {
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
            R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
              <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
              <ECEntityClass typeName="Model" >
                <ECProperty propertyName="Name" typeName="string" />
              </ECEntityClass>
              <ECEntityClass typeName="Element" >
                <ECProperty propertyName="Code" typeName="string" />
                <ECNavigationProperty propertyName="Model" relationshipName="ModelHasElements" direction="Backward" />
              </ECEntityClass>
              <ECRelationshipClass typeName="ModelHasElements" modifier="None" strength="embedding" strengthDirection="Backward">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="Model Has Elements">
                  <Class class="Model" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Model Has Elements (Reversed)">
                  <Class class="Element" />
                </Target>
              </ECRelationshipClass>
            </ECSchema>)xml"))) << "For a FKRelationship class with strength 'embedding', the cardinality 1-N requires the direction to be 'forward'.";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(
            R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
              <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
              <ECEntityClass typeName="Model" >
                <ECProperty propertyName="Name" typeName="string" />
              </ECEntityClass>
              <ECEntityClass typeName="Element" >
                <ECProperty propertyName="Code" typeName="string" />
                <ECNavigationProperty propertyName="Model" relationshipName="ModelHasElements" direction="Backward" />
              </ECEntityClass>
              <ECRelationshipClass typeName="ModelHasElements" modifier="None" strength="embedding" strengthDirection="Forward">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="Model Has Elements">
                  <Class class="Model" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Model Has Elements (Reversed)">
                  <Class class="Element" />
                </Target>
              </ECRelationshipClass>
            </ECSchema>)xml"))) << "Mapping of FKRelationshipClass with strength 'embedding' and direction 'forward' for a 1-N cardinality, is expected to succeed.";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(
            R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
              <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
              <ECEntityClass typeName="Model" >
                <ECProperty propertyName="Name" typeName="string" />
                <ECNavigationProperty propertyName="Element" relationshipName="ModelHasElements" direction="Forward" />
              </ECEntityClass>
              <ECEntityClass typeName="Element" >
                <ECProperty propertyName="Code" typeName="string" />
              </ECEntityClass>
              <ECRelationshipClass typeName="ModelHasElements" modifier="None" strength="embedding" strengthDirection="Backward">
                <Source multiplicity="(0..*)" polymorphic="True" roleLabel="Model Has Elements">
                  <Class class="Model" />
                </Source>
                <Target multiplicity="(0..1)" polymorphic="True" roleLabel="Model Has Elements (Reversed)">
                  <Class class="Element" />
                </Target>
              </ECRelationshipClass>
            </ECSchema>)xml"))) << "Mapping of FKRelationshipClass with strength 'embedding' and direction 'Backward' for a N-1 cardinality, is expected to succeed.";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
            "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
            "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
            "  <ECEntityClass typeName='Model' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "    <ECNavigationProperty propertyName='Element' relationshipName='ModelHasElements' direction='Backward' />"
            "  </ECEntityClass>"
            "  <ECEntityClass typeName='Element' >"
            "    <ECProperty propertyName='Code' typeName='string' />"
            "  </ECEntityClass>"
            "  <ECRelationshipClass typeName='ModelHasElements' modifier='None' strength='embedding' strengthDirection='Forward'>"
            "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements'>"
            "      <Class class='Model' />"
            "    </Source>"
            "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
            "      <Class class='Element' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>"))) << "For a FKRelationship class with strength 'embedding', the cardinality N-1 requires the direction to be 'Backward'.";

    ASSERT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(
            "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
            "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
            "  <ECEntityClass typeName='Model' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECEntityClass>"
            "  <ECEntityClass typeName='Element' >"
            "    <ECProperty propertyName='Code' typeName='string' />"
            "    <ECNavigationProperty propertyName='Model' relationshipName='ModelHasElements' direction='Backward' />"
            "  </ECEntityClass>"
            "  <ECRelationshipClass typeName='ModelHasElements' modifier='None' strength='embedding' strengthDirection='Forward'>"
            "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
            "      <Class class='Model' />"
            "    </Source>"
            "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
            "      <Class class='Element' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>"))) << "Mapping of FKRelationshipClass with strength 'embedding' and direction 'Backward' for a 1-1 cardinality, is expected to succeed.";
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                       08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, DiegoRelationshipTest)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("diegorelationshiptest.ecdb", SchemaItem(R"xml(
            <ECSchema schemaName="DiegoSchema1" alias="ds1" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                <ECEntityClass typeName="CivilModel">
                    <ECProperty propertyName="CMId" typeName="string"/>
                </ECEntityClass>
                <ECEntityClass typeName="DataSetModel">
                    <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="MSMId" typeName="string"/>
                    <ECNavigationProperty propertyName="CivilModel" relationshipName="CivilModelHasDataSetModel" direction="Backward" />
                </ECEntityClass>
                <ECRelationshipClass typeName="CivilModelHasDataSetModel" modifier="Sealed" strength="referencing" strengthDirection="forward">
                    <Source multiplicity="(0..1)" polymorphic="true" roleLabel="holds">
                        <Class class="CivilModel"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="true" roleLabel="is held by">
                        <Class class="DataSetModel"/>
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(
            <ECSchema schemaName="DiegoSchema2" alias="ds2" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="DiegoSchema1" version="01.00" alias="ds1"/>
                <ECEntityClass typeName="GeometricModel">
                    <BaseClass>ds1:DataSetModel</BaseClass>
                    <ECProperty propertyName="Name" typeName="string"/>
                 </ECEntityClass>
            </ECSchema>
            )xml")));

    ASSERT_TRUE(m_ecdb.Schemas().GetClass("DiegoSchema1", "CivilModel") != nullptr);
    ASSERT_TRUE(m_ecdb.Schemas().GetClass("DiegoSchema1", "DataSetModel") != nullptr);
    ASSERT_TRUE(m_ecdb.Schemas().GetClass("DiegoSchema1", "CivilModelHasDataSetModel") != nullptr);
    ASSERT_TRUE(m_ecdb.Schemas().GetClass("DiegoSchema2", "GeometricModel") != nullptr);

    ECInstanceKey civilModel1Key, civilModel2Key;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(civilModel1Key, "INSERT INTO ds1.CivilModel(CMId) VALUES('M-1')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(civilModel2Key, "INSERT INTO ds1.CivilModel(CMId) VALUES('M-2')"));
    
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ds2.GeometricModel(Name,CivilModel) VALUES('GM-10',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, civilModel2Key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, PhysicalForeignKey)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleFK.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>
                <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />
                <ECEntityClass typeName='A'>
                    <ECProperty propertyName='Price' typeName='double' />
                </ECEntityClass>
                <ECEntityClass typeName='B'>
                    <ECProperty propertyName='Cost' typeName='double' />
                    <ECNavigationProperty propertyName='A' relationshipName='AHasB' direction='Backward'>
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>
                        </ECCustomAttributes>
                    </ECNavigationProperty>
                </ECEntityClass>
               <ECRelationshipClass typeName='AHasB' strength='Referencing' modifier='Sealed' strengthDirection='Backward'>
                  <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>
                      <Class class ='A' />
                  </Source>
                  <Target multiplicity='(0..*)' polymorphic='False' roleLabel='B'>
                      <Class class ='B' />
                  </Target>
               </ECRelationshipClass>
            </ECSchema>)xml")));

    EXPECT_EQ(Table::Type::Primary, GetHelper().GetMappedTable("ts_A").GetType());
    EXPECT_EQ(ExpectedColumn("ts_A", "Id"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "A", "ECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_A", "ECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "A", "ECClassId")));
    EXPECT_EQ(ExpectedColumn("ts_A", "Price"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "A", "Price")));

    EXPECT_EQ(Table::Type::Virtual, GetHelper().GetMappedTable("ts_AHasB").GetType());
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "Id", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "ECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "ECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "ECClassId")));
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "SourceECInstanceId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "SourceECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "SourceECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "SourceECClassId")));
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "TargetECInstanceId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "TargetECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "TargetECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "TargetECClassId")));

    EXPECT_EQ(Table::Type::Primary, GetHelper().GetMappedTable("ts_B").GetType());
    EXPECT_EQ(ExpectedColumn("ts_B", "Id"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B", "ECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_B", "ECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B", "ECClassId")));
    EXPECT_EQ(ExpectedColumn("ts_B", "Cost"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B", "Cost")));
    EXPECT_EQ(ExpectedColumns({{"ts_B", "AId"},{"ts_B","ARelECClassId", Virtual::Yes}}), GetHelper().GetPropertyMapColumns(AccessString("TestSchema", "B", "A")));
    EXPECT_EQ(ExpectedColumn("ts_B", "AId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B", "A.Id")));
    EXPECT_EQ(ExpectedColumn("ts_B", "ARelECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B", "A.RelECClassId")));

    EXPECT_TRUE(GetHelper().IsForeignKeyColumn("ts_B", "AId", "SET NULL", nullptr)) << GetHelper().GetForeignKeyConstraintDdl("ts_B", "AId");

    EXPECT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.AHasB"));
    EXPECT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT ECInstanceId, ECClassId, Price FROM ts.A"));
    EXPECT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT ECInstanceId, ECClassId, Cost, A FROM ts.B"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, PhysicalForeignKeyWithRelSubclasses)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleFK.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="A">
                    <ECProperty propertyName="Price" typeName="double" />
                </ECEntityClass>
                <ECEntityClass typeName="B">
                    <ECCustomAttributes>
                        <ClassMap xlmns="ECDbMap.02.00">
                            <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                    </ECCustomAttributes>
                    <ECProperty propertyName="Cost" typeName="double" />
                    <ECNavigationProperty propertyName="A" relationshipName="AHasB" direction="Backward">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                    </ECNavigationProperty>
                </ECEntityClass>
                <ECEntityClass typeName="B1">
                    <BaseClass>B</BaseClass>
                    <ECProperty propertyName="Tag" typeName="double" />
                </ECEntityClass>
               <ECRelationshipClass typeName="AHasB" strength="Referencing" modifier="Abstract">
                  <Source multiplicity="(0..1)" polymorphic="True" roleLabel="A">
                      <Class class ="A" />
                  </Source>
                  <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                      <Class class ="B" />
                  </Target>
               </ECRelationshipClass>
               <ECRelationshipClass typeName="AHasB1" strength="Referencing" modifier="Sealed">
                  <BaseClass>AHasB</BaseClass>
                  <Source multiplicity="(0..1)" polymorphic="True" roleLabel="A1">
                      <Class class ="A" />
                  </Source>
                  <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B1">
                      <Class class ="B1" />
                  </Target>
               </ECRelationshipClass>
            </ECSchema>)xml")));

    EXPECT_EQ(ExpectedColumn("ts_A", "Id"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "A", "ECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_A", "ECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "A", "ECClassId")));
    EXPECT_EQ(ExpectedColumn("ts_A", "Price"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "A", "Price")));

    EXPECT_EQ(Table::Type::Virtual, GetHelper().GetMappedTable("ts_AHasB").GetType());
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "Id", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "ECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "SourceECInstanceId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "SourceECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "SourceECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "SourceECClassId")));
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "TargetECInstanceId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "TargetECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "TargetECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "TargetECClassId")));

    EXPECT_EQ(Table::Type::Primary, GetHelper().GetMappedTable("ts_B").GetType());
    EXPECT_EQ(ExpectedColumn("ts_B", "Id"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B", "ECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_B", "ECClassId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B", "ECClassId")));
    EXPECT_EQ(ExpectedColumn("ts_B", "Cost"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B", "Cost")));
    EXPECT_EQ(ExpectedColumns({{"ts_B", "AId"}, {"ts_B","ARelECClassId"}}), GetHelper().GetPropertyMapColumns(AccessString("TestSchema", "B", "A")));
    EXPECT_EQ(ExpectedColumn("ts_B", "AId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B", "A.Id")));
    EXPECT_EQ(ExpectedColumn("ts_B", "ARelECClassId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B", "A.RelECClassId")));

    EXPECT_EQ(ExpectedColumn("ts_B", "Id"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B1", "ECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_B", "ECClassId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B1", "ECClassId")));
    EXPECT_EQ(ExpectedColumn("ts_B", "Cost"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B1", "Cost")));
    EXPECT_EQ(ExpectedColumns({{"ts_B", "AId"}, {"ts_B","ARelECClassId"}}), GetHelper().GetPropertyMapColumns(AccessString("TestSchema", "B1", "A")));
    EXPECT_EQ(ExpectedColumn("ts_B", "AId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B1", "A.Id")));
    EXPECT_EQ(ExpectedColumn("ts_B", "ARelECClassId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B1", "A.RelECClassId")));

    EXPECT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.AHasB"));
    EXPECT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.AHasB1"));
    EXPECT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT ECInstanceId, ECClassId, Price FROM ts.A"));
    EXPECT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT ECInstanceId, ECClassId, Cost, A FROM ts.B"));
    EXPECT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT ECInstanceId, ECClassId, Cost, A, Tag FROM ts.B1"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Affan.Khan                          05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, PhysicalForeignKeyWithoutTph)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("SimpleFK.ecdb", SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="A">
                    <ECProperty propertyName="Price" typeName="double" />
                </ECEntityClass>
                <ECEntityClass typeName="B">
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECNavigationProperty propertyName="A" relationshipName="AHasB" direction="Backward">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                    </ECNavigationProperty>
                </ECEntityClass>
                <ECEntityClass typeName="B1">
                    <BaseClass>B</BaseClass>
                    <ECProperty propertyName="Tag" typeName="double" />
                </ECEntityClass>
               <ECRelationshipClass typeName="AHasB" strength="Referencing" modifier="None">
                  <Source multiplicity="(0..1)" polymorphic="True" roleLabel="A">
                      <Class class ="A" />
                  </Source>
                  <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                      <Class class ="B" />
                  </Target>
               </ECRelationshipClass>
               <ECRelationshipClass typeName="AHasB1" strength="Referencing" modifier="Sealed">
                  <BaseClass>AHasB</BaseClass>
                  <Source multiplicity="(0..1)" polymorphic="True" roleLabel="A1">
                      <Class class ="A" />
                  </Source>
                  <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B1">
                      <Class class ="B1" />
                  </Target>
               </ECRelationshipClass>
            </ECSchema>)xml")));

    EXPECT_EQ(ExpectedColumn("ts_A", "Id"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "A", "ECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_A", "ECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "A", "ECClassId")));
    EXPECT_EQ(ExpectedColumn("ts_A", "Price"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "A", "Price")));

    EXPECT_EQ(Table::Type::Virtual, GetHelper().GetMappedTable("ts_AHasB").GetType());
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "Id", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "ECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "SourceECInstanceId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "SourceECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "SourceECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "SourceECClassId")));
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "TargetECInstanceId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "TargetECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_AHasB", "TargetECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "AHasB", "TargetECClassId")));

    EXPECT_EQ(Table::Type::Primary, GetHelper().GetMappedTable("ts_B").GetType());
    EXPECT_EQ(ExpectedColumn("ts_B", "Id"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B", "ECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_B", "ECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B", "ECClassId")));
    EXPECT_EQ(ExpectedColumn("ts_B", "Name"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B", "Name")));
    EXPECT_EQ(ExpectedColumns({{"ts_B", "AId"}, {"ts_B","ARelECClassId"}}), GetHelper().GetPropertyMapColumns(AccessString("TestSchema", "B", "A")));
    EXPECT_EQ(ExpectedColumn("ts_B", "AId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B", "A.Id")));
    EXPECT_EQ(ExpectedColumn("ts_B", "ARelECClassId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B", "A.RelECClassId")));

    EXPECT_EQ(Table::Type::Primary, GetHelper().GetMappedTable("ts_B1").GetType());
    EXPECT_EQ(ExpectedColumn("ts_B1", "Id"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B1", "ECInstanceId")));
    EXPECT_EQ(ExpectedColumn("ts_B1", "ECClassId", Virtual::Yes), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B1", "ECClassId")));
    EXPECT_EQ(ExpectedColumn("ts_B1", "Name"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B1", "Name")));
    EXPECT_EQ(ExpectedColumn("ts_B1", "Tag"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B1", "Tag")));
    EXPECT_EQ(ExpectedColumns({{"ts_B1", "AId"}, {"ts_B1","ARelECClassId"}}), GetHelper().GetPropertyMapColumns(AccessString("TestSchema", "B1", "A")));
    EXPECT_EQ(ExpectedColumn("ts_B1", "AId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B1", "A.Id")));
    EXPECT_EQ(ExpectedColumn("ts_B1", "ARelECClassId"), GetHelper().GetPropertyMapColumn(AccessString("TestSchema", "B1", "A.RelECClassId")));

    EXPECT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.AHasB"));
    EXPECT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.AHasB1"));
    EXPECT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT ECInstanceId, ECClassId, Price FROM ts.A"));
    EXPECT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT ECInstanceId, ECClassId, Name, A FROM ts.B"));
    EXPECT_EQ(ECSqlStatus::Success, GetHelper().PrepareECSql("SELECT ECInstanceId, ECClassId, Name, A, Tag FROM ts.B1"));

    ECInstanceKey aKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(aKey, "INSERT INTO ts.A(Price) VALUES(3.99)"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.B(Name, A) VALUES('B-1',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, aKey.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema", "AHasB")));
    ECInstanceKey bKey;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(bKey)) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.B1(Name, Tag, A) VALUES('B1-1', 2.99,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(1, aKey.GetInstanceId(), m_ecdb.Schemas().GetClassId("TestSchema","AHasB1")));

    ECInstanceKey b1Key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(b1Key)) << stmt.GetECSql();
    stmt.Finalize();
    m_ecdb.SaveChanges();

    // Verify data via nav prop selects

    //polymorphic query on base class
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, Name, A FROM ts.B"));
    int rowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        rowCount++;

        ECInstanceId bId = stmt.GetValueId<ECInstanceId>(0);
        if (bId == bKey.GetInstanceId())
            {
            ASSERT_STREQ("B-1", stmt.GetValueText(1)) << "B instance";
            ECClassId relClassId;
            ASSERT_EQ(aKey.GetInstanceId(), stmt.GetValueNavigation<ECInstanceId>(2, &relClassId)) << "B instance";
            ASSERT_EQ(m_ecdb.Schemas().GetClassId("TestSchema", "AHasB"), relClassId) << "B instance";
            }
        else if (bId == b1Key.GetInstanceId())
            {
            ASSERT_STREQ("B1-1", stmt.GetValueText(1)) << "B1 instance";
            ECClassId relClassId;
            ASSERT_EQ(aKey.GetInstanceId(), stmt.GetValueNavigation<ECInstanceId>(2, &relClassId)) << "B1 instance";
            ASSERT_EQ(m_ecdb.Schemas().GetClassId("TestSchema", "AHasB1"), relClassId) << "B1 instance";
            }
        else
            FAIL() << "unexpected row from " << stmt.GetECSql();
        }

    ASSERT_EQ(2, rowCount) << stmt.GetECSql();
    stmt.Finalize();

    //non-polymorphic query on base class
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, Name, A FROM ONLY ts.B"));
    rowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        rowCount++;
        ASSERT_EQ(bKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql();
        ASSERT_STREQ("B-1", stmt.GetValueText(1)) << stmt.GetECSql();
        ECClassId relClassId;
        ASSERT_EQ(aKey.GetInstanceId(), stmt.GetValueNavigation<ECInstanceId>(2, &relClassId)) << stmt.GetECSql();
        ASSERT_EQ(m_ecdb.Schemas().GetClassId("TestSchema", "AHasB"), relClassId) << stmt.GetECSql();
        }

    ASSERT_EQ(1, rowCount) << stmt.GetECSql();
    stmt.Finalize();

    //query on sub class
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId, Name, A FROM ONLY ts.B1"));
    rowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        rowCount++;
        ASSERT_EQ(b1Key.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0)) << stmt.GetECSql();
        ASSERT_STREQ("B1-1", stmt.GetValueText(1)) << stmt.GetECSql();
        ECClassId relClassId;
        ASSERT_EQ(aKey.GetInstanceId(), stmt.GetValueNavigation<ECInstanceId>(2, &relClassId)) << stmt.GetECSql();
        ASSERT_EQ(m_ecdb.Schemas().GetClassId("TestSchema", "AHasB1"), relClassId) << stmt.GetECSql();
        }

    ASSERT_EQ(1, rowCount) << stmt.GetECSql();
    stmt.Finalize();

    // Verify data via relationship query

    //polymorphic query on base class
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId, TargetECInstanceId, ECClassId FROM ts.AHasB"));
    rowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        rowCount++;
        ASSERT_EQ(aKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0));

        ECInstanceId targetId = stmt.GetValueId<ECInstanceId>(1);
        if (targetId == bKey.GetInstanceId())
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClassId("TestSchema", "AHasB"), stmt.GetValueId<ECClassId>(2));
            }
        else if (targetId == b1Key.GetInstanceId())
            {
            ASSERT_EQ(m_ecdb.Schemas().GetClassId("TestSchema", "AHasB1"), stmt.GetValueId<ECClassId>(2));
            }
        else
            FAIL() << "unexpected row from " << stmt.GetECSql();
        }

    ASSERT_EQ(2, rowCount) << stmt.GetECSql();
    stmt.Finalize();

    //non-polymorphic query on base class
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId, TargetECInstanceId, ECClassId FROM ONLY ts.AHasB"));
    rowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        rowCount++;
        ASSERT_EQ(aKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0));
        ASSERT_EQ(bKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(1));
        ASSERT_EQ(m_ecdb.Schemas().GetClassId("TestSchema", "AHasB"), stmt.GetValueId<ECClassId>(2));
        }

    ASSERT_EQ(1, rowCount) << stmt.GetECSql();
    stmt.Finalize();

    //query on sub class
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId, TargetECInstanceId, ECClassId FROM ONLY ts.AHasB1"));
    rowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        rowCount++;
        ASSERT_EQ(aKey.GetInstanceId(), stmt.GetValueId<ECInstanceId>(0));
        ASSERT_EQ(b1Key.GetInstanceId(), stmt.GetValueId<ECInstanceId>(1));
        ASSERT_EQ(m_ecdb.Schemas().GetClassId("TestSchema", "AHasB1"), stmt.GetValueId<ECClassId>(2));
        }

    ASSERT_EQ(1, rowCount) << stmt.GetECSql();
    stmt.Finalize();
    }


//-------------------------------------------------------------------------------------- 
// @bsimethod                                 Krischan.Eberle                       07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, UpgradeModifierFromSealed)
    {
    //*** Physical FK
        {
            Utf8CP schema = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="A">
                <ECProperty propertyName="Price" typeName="double" />
            </ECEntityClass>
            <ECEntityClass typeName="B">
                <ECProperty propertyName="Name" typeName="string" />
                <ECNavigationProperty propertyName="A" relationshipName="AHasB" direction="Backward">
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                </ECNavigationProperty>
            </ECEntityClass>
            <ECRelationshipClass typeName="AHasB" strength="Referencing" modifier="%s">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="A">
                    <Class class ="A" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                    <Class class ="B" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

        Utf8String schemaV1;
        schemaV1.Sprintf(schema, "Sealed");

        ASSERT_EQ(SUCCESS, SetupECDb("UpgradeModifierFromSealed.ecdb", SchemaItem(schemaV1))) << "Rel is sealed";

        Utf8String schemaV2;
        schemaV2.Sprintf(schema, "Abstract");
        EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "Upgrading to Abstract";

        schemaV2.Sprintf(schema, "None");
        EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "Upgrading to None should fail because RelECClassId would have to be added";
        }

        //*** Logical FK
        {
        Utf8CP schema = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="A">
                <ECProperty propertyName="Price" typeName="double" />
            </ECEntityClass>
            <ECEntityClass typeName="B">
                <ECProperty propertyName="Name" typeName="string" />
                <ECNavigationProperty propertyName="A" relationshipName="AHasB" direction="Backward"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="AHasB" strength="Referencing" modifier="%s">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="A">
                    <Class class ="A" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                    <Class class ="B" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

        Utf8String schemaV1;
        schemaV1.Sprintf(schema, "Sealed");

        ASSERT_EQ(SUCCESS, SetupECDb("UpgradeModifierFromSealed.ecdb", SchemaItem(schemaV1))) << "Rel is sealed";

        Utf8String schemaV2;
        schemaV2.Sprintf(schema, "Abstract");
        EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "Upgrading to Abstract";

        schemaV2.Sprintf(schema, "None");
        EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "Upgrading to None should fail because RelECClassId would have to be added";
        }

        //*** Link Table
        {
        Utf8CP schema = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="A">
                <ECProperty propertyName="Price" typeName="double" />
            </ECEntityClass>
            <ECEntityClass typeName="B">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECRelationshipClass typeName="AHasB" strength="Referencing" modifier="%s">
                <Source multiplicity="(0..*)" polymorphic="True" roleLabel="A">
                    <Class class ="A" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                    <Class class ="B" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

        Utf8String schemaV1;
        schemaV1.Sprintf(schema, "Sealed");

        ASSERT_EQ(SUCCESS, SetupECDb("UpgradeModifierFromSealed.ecdb", SchemaItem(schemaV1))) << "Rel is sealed";

        Utf8String schemaV2;
        schemaV2.Sprintf(schema, "Abstract");
        EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "Upgrading to Abstract never supported";

        schemaV2.Sprintf(schema, "None");
        EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "Upgrading to None is not supported for link tables as it would change the mapping type";
        }
    }

//-------------------------------------------------------------------------------------- 
// @bsimethod                                 Krischan.Eberle                       07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, UpgradeModifierFromNone)
    {
        //*** Physical FK
                {
                Utf8CP schema = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="A">
                <ECProperty propertyName="Price" typeName="double" />
            </ECEntityClass>
            <ECEntityClass typeName="B">
                <ECProperty propertyName="Name" typeName="string" />
                <ECNavigationProperty propertyName="A" relationshipName="AHasB" direction="Backward">
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                </ECNavigationProperty>
            </ECEntityClass>
            <ECRelationshipClass typeName="AHasB" strength="Referencing" modifier="%s">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="A">
                    <Class class ="A" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                    <Class class ="B" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

                Utf8String schemaV1;
                schemaV1.Sprintf(schema, "None");

                ASSERT_EQ(SUCCESS, SetupECDb("UpgradeModifierFromNone.ecdb", SchemaItem(schemaV1))) << "None";

                Utf8String schemaV2;
                schemaV2.Sprintf(schema, "Abstract");
                EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "Upgrading to Abstract";

                schemaV2.Sprintf(schema, "Sealed");
                EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "Upgrading to Sealed should fail because RelECClassId is no longer needed";
                }

                //*** Logical FK
                {
                Utf8CP schema = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="A">
                <ECProperty propertyName="Price" typeName="double" />
            </ECEntityClass>
            <ECEntityClass typeName="B">
                <ECProperty propertyName="Name" typeName="string" />
                <ECNavigationProperty propertyName="A" relationshipName="AHasB" direction="Backward"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="AHasB" strength="Referencing" modifier="%s">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="A">
                    <Class class ="A" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                    <Class class ="B" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

                Utf8String schemaV1;
                schemaV1.Sprintf(schema, "None");

                ASSERT_EQ(SUCCESS, SetupECDb("UpgradeModifierFromNone.ecdb", SchemaItem(schemaV1))) << "Rel is sealed";

                Utf8String schemaV2;
                schemaV2.Sprintf(schema, "Abstract");
                EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "Upgrading to Abstract";

                schemaV2.Sprintf(schema, "Sealed");
                EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "Upgrading to Sealed should fail because RelECClassId is no longer needed";
                }

                //*** Link Table
                {
                Utf8CP schema = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="A">
                <ECProperty propertyName="Price" typeName="double" />
            </ECEntityClass>
            <ECEntityClass typeName="B">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECRelationshipClass typeName="AHasB" strength="Referencing" modifier="%s">
                <Source multiplicity="(0..*)" polymorphic="True" roleLabel="A">
                    <Class class ="A" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                    <Class class ="B" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

                Utf8String schemaV1;
                schemaV1.Sprintf(schema, "None");

                ASSERT_EQ(SUCCESS, SetupECDb("UpgradeModifierFromNone.ecdb", SchemaItem(schemaV1))) << "Rel is sealed";

                Utf8String schemaV2;
                schemaV2.Sprintf(schema, "Abstract");
                EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "Upgrading to Abstract never supported";

                schemaV2.Sprintf(schema, "Sealed");
                EXPECT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(schemaV2)));
                }
        }


//-------------------------------------------------------------------------------------- 
// @bsimethod                                 Krischan.Eberle                       07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, UpgradeModifierFromAbstract)
    {
    //*** Physical FK
            {
            Utf8CP schema = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="A">
                <ECProperty propertyName="Price" typeName="double" />
            </ECEntityClass>
            <ECEntityClass typeName="B">
                <ECProperty propertyName="Name" typeName="string" />
                <ECNavigationProperty propertyName="A" relationshipName="AHasB" direction="Backward">
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                </ECNavigationProperty>
            </ECEntityClass>
            <ECRelationshipClass typeName="AHasB" strength="Referencing" modifier="%s">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="A">
                    <Class class ="A" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                    <Class class ="B" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

            Utf8String schemaV1;
            schemaV1.Sprintf(schema, "Abstract");

            ASSERT_EQ(SUCCESS, SetupECDb("UpgradeModifierFromAbstract.ecdb", SchemaItem(schemaV1))) << "None";

            Utf8String schemaV2;
            schemaV2.Sprintf(schema, "None");
            EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "To None: Upgrading from Abstract is generally not supported";

            schemaV2.Sprintf(schema, "Sealed");
            EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "Upgrading to Sealed should fail because RelECClassId is no longer needed";
            }

            //*** Logical FK
            {
            Utf8CP schema = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="A">
                <ECProperty propertyName="Price" typeName="double" />
            </ECEntityClass>
            <ECEntityClass typeName="B">
                <ECProperty propertyName="Name" typeName="string" />
                <ECNavigationProperty propertyName="A" relationshipName="AHasB" direction="Backward"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="AHasB" strength="Referencing" modifier="%s">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="A">
                    <Class class ="A" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                    <Class class ="B" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

            Utf8String schemaV1;
            schemaV1.Sprintf(schema, "Abstract");

            ASSERT_EQ(SUCCESS, SetupECDb("UpgradeModifierFromAbstract.ecdb", SchemaItem(schemaV1))) << "Rel is sealed";

            Utf8String schemaV2;
            schemaV2.Sprintf(schema, "None");
            EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "To None: Upgrading from Abstract is generally not supported";

            schemaV2.Sprintf(schema, "Sealed");
            EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "Upgrading to Sealed should fail because RelECClassId is no longer needed";
            }

            //*** Link Table
            {
            Utf8CP schema = R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="A">
                <ECProperty propertyName="Price" typeName="double" />
            </ECEntityClass>
            <ECEntityClass typeName="B">
                <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECRelationshipClass typeName="AHasB" strength="Referencing" modifier="%s">
                <Source multiplicity="(0..*)" polymorphic="True" roleLabel="A">
                    <Class class ="A" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="B">
                    <Class class ="B" />
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

            Utf8String schemaV1;
            schemaV1.Sprintf(schema, "Abstract");

            ASSERT_EQ(SUCCESS, SetupECDb("UpgradeModifierFromAbstract.ecdb", SchemaItem(schemaV1))) << "Rel is sealed";

            Utf8String schemaV2;
            schemaV2.Sprintf(schema, "None");
            EXPECT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "To None: Upgrading from Abstract should be supported";

            schemaV2.Sprintf(schema, "Sealed");
            EXPECT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(schemaV2))) << "To Sealed: Upgrading from Abstract should be supported";
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Zaighum                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
struct ECDbHoldingRelationshipStrengthTestFixture : ECDbTestFixture
    {
    protected:
        bool InstanceExists(Utf8CP classExp, ECInstanceKey const& key) const
            {
            Utf8String ecsql;
            ecsql.Sprintf("SELECT NULL FROM %s WHERE ECInstanceId=?", classExp);
            ECSqlStatement stmt;
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));

            DbResult stat = stmt.Step();
            EXPECT_TRUE(stat == BE_SQLITE_ROW || stat == BE_SQLITE_DONE);
            return stat == BE_SQLITE_ROW;
            };

        bool RelationshipExists(Utf8CP relClassExp, ECInstanceKey const& sourceKey, ECInstanceKey const& targetKey) const
            {
            Utf8String ecsql;
            ecsql.Sprintf("SELECT NULL FROM %s WHERE SourceECInstanceId=? AND SourceECClassId=? AND TargetECInstanceId=? AND TargetECClassId=?", relClassExp);
            ECSqlStatement stmt;
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(1, sourceKey.GetInstanceId()));
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(2, sourceKey.GetClassId()));
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(3, targetKey.GetInstanceId()));
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(4, targetKey.GetClassId()));

            DbResult stat = stmt.Step();
            EXPECT_TRUE(stat == BE_SQLITE_ROW || stat == BE_SQLITE_DONE);
            return stat == BE_SQLITE_ROW;
            };
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Zaighum                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, OneToOneForward)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
                                 SchemaItem(
                                     R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.0">
                         <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
                           <ECEntityClass typeName="Geometry" >
                             <ECProperty propertyName="Type" typeName="string" />
                           </ECEntityClass>
                           <ECEntityClass typeName="GeometryPart" >
                             <ECProperty propertyName="Stream" typeName="binary" />
                             <ECNavigationProperty propertyName="Geometry" relationshipName="GeometryHoldsParts" direction="Backward"/>
                           </ECEntityClass>
                           <ECRelationshipClass typeName="GeometryHoldsParts" strength="holding" strengthDirection="Forward" modifier="Sealed">
                              <Source cardinality="(0,1)" polymorphic="True">
                                  <Class class="Geometry" />
                              </Source>
                             <Target cardinality="(0,1)" polymorphic="True">
                                 <Class class="GeometryPart" />
                              </Target>
                           </ECRelationshipClass>
                         </ECSchema>)xml"))) << "1:N and holding";

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Geometry(Type) VALUES(?)"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey1));
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey2));
    }

    {
    //Create relationships:
    //Geom-Part
    //1-1
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.GeometryPart SET Geometry.Id=? WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    }

    m_ecdb.SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(m_ecdb, "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());
    delGeomStmt.Reset();
    delGeomStmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_TRUE(RelationshipExists("ts.GeometryHoldsParts", geomKey1, partKey1)) << "ForeignKeyConstraint is missing, therefore ECSQL DELETE doesn't delete affected relationships";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Zaighum                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, OneToOneBackward)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
                                 SchemaItem(
                                     R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.0">
                           <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
                           <ECEntityClass typeName="Geometry" >
                             <ECProperty propertyName="Type" typeName="string" />
                           </ECEntityClass>
                           <ECEntityClass typeName="GeometryPart" >
                             <ECProperty propertyName="Stream" typeName="binary" />
                            <ECNavigationProperty propertyName="Geometry" relationshipName="PartHeldByGeometry" direction="Forward"/>
                           </ECEntityClass>
                           <ECRelationshipClass typeName="PartHeldByGeometry" strength="holding" strengthDirection="Backward" modifier="Sealed">
                             <Source cardinality="(0,1)" polymorphic="True">
                                 <Class class="GeometryPart" />
                              </Source>
                              <Target cardinality="(0,1)" polymorphic="True">
                                  <Class class="Geometry" />
                              </Target>
                           </ECRelationshipClass>
                         </ECSchema>)xml"))) << "1:N and holding";

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Geometry(Type) VALUES(?)"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey1));
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey2));
    }
    {
    //Create relationships:
    //Geom-Part
    //1-1
    m_ecdb.SaveChanges();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.GeometryPart SET Geometry.Id =? WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    }

    m_ecdb.SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(m_ecdb, "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());
    delGeomStmt.Reset();
    delGeomStmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_TRUE(RelationshipExists("ts.PartHeldByGeometry", partKey1, geomKey1)) << "ForeignKeyConstraint is missing, therefore ECSQL DELETE doesn't delete affected relationships";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Zaighum                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, OneToManyForward)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
                                 SchemaItem(
                                     R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.0">
                           <ECEntityClass typeName="Geometry" >
                             <ECProperty propertyName="Type" typeName="string" />
                             <ECNavigationProperty propertyName="Part" relationshipName="GeometryHoldsParts" direction="Backward"/>
                           </ECEntityClass>
                           <ECEntityClass typeName="GeometryPart" >
                             <ECProperty propertyName="Stream" typeName="binary" />
                           </ECEntityClass>
                           <ECRelationshipClass typeName="GeometryHoldsParts" strength="holding" strengthDirection="Forward" modifier="Sealed">
                             <Source cardinality="(0,1)" polymorphic="True">
                                 <Class class="GeometryPart" />
                              </Source>
                              <Target cardinality="(0,N)" polymorphic="True">
                                  <Class class="Geometry" />
                              </Target>
                           </ECRelationshipClass>
                         </ECSchema>)xml"))) << "1:N and holding";

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Geometry(Type) VALUES(?)"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.Geometry SET Part.Id=? WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey2.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    m_ecdb.SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(m_ecdb, "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());
    delGeomStmt.Reset();
    delGeomStmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_FALSE(RelationshipExists("ts.GeometryHoldsParts", partKey1, geomKey1)) << "ECSQL DELETE deletes affected relationships";
    ASSERT_TRUE(RelationshipExists("ts.GeometryHoldsParts", partKey1, geomKey2));

    //delete Geom2
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey2.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1)) << "Part 1 is not held anymore, but will only be deleted by Purge";
    ASSERT_FALSE(RelationshipExists("ts.GeometryHoldsParts", partKey1, geomKey2));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Zaighum                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, OneToManyBackward)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
                                 SchemaItem(
                                     R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.0">
                           <ECEntityClass typeName="Geometry" >
                             <ECProperty propertyName="Type" typeName="string" />
                             <ECNavigationProperty propertyName="Part" relationshipName="PartIsHeldByGeometry" direction="Forward"/>
                           </ECEntityClass>
                           <ECEntityClass typeName="GeometryPart" >
                             <ECProperty propertyName="Stream" typeName="binary" />
                           </ECEntityClass>
                           <ECRelationshipClass typeName="PartIsHeldByGeometry" strength="holding" strengthDirection="Backward" modifier="Sealed">
                              <Source cardinality="(0,N)" polymorphic="True">
                                  <Class class="Geometry" />
                              </Source>
                             <Target cardinality="(0,1)" polymorphic="True">
                                 <Class class="GeometryPart" />
                              </Target>
                           </ECRelationshipClass>
                         </ECSchema>)xml"))) << "1:N and holding";

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Geometry(Type) VALUES(?)"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE TestSchema.Geometry SET Part.Id=? WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    m_ecdb.SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(m_ecdb, "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());
    delGeomStmt.Reset();
    delGeomStmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_FALSE(RelationshipExists("ts.PartIsHeldByGeometry", geomKey1, partKey1)) << "ECSQL DELETE deletes affected relationships";
    ASSERT_TRUE(RelationshipExists("ts.PartIsHeldByGeometry", geomKey2, partKey1));

    //delete Geom2
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey2.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1)) << "Part 1 is not held anymore, but will only be deleted by Purge";
    ASSERT_FALSE(RelationshipExists("ts.PartIsHeldByGeometry", geomKey2, partKey1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, ManyToManyForward)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbrelationshipmappingrules_manytomanyandholding.ecdb",
                                 SchemaItem(
                                     R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.0">
                           <ECEntityClass typeName="Geometry" >
                             <ECProperty propertyName="Type" typeName="string" />
                           </ECEntityClass>
                           <ECEntityClass typeName="GeometryPart" >
                             <ECProperty propertyName="Stream" typeName="binary" />
                           </ECEntityClass>
                           <ECRelationshipClass typeName="GeometryHasParts" strength="holding" strengthDirection="Forward" modifier="Sealed">
                              <Source cardinality="(0,N)" polymorphic="True">
                                  <Class class="Geometry" />
                              </Source>
                             <Target cardinality="(0,N)" polymorphic="True">
                                 <Class class="GeometryPart" />
                              </Target>
                           </ECRelationshipClass>
                         </ECSchema>)xml"))) << "N:N and holding";

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Geometry(Type) VALUES(?)"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GeometryHasParts(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey1.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey2.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey2.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey2.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    m_ecdb.SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(m_ecdb, "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetInstanceId()));
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
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbrelationshipmappingrules_manytomanyandholding.ecdb",
                                 SchemaItem(
                                     R"xml(<ECSchema schemaName="TestSchema" nameSpacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.0">
                           <ECEntityClass typeName="Geometry" >
                             <ECProperty propertyName="Type" typeName="string" />
                           </ECEntityClass>
                           <ECEntityClass typeName="GeometryPart" >
                             <ECProperty propertyName="Stream" typeName="binary" />
                           </ECEntityClass>
                           <ECRelationshipClass typeName="PartsHeldByGeometry" strength="holding" strengthDirection="Backward" modifier="Sealed">
                             <Source cardinality="(0,N)" polymorphic="True">
                                 <Class class="GeometryPart" />
                              </Source>
                              <Target cardinality="(0,N)" polymorphic="True">
                                  <Class class="Geometry" />
                              </Target>
                           </ECRelationshipClass>
                         </ECSchema>)xml"))) << "N:N and holding";

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Geometry(Type) VALUES(?)"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.PartsHeldByGeometry(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey1.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey2.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey1.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey2.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey2.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    m_ecdb.SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(m_ecdb, "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetInstanceId()));
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
struct RelationshipsAndSharedTablesTestFixture : ECDbTestFixture
    {
    protected:
        static Utf8CP const SCHEMA_XML;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const RelationshipsAndSharedTablesTestFixture::SCHEMA_XML =
R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="test" nameSpacePrefix="t" version="1.0" description="Schema covers all the cases in which base class is OwnTable(Polymorphic)" displayLabel="Table Per Hierarchy" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.0">
    <ECSchemaReference name="ECDbMap" version="02.00" prefix="ecdbmap" />
    <ECEntityClass typeName="Base">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="P0" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassA" >
        <BaseClass>Base</BaseClass>
        <ECProperty propertyName="P1" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="ClassB" >
        <BaseClass>ClassA</BaseClass>
        <ECProperty propertyName="P2" typeName="string" />
    </ECEntityClass>
    <ECRelationshipClass typeName="BaseOwnsBase" strength="referencing" strengthDirection="forward" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <Source cardinality="(0,N)" polymorphic="True">
            <Class class="Base" />
        </Source>
        <Target cardinality="(0,N)" polymorphic="True">
            <Class class="Base" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BaseHasClassA" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <BaseClass>BaseOwnsBase</BaseClass>
        <Source cardinality="(0,1)" polymorphic="True">
            <Class class="Base" />
        </Source>
        <Target cardinality="(0,1)" polymorphic="True">
            <Class class="ClassA" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="BaseHasClassB" strength="referencing" strengthDirection="forward" modifier="Sealed">
        <BaseClass>BaseOwnsBase</BaseClass>
        <Source cardinality="(0,1)" polymorphic="True">
            <Class class="Base" />
        </Source>
        <Target cardinality="(0,1)" polymorphic="True">
            <Class class="ClassB" />
        </Target>
    </ECRelationshipClass>
</ECSchema>)xml";

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipsAndSharedTablesTestFixture, UniqueIndexesSupportFor1to1Relationship)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("RelationshipsAndTPH.ecdb", SchemaItem(SCHEMA_XML)));

    BeSQLite::Statement stmt;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT Id from ec_Class where ec_Class.Name = 'BaseHasClassA'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, stmt.Step());
    ECClassId classId = stmt.GetValueId<ECClassId>(0);
    stmt.Finalize();

    //verify that entry in the ec_Index table exists for relationship table BaseHasClassA
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT Name, IsUnique from ec_Index where ClassId = ?"));
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.BindId(1, classId));
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        ASSERT_EQ(1, stmt.GetValueInt(1)) << "Index value for 1:1 Relationship is not Unique";
        Utf8String indexName = stmt.GetValueText(0);
        ASSERT_TRUE(indexName == "idx_ECRel_Source_Unique_t_BaseOwnsBase" || "idx_ECRel_Target_Unique_t_BaseOwnsBase");
        }
    stmt.Finalize();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT Id from ec_Class where ec_Class.Name = 'BaseHasClassB'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, stmt.Step());
    classId = stmt.GetValueId<ECClassId>(0);
    stmt.Finalize();

    //verify that entry in ec_Index table also exists for relationship table BaseHasClassB
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT Name, IsUnique from ec_Index where ClassId = ?"));
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.BindId(1, classId));
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        ASSERT_EQ(1, stmt.GetValueInt(1)) << "Index value for 1:1 Relationship is not Unique";
        Utf8String indexName = stmt.GetValueText(0);
        ASSERT_TRUE(indexName == "uix_unique_t_BaseHasClassB_Source" || "uix_unique_t_BaseHasClassB_Target");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipsAndSharedTablesTestFixture, InstanceDeletionFromPolymorphicRelationships)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("RelationshipsAndTPH.ecdb", SchemaItem(SCHEMA_XML)));

    ASSERT_TRUE(GetHelper().TableExists("t_BaseOwnsBase"));
    ASSERT_FALSE(GetHelper().TableExists("t_BaseHasClassA"));
    ASSERT_FALSE(GetHelper().TableExists("t_BaseHasClassB"));

    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("test", true);
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

    ECInstanceInserter inserter(m_ecdb, *baseClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());

    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*baseInstance1, true));
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*baseInstance2, true));

    //Insert Instances for ClassA
    ECN::StandaloneECInstancePtr classAInstance1 = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECN::StandaloneECInstancePtr classAInstance2 = classA->GetDefaultStandaloneEnabler()->CreateInstance();

    classAInstance1->SetValue("P1", ECValue("string1"));
    classAInstance2->SetValue("P1", ECValue("string2"));

    ECInstanceInserter classAinserter(m_ecdb, *classA, nullptr);
    ASSERT_TRUE(classAinserter.IsValid());

    ASSERT_EQ(BE_SQLITE_OK, classAinserter.Insert(*classAInstance1, true));
    ASSERT_EQ(BE_SQLITE_OK, classAinserter.Insert(*classAInstance2, true));

    //Insert Instances for ClassB
    ECN::StandaloneECInstancePtr classBInstance1 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECN::StandaloneECInstancePtr classBInstance2 = classB->GetDefaultStandaloneEnabler()->CreateInstance();

    classBInstance1->SetValue("P2", ECValue("string1"));
    classBInstance2->SetValue("P2", ECValue("string2"));

    ECInstanceInserter classBinserter(m_ecdb, *classB, nullptr);
    ASSERT_TRUE(classBinserter.IsValid());

    ASSERT_EQ(BE_SQLITE_OK, classBinserter.Insert(*classBInstance1, true));
    ASSERT_EQ(BE_SQLITE_OK, classBinserter.Insert(*classBInstance2, true));

    //Get Relationship Classes
    ECRelationshipClassCP baseHasClassAClass = schema->GetClassCP("BaseHasClassA")->GetRelationshipClassCP();
    ASSERT_TRUE(baseHasClassAClass != nullptr);
    ECRelationshipClassCP baseHasClassBClass = schema->GetClassCP("BaseHasClassB")->GetRelationshipClassCP();
    ASSERT_TRUE(baseHasClassBClass != nullptr);

    {//Insert Instances for Relationship TPHhasClassA
    ECN::StandaloneECRelationshipInstancePtr relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*baseHasClassAClass)->CreateRelationshipInstance();
    ECInstanceInserter relationshipinserter(m_ecdb, *baseHasClassAClass, nullptr);
    ASSERT_TRUE(relationshipinserter.IsValid());

    {//Inserting 1st Instance
    relationshipInstance->SetSource(baseInstance1.get());
    relationshipInstance->SetTarget(classAInstance1.get());
    relationshipInstance->SetInstanceId("source->target");
    ASSERT_EQ(BE_SQLITE_OK, relationshipinserter.Insert(*relationshipInstance));
    }
    {//Inserting 2nd Instance
    relationshipInstance->SetSource(baseInstance2.get());
    relationshipInstance->SetTarget(classAInstance2.get());
    relationshipInstance->SetInstanceId("source->target");
    ASSERT_EQ(BE_SQLITE_OK, relationshipinserter.Insert(*relationshipInstance));
    }
    }

    {//Insert Instances for Relationship TPHhasClassB
    ECN::StandaloneECRelationshipInstancePtr relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*baseHasClassBClass)->CreateRelationshipInstance();
    ECInstanceInserter relationshipinserter(m_ecdb, *baseHasClassBClass, nullptr);
    ASSERT_TRUE(relationshipinserter.IsValid());

    {//Inserting 1st Instance
    relationshipInstance->SetSource(baseInstance1.get());
    relationshipInstance->SetTarget(classBInstance1.get());
    relationshipInstance->SetInstanceId("source->target");
    ASSERT_EQ(BE_SQLITE_OK, relationshipinserter.Insert(*relationshipInstance));
    }
    {//Inserting 2nd Instance
    relationshipInstance->SetSource(baseInstance2.get());
    relationshipInstance->SetTarget(classBInstance2.get());
    relationshipInstance->SetInstanceId("source->target");
    ASSERT_EQ(BE_SQLITE_OK, relationshipinserter.Insert(*relationshipInstance));
    }
    }
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM t.Base"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(6, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(4, stmt.GetValueInt(0));
    stmt.Finalize();

    //Deletes the instances of BaseOwnsBase class..
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "DELETE FROM ONLY t.BaseOwnsBase"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(4, stmt.GetValueInt(0));
    stmt.Finalize();

    //Deletes the instances of BaseHasClassA class..
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ONLY t.BaseHasClassA"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(2, stmt.GetValueInt(0));
    stmt.Finalize();

    //Deletes the instances of BaseHasClassB class..
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ONLY t.BaseHasClassB"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0));
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipsAndSharedTablesTestFixture, RetrieveConstraintClassInstanceBeforeAfterInsertingRelationshipInstance)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("RelationshipsAndTPH.ecdb", SchemaItem(SCHEMA_XML)));

    ASSERT_TRUE(GetHelper().TableExists("t_BaseOwnsBase"));
    ASSERT_FALSE(GetHelper().TableExists("t_BaseHasClassA"));
    ASSERT_FALSE(GetHelper().TableExists("t_BaseHasClassB"));

    ECSqlStatement insertStatement;
    ECInstanceKey TPHKey1;
    ECInstanceKey TPHKey2;
    ASSERT_EQ(ECSqlStatus::Success, insertStatement.Prepare(m_ecdb, "INSERT INTO t.Base (P0) VALUES ('string1')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStatement.Step(TPHKey1));
    ASSERT_TRUE(TPHKey1.IsValid());
    insertStatement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, insertStatement.Prepare(m_ecdb, "INSERT INTO t.Base (P0) VALUES ('string2')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStatement.Step(TPHKey2));
    ASSERT_TRUE(TPHKey2.IsValid());
    insertStatement.Finalize();

    ECInstanceKey classAKey1;
    ECInstanceKey classAKey2;
    ASSERT_EQ(ECSqlStatus::Success, insertStatement.Prepare(m_ecdb, "INSERT INTO t.ClassA (P1) VALUES ('string1')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStatement.Step(classAKey1));
    ASSERT_TRUE(classAKey1.IsValid());
    insertStatement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, insertStatement.Prepare(m_ecdb, "INSERT INTO t.ClassA (P1) VALUES ('string2')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStatement.Step(classAKey2));
    ASSERT_TRUE(classAKey2.IsValid());
    insertStatement.Finalize();

    //retrieve ECInstance from Db before inserting Relationship Instance, based on ECInstanceId, verify ECInstance is valid
    ECSqlStatement selectStmt;
    ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(m_ecdb, "SELECT * FROM t.Base WHERE ECInstanceId = ?"));
    selectStmt.BindId(1, TPHKey1.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
    ECInstanceECSqlSelectAdapter TPHadapter(selectStmt);
    IECInstancePtr readInstance = TPHadapter.GetInstance();
    ASSERT_TRUE(readInstance.IsValid());
    selectStmt.Finalize();

    ECSqlStatement relationStmt;
    ASSERT_EQ(relationStmt.Prepare(m_ecdb, "INSERT INTO t.BaseHasClassA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    relationStmt.BindId(1, TPHKey1.GetInstanceId());
    relationStmt.BindId(2, TPHKey1.GetClassId());
    relationStmt.BindId(3, classAKey1.GetInstanceId());
    relationStmt.BindId(4, classAKey1.GetClassId());
    ASSERT_EQ(BE_SQLITE_DONE, relationStmt.Step());
    relationStmt.Finalize();

    //try to insert Duplicate relationship step() should return error
    ASSERT_EQ(relationStmt.Prepare(m_ecdb, "INSERT INTO t.BaseHasClassA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    relationStmt.BindId(1, TPHKey1.GetInstanceId());
    relationStmt.BindId(2, TPHKey1.GetClassId());
    relationStmt.BindId(3, classAKey1.GetInstanceId());
    relationStmt.BindId(4, classAKey1.GetClassId());
    ASSERT_TRUE((BE_SQLITE_CONSTRAINT_BASE & relationStmt.Step()) == BE_SQLITE_CONSTRAINT_BASE);
    relationStmt.Finalize();

    //retrieve ECInstance from Db After Inserting Relationship Instance, based on ECInstanceId, verify ECInstance is valid
    ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(m_ecdb, "SELECT * FROM t.ClassA WHERE ECInstanceId = ?"));
    selectStmt.BindId(1, classAKey1.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
    ECInstanceECSqlSelectAdapter ClassAadapter(selectStmt);
    readInstance = ClassAadapter.GetInstance();
    ASSERT_TRUE(readInstance.IsValid());
    }

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
struct RelationshipStrengthTestFixture : ECDbTestFixture
    {
    protected:
        static Utf8CP GetTestSchemaXml() {  return R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="RelationshipStrengthTest" alias="rst" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
	        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Person" modifier="None">
                <ECProperty propertyName="FirstName" typeName="string" displayLabel="First Name" readOnly="false"/>
                <ECProperty propertyName="LastName" typeName="string" displayLabel="Last Name" readOnly="false"/>
                <ECNavigationProperty propertyName="Parent1" relationshipName="SingleParentHasChildren" direction="Backward" >
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                </ECNavigationProperty>
                <ECNavigationProperty propertyName="Parent2" relationshipName="SingleParentHasChildren_backward" direction="Forward" >
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                </ECNavigationProperty>
                <ECNavigationProperty propertyName="Spouse1" relationshipName="ParentHasSpouse" direction="Backward" >
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                </ECNavigationProperty>
                <ECNavigationProperty propertyName="Spouse2" relationshipName="ParentHasSpouse_backward" direction="Forward" >
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                </ECNavigationProperty>
            </ECEntityClass>
            <ECRelationshipClass typeName="SingleParentHasChildren" modifier="Sealed" strength="embedding" strengthDirection="forward">
                <Source multiplicity="(0..1)" roleLabel="Parent" polymorphic="true">
                    <Class class="Person"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="Children" polymorphic="true">
                    <Class class="Person"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="SingleParentHasChildren_backward" modifier="Sealed" strength="embedding" strengthDirection="backward">
                <Source multiplicity="(0..*)" roleLabel="Children" polymorphic="true">
                    <Class class="Person"/>
                </Source>
                <Target multiplicity="(0..1)" roleLabel="Single parent" polymorphic="true">
                    <Class class="Person"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="ChildrenHaveManyParents" modifier="Sealed" strength="referencing" strengthDirection="backward">
                <Source multiplicity="(0..*)" roleLabel="Children" polymorphic="true">
                    <Class class="Person"/>
                </Source>
                <Target multiplicity="(1..*)" roleLabel="Parents" polymorphic="true">
                    <Class class="Person"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="ManyParentsHaveChildren" modifier="Sealed" strength="referencing" strengthDirection="forward">
                <Source multiplicity="(1..*)" roleLabel="Parents" polymorphic="true">
                    <Class class="Person"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="Children" polymorphic="true">
                    <Class class="Person"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="ParentHasSpouse" modifier="Sealed" strength="referencing" strengthDirection="forward">
                <Source multiplicity="(0..1)" roleLabel="Parent" polymorphic="true">
                    <Class class="Person"/>
                </Source>
                <Target multiplicity="(0..1)" roleLabel="Spouse" polymorphic="true">
                    <Class class="Person"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="ParentHasSpouse_backward" modifier="Sealed" strength="referencing" strengthDirection="backward">
                <Source multiplicity="(0..1)" roleLabel="Parent" polymorphic="true">
                    <Class class="Person"/>
                </Source>
                <Target multiplicity="(0..1)" roleLabel="Spouse" polymorphic="true">
                    <Class class="Person"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>
                )xml";
        }

    //---------------------------------------------------------------------------------------
    //                                               Muhammad Hassan                  10/2014
    //+---------------+---------------+---------------+---------------+---------------+------
    ECInstanceKey InsertPerson(Utf8CP firstName, Utf8CP lastName, ECInstanceId parent1Id = ECInstanceId(), ECInstanceId parent2Id = ECInstanceId(), ECInstanceId spouse1Id = ECInstanceId(), ECInstanceId spouse2Id = ECInstanceId())
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO RelationshipStrengthTest.Person(FirstName,LastName,Parent1.Id,Parent2.Id,Spouse1.Id,Spouse2.Id) VALUES('%s','%s',?,?,?,?)", firstName, lastName);
        ECSqlStatement stmt;
        if (stmt.Prepare(m_ecdb, ecsql.c_str()) != ECSqlStatus::Success)
            return ECInstanceKey();

        if (parent1Id.IsValid())
            stmt.BindId(1, parent1Id);

        if (parent2Id.IsValid())
            stmt.BindId(2, parent2Id);

        if (spouse1Id.IsValid())
            stmt.BindId(3, spouse1Id);

        if (spouse2Id.IsValid())
            stmt.BindId(4, spouse2Id);

        ECInstanceKey key;
        stmt.Step(key);
        return key;
        }

    //---------------------------------------------------------------------------------------
    //                                               Muhammad Hassan                  10/2014
    //+---------------+---------------+---------------+---------------+---------------+------
    ECInstanceKey InsertLinkTableRelationship(Utf8CP relationshipClassECSqlName, ECInstanceKey const& source, ECInstanceKey const& target)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO %s(SourceECInstanceId, TargetECInstanceId) VALUES(%s,%s)", relationshipClassECSqlName, source.GetInstanceId().ToString().c_str(),
                      target.GetInstanceId().ToString().c_str());
        ECSqlStatement stmt;
        if (stmt.Prepare(m_ecdb, ecsql.c_str()) != ECSqlStatus::Success)
            return ECInstanceKey();

        ECInstanceKey key;
        stmt.Step(key);
        return key;
        }

    //---------------------------------------------------------------------------------------
    //                                               Muhammad Hassan                  10/2014
    //+---------------+---------------+---------------+---------------+---------------+------
    BentleyStatus DeleteInstance(ECInstanceKey const& key)
        {
        ECClassCP ecClass = m_ecdb.Schemas().GetClass(key.GetClassId());
        if (ecClass == nullptr)
            return ERROR;

        Utf8String ecsql;
        ecsql.Sprintf("DELETE FROM %s WHERE ECInstanceId=%s", ecClass->GetECSqlName().c_str(), key.GetInstanceId().ToString().c_str());
        ECSqlStatement stmt;
        if (stmt.Prepare(m_ecdb, ecsql.c_str()) != ECSqlStatus::Success)
            return ERROR;

        return BE_SQLITE_DONE == stmt.Step() ? SUCCESS : ERROR;
        }

    //---------------------------------------------------------------------------------------
    //                                               Muhammad Hassan                  10/2014
    //+---------------+---------------+---------------+---------------+---------------+------
    bool HasInstance(ECInstanceKey const& key)
        {
        ECClassCP ecClass = m_ecdb.Schemas().GetClass(key.GetClassId());
        if (ecClass == nullptr)
            return false;

        Utf8String ecsql;
        ecsql.Sprintf("SELECT NULL FROM ONLY %s WHERE ECInstanceId=%s",
                      ecClass->GetECSqlName().c_str(), key.GetInstanceId().ToString().c_str());

        ECSqlStatement statement;
        if (ECSqlStatus::Success != statement.Prepare(m_ecdb, ecsql.c_str()))
            return false;

        return statement.Step() == BE_SQLITE_ROW;
        }

    //---------------------------------------------------------------------------------------
    //                                               Muhammad Hassan                  10/2014
    //+---------------+---------------+---------------+---------------+---------------+------
    bool IsNavigationPropertySet(ECInstanceKey const& key, Utf8CP navPropName)
        {
        ECClassCP ecClass = m_ecdb.Schemas().GetClass(key.GetClassId());
        if (ecClass == nullptr)
            return false;

        Utf8String ecsql;
        ecsql.Sprintf("SELECT NULL FROM ONLY %s WHERE ECInstanceId=%s AND %s.Id IS NOT NULL",
                      ecClass->GetECSqlName().c_str(), key.GetInstanceId().ToString().c_str(), navPropName);

        ECSqlStatement statement;
        if (ECSqlStatus::Success != statement.Prepare(m_ecdb, ecsql.c_str()))
            return false;

        return statement.Step() == BE_SQLITE_ROW;
        }

    };


/*---------------------------------------------------------------------------------**//**
* @bsitest                                    Muhammad.Hassan                   10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelationshipStrengthTestFixture, BackwardEmbedding)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("BackwardRelationshipStrengthTest.ecdb", SchemaItem(GetTestSchemaXml())));
    /*
    *                                           SingleParent
    *                                                 |
    *                                                 | NavProp Parent2 (SingleParentHasChildren_backward (Backward EMBEDDING))
    *         ________________________________________|______________________________________
    *        |                                        |                                      |
    *      Child1                                   Child2                                 Child3
    */
    ECInstanceKey singleParent = InsertPerson("Only", "singleParent");

    ECInstanceKey child1 = InsertPerson("First", "Child", ECInstanceId(), singleParent.GetInstanceId());
    ECInstanceKey child2 = InsertPerson("Second", "Child" , ECInstanceId(), singleParent.GetInstanceId());
    ECInstanceKey child3 = InsertPerson("Third", "Child", ECInstanceId(), singleParent.GetInstanceId());


    /*
    * Test 1: Delete Child1
    * Validate child1HasSingleParent,  child1 have been deleted
    * Validate singleParent, child2HasSingleParent, child3HasSingleParent, child2, child3 are still there
    */
    DeleteInstance(child1);
    m_ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(child1));

    ASSERT_TRUE(HasInstance(singleParent));
    ASSERT_TRUE(HasInstance(child2));
    ASSERT_FALSE(IsNavigationPropertySet(child2, "Parent1"));
    ASSERT_TRUE(IsNavigationPropertySet(child2, "Parent2"));
    ASSERT_TRUE(HasInstance(child3));
    ASSERT_FALSE(IsNavigationPropertySet(child3, "Parent1"));
    ASSERT_TRUE(IsNavigationPropertySet(child3, "Parent2"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelationshipStrengthTestFixture, RelationshipTest)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("BackwardRelationshipStrengthTest.ecdb", SchemaItem(GetTestSchemaXml())));

    /*
     *          Create the following relationship hierarchy
     *
     * GrandParent1  <- ParentHasSpouse (REFERENCING) -> GrandParent2
     *     |__________________________________________________|
     *                             |
     *                             | ManyParentsHaveChildren (REFERENCING)
     *                             |
     *                         SingleParent
     *                             |
     *                             | SingleParentHasChildren (EMBEDDING)
     *      _______________________|__________________________
     *     |                                                  |
     *   Child1                                             Child2
     *
     */

    ECInstanceKey grandParent1 = InsertPerson("First", "GrandParent");
    ECInstanceKey grandParent2 = InsertPerson("Second", "GrandParent", ECInstanceId(), ECInstanceId(), grandParent1.GetInstanceId());
    ECInstanceKey singleParent = InsertPerson("Only", "SingleParent");
    ECInstanceKey child1 = InsertPerson("First", "Child", singleParent.GetInstanceId());
    ECInstanceKey child2 = InsertPerson("Second", "Child", singleParent.GetInstanceId());

    // Referencing relationship (GrandParent1, GrandParent2 -> SingleParent)
    ECInstanceKey grandParent1HasSingleParent = InsertLinkTableRelationship("RelationshipStrengthTest.ManyParentsHaveChildren", grandParent1, singleParent);
    ECInstanceKey grandParent2HasSingleParent = InsertLinkTableRelationship("RelationshipStrengthTest.ManyParentsHaveChildren", grandParent2, singleParent);

    m_ecdb.SaveChanges();

    //Verify instances before deletion
    ASSERT_TRUE(IsNavigationPropertySet(grandParent2, "Spouse1"));
    ASSERT_FALSE(IsNavigationPropertySet(grandParent2, "Spouse2"));

    /*
    * Test 1: Delete GrandParent1
    * Validate grandParent1HasSpouse, grandParent2HasSpouse, grandParent1HasSingleParent have been deleted (orphaned relationships)
    * Validate singleParent is still around (referencing relationship with one parent remaining)
    */
    DeleteInstance(grandParent1);
    m_ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(grandParent1));
    ASSERT_FALSE(IsNavigationPropertySet(grandParent2, "Spouse1"));
    ASSERT_FALSE(IsNavigationPropertySet(grandParent2, "Spouse2"));
    ASSERT_FALSE(HasInstance(grandParent1HasSingleParent));

    ASSERT_TRUE(HasInstance(singleParent));

    /*
    * Test 2: Delete GrandParent2
    * Validate grandParent2HasSingleParent has been deleted (orphaned relationship), *Validate singeParent has been deleted (held instance with no parents remaining)
    */
    DeleteInstance(grandParent2);
    m_ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(grandParent2));
    ASSERT_FALSE(HasInstance(grandParent2HasSingleParent));

    ASSERT_TRUE(HasInstance(singleParent));
    ASSERT_TRUE(HasInstance(child1));
    ASSERT_TRUE(IsNavigationPropertySet(child1, "Parent1"));
    ASSERT_TRUE(HasInstance(child2));
    ASSERT_TRUE(IsNavigationPropertySet(child2, "Parent1"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                    Muhammad.Hassan                   10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelationshipStrengthTestFixture, BackwardHoldingForwardEmbedding)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("BackwardRelationshipStrengthTest.ecdb", SchemaItem(GetTestSchemaXml())));

    /*
    *          Create the following relationship hierarchy
    *
    * GrandParent1  <- ParentHasSpouse (REFERENCING) -> GrandParent2
    *     |__________________________________________________|
    *                             |
    *                             | ChildrenHaveManyParents.( Backward REFERENCING)
    *                             |
    *                         SingleParent
    *                             |
    *                             | SingleParentHasChildren.( Forward EMBEDDING)
    *      _______________________|__________________________
    *     |                                                  |
    *   Child1                                             Child2
    *
    */
    ECInstanceKey grandParent1 = InsertPerson("First", "GrandParent");
    ECInstanceKey grandParent2 = InsertPerson("Second", "GrandParent", ECInstanceId(), ECInstanceId(), ECInstanceId(), grandParent1.GetInstanceId());
    ECInstanceKey singleParent = InsertPerson("Only", "singleParent");
    ECInstanceKey child1 = InsertPerson("First", "Child", singleParent.GetInstanceId());
    ECInstanceKey child2 = InsertPerson("Second", "Child", singleParent.GetInstanceId());

    // Backward referencing relationship (GrandParent1, GrandParent2 <- SingleParent)
    ECInstanceKey singleParentHasGrandParent1 = InsertLinkTableRelationship("RelationshipStrengthTest.ChildrenHaveManyParents", singleParent, grandParent1);
    ECInstanceKey singleParentHasGrandParent2 = InsertLinkTableRelationship("RelationshipStrengthTest.ChildrenHaveManyParents", singleParent, grandParent2);

    m_ecdb.SaveChanges();

    //Validate Instance exists before deletion
    ASSERT_TRUE(IsNavigationPropertySet(child1, "Parent1"));

    /*
    * Test 1: Delete Child1
    * Validate Child1 and singleParentHasChild1 have been deleted
    * Validate Child2 is Still there
    */
    DeleteInstance(child1);
    m_ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(child1));
    ASSERT_FALSE(IsNavigationPropertySet(child1, "Parent1"));

    ASSERT_TRUE(HasInstance(child2));
    ASSERT_TRUE(IsNavigationPropertySet(child2, "Parent1"));

    /*
    * Test 2: Delete Child2
    * Validate Child2 and singleParentHasChild2 have been deleted
    * Validate singleParent is still around (relationship grand parents remaining)
    */
    DeleteInstance(child2);
    m_ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(child2));
    ASSERT_FALSE(IsNavigationPropertySet(child2, "Parent1"));

    ASSERT_TRUE(HasInstance(singleParent));

    /*
    * Test 3: Delete GrandParent1
    * Validate GrandParent1, grandParent1HasSpouse, grandParent2HasSpouse, singleParentHasGrandParent1 have been deleted
    * Validate singleParent is still around (referencing relationship with one parent remaining)
    */
    DeleteInstance(grandParent1);
    m_ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(grandParent1));
    ASSERT_FALSE(IsNavigationPropertySet(grandParent2, "Spouse1"));
    ASSERT_FALSE(IsNavigationPropertySet(grandParent2, "Spouse2"));

    ASSERT_FALSE(HasInstance(singleParentHasGrandParent1));
    ASSERT_TRUE(HasInstance(singleParent));

    /*
    * Test 4: Delete GrandParent2
    * Validate GrandParent2, singleParentHasGrandParent2 have been deleted, * Single parent has been deleted too as no parent exists anymore
    */
    DeleteInstance(grandParent2);
    m_ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(grandParent2));
    ASSERT_FALSE(HasInstance(singleParentHasGrandParent2));
    ASSERT_TRUE(HasInstance(singleParent));
    }

END_ECDBUNITTESTS_NAMESPACE
