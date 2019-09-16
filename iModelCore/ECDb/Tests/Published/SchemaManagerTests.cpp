/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <set>

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE


struct SchemaManagerTests : ECDbTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, ImportDifferentInMemorySchemaVersions)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ecdbschemamanagertests.ecdb"));

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

    importSchema(m_ecdb, ECVersion::V2_0, false);
    importSchema(m_ecdb, ECVersion::V3_0, false);
    importSchema(m_ecdb, ECVersion::V3_1, false);
    importSchema(m_ecdb, ECVersion::V3_2, true);
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
        
        ASSERT_EQ(SUCCESS, TestHelper(ecdb).ImportSchema(SchemaItem(ecschemaXml))) << "SchemaImport into unrestricted ECDb failed unexpectedly for: " << ecschemaXml;
        ecdb.CloseDb();

        RestrictedSchemaImportECDb restrictedECDb(true);
        ASSERT_EQ(BE_SQLITE_OK, CloneECDb(restrictedECDb, (Utf8String(seedFilePath.GetFileNameWithoutExtension()) + "_restricted.ecdb").c_str(), seedFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));

        ASSERT_EQ(ERROR, TestHelper(restrictedECDb).ImportSchema(SchemaItem(ecschemaXml))) << "SchemaImport into restricted ECDb. Expected to fail for: " << ecschemaXml;
        };

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("importtokentests.ecdb"));
    BeFileName seedFileName(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();
    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='TestSchema' displayLabel='Test Schema' alias='ts1' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECEntityClass typeName='Foo' displayLabel='Spatial Element'>"
                 "        <ECProperty propertyName='Pet' typeName='string'/>"
                 "        <ECProperty propertyName='LastMod' typeName='DateTime'/>"
                 "    </ECEntityClass>"
                 "</ECSchema>", seedFileName);


    ASSERT_EQ(SUCCESS, SetupECDb("importtokentests.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
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
                    </ECSchema>)xml")));
    seedFileName.AssignUtf8(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();
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
TEST_F(SchemaManagerTests, DisallowMajorSchemaVersionImport)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("DisallowMajorSchemaVersionImport.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                                                  "<ECSchema schemaName='BaseSchema' alias='base' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                  "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbMap' />"
                                                "    <ECCustomAttributeClass typeName='MyBaseCA'>"
                                                "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                                                "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                                                "    </ECCustomAttributeClass>"
                                                "    <ECCustomAttributeClass typeName='MySubCA'>"
                                                "        <BaseClass>MyBaseCA</BaseClass>"
                                                "        <ECProperty propertyName='SubProp1' typeName='long' />"
                                                "        <ECProperty propertyName='SubProp2' typeName='string' />"
                                                "    </ECCustomAttributeClass>"
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
                                                  "</ECSchema>")));

    auto import = [this] (SchemaItem const& schema, SchemaManager::SchemaImportOptions options)
        {
        BentleyStatus stat = ERROR;
        Savepoint sp(m_ecdb, "");
        {
        stat = GetHelper().ImportSchema(schema, options);
        }
        sp.Cancel();
        return stat;
        };

    SchemaItem schema("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='BaseSchema' alias='base' version='1.0.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbMap' />"
                      "    <ECCustomAttributeClass typeName='MyBaseCA'>"
                      "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                      "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                      "    </ECCustomAttributeClass>"
                      "    <ECCustomAttributeClass typeName='MySubCA'>"
                      "        <BaseClass>MyBaseCA</BaseClass>"
                      "        <ECProperty propertyName='SubProp1' typeName='long' />"
                      "        <ECProperty propertyName='SubProp2' typeName='string' />"
                      "    </ECCustomAttributeClass>"
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
                "    <ECCustomAttributeClass typeName='MySubCA2'>"
                "        <BaseClass>MyBaseCA</BaseClass>"
                "        <ECProperty propertyName='SubProp3' typeName='long' />"
                "        <ECProperty propertyName='SubProp4' typeName='string' />"
                "    </ECCustomAttributeClass>"
                      "</ECSchema>");
    
    ASSERT_EQ(SUCCESS, import(schema, SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "adding subclasses";
    ASSERT_EQ(SUCCESS, import(schema, SchemaManager::SchemaImportOptions::None)) << "adding subclasses";

    schema = SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='BaseSchema' alias='base' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbMap' />"
                        "    <ECCustomAttributeClass typeName='MyBaseCA'>"
                        "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                        "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                        "    </ECCustomAttributeClass>"
                        "    <ECCustomAttributeClass typeName='MySubCA'>"
                        "        <BaseClass>MyBaseCA</BaseClass>"
                        "        <ECProperty propertyName='SubProp1' typeName='long' />"
                        "        <ECProperty propertyName='SubProp2' typeName='string' />"
                        "    </ECCustomAttributeClass>"
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
                 "</ECSchema>");

    ASSERT_EQ(ERROR, import(schema, SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "deleting a subclass with TPH (expected to succeed as no table is deleted)";
    ASSERT_EQ(SUCCESS, import(schema, SchemaManager::SchemaImportOptions::None)) << "deleting a subclass with TPH (expected to succeed as no table is deleted)";

    schema = SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
    "<ECSchema schemaName='BaseSchema' alias='base' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
    "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbMap' />"
                        "    <ECCustomAttributeClass typeName='MyBaseCA'>"
                        "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                        "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                        "    </ECCustomAttributeClass>"
                        "    <ECCustomAttributeClass typeName='MySubCA'>"
                        "        <BaseClass>MyBaseCA</BaseClass>"
                        "        <ECProperty propertyName='SubProp1' typeName='long' />"
                        "        <ECProperty propertyName='SubProp2' typeName='string' />"
                        "    </ECCustomAttributeClass>"
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
    "</ECSchema>");

    ASSERT_EQ(ERROR, import(schema, SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "deleting a subclass with TPH which is root of joined table (expected to fail as joined table is deleted)";
    ASSERT_EQ(SUCCESS, import(schema, SchemaManager::SchemaImportOptions::None)) << "deleting a subclass with TPH which is root of joined table";


    schema = SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='BaseSchema' alias='base' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbMap' />"
                        "    <ECCustomAttributeClass typeName='MyBaseCA'>"
                        "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                        "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                        "    </ECCustomAttributeClass>"
                        "    <ECCustomAttributeClass typeName='MySubCA'>"
                        "        <BaseClass>MyBaseCA</BaseClass>"
                        "        <ECProperty propertyName='SubProp1' typeName='long' />"
                        "        <ECProperty propertyName='SubProp2' typeName='string' />"
                        "    </ECCustomAttributeClass>"
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
                 "</ECSchema>");

    ASSERT_EQ(ERROR, import(schema, SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "deleting a subclass without TPH (expected to fail as table is deleted)";
    ASSERT_EQ(SUCCESS, import(schema, SchemaManager::SchemaImportOptions::None)) << "deleting a subclass without TPH";

    schema = SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                        "<ECSchema schemaName='BaseSchema' alias='base' version='2.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                        "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbMap' />"
                        "    <ECCustomAttributeClass typeName='MyBaseCA'>"
                        "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                        "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                        "    </ECCustomAttributeClass>"
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
                        "</ECSchema>");

    ASSERT_EQ(ERROR, import(schema, SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "deleting a sub CA class";
    ASSERT_EQ(SUCCESS, import(schema, SchemaManager::SchemaImportOptions::None)) << "deleting a sub CA class";

    schema = SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
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
                        "</ECSchema>");

    ASSERT_EQ(ERROR, import(schema, SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade)) << "deleting base CA and sub CA class";
    ASSERT_EQ(ERROR, import(schema, SchemaManager::SchemaImportOptions::None)) << "deleting base CA and sub CA class at once doesn't work right now";
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, ImportWithLocalizationSchemas)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("invariantculturetest.ecdb"));

    ASSERT_EQ(SUCCESS, ImportSchemas({SchemaItem(
        "<?xml version='1.0' encoding='utf-8' ?>"
        "<ECSchema schemaName='TestSchema' displayLabel='Test Schema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.2'>"
        "    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />"
        "    <ECEnumeration typeName='Animal' displayLabel='Animal' backingTypeName='int' isStrict='True'>"
        "        <ECEnumerator name='Dog' value='1' />"
        "        <ECEnumerator name='Cat' value='2' />"
        "    </ECEnumeration>"
        "    <ECEntityClass typeName='SpatialElement' displayLabel='Spatial Element'>"
        "        <ECProperty propertyName='Pet' typeName='Animal' displayLabel='Pet Type' />"
        "        <ECProperty propertyName='LastMod' typeName='DateTime' displayLabel='Last Modified Date'>"
        "          <ECCustomAttributes>"
        "            <DateTimeInfo xmlns='CoreCustomAttributes.01.00.00'>"
        "                <DateTimeKind>Unspecified</DateTimeKind>"
        "            </DateTimeInfo>"
        "          </ECCustomAttributes>"
        "        </ECProperty>"
        "    </ECEntityClass>"
        "</ECSchema>"),
        SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
        "<ECSchema schemaName='TestSchema_Supplemental_Localization' alias='loc_de_DE' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "    <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA' />"
        "    <ECSchemaReference name='SchemaLocalizationCustomAttributes' version='01.00.00' alias='LocCA' />"
        "<ECCustomAttributes>"
        "<SupplementalSchema xmlns='CoreCustomAttributes.01.00.00'>"
        "   <PrimarySchemaReference>"
        "       <SchemaName>House</SchemaName>"
        "       <MajorVersion>1</MajorVersion>"
        "       <WriteVersion>0</WriteVersion>"
        "       <MinorVersion>0</MinorVersion>"
        "   </PrimarySchemaReference>"
        "   <Precedence>9900</Precedence>"
        "   <Purpose>Localization</Purpose>"
        "</SupplementalSchema>"
        "<LocalizationSpecification xmlns='SchemaLocalizationCustomAttributes.01.00.00'>"
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
        "</ECSchema>")}));

    SchemaManager const& schemas = m_ecdb.Schemas();
    ECSchemaCP testSchema = schemas.GetSchema("TestSchema", false);
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_STRCASEEQ("Test Schema", testSchema->GetDisplayLabel().c_str());

    ECEnumerationCP animalEnum = schemas.GetEnumeration("TestSchema", "Animal");
    ASSERT_TRUE(animalEnum != nullptr);
    ASSERT_STRCASEEQ("Animal", animalEnum->GetDisplayLabel().c_str());
    ASSERT_EQ(2, (int) animalEnum->GetEnumeratorCount());
    for (ECEnumerator const* enumValue : animalEnum->GetEnumerators())
        {
        if (enumValue->GetName().EqualsIAscii("Dog"))
            ASSERT_STRCASEEQ("Dog", enumValue->GetDisplayLabel().c_str());
        else if (enumValue->GetName().EqualsIAscii("Cat"))
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
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbschemamanagertests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    BeFileName testFilePath(m_ecdb.GetDbFileName());

    const int expectedClassCount = m_ecdb.Schemas().GetSchema("ECSqlTest")->GetClassCount();
    m_ecdb.CloseDb();

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
    ASSERT_EQ(SUCCESS, SetupECDb("schemamanagercasingtests.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("ECDBFILEinfo");
    ASSERT_TRUE(schema != nullptr && schema->GetName().EqualsI("ECDbFileInfo"));

    schema = m_ecdb.Schemas().GetSchema("ecsqltest");
    ASSERT_TRUE(schema != nullptr && schema->GetName().EqualsI("ECSqlTest"));

    ECClassCP ecclass = nullptr;
    ecclass = m_ecdb.Schemas().GetClass("ecsqltest", "P");
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = m_ecdb.Schemas().GetClass("ECSqlTest", "p");
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = m_ecdb.Schemas().GetClass("ecSqL", "P", SchemaLookupMode::ByAlias);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = m_ecdb.Schemas().GetClass("ecsql", "p", SchemaLookupMode::ByAlias);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = m_ecdb.Schemas().GetClass("ecsqlTest", "P", SchemaLookupMode::AutoDetect);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = m_ecdb.Schemas().GetClass("ecsqL", "P", SchemaLookupMode::AutoDetect);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = m_ecdb.Schemas().GetClass("ECSqlTest", "p", SchemaLookupMode::AutoDetect);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = m_ecdb.Schemas().GetClass("ecsql", "p", SchemaLookupMode::AutoDetect);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetDerivedClasses)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecschemamanagertest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECClassCP baseClass = m_ecdb.Schemas().GetClass("ECSqlTest", "THBase");
    ASSERT_TRUE(baseClass != nullptr) << "Could not retrieve base class";

    //derived classes are not loaded when calling ECClass::GetDerivedClasses
    ASSERT_TRUE(baseClass->GetDerivedClasses().empty()) << "ECClass::GetDerivedClasses is expected to not load subclasses.";

    //derived classes are expected to be loaded when calling ECDbSchemaManager::GetDerivedClasses
    ASSERT_EQ(1, m_ecdb.Schemas().GetDerivedClasses(*baseClass).size()) << "Unexpected derived class count with derived classes now being loaded";

    //now ECClass::GetDerivedClasses can also be called
    ASSERT_EQ(1, baseClass->GetDerivedClasses().size()) << "Unexpected derived class count after derived classes were loaded";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetDerivedECClassesWithoutIncrementalLoading)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecschemamanagertest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("ECSqlTest", true);
    ASSERT_TRUE(testSchema != nullptr);

    ECClassCP baseClass = m_ecdb.Schemas().GetClass("ECSqlTest", "THBase");
    ASSERT_TRUE(baseClass != nullptr) << "Could not retrieve base class";

    ASSERT_EQ(1, baseClass->GetDerivedClasses().size()) << "Unexpected derived class count. Derived classes are expected to already be loaded along with having loaded the schema";

    //derived classes are expected to be loaded when calling ECDbSchemaManager::GetDerivedClasses
    ASSERT_EQ(1, m_ecdb.Schemas().GetDerivedClasses(*baseClass).size()) << "Unexpected derived class count";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetMixin)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("getmixin.ecdb",
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
                                          </ECSchema>)xml")));
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());
    m_ecdb.SaveChanges();
    m_ecdb.ClearECDbCache();

    //get in-memory schema without loading anything into it, so that we can track what gets loaded implicitly
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("TestSchema", false);
    ASSERT_EQ(0, schema->GetClassCount());
    //load mixin as first class and make sure the applies to class is loaded implicitly
    ECClassCP mixinRaw = m_ecdb.Schemas().GetClass("TestSchema", "IIsGeometric");
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
    m_ecdb.ClearECDbCache();
    schema = m_ecdb.Schemas().GetSchema("TestSchema", false);
    ASSERT_EQ(0, schema->GetClassCount());

    ECClassCP baseRelRaw = m_ecdb.Schemas().GetClass("TestSchema", "ModelHasElements");
    ASSERT_TRUE(baseRelRaw != nullptr && baseRelRaw->IsRelationshipClass());
    ASSERT_EQ(3, schema->GetClassCount()) << "Rel base class is loaded and its constraint classes";
    ASSERT_TRUE(schema->GetClassCP("ModelHasElements") != nullptr);
    ASSERT_TRUE(schema->GetClassCP("Model") != nullptr);
    ASSERT_TRUE(schema->GetClassCP("Element") != nullptr);

    ECClassCP subRelRaw = m_ecdb.Schemas().GetClass("TestSchema", "ModelHasGeometricElements");
    ASSERT_TRUE(subRelRaw != nullptr && subRelRaw->IsRelationshipClass());
    ASSERT_EQ(5, schema->GetClassCount()) << "Rel base class is loaded and its constraint classes";
    ASSERT_TRUE(schema->GetClassCP("ModelHasGeometricElements") != nullptr);
    ASSERT_TRUE(schema->GetClassCP("IIsGeometric") != nullptr);

    //now get relationship sub class before anything else
    m_ecdb.ClearECDbCache();
    schema = m_ecdb.Schemas().GetSchema("TestSchema", false);
    ASSERT_EQ(0, schema->GetClassCount());

    subRelRaw = m_ecdb.Schemas().GetClass("TestSchema", "ModelHasGeometricElements");
    ASSERT_TRUE(subRelRaw != nullptr && subRelRaw->IsRelationshipClass());
    ASSERT_EQ(5, schema->GetClassCount()) << "Rel base class is loaded and its constraint classes";
    ASSERT_TRUE(schema->GetClassCP("ModelHasElements") != nullptr);
    ASSERT_TRUE(schema->GetClassCP("ModelHasGeometricElements") != nullptr);
    ASSERT_TRUE(schema->GetClassCP("Model") != nullptr);
    ASSERT_TRUE(schema->GetClassCP("IIsGeometric") != nullptr);
    ASSERT_TRUE(schema->GetClassCP("Element") != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetPropertyMinMaxLength)
    {
    EXPECT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECProperty propertyName="Prop" typeName="string" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength supported for String";

    EXPECT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECArrayProperty propertyName="Prop" typeName="string" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength supported for String array prop";

    EXPECT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECProperty propertyName="Prop" typeName="binary" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength supported for Binary";

    EXPECT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECArrayProperty propertyName="Prop" typeName="binary" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength supported for Binary array prop";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECProperty propertyName="Prop" typeName="boolean" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for bools";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECArrayProperty propertyName="Prop" typeName="boolean" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for bool arrays";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECProperty propertyName="Prop" typeName="int" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for int";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECArrayProperty propertyName="Prop" typeName="int" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for int arrays";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECProperty propertyName="Prop" typeName="long" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for Long";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECArrayProperty propertyName="Prop" typeName="long" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for Long arrays";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECProperty propertyName="Prop" typeName="double" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for double";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECArrayProperty propertyName="Prop" typeName="double" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for double arrays";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECProperty propertyName="Prop" typeName="dateTime" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for DateTime";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECArrayProperty propertyName="Prop" typeName="dateTime" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for DateTime arrays";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECProperty propertyName="Prop" typeName="point2d" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for Point2d";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECArrayProperty propertyName="Prop" typeName="point2d" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for Point2d array";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECProperty propertyName="Prop" typeName="point3d" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for Point3d";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECArrayProperty propertyName="Prop" typeName="point3d" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for Point3d array";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECProperty propertyName="Prop" typeName="Bentley.Geometry.Common.IGeometry" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for IGeometry";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECArrayProperty propertyName="Prop" typeName="Bentley.Geometry.Common.IGeometry" minimumLength="5" maximumLength="10"/>
            </ECEntityClass>
        </ECSchema>)xml"))) << "MinimumLength/MaximumLength not supported for IGeometry arrays";

    ASSERT_EQ(SUCCESS, SetupECDb("GetPropertyMinMaxLength.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECStructClass typeName="MyStruct">
               <ECProperty propertyName="StrProp1" typeName="string" minimumLength="5" maximumLength="10"/>
               <ECProperty propertyName="StrProp2" typeName="string" minimumLength="5"/>
               <ECProperty propertyName="StrProp3" typeName="string" maximumLength="5"/>
               <ECProperty propertyName="BinaryProp1" typeName="Binary" minimumLength="5" maximumLength="10"/>
               <ECProperty propertyName="BinaryProp2" typeName="Binary" minimumLength="5"/>
               <ECProperty propertyName="BinaryProp3" typeName="Binary" maximumLength="5"/>
            </ECStructClass>
            <ECEntityClass typeName="Foo" modifier="None" >
               <ECProperty propertyName="StrProp1" typeName="string" minimumLength="5" maximumLength="10"/>
               <ECProperty propertyName="StrProp2" typeName="string" minimumLength="5"/>
               <ECProperty propertyName="StrProp3" typeName="string" maximumLength="5"/>
               <ECProperty propertyName="BinaryProp1" typeName="Binary" minimumLength="5" maximumLength="10"/>
               <ECProperty propertyName="BinaryProp2" typeName="Binary" minimumLength="5"/>
               <ECProperty propertyName="BinaryProp3" typeName="Binary" maximumLength="5"/>
               <ECStructProperty propertyName="MyStruct" typeName="MyStruct"/>
            </ECEntityClass>
       </ECSchema>)xml")));

    for (Utf8CP className : std::vector<Utf8CP>{"Foo", "MyStruct"})
        {
        ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", className);
        ASSERT_TRUE(testClass != nullptr) << className;
        EXPECT_TRUE(testClass->GetPropertyP("StrProp1")->IsMinimumLengthDefined()) << className;
        EXPECT_EQ(5, testClass->GetPropertyP("StrProp1")->GetMinimumLength()) << className;
        EXPECT_TRUE(testClass->GetPropertyP("StrProp1")->IsMaximumLengthDefined()) << className;
        EXPECT_EQ(10, testClass->GetPropertyP("StrProp1")->GetMaximumLength()) << className;

        EXPECT_TRUE(testClass->GetPropertyP("BinaryProp1")->IsMinimumLengthDefined()) << className;
        EXPECT_EQ(5, testClass->GetPropertyP("BinaryProp1")->GetMinimumLength()) << className;
        EXPECT_TRUE(testClass->GetPropertyP("BinaryProp1")->IsMaximumLengthDefined()) << className;
        EXPECT_EQ(10, testClass->GetPropertyP("BinaryProp1")->GetMaximumLength()) << className;

        EXPECT_TRUE(testClass->GetPropertyP("StrProp2")->IsMinimumLengthDefined()) << className;
        EXPECT_EQ(5, testClass->GetPropertyP("StrProp2")->GetMinimumLength()) << className;
        EXPECT_FALSE(testClass->GetPropertyP("StrProp2")->IsMaximumLengthDefined()) << className;

        EXPECT_TRUE(testClass->GetPropertyP("BinaryProp2")->IsMinimumLengthDefined()) << className;
        EXPECT_EQ(5, testClass->GetPropertyP("BinaryProp2")->GetMinimumLength()) << className;
        EXPECT_FALSE(testClass->GetPropertyP("BinaryProp2")->IsMaximumLengthDefined()) << className;

        EXPECT_FALSE(testClass->GetPropertyP("StrProp3")->IsMinimumLengthDefined()) << className;
        EXPECT_TRUE(testClass->GetPropertyP("StrProp3")->IsMaximumLengthDefined()) << className;
        EXPECT_EQ(5, testClass->GetPropertyP("StrProp3")->GetMaximumLength()) << className;

        EXPECT_FALSE(testClass->GetPropertyP("BinaryProp3")->IsMinimumLengthDefined()) << className;
        EXPECT_TRUE(testClass->GetPropertyP("BinaryProp3")->IsMaximumLengthDefined()) << className;
        EXPECT_EQ(5, testClass->GetPropertyP("BinaryProp3")->GetMaximumLength()) << className;
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetPropertyMinMaxValue)
    {
    EXPECT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECProperty propertyName="Prop" typeName="int" minimumValue="5" maximumValue="10"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue supported for int";

    EXPECT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECArrayProperty propertyName="Prop" typeName="int" minimumValue="5" maximumValue="10"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue supported for int array";

    EXPECT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECProperty propertyName="Prop" typeName="long" minimumValue="-5" maximumValue="10"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue supported for long";

    EXPECT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECArrayProperty propertyName="Prop" typeName="long" minimumValue="-5" maximumValue="10"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue supported for long array";

    EXPECT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECProperty propertyName="Prop" typeName="double" minimumValue="-5.3" maximumValue="10.13"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue supported for double";

    EXPECT_EQ(SUCCESS, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECArrayProperty propertyName="Prop" typeName="double" minimumValue="-5.3" maximumValue="10.13"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue supported for double array";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECProperty propertyName="Prop" typeName="bool" minimumValue="0" maximumValue="1"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue not expected to be supported for bool";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECProperty propertyName="Prop" typeName="bool" minimumValue="false"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue not expected to be supported for bool";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECProperty propertyName="Prop" typeName="bool" maximumValue="true"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue not expected to be supported for bool";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECProperty propertyName="Prop" typeName="dateTime" minimumValue="250000.5"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue not expected to be supported for date times";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECProperty propertyName="Prop" typeName="dateTime" minimumValue="2000-01-01T12:00:00"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue not expected to be supported for date times";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECProperty propertyName="Prop" typeName="Bentley.Geometry.Common.IGeometry" maximumValue="1"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue not expected to be supported for IGeometry";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECProperty propertyName="Prop" typeName="Point2d" minimumValue="0" maximumValue="1"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue not expected to be supported for Point2d";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECProperty propertyName="Prop" typeName="Point3d" minimumValue="0" maximumValue="1"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue not expected to be supported for Point3d";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECProperty propertyName="Prop" typeName="string" minimumValue="0" maximumValue="1"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue not expected to be supported for string";

    EXPECT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECProperty propertyName="Prop" typeName="string" minimumValue="aaa" maximumValue="DDD"/>
        </ECEntityClass>
    </ECSchema>)xml"))) << "MinimumValue/MaximumValue not expected to be supported for string";


    ASSERT_EQ(SUCCESS, SetupECDb("getpropertyminmaxvalue.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
        <ECStructClass typeName="MyStruct" modifier="None" >
            <ECProperty propertyName="IntProp1" typeName="int" minimumValue="0"/>
            <ECProperty propertyName="IntProp2" typeName="int" maximumValue="10"/>
            <ECProperty propertyName="IntProp3" typeName="int" minimumValue="-10" maximumValue="10"/>
            <ECProperty propertyName="LongProp1" typeName="long" minimumValue="0"/>
            <ECProperty propertyName="LongProp2" typeName="long" maximumValue="10"/>
            <ECProperty propertyName="LongProp3" typeName="long" minimumValue="-10" maximumValue="10"/>
            <ECProperty propertyName="DoubleProp1" typeName="double" minimumValue="0"/>
            <ECProperty propertyName="DoubleProp2" typeName="double" maximumValue="10"/>
            <ECProperty propertyName="DoubleProp3" typeName="double" minimumValue="-10.5" maximumValue="10.5"/>
    
            <ECArrayProperty propertyName="IntArrayProp1" typeName="int" minimumValue="0"/>
            <ECArrayProperty propertyName="IntArrayProp2" typeName="int" maximumValue="10"/>
            <ECArrayProperty propertyName="IntArrayProp3" typeName="int" minimumValue="-10" maximumValue="10"/>

            <ECArrayProperty propertyName="LongArrayProp1" typeName="long" minimumValue="0"/>
            <ECArrayProperty propertyName="LongArrayProp2" typeName="long" maximumValue="10"/>
            <ECArrayProperty propertyName="LongArrayProp3" typeName="long" minimumValue="-10" maximumValue="10"/>

            <ECArrayProperty propertyName="DoubleArrayProp1" typeName="double" minimumValue="0"/>
            <ECArrayProperty propertyName="DoubleArrayProp2" typeName="double" maximumValue="10"/>
            <ECArrayProperty propertyName="DoubleArrayProp3" typeName="double" minimumValue="-10.5" maximumValue="10.5"/>
        </ECStructClass>
        <ECEntityClass typeName="Foo" modifier="None" >
            <ECProperty propertyName="IntProp1" typeName="int" minimumValue="0"/>
            <ECProperty propertyName="IntProp2" typeName="int" maximumValue="10"/>
            <ECProperty propertyName="IntProp3" typeName="int" minimumValue="-10" maximumValue="10"/>
            <ECProperty propertyName="LongProp1" typeName="long" minimumValue="0"/>
            <ECProperty propertyName="LongProp2" typeName="long" maximumValue="10"/>
            <ECProperty propertyName="LongProp3" typeName="long" minimumValue="-10" maximumValue="10"/>
            <ECProperty propertyName="DoubleProp1" typeName="double" minimumValue="0"/>
            <ECProperty propertyName="DoubleProp2" typeName="double" maximumValue="10"/>
            <ECProperty propertyName="DoubleProp3" typeName="double" minimumValue="-10.5" maximumValue="10.5"/>
    
            <ECArrayProperty propertyName="IntArrayProp1" typeName="int" minimumValue="0"/>
            <ECArrayProperty propertyName="IntArrayProp2" typeName="int" maximumValue="10"/>
            <ECArrayProperty propertyName="IntArrayProp3" typeName="int" minimumValue="-10" maximumValue="10"/>

            <ECArrayProperty propertyName="LongArrayProp1" typeName="long" minimumValue="0"/>
            <ECArrayProperty propertyName="LongArrayProp2" typeName="long" maximumValue="10"/>
            <ECArrayProperty propertyName="LongArrayProp3" typeName="long" minimumValue="-10" maximumValue="10"/>

            <ECArrayProperty propertyName="DoubleArrayProp1" typeName="double" minimumValue="0"/>
            <ECArrayProperty propertyName="DoubleArrayProp2" typeName="double" maximumValue="10"/>
            <ECArrayProperty propertyName="DoubleArrayProp3" typeName="double" minimumValue="-10.5" maximumValue="10.5"/>

            <ECStructProperty propertyName="MyStruct" typeName="MyStruct" />
        </ECEntityClass>
    </ECSchema>)xml")));

    for (Utf8CP className : std::vector<Utf8CP> {"Foo", "MyStruct"})
        {
        ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", className);
        ASSERT_TRUE(testClass != nullptr) << className;
        ECPropertyCP prop = testClass->GetPropertyP("IntProp1");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_TRUE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        ECValue v;
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMinimumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(0, v.GetInteger()) << className << "." << prop->GetName().c_str();
        EXPECT_FALSE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();

        prop = testClass->GetPropertyP("IntProp2");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_FALSE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        EXPECT_TRUE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMaximumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(10, v.GetInteger()) << className << "." << prop->GetName().c_str();

        prop = testClass->GetPropertyP("IntProp3");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_TRUE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMinimumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(-10, v.GetInteger()) << className << "." << prop->GetName().c_str();
        EXPECT_TRUE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMaximumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(10, v.GetInteger()) << className << "." << prop->GetName().c_str();


        prop = testClass->GetPropertyP("LongProp1");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_TRUE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMinimumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(0, v.GetLong()) << className << "." << prop->GetName().c_str();
        EXPECT_FALSE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();

        prop = testClass->GetPropertyP("LongProp2");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_FALSE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        EXPECT_TRUE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMaximumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(10, v.GetLong()) << className << "." << prop->GetName().c_str();

        prop = testClass->GetPropertyP("LongProp3");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_TRUE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMinimumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(-10, v.GetLong()) << className << "." << prop->GetName().c_str();
        EXPECT_TRUE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMaximumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(10, v.GetLong()) << className << "." << prop->GetName().c_str();


        prop = testClass->GetPropertyP("DoubleProp1");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_TRUE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMinimumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_DOUBLE_EQ(0, v.GetDouble()) << className << "." << prop->GetName().c_str();
        EXPECT_FALSE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();

        prop = testClass->GetPropertyP("DoubleProp2");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_FALSE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        EXPECT_TRUE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMaximumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_DOUBLE_EQ(10, v.GetDouble()) << className << "." << prop->GetName().c_str();

        prop = testClass->GetPropertyP("DoubleProp3");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_TRUE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMinimumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_DOUBLE_EQ(-10.5, v.GetDouble()) << className << "." << prop->GetName().c_str();
        EXPECT_TRUE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMaximumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_DOUBLE_EQ(10.5, v.GetDouble()) << className << "." << prop->GetName().c_str();


        prop = testClass->GetPropertyP("IntArrayProp1");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_TRUE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMinimumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(0, v.GetInteger()) << className << "." << prop->GetName().c_str();
        EXPECT_FALSE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();

        prop = testClass->GetPropertyP("IntArrayProp2");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_FALSE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        EXPECT_TRUE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMaximumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(10, v.GetInteger()) << className << "." << prop->GetName().c_str();

        prop = testClass->GetPropertyP("IntArrayProp3");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_TRUE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMinimumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(-10, v.GetInteger()) << className << "." << prop->GetName().c_str();
        EXPECT_TRUE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMaximumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(10, v.GetInteger()) << className << "." << prop->GetName().c_str();


        prop = testClass->GetPropertyP("LongArrayProp1");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_TRUE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMinimumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(0, v.GetLong()) << className << "." << prop->GetName().c_str();
        EXPECT_FALSE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();

        prop = testClass->GetPropertyP("LongArrayProp2");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_FALSE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        EXPECT_TRUE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMaximumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(10, v.GetLong()) << className << "." << prop->GetName().c_str();

        prop = testClass->GetPropertyP("LongArrayProp3");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_TRUE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMinimumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(-10, v.GetLong()) << className << "." << prop->GetName().c_str();
        EXPECT_TRUE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMaximumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(10, v.GetLong()) << className << "." << prop->GetName().c_str();


        prop = testClass->GetPropertyP("DoubleArrayProp1");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_TRUE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMinimumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_DOUBLE_EQ(0, v.GetDouble()) << className << "." << prop->GetName().c_str();
        EXPECT_FALSE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();

        prop = testClass->GetPropertyP("DoubleArrayProp2");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_FALSE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        EXPECT_TRUE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMaximumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_DOUBLE_EQ(10, v.GetDouble()) << className << "." << prop->GetName().c_str();

        prop = testClass->GetPropertyP("DoubleArrayProp3");
        ASSERT_TRUE(prop != nullptr) << className;
        EXPECT_TRUE(prop->IsMinimumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMinimumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_DOUBLE_EQ(-10.5, v.GetDouble()) << className << "." << prop->GetName().c_str();
        EXPECT_TRUE(prop->IsMaximumValueDefined()) << className << "." << prop->GetName().c_str();
        ASSERT_EQ(ECObjectsStatus::Success, prop->GetMaximumValue(v)) << className << "." << prop->GetName().c_str();
        ASSERT_DOUBLE_EQ(10.5, v.GetDouble()) << className << "." << prop->GetName().c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetPropertyPriority)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("GetPropertyPriority.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"  >
            <ECStructClass typeName="MyStruct">
                <ECProperty propertyName="MinusFiveHundred" typeName="string" priority="-500"/>
            </ECStructClass>
            <ECEntityClass typeName="Parent" modifier="None" />
            <ECEntityClass typeName="Foo" modifier="None" >
                <ECProperty propertyName="None" typeName="string" />
                <ECProperty propertyName="Zero" typeName="string" priority="0" />
                <ECProperty propertyName="Ten" typeName="string" priority="10" />
                <ECProperty propertyName="MinusTen" typeName="string" priority="-10" />
                <ECStructProperty propertyName="MinusTwentyStruct" typeName="MyStruct" priority="-20" />
                <ECArrayProperty propertyName="MinusTenArray" typeName="string" priority="-10" />
                <ECStructArrayProperty propertyName="HundredStructArray" typeName="MyStruct" priority="100" />
                <ECNavigationProperty propertyName="MinusOneNavProp" relationshipName="Rel" direction="Backward" priority="-1"/>
            </ECEntityClass>
            <ECEntityClass typeName="FooSub" modifier="None" >
                <BaseClass>Foo</BaseClass>
                <ECProperty propertyName="Ten" typeName="string"/>
                <ECArrayProperty propertyName="MinusTenArray" typeName="string" priority="-11" />
            </ECEntityClass>
            <ECRelationshipClass typeName="Rel" modifier="Sealed" strength="Referencing">
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="refers to">
                    <Class class="Parent"/>
                </Source>
                <Target multiplicity="(0..*)" polymorphic="true" roleLabel="refers to">
                    <Class class="Foo"/>
                </Target>
           </ECRelationshipClass>
        </ECSchema>)xml")));

    ECClassCP fooClass = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
    ASSERT_TRUE(fooClass != nullptr);
    EXPECT_FALSE(fooClass->GetPropertyP("None")->IsPriorityLocallyDefined());
    EXPECT_EQ(0, fooClass->GetPropertyP("None")->GetPriority());
    EXPECT_TRUE(fooClass->GetPropertyP("Zero")->IsPriorityLocallyDefined());
    EXPECT_EQ(0, fooClass->GetPropertyP("Zero")->GetPriority());
    EXPECT_TRUE(fooClass->GetPropertyP("Ten")->IsPriorityLocallyDefined());
    EXPECT_EQ(10, fooClass->GetPropertyP("Ten")->GetPriority());
    EXPECT_TRUE(fooClass->GetPropertyP("MinusTen")->IsPriorityLocallyDefined());
    EXPECT_EQ(-10, fooClass->GetPropertyP("MinusTen")->GetPriority());
    EXPECT_TRUE(fooClass->GetPropertyP("MinusTwentyStruct")->IsPriorityLocallyDefined());
    EXPECT_EQ(-20, fooClass->GetPropertyP("MinusTwentyStruct")->GetPriority());
    EXPECT_TRUE(fooClass->GetPropertyP("MinusTenArray")->IsPriorityLocallyDefined());
    EXPECT_EQ(-10, fooClass->GetPropertyP("MinusTenArray")->GetPriority());
    EXPECT_TRUE(fooClass->GetPropertyP("HundredStructArray")->IsPriorityLocallyDefined());
    EXPECT_EQ(100, fooClass->GetPropertyP("HundredStructArray")->GetPriority());
    EXPECT_TRUE(fooClass->GetPropertyP("MinusOneNavProp")->IsPriorityLocallyDefined());
    EXPECT_EQ(-1, fooClass->GetPropertyP("MinusOneNavProp")->GetPriority());

    ECClassCP myStruct = m_ecdb.Schemas().GetClass("TestSchema", "MyStruct");
    ASSERT_TRUE(myStruct != nullptr);
    EXPECT_TRUE(myStruct->GetPropertyP("MinusFiveHundred")->IsPriorityLocallyDefined());
    EXPECT_EQ(-500, myStruct->GetPropertyP("MinusFiveHundred")->GetPriority());

    ECClassCP fooSubClass = m_ecdb.Schemas().GetClass("TestSchema", "FooSub");
    ASSERT_TRUE(fooSubClass != nullptr);
    EXPECT_TRUE(fooSubClass->GetPropertyP("MinusTen")->IsPriorityLocallyDefined()) << "Inherited property. As not overridden the priority is still considered locally defined";
    EXPECT_EQ(-10, fooSubClass->GetPropertyP("MinusTen")->GetPriority()) << "Inherited property. As not overridden the priority is still considered locally defined";
    EXPECT_FALSE(fooSubClass->GetPropertyP("Ten")->IsPriorityLocallyDefined()) << "Overridden property without specifying priority again";
    EXPECT_EQ(10, fooSubClass->GetPropertyP("Ten")->GetPriority()) << "Overridden property without specifying priority again";
    EXPECT_TRUE(fooSubClass->GetPropertyP("MinusTenArray")->IsPriorityLocallyDefined()) << "Overridden property with specifying new priority";
    EXPECT_EQ(-11, fooSubClass->GetPropertyP("MinusTenArray")->GetPriority()) << "Overridden property with specifying new priority";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetEnumeration)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("getenumeration.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "<ECSchemaReference name='ECDbFileInfo' version='02.00.00' prefix='ecdbf' />"
                                     "  <ECEntityClass typeName='Foo' >"
                                     "    <ECProperty propertyName='Folder' typeName='ecdbf:StandardRootFolderType' />"
                                     "    <ECProperty propertyName='Homepage' typeName='string' extendedTypeName='URL' />"
                                     "    <ECArrayProperty propertyName='FavoriteFolders' typeName='ecdbf:StandardRootFolderType' minOccurs='0' maxOccurs='unbounded' />"
                                     "  </ECEntityClass>"
                                     "</ECSchema>")));

    ASSERT_EQ(BeVersion(3, 0), GetHelper().GetOriginalECXmlVersion("TestSchema"));

    {
    ECEnumerationCP ecEnum = m_ecdb.Schemas().GetEnumeration("ECDbFileInfo", "StandardRootFolderType");
    ASSERT_TRUE(ecEnum != nullptr);
    ASSERT_EQ(4, ecEnum->GetEnumeratorCount());
    }

    {
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("ECDbFileInfo", false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(0, schema->GetEnumerationCount());
    ECClassCP classWithEnum = m_ecdb.Schemas().GetClass("ECDbFileInfo", "ExternalFileInfo");
    ASSERT_TRUE(classWithEnum != nullptr);

    ECPropertyCP prop = classWithEnum->GetPropertyP("RootFolder");
    ASSERT_TRUE(prop != nullptr);
    PrimitiveECPropertyCP primProp = prop->GetAsPrimitiveProperty();
    ASSERT_TRUE(primProp != nullptr);
    ECEnumerationCP ecEnum = primProp->GetEnumeration();
    ASSERT_TRUE(ecEnum != nullptr);
    ASSERT_STREQ("StandardRootFolderType", ecEnum->GetName().c_str());
    ASSERT_EQ(4, ecEnum->GetEnumeratorCount());

    ecEnum = schema->GetEnumerationCP("StandardRootFolderType");
    ASSERT_TRUE(ecEnum != nullptr);
    ASSERT_EQ(4, ecEnum->GetEnumeratorCount());
    }

    {
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("ECDbFileInfo", false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(0, schema->GetEnumerationCount());
    ECClassCP classWithEnum = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
    ASSERT_TRUE(classWithEnum != nullptr);

    ECPropertyCP prop = classWithEnum->GetPropertyP("Folder");
    ASSERT_TRUE(prop != nullptr);
    PrimitiveECPropertyCP primProp = prop->GetAsPrimitiveProperty();
    ASSERT_TRUE(primProp != nullptr);
    ECEnumerationCP ecEnum = primProp->GetEnumeration();
    ASSERT_TRUE(ecEnum != nullptr);
    ASSERT_STREQ("StandardRootFolderType", ecEnum->GetName().c_str());
    ASSERT_EQ(4, ecEnum->GetEnumeratorCount());

    prop = classWithEnum->GetPropertyP("FavoriteFolders");
    ASSERT_TRUE(prop != nullptr);
    PrimitiveArrayECPropertyCP primArrayProp = prop->GetAsPrimitiveArrayProperty();
    ASSERT_TRUE(primArrayProp != nullptr);
    ecEnum = primArrayProp->GetEnumeration();
    ASSERT_TRUE(ecEnum != nullptr);
    ASSERT_STREQ("StandardRootFolderType", ecEnum->GetName().c_str());
    ASSERT_EQ(4, ecEnum->GetEnumeratorCount());

    ecEnum = schema->GetEnumerationCP("StandardRootFolderType");
    ASSERT_TRUE(ecEnum != nullptr);
    ASSERT_EQ(4, ecEnum->GetEnumeratorCount());
    }

    {
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("ECDbFileInfo", true);
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
        ASSERT_STREQ("u:CM", actualKoq.GetPersistenceUnit()->GetQualifiedName(actualKoq.GetSchema()).c_str());
        ASSERT_DOUBLE_EQ(.5, actualKoq.GetRelativeError());
        ASSERT_EQ(2, actualKoq.GetPresentationFormats().size());
        ASSERT_STREQ("f:DefaultRealU[u:M]", actualKoq.GetPresentationFormats()[0].GetQualifiedFormatString(actualKoq.GetSchema()).c_str());
        ASSERT_STREQ("f:DefaultReal[u:M]", actualKoq.GetPresentationFormats()[1].GetQualifiedFormatString(actualKoq.GetSchema()).c_str());
        };

    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                     <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                     <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                                     <ECSchemaReference name="Formats" version="01.00.00" alias="f" />
                                     <KindOfQuantity typeName="MyKindOfQuantity" description="My KindOfQuantity"
                                                     displayLabel="My KindOfQuantity" persistenceUnit="u:CM" relativeError=".5"
                                                     presentationUnits="f:DefaultRealU[u:M];f:DefaultReal[u:M]" />
                                     </ECSchema>)xml"));

    testSchemas.push_back(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                     <ECSchema schemaName="Schema2" alias="s2" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                     <ECSchemaReference name="Schema1" version="01.00.00" alias="s1" />
                                       <ECEntityClass typeName="Foo" >
                                         <ECProperty propertyName="Length" typeName="double" kindOfQuantity="s1:MyKindOfQuantity" />
                                         <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                                         <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" kindOfQuantity="s1:MyKindOfQuantity"/>
                                         <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
                                       </ECEntityClass>
                                     </ECSchema>)xml"));

    ASSERT_EQ(SUCCESS, SetupECDb("getkindofquantity.ecdb", testSchemas[0]));
    ASSERT_TRUE(m_ecdb.Schemas().ContainsSchema("Units"));
    ASSERT_EQ(SUCCESS, ImportSchema(testSchemas[1]));
    ASSERT_EQ(BeVersion(3, 2), GetHelper().GetOriginalECXmlVersion("Schema1"));
    ASSERT_EQ(BeVersion(3, 2), GetHelper().GetOriginalECXmlVersion("Schema2"));

    {
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());


    KindOfQuantityCP koq = m_ecdb.Schemas().GetKindOfQuantity("Schema1", "MyKindOfQuantity");
    ASSERT_TRUE(koq != nullptr);
    assertKoq(*koq);
    }

    {
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1", false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(0, schema->GetKindOfQuantityCount());
    ECClassCP classWithKoq = m_ecdb.Schemas().GetClass("Schema2", "Foo");
    ASSERT_TRUE(classWithKoq != nullptr);
    ASSERT_EQ(1, schema->GetKindOfQuantityCount());
    KindOfQuantityCP koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_TRUE(koq != nullptr);
    assertKoq(*koq);

    ECPropertyCP prop = classWithKoq->GetPropertyP("Length");
    ASSERT_TRUE(prop != nullptr);
    koq = prop->GetKindOfQuantity();
    ASSERT_TRUE(koq != nullptr);
    assertKoq(*koq);
    }

    {
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1", true);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(1, schema->GetKindOfQuantityCount());
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, LoadAllUnitsImplicitly)
    {
    //This tests that units, unit systems, phenomena and formats are always loaded entirely if
    //a KOQ is loaded, a unit is loaded, a unit system is loaded or a phenomenon is loaded.

    ASSERT_EQ(SUCCESS, SetupECDb("LoadAllUnitsImplicitly.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                     <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                     <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                                     <ECSchemaReference name="Formats" version="01.00.00" alias="f" />
                                     <KindOfQuantity typeName="KoqWithoutPresentation" description="My KindOfQuantity"
                                                     displayLabel="My KindOfQuantity" persistenceUnit="u:CM" relativeError=".5" />
                                     <KindOfQuantity typeName="KoqWithPresentation" description="My KindOfQuantity"
                                                     displayLabel="My KindOfQuantity" persistenceUnit="u:CM" relativeError=".5"
                                                     presentationUnits="f:DefaultRealU[u:M];f:DefaultReal[u:M]" />
                                     </ECSchema>)xml")));
    auto assertLoadCount = [] (ECDbCR ecdb, Utf8CP schemaName, int expectedKoqCount, int expectedUnitCount, int expectedUnitSystemCount, int expectedPhenCount, int expectedFormatCount, Utf8CP assertMessage)
        {
        ECSchemaCP schema = ecdb.Schemas().GetSchema(schemaName, false);
        ASSERT_TRUE(schema != nullptr) << schemaName << " " << assertMessage;

        EXPECT_EQ(expectedKoqCount, schema->GetKindOfQuantityCount()) << schemaName << " " << assertMessage;
        EXPECT_EQ(expectedUnitCount, schema->GetUnitCount()) << schemaName << " " << assertMessage;
        EXPECT_EQ(expectedUnitSystemCount, schema->GetUnitSystemCount()) << schemaName << " " << assertMessage;
        EXPECT_EQ(expectedPhenCount, schema->GetPhenomenonCount()) << schemaName << " " << assertMessage;
        EXPECT_EQ(expectedFormatCount, schema->GetFormatCount()) << schemaName << " " << assertMessage;
        };

    const int standardUnitSystemCount = 12;
    const int standardPhenCount = 68;
    const int standardUnitCount = 458;
    const int standardFormatCount = 10;

    assertLoadCount(m_ecdb, "TestSchema", 0, 0, 0, 0, 0, "No schema elements are expected to be loaded at this point");
    assertLoadCount(m_ecdb, "Units", 0, 0, 0, 0, 0, "No schema elements are expected to be loaded at this point");
    assertLoadCount(m_ecdb, "Formats", 0, 0, 0, 0, 0, "No schema elements are expected to be loaded at this point");

    KindOfQuantityCP koqWithoutPres = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "KoqWithoutPresentation");
    ASSERT_TRUE(koqWithoutPres != nullptr);

    assertLoadCount(m_ecdb, "TestSchema", 1, 0, 0, 0, 0, "After a KOQ without presentation formats was loaded");
    assertLoadCount(m_ecdb, "Units", 0, standardUnitCount, standardUnitSystemCount, standardPhenCount, 0, "After a KOQ without presentation formats was loaded");
    assertLoadCount(m_ecdb, "Formats", 0, 0, 0, 0, standardFormatCount, "After a KOQ without presentation formats was loaded");

    m_ecdb.ClearECDbCache();

    KindOfQuantityCP koqWithPres = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "KoqWithPresentation");
    ASSERT_TRUE(koqWithPres != nullptr);

    assertLoadCount(m_ecdb, "TestSchema", 1, 0, 0, 0, 0, "After a KOQ with 2 presentation formats was loaded");
    assertLoadCount(m_ecdb, "Units", 0, standardUnitCount, standardUnitSystemCount, standardPhenCount, 0, "After a KOQ with 2 presentation formats was loaded");
    assertLoadCount(m_ecdb, "Formats", 0, 0, 0, 0, standardFormatCount, "After a KOQ with 2 presentation formats was loaded");

    koqWithoutPres = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "KoqWithoutPresentation");
    ASSERT_TRUE(koqWithoutPres != nullptr);

    assertLoadCount(m_ecdb, "TestSchema", 2, 0, 0, 0, 0, "After a second KOQ was loaded");
    assertLoadCount(m_ecdb, "Units", 0, standardUnitCount, standardUnitSystemCount, standardPhenCount, 0, "After a second KOQ was loaded");
    assertLoadCount(m_ecdb, "Formats", 0, 0, 0, 0, standardFormatCount, "After a second KOQ was loaded");

    m_ecdb.ClearECDbCache();
    ECUnitCP unit = m_ecdb.Schemas().GetUnit("Units", "KM");
    ASSERT_TRUE(unit != nullptr);

    assertLoadCount(m_ecdb, "TestSchema", 0, 0, 0, 0, 0, "After a Unit was loaded");
    assertLoadCount(m_ecdb, "Units", 0, standardUnitCount, standardUnitSystemCount, standardPhenCount, 0, "After a Unit was loaded");
    assertLoadCount(m_ecdb, "Formats", 0, 0, 0, 0, standardFormatCount, "After a Unit was loaded");

    unit = m_ecdb.Schemas().GetUnit("Units", "KG");
    ASSERT_TRUE(unit != nullptr);

    assertLoadCount(m_ecdb, "TestSchema", 0, 0, 0, 0, 0, "After a second Unit was loaded");
    assertLoadCount(m_ecdb, "Units", 0, standardUnitCount, standardUnitSystemCount, standardPhenCount, 0, "After a second Unit was loaded");
    assertLoadCount(m_ecdb, "Formats", 0, 0, 0, 0, standardFormatCount, "After a second Unit was loaded");

    m_ecdb.ClearECDbCache();
    ECFormatCP format = m_ecdb.Schemas().GetFormat("Formats", "Fractional");
    ASSERT_TRUE(format != nullptr);

    assertLoadCount(m_ecdb, "TestSchema", 0, 0, 0, 0, 0, "After a Format without composite units was loaded");
    assertLoadCount(m_ecdb, "Units", 0, standardUnitCount, standardUnitSystemCount, standardPhenCount, 0, "After a Format without composite units was loaded");
    assertLoadCount(m_ecdb, "Formats", 0, 0, 0, 0, standardFormatCount, "After a Format without composite units was loaded");

    format = m_ecdb.Schemas().GetFormat("Formats", "AngleDMS");
    ASSERT_TRUE(format != nullptr);

    assertLoadCount(m_ecdb, "TestSchema", 0, 0, 0, 0, 0, "After a Format with composite units was loaded");
    assertLoadCount(m_ecdb, "Units", 0, standardUnitCount, standardUnitSystemCount, standardPhenCount, 0, "After a second Format with composite units was loaded");
    assertLoadCount(m_ecdb, "Formats", 0, 0, 0, 0, standardFormatCount, "After a Format with composite units was loaded");

    m_ecdb.ClearECDbCache();
    UnitSystemCP unitSystem = m_ecdb.Schemas().GetUnitSystem("Units", "SI");
    ASSERT_TRUE(unitSystem != nullptr);

    assertLoadCount(m_ecdb, "TestSchema", 0, 0, 0, 0, 0, "After a UnitSystem was loaded");
    assertLoadCount(m_ecdb, "Units", 0, standardUnitCount, standardUnitSystemCount, standardPhenCount, 0, "After a UnitSystem was loaded");
    assertLoadCount(m_ecdb, "Formats", 0, 0, 0, 0, standardFormatCount, "After a UnitSystem was loaded");

    unitSystem = m_ecdb.Schemas().GetUnitSystem("Units", "FINANCE");
    ASSERT_TRUE(unitSystem != nullptr);

    assertLoadCount(m_ecdb, "TestSchema", 0, 0, 0, 0, 0, "After a second UnitSystem was loaded");
    assertLoadCount(m_ecdb, "Units", 0, standardUnitCount, standardUnitSystemCount, standardPhenCount, 0, "After a second UnitSystem was loaded");
    assertLoadCount(m_ecdb, "Formats", 0, 0, 0, 0, standardFormatCount, "After a second UnitSystem was loaded");

    m_ecdb.ClearECDbCache();
    PhenomenonCP phen = m_ecdb.Schemas().GetPhenomenon("Units", "AREA");
    ASSERT_TRUE(phen != nullptr);

    assertLoadCount(m_ecdb, "TestSchema", 0, 0, 0, 0, 0, "After a Phenomenon was loaded");
    assertLoadCount(m_ecdb, "Units", 0, standardUnitCount, standardUnitSystemCount, standardPhenCount, 0, "After a Phenomenon was loaded");
    assertLoadCount(m_ecdb, "Formats", 0, 0, 0, 0, standardFormatCount, "After a Phenomenon was loaded");

    phen = m_ecdb.Schemas().GetPhenomenon("Units", "THERMAL_CONDUCTIVITY");
    ASSERT_TRUE(phen != nullptr);

    assertLoadCount(m_ecdb, "TestSchema", 0, 0, 0, 0, 0, "After a second Phenomenon was loaded");
    assertLoadCount(m_ecdb, "Units", 0, standardUnitCount, standardUnitSystemCount, standardPhenCount, 0, "After a second Phenomenon was loaded");
    assertLoadCount(m_ecdb, "Formats", 0, 0, 0, 0, standardFormatCount, "After a second Phenomenon was loaded");
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, ImportPreEC32KindOfQuantity)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ImportPreEC32KindOfQuantity.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                     <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                     <KindOfQuantity typeName="MyKindOfQuantity" persistenceUnit="CM" relativeError=".5"  presentationUnits="FT;IN" />
                                     </ECSchema>)xml")));

    ECSchemaCP testSchema = m_ecdb.Schemas().GetSchema("TestSchema");
    ASSERT_TRUE(testSchema != nullptr);
    
    ASSERT_EQ(2, testSchema->GetReferencedSchemas().size());
    KindOfQuantityCP koq = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "MyKindOfQuantity");
    ASSERT_TRUE(koq != nullptr);
    ASSERT_EQ(2, koq->GetPresentationFormats().size());
    EXPECT_STREQ("f:DefaultReal[u:FT]", koq->GetPresentationFormats()[0].GetQualifiedFormatString(*testSchema).c_str());
    EXPECT_STREQ("f:DefaultReal[u:IN]", koq->GetPresentationFormats()[1].GetQualifiedFormatString(*testSchema).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetPreEC32KindOfQuantity)
    {
    auto assertKoq = [] (KindOfQuantityCR actualKoq)
        {
        EXPECT_STREQ("My KindOfQuantity", actualKoq.GetDisplayLabel().c_str());
        EXPECT_STREQ("My KindOfQuantity", actualKoq.GetDescription().c_str());
        EXPECT_STREQ("u:CM", actualKoq.GetPersistenceUnit()->GetQualifiedName(actualKoq.GetSchema()).c_str());
        EXPECT_DOUBLE_EQ(.5, actualKoq.GetRelativeError());
        ASSERT_EQ(2, actualKoq.GetPresentationFormats().size());
        EXPECT_STREQ("f:DefaultReal[u:FT]", actualKoq.GetPresentationFormats()[0].GetQualifiedFormatString(actualKoq.GetSchema()).c_str());
        EXPECT_STREQ("f:DefaultReal[u:IN]", actualKoq.GetPresentationFormats()[1].GetQualifiedFormatString(actualKoq.GetSchema()).c_str());
        };

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

    ASSERT_EQ(SUCCESS, SetupECDb("getkindofquantity.ecdb", testSchemas[0]));
    ASSERT_TRUE(m_ecdb.Schemas().ContainsSchema("Units"));
    ASSERT_EQ(SUCCESS, ImportSchema(testSchemas[1]));
    ASSERT_EQ(BeVersion(3,1), GetHelper().GetOriginalECXmlVersion("Schema1"));
    ASSERT_EQ(BeVersion(3, 1), GetHelper().GetOriginalECXmlVersion("Schema2"));

    {
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());


    KindOfQuantityCP koq = m_ecdb.Schemas().GetKindOfQuantity("Schema1", "MyKindOfQuantity");
    ASSERT_TRUE(koq != nullptr);
    assertKoq(*koq);
    }

    {
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1", false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(0, schema->GetKindOfQuantityCount());
    ECClassCP classWithKoq = m_ecdb.Schemas().GetClass("Schema2", "Foo");
    ASSERT_TRUE(classWithKoq != nullptr);
    ASSERT_EQ(1, schema->GetKindOfQuantityCount());
    KindOfQuantityCP koq = schema->GetKindOfQuantityCP("MyKindOfQuantity");
    ASSERT_TRUE(koq != nullptr);
    assertKoq(*koq);

    ECPropertyCP prop = classWithKoq->GetPropertyP("Length");
    ASSERT_TRUE(prop != nullptr);
    koq = prop->GetKindOfQuantity();
    ASSERT_TRUE(koq != nullptr);
    assertKoq(*koq);
    }

    {
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1", true);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(1, schema->GetKindOfQuantityCount());
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  05/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, Formats)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("schemamanager_formats.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                <ECSchema schemaName="Schema1" alias="s1" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                    <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                                    <Unit typeName="MyMeter" displayLabel="My Metre" definition="u:M" numerator="1.0" phenomenon="u:LENGTH" unitSystem="u:METRIC" />
                                    <Format typeName="Format1" displayLabel="Format 1" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero"
                                            precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                                    </Format>
                                    <Format typeName="Format2" displayLabel="Format 2" description="Nice format 2" roundFactor="0.3" type="Scientific" scientificType="ZeroNormalized" showSignOption="NegativeParentheses" formatTraits="ShowUnitLabel|PrependUnitLabel|KeepSingleZero"
                                            precision="4" decimalSeparator="," thousandSeparator="." uomSeparator="*">
                                        <Composite>
                                            <Unit label="m">MyMeter</Unit>
                                            <Unit label="mm">u:MM</Unit>
                                        </Composite>
                                    </Format>
                                    <Format typeName="Format3" displayLabel="Format 3" roundFactor="2.3" type="Station" stationOffsetSize="12" showSignOption="SignAlways" formatTraits="Use1000Separator|ShowUnitLabel|ApplyRounding"
                                            precision="5" decimalSeparator="," thousandSeparator="." uomSeparator="(">
                                        <Composite spacer="/" includeZero="False">
                                            <Unit label="kilogram">u:KG</Unit>
                                        </Composite>
                                    </Format>
                                    <Format typeName="Format4" displayLabel="Format 4" description="Nice format 4" roundFactor="12" type="Station" stationSeparator="$" stationOffsetSize="12" showSignOption="SignAlways" formatTraits="ShowUnitLabel|ApplyRounding|ZeroEmpty"
                                            precision="2" decimalSeparator="," thousandSeparator="." uomSeparator="#">
                                        <Composite spacer="?" includeZero="True">
                                            <Unit label="Newton">u:N</Unit>
                                        </Composite>
                                    </Format>
                                </ECSchema>)xml")));

    auto assertFormat = [] (ECSchemaCR schema, Utf8CP name, Utf8CP displayLabel, Utf8CP description, JsonValue const& numericSpec, JsonValue const& compSpec)
        {
        ECFormatCP format = schema.GetFormatCP(name);
        ASSERT_TRUE(format != nullptr) << schema.GetFullSchemaName() << ":" << name;
        ASSERT_STREQ(name, format->GetName().c_str()) << format->GetFullName();
        ASSERT_STREQ(displayLabel, format->GetDisplayLabel().c_str()) << format->GetFullName();
        ASSERT_STREQ(description, format->GetDescription().c_str()) << format->GetFullName();
        if (numericSpec.m_value.isNull())
            ASSERT_FALSE(format->HasNumeric()) << format->GetFullName();
        else
            {
            ASSERT_TRUE(format->HasNumeric()) << format->GetFullName();
            Json::Value jval;
            ASSERT_TRUE(format->GetNumericSpec()->ToJson(jval, false)) << format->GetFullName();
            ASSERT_EQ(numericSpec, JsonValue(jval)) << format->GetFullName();
            }

        if (compSpec.m_value.isNull())
            ASSERT_FALSE(format->HasComposite()) << format->GetFullName();
        else
            {
            Json::Value jval;
            ASSERT_TRUE(format->GetCompositeSpec()->ToJson(jval)) << format->GetFullName();
            ASSERT_TRUE(format->HasComposite()) << format->GetFullName();
            ASSERT_EQ(compSpec, JsonValue(jval)) << format->GetFullName();
            }
        };

    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1");
    ASSERT_TRUE(schema != nullptr);

    assertFormat(*schema, "Format1", "Format 1", "",
                 JsonValue(R"json({"roundFactor":0.3, "type": "Fractional", "showSignOption": "OnlyNegative", "formatTraits": ["trailZeroes", "keepSingleZero"], "precision": 4, "decimalSeparator": ".", "thousandSeparator": ",", "uomSeparator": " "})json"),
                 JsonValue());

    assertFormat(*schema, "Format2", "Format 2", "Nice format 2",
                 JsonValue(R"json({"roundFactor":0.3, "type": "Scientific", "scientificType": "ZeroNormalized", "showSignOption": "NegativeParentheses", "formatTraits": ["keepSingleZero", "showUnitLabel", "prependUnitLabel"], "precision": 4, "decimalSeparator": ",", "thousandSeparator": ".", "uomSeparator": "*"})json"),
                 JsonValue(R"json({"includeZero": true, "units": [
                              { "name": "MyMeter", "label": "m" },
                              { "name": "MM", "label": "mm" }]})json"));

    assertFormat(*schema, "Format3", "Format 3", "",
                 JsonValue(R"json({"roundFactor":2.3, "type": "Station", "stationOffsetSize":12, "showSignOption": "SignAlways", "formatTraits":["applyRounding", "showUnitLabel", "use1000Separator"], "precision": 5, "decimalSeparator": ",", "thousandSeparator": ".", "uomSeparator": "("})json"),
                 JsonValue(R"json({"spacer": "/", "includeZero": false, "units": [
                              { "name": "KG", "label": "kilogram" }]})json"));

    assertFormat(*schema, "Format4", "Format 4", "Nice format 4",
                 JsonValue(R"json({"roundFactor":12.0, "type": "Station", "stationOffsetSize":12, "stationSeparator":"$", "showSignOption": "SignAlways", "formatTraits":["zeroEmpty", "applyRounding", "showUnitLabel"], "precision": 2, "decimalSeparator": ",", "thousandSeparator": ".", "uomSeparator": "#"})json"),
                 JsonValue(R"json({"spacer": "?", "includeZero": true, "units": [
                              { "name": "N", "label": "Newton"}]})json"));

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetPropertyCategory)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("getpropertycategories.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                     <ECSchema schemaName="Schema1" alias="s1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                        <PropertyCategory typeName="Core" description="Core" displayLabel="Core" priority="1" />
                                     </ECSchema>)xml")));


    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                     <ECSchema schemaName="Schema2" alias="s2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                     <ECSchemaReference name="Schema1" version="01.00.00" alias="s1" />
                                       <PropertyCategory typeName="Misc" description="Miscellaneous" displayLabel="Miscellaneous" />
                                       <ECEntityClass typeName="Foo" >
                                         <ECProperty propertyName="Length" typeName="double" category="s1:Core" />
                                         <ECProperty propertyName="Homepage" typeName="string" extendedTypeName="URL" />
                                         <ECArrayProperty propertyName="AlternativeLengths" typeName="double" minOccurs="0" maxOccurs="unbounded" category="Misc"/>
                                         <ECArrayProperty propertyName="Favorites" typeName="string" extendedTypeName="URL" minOccurs="0" maxOccurs="unbounded" />
                                       </ECEntityClass>
                                     </ECSchema>)xml")));
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ASSERT_EQ(BeVersion(3, 1), GetHelper().GetOriginalECXmlVersion("Schema1"));
    ASSERT_EQ(BeVersion(3, 1), GetHelper().GetOriginalECXmlVersion("Schema2"));


    PropertyCategoryCP coreCat = m_ecdb.Schemas().GetPropertyCategory("Schema1", "Core");
    ASSERT_TRUE(coreCat != nullptr);
    ASSERT_STREQ("Core", coreCat->GetName().c_str());
    ASSERT_EQ(1, (int) coreCat->GetPriority());

    {
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ECSchemaCP schema1 = m_ecdb.Schemas().GetSchema("Schema1", false);
    ASSERT_TRUE(schema1 != nullptr);
    ASSERT_EQ(0, schema1->GetPropertyCategoryCount());

    ECSchemaCP schema2 = m_ecdb.Schemas().GetSchema("Schema2", false);
    ASSERT_TRUE(schema2 != nullptr);
    ASSERT_EQ(0, schema2->GetPropertyCategoryCount());

    ECClassCP testClass = m_ecdb.Schemas().GetClass("Schema2", "Foo");
    ASSERT_TRUE(testClass != nullptr);
    ASSERT_EQ(1, schema1->GetPropertyCategoryCount());
    ASSERT_EQ(1, schema2->GetPropertyCategoryCount());

    coreCat = schema1->GetPropertyCategoryCP("Core");
    ASSERT_TRUE(coreCat != nullptr);
    ASSERT_STREQ("Core", coreCat->GetName().c_str());
    ASSERT_EQ(1, (int) coreCat->GetPriority());

    PropertyCategoryCP miscCat = schema2->GetPropertyCategoryCP("Misc");
    ASSERT_TRUE(miscCat != nullptr);
    ASSERT_STREQ("Misc", miscCat->GetName().c_str());
    ASSERT_EQ(0, (int) miscCat->GetPriority());

    ECPropertyCP prop = testClass->GetPropertyP("Length");
    ASSERT_TRUE(prop != nullptr);
    coreCat = prop->GetCategory();
    ASSERT_TRUE(coreCat != nullptr);
    ASSERT_STREQ("Core", coreCat->GetName().c_str());
    ASSERT_EQ(1, (int) coreCat->GetPriority());

    prop = testClass->GetPropertyP("AlternativeLengths");
    ASSERT_TRUE(prop != nullptr);
    miscCat = prop->GetCategory();
    ASSERT_TRUE(miscCat != nullptr);
    ASSERT_STREQ("Misc", miscCat->GetName().c_str());
    ASSERT_EQ(0, (int) miscCat->GetPriority());

    prop = testClass->GetPropertyP("Homepage");
    ASSERT_TRUE(prop != nullptr);
    ASSERT_TRUE(prop->GetCategory() == nullptr);

    prop = testClass->GetPropertyP("Favorites");
    ASSERT_TRUE(prop != nullptr);
    ASSERT_TRUE(prop->GetCategory() == nullptr);
    }

    {
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());

    ECSchemaCP schema = m_ecdb.Schemas().GetSchema("Schema1", true);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(1, schema->GetPropertyCategoryCount());

    schema = m_ecdb.Schemas().GetSchema("Schema2", true);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(1, schema->GetPropertyCategoryCount());
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetPropertyWithExtendedType)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("propertywithextendedtype.ecdb", 
                           SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                                      "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                      "  <ECEntityClass typeName='Foo' >"
                                      "    <ECProperty propertyName='Name' typeName='string' />"
                                      "    <ECProperty propertyName='Homepage' typeName='string' extendedTypeName='URL' />"
                                      "    <ECArrayProperty propertyName='Addresses' typeName='string' minOccurs='0' maxOccurs='unbounded' />"
                                      "    <ECArrayProperty propertyName='Favorites' typeName='string' extendedTypeName='URL' minOccurs='0' maxOccurs='unbounded' />"
                                      "  </ECEntityClass>"
                                      "</ECSchema>")));

    ECClassCP fooClass = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
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
    ASSERT_EQ(SUCCESS, SetupECDb("relationshipwithabstractconstraintclass.ecdb",
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
                                        </ECEntityClass>
                                        <ECEntityClass typeName="FooElement" modifier="Sealed" >
                                            <BaseClass>BaseElement</BaseClass>
                                            <ECNavigationProperty propertyName="Model2" relationshipName="ModelHasFooOrGooElements" direction="Backward"/>
                                        </ECEntityClass>
                                        <ECEntityClass typeName="GooElement" modifier="Sealed" >
                                            <BaseClass>BaseElement</BaseClass>
                                            <ECNavigationProperty propertyName="Model2" relationshipName="ModelHasFooOrGooElements" direction="Backward"/>
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
                                      </ECSchema>)xml")));

    ECClassCP ecclass = m_ecdb.Schemas().GetClass("TestSchema", "ModelHasFooOrGooElements");
    ASSERT_TRUE(ecclass != nullptr);
    ECRelationshipClassCP relWithAbstractConstraint = ecclass->GetRelationshipClassCP();
    ASSERT_TRUE(relWithAbstractConstraint != nullptr);
    
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "Model"), relWithAbstractConstraint->GetSource().GetAbstractConstraint());
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "BaseElement"), relWithAbstractConstraint->GetTarget().GetAbstractConstraint());

    ecclass = m_ecdb.Schemas().GetClass("TestSchema", "ModelHasElements");
    ASSERT_TRUE(ecclass != nullptr);
    ECRelationshipClassCP relWithoutAbstractConstraint = ecclass->GetRelationshipClassCP();
    ASSERT_TRUE(relWithoutAbstractConstraint != nullptr);
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "Model"), relWithoutAbstractConstraint->GetSource().GetAbstractConstraint());
    ASSERT_EQ(m_ecdb.Schemas().GetClass("TestSchema", "Element"), relWithoutAbstractConstraint->GetTarget().GetAbstractConstraint());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, AddDuplicateECSchemaInReadContext)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ecschemamanagertest.ecdb"));

    ECSchemaReadContextPtr context1 = nullptr;
    ASSERT_EQ(SUCCESS, ReadECSchema(context1, m_ecdb, SchemaItem::CreateForFile("BaseSchemaA.01.00.00.ecschema.xml")));
    ECSchemaReadContextPtr context2 = nullptr;
    ASSERT_EQ(SUCCESS, ReadECSchema(context2, m_ecdb, SchemaItem::CreateForFile("BaseSchemaA.01.00.00.ecschema.xml")));

    bvector<ECSchemaCP> duplicateSchemas;
    duplicateSchemas.push_back(context1->GetCache().GetSchema(SchemaKey("BaseSchemaA", 1, 0, 0), SchemaMatchType::Latest));
    duplicateSchemas.push_back(context2->GetCache().GetSchema(SchemaKey("BaseSchemaA", 1, 0, 0), SchemaMatchType::Latest));
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(duplicateSchemas));
    ECClassCP ecclass = m_ecdb.Schemas().GetClass("BaseSchemaA", "Address");
    ASSERT_TRUE(ecclass != nullptr) << "ecclass with the specified name does not exist";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, ImportDuplicateSchema)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecschemamanagertest.ecdb", SchemaItem::CreateForFile("BaseSchemaA.01.00.00.ecschema.xml")));

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem::CreateForFile("BaseSchemaA.01.00.00.ecschema.xml")));
    ECClassCP ecclass = m_ecdb.Schemas().GetClass("BaseSchemaA", "Address");
    ASSERT_TRUE(ecclass != nullptr) << "Class with the specified name doesn't exist :- ecclass is empty";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, ImportMultipleSupplementalSchemas)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("supplementalSchematest.ecdb"));
    ASSERT_EQ(SUCCESS, ImportSchemas({ SchemaItem(R"xml(<ECSchema schemaName="SchoolSchema" alias="SS" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA"/>
                <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap"/>
                <ECEntityClass typeName="School" modifier="None">
                    <ECProperty propertyName="Code" typeName="string"/>
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECProperty propertyName="FoundationDate" typeName="DateTime" />
                </ECEntityClass>
                <ECEntityClass typeName="Course" modifier="None">
                    <ECProperty propertyName="LastMod1" typeName="DateTime" readOnly="True"/>
                    <ECProperty propertyName="LastMod2" typeName="DateTime" readOnly="True"/>
                    <ECNavigationProperty propertyName="School" relationshipName="SchoolHasCourses" direction="Backward">
                        <ECCustomAttributes>
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00"/>
                        </ECCustomAttributes>
                    </ECNavigationProperty>
                </ECEntityClass>
                <ECRelationshipClass typeName="SchoolHasCourses" modifier="Sealed" strength="embedding">
                    <Source multiplicity="(0..1)" polymorphic="true" roleLabel="has">
                        <Class class="School"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="true" roleLabel="belongs to">
                        <Class class="Course"/>
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml"),

            SchemaItem(R"xml(<ECSchema schemaName="SchoolSchema_Supplemental_1" alias="SSS1" version="01.00" displayLabel="School Supplemental1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
                </ECSchema>)xml"),

            //Nav props must be declared as string properties in supplemental schemas
            SchemaItem(R"xml(<ECSchema schemaName="SchoolSchema_Supplemental_2" alias="SSS2" version="01.00" displayLabel="School Supplemental2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
                <ECEntityClass typeName="School" modifier="None"/>
                <ECEntityClass typeName="Course" modifier="None">
                    <ECCustomAttributes>
                        <ClassHasCurrentTimeStampProperty xmlns="CoreCustomAttributes.01.00">
                            <PropertyName>LastMod2</PropertyName>
                        </ClassHasCurrentTimeStampProperty>
                    </ECCustomAttributes>
                    <ECProperty propertyName="School" typeName="string">
                        <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00">
                            <OnDeleteAction>Cascade</OnDeleteAction>
                        </ForeignKeyConstraint>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECEntityClass>
                <ECRelationshipClass typeName="SchoolHasCourses" modifier="Sealed" strength="embedding">
                    <Source multiplicity="(0..1)" polymorphic="true" roleLabel="has">
                        <Class class="School"/>
                    </Source>
                    <Target multiplicity="(0..*)" polymorphic="true" roleLabel="belongs to">
                        <Class class="Course"/>
                    </Target>
                </ECRelationshipClass>
                </ECSchema>)xml")}));

    ECClassCP schoolClass = m_ecdb.Schemas().GetClass("SchoolSchema", "School");
    ASSERT_TRUE(schoolClass != nullptr);
    ECClassCP courseClass = m_ecdb.Schemas().GetClass("SchoolSchema", "Course");
    ASSERT_TRUE(courseClass != nullptr);

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
    ECPropertyCP schoolProp = courseClass->GetPropertyP("School");
    ASSERT_TRUE(schoolProp != nullptr);
    caCount = 0;
    for (IECInstancePtr ca : schoolProp->GetCustomAttributes(false))
        {
        caCount++;
        ASSERT_STREQ("ForeignKeyConstraint", ca->GetClass().GetName().c_str()) << "ECProperty Course.School";
        ECValue v;
        ASSERT_EQ(ECObjectsStatus::Success, ca->GetValue(v, "OnDeleteAction")) << "ECProperty Course.School";
        ASSERT_STREQ("Cascade", v.GetUtf8CP()) << "ECProperty Course.School";
        }
    ASSERT_EQ(1, caCount) << "ECProperty Course.School";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, GetClass)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecschemamanagertest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    ECClassCP ecClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSA");
    EXPECT_TRUE(ecClass != nullptr);
    ecClass = m_ecdb.Schemas().GetClass("ecsql", "PSA", SchemaLookupMode::ByAlias);
    EXPECT_TRUE(ecClass != nullptr);

    ecClass = m_ecdb.Schemas().GetClass("ECSqlTest", "PSA", SchemaLookupMode::AutoDetect);
    EXPECT_TRUE(ecClass != nullptr);

    ecClass = m_ecdb.Schemas().GetClass("ecsql", "PSA", SchemaLookupMode::AutoDetect);
    EXPECT_TRUE(ecClass != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  05/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, ClassLocater)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecschemamanagertest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    
    ECClassCP ecClass = m_ecdb.GetClassLocater().LocateClass("ECSqlTest", "PSA");
    EXPECT_TRUE(ecClass != nullptr) << "Look up by schema name";
    EXPECT_STRCASEEQ("ECSqlTest", ecClass->GetSchema().GetName().c_str()) << "Look up by schema name";
    EXPECT_STRCASEEQ("PSA", ecClass->GetName().c_str()) << "Look up by schema name";

    ecClass = m_ecdb.GetClassLocater().LocateClass("ecsql", "PSA");
    EXPECT_TRUE(ecClass != nullptr) << "Look up by schema alias";
    EXPECT_STRCASEEQ("ECSqlTest", ecClass->GetSchema().GetName().c_str()) << "Look up by schema alias";
    EXPECT_STRCASEEQ("PSA", ecClass->GetName().c_str()) << "Look up by schema alias";

    ECClassId idBySchemaName = m_ecdb.GetClassLocater().LocateClassId("ECSqlTest", "PSA");
    EXPECT_TRUE(idBySchemaName.IsValid()) << "Look up by schema name";
    EXPECT_EQ(idBySchemaName, m_ecdb.GetClassLocater().LocateClassId("ecsql", "PSA"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, Supplementation_DifferentPrecedences)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("supplementalschematest.ecdb"));

    ECSchemaReadContextPtr context = nullptr;
    ASSERT_EQ(SUCCESS, ReadECSchema(context, m_ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" alias="BS" version="01.70" displayLabel="Test Schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="Base" modifier="None">
                    <ECProperty propertyName="lastmod" typeName="datetime"/>
                </ECEntityClass>
            </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, ReadECSchema(context, m_ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema_Supplemental" alias="BSS" version="01.60" displayLabel="TestSchema Supplemental" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA"/>
                  <ECCustomAttributes>
                    <SupplementalSchema xmlns="CoreCustomAttributes.01.00">
                        <PrimarySchemaReference>
                            <SchemaName>TestSchema</SchemaName>
                            <MajorVersion>1</MajorVersion>
                            <WriteVersion>0</WriteVersion>
                            <MinorVersion>70</MinorVersion>
                        </PrimarySchemaReference>
                        <Precedence>51</Precedence>
                        <Purpose>Test</Purpose>
                    </SupplementalSchema>
                </ECCustomAttributes>
                <ECEntityClass typeName="Base" modifier="None">
                    <ECProperty propertyName="lastmod" typeName="datetime">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00">
                                <DateTimeComponent>Date</DateTimeComponent>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECEntityClass>
            </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, ReadECSchema(context, m_ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema_Supplemental" alias="BSS" version="01.90" displayLabel="Test Schema Supplemental" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA"/>
                  <ECCustomAttributes>
                    <SupplementalSchema xmlns="CoreCustomAttributes.01.00">
                        <PrimarySchemaReference>
                            <SchemaName>TestSchema</SchemaName>
                            <MajorVersion>1</MajorVersion>
                            <WriteVersion>0</WriteVersion>
                            <MinorVersion>70</MinorVersion>
                        </PrimarySchemaReference>
                        <Precedence>52</Precedence>
                        <Purpose>Test</Purpose>
                    </SupplementalSchema>
                </ECCustomAttributes>
                <ECEntityClass typeName="Base" modifier="None">
                    <ECProperty propertyName="lastmod" typeName="datetime">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00">
                                <DateTimeKind>Utc</DateTimeKind>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECEntityClass>
            </ECSchema>)xml")));
   

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "Base");
    ASSERT_TRUE(testClass != nullptr);
    ECPropertyCP testProperty = testClass->GetPropertyP("lastmod");
    ASSERT_TRUE(testProperty != nullptr);

    IECInstancePtr ca = testProperty->GetCustomAttribute("CoreCustomAttributes", "DateTimeInfo");
    ASSERT_TRUE(ca != nullptr);
    ECValue v;
    ASSERT_EQ(ECObjectsStatus::Success, ca->GetValue(v, "DateTimeKind"));
    ASSERT_FALSE(v.IsNull()) << "DateTimeInfo::DateTimeKind. Expected to get CA from schema with higher precedence";
    ASSERT_STREQ("Utc",v.GetUtf8CP()) << "DateTimeInfo::DateTimeKind. Expected to get CA from schema with higher precedence";
    v.Clear();
    ASSERT_EQ(ECObjectsStatus::Success, ca->GetValue(v, "DateTimeComponent"));
    ASSERT_TRUE(v.IsNull()) << "DateTimeInfo::DateTimeComponent";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, Supplementation_SuppSchemaTargetsNewerPrimaryMajorVersion)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("supplementalschematest.ecdb"));

    ECSchemaReadContextPtr context = nullptr;
    ASSERT_EQ(SUCCESS, ReadECSchema(context, m_ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" alias="BS" version="01.70" displayLabel="TestSchema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="Base" modifier="None">
                    <ECProperty propertyName="lastmod" typeName="datetime"/>
                </ECEntityClass>
            </ECSchema>)xml")));
    ASSERT_EQ(SUCCESS, ReadECSchema(context, m_ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema_Supplemental" alias="BSS" version="01.90" displayLabel="TestSchema Supplemental" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA"/>
                  <ECCustomAttributes>
                    <SupplementalSchema xmlns="CoreCustomAttributes.01.00">
                        <PrimarySchemaReference>
                            <SchemaName>TestSchema</SchemaName>
                            <MajorVersion>1</MajorVersion>
                            <WriteVersion>0</WriteVersion>
                            <MinorVersion>70</MinorVersion>
                        </PrimarySchemaReference>
                        <Precedence>52</Precedence>
                        <Purpose>Test</Purpose>
                    </SupplementalSchema>
                </ECCustomAttributes>
                <ECEntityClass typeName="Base" modifier="None">
                    <ECProperty propertyName="lastmod" typeName="datetime">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00">
                                <DateTimeKind>Utc</DateTimeKind>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECEntityClass>
            </ECSchema>)xml")));
    ASSERT_EQ(SUCCESS, ReadECSchema(context, m_ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema_Supplemental" alias="BSS" version="02.10" displayLabel="TestSchema Supplemental" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA"/>
                  <ECCustomAttributes>
                    <SupplementalSchema xmlns="CoreCustomAttributes.01.00">
                        <PrimarySchemaReference>
                            <SchemaName>TestSchema</SchemaName>
                            <MajorVersion>2</MajorVersion>
                            <WriteVersion>0</WriteVersion>
                            <MinorVersion>70</MinorVersion>
                        </PrimarySchemaReference>
                        <Precedence>53</Precedence>
                        <Purpose>Test</Purpose>
                    </SupplementalSchema>
                </ECCustomAttributes>
                <ECEntityClass typeName="Base" modifier="None">
                    <ECProperty propertyName="lastmod" typeName="datetime">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00">
                                <DateTimeComponent>Date</DateTimeComponent>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECEntityClass>
            </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema","Base");
    ASSERT_TRUE(testClass != nullptr);
    ECPropertyCP testProperty = testClass->GetPropertyP("lastmod");
    ASSERT_TRUE(testProperty != nullptr);

    IECInstancePtr ca = testProperty->GetCustomAttribute("CoreCustomAttributes", "DateTimeInfo");
    ASSERT_TRUE(ca != nullptr);
    ECValue v;
    ASSERT_EQ(ECObjectsStatus::Success, ca->GetValue(v, "DateTimeKind"));
    ASSERT_FALSE(v.IsNull()) << "DateTimeInfo::DateTimeKind. Expected to get CA from schema with matching major version";
    ASSERT_STREQ("Utc", v.GetUtf8CP()) << "DateTimeInfo::DateTimeKind. Expected to get CA from schema with matching major version";
    v.Clear();
    ASSERT_EQ(ECObjectsStatus::Success, ca->GetValue(v, "DateTimeComponent"));
    ASSERT_TRUE(v.IsNull()) << "DateTimeInfo::DateTimeComponent";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, Supplementation_SuppSchemaTargetsNewerPrimaryMinorVersion)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("supplementalschematest.ecdb"));

    ECSchemaReadContextPtr context = nullptr;
    ASSERT_EQ(SUCCESS, ReadECSchema(context, m_ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" alias="BS" version="01.69" displayLabel="TestSchema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEntityClass typeName="Base" modifier="None">
                    <ECProperty propertyName="lastmod" typeName="datetime"/>
                </ECEntityClass>
            </ECSchema>)xml")));
    ASSERT_EQ(SUCCESS, ReadECSchema(context, m_ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema_Supplemental" alias="BSS" version="01.69" displayLabel="TestSchema Supplemental" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA"/>
                  <ECCustomAttributes>
                    <SupplementalSchema xmlns="CoreCustomAttributes.01.00">
                        <PrimarySchemaReference>
                            <SchemaName>TestSchema</SchemaName>
                            <MajorVersion>1</MajorVersion>
                            <WriteVersion>0</WriteVersion>
                            <MinorVersion>69</MinorVersion>
                        </PrimarySchemaReference>
                        <Precedence>50</Precedence>
                        <Purpose>Test</Purpose>
                    </SupplementalSchema>
                </ECCustomAttributes>
                <ECEntityClass typeName="Base" modifier="None">
                    <ECProperty propertyName="lastmod" typeName="datetime">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00">
                                <DateTimeKind>Utc</DateTimeKind>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECEntityClass>
            </ECSchema>)xml")));
    ASSERT_EQ(SUCCESS, ReadECSchema(context, m_ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema_Supplemental" alias="BSS" version="01.90" displayLabel="TestSchema Supplemental" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA"/>
                  <ECCustomAttributes>
                    <SupplementalSchema xmlns="CoreCustomAttributes.01.00">
                        <PrimarySchemaReference>
                            <SchemaName>TestSchema</SchemaName>
                            <MajorVersion>1</MajorVersion>
                            <WriteVersion>0</WriteVersion>
                            <MinorVersion>70</MinorVersion>
                        </PrimarySchemaReference>
                        <Precedence>52</Precedence>
                        <Purpose>Test</Purpose>
                    </SupplementalSchema>
                </ECCustomAttributes>
                <ECEntityClass typeName="Base" modifier="None">
                    <ECProperty propertyName="lastmod" typeName="datetime">
                        <ECCustomAttributes>
                            <DateTimeInfo xmlns="CoreCustomAttributes.01.00">
                                <DateTimeComponent>Date</DateTimeComponent>
                            </DateTimeInfo>
                        </ECCustomAttributes>
                    </ECProperty>
                </ECEntityClass>
            </ECSchema>)xml")));

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas()));
    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "Base");
    ASSERT_TRUE(testClass != nullptr);
    ECPropertyCP testProperty = testClass->GetPropertyP("lastmod");
    ASSERT_TRUE(testProperty != nullptr);

    IECInstancePtr ca = testProperty->GetCustomAttribute("CoreCustomAttributes", "DateTimeInfo");
    ASSERT_TRUE(ca != nullptr);
    ECValue v;
    ASSERT_EQ(ECObjectsStatus::Success, ca->GetValue(v, "DateTimeKind")) << "DateTimeInfo::DateTimeKind. Expected to get CA from schema with matching major version";
    ASSERT_TRUE(v.IsNull()) << "DateTimeInfo::DateTimeKind. Expected to get CA from schema with matching major version";
    v.Clear();
    ASSERT_EQ(ECObjectsStatus::Success, ca->GetValue(v, "DateTimeComponent"));
    ASSERT_FALSE(v.IsNull()) << "DateTimeInfo::DateTimeComponent";
    ASSERT_STREQ("Date", v.GetUtf8CP()) << "DateTimeInfo::DateTimeKind. Expected to get CA from schema with matching major version";
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
DbResult PrepareECClassViews(bmap<Utf8String, bset<Utf8String>>& ecclassViewInfo, ECDbCR ecdb)
    {
    ecclassViewInfo.clear();

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
        if (BE_SQLITE_OK != stat)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Affan Khan                        12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, CreateECClassViews)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("createecclassviews.ecdb"));

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());
    m_ecdb.SaveChanges();
    bmap<Utf8String, bset<Utf8String>> schemasWithECClassViews;
    ASSERT_EQ(BE_SQLITE_OK, PrepareECClassViews(schemasWithECClassViews, m_ecdb));

    ASSERT_EQ(2, schemasWithECClassViews.size()) << "Unexpected number of schemas with ECClassViews";
    ASSERT_EQ(4, schemasWithECClassViews["ecdbf"].size()) << "Unexpected number of ECClassViews";

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));
    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());
    m_ecdb.SaveChanges();
    ASSERT_EQ(BE_SQLITE_OK, PrepareECClassViews(schemasWithECClassViews, m_ecdb));
    ASSERT_EQ(3, schemasWithECClassViews.size()) << "Unexpected number of schemas with ECClassViews";
    ASSERT_EQ(4, schemasWithECClassViews["ecdbf"].size()) << "Unexpected number of ECClassViews";
    ASSERT_EQ(26, schemasWithECClassViews["stco"].size()) << "Unexpected number of ECClassViews";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                   12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, CreateECClassViewsForSubsetOfClasses)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("createecclassviewspartially.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM meta.ECClassDef WHERE Name IN ('FileInfo', 'FileInfoOwnership', 'AAA','Cubicle', 'RelationWithLinkTableMapping')"));
    bvector<ECClassId> classIds;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ECClassId id = stmt.GetValueId<ECClassId>(0);
        ASSERT_TRUE(id.IsValid());
        classIds.push_back(id);
        }

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb(classIds));
    m_ecdb.SaveChanges();

    bmap<Utf8String, bset<Utf8String>> schemasWithECClassViews;
    ASSERT_EQ(BE_SQLITE_OK, PrepareECClassViews(schemasWithECClassViews, m_ecdb));

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
    ASSERT_EQ(SUCCESS, SetupECDb("createecclassviewssharedcols.ecdb",
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
            "</ECSchema>")));
    ASSERT_EQ(SUCCESS, PopulateECDb(3));

    ASSERT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());
    bmap<Utf8String, bset<Utf8String>> classViewInfo;
    ASSERT_EQ(BE_SQLITE_OK, PrepareECClassViews(classViewInfo, m_ecdb));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                   12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, CreateECClassViewsForInvalidClasses)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("createecclassviewsforinvalidclasses.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM meta.ECClassDef WHERE Name IN ('AnglesStruct', 'ClassMap', 'AClassThatDoesNotGetMappedToDb')"));
    bvector<ECClassId> classIds;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ECClassId id = stmt.GetValueId<ECClassId>(0);
        ASSERT_TRUE(id.IsValid());
        classIds.push_back(id);
        }

    ASSERT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb(classIds));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad.Hassan                   12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, CreateECClassViewsForCombinationofValidInvalidClasses)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("createecclassviewsforvalidinvalidclasses.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT ECInstanceId FROM meta.ECClassDef WHERE Name IN ('AAA', 'AnglesStruct', 'AClassThatDoesNotGetMappedToDb')"));
    bvector<ECClassId> classIds;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ECClassId id = stmt.GetValueId<ECClassId>(0);
        ASSERT_TRUE(id.IsValid());
        classIds.push_back(id);
        }

    ASSERT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb(classIds));
    m_ecdb.SaveChanges();

    // Class view will be created for the provided list of ECClassIds until it get the first invalid one
    // so ClassView only for class "AAA" will exist.
    std::map<Utf8String, std::set<Utf8String>> schemasWithECClassViews = GetECViewNamesByPrefix(m_ecdb);
    ASSERT_EQ(1, schemasWithECClassViews.size()) << "Unexpected number of schemas with ECClassViews";

    auto it = schemasWithECClassViews.find("stco");
    ASSERT_TRUE(it != schemasWithECClassViews.end());
    std::set<Utf8String> const& stcoViews = it->second;
    ASSERT_EQ(1, stcoViews.size());
    ASSERT_TRUE(stcoViews.find(Utf8String("[stco.AAA]")) != stcoViews.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                       Krischan.Eberle                  10/14
+---------------+---------------+---------------+---------------+---------------+------*/
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

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("importschemawithsubclassestoexistingschema1.ecdb"));

    ECInstanceKey activityKey;
    setup(activityKey, m_ecdb);
    ASSERT_TRUE(activityKey.IsValid());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT PlanId, OutlineIndex FROM p.Activity WHERE ECInstanceId=?"));
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
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbgeometrytypes.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                          "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "    <ECEntityClass typeName='Foo' >"
                          "        <ECProperty propertyName='g1' typeName='Bentley.Geometry.Common.IGeometry' />"
                          "        <ECProperty propertyName='g2' typeName='Bentley.GeometryNET.Common.IGeometry' />"
                          "        <ECProperty propertyName='g3' typeName='Bentley.GeometryNET.Common.ICoordinate' />"
                          "    </ECEntityClass>"
                          "</ECSchema>")));

    ECClassCP cl = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(g1,g2,g3) VALUES(?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(1, *g1));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(2, *g2));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(3, *g3));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    m_ecdb.SaveChanges();
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT g1,g2,g3 FROM ts.Foo WHERE ECInstanceId=?"));
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
    ASSERT_EQ(SUCCESS, SetupECDb("propertywithEnumerationType.ecdb",
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
                                      "</ECSchema>")));

    //non strict enum Insert tests
    ECSqlStatement statement;
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.File(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 2));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.File(Type) VALUES(0)"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.File(Type) VALUES(2)"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    //non strict enum Update tests
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "UPDATE ts.File SET Type=1 WHERE Type=0"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "UPDATE ts.File SET Type=3 WHERE Type=1"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "UPDATE ts.File SET Type=? WHERE Type=?"));
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
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.Folder(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 0));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Reset();
    statement.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, statement.BindInt(1, 2));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step()) << "ECDb does not enforce strict enums, so inserting a wrong value is expected to not fail.";
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.Folder(Type) VALUES(1)"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "INSERT INTO ts.Folder(Type) VALUES(2)"));
    ASSERT_EQ(BE_SQLITE_DONE, statement.Step()) << "ECDb does not enforce strict enums, so inserting a wrong value is expected to not fail.";
    statement.Finalize();

    //strict enum Update tests
    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "UPDATE ts.Folder SET Type=1 WHERE Type=0"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "UPDATE ts.Folder SET Type=2 WHERE Type=1"));
    ASSERT_EQ(DbResult::BE_SQLITE_DONE, statement.Step());
    statement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, statement.Prepare(m_ecdb, "UPDATE ts.Folder SET Type=? WHERE Type=?"));
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

/*---------------------------------------------------------------------------------**//**
* @bsitest                                       Krischan.Eberle                  10/14
+---------------+---------------+---------------+---------------+---------------+------*/
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


    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("duplicateInMemorySchemaTest.ecdb"));
    ASSERT_EQ(BentleyStatus::SUCCESS, m_ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas()));

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(usr, usrXml, *readContext));
    ASSERT_EQ(BentleyStatus::SUCCESS, m_ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas())) << "Failed because locater was not added for schemas that already exist in ECDb";
    }

END_ECDBUNITTESTS_NAMESPACE

