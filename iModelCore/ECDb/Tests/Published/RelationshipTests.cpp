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
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem(
            "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
            "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
            "  <ECEntityClass typeName='Model' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECEntityClass>"
            "  <ECEntityClass typeName='Element' >"
            "    <ECProperty propertyName='Code' typeName='string' />"
            "  </ECEntityClass>"
            "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
            "    <ECCustomAttributes>"
            "        <LinkTableRelationshipMap xmlns='ECDbMap.02.00'/>"
            "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
            "    </ECCustomAttributes>"
            "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
            "      <Class class='Model' />"
            "    </Source>"
            "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
            "      <Class class='Element' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", false, "RelationshipClass has the violating custom attributes 'ForeignKeyConstraint' and 'LinkTableRelationshipMap' "));

    testSchemas.push_back(
        SchemaItem(
            "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
            "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
            "  <ECEntityClass typeName='Model' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECEntityClass>"
            "  <ECEntityClass typeName='Element' >"
            "    <ECProperty propertyName='Code' typeName='string' />"
            "  </ECEntityClass>"
            "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
            "    <ECCustomAttributes>"
            "        <LinkTableRelationshipMap xmlns='ECDbMap.02.00'/>"
            "        <UseECInstanceIdAsForeignKey xmlns = 'ECDbMap.02.00'/>"
            "    </ECCustomAttributes>"
            "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
            "      <Class class='Model' />"
            "    </Source>"
            "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
            "      <Class class='Element' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", false, "RelationshipClass has the violating custom attributes 'UseECInstanceIdAsForeignKey' and 'LinkTableRelationshipMap' "));

    testSchemas.push_back(
        SchemaItem(
            "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
            "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
            "  <ECEntityClass typeName='Model' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECEntityClass>"
            "  <ECEntityClass typeName='Element' >"
            "    <ECProperty propertyName='Code' typeName='string' />"
            "  </ECEntityClass>"
            "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
            "    <ECCustomAttributes>"
            "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
            "    </ECCustomAttributes>"
            "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements'>"
            "      <Class class='Model' />"
            "    </Source>"
            "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
            "      <Class class='Element' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", false, "Relationship mapping failed as it has a ForeignKey constraint CA but implies a link table mapping because of its cardinality."));

    testSchemas.push_back(
        SchemaItem(
            "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
            "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
            "  <ECEntityClass typeName='Model' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECEntityClass>"
            "  <ECEntityClass typeName='Element' >"
            "    <ECProperty propertyName='Code' typeName='string' />"
            "  </ECEntityClass>"
            "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
            "    <ECCustomAttributes>"
            "        <UseECInstanceIdAsForeignKey xmlns = 'ECDbMap.02.00'/>"
            "    </ECCustomAttributes>"
            "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements'>"
            "      <Class class='Model' />"
            "    </Source>"
            "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
            "      <Class class='Element' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", false, "Relationship mapping failed as it has a UseECInstanceIdAsForeignKey CA but implies a link table mapping because of its cardinality."));

    testSchemas.push_back(
        SchemaItem(
            "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
            "  <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
            "  <ECEntityClass typeName='Model' >"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECEntityClass>"
            "  <ECEntityClass typeName='Element' >"
            "    <ECProperty propertyName='Code' typeName='string' />"
            "  </ECEntityClass>"
            "  <ECRelationshipClass typeName='ModelHasElements' modifier='Abstract' strength='embedding'>"
            "    <ECProperty propertyName='RelProp' typeName='string' />"
            "    <ECCustomAttributes>"
            "        <UseECInstanceIdAsForeignKey xmlns = 'ECDbMap.02.00'/>"
            "    </ECCustomAttributes>"
            "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
            "      <Class class='Model' />"
            "    </Source>"
            "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
            "      <Class class='Element' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", false, "Mapping failed because the RelationshipClass has UseECInstanceIdAsForeignKey CA and also defines a property."));

    testSchemas.push_back(
        SchemaItem(
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
            "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
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
            "</ECSchema>", false, "ForeignKey mapping can only have a CA when the mapping strategy is set to NotMapped."));

    testSchemas.push_back(
        SchemaItem(
            "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
            "    <ECEntityClass typeName='A'>"
            "        <ECProperty propertyName='Price' typeName='double'/>"
            "    </ECEntityClass>"
            "    <ECEntityClass typeName='B'>"
            "        <ECProperty propertyName='Name' typeName='string'/>"
            "       <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward'/>"
            "    </ECEntityClass>"
            "    <ECRelationshipClass typeName='Rel' modifier='None' strength='referencing' >"
            "       <Source multiplicity='(1..1)' polymorphic='true' roleLabel='A'>"
            "           <Class class='A' />"
            "       </Source>"
            "       <Target multiplicity='(0..*)' polymorphic='true' roleLabel='B'>"
            "           <Class class='B' />"
            "       </Target>"
            "     </ECRelationshipClass>"
            "    <ECRelationshipClass typeName='Rel1' modifier='None' strength='referencing' >"
            "       <ECCustomAttributes>"
            "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
            "       </ECCustomAttributes>"
            "       <BaseClass>Rel</BaseClass>"
            "       <Source multiplicity='(1..1)' polymorphic='true' roleLabel='As'>"
            "           <Class class='A' />"
            "       </Source>"
            "       <Target multiplicity='(0..*)' polymorphic='true' roleLabel='Bs'>"
            "           <Class class='B' />"
            "       </Target>"
            "     </ECRelationshipClass>"
            "</ECSchema>", false, "ForeignKeyConstraint CA on child RelationshipClass is not supported. Only the root class can have it."));

    AssertSchemaImport(testSchemas, "RelationshipMappingTests.ecdb");
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, IndexCreationForRelationships)
    {
            {
            SchemaItem testItem(
                R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                    <ECEntityClass typeName="A" modifier="None" >
                        <ECProperty propertyName="AId" typeName="string" />
                        <ECNavigationProperty propertyName="PartnerB" relationshipName="Rel11Backwards" direction="Forward"/>
                    </ECEntityClass>
                    <ECEntityClass typeName="B" modifier="None">
                        <ECCustomAttributes>
                            <ClassMap xmlns="ECDbMap.02.00">
                                <MapStrategy>TablePerHierarchy</MapStrategy>
                            </ClassMap>
                        </ECCustomAttributes>
                        <ECNavigationProperty propertyName="AId" relationshipName="Rel" direction="Backward"/>
                        <ECNavigationProperty propertyName="PartnerA" relationshipName="Rel11" direction="Backward"/>
                        <ECProperty propertyName="BId" typeName="long" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BB" modifier="None">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="BBId" typeName="long" />
                    </ECEntityClass>
                   <ECRelationshipClass typeName="Rel" strength="embedding" modifier="Sealed">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="owns">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                      <Class class="B" />
                    </Target>
                  </ECRelationshipClass>
                   <ECRelationshipClass typeName="Rel11" strength="embedding" modifier="Sealed">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                    <Source multiplicity="(1..1)" polymorphic="True" roleLabel="relates">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(1..1)" polymorphic="True" roleLabel="relates">
                      <Class class="B" />
                    </Target>
                  </ECRelationshipClass>
                   <ECRelationshipClass typeName="Rel11Backwards" strength="embedding" strengthDirection="Backward" modifier="Sealed">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
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
                </ECSchema>)xml", true, "");

            ECDb ecdb;
            bool asserted = false;
            AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships1.ecdb");
            ASSERT_FALSE(asserted);

            AssertIndex(ecdb, "ix_ts1_B_fk_ts1_Rel_target", false, "ts1_B", {"AId"});
            AssertIndex(ecdb, "uix_ts1_B_fk_ts1_Rel11_target", true, "ts1_B", {"PartnerAId"});
            AssertIndex(ecdb, "uix_ts1_A_fk_ts1_Rel11Backwards_source", true, "ts1_A", {"PartnerBId"});

            AssertIndex(ecdb, "ix_ts1_RelNN_source", false, "ts1_RelNN", {"SourceId"});
            AssertIndex(ecdb, "ix_ts1_RelNN_target", false, "ts1_RelNN", {"TargetId"});
            AssertIndex(ecdb, "uix_ts1_RelNN_sourcetarget", true, "ts1_RelNN", {"SourceId", "TargetId"});
            }

            {
            SchemaItem testItem(
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
                        <ECNavigationProperty propertyName="A" relationshipName="Rel" direction="Backward"/>
                        <ECProperty propertyName="BId" typeName="long" />
                    </ECEntityClass>
                    <ECEntityClass typeName="BB" modifier="None">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="BBId" typeName="long" />
                    </ECEntityClass>
                   <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="embedding">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is owned by">
                      <Class class="B" />
                    </Target>
                  </ECRelationshipClass>
                </ECSchema>)xml", true, "");

            ECDb ecdb;
            bool asserted = false;
            AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships2.ecdb");
            ASSERT_FALSE(asserted);
            AssertIndex(ecdb, "ix_ts2_B_fk_ts2_Rel_target", false, "ts2_B", {"AId"}, "([AId] IS NOT NULL)");
            AssertPropertyMapping(ecdb, PropertyAccessString("TestSchema", "B", "A"),
                                            {{"A.Id", ColumnInfo("ts2_b","AId")},
                                            {"A.RelECClassId", ColumnInfo("ts2_b","ARelECClassId", true)}});
            }

            {
            SchemaItem testItem(
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
                "        <ECNavigationProperty propertyName='AId' relationshipName='Rel' direction='Backward' />"
                "    </ECEntityClass>"
                "   <ECRelationshipClass typeName='Rel' modifier='Sealed' strength='embedding'>"
                "        <ECCustomAttributes>"
                "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                "        </ECCustomAttributes>"
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
                "        <ECProperty propertyName='Code' typeName='string' />"
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
                "        <ECCustomAttributes>"
                "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                "        </ECCustomAttributes>"
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
                        <ECNavigationProperty propertyName="A" relationshipName="RelBase" direction="Backward" />
                    </ECEntityClass>
                    <ECEntityClass typeName="B1" modifier="None">
                        <BaseClass>B</BaseClass>
                        <ECProperty propertyName="B1Id" typeName="long" />
                    </ECEntityClass>
                   <ECRelationshipClass typeName="RelBase" modifier="Abstract" strength="referencing">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
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
                </ECSchema>)xml", true, "");

            ECDb ecdb;
            bool asserted = false;
            AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships50.ecdb");
            ASSERT_FALSE(asserted);

            ASSERT_EQ(3, (int) RetrieveIndicesForTable(ecdb, "ts50_B").size()) << "Expected indices: class id index, user defined index; no indexes for the relationship constraints";

            AssertIndex(ecdb, "ix_ts50_B_fk_ts50_RelBase_target", false, "ts50_B", {"AId"}, "([AId] IS NOT NULL)");
            AssertIndex(ecdb, "ix_ts50_B_ARelECClassId", false, "ts50_B", {"ARelECClassId"}, "([ARelECClassId] IS NOT NULL)");
            AssertIndexExists(ecdb, "uix_ts50_B_fk_ts50_RelSub1_target", false);
            }

            {
            SchemaItem testItem(
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
                "        <ECNavigationProperty propertyName='AId' relationshipName='RelBase' direction='Backward' />"
                "    </ECEntityClass>"
                "    <ECEntityClass typeName='B1' modifier='None'>"
                "        <BaseClass>B</BaseClass>"
                "        <ECProperty propertyName='B1Id' typeName='long' />"
                "    </ECEntityClass>"
                "   <ECRelationshipClass typeName='RelBase' modifier='Abstract' strength='referencing'>"
                "        <ECCustomAttributes>"
                "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                "        </ECCustomAttributes>"
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
                "        <ECProperty propertyName='Code' typeName='string' />"
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
                "        <ECCustomAttributes>"
                "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                "        </ECCustomAttributes>"
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
            AssertIndex(ecdb, "ix_ts6_B_fk_ts6_RelBase_target", false, "ts6_B", {"AInstanceId"});
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
                "</ECSchema>", true, "");

            ECDb ecdb;
            bool asserted = false;
            AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships7.ecdb");
            ASSERT_FALSE(asserted);

            ASSERT_EQ(9, (int) RetrieveIndicesForTable(ecdb, "ts7_RelBase").size());
            }

            {
            SchemaItem testItem(
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
                        <ECNavigationProperty propertyName="A1" relationshipName="RelPoly" direction="Backward" />
                        <ECNavigationProperty propertyName="A2" relationshipName="RelNonPoly" direction="Backward" />
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
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(0..1)" polymorphic="False" roleLabel="references">
                      <Class class="B1" />
                    </Target>
                  </ECRelationshipClass>
                   <ECRelationshipClass typeName="RelPoly" modifier="Sealed" strength="referencing">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                    <Source multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                      <Class class="A" />
                    </Source>
                    <Target multiplicity="(0..1)" polymorphic="True" roleLabel="references">
                      <Class class="B1" />
                    </Target>
                  </ECRelationshipClass>
                </ECSchema>)xml", true, "");

            ECDb ecdb;
            bool asserted = false;
            AssertSchemaImport(ecdb, asserted, testItem, "indexcreationforrelationships8.ecdb");
            ASSERT_FALSE(asserted);

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
            SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts9\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
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
// @bsimethod                                   Maha Nasir                     10/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, CascadeDeletion)
    {
    SchemaItem testItem("<?xml version='1.0' encoding='utf-8'?>"
                        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbmap' />"
                        "    <ECEntityClass typeName='ClassA' modifier='None'>"
                        "        <ECProperty propertyName='AA' typeName='string' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='ClassB' modifier='None'>"
                        "        <ECProperty propertyName='BB' typeName='string' />"
                        "        <ECNavigationProperty propertyName='A' relationshipName='AHasB' direction='Backward'/>"
                        "    </ECEntityClass>"
                        "    <ECRelationshipClass typeName='AHasB' modifier='Sealed' strength='embedding'>"
                        "        <ECCustomAttributes>"
                        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                        "               <OnDeleteAction>Cascade</OnDeleteAction>"
                        "            </ForeignKeyConstraint>"
                        "        </ECCustomAttributes>"
                        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                        "           <Class class='ClassA' />"
                        "       </Source>"
                        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target'>"
                        "           <Class class='ClassB' />"
                        "       </Target>"
                        "     </ECRelationshipClass>"
                        "    <ECEntityClass typeName='ClassC' modifier='None'>"
                        "        <ECProperty propertyName='CC' typeName='string' />"
                        "        <ECNavigationProperty propertyName='B' relationshipName='BHasC' direction='Backward'/>"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='BHasC' modifier='Sealed' strength='embedding'>"
                        "        <ECCustomAttributes>"
                        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                        "        </ECCustomAttributes>"
                        "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Source'>"
                        "      <Class class = 'ClassB' />"
                        "    </Source>"
                        "    <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Target'>"
                        "      <Class class = 'ClassC' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "</ECSchema>", true, "");

    ECDb db;
    bool asserted = false;
    AssertSchemaImport(db, asserted, testItem, "CascadeDeletion.ecdb");
    ASSERT_FALSE(asserted);

    ECSchemaCP schema = db.Schemas().GetSchema("TestSchema");
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
    ECInstanceInserter ClassA_Inserter(db, *ClassA, nullptr);
    ASSERT_TRUE(ClassA_Inserter.IsValid());
    ClassA_Inserter.Insert(*ClassA_Instance);

    //Inserter of ClassB
    ECInstanceInserter ClassB_Inserter(db, *ClassB, nullptr);
    ASSERT_TRUE(ClassB_Inserter.IsValid());
    ClassB_Inserter.Insert(*ClassB_Instance);

    ECRelationshipClassCP AHasB = db.Schemas().GetClass("TestSchema", "AHasB")->GetRelationshipClassCP();
    ECRelationshipClassCP BHasC = db.Schemas().GetClass("TestSchema", "BHasC")->GetRelationshipClassCP();

    //Inserting relationship instance.
    ECN::StandaloneECRelationshipInstancePtr ClassAHasB_relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*AHasB)->CreateRelationshipInstance();
    ClassAHasB_relationshipInstance->SetSource(ClassA_Instance.get());
    ClassAHasB_relationshipInstance->SetTarget(ClassB_Instance.get());

    ECInstanceInserter AHasB_relationshipInserter(db, *AHasB, nullptr);
    AHasB_relationshipInserter.Insert(*ClassAHasB_relationshipInstance);


    //Inserting instances of ClassC
    StandaloneECInstancePtr ClassC_Instance = ClassC->GetDefaultStandaloneEnabler()->CreateInstance();
    ClassC_Instance->SetValue("CC", ECValue("val5"));

    //Inserter of ClassC
    ECInstanceInserter ClassC_Inserter(db, *ClassC, nullptr);
    ASSERT_TRUE(ClassC_Inserter.IsValid());
    ClassC_Inserter.Insert(*ClassC_Instance);

    //Inserting relationship instances.
    ECN::StandaloneECRelationshipInstancePtr ClassBHasC_relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*BHasC)->CreateRelationshipInstance();
    ClassBHasC_relationshipInstance->SetSource(ClassB_Instance.get());
    ClassBHasC_relationshipInstance->SetTarget(ClassC_Instance.get());

    ECInstanceInserter BHasC_relationshipInserter(db, *BHasC, nullptr);
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
// @bsimethod                                   Krischan.Eberle                     03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MultipleFkEndTables)
    {
    ECDbCR ecdb = SetupECDb("multiplefkendtables.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                            <ECEntityClass typeName="Parent">
                              <ECProperty propertyName="Code" typeName="int" />
                            </ECEntityClass>
                            <ECEntityClass typeName="Child">
                              <ECProperty propertyName="Name" typeName="string" />
                              <ECNavigationProperty propertyName="MyParent" relationshipName="Rel" direction="Backward"/>
                            </ECEntityClass>
                            <ECEntityClass typeName="SpecialChild">
                              <BaseClass>Child</BaseClass>
                              <ECProperty propertyName="SpecialName" typeName="string" />
                            </ECEntityClass>
                            <ECRelationshipClass typeName="Rel" strength="Referencing" modifier="Sealed">
                                <ECCustomAttributes>
                                    <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                                </ECCustomAttributes>
                                <Source multiplicity="(0..1)" polymorphic="False" roleLabel="Parent">
                                   <Class class ="Parent" />
                                </Source>
                                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Children">
                                   <Class class ="Child" />
                                </Target>
                            </ECRelationshipClass>
                        </ECSchema>)xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECInstanceKey parentKey, childKey, specialChildKey;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Parent(Code) VALUES(1)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(parentKey)) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Child(Name) VALUES('Child1')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(childKey)) << stmt.GetECSql();
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.SpecialChild(Name,SpecialName) VALUES('Child2','I am special')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(specialChildKey)) << stmt.GetECSql();
    stmt.Finalize();
    
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(ecdb, "INSERT INTO ts.Rel(SourceECInstanceId, TargetECInstanceId) VALUES(?,?)"));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE ts.Child SET MyParent.Id=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, parentKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, ECClassId, Name FROM ts.Child ORDER BY ECInstanceId"));
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

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT SourceECInstanceId, TargetECInstanceId, TargetECClassId FROM ts.Rel ORDER BY TargetECInstanceId"));
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
    ECDbCR ecdb = SetupECDb("multipleconstraintclasses.ecdb", 
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
                        </ECSchema>)xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, LogicalForeignKeyRelationship)
    {
    SetupECDb("LogicalForeignKeyRelationship.ecdb",
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
                         "</ECSchema>"));

    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().Schemas().CreateClassViewsInDb();
    GetECDb().SaveChanges();
    ECClassId primaryClassAHasSecondaryClassAId = GetECDb().Schemas().GetClassId("TestSchema", "PrimaryClassAHasSecondaryClassA");
    ECClassId primaryClassAHasSecondaryClassBId = GetECDb().Schemas().GetClassId("TestSchema", "PrimaryClassAHasSecondaryClassB");

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(101, 10000)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(102, 20000)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(103, 30000)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(104, 40000)"));

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), Utf8PrintfString("INSERT INTO ts.SecondaryClassA(ECInstanceId, T1, PrimaryClassA.Id, PrimaryClassA.RelECClassId) VALUES(201, 10000, 101, %ld)", primaryClassAHasSecondaryClassBId.GetValue()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.SecondaryClassA(ECInstanceId, T1, PrimaryClassA.Id) VALUES(202, 20000, 102)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.SecondaryClassA(ECInstanceId, T1) VALUES(203, 30000)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.SecondaryClassA(ECInstanceId, T1) VALUES(204, 40000)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), Utf8PrintfString("UPDATE ts.SecondaryClassA SET PrimaryClassA.Id = 103, T1=300002, PrimaryClassA.RelECClassId = %ld  WHERE ECInstanceId = 203", primaryClassAHasSecondaryClassBId.GetValue()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.PrimaryClassAHasSecondaryClassB(SourceECInstanceId, TargetECInstanceId) VALUES(104, 204)"));
    GetECDb().SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, LogicalForeignKeyRelationshipMappedToSharedColumn)
    {
    SetupECDb("logicalfk_sharedcol.ecdb",
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

                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().SaveChanges();

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Car            (Name                ) VALUES ('BMW-S')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Engine         (Code, www, Volumn,Car.Id,Car.RelECClassId ) VALUES ('CODE-1','www1', 2000.0,1,53 )"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Sterring       (Code, www, Type,Car.Id,Car.RelECClassId   ) VALUES ('CODE-2','www2', 'S-Type',1,53)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Tire           (Code, Diameter      ) VALUES ('CODE-3', 15.0)"));


    GetECDb().Schemas().CreateClassViewsInDb();
    GetECDb().SaveChanges();
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.CarHasEndPoint"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(2, stmt.GetValueInt64(0));
    ASSERT_EQ(53, stmt.GetValueInt64(1));
    ASSERT_EQ(1, stmt.GetValueInt64(2));
    ASSERT_EQ(51, stmt.GetValueInt64(3));
    ASSERT_EQ(2, stmt.GetValueInt64(4));
    ASSERT_EQ(54, stmt.GetValueInt64(5));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(3, stmt.GetValueInt64(0));
    ASSERT_EQ(53, stmt.GetValueInt64(1));
    ASSERT_EQ(1, stmt.GetValueInt64(2));
    ASSERT_EQ(51, stmt.GetValueInt64(3));
    ASSERT_EQ(3, stmt.GetValueInt64(4));
    ASSERT_EQ(56, stmt.GetValueInt64(5));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT Car.Id,Car.RelECClassId FROM ts.Engine"));
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT Car.Id,Car.RelECClassId FROM ts.Sterring"));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, LogicalForeignKeyRelationshipMappedToUnsharedColumn)
    {
    SetupECDb("logicalfk_unsharedcol.ecdb",
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
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    GetECDb().Schemas().CreateClassViewsInDb();
    GetECDb().SaveChanges();

    ECClassCP elementClass = GetECDb().Schemas().GetClass("TestSchema", "Element");
    ASSERT_TRUE(elementClass != nullptr);

    AssertForeignKey(false, GetECDb(), "ts_Element", "ModelId");
    AssertIndexExists(GetECDb(), "x_ts_Element_fk_ts_Rel_target", false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MixinAsRelationshipEnd)
    {
    SetupECDb("diamond_problem3.ecdb",
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
                         "      <ECNavigationProperty propertyName='Car' relationshipName='CarHasEndPoint' direction='Backward' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='CarHasEndPoint' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                         "        <ECCustomAttributes>"
                         "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                         "        </ECCustomAttributes>"
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

                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().SaveChanges();

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Car(Name) VALUES ('BMW-S')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Engine(Code, www, Volumn,Car.Id) VALUES ('CODE-1','www1', 2000.0,1 )"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Sterring(Code, www, Type,Car.Id) VALUES ('CODE-2','www2', 'S-Type',1)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Tire(Code, Diameter) VALUES ('CODE-3', 15.0)"));


    GetECDb().Schemas().CreateClassViewsInDb();
    GetECDb().SaveChanges();
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.CarHasEndPoint"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt64(0));
    ASSERT_EQ(50, stmt.GetValueInt64(1));
    ASSERT_EQ(2, stmt.GetValueInt64(2));
    ASSERT_EQ(53, stmt.GetValueInt64(3));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(1, stmt.GetValueInt64(0));
    ASSERT_EQ(50, stmt.GetValueInt64(1));
    ASSERT_EQ(3, stmt.GetValueInt64(2));
    ASSERT_EQ(55, stmt.GetValueInt64(3));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MixinAsRelationshipEnd2)
    {
    SetupECDb("diamond_problem3.ecdb",
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
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().SaveChanges();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Car            (Name              ) VALUES ('BMW-S')")) << GetECDb().GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << ": " << GetECDb().GetLastError().c_str();
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Engine         (Code, www, Volumn ) VALUES ('CODE-1','www1', 2000.0 )")) << GetECDb().GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << ": " << GetECDb().GetLastError().c_str();
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Sterring       (Code, www, Type   ) VALUES ('CODE-2','www2', 'S-Type')")) << GetECDb().GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << ": " << GetECDb().GetLastError().c_str();
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Tire           (Code, Diameter    ) VALUES ('CODE-3', 15.0)")) << GetECDb().GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << ": " << GetECDb().GetLastError().c_str();
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.CarHasEndPoint2 (SourceECInstanceId, TargetECInstanceId, TargetECClassId, Tag, Rule) VALUES (1,2,54,'tag1','Rule1')")) << GetECDb().GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << ": " << GetECDb().GetLastError().c_str();
    stmt.Finalize();
    GetECDb().SaveChanges();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.CarHasEndPoint2 (SourceECInstanceId, TargetECInstanceId, TargetECClassId, Tag, Rule) VALUES (1,3,56,'tag2','Rule2')")) << GetECDb().GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql() << ": " << GetECDb().GetLastError().c_str();
    stmt.Finalize();
    GetECDb().Schemas().CreateClassViewsInDb();
    GetECDb().SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MixinAsRelationshipEnd3)
    {
    SetupECDb("diamond_problem3.ecdb",
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
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().Schemas().CreateClassViewsInDb();
    GetECDb().SaveChanges();

    Reopen();

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Car            (Name              ) VALUES ('BMW-S')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Engine         (Code, www, Volumn ) VALUES ('CODE-1','www1', 2000.0 )"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Sterring       (Code, www, Type   ) VALUES ('CODE-2','www2', 'S-Type')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Tire           (Code, Diameter    ) VALUES ('CODE-3', 15.0)"));

    SchemaItem item = SchemaItem("Diamond Problem2",
                                 "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
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
                                 "</ECSchema>");
    bool asserted = false;
    AssertSchemaImport(asserted, GetECDb(), item);
    GetECDb().Schemas().CreateClassViewsInDb();
    GetECDb().SaveChanges();

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.CarHasEndPoint2 (SourceECInstanceId, TargetECInstanceId, TargetECClassId, Tag, Rule) VALUES (1,2,54,'tag1','Rule1')"));
    GetECDb().SaveChanges();
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.CarHasEndPoint2 (SourceECInstanceId, TargetECInstanceId, TargetECClassId, Tag, Rule) VALUES (1,3,56,'tag2','Rule2')"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, MixinsMappedToMultipleJoinedTablesOnReferencedEnd)
    {
    ECDbR ecdb = SetupECDb("multijoinedtablemixinonreferencedend.ecdb",
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
                                          </ECSchema>)xml"));

    ASSERT_TRUE(ecdb.IsDbOpen());
    ASSERT_EQ(SUCCESS, ecdb.Schemas().CreateClassViewsInDb());
    ecdb.SaveChanges();

    std::vector<ECInstanceKey> model2dKeys, model3dKeys, element2dKeys, element3dKeys;
    //INSERT
    {
    ECInstanceKey key;
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Model2d(Name,Is2d,SupportedGeometryType,Origin2d) VALUES(?,true,'Line',?)"));
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

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Model3d(Name,Is2d,SupportedGeometryType,Origin3d) VALUES(?,false,'Solid',?)"));
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

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Element(Code, Model.Id) VALUES(?,?)"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, Code, Model.Id FROM ts.Element"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, ECClassId, Is2d, SupportedGeometryType FROM ts.IIsGeometricModel"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, Name, SupportedGeometryType, Origin2d FROM ts.Model2d"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId, Name, SupportedGeometryType, Origin3d FROM ts.Model3d"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "UPDATE ts.Element SET Model.Id=? WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, model2dKeys[1].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, element2dKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(1, ecdb.GetModifiedRowCount()) << stmt.GetECSql();
    stmt.ClearBindings();
    stmt.Reset();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, model3dKeys[1].GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, element3dKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << stmt.GetECSql();
    ASSERT_EQ(1, ecdb.GetModifiedRowCount()) << stmt.GetECSql();
    stmt.Finalize();
    }

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT e.ECInstanceId, m.ECInstanceId, m.ECClassId, m.Is2d, m.SupportedGeometryType FROM ts.Element e JOIN ts.IIsGeometricModel m USING ts.GeometricModelHasElements ORDER BY m.ECInstanceId"));
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
    ASSERT_TRUE(db.ColumnExists("ts_AHasB", "SourceId"));
    ASSERT_TRUE(db.ColumnExists("ts_AHasB", "TargetId"));
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
// @bsimethod                                   Krischan.Eberle                     05/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, FKConstraintsOnLinkTables)
    {
    ECDbCR ecdb = SetupECDb("fkconstraintsonlinktables.ecdb", SchemaItem(
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
                 </ECSchema>)xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    AssertForeignKey(true, ecdb, "ts_LinkTableWithFk1", "SourceId");
    AssertForeignKey(true, ecdb, "ts_LinkTableWithFk1", "TargetId");
    AssertForeignKey(true, ecdb, "ts_LinkTableWithFk2", "SourceId");
    AssertForeignKey(true, ecdb, "ts_LinkTableWithFk2", "TargetId");
    AssertForeignKey(true, ecdb, "ts_LinkTableWithFk3", "SourceId");
    AssertForeignKey(true, ecdb, "ts_LinkTableWithFk3", "TargetId");
    AssertForeignKey(false, ecdb, "ts_LinkTableWithoutFk");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, AmbigousRelationshipProperty)
    {
    SetupECDb("ambigousRelationshipProperty.ecdb",
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
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());
    ECInstanceKey geometryKey, geometryPartKey;
    {//INSERT Geometry
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Geometry(P1) VALUES('G1')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geometryKey));
    GetECDb().SaveChanges();
    }//===============

    {//INSERT GeometryPart
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryPart(P1) VALUES('GP1')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geometryPartKey));
    GetECDb().SaveChanges();
    }//===============

    {//INSERT GeometryHoldsParts
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryHoldsParts(SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, P1) VALUES(?,?,?,?, 'GHP1')"));
    stmt.BindId(1, geometryKey.GetInstanceId());
    stmt.BindId(2, geometryKey.GetClassId());
    stmt.BindId(3, geometryPartKey.GetInstanceId());
    stmt.BindId(4, geometryPartKey.GetClassId());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    GetECDb().SaveChanges();
    }//===============

    {//Verify
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, P1 FROM ts.GeometryHoldsParts WHERE P1 = 'GHP1'"));
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
            SchemaItem testItem("relationship classes with nav props",
                                "<?xml version='1.0' encoding='utf-8'?>"
                                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                "   <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
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
                                "    <ECCustomAttributes>"
                                "       <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
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
                                "       <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
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
                                "       <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
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
                                "       <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
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

            Utf8String ddl = RetrieveDdl(ecdb, "ts_B");
            ASSERT_FALSE(ddl.empty());

            ASSERT_TRUE(ddl.ContainsI("[AId_Rel0NId] INTEGER,")) << testItem.m_name.c_str() << " Actual DDL: " << ddl.c_str();
            ASSERT_TRUE(ddl.ContainsI("[AId_Rel1NId] INTEGER NOT NULL,")) << testItem.m_name.c_str() << " Actual DDL: " << ddl.c_str();
            ASSERT_TRUE(ddl.ContainsI("[AId_RelN0Id] INTEGER,")) << testItem.m_name.c_str() << " Actual DDL: " << ddl.c_str();
            ASSERT_TRUE(ddl.ContainsI("[AId_RelN1Id] INTEGER NOT NULL,")) << testItem.m_name.c_str() << " Actual DDL: " << ddl.c_str();
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
                                "        <ECNavigationProperty propertyName='A1' relationshipName='Rel1N' direction='Backward' />"
                                "        <ECNavigationProperty propertyName='A2' relationshipName='Rel0N' direction='Backward' />"
                                "        <ECNavigationProperty propertyName='A3' relationshipName='RelN0' direction='Forward' />"
                                "        <ECNavigationProperty propertyName='A4' relationshipName='RelN1' direction='Forward' />"
                                "    </ECEntityClass>"
                                "  <ECRelationshipClass typeName='Rel1N' strength='embedding' modifier='Sealed'>"
                                "    <ECCustomAttributes>"
                                "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                "        </ForeignKeyConstraint>"
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
                                "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                "        </ForeignKeyConstraint>"
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
                                "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                "        </ForeignKeyConstraint>"
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
                                "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                "        </ForeignKeyConstraint>"
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

            Utf8String ddl = RetrieveDdl(ecdb, "ts_B");
            ASSERT_FALSE(ddl.empty());

            ASSERT_TRUE(ddl.ContainsI("[A1Id] INTEGER NOT NULL,")) << testItem.m_name.c_str() << " Actual DDL: " << ddl.c_str();
            ASSERT_TRUE(ddl.ContainsI("[A2Id] INTEGER,")) << testItem.m_name.c_str() << " Actual DDL: " << ddl.c_str();
            ASSERT_TRUE(ddl.ContainsI("[A3Id] INTEGER,")) << testItem.m_name.c_str() << " Actual DDL: " << ddl.c_str();
            ASSERT_TRUE(ddl.ContainsI("[A4Id] INTEGER NOT NULL,")) << testItem.m_name.c_str() << " Actual DDL: " << ddl.c_str();
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
                                "    <ECCustomAttributes>"
                                "       <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                "    </ECCustomAttributes>"
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

            Utf8String ddl = RetrieveDdl(ecdb, "ts_B");
            ASSERT_FALSE(ddl.empty());

            ASSERT_TRUE(ddl.ContainsI("[AId] INTEGER,")) << testItem.m_name.c_str() << " Actual DDL: " << ddl.c_str();
            }

            {
            SchemaItem testItem("Logical FK", "<?xml version='1.0' encoding='utf-8'?>"
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
                                "</ECSchema>");

            ECDb ecdb;
            bool asserted = false;
            AssertSchemaImport(ecdb, asserted, testItem, "notnullconstraintsonfk.ecdb");
            ASSERT_FALSE(asserted);

            Utf8String ddl = RetrieveDdl(ecdb, "ts_Base");
            ASSERT_FALSE(ddl.empty());
            ASSERT_TRUE(ddl.ContainsI("[AId] INTEGER NOT NULL,")) << testItem.m_name.c_str() << " Actual DDL: " << ddl.c_str();
            }

            {
            SchemaItem testItem("Logical FK with dropped not null constraint", "<?xml version='1.0' encoding='utf-8'?>"
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

            Utf8String ddl = RetrieveDdl(ecdb, "ts_Base");
            ASSERT_FALSE(ddl.empty());
            ASSERT_TRUE(ddl.ContainsI("[AId] INTEGER,")) << testItem.m_name.c_str() << " Actual DDL: " << ddl.c_str();
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
                        "        <ECCustomAttributes>"
                        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                        "        </ECCustomAttributes>"
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
                        "        <ECCustomAttributes>"
                        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                        "        </ECCustomAttributes>"
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
    SchemaItem testItem("Two Nav props in a row", "<?xml version='1.0' encoding='utf-8'?>"
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
                        "        <ECNavigationProperty propertyName='Parent1' relationshipName='Rel1' direction='Backward' />"
                        "        <ECNavigationProperty propertyName='Parent2' relationshipName='Rel2' direction='Backward' />"
                        "    </ECEntityClass>"
                        "    <ECEntityClass typeName='C'>"
                        "        <BaseClass>Base</BaseClass>"
                        "        <ECProperty propertyName='CProp1' typeName='string' />"
                        "    </ECEntityClass>"
                        "  <ECRelationshipClass typeName='Rel1' strength='embedding' modifier='Sealed'>"
                        "        <ECCustomAttributes>"
                        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                        "        </ECCustomAttributes>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'B' />"
                        "    </Target>"
                        "  </ECRelationshipClass>"
                        "  <ECRelationshipClass typeName='Rel2' strength='embedding' modifier='Sealed'>"
                        "        <ECCustomAttributes>"
                        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                        "        </ECCustomAttributes>"
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

    AssertForeignKey(true, ecdb, "ts_Base", "Parent1Id");
    AssertForeignKey(true, ecdb, "ts_Base", "Parent2Id");
    //WIP: Column order for two nav props in a row is not correct yet. Once fixed, flip positions in the below calls.
    assertColumnPosition(ecdb, "ts_Base", "Parent1Id", 6, testItem.m_name.c_str());
    assertColumnPosition(ecdb, "ts_Base", "Parent2Id", 5, testItem.m_name.c_str());
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
                        "        <ECCustomAttributes>"
                        "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                        "        </ECCustomAttributes>"
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
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                        "        </ForeignKeyConstraint>"
                        "    </ECCustomAttributes>"
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
TEST_F(RelationshipMappingTestFixture, OneToOneRelationshipMapping)
    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem("embedding relationships", "<?xml version='1.0' encoding='utf-8'?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                                     "    <ECEntityClass typeName='A'>"
                                     "        <ECProperty propertyName='AName' typeName='string' />"
                                     "        <ECNavigationProperty propertyName='B1' relationshipName='Rel11back' direction='Forward' />"
                                     "        <ECNavigationProperty propertyName='B2' relationshipName='Rel01back' direction='Forward' />"
                                     "        <ECNavigationProperty propertyName='B3' relationshipName='Rel10back' direction='Forward' />"
                                     "        <ECNavigationProperty propertyName='B4' relationshipName='Rel00back' direction='Forward' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B'>"
                                     "        <ECProperty propertyName='BName' typeName='string' />"
                                     "        <ECNavigationProperty propertyName='A1' relationshipName='Rel11' direction='Backward' />"
                                     "        <ECNavigationProperty propertyName='A2' relationshipName='Rel01' direction='Backward' />"
                                     "        <ECNavigationProperty propertyName='A3' relationshipName='Rel10' direction='Backward' />"
                                     "        <ECNavigationProperty propertyName='A4' relationshipName='Rel00' direction='Backward' />"
                                     "    </ECEntityClass>"
                                     "  <ECRelationshipClass typeName='Rel11' strength='embedding' modifier='Sealed'>"
                                     "    <ECCustomAttributes>"
                                     "         <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "    </ECCustomAttributes>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel01' strength='embedding' modifier='Sealed'>"
                                     "    <ECCustomAttributes>"
                                     "         <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "    </ECCustomAttributes>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel10' strength='embedding' modifier='Sealed'>"
                                     "        <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "        </ECCustomAttributes>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel00' strength='embedding' modifier='Sealed'>"
                                     "        <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "        </ECCustomAttributes>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel11back' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                                     "        <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "        </ECCustomAttributes>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel01back' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                                     "        <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "        </ECCustomAttributes>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel10back' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                                     "        <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "        </ECCustomAttributes>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel00back' strength='embedding' strengthDirection='Backward' modifier='Sealed'>"
                                     "        <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "        </ECCustomAttributes>"
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
                                     "        <ECNavigationProperty propertyName='B1' relationshipName='Rel11back' direction='Forward' />"
                                     "        <ECNavigationProperty propertyName='B2' relationshipName='Rel01back' direction='Forward' />"
                                     "        <ECNavigationProperty propertyName='B3' relationshipName='Rel10back' direction='Forward' />"
                                     "        <ECNavigationProperty propertyName='B4' relationshipName='Rel00back' direction='Forward' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B'>"
                                     "        <ECProperty propertyName='BName' typeName='string' />"
                                     "        <ECNavigationProperty propertyName='A1' relationshipName='Rel11' direction='Backward' />"
                                     "        <ECNavigationProperty propertyName='A2' relationshipName='Rel01' direction='Backward' />"
                                     "        <ECNavigationProperty propertyName='A3' relationshipName='Rel10' direction='Backward' />"
                                     "        <ECNavigationProperty propertyName='A4' relationshipName='Rel00' direction='Backward' />"
                                     "    </ECEntityClass>"
                                     "  <ECRelationshipClass typeName='Rel11' strength='holding' modifier='Sealed'>"
                                     "        <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "        </ECCustomAttributes>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel01' strength='holding' modifier='Sealed'>"
                                     "        <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "        </ECCustomAttributes>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel10' strength='holding' modifier='Sealed'>"
                                     "        <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "        </ECCustomAttributes>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel00' strength='holding' modifier='Sealed'>"
                                     "        <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "        </ECCustomAttributes>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel11back' strength='holding' strengthDirection='Backward' modifier='Sealed'>"
                                     "        <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "        </ECCustomAttributes>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel01back' strength='holding' strengthDirection='Backward' modifier='Sealed'>"
                                     "        <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "        </ECCustomAttributes>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel10back' strength='holding' strengthDirection='Backward' modifier='Sealed'>"
                                     "        <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "        </ECCustomAttributes>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel00back' strength='holding' strengthDirection='Backward' modifier='Sealed'>"
                                     "        <ECCustomAttributes>"
                                     "            <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "        </ECCustomAttributes>"
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
                                     "        <ECNavigationProperty propertyName='B1' relationshipName='Rel11back' direction='Forward' />"
                                     "        <ECNavigationProperty propertyName='B2' relationshipName='Rel01back' direction='Forward' />"
                                     "        <ECNavigationProperty propertyName='B3' relationshipName='Rel10back' direction='Forward' />"
                                     "        <ECNavigationProperty propertyName='B4' relationshipName='Rel00back' direction='Forward' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B'>"
                                     "        <ECProperty propertyName='BName' typeName='string' />"
                                     "        <ECNavigationProperty propertyName='A1' relationshipName='Rel11' direction='Backward' />"
                                     "        <ECNavigationProperty propertyName='A2' relationshipName='Rel01' direction='Backward' />"
                                     "        <ECNavigationProperty propertyName='A3' relationshipName='Rel10' direction='Backward' />"
                                     "        <ECNavigationProperty propertyName='A4' relationshipName='Rel00' direction='Backward' />"
                                     "    </ECEntityClass>"
                                     "  <ECRelationshipClass typeName='Rel11' strength='referencing' modifier='Sealed'>"
                                     "    <ECCustomAttributes>"
                                     "         <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "    </ECCustomAttributes>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel01' strength='referencing' modifier='Sealed'>"
                                     "    <ECCustomAttributes>"
                                     "         <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "    </ECCustomAttributes>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel10' strength='referencing' modifier='Sealed'>"
                                     "    <ECCustomAttributes>"
                                     "         <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "    </ECCustomAttributes>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel00' strength='referencing' modifier='Sealed'>"
                                     "    <ECCustomAttributes>"
                                     "         <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "    </ECCustomAttributes>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel11back' strength='referencing' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <ECCustomAttributes>"
                                     "         <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "    </ECCustomAttributes>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel01back' strength='referencing' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <ECCustomAttributes>"
                                     "         <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "    </ECCustomAttributes>"
                                     "    <Source cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel10back' strength='referencing' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <ECCustomAttributes>"
                                     "         <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "    </ECCustomAttributes>"
                                     "    <Source cardinality='(1,1)' polymorphic='True'>"
                                     "      <Class class = 'A' />"
                                     "    </Source>"
                                     "    <Target cardinality='(0,1)' polymorphic='True'>"
                                     "      <Class class = 'B'/>"
                                     "    </Target>"
                                     "  </ECRelationshipClass>"
                                     "  <ECRelationshipClass typeName='Rel00back' strength='referencing' strengthDirection='Backward' modifier='Sealed'>"
                                     "    <ECCustomAttributes>"
                                     "         <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                                     "    </ECCustomAttributes>"
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
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
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
                                     "        <ECNavigationProperty propertyName='A2' relationshipName='Rel2' direction='Backward' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B1'>"
                                     "        <BaseClass>B</BaseClass>"
                                     "        <ECProperty propertyName='B1Name' typeName='string' />"
                                     "        <ECNavigationProperty propertyName='A3' relationshipName='Rel3' direction='Backward' />"
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
                                     "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                     "          <OnDeleteAction>Cascade</OnDeleteAction>"
                                     "        </ForeignKeyConstraint>"
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
                                     "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                     "          <OnDeleteAction>Restrict</OnDeleteAction>"
                                     "        </ForeignKeyConstraint>"
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
                                     "        <ECNavigationProperty propertyName='A' relationshipName='Rel' direction='Backward' />"
                                     "    </ECEntityClass>"
                                     "    <ECEntityClass typeName='B2'>"
                                     "        <BaseClass>B</BaseClass>"
                                     "        <ECProperty propertyName='B2Name' typeName='string' />"
                                     "    </ECEntityClass>"
                                     "  <ECRelationshipClass typeName='Rel' strength='embedding' modifier='Sealed'>"
                                     "      <ECCustomAttributes>"
                                     "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'>"
                                     "          <OnDeleteAction>Cascade</OnDeleteAction>"
                                     "        </ForeignKeyConstraint>"
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
TEST_F(RelationshipMappingTestFixture, ForeignKeyConstraintWhereLinkTableIsRequired)
    {
    SchemaItem testItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.0\">"
                        "  <ECSchemaReference name = 'ECDbMap' version='02.00' prefix = 'ecdbmap' />"
                        "  <ECEntityClass typeName='Parent' >"
                        "    <ECProperty propertyName='Name' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECEntityClass typeName='Child' >"
                        "    <ECProperty propertyName='ParentId' typeName='long' />"
                        "    <ECProperty propertyName='ChildName' typeName='string' />"
                        "    <ECNavigationProperty propertyName='Parent' relationshipName='ParentHasChildren' direction='Backward' />"
                        "  </ECEntityClass>"
                        "  <ECEntityClass typeName='Child2' >"
                        "    <ECProperty propertyName='ParentId' typeName='long' />"
                        "    <ECProperty propertyName='ChildName' typeName='string' />"
                        "  </ECEntityClass>"
                        "  <ECRelationshipClass typeName='ParentHasChildren' strength='referencing' modifier='Sealed'>"
                        "    <ECCustomAttributes>"
                        "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                        "    </ECCustomAttributes>"
                        "    <Source cardinality='(0,1)' polymorphic='True'>"
                        "      <Class class = 'Parent' />"
                        "    </Source>"
                        "    <Target cardinality='(0,N)' polymorphic='True'>"
                        "      <Class class = 'Child' />"
                        "    </Target>"
                        "    <ECProperty propertyName='ForcingToLinkTable' typeName='string' />"
                        "  </ECRelationshipClass>"
                        "</ECSchema>", false, "Cannot apply ForeignKeyConstraint when a link table is required.");

    AssertSchemaImport(testItem, "ForeignKeyConstraintWhereLinkTableIsRequired.ecdb");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, UseECInstanceIdAsForeignKey)
    {
    SetupECDb("useecinstanceidasfk1.ecdb", SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                             <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Parent">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECEntityClass typeName="Child" >
                                <ECProperty propertyName="ChildName" typeName="string" />
                                <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasChildren" direction="Backward"/>
                              </ECEntityClass>
                              <ECRelationshipClass typeName="ParentHasChildren" strength="referencing" modifier="Sealed">
                                 <ECCustomAttributes>
                                    <UseECInstanceIdAsForeignKey xmlns='ECDbMap.02.00'/>
                                    <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>
                                 </ECCustomAttributes>
                                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="is parent of">
                                  <Class class="Parent" />
                                </Source>
                                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is child of">
                                  <Class class="Child"/>
                                </Target>
                              </ECRelationshipClass>
                            </ECSchema>)xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    AssertForeignKey(true, GetECDb(), "ts1_Child", "Id");
    GetECDb().CloseDb();

    SetupECDb("useecinstanceidasfk2.ecdb", SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                             <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Parent">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECEntityClass typeName="Child" >
                                <ECProperty propertyName="ChildName" typeName="string" />
                                <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasChildren" direction="Backward"/>
                              </ECEntityClass>
                              <ECRelationshipClass typeName="ParentHasChildren" strength="embedding" modifier="Sealed">
                                 <ECCustomAttributes>
                                    <ForeignKeyConstraint xmlns='ECDbMap.02.00'>
                                        <OnDeleteAction>Cascade</OnDeleteAction>
                                    </ForeignKeyConstraint>
                                    <UseECInstanceIdAsForeignKey xmlns='ECDbMap.02.00'/>
                                 </ECCustomAttributes>
                                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="is parent of">
                                  <Class class="Parent" />
                                </Source>
                                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is child of">
                                  <Class class="Child"/>
                                </Target>
                              </ECRelationshipClass>
                            </ECSchema>)xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    AssertForeignKeyDdl(GetECDb(), "ts2_Child", "FOREIGN KEY([Id]) REFERENCES [ts2_Parent]([Id]) ON DELETE CASCADE)");
    GetECDb().CloseDb();

    //UseECInstanceIdAsForeignKey where FK is in joined table
    SetupECDb("useecinstanceidasfk4.ecdb", SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts4" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                             <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Parent">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECEntityClass typeName="Child" >
                                 <ECCustomAttributes>
                                    <ClassMap xmlns="ECDbMap.02.00">
                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00" />
                                 </ECCustomAttributes>
                                <ECProperty propertyName="ChildName" typeName="string" />
                              </ECEntityClass>
                              <ECEntityClass typeName="SubChild" >
                                <BaseClass>Child</BaseClass>
                                <ECProperty propertyName="SubChildName" typeName="string" />
                                <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasSubChildren" direction="Backward"/>
                              </ECEntityClass>
                              <ECRelationshipClass typeName="ParentHasSubChildren" strength="referencing" modifier="Sealed">
                                 <ECCustomAttributes>
                                    <ForeignKeyConstraint xmlns="ECDbMap.02.00">
                                        <OnDeleteAction>SetNull</OnDeleteAction>
                                    </ForeignKeyConstraint>
                                    <UseECInstanceIdAsForeignKey xmlns="ECDbMap.02.00"/>
                                 </ECCustomAttributes>
                                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="is parent of">
                                  <Class class="Parent" />
                                </Source>
                                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is subchild of">
                                  <Class class="SubChild" />
                                </Target>
                              </ECRelationshipClass>
                            </ECSchema>)xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    AssertForeignKeyDdl(GetECDb(), "ts4_SubChild", "FOREIGN KEY([ChildId]) REFERENCES [ts4_Parent]([Id]) ON DELETE SET NULL)");
    GetECDb().CloseDb();

    SetupECDb("useecinstanceidasfk5.ecdb", SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts5" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                             <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Parent">
                                <ECProperty propertyName="Name" typeName="string" />
                              </ECEntityClass>
                              <ECEntityClass typeName="Child" >
                                 <ECCustomAttributes>
                                    <ClassMap xmlns="ECDbMap.02.00">
                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00" />
                                 </ECCustomAttributes>
                                <ECProperty propertyName="ChildName" typeName="string" />
                              </ECEntityClass>
                              <ECEntityClass typeName="SubChild" >
                                <BaseClass>Child</BaseClass>
                                <ECProperty propertyName="SubChildName" typeName="string" />
                                <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasSubChildren" direction="Backward"/>
                              </ECEntityClass>
                              <ECRelationshipClass typeName="ParentHasSubChildren" strength="referencing" modifier="None">
                                 <ECCustomAttributes>
                                    <ForeignKeyConstraint xmlns="ECDbMap.02.00">
                                        <OnDeleteAction>SetNull</OnDeleteAction>
                                    </ForeignKeyConstraint>
                                    <UseECInstanceIdAsForeignKey xmlns="ECDbMap.02.00"/>
                                 </ECCustomAttributes>
                                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="is parent of">
                                  <Class class="Parent" />
                                </Source>
                                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is subchild of">
                                  <Class class="SubChild" />
                                </Target>
                              </ECRelationshipClass>
                              <ECRelationshipClass typeName="ParentHasSubChildren_2" strength="referencing" modifier="Sealed">
                                <BaseClass>ParentHasSubChildren</BaseClass>
                                <Source multiplicity="(1..1)" polymorphic="True" roleLabel="is parent of">
                                  <Class class="Parent" />
                                </Source>
                                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is subchild of">
                                  <Class class="SubChild" />
                                </Target>
                              </ECRelationshipClass>
                            </ECSchema>)xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    AssertForeignKeyDdl(GetECDb(), "ts5_SubChild", "FOREIGN KEY([ChildId]) REFERENCES [ts5_Parent]([Id]) ON DELETE SET NULL)");
    ASSERT_TRUE(GetECDb().ColumnExists("ts5_SubChild", "ChildRelECClassId"));
    GetECDb().CloseDb();


    SetupECDb("useecinstanceidasfk6.ecdb", SchemaItem(R"xml(
                            <ECSchema schemaName="TestSchema" alias="ts6" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                             <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                              <ECEntityClass typeName="Model">
                                 <ECCustomAttributes>
                                    <ClassMap xmlns="ECDbMap.02.00">
                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>                               
                                 </ECCustomAttributes>
                                 <ECProperty propertyName="Name" typeName="string" />
                                 <ECNavigationProperty propertyName="Element" relationshipName="ModelModelsElement" direction="Forward"/>
                              </ECEntityClass>
                              <ECEntityClass typeName="PhysicalModel">
                                <BaseClass>Model</BaseClass>
                                <ECProperty propertyName="Bla" typeName="string" />
                              </ECEntityClass>
                              <ECEntityClass typeName="Element" >
                                 <ECCustomAttributes>
                                    <ClassMap xmlns="ECDbMap.02.00">
                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00" />
                                 </ECCustomAttributes>
                                <ECProperty propertyName="Code" typeName="string" />
                              </ECEntityClass>
                              <ECEntityClass typeName="PhysicalElement" >
                                <BaseClass>Element</BaseClass>
                                <ECProperty propertyName="Geomstream" typeName="binary" />
                              </ECEntityClass>
                              <ECRelationshipClass typeName="ModelModelsElement" strength="embedding" strengthDirection="Backward" modifier="None">
                                 <ECCustomAttributes>
                                    <ForeignKeyConstraint xmlns="ECDbMap.02.00">
                                        <OnDeleteAction>NoAction</OnDeleteAction>
                                    </ForeignKeyConstraint>
                                    <UseECInstanceIdAsForeignKey xmlns="ECDbMap.02.00"/>
                                 </ECCustomAttributes>
                                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="models">
                                  <Class class="Model" />
                                </Source>
                                <Target multiplicity="(1..1)" polymorphic="True" roleLabel="is modeled by">
                                  <Class class="Element" />
                                </Target>
                              </ECRelationshipClass>
                              <ECRelationshipClass typeName="PhysicalModelBreaksDownPhysicalElement" strength="embedding" strengthDirection="Backward" modifier="None">
                                <BaseClass>ModelModelsElement</BaseClass>
                                <Source multiplicity="(0..1)" roleLabel="breaks down" polymorphic="true">
                                    <Class class="PhysicalModel"/>
                                </Source>
                                <Target multiplicity="(1..1)" roleLabel="is broken down by" polymorphic="true">
                                    <Class class="PhysicalElement" />
                                </Target>
                              </ECRelationshipClass>
                            </ECSchema>)xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    AssertForeignKeyDdl(GetECDb(), "ts6_Model", "FOREIGN KEY([Id]) REFERENCES [ts6_Element]([Id]) ON DELETE NO ACTION)");
    Utf8String modelDdl = RetrieveDdl(GetECDb(), "ts6_Model");
    ASSERT_FALSE(modelDdl.empty());
    ASSERT_TRUE(modelDdl.ContainsI("[RelECClassId] INTEGER NOT NULL,")) << modelDdl.c_str();
    GetECDb().CloseDb();


    std::vector<SchemaItem> invalidSchemas;
    invalidSchemas.push_back(SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts_invalid" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Parent">
               <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child" >
               <ECProperty propertyName="ChildName" typeName="string" />
               <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasChildren" direction="Backward" />                 
            </ECEntityClass>
            <ECRelationshipClass typeName="ParentHasChildren" strength="referencing" modifier="Sealed">
                <ECCustomAttributes>
                    <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>
                    <UseECInstanceIdAsForeignKey xmlns='ECDbMap.02.00'/>
                </ECCustomAttributes>
                <Source multiplicity="(0..*)" polymorphic="True" roleLabel="is parent of">
                    <Class class="Parent" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is child of">
                    <Class class="Child"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml", false, "UseECInstanceIdAsForeignKey on link table is not supported"));

    invalidSchemas.push_back(SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts_invalid" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Parent">
               <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child" >
               <ECProperty propertyName="ChildName" typeName="string" />
               <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasChildren" direction="Backward" />                 
            </ECEntityClass>
            <ECRelationshipClass typeName="ParentHasChildren" strength="referencing" modifier="Sealed">
                <ECCustomAttributes>
                    <LinkTableRelationshipMap xmlns='ECDbMap.02.00'/>
                    <UseECInstanceIdAsForeignKey xmlns='ECDbMap.02.00'/>
                </ECCustomAttributes>
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="is parent of">
                    <Class class="Parent" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is child of">
                    <Class class="Child"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml", false, "UseECInstanceIdAsForeignKey if LinkTableRelationshipMap CA is present is not supported"));

    invalidSchemas.push_back(SchemaItem(R"xml(
        <ECSchema schemaName="TestSchema" alias="ts_invalid" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Parent">
               <ECProperty propertyName="Name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child" >
               <ECProperty propertyName="ChildName" typeName="string" />
               <ECNavigationProperty propertyName="Parent" relationshipName="ParentHasChildren" direction="Backward" />                 
            </ECEntityClass>
            <ECRelationshipClass typeName="ParentHasChildren" strength="referencing" modifier="Sealed">
                <ECCustomAttributes>
                    <UseECInstanceIdAsForeignKey xmlns='ECDbMap.02.00'/>
                    <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>
                </ECCustomAttributes>
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="is parent of">
                    <Class class="Parent" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="is child of">
                    <Class class="Child"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml", false, "UseECInstanceIdAsForeignKey implies multiplicity of 1..1 on end side"));

    AssertSchemaImport(invalidSchemas, "UseECInstanceIdAsForeignKey_invalidcases.ecdb");

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, RelationshipWithAbstractConstraintClassAndNoSubclasses)
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
        </ECSchema>)xml", false, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipWithAbstractBaseClass.ecdb");
    ASSERT_FALSE(asserted);
    }

    {
    SchemaItem testItem(
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
        </ECSchema>)xml", false, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipWithAbstractBaseClass.ecdb");
    ASSERT_FALSE(asserted);
    }

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
        bool asserted = false;
        AssertSchemaImport(ecdb, asserted, testSchema, "RelationshipWithAbstractBaseClass.ecdb");
        ASSERT_FALSE(asserted);

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
    SchemaItem testItem(R"xml(<ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
                        </ECSchema>)xml", true, "");

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipWithAbstractClassAsConstraintOnChildEnd.ecdb");
    ASSERT_FALSE(asserted);

    ECClassId faceClassId = ecdb.Schemas().GetClassId("TestSchema", "Face");
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
TEST_F(RelationshipMappingTestFixture, RelationshipWithNotMappedClassAsConstraint)
    {
            {
            SchemaItem testItem(
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
                </ECSchema>)xml", false, "1:N Relationship having NotMapped constraint class on both sides of relationship are not supported");
            bool asserted = false;
            ECDb ecdb;
            AssertSchemaImport(ecdb, asserted, testItem, "relationshipwithnotmappedclassacsconstraint.ecdb");
            ASSERT_FALSE(asserted);
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
                "</ECSchema>", false, "N:N Relationship having not mapped constraint class on both sides of relationship are not supported");

            bool asserted = false;
            ECDb ecdb;
            AssertSchemaImport(ecdb, asserted, testItem, "relationshipwithnotmappedclassacsconstraint.ecdb");
            ASSERT_FALSE(asserted);
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
                "</ECSchema>", false, "N:N Relationship having at least one NotMapped constraint class on both sides of relationship are not supported");

            bool asserted = false;
            ECDb ecdb;
            AssertSchemaImport(ecdb, asserted, testItem, "relationshipwithnotmappedclassacsconstraint.ecdb");
            ASSERT_FALSE(asserted);
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
                "</ECSchema>", false, "N:N Relationships having at least one NotMapped constraint class on one sides of relationship are not supported");

            bool asserted = false;
            ECDb ecdb;
            AssertSchemaImport(ecdb, asserted, testItem, "relationshipwithnotmappedclassacsconstraint.ecdb");
            ASSERT_FALSE(asserted);
            }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, AddDerivedClassOfConstraintOnNsideOf1NRelationship)
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

    ECClassCP item = ecdb.Schemas().GetClass("OpenPlant", "ITEM");
    ECClassCP unit = ecdb.Schemas().GetClass("OpenPlant", "UNIT");

    Savepoint sp(ecdb, "CRUD Operations");
    //Insert Statements
    {
    //relationship between UNIT and ITEM
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')"));

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401, 201, %llu, 101, %llu)", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    }

    //Select statements
    {
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(ecdb, "SELECT * FROM op.UNIT_HAS_ITEM"));

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId=%s", item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %s", item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId=%s", item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
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

    item = ecdb.Schemas().GetClass("OpenPlant", "ITEM");
    unit = ecdb.Schemas().GetClass("OpenPlant", "UNIT");
    ECClassCP item_3D = ecdb.Schemas().GetClass("OpenPlant_3D", "ITEM_3D");

    //Insert Statements
    {
    //relationship between UNIT and ITEM
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')"));

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401, 201, %llu, 101, %llu)", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));

    //relationship between UNIT and ITEM_3D(new derived Class)
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(202, 'unitString2')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op3d.ITEM_3D(ECInstanceId, op_ITEM_prop, op3d_ITEM_prop) VALUES(301, 'itemString1', 'item3dString1')"));

    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(402, 202, %llu, 301, %llu)", unit->GetId().GetValue(), item_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    }

    //Select statements
    {
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(ecdb, "SELECT * FROM op.UNIT_HAS_ITEM"));

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));

    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));

    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE ECInstanceId = 402 AND TargetECClassId = %llu", item_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    //verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE TargetECClassId = %llu", item_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, AddDerivedClassOfConstraintOn1sideOf1NRelationship)
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

    ECClassCP unit = ecdb.Schemas().GetClass("OpenPlant", "UNIT");
    ECClassCP item = ecdb.Schemas().GetClass("OpenPlant", "ITEM");

    Savepoint sp(ecdb, "CRUD operations");
    //Insert Statements
    {
    //relationship between UNIT and ITEM
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')"));

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401, 201, %llu, 101, %llu)", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    }

    //Select Statements
    {
    Utf8String ecsql;
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(ecdb, "SELECT * FROM op.UNIT_HAS_ITEM"));

    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
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

    item = ecdb.Schemas().GetClass("OpenPlant", "ITEM");
    unit = ecdb.Schemas().GetClass("OpenPlant", "UNIT");
    ECClassCP unit_3D = ecdb.Schemas().GetClass("OpenPlant_3D", "UNIT_3D");

    //Insert Statements
    {
    Utf8String ecsql;
    //relationship between UNIT and ITEM
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')"));

    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(401, 201, %llu, 101, %llu)", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));

    //relationship between UNIT_3D(new derived Class) and ITEM
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op3d.UNIT_3D(ECInstanceId, op_UNIT_prop, op3d_UNIT_prop) VALUES(301, 'unitString2', 'unit3dString2')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(102, 'itemString2')"));

    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(402, 301, %llu, 102, %llu)", unit_3D->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    }

    //Select Statements
    {
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(ecdb, "SELECT * FROM op.UNIT_HAS_ITEM"));

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));

    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));

    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE ECInstanceId = 402 AND SourceECClassId=%s", unit_3D->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu", unit_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, AddDerivedClassOfConstraintsForNNRelationship)
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

    ECClassCP item = ecdb.Schemas().GetClass("OpenPlant", "ITEM");
    ECClassCP unit = ecdb.Schemas().GetClass("OpenPlant", "UNIT");

    Savepoint sp(ecdb, "CRUD Operations");
    //Insert Statements
    {
    //relationship between UNIT and ITEM
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')"));

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, relProp) VALUES(401, 201, %llu, 101, %llu, 'relPropString1')", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    }

    //Select statements
    {
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(ecdb, "SELECT * FROM op.UNIT_HAS_ITEM"));

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu AND TargetECClassId = %llu", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    }

    //update Statement
    {
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "UPDATE op.UNIT_HAS_ITEM SET relProp='relPropUpdatedString1' WHERE ECInstanceId=401"));
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %s AND TargetECClassId = %s", unit->GetId().ToString().c_str(),
                  item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    //verify Deltion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %s AND TargetECClassId = %s", unit->GetId().ToString().c_str(), item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
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

    item = ecdb.Schemas().GetClass("OpenPlant", "ITEM");
    unit = ecdb.Schemas().GetClass("OpenPlant", "UNIT");
    ECClassCP item_3D = ecdb.Schemas().GetClass("OpenPlant_3D", "ITEM_3D");
    ECClassCP unit_3D = ecdb.Schemas().GetClass("OpenPlant_3D", "UNIT_3D");

    //Insert Statements
    {
    //relationship between UNIT and ITEM
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op.UNIT(ECInstanceId, op_UNIT_prop) VALUES(201, 'unitString1')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op.ITEM(ECInstanceId, op_ITEM_prop) VALUES(101, 'itemString1')"));

    Utf8String ecsql;
    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, relProp) VALUES(501, 201, %llu, 101, %llu, 'relPropString1')", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));

    //relationship between UNIT_3D and ITEM_3D newly added derived classes
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op3d.UNIT_3D(ECInstanceId, op_UNIT_prop, op3d_UNIT_prop) VALUES(401, 'unitString2', 'unit3dString2')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "INSERT INTO op3d.ITEM_3D(ECInstanceId, op_ITEM_prop, op3d_ITEM_prop) VALUES(301, 'itemString2', 'item3dString2')"));

    ecsql.Sprintf("INSERT INTO op.UNIT_HAS_ITEM(ECInstanceId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId, relProp) VALUES(502, 401, %llu, 301, %llu, 'relPropString2')", unit_3D->GetId().GetValue(), item_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    }

    //Select statements
    {
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(ecdb, "SELECT * FROM op.UNIT_HAS_ITEM"));

    Utf8String ecsql;
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu AND TargetECClassId = %llu", unit->GetId().GetValue(), item->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));

    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %llu AND TargetECClassId = %llu", unit_3D->GetId().GetValue(), item_3D->GetId().GetValue());
    ASSERT_EQ(BE_SQLITE_ROW, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    }

    //update Statement
    {
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "UPDATE op.UNIT_HAS_ITEM SET relProp='relPropUpdatedString1' WHERE ECInstanceId=501"));

    //update relationship between newly added derived classes
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, "UPDATE op.UNIT_HAS_ITEM SET relProp='relPropUpdatedString2' WHERE ECInstanceId=502"));
    }

    //Delete Statements
    {
    Utf8String ecsql;
    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %s AND TargetECClassId = %s", unit->GetId().ToString().c_str(), item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %s AND TargetECClassId = %s", unit->GetId().ToString().c_str(), item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));

    ecsql.Sprintf("DELETE FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %s AND TargetECClassId = %s", unit_3D->GetId().ToString().c_str(), item_3D->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    //Verify Deletion
    ecsql.Sprintf("SELECT * FROM op.UNIT_HAS_ITEM WHERE SourceECClassId = %s AND TargetECClassId = %s", unit->GetId().ToString().c_str(), item->GetId().ToString().c_str());
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(ecdb, ecsql.c_str()));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Maha Nasir                  02/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, StrengthDirectionValidityOnEndTableRelationship)
    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(
        SchemaItem(
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
                <ECCustomAttributes>
                    <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                </ECCustomAttributes>
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="Model Has Elements">
                  <Class class="Model" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Model Has Elements (Reversed)">
                  <Class class="Element" />
                </Target>
              </ECRelationshipClass>
            </ECSchema>)xml", false, "For a FKRelationship class with strength 'embedding', the cardinality 1-N requires the direction to be 'forward'.")); //Fails because the direction is Forward despite setting it to Backward.

    testSchemas.push_back(
        SchemaItem(
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
                <ECCustomAttributes>
                    <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                </ECCustomAttributes>
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="Model Has Elements">
                  <Class class="Model" />
                </Source>
                <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Model Has Elements (Reversed)">
                  <Class class="Element" />
                </Target>
              </ECRelationshipClass>
            </ECSchema>)xml", true, "Mapping of FKRelationshipClass with strength 'embedding' and direction 'forward' for a 1-N cardinality, is expected to succeed."));

    testSchemas.push_back(
        SchemaItem(
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
                <ECCustomAttributes>
                    <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                </ECCustomAttributes>
                <Source multiplicity="(0..*)" polymorphic="True" roleLabel="Model Has Elements">
                  <Class class="Model" />
                </Source>
                <Target multiplicity="(0..1)" polymorphic="True" roleLabel="Model Has Elements (Reversed)">
                  <Class class="Element" />
                </Target>
              </ECRelationshipClass>
            </ECSchema>)xml", true, "Mapping of FKRelationshipClass with strength 'embedding' and direction 'Backward' for a N-1 cardinality, is expected to succeed.")); //Fails because the direction is Forward despite setting it to Backward.

    testSchemas.push_back(
        SchemaItem(
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
            "    <ECCustomAttributes>"
            "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
            "    </ECCustomAttributes>"
            "    <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Model Has Elements'>"
            "      <Class class='Model' />"
            "    </Source>"
            "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
            "      <Class class='Element' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", false, "For a FKRelationship class with strength 'embedding', the cardinality N-1 requires the direction to be 'Backward'."));

    testSchemas.push_back(
        SchemaItem(
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
            "    <ECCustomAttributes>"
            "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
            "    </ECCustomAttributes>"
            "    <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements'>"
            "      <Class class='Model' />"
            "    </Source>"
            "    <Target multiplicity='(0..1)' polymorphic='True' roleLabel='Model Has Elements (Reversed)'>"
            "      <Class class='Element' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>", true, "Mapping of FKRelationshipClass with strength 'embedding' and direction 'Backward' for a 1-1 cardinality, is expected to succeed."));

    AssertSchemaImport(testSchemas, "StrengthDirectionValidityTest.ecdb");
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                       08/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, DiegoRelationshipTest)
    {
        ECDbCR ecdb = SetupECDb("diegorelationshiptest.ecdb", SchemaItem({
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
            )xml"}, true, ""));
        ASSERT_TRUE(ecdb.IsDbOpen());

        ECClassCP civilModelClass = ecdb.Schemas().GetClass("DiegoSchema1", "CivilModel");
        ASSERT_TRUE(civilModelClass != nullptr);
        ECClassCP datasetModelClass = ecdb.Schemas().GetClass("DiegoSchema1", "DataSetModel");
        ASSERT_TRUE(datasetModelClass != nullptr);
        ECClassCP relClass = ecdb.Schemas().GetClass("DiegoSchema1", "CivilModelHasDataSetModel");
        ASSERT_TRUE(relClass != nullptr);
        ECClassCP geometricModelClass = ecdb.Schemas().GetClass("DiegoSchema2", "GeometricModel");
        ASSERT_TRUE(geometricModelClass != nullptr);

        IECInstancePtr civilModel1 = ECDbTestUtility::CreateArbitraryECInstance(*civilModelClass);
        IECInstancePtr civilModel2 = ECDbTestUtility::CreateArbitraryECInstance(*civilModelClass);
        IECInstancePtr geometricModel = ECDbTestUtility::CreateArbitraryECInstance(*geometricModelClass);

        StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*(relClass->GetRelationshipClassCP()));
        StandaloneECRelationshipInstancePtr rel1 = relationshipEnabler->CreateRelationshipInstance();

        rel1->SetSource(civilModel2.get());
        rel1->SetTarget(geometricModel.get());

        ECInstanceInserter civilModelInserter(ecdb, *civilModelClass, nullptr);
        ASSERT_TRUE(civilModelInserter.IsValid());
        ASSERT_EQ(BE_SQLITE_OK, civilModelInserter.Insert(*civilModel1));
        ASSERT_EQ(BE_SQLITE_OK, civilModelInserter.Insert(*civilModel2));

        ECInstanceInserter geometricModelInserter(ecdb, *geometricModelClass, nullptr);
        ASSERT_TRUE(geometricModelInserter.IsValid());
        ASSERT_EQ(BE_SQLITE_OK, geometricModelInserter.Insert(*geometricModel));

        ECInstanceInserter relInserter(ecdb, *relClass, nullptr);
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
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), ecsql.c_str())) << ecsql.c_str();
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
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), ecsql.c_str())) << ecsql.c_str();
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
    SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
              SchemaItem("1:N and holding",
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
                              <ECCustomAttributes>
                                  <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                              </ECCustomAttributes>
                              <Source cardinality="(0,1)" polymorphic="True">
                                  <Class class="Geometry" />
                              </Source>
                             <Target cardinality="(0,1)" polymorphic="True">
                                 <Class class="GeometryPart" />
                              </Target>
                           </ECRelationshipClass>
                         </ECSchema>)xml"));
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey1.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    }

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetInstanceId()));
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
// @bsimethod                                   Muhammad.Zaighum                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, OneToOneBackward)
    {
    SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
              SchemaItem("1:N and holding",
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
                             <ECCustomAttributes>
                                 <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                             </ECCustomAttributes>
                             <Source cardinality="(0,1)" polymorphic="True">
                                 <Class class="GeometryPart" />
                              </Source>
                              <Target cardinality="(0,1)" polymorphic="True">
                                  <Class class="Geometry" />
                              </Target>
                           </ECRelationshipClass>
                         </ECSchema>)xml"));
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
    GetECDb().SaveChanges();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.PartHeldByGeometry(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey1.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    }

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetInstanceId()));
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
// @bsimethod                                   Muhammad.Zaighum                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, OneToManyForward)
    {
    SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
              SchemaItem("1:N and holding",
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
                         </ECSchema>)xml"));
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

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
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
    SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
              SchemaItem("1:N and holding",
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
                         </ECSchema>)xml"));
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

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
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
    SetupECDb("ecdbrelationshipmappingrules_manytomanyandholding.ecdb",
              SchemaItem("N:N and holding",
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
                         </ECSchema>)xml"));
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

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
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
    SetupECDb("ecdbrelationshipmappingrules_manytomanyandholding.ecdb",
              SchemaItem("N:N and holding",
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
                         </ECSchema>)xml"));
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

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
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

    ECSchemaCP schema = ecdb.Schemas().GetSchema("test", true);
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

    ECInstanceInserter inserter(ecdb, *baseClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());

    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*baseInstance1, true));
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*baseInstance2, true));

    //Insert Instances for ClassA
    ECN::StandaloneECInstancePtr classAInstance1 = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECN::StandaloneECInstancePtr classAInstance2 = classA->GetDefaultStandaloneEnabler()->CreateInstance();

    classAInstance1->SetValue("P1", ECValue("string1"));
    classAInstance2->SetValue("P1", ECValue("string2"));

    ECInstanceInserter classAinserter(ecdb, *classA, nullptr);
    ASSERT_TRUE(classAinserter.IsValid());

    ASSERT_EQ(BE_SQLITE_OK, classAinserter.Insert(*classAInstance1, true));
    ASSERT_EQ(BE_SQLITE_OK, classAinserter.Insert(*classAInstance2, true));

    //Insert Instances for ClassB
    ECN::StandaloneECInstancePtr classBInstance1 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECN::StandaloneECInstancePtr classBInstance2 = classB->GetDefaultStandaloneEnabler()->CreateInstance();

    classBInstance1->SetValue("P2", ECValue("string1"));
    classBInstance2->SetValue("P2", ECValue("string2"));

    ECInstanceInserter classBinserter(ecdb, *classB, nullptr);
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
    ECInstanceInserter relationshipinserter(ecdb, *baseHasClassAClass, nullptr);
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
    ECInstanceInserter relationshipinserter(ecdb, *baseHasClassBClass, nullptr);
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
    selectStmt.BindId(1, TPHKey1.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
    ECInstanceECSqlSelectAdapter TPHadapter(selectStmt);
    IECInstancePtr readInstance = TPHadapter.GetInstance();
    ASSERT_TRUE(readInstance.IsValid());
    selectStmt.Finalize();

    ECSqlStatement relationStmt;
    ASSERT_EQ(relationStmt.Prepare(ecdb, "INSERT INTO t.BaseHasClassA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    relationStmt.BindId(1, TPHKey1.GetInstanceId());
    relationStmt.BindId(2, TPHKey1.GetClassId());
    relationStmt.BindId(3, classAKey1.GetInstanceId());
    relationStmt.BindId(4, classAKey1.GetClassId());
    ASSERT_EQ(BE_SQLITE_DONE, relationStmt.Step());
    relationStmt.Finalize();

    //try to insert Duplicate relationship step() should return error
    ASSERT_EQ(relationStmt.Prepare(ecdb, "INSERT INTO t.BaseHasClassA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    relationStmt.BindId(1, TPHKey1.GetInstanceId());
    relationStmt.BindId(2, TPHKey1.GetClassId());
    relationStmt.BindId(3, classAKey1.GetInstanceId());
    relationStmt.BindId(4, classAKey1.GetClassId());
    ASSERT_TRUE((BE_SQLITE_CONSTRAINT_BASE & relationStmt.Step()) == BE_SQLITE_CONSTRAINT_BASE);
    relationStmt.Finalize();

    //retrieve ECInstance from Db After Inserting Relationship Instance, based on ECInstanceId, verify ECInstance is valid
    ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(ecdb, "SELECT * FROM t.ClassA WHERE ECInstanceId = ?"));
    selectStmt.BindId(1, classAKey1.GetInstanceId());
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
struct ReferentialIntegrityTestFixture : DbMappingTestFixture
    {
    private:
        void VerifyRelationshipInsertionIntegrity(ECDbCR ecdb, Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<DbResult> const& expected, size_t& rowInserted) const;
        size_t GetRelationshipInstanceCount(ECDbCR ecdb, Utf8CP relationshipClass) const;

    protected:
        void ExecuteRelationshipInsertionIntegrityTest(ECDbR ecdb, bool allowDuplicateRelationships, bool allowForeignKeyConstraint, bool schemaImportExpectedToSucceed) const;
    };

//--------------------------------------------------------------------------------------
// @bsimethod                              Muhammad Hassan                         04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ReferentialIntegrityTestFixture, ForeignKeyConstraint_EnforceReferentialIntegrity)
    {
    ECDbR ecdb = SetupECDb("ForeignKeyConstraint_EnforceReferentialIntegrity.ecdb");
    ExecuteRelationshipInsertionIntegrityTest(ecdb, false, true, true);
    //when AllowDuplicate is turned of, OneFooHasManyGoo will also be mapped as endtable therefore ReferentialIntegrityCheck will be performed for it, so there will be two rows in the ForeignKey table
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasManyGoo"));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                              Muhammad Hassan                         04/15
//--------------------------------------------------------------------------------------
TEST_F(ReferentialIntegrityTestFixture, ForeignKeyConstraint_EnforceReferentialIntegrityCheck_AllowDuplicateRelation)
    {
    ECDbR ecdb = SetupECDb("ForeignKeyConstraint_EnforceReferentialIntegrityCheck_AllowDuplicateRelation.ecdb");
    ExecuteRelationshipInsertionIntegrityTest(ecdb, true, true, true);
    //when AllowDuplicate is turned on, OneFooHasManyGoo will also be mapped as endtable therefore there will be only one row in the ForeignKey table
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasManyGoo"));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/15
//--------------------------------------------------------------------------------------
TEST_F(ReferentialIntegrityTestFixture, DoNotAllowDuplicateRelationships)
    {
    ECDbR ecdb = SetupECDb("RelationshipCardinalityTest.ecdb");
    ExecuteRelationshipInsertionIntegrityTest(ecdb, false, true, true);
    ASSERT_TRUE(ecdb.TableExists("ts_Foo"));
    ASSERT_TRUE(ecdb.TableExists("ts_Goo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasManyGoo"));
    ASSERT_TRUE(ecdb.TableExists("ts_ManyFooHasManyGoo"));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         02/15
//--------------------------------------------------------------------------------------
TEST_F(ReferentialIntegrityTestFixture, AllowDuplicateRelationships)
    {
    ECDbR ecdb = SetupECDb("RelationshipCardinalityTest_AllowDuplicateRelationships.ecdb");
    ExecuteRelationshipInsertionIntegrityTest(ecdb, true, true, true);
    ASSERT_TRUE(ecdb.TableExists("ts_Foo"));
    ASSERT_TRUE(ecdb.TableExists("ts_Goo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasManyGoo"));
    ASSERT_TRUE(ecdb.TableExists("ts_ManyFooHasManyGoo"));
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
void ReferentialIntegrityTestFixture::ExecuteRelationshipInsertionIntegrityTest(ECDbR ecdb, bool allowDuplicateRelationships, bool allowForeignKeyConstraint, bool schemaImportExpectedToSucceed) const
    {
    Utf8CP schemaTemplate = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="Testschema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
    <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
    <ECEntityClass typeName="Foo">
        <ECProperty propertyName="fooProp" typeName="string" />
    </ECEntityClass>
    <ECEntityClass typeName="Goo" >
        <ECProperty propertyName="gooProp" typeName="string" />
        <ECNavigationProperty propertyName="PartnerFoo" relationshipName="OneFooHasOneGoo" direction="Backward"/>
        <ECNavigationProperty propertyName="ParentFoo" relationshipName="OneFooHasManyGoo" direction="Backward"/>
    </ECEntityClass>
    <ECRelationshipClass typeName="OneFooHasOneGoo" strength="referencing" modifier="Sealed">
        <ECCustomAttributes>
            %s
        </ECCustomAttributes>
        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
            <Class class="Foo" />
        </Source>
        <Target multiplicity="(0..1)" polymorphic="True" roleLabel="has">
            <Class class="Goo" />
        </Target>
    </ECRelationshipClass>
    <ECRelationshipClass typeName="OneFooHasManyGoo" strength="referencing" modifier="Sealed">
        <ECCustomAttributes>
            %s
        </ECCustomAttributes>
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

    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, SchemaItem(testSchemaXml.c_str(), schemaImportExpectedToSucceed));
    ASSERT_FALSE(asserted);
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
        if (stmt.Prepare(GetECDb(), ecsql.c_str()) != ECSqlStatus::Success)
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
        if (stmt.Prepare(GetECDb(), ecsql.c_str()) != ECSqlStatus::Success)
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
        ECClassCP ecClass = GetECDb().Schemas().GetClass(key.GetClassId());
        if (ecClass == nullptr)
            return ERROR;

        Utf8String ecsql;
        ecsql.Sprintf("DELETE FROM %s WHERE ECInstanceId=%s", ecClass->GetECSqlName().c_str(), key.GetInstanceId().ToString().c_str());
        ECSqlStatement stmt;
        if (stmt.Prepare(GetECDb(), ecsql.c_str()) != ECSqlStatus::Success)
            return ERROR;

        return BE_SQLITE_DONE == stmt.Step() ? SUCCESS : ERROR;
        }

    //---------------------------------------------------------------------------------------
    //                                               Muhammad Hassan                  10/2014
    //+---------------+---------------+---------------+---------------+---------------+------
    bool HasInstance(ECInstanceKey const& key)
        {
        ECClassCP ecClass = GetECDb().Schemas().GetClass(key.GetClassId());
        if (ecClass == nullptr)
            return false;

        Utf8String ecsql;
        ecsql.Sprintf("SELECT NULL FROM ONLY %s WHERE ECInstanceId=%s",
                      ecClass->GetECSqlName().c_str(), key.GetInstanceId().ToString().c_str());

        ECSqlStatement statement;
        if (ECSqlStatus::Success != statement.Prepare(GetECDb(), ecsql.c_str()))
            return false;

        return statement.Step() == BE_SQLITE_ROW;
        }
    };



//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipStrengthTestFixture, BackwardEmbedding)
    {
    ECDbR ecdb = SetupECDb("BackwardRelationshipStrengthTest.ecdb", BeFileName(L"RelationshipStrengthTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());
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
    ecdb.SaveChanges();

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
    ECDbR ecdb = SetupECDb("RelationshipStrengthTest.ecdb", BeFileName(L"RelationshipStrengthTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

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

    ecdb.SaveChanges();

    //Verify instances before deletion
    ASSERT_TRUE(HasInstance(grandParent1HasSpouse));
    ASSERT_TRUE(HasInstance(grandParent2HasSpouse));

    /*
    * Test 1: Delete GrandParent1
    * Validate grandParent1HasSpouse, grandParent2HasSpouse, grandParent1HasSingleParent have been deleted (orphaned relationships)
    * Validate singleParent is still around (referencing relationship with one parent remaining)
    */
    DeleteInstance(grandParent1);
    ecdb.SaveChanges();

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
    ecdb.SaveChanges();

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
    ECDbR ecdb = SetupECDb("BackwardRelationshipStrengthTest.ecdb", BeFileName(L"RelationshipStrengthTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

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

    ecdb.SaveChanges();

    //Validate Instance exists before deletion
    ASSERT_TRUE(HasInstance(singleParentHasChild1));

    /*
    * Test 1: Delete Child1
    * Validate Child1 and singleParentHasChild1 have been deleted
    * Validate Child2 is Still there
    */
    DeleteInstance(child1);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(child1));
    ASSERT_FALSE(HasInstance(singleParentHasChild1));

    ASSERT_TRUE(HasInstance(child2));

    /*
    * Test 2: Delete Child2
    * Validate Child2 and singleParentHasChild2 have been deleted
    * Validate singleParent is still around (relationship grand parents remaining)
    */
    DeleteInstance(child2);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(child2));
    ASSERT_FALSE(HasInstance(singleParentHasChild2));

    ASSERT_TRUE(HasInstance(singleParent));

    /*
    * Test 3: Delete GrandParent1
    * Validate GrandParent1, grandParent1HasSpouse, grandParent2HasSpouse, singleParentHasGrandParent1 have been deleted
    * Validate singleParent is still around (referencing relationship with one parent remaining)
    */
    DeleteInstance(grandParent1);
    ecdb.SaveChanges();

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
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(grandParent2));
    ASSERT_FALSE(HasInstance(singleParentHasGrandParent2));
    ASSERT_TRUE(HasInstance(singleParent));
    }

END_ECDBUNITTESTS_NAMESPACE
