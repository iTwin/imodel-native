/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/RelationshipTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
struct RelationshipMappingTestFixture : DbMappingTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                  02/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, RelationshipMapping_FailingScenarios)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
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

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
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

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
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

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
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

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
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

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
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
    
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
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
            "</ECSchema>"))) << "ForeignKey mapping can only have a CA when the mapping strategy is set to NotMapped.";
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, IndexCreationForRelationships)
    {
            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="A" modifier="None" >
                        <ECProperty propertyName="AId" typeName="string" />
                        <ECNavigationProperty propertyName="PartnerB" relationshipName="Rel11Backwards" direction="Forward">
                            <ECCustomAttributes>
                                <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                            </ECCustomAttributes>
                        </ECNavigationProperty>
                    </ECEntityClass>
                    <ECEntityClass typeName="B" modifier="None">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECNavigationProperty propertyName="AId" relationshipName="Rel" direction="Backward">
                            <ECCustomAttributes>
                                <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                            </ECCustomAttributes>
                        </ECNavigationProperty>
                        <ECNavigationProperty propertyName="PartnerA" relationshipName="Rel11" direction="Backward">
                            <ECCustomAttributes>
                                <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                            </ECCustomAttributes>
                        </ECNavigationProperty>
                        <ECProperty propertyName="BId" typeName="long" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BB" modifier="None">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="BBId" typeName="long" />
                    </ECEntityClass>
                   <ECRelationshipClass typeName="Rel" strength="embedding" modifier="Sealed">
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                      <Class class="B" />
                    </Target>
                  </ECRelationshipClass>
                   <ECRelationshipClass typeName="Rel11" strength="embedding" modifier="Sealed">
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="relates">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(1..1)" polymorphic="True" roleLabel="relates">
                      <Class class="B" />
                    </Target>
                  </ECRelationshipClass>
                   <ECRelationshipClass typeName="Rel11Backwards" strength="embedding" strengthDirection="Backward" modifier="Sealed">
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="relates">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(1..1)" polymorphic="True" roleLabel="relates">
                      <Class class="B" />
                    </Target>
                  </ECRelationshipClass>
                   <ECRelationshipClass typeName="RelNN" strength="referencing" modifier="Sealed">
                    <Source multiplicity="(1..*)" polymorphic="True" roleLabel="references">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(1..*)" polymorphic="True" roleLabel="references">
                      <Class class="B" />
                    </Target>
                  </ECRelationshipClass>
                </ECSchema>)xml"), "indexcreationforrelationships1.ecdb"));

            AssertIndex(ecdb, "ix_ts1_B_fk_ts1_Rel_target", false, "ts1_B", {"AId"});
            AssertIndex(ecdb, "uix_ts1_B_fk_ts1_Rel11_target", true, "ts1_B", {"PartnerAId"});
            AssertIndex(ecdb, "uix_ts1_A_fk_ts1_Rel11Backwards_source", true, "ts1_A", {"PartnerBId"});

            AssertIndex(ecdb, "ix_ts1_RelNN_source", false, "ts1_RelNN", {"SourceId"});
            AssertIndex(ecdb, "ix_ts1_RelNN_target", false, "ts1_RelNN", {"TargetId"});
            AssertIndex(ecdb, "uix_ts1_RelNN_sourcetarget", true, "ts1_RelNN", {"SourceId", "TargetId"});
            }

            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="A" modifier="None" >
                        <ECProperty propertyName="AId" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B" modifier="None">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                            <ShareColumns xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                        <ECProperty propertyName="AId" typeName="long" />
                        <ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward">
                            <ECCustomAttributes>
                                <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                            </ECCustomAttributes>
                        </ECNavigationProperty>
                        <ECProperty propertyName="BId" typeName="long" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BB" modifier="None">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="BBId" typeName="long" />
                    </ECEntityClass>
                   <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="embedding">
                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                      <Class class="B" />
                    </Target>
                  </ECRelationshipClass>
                </ECSchema>)xml"), "indexcreationforrelationships2.ecdb"));

            AssertIndex(ecdb, "ix_ts2_B_fk_ts2_Rel_target", false, "ts2_B", {"AId"}, "([AId] IS NOT NULL)");

            ASSERT_PROPERTYMAPPING_MULTICOL(ecdb, PropertyAccessString("TestSchema", "B", "A"),
                                  ColumnInfo::List({ {"A.Id", "ts2_b","AId"}, {"A.RelECClassId", "ts2_b","ARelECClassId", true} }));
            }

            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts3' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='A' modifier='None' >"
                "        <ECProperty propertyName='Code' typeName='string' />"
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
                "        <ECNavigationProperty propertyName='AId' relationshipName='Rel' direction='Backward' >"
                "           <ECCustomAttributes>"
                "               <ForeignKeyConstraint xmlns='ECDbMap.02.00' />"
                "           </ECCustomAttributes>"
                "        </ECNavigationProperty>"
                "    </ECEntityClass>"
                "   <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='embedding'>"
                "    <Source cardinality='(0,1)' polymorphic='True'>"
                "      <Class class='A' />"
                "    </Source>"
                "    <Target cardinality='(0,N)' polymorphic='True'>"
                "      <Class class='BB'/>"
                "    </Target>"
                "  </ECRelationshipClass>"
                "</ECSchema>"), "indexcreationforrelationships3.ecdb"));

            AssertIndex(ecdb, "ix_ts3_B_fk_ts3_Rel_target", false, "ts3_B", {"AId"}, "([AId] IS NOT NULL)");
            }

            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts4' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='A' modifier='None' >"
                "        <ECProperty propertyName='Code' typeName='string' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "            </ClassMap>"
                "            <ShareColumns xmlns='ECDbMap.02.00'/>"
                "        </ECCustomAttributes>"
                "        <ECNavigationProperty propertyName='AId' relationshipName='Rel11' direction='Backward' >"
                "           <ECCustomAttributes>"
                "               <ForeignKeyConstraint xmlns='ECDbMap.02.00' />"
                "           </ECCustomAttributes>"
                "        </ECNavigationProperty>"
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
                "</ECSchema>"), "indexcreationforrelationships4.ecdb"));


            AssertIndex(ecdb, "uix_ts4_B_fk_ts4_Rel11_target", true, "ts4_B", {"AId"}, "([AId] IS NOT NULL)");
            }

            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts50" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="A" modifier="None">
                        <ECProperty propertyName="Code" typeName="string" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B" modifier="None">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                             </ClassMap>
                        </ECCustomAttributes>
                        <ECNavigationProperty propertyName="A" relationshipName="RelBase" direction="Backward">
                           <ECCustomAttributes>
                               <ForeignKeyConstraint xmlns="ECDbMap.02.00" />
                           </ECCustomAttributes>
                        </ECNavigationProperty>
                    </ECEntityClass>
                    <ECEntityClass typeName="B1" modifier="None">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="B1Id" typeName="long" />
                    </ECEntityClass>
                   <ECRelationshipClass typeName="RelBase" modifier="Abstract" strength="referencing">
                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                      <Class class="A"/>
                    </Source>
                    <Target multiplicity="(1..*)" polymorphic="True" roleLabel="is referenced by">
                      <Class class="B"/>
                    </Target>
                  </ECRelationshipClass>
                   <ECRelationshipClass typeName="RelSub1" modifier="Sealed" strength="referencing">
                    <BaseClass>RelBase</BaseClass>
                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(1..1)" polymorphic="True" roleLabel="is referenced by">
                      <Class class="B1"/>
                    </Target>
                  </ECRelationshipClass>
                </ECSchema>)xml"), "indexcreationforrelationships50.ecdb"));

            ASSERT_EQ(3, (int) RetrieveIndicesForTable(ecdb, "ts50_B").size()) << "Expected indices: class id index, user defined index; no indexes for the relationship constraints";

            AssertIndex(ecdb, "ix_ts50_B_fk_ts50_RelBase_target", false, "ts50_B", {"AId"}, "([AId] IS NOT NULL)");
            AssertIndex(ecdb, "ix_ts50_B_ARelECClassId", false, "ts50_B", {"ARelECClassId"}, "([ARelECClassId] IS NOT NULL)");
            AssertIndexExists(ecdb, "uix_ts50_B_fk_ts50_RelSub1_target", false);
            }

            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts5' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='A' modifier='None'>"
                "        <ECProperty propertyName='Code' typeName='string' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "             </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECNavigationProperty propertyName='AId' relationshipName='RelBase' direction='Backward'>"
                "           <ECCustomAttributes>"
                "               <ForeignKeyConstraint xmlns='ECDbMap.02.00' />"
                "           </ECCustomAttributes>"
                "        </ECNavigationProperty>"
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
                "</ECSchema>"), "indexcreationforrelationships5.ecdb"));

            ASSERT_EQ(3, (int) RetrieveIndicesForTable(ecdb, "ts5_B").size()) << "Expected indices: class id index, user defined index; no indexes for the relationship constraints";

            AssertIndex(ecdb, "ix_ts5_B_fk_ts5_RelBase_target", false, "ts5_B", {"AId"}, "([AId] IS NOT NULL)");
            AssertIndex(ecdb, "ix_ts5_B_ARelECClassId", false, "ts5_B", {"ARelECClassId"}, "([ARelECClassId] IS NOT NULL)");
            AssertIndexExists(ecdb, "uix_ts5_B_fk_ts5_RelSub1_target", false);
            }

            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts6' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='A' modifier='None'>"
                "        <ECProperty propertyName='Code' typeName='string' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "             </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECNavigationProperty propertyName='AInstance' relationshipName='RelBase' direction='Backward' >"
                "           <ECCustomAttributes>"
                "               <ForeignKeyConstraint xmlns='ECDbMap.02.00' />"
                "           </ECCustomAttributes>"
                "        </ECNavigationProperty>"
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
                "</ECSchema>"), "indexcreationforrelationships6.ecdb"));

            ASSERT_EQ(3, (int) RetrieveIndicesForTable(ecdb, "ts6_B").size()) << "Expected indices: class id index, user defined index; no indexes for the relationship constraints";

            AssertIndex(ecdb, "ix_ts6_B_AInstanceRelECClassId", false, "ts6_B", {"AInstanceRelECClassId"});
            AssertIndex(ecdb, "ix_ts6_B_fk_ts6_RelBase_target", false, "ts6_B", {"AInstanceId"});
            AssertIndexExists(ecdb, "uix_ts6_B_fk_ts6_RelSub1_target", false);
            }


            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts7' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='B' modifier='None'>"
                "        <ECCustomAttributes>"
                "            <ClassMap xmlns='ECDbMap.02.00'>"
                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                "             </ClassMap>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='Code' typeName='long' />"
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
                "</ECSchema>"), "indexcreationforrelationships7.ecdb"));

            ASSERT_EQ(9, (int) RetrieveIndicesForTable(ecdb, "ts7_RelBase").size());
            }

            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts8" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="A" modifier="None">
                        <ECProperty propertyName="AId" typeName="long" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B" modifier="None">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                             </ClassMap>
                        </ECCustomAttributes>
                        <ECProperty propertyName="Code" typeName="long" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B1" modifier="None">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="B1Code" typeName="long" />
                        <ECNavigationProperty propertyName="A1" relationshipName="RelPoly" direction="Backward" >
                           <ECCustomAttributes>
                               <ForeignKeyConstraint xmlns="ECDbMap.02.00" />
                           </ECCustomAttributes>
                        </ECNavigationProperty>
                        <ECNavigationProperty propertyName="A2" relationshipName="RelNonPoly" direction="Backward" >
                           <ECCustomAttributes>
                               <ForeignKeyConstraint xmlns="ECDbMap.02.00" />
                           </ECCustomAttributes>
                        </ECNavigationProperty>
                    </ECEntityClass>
                    <ECEntityClass typeName="B11" modifier="None">
                        <BaseClass>B1</BaseClass>
                        <ECProperty propertyName="B11Code" typeName="long" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B2" modifier="None">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="B2Code" typeName="long" />
                    </ECEntityClass>
                   <ECRelationshipClass typeName="RelNonPoly" modifier="Sealed" strength="referencing">
                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(0..1)" polymorphic="False" roleLabel="references">
                      <Class class="B1" />
                    </Target>
                  </ECRelationshipClass>
                   <ECRelationshipClass typeName="RelPoly" modifier="Sealed" strength="referencing">
                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                      <Class class="B1" />
                    </Target>
                  </ECRelationshipClass>
                </ECSchema>)xml"), "indexcreationforrelationships8.ecdb"));

            ASSERT_EQ(3, (int) RetrieveIndicesForTable(ecdb, "ts8_B").size());

            ECClassId b1ClassId = ecdb.Schemas().GetClassId("TestSchema", "B1");
            ECClassId b11ClassId = ecdb.Schemas().GetClassId("TestSchema", "B11");

            //RelNonPoly must exclude index on B11 as the constraint is non-polymorphic
            Utf8String indexWhereClause;
            indexWhereClause.Sprintf("([A2Id] IS NOT NULL) AND (ECClassId=%s)", b1ClassId.ToString().c_str());

            AssertIndex(ecdb, "uix_ts8_B_fk_ts8_RelNonPoly_target", true, "ts8_B", {"A2Id"}, indexWhereClause.c_str());

            //RelPoly must include index on B11 as the constraint is polymorphic
            indexWhereClause.Sprintf("([A1Id] IS NOT NULL) AND (ECClassId=%s OR ECClassId=%s)", b1ClassId.ToString().c_str(), b11ClassId.ToString().c_str());
            AssertIndex(ecdb, "uix_ts8_B_fk_ts8_RelPoly_target", true, "ts8_B", {"A1Id"}, indexWhereClause.c_str());
            }

            {
            //Tests that AllowDuplicateRelationships Flag from LinkTableRelationshipMap CA is applied to subclasses
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts9\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                                "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                                "  <ECEntityClass typeName='A' modifier='None'>"
                                "    <ECProperty propertyName='Name' typeName='string' />"
                                "  </ECEntityClass>"
                                "  <ECEntityClass typeName='B' modifier='None'>"
                                "    <ECCustomAttributes>"
                                "        <ClassMap xmlns='ECDbMap.02.00'>"
                                "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                "        </ClassMap>"
                                "    </ECCustomAttributes>"
                                "    <ECProperty propertyName='BName' typeName='string' />"
                                "  </ECEntityClass>"
                                "  <ECEntityClass typeName='C' modifier='None'>"
                                "    <BaseClass>B</BaseClass>"
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
                                "</ECSchema>"), "indexcreationforrelationships9.ecdb"));

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
    StandaloneECInstancePtr ClassA_Instance = ClassA->GetDefaultStandaloneEnabler()->CreateInstance();
    ClassA_Instance->SetValue("AA", ECValue("val1"));

    //Instance of ClassB
    StandaloneECInstancePtr ClassB_Instance = ClassB->GetDefaultStandaloneEnabler()->CreateInstance();
    ClassB_Instance->SetValue("BB", ECValue("val3"));

    //Inserter of ClassA
    ECInstanceInserter ClassA_Inserter(m_ecdb, *ClassA, nullptr);
    ASSERT_TRUE(ClassA_Inserter.IsValid());
    ClassA_Inserter.Insert(*ClassA_Instance);

    //Inserter of ClassB
    ECInstanceInserter ClassB_Inserter(m_ecdb, *ClassB, nullptr);
    ASSERT_TRUE(ClassB_Inserter.IsValid());
    ClassB_Inserter.Insert(*ClassB_Instance);

    ECRelationshipClassCP AHasB = m_ecdb.Schemas().GetClass("TestSchema", "AHasB")->GetRelationshipClassCP();
    ECRelationshipClassCP BHasC = m_ecdb.Schemas().GetClass("TestSchema", "BHasC")->GetRelationshipClassCP();

    //Inserting relationship instance.
    ECN::StandaloneECRelationshipInstancePtr ClassAHasB_relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*AHasB)->CreateRelationshipInstance();
    ClassAHasB_relationshipInstance->SetSource(ClassA_Instance.get());
    ClassAHasB_relationshipInstance->SetTarget(ClassB_Instance.get());

    ECInstanceInserter AHasB_relationshipInserter(m_ecdb, *AHasB, nullptr);
    AHasB_relationshipInserter.Insert(*ClassAHasB_relationshipInstance);


    //Inserting instances of ClassC
    StandaloneECInstancePtr ClassC_Instance = ClassC->GetDefaultStandaloneEnabler()->CreateInstance();
    ClassC_Instance->SetValue("CC", ECValue("val5"));

    //Inserter of ClassC
    ECInstanceInserter ClassC_Inserter(m_ecdb, *ClassC, nullptr);
    ASSERT_TRUE(ClassC_Inserter.IsValid());
    ClassC_Inserter.Insert(*ClassC_Instance);

    //Inserting relationship instances.
    ECN::StandaloneECRelationshipInstancePtr ClassBHasC_relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*BHasC)->CreateRelationshipInstance();
    ClassBHasC_relationshipInstance->SetSource(ClassB_Instance.get());
    ClassBHasC_relationshipInstance->SetTarget(ClassC_Instance.get());

    ECInstanceInserter BHasC_relationshipInserter(m_ecdb, *BHasC, nullptr);
    BHasC_relationshipInserter.Insert(*ClassBHasC_relationshipInstance);

    //Deletes instance of ClassA. Instances of ClassB and ClassC are also deleted.
    ECSqlStatement stmt;
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Child(Name) VALUES('Child1')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(childKey)) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.SpecialChild(Name,SpecialName) VALUES('Child2','I am special')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(specialChildKey)) << stmt.GetECSql();
    stmt.Finalize();
    
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "INSERT INTO ts.Rel(SourceECInstanceId, TargetECInstanceId) VALUES(?,?)"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Child SET MyParent.Id=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, parentKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
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

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId, TargetECInstanceId, TargetECClassId FROM ts.Rel ORDER BY TargetECInstanceId"));
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
              SchemaItem("Diamond Problem",
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
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

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(101, 10000)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(102, 20000)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(103, 30000)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(104, 40000)"));

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, Utf8PrintfString("INSERT INTO ts.SecondaryClassA(ECInstanceId, T1, PrimaryClassA.Id, PrimaryClassA.RelECClassId) VALUES(201, 10000, 101, %ld)", primaryClassAHasSecondaryClassBId.GetValue()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.SecondaryClassA(ECInstanceId, T1, PrimaryClassA.Id) VALUES(202, 20000, 102)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.SecondaryClassA(ECInstanceId, T1) VALUES(203, 30000)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.SecondaryClassA(ECInstanceId, T1) VALUES(204, 40000)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, Utf8PrintfString("UPDATE ts.SecondaryClassA SET PrimaryClassA.Id = 103, T1=300002, PrimaryClassA.RelECClassId = %ld  WHERE ECInstanceId = 203", primaryClassAHasSecondaryClassBId.GetValue()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.PrimaryClassAHasSecondaryClassB(SourceECInstanceId, TargetECInstanceId) VALUES(104, 204)"));
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

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.Car            (Name                                      ) VALUES ('BMW-S')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, SqlPrintfString("INSERT INTO ts.Engine         (Code, www, Volumn,Car.Id,Car.RelECClassId ) VALUES ('CODE-1','www1', 2000.0,1,%d )", relId.GetValue())));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, SqlPrintfString("INSERT INTO ts.Sterring       (Code, www, Type,Car.Id,Car.RelECClassId   ) VALUES ('CODE-2','www2', 'S-Type',1,%d)", relId.GetValue())));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.Tire           (Code, Diameter                            ) VALUES ('CODE-3', 15.0)"));


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
              SchemaItem("Diamond Problem",
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

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb,                 "INSERT INTO ts.Car            (Name                                      ) VALUES ('BMW-S')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, SqlPrintfString("INSERT INTO ts.Engine         (Code, www, Volumn,Car.Id,Car.RelECClassId ) VALUES ('CODE-1','www1', 2000.0,1,%d )", relId.GetValue())));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, SqlPrintfString("INSERT INTO ts.Sterring       (Code, www, Type,Car.Id,Car.RelECClassId   ) VALUES ('CODE-2','www2', 'S-Type',1,%d)", relId.GetValue())));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb,                 "INSERT INTO ts.Tire           (Code, Diameter                            ) VALUES ('CODE-3', 15.0)"));


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
// @bsimethod                                   Krischan.Eberle                     03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, LogicalForeignKeyRelationshipMappedToUnsharedColumn)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("logicalfk_unsharedcol.ecdb",
              SchemaItem("Diamond Problem",
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

    AssertForeignKey(false, m_ecdb, "ts_Element", "ModelId");
    AssertIndexExists(m_ecdb, "x_ts_Element_fk_ts_Rel_target", false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MixinAsRelationshipEnd)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("diamond_problem3.ecdb",
              SchemaItem("Diamond Problem",
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
                         "      <Target multiplicity='(0..N)' polymorphic='True' roleLabel='B'>"
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

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.Car(Name) VALUES ('BMW-S')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.Engine(Code, www, Volumn,Car.Id) VALUES ('CODE-1','www1', 2000.0,1 )"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.Sterring(Code, www, Type,Car.Id) VALUES ('CODE-2','www2', 'S-Type',1)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.Tire(Code, Diameter) VALUES ('CODE-3', 15.0)"));


    m_ecdb.Schemas().CreateClassViewsInDb();
    m_ecdb.SaveChanges();
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.CarHasEndPoint"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(ECInstanceId(UINT64_C(1)), stmt.GetValueId<ECInstanceId>(0));
    ASSERT_EQ(ECClassId(UINT64_C(49)), stmt.GetValueId<ECClassId>(1));

    ASSERT_EQ(ECInstanceId(UINT64_C(2)), stmt.GetValueId<ECInstanceId>(2));
    ASSERT_EQ(ECClassId(UINT64_C(52)), stmt.GetValueId<ECClassId>(3));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(ECInstanceId(UINT64_C(1)), stmt.GetValueId<ECInstanceId>(0));
    ASSERT_EQ(ECClassId(UINT64_C(49)), stmt.GetValueId<ECClassId>(1));
    ASSERT_EQ(ECInstanceId(UINT64_C(3)), stmt.GetValueId<ECInstanceId>(2));
    ASSERT_EQ(ECClassId(UINT64_C(54)), stmt.GetValueId<ECClassId>(3));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MixinAsRelationshipEnd2)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("diamond_problem3.ecdb",
              SchemaItem("Diamond Problem",
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
                         "      <Target multiplicity='(0..N)' polymorphic='True' roleLabel='B'>"
                         "        <Class class='IEndPoint' />"
                         "     </Target>"
                         "      <ECProperty propertyName='Tag' typeName='string' />"
                         "  </ECRelationshipClass>"
                         "  <ECRelationshipClass typeName='CarHasEndPoint2' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                         "      <BaseClass>CarHasEndPoint</BaseClass>"
                         "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
                         "         <Class class='Car' />"
                         "     </Source>"
                         "      <Target multiplicity='(0..N)' polymorphic='True' roleLabel='B'>"
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
              SchemaItem("Diamond Problem",
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
                         "      <Target multiplicity='(0..N)' polymorphic='True' roleLabel='B'>"
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

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.Car            (Name              ) VALUES ('BMW-S')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.Engine         (Code, www, Volumn ) VALUES ('CODE-1','www1', 2000.0 )"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.Sterring       (Code, www, Type   ) VALUES ('CODE-2','www2', 'S-Type')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.Tire           (Code, Diameter    ) VALUES ('CODE-3', 15.0)"));

    ASSERT_EQ(SUCCESS, ImportSchema(m_ecdb, SchemaItem("<ECSchema schemaName='TestSchema' alias='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                                       "      <Target multiplicity='(0..N)' polymorphic='True' roleLabel='B'>"
                                                       "        <Class class='IEndPoint' />"
                                                       "     </Target>"
                                                       "      <ECProperty propertyName='Tag' typeName='string' />"
                                                       "  </ECRelationshipClass>"
                                                       "  <ECRelationshipClass typeName='CarHasEndPoint2' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                                                       "      <BaseClass>CarHasEndPoint</BaseClass>"
                                                       "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
                                                       "         <Class class='Car' />"
                                                       "     </Source>"
                                                       "      <Target multiplicity='(0..N)' polymorphic='True' roleLabel='B'>"
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

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.CarHasEndPoint2 (SourceECInstanceId, TargetECInstanceId, TargetECClassId, Tag, Rule) VALUES (1,2,54,'tag1','Rule1')"));
    m_ecdb.SaveChanges();
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO ts.CarHasEndPoint2 (SourceECInstanceId, TargetECInstanceId, TargetECClassId, Tag, Rule) VALUES (1,3,56,'tag2','Rule2')"));
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
    ASSERT_TRUE(m_ecdb.TableExists("ts_AHasB"));
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

    AssertForeignKey(true, m_ecdb, "ts_LinkTableWithFk1", "SourceId");
    AssertForeignKey(true, m_ecdb, "ts_LinkTableWithFk1", "TargetId");
    AssertForeignKey(true, m_ecdb, "ts_LinkTableWithFk2", "SourceId");
    AssertForeignKey(true, m_ecdb, "ts_LinkTableWithFk2", "TargetId");
    AssertForeignKey(true, m_ecdb, "ts_LinkTableWithFk3", "SourceId");
    AssertForeignKey(true, m_ecdb, "ts_LinkTableWithFk3", "TargetId");
    AssertForeignKey(false, m_ecdb, "ts_LinkTableWithoutFk");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, AmbigousRelationshipProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ambigousRelationshipProperty.ecdb",
              SchemaItem("N:N and holding",
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
                         "</ECSchema>")));
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
            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                                                    "</ECSchema>"), "notnullconstraintsonfkcolumns.ecdb")) << "relationship classes with nav props";

            Utf8String ddl = RetrieveDdl(ecdb, "ts_B");
            ASSERT_FALSE(ddl.empty());

            ASSERT_TRUE(ddl.ContainsI("[AId_Rel0NId] INTEGER,")) << "relationship classes with nav props> Actual DDL: " << ddl.c_str();
            ASSERT_TRUE(ddl.ContainsI("[AId_Rel1NId] INTEGER NOT NULL,")) << "relationship classes with nav props> Actual DDL: " << ddl.c_str();
            ASSERT_TRUE(ddl.ContainsI("[AId_RelN0Id] INTEGER,")) << "relationship classes with nav props> Actual DDL: " << ddl.c_str();
            ASSERT_TRUE(ddl.ContainsI("[AId_RelN1Id] INTEGER NOT NULL,")) << "relationship classes with nav props> Actual DDL: " << ddl.c_str();
            }

            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem(
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
                                "</ECSchema>"), "notnullconstraintsonfkcolumns.ecdb")) << "relationship classes with custom fk names";

            Utf8String ddl = RetrieveDdl(ecdb, "ts_B");
            ASSERT_FALSE(ddl.empty());

            ASSERT_TRUE(ddl.ContainsI("[A1Id] INTEGER NOT NULL,")) << "relationship classes with custom fk names> Actual DDL: " << ddl.c_str();
            ASSERT_TRUE(ddl.ContainsI("[A2Id] INTEGER,")) << "relationship classes with custom fk names> Actual DDL: " << ddl.c_str();
            ASSERT_TRUE(ddl.ContainsI("[A3Id] INTEGER,")) << "relationship classes with custom fk names> Actual DDL: " << ddl.c_str();
            ASSERT_TRUE(ddl.ContainsI("[A4Id] INTEGER NOT NULL,")) << "relationship classes with custom fk names> Actual DDL: " << ddl.c_str();
            }


            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                "</ECSchema>"), "notnullconstraintsonfkcolumns.ecdb")) << "(1,1) rel with dropped NOT NULL constraint";

            Utf8String ddl = RetrieveDdl(ecdb, "ts_B");
            ASSERT_FALSE(ddl.empty());

            ASSERT_TRUE(ddl.ContainsI("[AId] INTEGER,")) << "(1,1) rel with dropped NOT NULL constraint> Actual DDL: " << ddl.c_str();
            }

            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                "</ECSchema>"), "notnullconstraintsonfk.ecdb")) << "Logical FK";

            Utf8String ddl = RetrieveDdl(ecdb, "ts_Base");
            ASSERT_FALSE(ddl.empty());
            ASSERT_TRUE(ddl.ContainsI("[AId] INTEGER NOT NULL,")) << "Logical FK> Actual DDL: " << ddl.c_str();
            }

            {
            ECDb ecdb;
            ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                "</ECSchema>"), "notnullconstraintsonfk.ecdb"));

            Utf8String ddl = RetrieveDdl(ecdb, "ts_Base");
            ASSERT_FALSE(ddl.empty());
            ASSERT_TRUE(ddl.ContainsI("[AId] INTEGER,")) <<  "Logical FK with dropped not null constraint> Actual DDL: " << ddl.c_str();
            }

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

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                        "</ECSchema>"), "fkcolumnposition.ecdb")) << "Nav Prop as first prop";

    AssertForeignKey(true, ecdb, "ts_Base", "ParentId");
    //Subsubclasses come before sibling classes, therefore parent id is after AAProp1
    assertColumnPosition(ecdb, "ts_Base", "ParentId", 4, "Nav Prop as first prop");
    }

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                                            "</ECSchema>"), "fkcolumnposition.ecdb")) << "Nav prop as last property";

    AssertForeignKey(true, ecdb, "ts_Base", "ParentId");
    assertColumnPosition(ecdb, "ts_Base", "ParentId", 5, "Nav prop as last property");
    }

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                        "</ECSchema>"), "fkcolumnposition.ecdb")) << "Two Nav props in a row";

    AssertForeignKey(true, ecdb, "ts_Base", "Parent1Id");
    AssertForeignKey(true, ecdb, "ts_Base", "Parent2Id");
    //WIP: Column order for two nav props in a row is not correct yet. Once fixed, flip positions in the below calls.
    assertColumnPosition(ecdb, "ts_Base", "Parent1Id", 5, "Two Nav props in a row");
    assertColumnPosition(ecdb, "ts_Base", "Parent2Id", 6, "Two Nav props in a row");
    }

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                        "</ECSchema>"), "fkcolumnposition.ecdb")) << "Nav prop is only prop";

    AssertForeignKey(true, ecdb, "ts_Base", "ParentId");
    assertColumnPosition(ecdb, "ts_Base", "ParentId", 4, "Nav prop is only prop");
    }

    {
    ECDb ecdb;
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                        "</ECSchema>"), "fkcolumnposition.ecdb")) << "Nav Prop in class with shared columns";

    AssertForeignKey(true, ecdb, "ts_Base", "ParentId");
    assertColumnPosition(ecdb, "ts_Base", "ParentId", -1, "Nav Prop in class with shared columns");
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, OneToOneRelationshipMapping)
    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem("embedding relationships", "<?xml version='1.0' encoding='utf-8'?>"
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

    testSchemas.push_back(SchemaItem("holding relationships", "<?xml version='1.0' encoding='utf-8'?>"
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

    testSchemas.push_back(SchemaItem("referencing relationships", "<?xml version='1.0' encoding='utf-8'?>"
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
        ECDb ecdb;
        ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, testSchema, "onetoonerelationshipmappings.ecdb"));

        AssertForeignKey(true, ecdb, "ts_b", "A1Id");
        AssertForeignKey(true, ecdb, "ts_b", "A2Id");
        AssertForeignKey(true, ecdb, "ts_b", "A3Id");
        AssertForeignKey(true, ecdb, "ts_b", "A4Id");
        AssertForeignKey(true, ecdb, "ts_a", "B1Id");
        AssertForeignKey(true, ecdb, "ts_a", "B2Id");
        AssertForeignKey(true, ecdb, "ts_a", "B3Id");
        AssertForeignKey(true, ecdb, "ts_a", "B4Id");

        AssertForeignKey(false, ecdb, "ts_a", "A1Id");
        AssertForeignKey(false, ecdb, "ts_a", "A2Id");
        AssertForeignKey(false, ecdb, "ts_a", "A3Id");
        AssertForeignKey(false, ecdb, "ts_a", "A4Id");
        AssertForeignKey(false, ecdb, "ts_b", "B1Id");
        AssertForeignKey(false, ecdb, "ts_b", "B2Id");
        AssertForeignKey(false, ecdb, "ts_b", "B3Id");
        AssertForeignKey(false, ecdb, "ts_b", "B4Id");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, DisallowCascadingDeleteOnJoinedTable)
    {
    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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


    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
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

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
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
    auto getGeometrySourceHasGeometryRowCount = [] (ECDbCR ecdb)
        {
        ECSqlStatement selectStmt;
        if (ECSqlStatus::Success != selectStmt.Prepare(ecdb, "SELECT count(*) FROM ts.GeometrySourceHasGeometry"))
            return -1;

        if (BE_SQLITE_ROW != selectStmt.Step())
            return -1;

        return selectStmt.GetValueInt(0);
        };

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
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
        ECDb ecdb;
        ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(ecdb, testSchema, "RelationshipWithAbstractBaseClass.ecdb"));

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

        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(1, geomElem1Key.GetInstanceId()));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(2, geomElem1Key.GetClassId()));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(3, geom1Key.GetInstanceId()));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(4, geom1Key.GetClassId()));

        ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step()) << "Inserting GeometrySourceHasGeometry against GeometricElement is expected to succeed";
        insertStmt.Reset();
        insertStmt.ClearBindings();

        ASSERT_EQ(1, getGeometrySourceHasGeometryRowCount(ecdb)) << "After inserting one relationship [Scenario: " << testSchema.m_name << "]";

        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(1, elem1Key.GetInstanceId()));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(2, elem1Key.GetClassId()));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(3, geom2Key.GetInstanceId()));
        ASSERT_EQ(ECSqlStatus::Success, insertStmt.BindId(4, geom2Key.GetClassId()));
        ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step()) << "Inserting GeometrySourceHasGeometry against ExtendedElement is also expected to succeed";
        insertStmt.Reset();
        insertStmt.ClearBindings();
        }
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
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
        R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
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
        "</ECSchema>"))) << "N:N Relationship having not mapped constraint class on both sides of relationship are not supported";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
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
        "</ECSchema>"))) << "N:N Relationship having at least one NotMapped constraint class on both sides of relationship are not supported";

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
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
        "</ECSchema>"))) << "N:N Relationships having at least one NotMapped constraint class on one sides of relationship are not supported";
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
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')"));

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401, 201, %llu, 101, %llu)", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }

    //Select statements
    {
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(m_ecdb, "SELECT * FROM op.UNIT_HAS_ITEM"));

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId=%s", item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %s", item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId=%s", item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }
    sp.Cancel();

    m_ecdb.SaveChanges();
    ASSERT_EQ(SUCCESS, ImportSchema(m_ecdb, SchemaItem(
        "<?xml version = '1.0' encoding = 'utf-8'?>"
        "<ECSchema schemaName='OpenPlant_3D' nameSpacePrefix='op3d' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='OpenPlant' version='01.00' prefix='op' />"
        "  <ECEntityClass typeName='ITEM_3D' >"
        "    <BaseClass>op:ITEM</BaseClass>"
        "    <ECProperty propertyName='op3d_ITEM_prop' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>")));

    item = m_ecdb.Schemas().GetClass("OpenPlant", "ITEM");
    unit = m_ecdb.Schemas().GetClass("OpenPlant", "UNIT");
    ECClassCP item_3D = m_ecdb.Schemas().GetClass("OpenPlant_3D", "ITEM_3D");

    //Insert Statements
    {
    //relationship between UNIT and ITEM
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')"));

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401, 201, %llu, 101, %llu)", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));

    //relationship between UNIT and ITEM_3D(new derived Class)
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(202, 'unitString2')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op3d.ITEM_3D(ECInstanceId, op_ITEM_prop, op3d_ITEM_prop) VALUES(301, 'itemString1', 'item3dString1')"));

    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(402, 202, %llu, 301, %llu)", unit->GetId().GetValue(), item_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }

    //Select statements
    {
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(m_ecdb, "SELECT * FROM op.UNIT_HAS_ITEM"));

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));

    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));

    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE ECInstanceId = 402 AND TargetECClassId = %llu", item_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    //verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }
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

    ECClassCP unit = m_ecdb.Schemas().GetClass("OpenPlant", "UNIT");
    ECClassCP item = m_ecdb.Schemas().GetClass("OpenPlant", "ITEM");

    Savepoint sp(m_ecdb, "CRUD operations");
    //Insert Statements
    {
    //relationship between UNIT and ITEM
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')"));

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401, 201, %llu, 101, %llu)", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }

    //Select Statements
    {
    Utf8String ecsql;
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(m_ecdb, "SELECT * FROM op.UNIT_HAS_ITEM"));

    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }
    sp.Cancel();

    m_ecdb.SaveChanges();
    ASSERT_EQ(SUCCESS, ImportSchema(m_ecdb, SchemaItem(
        "<?xml version = '1.0' encoding = 'utf-8'?>"
        "<ECSchema schemaName='OpenPlant_3D' nameSpacePrefix='op3d' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "  <ECSchemaReference name='OpenPlant' version='01.00' prefix='op' />"
        "  <ECEntityClass typeName='UNIT_3D' >"
        "    <BaseClass>op:UNIT</BaseClass>"
        "    <ECProperty propertyName='op3d_UNIT_prop' typeName='string' />"
        "  </ECEntityClass>"
        "</ECSchema>")));

    item = m_ecdb.Schemas().GetClass("OpenPlant", "ITEM");
    unit = m_ecdb.Schemas().GetClass("OpenPlant", "UNIT");
    ECClassCP unit_3D = m_ecdb.Schemas().GetClass("OpenPlant_3D", "UNIT_3D");

    //Insert Statements
    {
    Utf8String ecsql;
    //relationship between UNIT and ITEM
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')"));

    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401, 201, %llu, 101, %llu)", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));

    //relationship between UNIT_3D(new derived Class) and ITEM
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op3d.UNIT_3D(ECInstanceId, op_UNIT_prop, op3d_UNIT_prop) VALUES(301, 'unitString2', 'unit3dString2')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(102, 'itemString2')"));

    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(402, 301, %llu, 102, %llu)", unit_3D->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }

    //Select Statements
    {
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(m_ecdb, "SELECT * FROM op.UNIT_HAS_ITEM"));

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));

    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));

    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE ECInstanceId = 402 AND SourceECClassId=%s", unit_3D->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }
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
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')"));

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, relProp) VALUES(401, 201, %llu, 101, %llu, 'relPropString1')", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }

    //Select statements
    {
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(m_ecdb, "SELECT * FROM op.UNIT_HAS_ITEM"));

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu AND TargetECClassId = %llu", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }

    //update Statement
    {
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "UPDATE op.UNIT_HAS_ITEM SET relProp='relPropUpdatedString1' WHERE ECInstanceId=401"));
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %s AND TargetECClassId = %s", unit->GetId().ToString().c_str(),
                  item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    //verify Deltion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %s AND TargetECClassId = %s", unit->GetId().ToString().c_str(), item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }
    sp.Cancel();

    m_ecdb.SaveChanges();
    ASSERT_EQ(SUCCESS, ImportSchema(m_ecdb, SchemaItem(
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

    item = m_ecdb.Schemas().GetClass("OpenPlant", "ITEM");
    unit = m_ecdb.Schemas().GetClass("OpenPlant", "UNIT");
    ECClassCP item_3D = m_ecdb.Schemas().GetClass("OpenPlant_3D", "ITEM_3D");
    ECClassCP unit_3D = m_ecdb.Schemas().GetClass("OpenPlant_3D", "UNIT_3D");

    //Insert Statements
    {
    //relationship between UNIT and ITEM
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')"));

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, relProp) VALUES(501, 201, %llu, 101, %llu, 'relPropString1')", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));

    //relationship between UNIT_3D and ITEM_3D newly added derived classes
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op3d.UNIT_3D(ECInstanceId, op_UNIT_prop, op3d_UNIT_prop) VALUES(401, 'unitString2', 'unit3dString2')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "INSERT INTO op3d.ITEM_3D(ECInstanceId, op_ITEM_prop, op3d_ITEM_prop) VALUES(301, 'itemString2', 'item3dString2')"));

    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, relProp) VALUES(502, 401, %llu, 301, %llu, 'relPropString2')", unit_3D->GetId().GetValue(), item_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }

    //Select statements
    {
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(m_ecdb, "SELECT * FROM op.UNIT_HAS_ITEM"));

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu AND TargetECClassId = %llu", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));

    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu AND TargetECClassId = %llu", unit_3D->GetId().GetValue(), item_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }

    //update Statement
    {
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "UPDATE op.UNIT_HAS_ITEM SET relProp='relPropUpdatedString1' WHERE ECInstanceId=501"));

    //update relationship between newly added derived classes
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, "UPDATE op.UNIT_HAS_ITEM SET relProp='relPropUpdatedString2' WHERE ECInstanceId=502"));
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %s AND TargetECClassId = %s", unit->GetId().ToString().c_str(), item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %s AND TargetECClassId = %s", unit->GetId().ToString().c_str(), item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));

    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %s AND TargetECClassId = %s", unit_3D->GetId().ToString().c_str(), item_3D->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %s AND TargetECClassId = %s", unit->GetId().ToString().c_str(), item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(m_ecdb, ecsql.c_str()));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                  02/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, StrengthDirectionValidityOnEndTableRelationship)
    {
    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
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

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem(
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

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem(
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

    ASSERT_EQ(ERROR, CreateECDbAndImportSchema(SchemaItem(
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

    ASSERT_EQ(SUCCESS, CreateECDbAndImportSchema(SchemaItem(
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
        ASSERT_EQ(SUCCESS, SetupECDb("diegorelationshiptest.ecdb", SchemaItem({
            R"xml(
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
            </ECSchema>)xml",
            R"xml(
            <ECSchema schemaName="DiegoSchema2" alias="ds2" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="DiegoSchema1" version="01.00" alias="ds1"/>
                <ECEntityClass typeName="GeometricModel">
                    <BaseClass>ds1:DataSetModel</BaseClass>
                    <ECProperty propertyName="Name" typeName="string"/>
                 </ECEntityClass>
            </ECSchema>
            )xml"})));

        ECClassCP civilModelClass = m_ecdb.Schemas().GetClass("DiegoSchema1", "CivilModel");
        ASSERT_TRUE(civilModelClass != nullptr);
        ECClassCP datasetModelClass = m_ecdb.Schemas().GetClass("DiegoSchema1", "DataSetModel");
        ASSERT_TRUE(datasetModelClass != nullptr);
        ECClassCP relClass = m_ecdb.Schemas().GetClass("DiegoSchema1", "CivilModelHasDataSetModel");
        ASSERT_TRUE(relClass != nullptr);
        ECClassCP geometricModelClass = m_ecdb.Schemas().GetClass("DiegoSchema2", "GeometricModel");
        ASSERT_TRUE(geometricModelClass != nullptr);

        IECInstancePtr civilModel1 = ECDbTestUtility::CreateArbitraryECInstance(*civilModelClass);
        IECInstancePtr civilModel2 = ECDbTestUtility::CreateArbitraryECInstance(*civilModelClass);
        IECInstancePtr geometricModel = ECDbTestUtility::CreateArbitraryECInstance(*geometricModelClass);

        StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*(relClass->GetRelationshipClassCP()));
        StandaloneECRelationshipInstancePtr rel1 = relationshipEnabler->CreateRelationshipInstance();

        rel1->SetSource(civilModel2.get());
        rel1->SetTarget(geometricModel.get());

        ECInstanceInserter civilModelInserter(m_ecdb, *civilModelClass, nullptr);
        ASSERT_TRUE(civilModelInserter.IsValid());
        ASSERT_EQ(BE_SQLITE_OK, civilModelInserter.Insert(*civilModel1));
        ASSERT_EQ(BE_SQLITE_OK, civilModelInserter.Insert(*civilModel2));

        ECInstanceInserter geometricModelInserter(m_ecdb, *geometricModelClass, nullptr);
        ASSERT_TRUE(geometricModelInserter.IsValid());
        ASSERT_EQ(BE_SQLITE_OK, geometricModelInserter.Insert(*geometricModel));

        ECInstanceInserter relInserter(m_ecdb, *relClass, nullptr);
        ASSERT_TRUE(relInserter.IsValid());
        ASSERT_EQ(BE_SQLITE_OK, relInserter.Insert(*rel1));
        }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Zaighum                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
struct ECDbHoldingRelationshipStrengthTestFixture : DbMappingTestFixture
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GeometryHoldsParts(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey1.GetClassId()));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.PartHeldByGeometry(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey1.GetClassId()));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GeometryHoldsParts(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey1.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey1.GetClassId()));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.PartIsHeldByGeometry(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey1.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey2.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey1.GetClassId()));
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
struct RelationshipsAndSharedTablesTestFixture : DbMappingTestFixture
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

    ASSERT_TRUE(m_ecdb.TableExists("t_BaseOwnsBase"));
    ASSERT_FALSE(m_ecdb.TableExists("t_BaseHasClassA"));
    ASSERT_FALSE(m_ecdb.TableExists("t_BaseHasClassB"));

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
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(6, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(4, stmt.GetValueInt(0));
    stmt.Finalize();

    //Deletes the instances of BaseOwnsBase class..
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ONLY t.BaseOwnsBase"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(4, stmt.GetValueInt(0));
    stmt.Finalize();

    //Deletes the instances of BaseHasClassA class..
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ONLY t.BaseHasClassA"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(2, stmt.GetValueInt(0));
    stmt.Finalize();

    //Deletes the instances of BaseHasClassB class..
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "DELETE FROM ONLY t.BaseHasClassB"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0));
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipsAndSharedTablesTestFixture, RetrieveConstraintClassInstanceBeforeAfterInsertingRelationshipInstance)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("RelationshipsAndTPH.ecdb", SchemaItem(SCHEMA_XML)));

    ASSERT_TRUE(m_ecdb.TableExists("t_BaseOwnsBase"));
    ASSERT_FALSE(m_ecdb.TableExists("t_BaseHasClassA"));
    ASSERT_FALSE(m_ecdb.TableExists("t_BaseHasClassB"));

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

//=======================================================================================    
// @bsiclass                                   Muhammad Hassan                     05/15
//=======================================================================================    
struct ReferentialIntegrityTestFixture : DbMappingTestFixture
    {
    private:
        void VerifyRelationshipInsertionIntegrity(ECDbCR ecdb, Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<DbResult> const& expected, size_t& rowInserted) const;
        size_t GetRelationshipInstanceCount(ECDbCR ecdb, Utf8CP relationshipClass) const;

    protected:
        void ExecuteRelationshipInsertionIntegrityTest(ECDbCR ecdb, bool allowDuplicateRelationships, bool allowForeignKeyConstraint, bool schemaImportExpectedToSucceed) const;
    };

//--------------------------------------------------------------------------------------
// @bsimethod                              Muhammad Hassan                         04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ReferentialIntegrityTestFixture, ForeignKeyConstraint_EnforceReferentialIntegrity)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ForeignKeyConstraint_EnforceReferentialIntegrity.ecdb"));
    ExecuteRelationshipInsertionIntegrityTest(m_ecdb, false, true, true);
    //when AllowDuplicate is turned of, OneFooHasManyGoo will also be mapped as endtable therefore ReferentialIntegrityCheck will be performed for it, so there will be two rows in the ForeignKey table
    ASSERT_FALSE(m_ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(m_ecdb.TableExists("ts_OneFooHasManyGoo"));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                              Muhammad Hassan                         04/15
//--------------------------------------------------------------------------------------
TEST_F(ReferentialIntegrityTestFixture, ForeignKeyConstraint_EnforceReferentialIntegrityCheck_AllowDuplicateRelation)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ForeignKeyConstraint_EnforceReferentialIntegrityCheck_AllowDuplicateRelation.ecdb"));
    ExecuteRelationshipInsertionIntegrityTest(m_ecdb, true, true, true);
    //when AllowDuplicate is turned on, OneFooHasManyGoo will also be mapped as endtable therefore there will be only one row in the ForeignKey table
    ASSERT_FALSE(m_ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(m_ecdb.TableExists("ts_OneFooHasManyGoo"));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/15
//--------------------------------------------------------------------------------------
TEST_F(ReferentialIntegrityTestFixture, DoNotAllowDuplicateRelationships)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("RelationshipCardinalityTest.ecdb"));
    ExecuteRelationshipInsertionIntegrityTest(m_ecdb, false, true, true);
    ASSERT_TRUE(m_ecdb.TableExists("ts_Foo"));
    ASSERT_TRUE(m_ecdb.TableExists("ts_Goo"));
    ASSERT_FALSE(m_ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(m_ecdb.TableExists("ts_OneFooHasManyGoo"));
    ASSERT_TRUE(m_ecdb.TableExists("ts_ManyFooHasManyGoo"));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/15
//--------------------------------------------------------------------------------------
TEST_F(ReferentialIntegrityTestFixture, AllowDuplicateRelationships)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("RelationshipCardinalityTest_AllowDuplicateRelationships.ecdb"));
    ExecuteRelationshipInsertionIntegrityTest(m_ecdb, true, true, true);
    ASSERT_TRUE(m_ecdb.TableExists("ts_Foo"));
    ASSERT_TRUE(m_ecdb.TableExists("ts_Goo"));
    ASSERT_FALSE(m_ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(m_ecdb.TableExists("ts_OneFooHasManyGoo"));
    ASSERT_TRUE(m_ecdb.TableExists("ts_ManyFooHasManyGoo"));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/15
//--------------------------------------------------------------------------------------
void ReferentialIntegrityTestFixture::VerifyRelationshipInsertionIntegrity(ECDbCR ecdb, Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<DbResult> const& expected, size_t& rowInserted) const
    {
    ECSqlStatement stmt;
    auto sql = SqlPrintfString("INSERT INTO %s (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(:sECInstanceId,:sECClassId,:tECInstanceId,:tECClassId)", relationshipClass);
    ASSERT_EQ(stmt.Prepare(ecdb, sql.GetUtf8CP()), ECSqlStatus::Success);
    ASSERT_EQ(expected.size(), sourceKeys.size() * targetKeys.size());

    const int sECInstanceId = stmt.GetParameterIndex("sECInstanceId");
    const int sECClassId = stmt.GetParameterIndex("sECClassId");
    const int tECInstanceId = stmt.GetParameterIndex("tECInstanceId");
    const int tECClassId = stmt.GetParameterIndex("tECClassId");

    int n = 0;
    for (auto& fooKey : sourceKeys)
        {
        for (auto& gooKey : targetKeys)
            {
            stmt.Reset();
            ASSERT_EQ(ECSqlStatus::Success, stmt.ClearBindings());
            stmt.BindId(sECInstanceId, fooKey.GetInstanceId());
            stmt.BindId(sECClassId, fooKey.GetClassId());
            stmt.BindId(tECInstanceId, gooKey.GetInstanceId());
            stmt.BindId(tECClassId, gooKey.GetClassId());
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

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/15
//--------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/15
//--------------------------------------------------------------------------------------
void ReferentialIntegrityTestFixture::ExecuteRelationshipInsertionIntegrityTest(ECDbCR ecdb, bool allowDuplicateRelationships, bool allowForeignKeyConstraint, bool schemaImportExpectedToSucceed) const
    {
    Utf8CP schemaTemplate = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="Testschema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
    <ECEntityClass typeName="Foo">
        <ECProperty propertyName="fooProp" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Goo" >
        <ECProperty propertyName="gooProp" typeName="string" />
        <ECNavigationProperty propertyName="PartnerFoo" relationshipName="OneFooHasOneGoo" direction="Backward">
          <ECCustomAttributes>
            %s
          </ECCustomAttributes>
        </ECNavigationProperty>                
        <ECNavigationProperty propertyName="ParentFoo" relationshipName="OneFooHasManyGoo" direction="Backward">
          <ECCustomAttributes>
            %s
          </ECCustomAttributes>
        </ECNavigationProperty>                
    </ECEntityClass>
    <ECRelationshipClass typeName="OneFooHasOneGoo" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
            <Class class="Foo" />
        </Source>
        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="has">
            <Class class="Goo" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="OneFooHasManyGoo" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
            <Class class="Foo" />
        </Source>
        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
            <Class class="Goo" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="ManyFooHasManyGoo" strength="referencing" modifier="Sealed">
        <ECCustomAttributes>
            <LinkTableRelationshipMap xmlns="ECDbMap.02.00">
            %s
            </LinkTableRelationshipMap>
        </ECCustomAttributes>
        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="has">
            <Class class="Foo" />
        </Source>
        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="has">
            <Class class="Goo" />
        </Target>
    </ECRelationshipClass>
    </ECSchema>)xml";

    Utf8CP linkTableCAStr = "";
    Utf8CP fkCAStr = "";
    if (allowDuplicateRelationships)
        linkTableCAStr = R"xml(<AllowDuplicateRelationships>true</AllowDuplicateRelationships>)xml";

    if (allowForeignKeyConstraint)
        fkCAStr = R"xml(<ForeignKeyConstraint xmlns="ECDbMap.02.00"/>)xml";

    Utf8String testSchemaXml;
    testSchemaXml.Sprintf(schemaTemplate, fkCAStr, fkCAStr, linkTableCAStr);

    BentleyStatus expectedStat = schemaImportExpectedToSucceed ? SUCCESS : ERROR;
    ASSERT_EQ(expectedStat, ImportSchema(ecdb, SchemaItem(testSchemaXml.c_str())));
    if (!schemaImportExpectedToSucceed)
        return;

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;

    ECSqlStatement fooStmt;
    ASSERT_EQ(fooStmt.Prepare(ecdb, "INSERT INTO ts.Foo(fooProp) VALUES(?)"), ECSqlStatus::Success);
    for (int i = 0; i < maxFooInstances; i++)
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
    for (int i = 0; i < maxGooInstances; i++)
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
    for (int f = 0; f < maxFooInstances; f++)
        {
        for (int g = 0; g < maxGooInstances; g++)
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
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClassId("TestSchema","OneFooHasOneGoo")));
    ASSERT_EQ((int) MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, (int) mapStrategy.m_strategy);
    ASSERT_EQ(count_OneFooHasOneGoo, GetRelationshipInstanceCount(ecdb, "ts.OneFooHasOneGoo"));

    //1:N--------------------------------
    size_t count_OneFooHasManyGoo = 0;
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.OneFooHasManyGoo", fooKeys, gooKeys, oneFooHasManyGooResult, count_OneFooHasManyGoo);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClassId("TestSchema", "OneFooHasManyGoo")));
    ASSERT_EQ((int) MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, (int) mapStrategy.m_strategy);
    ASSERT_EQ(count_OneFooHasManyGoo, GetRelationshipInstanceCount(ecdb, "ts.OneFooHasManyGoo"));

    //N:N--------------------------------
    size_t count_ManyFooHasManyGoo = 0;
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, manyFooHasManyGooResult, count_ManyFooHasManyGoo);
    if (allowDuplicateRelationships)
        VerifyRelationshipInsertionIntegrity(ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, reinsertResultDone, count_ManyFooHasManyGoo);
    else
        VerifyRelationshipInsertionIntegrity(ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, reinsertResultError, count_ManyFooHasManyGoo);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, ecdb.Schemas().GetClassId("TestSchema", "ManyFooHasManyGoo")));

    ASSERT_EQ((int) MapStrategyInfo::Strategy::OwnTable, (int) mapStrategy.m_strategy);
    ASSERT_TRUE(mapStrategy.m_tphInfo.IsUnset());
    ASSERT_EQ(count_ManyFooHasManyGoo, GetRelationshipInstanceCount(ecdb, "ts.ManyFooHasManyGoo"));
    }



//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
struct RelationshipStrengthTestFixture : ECDbTestFixture
    {
    protected:
    //---------------------------------------------------------------------------------------
    //                                               Muhammad Hassan                  10/2014
    //+---------------+---------------+---------------+---------------+---------------+------
    ECInstanceKey InsertPerson(Utf8CP firstName, Utf8CP lastName)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO RelationshipStrengthTest.Person(FirstName,LastName) VALUES('%s','%s')", firstName, lastName);
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
    ECInstanceKey InsertRelationship(Utf8CP relationshipClassECSqlName, ECInstanceKey const& source, ECInstanceKey const& target)
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
    };



//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipStrengthTestFixture, BackwardEmbedding)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("BackwardRelationshipStrengthTest.ecdb", BeFileName(L"RelationshipStrengthTest.01.00.ecschema.xml")));
    /*
    *                                           SingleParent
    *                                                 |
    *                                                 | SingleParentHasChildren_backward (Backward EMBEDDING)
    *         ________________________________________|______________________________________
    *        |                                        |                                      |
    *      Child1                                   Child2                                 Child3
    */
    ECInstanceKey child1 = InsertPerson("First", "Child");
    ECInstanceKey child2 = InsertPerson("Second", "Child");
    ECInstanceKey child3 = InsertPerson("Third", "Child");
    ECInstanceKey singleParent = InsertPerson("Only", "singleParent");

    //Backward Embedding relationship (SingleParent -> Child1, Child2)
    ECInstanceKey child1HasSingleParent = InsertRelationship("RelationshipStrengthTest.SingleParentHasChildren_backward", child1, singleParent);
    ECInstanceKey child2HasSingleParent = InsertRelationship("RelationshipStrengthTest.SingleParentHasChildren_backward", child2, singleParent);
    ECInstanceKey child3HasSingleParent = InsertRelationship("RelationshipStrengthTest.SingleParentHasChildren_backward", child3, singleParent);

    /*
    * Test 1: Delete Child1
    * Validate child1HasSingleParent,  child1 have been deleted
    * Validate singleParent, child2HasSingleParent, child3HasSingleParent, child2, child3 are still there
    */
    DeleteInstance(child1);
    m_ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(child1));
    ASSERT_FALSE(HasInstance(child1HasSingleParent));

    ASSERT_TRUE(HasInstance(singleParent));
    ASSERT_TRUE(HasInstance(child2HasSingleParent));
    ASSERT_TRUE(HasInstance(child3HasSingleParent));
    ASSERT_TRUE(HasInstance(child2));
    ASSERT_TRUE(HasInstance(child3));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelationshipStrengthTestFixture, RelationshipTest)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("RelationshipStrengthTest.ecdb", BeFileName(L"RelationshipStrengthTest.01.00.ecschema.xml")));

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
    ECInstanceKey grandParent2 = InsertPerson("Second", "GrandParent");
    ECInstanceKey singleParent = InsertPerson("Only", "SingleParent");
    ECInstanceKey child1 = InsertPerson("First", "Child");
    ECInstanceKey child2 = InsertPerson("Second", "Child");

    // Referencing relationship (GrandParent1, GrandParent2 -> SingleParent)
    ECInstanceKey grandParent1HasSingleParent = InsertRelationship("RelationshipStrengthTest.ManyParentsHaveChildren", grandParent1, singleParent);
    ECInstanceKey grandParent2HasSingleParent = InsertRelationship("RelationshipStrengthTest.ManyParentsHaveChildren", grandParent2, singleParent);

    // Embedding relationship (SingleParent -> Child1, Child2)
    ECInstanceKey singleParentHasChild1 = InsertRelationship("RelationshipStrengthTest.SingleParentHasChildren", singleParent, child1);
    ECInstanceKey singleParentHasChild2 = InsertRelationship("RelationshipStrengthTest.SingleParentHasChildren", singleParent, child2);

    // Referencing relationship (GrandParent1 <-> GrandParent2)
    ECInstanceKey grandParent1HasSpouse = InsertRelationship("RelationshipStrengthTest.ParentHasSpouse", grandParent1, grandParent2);
    ECInstanceKey grandParent2HasSpouse = InsertRelationship("RelationshipStrengthTest.ParentHasSpouse", grandParent2, grandParent1);

    m_ecdb.SaveChanges();

    //Verify instances before deletion
    ASSERT_TRUE(HasInstance(grandParent1HasSpouse));
    ASSERT_TRUE(HasInstance(grandParent2HasSpouse));

    /*
    * Test 1: Delete GrandParent1
    * Validate grandParent1HasSpouse, grandParent2HasSpouse, grandParent1HasSingleParent have been deleted (orphaned relationships)
    * Validate singleParent is still around (referencing relationship with one parent remaining)
    */
    DeleteInstance(grandParent1);
    m_ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(grandParent1));
    ASSERT_FALSE(HasInstance(grandParent1HasSpouse));
    ASSERT_FALSE(HasInstance(grandParent2HasSpouse));
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
    ASSERT_TRUE(HasInstance(singleParentHasChild1));
    ASSERT_TRUE(HasInstance(singleParentHasChild2));
    ASSERT_TRUE(HasInstance(child1));
    ASSERT_TRUE(HasInstance(child2));
    }

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipStrengthTestFixture, BackwardHoldingForwardEmbedding)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("BackwardRelationshipStrengthTest.ecdb", BeFileName(L"RelationshipStrengthTest.01.00.ecschema.xml")));

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
    ECInstanceKey child1 = InsertPerson("First", "Child");
    ECInstanceKey child2 = InsertPerson("Second", "Child");
    ECInstanceKey singleParent = InsertPerson("Only", "singleParent");
    ECInstanceKey grandParent1 = InsertPerson("First", "GrandParent");
    ECInstanceKey grandParent2 = InsertPerson("Second", "GrandParent");

    // Backward referencing relationship (GrandParent1, GrandParent2 <- SingleParent)
    ECInstanceKey singleParentHasGrandParent1 = InsertRelationship("RelationshipStrengthTest.ChildrenHaveManyParents", singleParent, grandParent1);
    ECInstanceKey singleParentHasGrandParent2 = InsertRelationship("RelationshipStrengthTest.ChildrenHaveManyParents", singleParent, grandParent2);

    //Forward Embedding relationship (SingleParent -> Child1, Child2)
    ECInstanceKey singleParentHasChild1 = InsertRelationship("RelationshipStrengthTest.SingleParentHasChildren", singleParent, child1);
    ECInstanceKey singleParentHasChild2 = InsertRelationship("RelationshipStrengthTest.SingleParentHasChildren", singleParent, child2);

    //Backward Referencing relationship (GrandParent1 <-> GrandParent2)
    ECInstanceKey grandParent1HasSpouse = InsertRelationship("RelationshipStrengthTest.ParentHasSpouse_backward", grandParent1, grandParent2);
    ECInstanceKey grandParent2HasSpouse = InsertRelationship("RelationshipStrengthTest.ParentHasSpouse_backward", grandParent2, grandParent1);

    m_ecdb.SaveChanges();

    //Validate Instance exists before deletion
    ASSERT_TRUE(HasInstance(singleParentHasChild1));

    /*
    * Test 1: Delete Child1
    * Validate Child1 and singleParentHasChild1 have been deleted
    * Validate Child2 is Still there
    */
    DeleteInstance(child1);
    m_ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(child1));
    ASSERT_FALSE(HasInstance(singleParentHasChild1));

    ASSERT_TRUE(HasInstance(child2));

    /*
    * Test 2: Delete Child2
    * Validate Child2 and singleParentHasChild2 have been deleted
    * Validate singleParent is still around (relationship grand parents remaining)
    */
    DeleteInstance(child2);
    m_ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(child2));
    ASSERT_FALSE(HasInstance(singleParentHasChild2));

    ASSERT_TRUE(HasInstance(singleParent));

    /*
    * Test 3: Delete GrandParent1
    * Validate GrandParent1, grandParent1HasSpouse, grandParent2HasSpouse, singleParentHasGrandParent1 have been deleted
    * Validate singleParent is still around (referencing relationship with one parent remaining)
    */
    DeleteInstance(grandParent1);
    m_ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(grandParent1));
    ASSERT_FALSE(HasInstance(grandParent1HasSpouse));
    ASSERT_FALSE(HasInstance(grandParent2HasSpouse));
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
