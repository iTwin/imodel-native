/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/SchemaManagerTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE


struct SchemaManagerTests : SchemaImportTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, ImportDifferentInMemorySchemaVersions)
    {
    ECDbR ecdb = SetupECDb("ecdbschemamanagertests.ecdb");

    auto importSchema = [] (ECDbR ecdb, ECVersion version, bool expectedToSucceed)
        {
        ecdb.SaveChanges();
        ECSchemaPtr schema = nullptr;
        ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "schema", "s", 1, 0, 0, version)) << "ECVersion " << ECSchema::GetECVersionString(version);

        bvector<ECSchemaCP> schemas;
        schemas.push_back(schema.get());

        if (expectedToSucceed)
            ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas(schemas)) << "ECVersion " << ECSchema::GetECVersionString(version);
        else
            ASSERT_EQ(ERROR, ecdb.Schemas().ImportSchemas(schemas)) << "ECVersion " << ECSchema::GetECVersionString(version);

        ecdb.AbandonChanges();
        };

    importSchema(ecdb, ECVersion::V2_0, false);
    importSchema(ecdb, ECVersion::V3_0, false);
    importSchema(ecdb, ECVersion::V3_1, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, ImportToken)
    {
    auto assertImport = [this] (Utf8CP ecschemaXml, BeFileNameCR seedFilePath)
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, CloneECDb(ecdb, (Utf8String(seedFilePath.GetFileNameWithoutExtension()) + "_unrestricted.ecdb").c_str(), seedFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));
        bool asserted = false;
        AssertSchemaImport(asserted, ecdb, SchemaItem(ecschemaXml, true));
        ASSERT_FALSE(asserted) << "SchemaImport into unrestricted ECDb failed unexpectedly for: " << ecschemaXml;
        ecdb.CloseDb();

        RestrictedSchemaImportECDb restrictedECDb(true, false);
        ASSERT_EQ(BE_SQLITE_OK, CloneECDb(restrictedECDb, (Utf8String(seedFilePath.GetFileNameWithoutExtension()) + "_restricted.ecdb").c_str(), seedFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));

        asserted = false;
        AssertSchemaImport(asserted, restrictedECDb, SchemaItem(ecschemaXml, false));
        ASSERT_FALSE(asserted) << "SchemaImport into restricted ECDb. Expected to fail for: " << ecschemaXml;
        restrictedECDb.CloseDb();
        };

    SetupECDb("importtokentests.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());
    BeFileName seedFileName(GetECDb().GetDbFileName());
    GetECDb().CloseDb();
    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='TestSchema' displayLabel='Test Schema' alias='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECEntityClass typeName='Foo' displayLabel='Spatial Element'>"
                 "        <ECProperty propertyName='Pet' typeName='string'/>"
                 "        <ECProperty propertyName='LastMod' typeName='DateTime'/>"
                 "    </ECEntityClass>"
                 "</ECSchema>", seedFileName);


    SetupECDb("importtokentests.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                    R"xml(<ECSchema schemaName="BaseSchema" alias="base" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbMap" />
                        <ECEntityClass typeName="BaseNoTph" modifier="Abstract">
                            <ECProperty propertyName="BaseProp1" typeName="long" />
                            <ECProperty propertyName="BaseProp2" typeName="string" />
                        </ECEntityClass>
                        <ECEntityClass typeName="Foo" modifier="Sealed">
                            <ECProperty propertyName="SubNoTphProp1" typeName="long" />
                            <ECProperty propertyName="SubNoTphProp2" typeName="string" />
                        </ECEntityClass>
                        <ECEntityClass typeName="TphBase" modifier="Abstract">
                            <ECCustomAttributes>
                                <ClassMap xmlns="ECDbMap.02.00">
                                    <MapStrategy>TablePerHierarchy</MapStrategy>
                                </ClassMap>
                                <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                            </ECCustomAttributes>
                            <ECProperty propertyName="BaseProp1" typeName="long" />
                            <ECProperty propertyName="BaseProp2" typeName="string" />
                            <ECNavigationProperty propertyName="Foo" relationshipName="FKRel" direction="Backward" />
                        </ECEntityClass>
                        <ECEntityClass typeName="SubSharedCols" modifier="None">
                            <ECCustomAttributes>
                                <ShareColumns xmlns="ECDbMap.02.00" >
                                    <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>
                                    <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                                </ShareColumns>
                            </ECCustomAttributes>
                            <BaseClass>TphBase</BaseClass>
                            <ECProperty propertyName="SubSharedColsProp1" typeName="long" />
                            <ECProperty propertyName="SubSharedColsProp2" typeName="string" />
                        </ECEntityClass>
                        <ECEntityClass typeName="SubNotSharedCols" modifier="None">
                            <BaseClass>TphBase</BaseClass>
                            <ECProperty propertyName="SubNotSharedColsProp1" typeName="long" />
                            <ECProperty propertyName="SubNotSharedColsProp2" typeName="string" />
                        </ECEntityClass>
                        <ECRelationshipClass typeName="FKRel" strength="Referencing" modifier="None">
                        <Source multiplicity="(0..1)" polymorphic="True" roleLabel="Parent">
                            <Class class ="Foo" />
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Children">
                            <Class class ="TphBase" />
                        </Target>
                        </ECRelationshipClass>
                        <ECRelationshipClass typeName="LinkTableRel" strength="Referencing" modifier="None">
                        <Source multiplicity="(0..*)" polymorphic="True" roleLabel="Lhs">
                            <Class class ="Foo" />
                        </Source>
                        <Target multiplicity="(0..*)" polymorphic="True" roleLabel="Rhs">
                            <Class class ="TphBase" />
                        </Target>
                        </ECRelationshipClass>
                    </ECSchema>)xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());
    seedFileName.AssignUtf8(GetECDb().GetDbFileName());
    GetECDb().CloseDb();
    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='SubClassNoTph' alias='sub' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='BaseSchema' version='01.00' alias='base' />"
                 "    <ECEntityClass typeName='Sub2' modifier='None'>"
                 "        <BaseClass>base:BaseNoTph</BaseClass>"
                 "        <ECProperty propertyName='Sub2Prop1' typeName='long' />"
                 "        <ECProperty propertyName='Sub2Prop2' typeName='string' />"
                 "    </ECEntityClass>"
                 "</ECSchema>", seedFileName);
    
    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='SubClassGetsJoinedTable' alias='sub' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='BaseSchema' version='01.00' alias='base' />"
                 "    <ECEntityClass typeName='Sub2' modifier='None'>"
                 "        <BaseClass>base:TphBase</BaseClass>"
                 "        <ECProperty propertyName='Sub2Prop1' typeName='long' />"
                 "        <ECProperty propertyName='Sub2Prop2' typeName='string' />"
                 "    </ECEntityClass>"
                 "</ECSchema>", seedFileName);

    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='NotSharedCols' alias='sub' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='BaseSchema' version='01.00' alias='base' />"
                 "    <ECEntityClass typeName='SubSub' modifier='None'>"
                 "        <BaseClass>base:SubNotSharedCols</BaseClass>"
                 "        <ECProperty propertyName='SubSubProp1' typeName='long' />"
                 "        <ECProperty propertyName='SubSubProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "</ECSchema>", seedFileName);

    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='SharedCols' alias='sub' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='BaseSchema' version='01.00' alias='base' />"
                 "    <ECEntityClass typeName='SubSub' modifier='None'>"
                 "        <BaseClass>base:SubSharedCols</BaseClass>"
                 "        <ECProperty propertyName='SubSubProp1' typeName='long' />"
                 "        <ECProperty propertyName='SubSubProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "</ECSchema>", seedFileName);

    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='RootFkRel' alias='sub' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='BaseSchema' version='01.00' alias='base' />"
                 "    <ECEntityClass typeName='Sub' modifier='None'>"
                 "        <BaseClass>base:TphBase</BaseClass>"
                 "        <ECProperty propertyName='SubSubProp1' typeName='long' />"
                 "        <ECNavigationProperty propertyName='Foo2' relationshipName='NewFKRel' direction='Backward'/>"
                 "    </ECEntityClass>"
                 "   <ECRelationshipClass typeName='NewFKRel' strength='Referencing' modifier='None'>"
                 "      <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent'>"
                 "          <Class class ='base:Foo' />"
                 "      </Source>"
                 "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Children'>"
                 "          <Class class ='Sub' />"
                 "      </Target>"
                 "   </ECRelationshipClass>"
                 "</ECSchema>", seedFileName);

    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='SubFkRel' alias='sub' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='BaseSchema' version='01.00' alias='base' />"
                 "   <ECRelationshipClass typeName='FKRelSub' strength='Referencing' modifier='None'>"
                 "        <BaseClass>base:FkRel</BaseClass>"
                 "      <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent'>"
                 "          <Class class ='base:Foo' />"
                 "      </Source>"
                 "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Children'>"
                 "          <Class class ='base:TphBase' />"
                 "      </Target>"
                 "   </ECRelationshipClass>"
                 "</ECSchema>", seedFileName);

    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='RootLinkTableRel' alias='sub' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='BaseSchema' version='01.00' alias='base' />"
                 "   <ECRelationshipClass typeName='NewLinkTableRel' strength='Referencing' modifier='None'>"
                 "      <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Parent'>"
                 "          <Class class ='base:Foo' />"
                 "      </Source>"
                 "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Children'>"
                 "          <Class class ='base:TphBase' />"
                 "      </Target>"
                 "   </ECRelationshipClass>"
                 "</ECSchema>", seedFileName);

    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='SubLinkTableRel' alias='sub' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='BaseSchema' version='01.00' alias='base' />"
                 "   <ECRelationshipClass typeName='NewLinkTableRel' strength='Referencing' modifier='None'>"
                 "        <BaseClass>base:LinkTableRel</BaseClass>"
                 "      <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Parent'>"
                 "          <Class class ='base:Foo' />"
                 "      </Source>"
                 "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Children'>"
                 "          <Class class ='base:TphBase' />"
                 "      </Target>"
                 "   </ECRelationshipClass>"
                 "</ECSchema>", seedFileName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, AllowChangesetMergingIncompatibleECSchemaImport)
    {
    auto assertImport = [this] (Utf8CP ecschemaXml, BeFileNameCR seedFilePath, std::pair<bool,bool> const& expectedToSucceed, Utf8CP scenario)
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, CloneECDb(ecdb, (Utf8String(seedFilePath.GetFileNameWithoutExtension()) + "_unrestricted.ecdb").c_str(), seedFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));
        bool asserted = false;
        AssertSchemaImport(asserted, ecdb, SchemaItem(ecschemaXml, expectedToSucceed.first));
        ASSERT_FALSE(asserted) << "SchemaImport into unrestricted ECDb failed unexpectedly for scenario " << scenario;
        ecdb.CloseDb();

        RestrictedSchemaImportECDb restrictedECDb(false, false);
        ASSERT_EQ(BE_SQLITE_OK, CloneECDb(restrictedECDb, (Utf8String(seedFilePath.GetFileNameWithoutExtension()) + "_restricted.ecdb").c_str(), seedFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));

        asserted = false;
        AssertSchemaImport(asserted, restrictedECDb, SchemaItem(ecschemaXml, expectedToSucceed.second));
        ASSERT_FALSE(asserted) << "SchemaImport into restricted ECDb. Expected to fail for scenario " << scenario;
        restrictedECDb.CloseDb();
        };

    SetupECDb("allowChangesetMergingIncompatibleECSchemaImport.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                                                  "<ECSchema schemaName='BaseSchema' alias='base' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                  "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbMap' />"
                                                  "    <ECEntityClass typeName='BaseNoChildren' modifier='Abstract'>"
                                                  "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                                                  "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='BaseNoTph' modifier='Abstract'>"
                                                  "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                                                  "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='Foo' modifier='Sealed'>"
                                                  "        <BaseClass>BaseNoTph</BaseClass>"
                                                  "        <ECProperty propertyName='FooProp1' typeName='long' />"
                                                  "        <ECProperty propertyName='FooProp2' typeName='string' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='TphBase' modifier='Abstract'>"
                                                  "          <ECCustomAttributes>"
                                                  "            <ClassMap xmlns='ECDbMap.02.00'>"
                                                  "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                                  "            </ClassMap>"
                                                  "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                                                  "          </ECCustomAttributes>"
                                                  "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                                                  "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='Sub1' modifier='None'>"
                                                  "        <BaseClass>TphBase</BaseClass>"
                                                  "        <ECProperty propertyName='Sub1Prop1' typeName='long' />"
                                                  "        <ECProperty propertyName='Sub1Prop2' typeName='string' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='Sub11' modifier='None'>"
                                                  "        <BaseClass>Sub1</BaseClass>"
                                                  "        <ECProperty propertyName='Sub11Prop1' typeName='long' />"
                                                  "        <ECProperty propertyName='Sub11Prop2' typeName='string' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='Sub2' modifier='None'>"
                                                  "        <BaseClass>TphBase</BaseClass>"
                                                  "        <ECProperty propertyName='Sub2Prop1' typeName='long' />"
                                                  "        <ECProperty propertyName='Sub2Prop2' typeName='string' />"
                                                  "    </ECEntityClass>"
                                                  "</ECSchema>", true));
    ASSERT_TRUE(GetECDb().IsDbOpen());
    BeFileName seedFileName(GetECDb().GetDbFileName());
    GetECDb().CloseDb();

    //adding a subclass for TPH and non-TPH
    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='BaseSchema' alias='base' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbMap' />"
                 "    <ECEntityClass typeName='BaseNoChildren' modifier='Abstract'>"
                 "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                 "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='BaseNoTph' modifier='Abstract'>"
                 "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                 "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='Foo' modifier='Sealed'>"
                 "        <BaseClass>BaseNoTph</BaseClass>"
                 "        <ECProperty propertyName='FooProp1' typeName='long' />"
                 "        <ECProperty propertyName='FooProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='Goo' modifier='Sealed'>"
                 "        <BaseClass>BaseNoTph</BaseClass>"
                 "        <ECProperty propertyName='GooProp1' typeName='long' />"
                 "        <ECProperty propertyName='GooProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='TphBase' modifier='Abstract'>"
                 "          <ECCustomAttributes>"
                 "            <ClassMap xmlns='ECDbMap.02.00'>"
                 "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                 "            </ClassMap>"
                 "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                 "          </ECCustomAttributes>"
                 "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                 "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='Sub1' modifier='None'>"
                 "        <BaseClass>TphBase</BaseClass>"
                 "        <ECProperty propertyName='Sub1Prop1' typeName='long' />"
                 "        <ECProperty propertyName='Sub1Prop2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='Sub11' modifier='None'>"
                 "        <BaseClass>Sub1</BaseClass>"
                 "        <ECProperty propertyName='Sub11Prop1' typeName='long' />"
                 "        <ECProperty propertyName='Sub11Prop2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='Sub12' modifier='None'>"
                 "        <BaseClass>Sub1</BaseClass>"
                 "        <ECProperty propertyName='Sub12Prop1' typeName='long' />"
                 "        <ECProperty propertyName='Sub12Prop2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='Sub2' modifier='None'>"
                 "        <BaseClass>TphBase</BaseClass>"
                 "        <ECProperty propertyName='Sub2Prop1' typeName='long' />"
                 "        <ECProperty propertyName='Sub2Prop2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='Sub3' modifier='None'>"
                 "        <BaseClass>TphBase</BaseClass>"
                 "        <ECProperty propertyName='Sub3Prop1' typeName='long' />"
                 "        <ECProperty propertyName='Sub3Prop2' typeName='string' />"
                 "    </ECEntityClass>"
                 "</ECSchema>", seedFileName, {true, true}, "adding subclasses");

    //deleting a subclass with TPH (expected to succeed as no table is deleted)
    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='BaseSchema' alias='base' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbMap' />"
                 "    <ECEntityClass typeName='BaseNoChildren' modifier='Abstract'>"
                 "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                 "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='BaseNoTph' modifier='Abstract'>"
                 "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                 "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='Foo' modifier='Sealed'>"
                 "        <BaseClass>BaseNoTph</BaseClass>"
                 "        <ECProperty propertyName='FooProp1' typeName='long' />"
                 "        <ECProperty propertyName='FooProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='TphBase' modifier='Abstract'>"
                 "          <ECCustomAttributes>"
                 "            <ClassMap xmlns='ECDbMap.02.00'>"
                 "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                 "            </ClassMap>"
                 "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                 "          </ECCustomAttributes>"
                 "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                 "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='Sub1' modifier='None'>"
                 "        <BaseClass>TphBase</BaseClass>"
                 "        <ECProperty propertyName='Sub1Prop1' typeName='long' />"
                 "        <ECProperty propertyName='Sub1Prop2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='Sub11' modifier='None'>"
                 "        <BaseClass>Sub1</BaseClass>"
                 "        <ECProperty propertyName='Sub11Prop1' typeName='long' />"
                 "        <ECProperty propertyName='Sub11Prop2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='Sub2' modifier='None'>"
                 "        <BaseClass>TphBase</BaseClass>"
                 "        <ECProperty propertyName='Sub2Prop1' typeName='long' />"
                 "        <ECProperty propertyName='Sub2Prop2' typeName='string' />"
                 "    </ECEntityClass>"
                 "</ECSchema>", seedFileName, {true, true}, "deleting a subclass with TPH");

    //deleting a subclass with TPH which is root of joined table (expected to fail as joined table is deleted)
    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
    "<ECSchema schemaName='BaseSchema' alias='base' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
    "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbMap' />"
    "    <ECEntityClass typeName='BaseNoChildren' modifier='Abstract'>"
    "        <ECProperty propertyName='BaseProp1' typeName='long' />"
    "        <ECProperty propertyName='BaseProp2' typeName='string' />"
    "    </ECEntityClass>"
    "    <ECEntityClass typeName='BaseNoTph' modifier='Abstract'>"
    "        <ECProperty propertyName='BaseProp1' typeName='long' />"
    "        <ECProperty propertyName='BaseProp2' typeName='string' />"
    "    </ECEntityClass>"
    "    <ECEntityClass typeName='Foo' modifier='Sealed'>"
    "        <BaseClass>BaseNoTph</BaseClass>"
    "        <ECProperty propertyName='FooProp1' typeName='long' />"
    "        <ECProperty propertyName='FooProp2' typeName='string' />"
    "    </ECEntityClass>"
    "    <ECEntityClass typeName='TphBase' modifier='Abstract'>"
    "          <ECCustomAttributes>"
    "            <ClassMap xmlns='ECDbMap.02.00'>"
    "                <MapStrategy>TablePerHierarchy</MapStrategy>"
    "            </ClassMap>"
    "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
    "          </ECCustomAttributes>"
    "        <ECProperty propertyName='BaseProp1' typeName='long' />"
    "        <ECProperty propertyName='BaseProp2' typeName='string' />"
    "    </ECEntityClass>"
    "    <ECEntityClass typeName='Sub1' modifier='None'>"
    "        <BaseClass>TphBase</BaseClass>"
    "        <ECProperty propertyName='Sub1Prop1' typeName='long' />"
    "        <ECProperty propertyName='Sub1Prop2' typeName='string' />"
    "    </ECEntityClass>"
    "    <ECEntityClass typeName='Sub11' modifier='None'>"
    "        <BaseClass>Sub1</BaseClass>"
    "        <ECProperty propertyName='Sub11Prop1' typeName='long' />"
    "        <ECProperty propertyName='Sub11Prop2' typeName='string' />"
    "    </ECEntityClass>"
                 "</ECSchema>", seedFileName, {true, false}, "deleting a subclass which is root of joined table");

    //deleting a subclass with non-TPH (expected to fail as table is deleted)
    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='BaseSchema' alias='base' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbMap' />"
                 "    <ECEntityClass typeName='BaseNoChildren' modifier='Abstract'>"
                 "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                 "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='BaseNoTph' modifier='Abstract'>"
                 "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                 "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='TphBase' modifier='Abstract'>"
                 "          <ECCustomAttributes>"
                 "            <ClassMap xmlns='ECDbMap.02.00'>"
                 "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                 "            </ClassMap>"
                 "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                 "          </ECCustomAttributes>"
                 "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                 "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='Sub1' modifier='None'>"
                 "        <BaseClass>TphBase</BaseClass>"
                 "        <ECProperty propertyName='Sub1Prop1' typeName='long' />"
                 "        <ECProperty propertyName='Sub1Prop2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='Sub11' modifier='None'>"
                 "        <BaseClass>Sub1</BaseClass>"
                 "        <ECProperty propertyName='Sub11Prop1' typeName='long' />"
                 "        <ECProperty propertyName='Sub11Prop2' typeName='string' />"
                 "    </ECEntityClass>"
                 "    <ECEntityClass typeName='Sub2' modifier='None'>"
                 "        <BaseClass>TphBase</BaseClass>"
                 "        <ECProperty propertyName='Sub2Prop1' typeName='long' />"
                 "        <ECProperty propertyName='Sub2Prop2' typeName='string' />"
                 "    </ECEntityClass>"
                 "</ECSchema>", seedFileName, {true, false}, "deleting a subclass without TPH");
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, ImportWithLocalizationSchemas)
    {
    ECDbCR ecdb = SetupECDb("invariantculturetest.ecdb");

    bool asserted = false;
    AssertSchemaImport(asserted, ecdb, SchemaItem({
        "<?xml version='1.0' encoding='utf-8' ?>"
        "<ECSchema schemaName='TestSchema' displayLabel='Test Schema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='CoreCustomAttributes' version='01.00' alias='CoreCA' />"
        "    <ECEnumeration typeName='Animal' displayLabel='Animal' backingTypeName='string' isStrict='True'>"
        "        <ECEnumerator value='Dog' />"
        "        <ECEnumerator value='Cat'/>"
        "    </ECEnumeration>"
        "    <ECEntityClass typeName='SpatialElement' displayLabel='Spatial Element'>"
        "        <ECProperty propertyName='Pet' typeName='Animal' displayLabel='Pet Type' />"
        "        <ECProperty propertyName='LastMod' typeName='DateTime' displayLabel='Last Modified Date'>"
        "          <ECCustomAttributes>"
        "            <DateTimeInfo xmlns='CoreCustomAttributes.01.00'>"
        "                <DateTimeKind>Unspecified</DateTimeKind>"
        "            </DateTimeInfo>"
        "          </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "</ECSchema>",
        "<?xml version='1.0' encoding='utf-8' ?>"
        "<ECSchema schemaName='TestSchema_Supplemental_Localization' alias='loc_de_DE' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00.13' alias='bsca' />"
        "    <ECSchemaReference name='SchemaLocalizationCustomAttributes' version='01.00.00' alias='LocCA' />"
        "<ECCustomAttributes>"
        "<SupplementalSchemaMetaData xmlns='Bentley_Standard_CustomAttributes.01.13'>"
        "  <PrimarySchemaName>House</PrimarySchemaName>"
        "  <PrimarySchemaMajorVersion>1</PrimarySchemaMajorVersion>"
        "  <PrimarySchemaMinorVersion>0</PrimarySchemaMinorVersion>"
        "  <Precedence>9900</Precedence>"
        "  <Purpose>Localization</Purpose>"
        "  <IsUserSpecific>False</IsUserSpecific>"
        "</SupplementalSchemaMetaData>"
        "<LocalizationSpecification xmlns='SchemaLocalizationCustomAttributes.01.00'>"
        "   <Locale>de-DE</Locale>"
        "   <Resource>"
        "      <LocalizationData>"
        "           <Key>TestSchema.DisplayLabel:aec9699d</Key>"
        "           <Value>Test Schema</Value>"
        "       </LocalizationData>"
        "       <LocalizationData>"
        "           <Key>TestSchema:SpatialElement.DisplayLabel:8d38c538</Key>"
        "           <Value>Raeumliches Element</Value>"
        "       </LocalizationData>"
        "       <LocalizationData>"
        "           <Key>TestSchema:SpatialElement:Pet.DisplayLabel:71edba29</Key>"
        "           <Value>Haustiertyp</Value>"
        "       </LocalizationData>"
        "       <LocalizationData>"
        "           <Key>TestSchema:SpatialElement:LastMod.DisplayLabel:51b7fc8b</Key>"
        "           <Value>Letzte Aenderung</Value>"
        "       </LocalizationData>"
        "       <LocalizationData>"
        "           <Key>TestSchema:Animal.DisplayLabel:88c59a40</Key>"
        "           <Value>Tier</Value>"
        "       </LocalizationData>"
        "       <LocalizationData>"
        "           <Key>TestSchema:Animal:Dog.DisplayLabel:71bb24ef</Key>"
        "           <Value>Hund</Value>"
        "       </LocalizationData>"
        "       <LocalizationData>"
        "           <Key>TestSchema:Animal:Cat.DisplayLabel:7fb2ebb9</Key>"
        "           <Value>Katze</Value>"
        "       </LocalizationData>"
        "   </Resource>"
        "</LocalizationSpecification>"
        "</ECCustomAttributes>"
        "</ECSchema>"
        }, true, ""));
    ASSERT_FALSE(asserted);

    SchemaManager const& schemas = GetECDb().Schemas();
    ECSchemaCP testSchema = schemas.GetSchema("TestSchema", false);
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_STRCASEEQ("Test Schema", testSchema->GetDisplayLabel().c_str());

    ECEnumerationCP animalEnum = schemas.GetEnumeration("TestSchema", "Animal");
    ASSERT_TRUE(animalEnum != nullptr);
    ASSERT_STRCASEEQ("Animal", animalEnum->GetDisplayLabel().c_str());
    ASSERT_EQ(2, (int) animalEnum->GetEnumeratorCount());
    for (ECEnumerator const* enumValue : animalEnum->GetEnumerators())
        {
        if (enumValue->GetString().EqualsIAscii("Dog"))
            ASSERT_STRCASEEQ("Dog", enumValue->GetDisplayLabel().c_str());
        else if (enumValue->GetString().EqualsIAscii("Cat"))
            ASSERT_STRCASEEQ("Cat", enumValue->GetDisplayLabel().c_str());
        else
            FAIL();
        }

    ECClassCP spatialElementClass = schemas.GetClass("TestSchema", "SpatialElement");
    ASSERT_TRUE(spatialElementClass != nullptr);
    ASSERT_STRCASEEQ("Spatial Element", spatialElementClass->GetDisplayLabel().c_str());

    ECPropertyCP prop = spatialElementClass->GetPropertyP("Pet");
    ASSERT_TRUE(prop != nullptr);
    ASSERT_STRCASEEQ("Pet Type", prop->GetDisplayLabel().c_str());

    prop = spatialElementClass->GetPropertyP("LastMod");
    ASSERT_TRUE(prop != nullptr);
    ASSERT_STRCASEEQ("Last Modified Date", prop->GetDisplayLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, IncrementalLoading)
    {
    SetupECDb("ecdbschemamanagertests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    BeFileName testFilePath(GetECDb().GetDbFileName());

    const int expectedClassCount = GetECDb().Schemas().GetSchema("ECSqlTest")->GetClassCount();
    GetECDb().CloseDb();

    {
    //GetSchema with ensureAllClassesLoaded = false
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));
    ECSchemaCP schema = ecdb.Schemas().GetSchema("ECSqlTest", false);
    ASSERT_TRUE(schema != nullptr);

    ASSERT_EQ(0, schema->GetClassCount()) << "ECDbSchemaManager::GetSchema (..., false) is expected to return an empty schema";
    }

    //GetSchema with ensureAllClassesLoaded = true
    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));
    ECSchemaCP schema = ecdb.Schemas().GetSchema("ECSqlTest", true);
    ASSERT_TRUE(schema != nullptr);

    ASSERT_EQ(expectedClassCount, schema->GetClassCount()) << "ECDbSchemaManager::GetSchema (..., true) is expected to return a fully populated schema";
    }

    //GetClass from a different schema first and then GetSchema with ensureAllClassesLoaded = true
    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));
    ECClassCP ecClass = ecdb.Schemas().GetClass("ECDbMeta", "ECClassDef");
    ASSERT_TRUE(ecClass != nullptr) << "ECDbSchemaManager::GetClass ('ECDbMeta', 'ECClassDef') is expected to succeed as the class exists in the ecdb file.";

    ECSchemaCP schema = ecdb.Schemas().GetSchema("ECSqlTest", false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(0, schema->GetClassCount()) << "ECDbSchemaManager::GetSchema (..., false) is expected to return a schema with only the classes already loaded.";

    schema = ecdb.Schemas().GetSchema("ECSqlTest", true);
    ASSERT_TRUE(schema != nullptr);

    ASSERT_EQ(expectedClassCount, schema->GetClassCount()) << "ECDbSchemaManager::GetSchema (..., true) is expected to return a fully populated schema even if ECDbSchemaManager::GetClass was called before for a class in a different ECSchema.";
    }

    //GetClass from same schema first and then GetSchema with ensureAllClassesLoaded = true
    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));

    ECClassCP ecClass = ecdb.Schemas().GetClass("ECSqlTest", "PA");
    ASSERT_TRUE(ecClass != nullptr) << "ECDbSchemaManager::GetClass('PA') is expected to succeed as the class exists in the ecdb file.";

    ECSchemaCP schema = ecdb.Schemas().GetSchema("ECSqlTest", false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(1, schema->GetClassCount()) << "ECDbSchemaManager::GetSchema(..., false) is expected to return a schema with only the classes already loaded.";

    schema = ecdb.Schemas().GetSchema("ECSqlTest", true);
    ASSERT_TRUE(schema != nullptr);

    ASSERT_EQ(expectedClassCount, schema->GetClassCount()) << "ECDbSchemaManager::GetSchema(..., true) is expected to return a fully populated schema even if ECDbSchemaManager::GetClass was called before for a class in the same schema.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, CasingTests)
    {
    ECDbCR ecdb = SetupECDb("schemamanagercasingtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSchemaCP schema = ecdb.Schemas().GetSchema("ECDBFILEinfo");
    ASSERT_TRUE(schema != nullptr && schema->GetName().EqualsI("ECDbFileInfo"));

    schema = ecdb.Schemas().GetSchema("ecsqltest");
    ASSERT_TRUE(schema != nullptr && schema->GetName().EqualsI("ECSqlTest"));

    ECClassCP ecclass = nullptr;
    ecclass = ecdb.Schemas().GetClass("ecsqltest", "P");
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = ecdb.Schemas().GetClass("ECSqlTest", "p");
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = ecdb.Schemas().GetClass("ecSqL", "P", SchemaLookupMode::ByAlias);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = ecdb.Schemas().GetClass("ecsql", "p", SchemaLookupMode::ByAlias);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = ecdb.Schemas().GetClass("ecsqlTest", "P", SchemaLookupMode::AutoDetect);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = ecdb.Schemas().GetClass("ecsqL", "P", SchemaLookupMode::AutoDetect);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = ecdb.Schemas().GetClass("ECSqlTest", "p", SchemaLookupMode::AutoDetect);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = ecdb.Schemas().GetClass("ecsql", "p", SchemaLookupMode::AutoDetect);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetDerivedClasses)
    {
    ECDbCR ecdb = SetupECDb("ecschemamanagertest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassCP baseClass = ecdb.Schemas().GetClass("ECSqlTest", "THBase");
    ASSERT_TRUE(baseClass != nullptr) << "Could not retrieve base class";

    //derived classes are not loaded when calling ECClass::GetDerivedClasses
    ASSERT_TRUE(baseClass->GetDerivedClasses().empty()) << "ECClass::GetDerivedClasses is expected to not load subclasses.";

    //derived classes are expected to be loaded when calling ECDbSchemaManager::GetDerivedClasses
    ASSERT_EQ(1, ecdb.Schemas().GetDerivedClasses(*baseClass).size()) << "Unexpected derived class count with derived classes now being loaded";

    //now ECClass::GetDerivedClasses can also be called
    ASSERT_EQ(1, baseClass->GetDerivedClasses().size()) << "Unexpected derived class count after derived classes were loaded";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetDerivedECClassesWithoutIncrementalLoading)
    {
    ECDbCR ecdb = SetupECDb("ecschemamanagertest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSchemaCP testSchema = ecdb.Schemas().GetSchema("ECSqlTest", true);
    ASSERT_TRUE(testSchema != nullptr);

    ECClassCP baseClass = ecdb.Schemas().GetClass("ECSqlTest", "THBase");
    ASSERT_TRUE(baseClass != nullptr) << "Could not retrieve base class";

    ASSERT_EQ(1, baseClass->GetDerivedClasses().size()) << "Unexpected derived class count. Derived classes are expected to already be loaded along with having loaded the schema";

    //derived classes are expected to be loaded when calling ECDbSchemaManager::GetDerivedClasses
    ASSERT_EQ(1, ecdb.Schemas().GetDerivedClasses(*baseClass).size()) << "Unexpected derived class count";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetMixin)
    {
    ECDbR ecdb = SetupECDb("getmixin.ecdb",
                           SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                                          <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                          <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
                                          <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                                            <ECEntityClass typeName="Model">
                                                    <ECProperty propertyName="Name" typeName="string" />
                                            </ECEntityClass>
                                            <ECEntityClass typeName="IIsGeometric" modifier="Abstract">
                                                <ECCustomAttributes>
                                                <IsMixin xmlns="CoreCustomAttributes.01.00">
                                                    <AppliesToEntityClass>Element</AppliesToEntityClass>
                                                </IsMixin>"
                                                </ECCustomAttributes>
                                                <ECProperty propertyName="Is2d" typeName="boolean"/>
                                                <ECProperty propertyName="SupportedGeometryType" typeName="string" />
                                            </ECEntityClass>
                                            <ECEntityClass typeName="Element">
                                                    <ECCustomAttributes>
                                                    <ClassMap xmlns="ECDbMap.02.00">
                                                            <MapStrategy>TablePerHierarchy</MapStrategy>
                                                    </ClassMap>
                                                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00"/>
                                                    </ECCustomAttributes>
                                                    <ECProperty propertyName="Code" typeName="string" />
                                                    <ECNavigationProperty propertyName="Model" relationshipName="ModelHasElements" direction="Backward"/>
                                            </ECEntityClass>
                                            <ECEntityClass typeName="Element2d">
                                                <BaseClass>Element</BaseClass>
                                                <BaseClass>IIsGeometric</BaseClass>
                                                <ECProperty propertyName="Origin2d" typeName="Point2d" />
                                            </ECEntityClass>
                                            <ECEntityClass typeName="Element3d">
                                                <BaseClass>Element</BaseClass>
                                                <BaseClass>IIsGeometric</BaseClass>
                                                <ECProperty propertyName="Origin3d" typeName="Point3d" />
                                            </ECEntityClass>
                                            <ECRelationshipClass typeName="ModelHasElements" strength="Referencing" modifier="None">
                                                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="Model">
                                                  <Class class="Model" />
                                                </Source>
                                                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="Element">
                                                    <Class class="Element" />
                                                </Target>
                                            </ECRelationshipClass>
                                            <ECRelationshipClass typeName="ModelHasGeometricElements" strength="Referencing" modifier="Sealed">
                                                <BaseClass>ModelHasElements</BaseClass>
                                                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="Model">
                                                  <Class class="Model" />
                                                </Source>
                                                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="GeometricElement">
                                                    <Class class="IIsGeometric" />
                                                </Target>
                                            </ECRelationshipClass>
                                          </ECSchema>)xml"));

    ASSERT_TRUE(ecdb.IsDbOpen());
    ASSERT_EQ(SUCCESS, ecdb.Schemas().CreateClassViewsInDb());
    ecdb.SaveChanges();
    ecdb.ClearECDbCache();

    //get in-memory schema without loading anything into it, so that we can track what gets loaded implicitly
    ECSchemaCP schema = ecdb.Schemas().GetSchema("TestSchema", false);
    ASSERT_EQ(0, schema->GetClassCount());
    //load mixin as first class and make sure the applies to class is loaded implicitly
    ECClassCP mixinRaw = ecdb.Schemas().GetClass("TestSchema", "IIsGeometric");
    ASSERT_TRUE(mixinRaw != nullptr && mixinRaw->IsEntityClass());
    ASSERT_EQ(4, schema->GetClassCount()) << "Mixin class is loaded and its applies to class and the nav prop's rel class plus constraint classes";
    ASSERT_TRUE(schema->GetClassCP("IIsGeometric") != nullptr);
    ASSERT_TRUE(schema->GetClassCP("Element") != nullptr) << "Applies to class of mixin is expected to be loaded";
    ASSERT_TRUE(schema->GetClassCP("ModelHasElements") != nullptr) << "Nav prop rel class is expected to be loaded";
    ASSERT_TRUE(schema->GetClassCP("ModelHasGeometricElements") == nullptr) << "Subclass of nav prop rel class is not expected to be loaded";
    ASSERT_TRUE(schema->GetClassCP("Model") != nullptr) << "Constraint class of nav prop rel class is expected to be loaded";

    ECEntityClassCP mixin = mixinRaw->GetEntityClassCP();
    ASSERT_TRUE(mixin->IsMixin());
    ECEntityClassCP appliesToClass = mixin->GetAppliesToClass();
    ASSERT_TRUE(appliesToClass != nullptr);
    ASSERT_STREQ("Element", appliesToClass->GetName().c_str());

    //now get relationship classes
    ecdb.ClearECDbCache();
    schema = ecdb.Schemas().GetSchema("TestSchema", false);
    ASSERT_EQ(0, schema->GetClassCount());

    ECClassCP baseRelRaw = ecdb.Schemas().GetClass("TestSchema", "ModelHasElements");
    ASSERT_TRUE(baseRelRaw != nullptr && baseRelRaw->IsRelationshipClass());
    ASSERT_EQ(3, schema->GetClassCount()) << "Rel base class is loaded and its constraint classes";
    ASSERT_TRUE(schema->GetClassCP("ModelHasElements") != nullptr);
    ASSERT_TRUE(schema->GetClassCP("Model") != nullptr);
    ASSERT_TRUE(schema->GetClassCP("Element") != nullptr);

    ECClassCP subRelRaw = ecdb.Schemas().GetClass("TestSchema", "ModelHasGeometricElements");
    ASSERT_TRUE(subRelRaw != nullptr && subRelRaw->IsRelationshipClass());
    ASSERT_EQ(5, schema->GetClassCount()) << "Rel base class is loaded and its constraint classes";
    ASSERT_TRUE(schema->GetClassCP("ModelHasGeometricElements") != nullptr);
    ASSERT_TRUE(schema->GetClassCP("IIsGeometric") != nullptr);

    //now get relationship sub class before anything else
    ecdb.ClearECDbCache();
    schema = ecdb.Schemas().GetSchema("TestSchema", false);
    ASSERT_EQ(0, schema->GetClassCount());

    subRelRaw = ecdb.Schemas().GetClass("TestSchema", "ModelHasGeometricElements");
    ASSERT_TRUE(subRelRaw != nullptr && subRelRaw->IsRelationshipClass());
    ASSERT_EQ(5, schema->GetClassCount()) << "Rel base class is loaded and its constraint classes";
    ASSERT_TRUE(schema->GetClassCP("ModelHasElements") != nullptr);
    ASSERT_TRUE(schema->GetClassCP("ModelHasGeometricElements") != nullptr);
    ASSERT_TRUE(schema->GetClassCP("Model") != nullptr);
    ASSERT_TRUE(schema->GetClassCP("IIsGeometric") != nullptr);
    ASSERT_TRUE(schema->GetClassCP("Element") != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetEnumeration)
    {
    ECDbCR ecdb = SetupECDb("getenumeration.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "<ECSchemaReference name='ECDbFileInfo' version='02.00.00' prefix='ecdbf' />"
                                     "  <ECEntityClass typeName='Foo' >"
                                     "    <ECProperty propertyName='Folder' typeName='ecdbf:StandardRootFolderType' />"
                                     "    <ECProperty propertyName='Homepage' typeName='string' extendedTypeName='URL' />"
                                     "    <ECArrayProperty propertyName='FavoriteFolders' typeName='ecdbf:StandardRootFolderType' minOccurs='0' maxOccurs='unbounded' />"
                                     "  </ECEntityClass>"
                                     "</ECSchema>"));

    ASSERT_TRUE(ecdb.IsDbOpen());
    Utf8String ecdbPath(ecdb.GetDbFileName());

    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly))) << "Could not open test file " << ecdbPath.c_str();

    ECEnumerationCP ecEnum = ecdb.Schemas().GetEnumeration("ECDbFileInfo", "StandardRootFolderType");
    ASSERT_TRUE(ecEnum != nullptr);
    ASSERT_EQ(4, ecEnum->GetEnumeratorCount());
    }

    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly))) << "Could not open test file " << ecdbPath.c_str();

    ECSchemaCP schema = ecdb.Schemas().GetSchema("ECDbFileInfo", false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(0, schema->GetEnumerationCount());
    ECClassCP classWithEnum = ecdb.Schemas().GetClass("ECDbFileInfo", "ExternalFileInfo");
    ASSERT_TRUE(classWithEnum != nullptr);

    ECPropertyCP prop = classWithEnum->GetPropertyP("RootFolder");
    ASSERT_TRUE(prop != nullptr);
    PrimitiveECPropertyCP primProp = prop->GetAsPrimitiveProperty();
    ASSERT_TRUE(primProp != nullptr);
    ECEnumerationCP ecEnum = primProp->GetEnumeration();
    ASSERT_TRUE(ecEnum != nullptr);
    ASSERT_EQ(4, ecEnum->GetEnumeratorCount());

    ecEnum = schema->GetEnumerationCP("StandardRootFolderType");
    ASSERT_TRUE(ecEnum != nullptr);
    ASSERT_EQ(4, ecEnum->GetEnumeratorCount());
    }

    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly))) << "Could not open test file " << ecdbPath.c_str();

    ECSchemaCP schema = ecdb.Schemas().GetSchema("ECDbFileInfo", false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(0, schema->GetEnumerationCount());
    ECClassCP classWithEnum = ecdb.Schemas().GetClass("TestSchema", "Foo");
    ASSERT_TRUE(classWithEnum != nullptr);

    ECPropertyCP prop = classWithEnum->GetPropertyP("Folder");
    ASSERT_TRUE(prop != nullptr);
    PrimitiveECPropertyCP primProp = prop->GetAsPrimitiveProperty();
    ASSERT_TRUE(primProp != nullptr);
    ECEnumerationCP ecEnum = primProp->GetEnumeration();
    ASSERT_TRUE(ecEnum != nullptr);
    ASSERT_EQ(4, ecEnum->GetEnumeratorCount());

    ecEnum = schema->GetEnumerationCP("StandardRootFolderType");
    ASSERT_TRUE(ecEnum != nullptr);
    ASSERT_EQ(4, ecEnum->GetEnumeratorCount());
    }

    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly))) << "Could not open test file " << ecdbPath.c_str();

    ECSchemaCP schema = ecdb.Schemas().GetSchema("ECDbFileInfo", true);
    ASSERT_TRUE(schema != nullptr);
    ECEnumerationCP ecEnum = schema->GetEnumerationCP("StandardRootFolderType");
    ASSERT_TRUE(ecEnum != nullptr);
    ASSERT_EQ(4, ecEnum->GetEnumeratorCount());
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetKindOfQuantity)
    {
    auto assertKoq = [] (KindOfQuantityCR actualKoq)
        {
        ASSERT_STREQ("My KindOfQuantity", actualKoq.GetDisplayLabel().c_str());
        ASSERT_STREQ("My KindOfQuantity", actualKoq.GetDescription().c_str());
        ASSERT_STREQ("CM(real)", actualKoq.GetPersistenceUnit().ToText(true).c_str());
        ASSERT_DOUBLE_EQ(.5, actualKoq.GetRelativeError());
        bvector<Formatting::FormatUnitSet> const& actualPresentationUnits = actualKoq.GetPresentationUnitList();
        ASSERT_EQ(2, actualPresentationUnits.size());
        ASSERT_STREQ("FT(real)", actualPresentationUnits[0].ToText(true).c_str());
        ASSERT_STREQ("IN(real)", actualPresentationUnits[1].ToText(true).c_str());
        };

    Utf8String ecdbPath;
    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                     <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                     <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                                                     displayLabel="My KindOfQuantity" persistenceUnit="CM" relativeError=".5"
                                                     presentationUnits="FT;IN" />
                                     </ECSchema>)xml"));

    testSchemas.push_back(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                     <ECSchema schemaName="Schema2" alias="s2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                     <ECSchemaReference name="Schema1" version="01.00.00" alias="s1" />
                                       <ECEntityClass typeName="Foo" >
                                         <ECProperty propertyName="Length" typeName="double" kindOfQuantity="s1:MyKindOfQuantity" />
                                         <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                                         <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="s1:MyKindOfQuantity"/>
                                         <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
                                       </ECEntityClass>
                                     </ECSchema>)xml"));

    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testSchemas[0], "getkindofquantity.ecdb");
    ASSERT_FALSE(asserted);

    AssertSchemaImport(asserted, ecdb, testSchemas[1]);
    ASSERT_FALSE(asserted);

    ecdbPath.assign(ecdb.GetDbFileName());
    }

    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly))) << "Could not open test file " << ecdbPath.c_str();

    KindOfQuantityCP koq = ecdb.Schemas().GetKindOfQuantity("Schema1", "MyKindOfQuantity");
    ASSERT_TRUE(koq != nullptr);
    assertKoq(*koq);
    }

    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly))) << "Could not open test file " << ecdbPath.c_str();

    ECSchemaCP schema = ecdb.Schemas().GetSchema("Schema1", false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(0, schema->GetKindOfQuantityCount());
    ECClassCP classWithKoq = ecdb.Schemas().GetClass("Schema2", "Foo");
    ASSERT_TRUE(classWithKoq != nullptr);
    ASSERT_EQ(1, schema->GetKindOfQuantityCount());
    KindOfQuantityCP koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_TRUE(koq != nullptr);
    assertKoq(*koq);

    ECPropertyCP prop = classWithKoq->GetPropertyP("Length");
    ASSERT_TRUE(prop != nullptr);
    PrimitiveECPropertyCP primProp = prop->GetAsPrimitiveProperty();
    ASSERT_TRUE(primProp != nullptr);
    koq = primProp->GetKindOfQuantity();
    ASSERT_TRUE(koq != nullptr);
    assertKoq(*koq);
    }

    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly))) << "Could not open test file " << ecdbPath.c_str();

    ECSchemaCP schema = ecdb.Schemas().GetSchema("Schema1", true);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(1, schema->GetKindOfQuantityCount());
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetPropertyWithExtendedType)
    {
    ECDbR ecdb = SetupECDb("propertywithextendedtype.ecdb", 
                           SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                                      "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                      "  <ECEntityClass typeName='Foo' >"
                                      "    <ECProperty propertyName='Name' typeName='string' />"
                                      "    <ECProperty propertyName='Homepage' typeName='string' extendedTypeName='URL' />"
                                      "    <ECArrayProperty propertyName='Addresses' typeName='string' minOccurs='0' maxOccurs='unbounded' />"
                                      "    <ECArrayProperty propertyName='Favorites' typeName='string' extendedTypeName='URL' minOccurs='0' maxOccurs='unbounded' />"
                                      "  </ECEntityClass>"
                                      "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassCP fooClass = ecdb.Schemas().GetClass("TestSchema", "Foo");
    ASSERT_TRUE(fooClass != nullptr);

    ECPropertyCP prop = fooClass->GetPropertyP("Name");
    ASSERT_TRUE(prop != nullptr && prop->GetAsPrimitiveProperty() != nullptr);
    ASSERT_FALSE(prop->HasExtendedType());

    prop = fooClass->GetPropertyP("Homepage");
    ASSERT_TRUE(prop != nullptr && prop->GetAsPrimitiveProperty() != nullptr);
    ASSERT_TRUE(prop->HasExtendedType());
    ASSERT_STREQ("URL", prop->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str());

    prop = fooClass->GetPropertyP("Addresses");
    ASSERT_TRUE(prop != nullptr && prop->GetAsPrimitiveArrayProperty() != nullptr);
    ASSERT_FALSE(prop->HasExtendedType());

    prop = fooClass->GetPropertyP("Favorites");
    ASSERT_TRUE(prop != nullptr && prop->GetAsPrimitiveArrayProperty() != nullptr);
    ASSERT_TRUE(prop->HasExtendedType());
    ASSERT_STREQ("URL", prop->GetAsPrimitiveArrayProperty()->GetExtendedTypeName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetRelationshipWithAbstractConstraintClass)
    {
    ECDbR ecdb = SetupECDb("relationshipwithabstractconstraintclass.ecdb",
                           SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                      <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                                        <ECEntityClass typeName="Model" >
                                          <ECProperty propertyName="Name" typeName="string" />
                                        </ECEntityClass>
                                        <ECEntityClass typeName="Element" modifier="Abstract" >
                                              <ECCustomAttributes>
                                                  <ClassMap xmlns="ECDbMap.02.00">
                                                      <MapStrategy>TablePerHierarchy</MapStrategy>
                                                  </ClassMap>
                                              </ECCustomAttributes>
                                          <ECProperty propertyName="Code" typeName="string" />
                                          <ECNavigationProperty propertyName="Model1" relationshipName="ModelHasElements" direction="Backward"/>
                                        </ECEntityClass>
                                        <ECEntityClass typeName="BaseElement" modifier="Abstract" >
                                            <BaseClass>Element</BaseClass>
                                            <ECNavigationProperty propertyName="Model2" relationshipName="ModelHasFooOrGooElements" direction="Backward"/>
                                        </ECEntityClass>
                                        <ECEntityClass typeName="FooElement" modifier="Sealed" >
                                            <BaseClass>BaseElement</BaseClass>
                                        </ECEntityClass>
                                        <ECEntityClass typeName="GooElement" modifier="Sealed" >
                                            <BaseClass>BaseElement</BaseClass>
                                        </ECEntityClass>
                                        <ECRelationshipClass typeName="ModelHasFooOrGooElements" modifier="Sealed" >
                                            <Source multiplicity="(0..1)" polymorphic="False" roleLabel="Model">
                                                <Class class ="Model" />
                                            </Source>
                                            <Target multiplicity="(0..*)" polymorphic="False" abstractConstraint="BaseElement" roleLabel="Foo or Goo Elements">
                                                <Class class ="FooElement" />
                                                <Class class ="GooElement" />
                                            </Target>
                                        </ECRelationshipClass>
                                        <ECRelationshipClass typeName="ModelHasElements" modifier="Sealed" >
                                            <Source multiplicity="(0..1)" polymorphic="False" roleLabel="Model">
                                                <Class class ="Model" />
                                            </Source>
                                            <Target multiplicity="(0..*)" polymorphic="False" roleLabel="Elements">
                                                <Class class ="Element" />
                                            </Target>
                                        </ECRelationshipClass>
                                      </ECSchema>)xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassCP ecclass = ecdb.Schemas().GetClass("TestSchema", "ModelHasFooOrGooElements");
    ASSERT_TRUE(ecclass != nullptr);
    ECRelationshipClassCP relWithAbstractConstraint = ecclass->GetRelationshipClassCP();
    ASSERT_TRUE(relWithAbstractConstraint != nullptr);
    
    ASSERT_EQ(ecdb.Schemas().GetClass("TestSchema", "Model"), relWithAbstractConstraint->GetSource().GetAbstractConstraint());
    ASSERT_EQ(ecdb.Schemas().GetClass("TestSchema", "BaseElement"), relWithAbstractConstraint->GetTarget().GetAbstractConstraint());

    ecclass = ecdb.Schemas().GetClass("TestSchema", "ModelHasElements");
    ASSERT_TRUE(ecclass != nullptr);
    ECRelationshipClassCP relWithoutAbstractConstraint = ecclass->GetRelationshipClassCP();
    ASSERT_TRUE(relWithoutAbstractConstraint != nullptr);
    ASSERT_EQ(ecdb.Schemas().GetClass("TestSchema", "Model"), relWithoutAbstractConstraint->GetSource().GetAbstractConstraint());
    ASSERT_EQ(ecdb.Schemas().GetClass("TestSchema", "Element"), relWithoutAbstractConstraint->GetTarget().GetAbstractConstraint());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, AddDuplicateECSchemaInCache)
    {
    ECDbCR ecdb = SetupECDb("ecschemamanagertest.ecdb");
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSchemaReadContextPtr context = nullptr;
    ECSchemaPtr schemaPtr = ReadECSchemaFromDisk(context, BeFileName(L"BaseSchemaA.01.00.ecschema.xml"));
    ASSERT_TRUE(schemaPtr != nullptr);
    context = nullptr;
    ECSchemaPtr schemaPtr1 = ReadECSchemaFromDisk(context, BeFileName(L"BaseSchemaA.01.00.ecschema.xml"));
    ASSERT_TRUE(schemaPtr1 != nullptr);

    ECSchemaCachePtr schemacache = ECSchemaCache::Create();
    ASSERT_EQ(ECObjectsStatus::Success, schemacache->AddSchema(*schemaPtr)) << "couldn't add schema to the cache" << schemaPtr->GetName().c_str();
    ASSERT_EQ(ECObjectsStatus::DuplicateSchema, schemacache->AddSchema(*schemaPtr1));
    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas(schemacache->GetSchemas())) << "could not import test ecschema.";

    ECClassCP ecclass = ecdb.Schemas().GetClass("BaseSchemaA", "Address");
    EXPECT_TRUE(ecclass != NULL) << "ecclass with the specified name does not exist";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, ImportDuplicateSchema)
    {
    ECDbR ecdb = SetupECDb("ecschemamanagertest.ecdb", BeFileName(L"BaseSchemaA.01.00.ecschema.xml"), 3);

    ECSchemaReadContextPtr context = nullptr;
    ECSchemaPtr schema = ReadECSchemaFromDisk(context, BeFileName(L"BaseSchemaA.01.00.ecschema.xml"));
    ASSERT_TRUE(schema != nullptr);

    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    schemaCache->AddSchema(*schema);

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas(schemaCache->GetSchemas()));

    ECClassCP ecclass = ecdb.Schemas().GetClass("BaseSchemaA", "Address");
    ASSERT_TRUE(ecclass != nullptr) << "Class with the specified name doesn't exist :- ecclass is empty";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, ImportMultipleSupplementalSchemas)
    {
    ECDbCR ecdb = SetupECDb("supplementalSchematest.ecdb",
                 SchemaItem({R"xml(<ECSchema schemaName="SchoolSchema" alias="SS" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA"/>
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                <ECEntityClass typeName="School" modifier="None">
                    <ECProperty propertyName="Code" typeName="string"/>
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECProperty propertyName="FoundationDate" typeName="DateTime" />
                </ECEntityClass>
                <ECEntityClass typeName="Course" modifier="None">
                    <ECProperty propertyName="LastMod1" typeName="DateTime"/>
                    <ECProperty propertyName="LastMod2" typeName="DateTime"/>
                    <ECNavigationProperty propertyName="School" relationshipName="SchoolHasCourses" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="SchoolHasCourses" modifier="Sealed" strength="embedding">
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                    </ECCustomAttributes>
                    <Source multiplicity="(0..1)" polymorphic="true" roleLabel="has">
                        <Class class="School"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="true" roleLabel="belongs to">
                        <Class class="Course"/>
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml",

            R"xml(<ECSchema schemaName="SchoolSchema_Supplemental_1" alias="SSS1" version="01.00" displayLabel="School Supplemental1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA"/>
                <ECCustomAttributes>
                    <SupplementalSchema xmlns="CoreCustomAttributes.01.00">
                        <PrimarySchemaReference>
                            <SchemaName>SchoolSchema</SchemaName>
                            <MajorVersion>1</MajorVersion>
                            <WriteVersion>0</WriteVersion>
                            <MinorVersion>0</MinorVersion>
                        </PrimarySchemaReference>
                        <Precedence>210</Precedence>
                        <Purpose>Test1</Purpose>
                    </SupplementalSchema>
                </ECCustomAttributes>
                <ECEntityClass typeName="School" modifier="None">
                    <ECProperty propertyName="FoundationDate" typeName="DateTime">
                    <ECCustomAttributes>
                        <DateTimeInfo xmlns="CoreCustomAttributes.01.00">
                            <DateTimeComponent>Date</DateTimeComponent>
                        </DateTimeInfo>
                    </ECCustomAttributes>
                    </ECProperty>
                </ECEntityClass>
                <ECEntityClass typeName="Course" modifier="None">
                    <ECCustomAttributes>
                        <ClassHasCurrentTimeStampProperty xmlns="CoreCustomAttributes.01.00">
                            <PropertyName>LastMod1</PropertyName>
                        </ClassHasCurrentTimeStampProperty>
                    </ECCustomAttributes>
                </ECEntityClass>
                </ECSchema>)xml",

            R"xml(<ECSchema schemaName="SchoolSchema_Supplemental_2" alias="SSS2" version="01.00" displayLabel="School Supplemental2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA"/>
                <ECCustomAttributes>
                    <SupplementalSchema xmlns="CoreCustomAttributes.01.00">
                        <PrimarySchemaReference>
                            <SchemaName>SchoolSchema</SchemaName>
                            <MajorVersion>1</MajorVersion>
                            <WriteVersion>0</WriteVersion>
                            <MinorVersion>0</MinorVersion>
                        </PrimarySchemaReference>
                        <Precedence>215</Precedence>
                        <Purpose>Test2</Purpose>
                    </SupplementalSchema>
                </ECCustomAttributes>
                <ECEntityClass typeName="Course" modifier="None">
                    <ECCustomAttributes>
                        <ClassHasCurrentTimeStampProperty xmlns="CoreCustomAttributes.01.00">
                            <PropertyName>LastMod2</PropertyName>
                        </ClassHasCurrentTimeStampProperty>
                    </ECCustomAttributes>
                </ECEntityClass>
                <ECRelationshipClass typeName="SchoolHasCourses" modifier="Sealed" strength="embedding">
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00">
                            <OnDeleteAction>Cascade</OnDeleteAction>
                        </ForeignKeyConstraint>
                    </ECCustomAttributes>
                </ECRelationshipClass>
                </ECSchema>)xml"}, true, ""));

    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassCP schoolClass = ecdb.Schemas().GetClass("SchoolSchema", "School");
    ASSERT_TRUE(schoolClass != nullptr);
    ECClassCP courseClass = ecdb.Schemas().GetClass("SchoolSchema", "Course");
    ASSERT_TRUE(courseClass != nullptr);
    ECClassCP relClass = ecdb.Schemas().GetClass("SchoolSchema", "SchoolHasCourses");
    ASSERT_TRUE(relClass != nullptr);

    int caCount = 0;
    for (IECInstancePtr ca : schoolClass->GetCustomAttributes(false))
        {
        caCount++;
        }
    ASSERT_EQ(0, caCount) << "ECClass School";

    ECPropertyCP foundationDateProp = schoolClass->GetPropertyP("FoundationDate");
    ASSERT_TRUE(foundationDateProp != nullptr);
    caCount = 0;
    for (IECInstancePtr ca : foundationDateProp->GetCustomAttributes(false))
        {
        caCount++;
        ASSERT_STREQ("DateTimeInfo", ca->GetClass().GetName().c_str()) << "ECProperty School.FoundationDate";
        ECValue v;
        ASSERT_EQ(ECObjectsStatus::Success, ca->GetValue(v, "DateTimeComponent")) << "ECProperty School.FoundationDate";
        ASSERT_STREQ("Date", v.GetUtf8CP()) << "ECProperty School.FoundationDate";
        }
    ASSERT_EQ(1, caCount) << "ECProperty School.FoundationDate";


    caCount = 0;
    for (IECInstancePtr ca : courseClass->GetCustomAttributes(false))
        {
        caCount++;
        ASSERT_STREQ("ClassHasCurrentTimeStampProperty", ca->GetClass().GetName().c_str()) << "ECClass Course";
        ECValue v;
        ASSERT_EQ(ECObjectsStatus::Success, ca->GetValue(v, "PropertyName")) << "ECClass Course";
        ASSERT_STREQ("LastMod2", v.GetUtf8CP()) << "ECClass Course";
        }
    ASSERT_EQ(1, caCount) << "ECClass Course";

    caCount = 0;
    for (IECInstancePtr ca : relClass->GetCustomAttributes(false))
        {
        caCount++;
        ASSERT_STREQ("ForeignKeyConstraint", ca->GetClass().GetName().c_str()) << "RelationshipECClass SchoolHasCourses";
        ECValue v;
        ASSERT_EQ(ECObjectsStatus::Success, ca->GetValue(v, "OnDeleteAction")) << "RelationshipECClass SchoolHasCourses";
        ASSERT_STREQ("Cascade", v.GetUtf8CP()) << "RelationshipECClass SchoolHasCourses";
        }

    ASSERT_EQ(1, caCount) << "RelationshipECClass SchoolHasCourses";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, TestGetClassResolver)
    {
    ECDbCR ecdb = SetupECDb("ecschemamanagertest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());
    ECClassCP ecClass = ecdb.Schemas().GetClass("ECSqlTest", "PSA");
    EXPECT_TRUE(ecClass != nullptr);
    ecClass = ecdb.Schemas().GetClass("ecsql", "PSA", SchemaLookupMode::ByAlias);
    EXPECT_TRUE(ecClass != nullptr);

    ecClass = ecdb.Schemas().GetClass("ECSqlTest", "PSA", SchemaLookupMode::AutoDetect);
    EXPECT_TRUE(ecClass != nullptr);

    ecClass = ecdb.Schemas().GetClass("ecsql", "PSA", SchemaLookupMode::AutoDetect);
    EXPECT_TRUE(ecClass != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
// A primary schema should be supplemented with the latest available supplemental schema
TEST_F(SchemaManagerTests, SupplementWithLatestCompatibleSupplementalSchema)
    {
    ECDbCR ecdb = SetupECDb("supplementalSchematest.ecdb");
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSchemaReadContextPtr context = nullptr;
    ECSchemaCachePtr schemacache = ECSchemaCache::Create();

    ECSchemaPtr schemaptr = ReadECSchemaFromDisk(context, BeFileName(L"BasicSchema.01.70.ecschema.xml"));
    ASSERT_TRUE(schemaptr != NULL);
    schemacache->AddSchema(*schemaptr);

    ECSchemaPtr supple = ReadECSchemaFromDisk(context, BeFileName(L"BasicSchema_Supplemental_Localization.01.90.ecschema.xml"));
    ASSERT_TRUE(supple != NULL);
    schemacache->AddSchema(*supple);

    supple = ReadECSchemaFromDisk(context, BeFileName(L"BasicSchema_Supplemental_Localization.01.60.ecschema.xml"));
    ASSERT_TRUE(supple != NULL);
    schemacache->AddSchema(*supple);

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas(schemacache->GetSchemas())) << "couldn't import the schema";
    ECSchemaCP basicSupplSchema = ecdb.Schemas().GetSchema("BasicSchema", true);
    ASSERT_TRUE(basicSupplSchema != NULL);

    ECClassCP ecclassBase = basicSupplSchema->GetClassCP("Base");
    ASSERT_TRUE(ecclassBase != NULL);
    //get custom attributes from base class (false)
    ECCustomAttributeInstanceIterable iterator1 = ecclassBase->GetCustomAttributes(false);
    int i = 0;
    for (IECInstancePtr instance : iterator1)
        {
        i++;
        }
    EXPECT_EQ(5, i) << "the number of custom attributes on the Class Base do not match the original";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
//supplemental schema whose targeted primary schema's major version is greater then the major version of of Schema to supplement.
TEST_F(SchemaManagerTests, SupplementSchemaWhoseTargetedPrimaryHasGreaterMajorVersion)
    {
    ECDbCR ecdb = SetupECDb("supplementalSchematest.ecdb");
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSchemaReadContextPtr context = nullptr;
    ECSchemaCachePtr schemacache = ECSchemaCache::Create();

    ECSchemaPtr schemaptr = ReadECSchemaFromDisk(context, BeFileName(L"BasicSchema.01.70.ecschema.xml"));
    ASSERT_TRUE(schemaptr != NULL);
    schemacache->AddSchema(*schemaptr);

    ECSchemaPtr supple = ReadECSchemaFromDisk(context, BeFileName(L"BasicSchema_Supplemental_Localization.01.90.ecschema.xml"));
    ASSERT_TRUE(supple != NULL);
    schemacache->AddSchema(*supple);

    supple = ReadECSchemaFromDisk(context, BeFileName(L"BasicSchema_Supplemental_Localization.02.10.ecschema.xml"));
    ASSERT_TRUE(supple != NULL);
    schemacache->AddSchema(*supple);

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas(schemacache->GetSchemas())) << "couldn't import the schema";
    ECSchemaCP basicSupplSchema = ecdb.Schemas().GetSchema("BasicSchema", true);
    ASSERT_TRUE(basicSupplSchema != NULL);

    ECClassCP ecclassBase = basicSupplSchema->GetClassCP("Base");
    ASSERT_TRUE(ecclassBase != NULL);
    //get custom attributes from base class (false)
    ECCustomAttributeInstanceIterable iterator1 = ecclassBase->GetCustomAttributes(false);
    int i = 0;
    for (IECInstancePtr instance : iterator1)
        {
        i++;
        }
    EXPECT_EQ(3, i) << "the number of custom attributes on the Class Base do not match the original";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan Khan                        12/15
//+---------------+---------------+---------------+---------------+---------------+------
std::map<Utf8String, std::set<Utf8String>> GetECViewNamesByPrefix(ECDbR ecdb)
    {
    Statement stmt;
    stmt.Prepare(ecdb, "select  substr (name, 1,  instr (name,'.') - 1), '[' || name || ']'  from sqlite_master where type = 'view' and instr (name,'.') and instr(sql, '--### ECCLASS VIEW')");
    std::map<Utf8String, std::set<Utf8String>> ecclassViews;
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        ecclassViews[stmt.GetValueText(0)].insert(stmt.GetValueText(1));
        }

    return ecclassViews;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan Khan                        12/15
//+---------------+---------------+---------------+---------------+---------------+------
void AssertECClassViews(bmap<Utf8String, bset<Utf8String>>& ecclassViewInfo, bool& validationFailed, ECDbCR ecdb)
    {
    ecclassViewInfo.clear();
    validationFailed = false;

    Statement stmt;
    stmt.Prepare(ecdb, "select  substr (name, 1,  instr (name,'.') - 1), '[' || name || ']'  from sqlite_master where type = 'view' and instr (name,'.') and instr(sql, '--### ECCLASS VIEW')");
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        Utf8CP viewName = stmt.GetValueText(1);
        ecclassViewInfo[stmt.GetValueText(0)].insert(viewName);

        Utf8String sql("SELECT * FROM ");
        sql.append(viewName);
        Statement validStmt;
        DbResult stat = validStmt.TryPrepare(ecdb, sql.c_str());
        ASSERT_EQ(BE_SQLITE_OK, stat) << "ECClassView " << viewName << " has invalid DDL: " << ecdb.GetLastError().c_str();
        if (BE_SQLITE_OK != stat)
            validationFailed = true;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan Khan                        12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, CreateECClassViews)
    {
    ECDbR ecdb = SetupECDb("createecclassviews.ecdb");
    ASSERT_TRUE(ecdb.IsDbOpen());

    ASSERT_EQ(SUCCESS, ecdb.Schemas().CreateClassViewsInDb());
    ecdb.SaveChanges();
    bmap<Utf8String, bset<Utf8String>> schemasWithECClassViews;
    bool validationFailed = false;
    AssertECClassViews(schemasWithECClassViews, validationFailed, ecdb);
    ASSERT_FALSE(validationFailed);

    ASSERT_EQ(2, schemasWithECClassViews.size()) << "Unexpected number of schemas with ECClassViews";
    ASSERT_EQ(4, schemasWithECClassViews["ecdbf"].size()) << "Unexpected number of ECClassViews";

    ECSchemaReadContextPtr context = nullptr;
    ECSchemaPtr schemaptr= ReadECSchemaFromDisk(context, BeFileName(L"StartupCompany.02.00.ecschema.xml"));

    ECSchemaCachePtr schemacache = ECSchemaCache::Create();
    schemacache->AddSchema(*schemaptr);

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas(schemacache->GetSchemas())) << "couldn't import the schema";
    ASSERT_EQ(SUCCESS, ecdb.Schemas().CreateClassViewsInDb());
    ecdb.SaveChanges();
    AssertECClassViews(schemasWithECClassViews, validationFailed, ecdb);
    ASSERT_FALSE(validationFailed);
    ASSERT_EQ(3, schemasWithECClassViews.size()) << "Unexpected number of schemas with ECClassViews";
    ASSERT_EQ(4, schemasWithECClassViews["ecdbf"].size()) << "Unexpected number of ECClassViews";
    ASSERT_EQ(30, schemasWithECClassViews["stco"].size()) << "Unexpected number of ECClassViews";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                   12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, CreateECClassViewsForSubsetOfClasses)
    {
    ECDbR ecdb = SetupECDb("createecclassviewspartially.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId FROM meta.ECClassDef WHERE Name IN ('FileInfo', 'FileInfoOwnership', 'AAA','Cubicle', 'RelationWithLinkTableMapping')"));
    bvector<ECClassId> classIds;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ECClassId id = stmt.GetValueId<ECClassId>(0);
        ASSERT_TRUE(id.IsValid());
        classIds.push_back(id);
        }

    ASSERT_EQ(SUCCESS, ecdb.Schemas().CreateClassViewsInDb(classIds));
    ecdb.SaveChanges();

    bmap<Utf8String, bset<Utf8String>> schemasWithECClassViews;
    bool validationFailed = false;
    AssertECClassViews(schemasWithECClassViews, validationFailed, ecdb);
    ASSERT_FALSE(validationFailed);

    ASSERT_EQ(2, schemasWithECClassViews.size()) << "Unexpected number of schemas with ECClassViews";
    auto it = schemasWithECClassViews.find("ecdbf");
    ASSERT_TRUE(it != schemasWithECClassViews.end());
    bset<Utf8String> const& ecdbfViews = it->second;
    ASSERT_EQ(2, ecdbfViews.size());
    ASSERT_TRUE(ecdbfViews.find(Utf8String("[ecdbf.FileInfo]")) != ecdbfViews.end());
    ASSERT_TRUE(ecdbfViews.find(Utf8String("[ecdbf.FileInfoOwnership]")) != ecdbfViews.end());

    it = schemasWithECClassViews.find("stco");
    ASSERT_TRUE(it != schemasWithECClassViews.end());
    bset<Utf8String> const& stcoViews = it->second;
    ASSERT_EQ(3, stcoViews.size());
    ASSERT_TRUE(stcoViews.find(Utf8String("[stco.AAA]")) != stcoViews.end());
    ASSERT_TRUE(stcoViews.find(Utf8String("[stco.Cubicle]")) != stcoViews.end());
    ASSERT_TRUE(stcoViews.find(Utf8String("[stco.RelationWithLinkTableMapping]")) != stcoViews.end());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                   01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, CreateECClassViews_SharedColumns)
    {
    ECDbR ecdb = SetupECDb("createecclassviewssharedcols.ecdb",
                      SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
            "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
            "  <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' />"
            "  <ECEntityClass typeName='Model' >"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.02.00'>"
            "                <MapStrategy>TablePerHierarchy</MapStrategy>"
            "            </ClassMap>"
            "            <ShareColumns xmlns='ECDbMap.02.00'>"
            "                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
            "            </ShareColumns>"
            "        </ECCustomAttributes>"
            "    <ECProperty propertyName='BoolProp' typeName='Boolean' />"
            "    <ECProperty propertyName='StringProp' typeName='string' />"
            "    <ECProperty propertyName='DateTimeProp' typeName='dateTime' />"
            "    <ECProperty propertyName='BlobProp' typeName='binary' />"
            "    <ECProperty propertyName='BoolProp_Overflow' typeName='Boolean' />"
            "    <ECProperty propertyName='GeomProp_Overflow' typeName='Bentley.Geometry.Common.IGeometry' />"
            "    <ECProperty propertyName='BlobProp_Overflow' typeName='binary' />"
            "    <ECProperty propertyName='DateTimeProp_Overflow' typeName='dateTime' />"
            "  </ECEntityClass>"
            "  <ECEntityClass typeName='Element' modifier='Abstract' >"
            "        <ECCustomAttributes>"
            "            <ClassMap xmlns='ECDbMap.02.00'>"
            "                <MapStrategy>TablePerHierarchy</MapStrategy>"
            "            </ClassMap>"
            "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
            "        </ECCustomAttributes>"
            "    <ECProperty propertyName='Code' typeName='string' />"
            "  </ECEntityClass>"
            "  <ECEntityClass typeName='SubElementSharedCols' modifier='Sealed' >"
            "        <ECCustomAttributes>"
            "            <ShareColumns xmlns='ECDbMap.02.00'>"
            "                <MaxSharedColumnsBeforeOverflow>5</MaxSharedColumnsBeforeOverflow>"
            "            </ShareColumns>"
            "        </ECCustomAttributes>"
            "      <BaseClass>Element</BaseClass>"
            "    <ECProperty propertyName='BoolProp' typeName='Boolean' />"
            "    <ECProperty propertyName='StringProp' typeName='string' />"
            "    <ECProperty propertyName='DateTimeProp' typeName='dateTime' />"
            "    <ECProperty propertyName='BlobProp' typeName='binary' />"
            "    <ECProperty propertyName='BoolProp_Overflow' typeName='Boolean' />"
            "    <ECProperty propertyName='GeomProp_Overflow' typeName='Bentley.Geometry.Common.IGeometry' />"
            "    <ECProperty propertyName='BlobProp_Overflow' typeName='binary' />"
            "    <ECProperty propertyName='DateTimeProp_Overflow' typeName='dateTime' />"
            "  </ECEntityClass>"
            "  <ECEntityClass typeName='SubElementNoSharedCols' modifier='Sealed' >"
            "      <BaseClass>Element</BaseClass>"
            "    <ECProperty propertyName='PropA' typeName='dateTime' />"
            "    <ECProperty propertyName='PropB' typeName='binary' />"
            "    <ECProperty propertyName='PropC' typeName='Boolean' />"
            "  </ECEntityClass>"
            "</ECSchema>"), 3);
    ASSERT_TRUE(ecdb.IsDbOpen());

    ASSERT_EQ(SUCCESS, ecdb.Schemas().CreateClassViewsInDb());
    bmap<Utf8String, bset<Utf8String>> classViewInfo;
    bool validationFailed = false;
    AssertECClassViews(classViewInfo, validationFailed, ecdb);
    ASSERT_FALSE(validationFailed);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                   12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, CreateECClassViewsForInvalidClasses)
    {
    ECDbCR ecdb = SetupECDb("createecclassviewsforinvalidclasses.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId FROM meta.ECClassDef WHERE Name IN ('AnglesStruct', 'ClassMap', 'AClassThatDoesNotGetMappedToDb')"));
    bvector<ECClassId> classIds;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ECClassId id = stmt.GetValueId<ECClassId>(0);
        ASSERT_TRUE(id.IsValid());
        classIds.push_back(id);
        }

    ASSERT_EQ(ERROR, ecdb.Schemas().CreateClassViewsInDb(classIds));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad.Hassan                   12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, CreateECClassViewsForCombinationofValidInvalidClasses)
    {
    ECDbR ecdb = SetupECDb("createecclassviewsforvalidinvalidclasses.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId FROM meta.ECClassDef WHERE Name IN ('AAA', 'AnglesStruct', 'AClassThatDoesNotGetMappedToDb')"));
    bvector<ECClassId> classIds;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ECClassId id = stmt.GetValueId<ECClassId>(0);
        ASSERT_TRUE(id.IsValid());
        classIds.push_back(id);
        }

    ASSERT_EQ(ERROR, ecdb.Schemas().CreateClassViewsInDb(classIds));
    ecdb.SaveChanges();

    // Class view will be created for the provided list of ECClassIds until it get the first invalid one
    // so ClassView only for class "AAA" will exist.
    std::map<Utf8String, std::set<Utf8String>> schemasWithECClassViews = GetECViewNamesByPrefix(ecdb);
    ASSERT_EQ(1, schemasWithECClassViews.size()) << "Unexpected number of schemas with ECClassViews";

    auto it = schemasWithECClassViews.find("stco");
    ASSERT_TRUE(it != schemasWithECClassViews.end());
    std::set<Utf8String> const& stcoViews = it->second;
    ASSERT_EQ(1, stcoViews.size());
    ASSERT_TRUE(stcoViews.find(Utf8String("[stco.AAA]")) != stcoViews.end());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
//supplemental schema whose Targeted primary schema's minor version is greater then minor Version of schema to supplement.
TEST_F(SchemaManagerTests, SupplementSchemaWhoseTargetedPrimaryHasGreaterMinorVersion)
    {
    ECDbCR testecdb = SetupECDb("supplementalSchematest.ecdb");
    ASSERT_TRUE(testecdb.IsDbOpen());

    ECSchemaReadContextPtr context = nullptr;
    ECSchemaCachePtr schemacache = ECSchemaCache::Create();

    ECSchemaPtr schemaptr = ReadECSchemaFromDisk(context, BeFileName(L"BasicSchema.01.69.ecschema.xml"));
    ASSERT_TRUE(schemaptr != nullptr);
    schemacache->AddSchema(*schemaptr);

    ECSchemaPtr supple = ReadECSchemaFromDisk(context, BeFileName(L"BasicSchema_Supplemental_Localization.01.69.ecschema.xml"));
    ASSERT_TRUE(supple != nullptr);
    schemacache->AddSchema(*supple);

    //With new supplementation Behaviour, this one will not be ignored though it is not targeting the primary schema's exact version.
    supple = ReadECSchemaFromDisk(context, BeFileName(L"BasicSchema_Supplemental_Localization.01.90.ecschema.xml"));
    ASSERT_TRUE(supple != nullptr);
    schemacache->AddSchema(*supple);

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportSchemas(schemacache->GetSchemas())) << "couldn't import the schema";
    ECSchemaCP basicSupplSchema = testecdb.Schemas().GetSchema("BasicSchema", true);
    ASSERT_TRUE(basicSupplSchema != NULL);

    ECClassCP ecclassBase = basicSupplSchema->GetClassCP("Base");
    ASSERT_TRUE(ecclassBase != NULL);
    //get custom attributes from base class (false)
    int i = 0;
    for (IECInstancePtr instance : ecclassBase->GetCustomAttributes(false))
        {
        i++;
        }
    ASSERT_EQ(4, i) << "the number of custom attributes on the Class Base do not match the original";
    }

//---------------------------------------------------------------------------------------
//                                               Krischan.Eberle                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, ImportSchemaWithSubclassesToBaseClassInExistingSchema)
    {
    auto setup = [] (ECInstanceKey& activityKey, ECDbCR ecdb)
        {
        Utf8CP baseSchemaXml =
            "<ECSchema schemaName='Planning' nameSpacePrefix='p' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
            "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
            "  <ECEntityClass typeName='Element'>"
            "    <ECCustomAttributes>"
            "        <ClassMap xmlns='ECDbMap.02.00'>"
            "                <MapStrategy>TablePerHierarchy</MapStrategy>"
            "        </ClassMap>"
            "    </ECCustomAttributes>"
            "    <ECProperty propertyName='Code' typeName='string' />"
            "  </ECEntityClass>"
            "  <ECEntityClass typeName='Activity'>"
            "    <BaseClass>Element</BaseClass>"
            "    <ECProperty propertyName='PlanId' typeName='long' />"
            "    <ECProperty propertyName='OutlineIndex' typeName='int' />"
            "  </ECEntityClass>"
            "</ECSchema>";

        ECSchemaReadContextPtr context1 = ECSchemaReadContext::CreateContext();
        context1->AddSchemaLocater(ecdb.GetSchemaLocater());
        ECSchemaPtr schema1 = nullptr;
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema1, baseSchemaXml, *context1));
        ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas(context1->GetCache().GetSchemas()));

        Utf8CP secondSchemaXml =
            "<ECSchema schemaName='Construction' nameSpacePrefix='c' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
            "  <ECSchemaReference name='Planning' version='01.00' prefix='p' />"
            "  <ECEntityClass typeName='Activity'>"
            "    <BaseClass>p:Activity</BaseClass>"
            "    <ECProperty propertyName='Name' typeName='string' />"
            "  </ECEntityClass>"
            "</ECSchema>";

        ECSchemaReadContextPtr context2 = ECSchemaReadContext::CreateContext();
        context2->AddSchemaLocater(ecdb.GetSchemaLocater());
        ECSchemaPtr schema2 = nullptr;
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema2, secondSchemaXml, *context2));
        ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas(context2->GetCache().GetSchemas()));

        ECInstanceKey newKey;
        ECSqlStatement insStmt;
        ASSERT_EQ(ECSqlStatus::Success, insStmt.Prepare(ecdb, "INSERT INTO c.Activity (Code, Name) VALUES ('ConstructionActivity-1', 'Do something')"));
        ASSERT_EQ(BE_SQLITE_DONE, insStmt.Step(newKey));

        ECSqlStatement updStmt;
        ASSERT_EQ(ECSqlStatus::Success, updStmt.Prepare(ecdb, "UPDATE p.Activity SET PlanId=100, OutlineIndex=100 WHERE ECInstanceId=?"));
        updStmt.BindId(1, newKey.GetInstanceId());
        ASSERT_EQ(BE_SQLITE_DONE, updStmt.Step());

        activityKey = newKey;
        };

    ECDbR ecdb = SetupECDb("importschemawithsubclassestoexistingschema1.ecdb");

    ECInstanceKey activityKey;
    setup(activityKey, ecdb);
    ASSERT_TRUE(activityKey.IsValid());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT PlanId, OutlineIndex FROM p.Activity WHERE ECInstanceId=?"));
    stmt.BindId(1, activityKey.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_FALSE(stmt.IsValueNull(0)) << "This should start to fail if ECDb still caching horizontal paratition even after import a second schema";
    ASSERT_FALSE(stmt.IsValueNull(1)) << "This should start to fail if ECDb still caching horizontal paratition even after import a second schema";

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
   
    }


//-------------------------------------------------------------------------------------- -
// @bsimethod                                     Krischan.Eberle           11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, IGeometryTypes)
    {
    SchemaItem testSchema("<?xml version='1.0' encoding='utf-8'?>"
                          "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "    <ECEntityClass typeName='Foo' >"
                          "        <ECProperty propertyName='g1' typeName='Bentley.Geometry.Common.IGeometry' />"
                          "        <ECProperty propertyName='g2' typeName='Bentley.GeometryNET.Common.IGeometry' />"
                          "        <ECProperty propertyName='g3' typeName='Bentley.GeometryNET.Common.ICoordinate' />"
                          "    </ECEntityClass>"
                          "</ECSchema>");

    ECDb& ecdb = SetupECDb("ecdbgeometrytypes.ecdb", testSchema);
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassCP cl = ecdb.Schemas().GetClass("TestSchema", "Foo");
    ASSERT_TRUE(cl != nullptr);
    ASSERT_EQ(PRIMITIVETYPE_IGeometry, cl->GetPropertyP("g1")->GetAsPrimitiveProperty()->GetType());
    ASSERT_EQ(PRIMITIVETYPE_IGeometry, cl->GetPropertyP("g2")->GetAsPrimitiveProperty()->GetType());
    ASSERT_EQ(PRIMITIVETYPE_IGeometry, cl->GetPropertyP("g3")->GetAsPrimitiveProperty()->GetType());

    IGeometryPtr g1 = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    IGeometryPtr g2 = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(1.0, 1.0, 1.0, 2.0, 2.0, 2.0)));
    IGeometryPtr g3 = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(2.0, 2.0, 2.0, 3.0, 3.0, 3.0)));

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Foo(g1,g2,g3) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(1, *g1));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(2, *g2));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(3, *g3));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ecdb.SaveChanges();
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT g1,g2,g3 FROM ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_TRUE(stmt.GetValueGeometry(0)->IsSameStructureAndGeometry(*g1));
    ASSERT_TRUE(stmt.GetValueGeometry(1)->IsSameStructureAndGeometry(*g2));
    ASSERT_TRUE(stmt.GetValueGeometry(2)->IsSameStructureAndGeometry(*g3));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad.Hassan                 02/16
//+---------------+---------------+---------------+---------------+---------------+-----
TEST_F(SchemaManagerTests, EnforceECEnumeration)
    {
    ECDbR ecdb = SetupECDb("propertywithEnumerationType.ecdb",
                           SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                                      "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                      " <ECEnumeration typeName='NonStrictEnum' backingTypeName='int' isStrict='False'>"
                                      "   <ECEnumerator value = '0' displayLabel = 'txt' />"
                                      "   <ECEnumerator value = '1' displayLabel = 'bat' />"
                                      " </ECEnumeration>"
                                      " <ECEnumeration typeName='StrictEnum' backingTypeName='int' isStrict='True'>"
                                      "   <ECEnumerator value = '0' displayLabel = 'txt' />"
                                      "   <ECEnumerator value = '1' displayLabel = 'bat' />"
                                      " </ECEnumeration>"
                                      "  <ECEntityClass typeName='File' >"
                                      "    <ECProperty propertyName='Type' typeName='NonStrictEnum' />"
                                      "  </ECEntityClass>"
                                      "  <ECEntityClass typeName='Folder' >"
                                      "    <ECProperty propertyName='Type' typeName='StrictEnum' />"
                                      "  </ECEntityClass>"
                                      "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    //non strict enum Insert tests
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.File(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 2));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.File(Type) VALUES(0)"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.File(Type) VALUES(2)"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    //non strict enum Update tests
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "UPDATE ts.File SET Type=1 WHERE Type=0"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "UPDATE ts.File SET Type=3 WHERE Type=1"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "UPDATE ts.File SET Type=? WHERE Type=?"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(2, 3));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 1));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(2, 2));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    //strict enum Insert tests
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.Folder(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 2));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step()) << "ECDb does not enforce strict enums, so inserting a wrong value is expected to not fail.";
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.Folder(Type) VALUES(1)"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "INSERT INTO ts.Folder(Type) VALUES(2)"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step()) << "ECDb does not enforce strict enums, so inserting a wrong value is expected to not fail.";
    statement.Finalize();

    //strict enum Update tests
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "UPDATE ts.Folder SET Type=1 WHERE Type=0"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "UPDATE ts.Folder SET Type=2 WHERE Type=1"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(ecdb, "UPDATE ts.Folder SET Type=? WHERE Type=?"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(2, 1));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 2));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(2, 0));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();
    }

//---------------------------------------------------------------------------------------
//                                               Krischan.Eberle                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, DuplicateInMemorySchemaTest)
    {
    Utf8CP stdXml = 
        "<?xml version='1.0' encoding='utf-8' ?>"
        "<ECSchema schemaName='std' nameSpacePrefix='std' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='Foo' >"
        "       <ECProperty propertyName='Test' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    Utf8CP usrXml =
        "<?xml version='1.0' encoding='utf-8' ?>"
        "<ECSchema schemaName='usr' nameSpacePrefix='usr' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECSchemaReference name='std' version='01.00.00' prefix='std'/>"
        "   <ECEntityClass typeName='FooDerive' >"
        "       <BaseClass>std:Foo</BaseClass>"
        "       <ECProperty propertyName='Test1' typeName='string' />"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='Goo' >"
        "       <ECProperty propertyName='Test' typeName='string' />"
        "   </ECEntityClass>"
        "</ECSchema>";

    ECSchemaPtr std, usr;
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString(std, stdXml, *readContext));


    ECDb& ecdb = SetupECDb("duplicateInMemorySchemaTest.ecdb");
    ASSERT_TRUE(ecdb.IsDbOpen());
    ASSERT_EQ(BentleyStatus::SUCCESS, ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas()));

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(usr, usrXml, *readContext));
    ASSERT_EQ(BentleyStatus::SUCCESS, ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas())) << "Failed because locater was not added for schemas that already exist in ECDb";
    }

END_ECDBUNITTESTS_NAMESPACE

