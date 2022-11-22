/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <set>

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE


struct SchemaManagerTests : ECDbTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, ImportToken)
    {
    auto assertImport = [this] (Utf8CP ecschemaXml, BeFileNameCR seedFilePath)
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, CloneECDb(ecdb, (Utf8String(seedFilePath.GetFileNameWithoutExtension()) + "_unrestricted.ecdb").c_str(), seedFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));

        ASSERT_EQ(SUCCESS, TestHelper(ecdb).ImportSchema(SchemaItem(ecschemaXml))) << "SchemaImport into unrestricted ECDb failed unexpectedly for: " << ecschemaXml;
        ecdb.SaveChanges();
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, SchemaImportOptionsDoNotOverlap)
    {
    ASSERT_EQ(0, (int)(SchemaManager::SchemaImportOptions::None & SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues));
    ASSERT_EQ(0, (int)(SchemaManager::SchemaImportOptions::None & SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade));
    ASSERT_EQ(0, (int)(SchemaManager::SchemaImportOptions::None & SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications));
    ASSERT_EQ(0, (int)(SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues & SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade));
    ASSERT_EQ(0, (int)(SchemaManager::SchemaImportOptions::DoNotFailSchemaValidationForLegacyIssues & SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications));
    ASSERT_EQ(0, (int)(SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade & SchemaManager::SchemaImportOptions::DoNotFailForDeletionsOrModifications));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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

    //! can count entries in any git revision using bash like so:
    //! > cd $SrcRoot/BisSchemas
    //! > git show master:Standard/Units.ecschema.xml |  grep '<Phenomenon ' | wc -l
    //! replace "Phenom" with other xml tags
    //! the units count includes inverted and constants
    const int standardUnitSystemCount = 12;
    const int standardPhenCount = 79;
    const int standardUnitCount = 518;
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
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
            BeJsDocument jval;
            ASSERT_TRUE(format->GetNumericSpec()->ToJson(jval, false)) << format->GetFullName();
            ASSERT_EQ(numericSpec, JsonValue(jval)) << format->GetFullName();
            }

        if (compSpec.m_value.isNull())
            ASSERT_FALSE(format->HasComposite()) << format->GetFullName();
        else
            {
            BeJsDocument jval;
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
// @bsiclass
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
// @bsiclass
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
// @bsiclass
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
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, ImportDuplicateSchema)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecschemamanagertest.ecdb", SchemaItem::CreateForFile("BaseSchemaA.01.00.00.ecschema.xml")));

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem::CreateForFile("BaseSchemaA.01.00.00.ecschema.xml")));
    ECClassCP ecclass = m_ecdb.Schemas().GetClass("BaseSchemaA", "Address");
    ASSERT_TRUE(ecclass != nullptr) << "Class with the specified name doesn't exist :- ecclass is empty";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
                            <ForeignKeyConstraint xmlns="ECDbMap.02.00">
                                <OnDeleteAction>Cascade</OnDeleteAction>
                            </ForeignKeyConstraint>
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
* @bsitest
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
// @bsimethod
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

TEST_F(SchemaManagerTests, SchemaChangeEvent)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("schema_events.ecdb"));

    uint32_t beforeEventCount = 0;
    uint32_t afterEventCount = 0;

    auto e1 = m_ecdb.Schemas().OnBeforeSchemaChanges().AddListener([&](ECDbCR ecdb, SchemaChangeType type){ ++beforeEventCount; });
    auto e2 = m_ecdb.Schemas().OnAfterSchemaChanges().AddListener([&](ECDbCR ecdb, SchemaChangeType type){ ++afterEventCount; });

    ASSERT_EQ(m_ecdb.Schemas().OnBeforeSchemaChanges().Count(), 1);
    ASSERT_EQ(m_ecdb.Schemas().OnAfterSchemaChanges().Count(), 1);

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                          "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "    <ECEntityClass typeName='Foo' />"
                          "</ECSchema>")));

    // success
    ASSERT_EQ(beforeEventCount, afterEventCount);
    ASSERT_EQ(beforeEventCount, 1);
    ASSERT_EQ(afterEventCount, 1);


    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                          "<ECSchema schemaName='TestSchema1' nameSpacePrefix='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "    <ECEntityClass typeName='Foo' />"
                          "</ECSchema>")));

    // success
    ASSERT_EQ(beforeEventCount, afterEventCount);
    ASSERT_EQ(beforeEventCount, 2);
    ASSERT_EQ(afterEventCount, 2);


    ASSERT_EQ(ERROR, ImportSchema(SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                          "<ECSchema schemaName='TestSchema1' nameSpacePrefix='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                          "</ECSchema>")));

    // error
    ASSERT_EQ(beforeEventCount, afterEventCount);
    ASSERT_EQ(beforeEventCount, 3);
    ASSERT_EQ(afterEventCount, 3);

    e1(); // remove before event handler
    e2(); // remove after event handler

    ASSERT_EQ(m_ecdb.Schemas().OnBeforeSchemaChanges().Count(), 0);
    ASSERT_EQ(m_ecdb.Schemas().OnAfterSchemaChanges().Count(), 0);

    m_ecdb.SaveChanges();
    }
//--------------------------------------------------------------------------------------
// @bsimethod
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
* @bsitest
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

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaManagerTests, ForeignKeyOnWrongSide)
    {
    // Bug354429, when trying to use a navigation property with the foreign key being on the other side of the relationship, should throw an error
    // Importing this schema produced a crash before fix 354429
    Utf8CP schemaXml = R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="NavPropertyBug354429" alias="cbg" version="01.00.00" displayLabel="TFS Bug 354429 reproduce schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
            <ECNavigationProperty propertyName="NavD" relationshipName="TheRelationship" direction="forward" />
          </ECEntityClass>
          <ECEntityClass typeName="D" />
          <ECRelationshipClass typeName="TheRelationship" strength="embedding" strengthDirection="forward" modifier="Sealed">
            <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true"><Class class="A"/></Source>
            <Target multiplicity="(0..1)" roleLabel="is owned by parent" polymorphic="true"><Class class="D"/></Target>
          </ECRelationshipClass>
        </ECSchema>
        )schema";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *readContext));


    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("bug354429foreignKey.ecdb"));
    auto result = m_ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas());
    ASSERT_EQ(BentleyStatus::ERROR, result); // We expect the schema to fail because foreign key is on the wrong side
    // Previously this caused the application to crash (SEHException, access violation)
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaManagerTests, ImportForwardNavPropertyOnStrengDirectionBackward)
    {
    // This is the counterpart to the previous test above "ForeignKeyOnWrongSide", while the other should return an ERROR, this
    // Schema variation should return SUCCESS.
    Utf8CP schemaXml = R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="NavPropertyBug354429" alias="cbg" version="01.00.00" displayLabel="TFS Bug 354429 reproduce schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
            <ECNavigationProperty propertyName="NavD" relationshipName="TheRelationship" direction="forward" />
          </ECEntityClass>
          <ECEntityClass typeName="D" />
          <ECRelationshipClass typeName="TheRelationship" strength="embedding" strengthDirection="backward" modifier="Sealed">
            <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true"><Class class="A"/></Source>
            <Target multiplicity="(0..1)" roleLabel="is owned by parent" polymorphic="true"><Class class="D"/></Target>
          </ECRelationshipClass>
        </ECSchema>
        )schema";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *readContext));


    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("bug354429foreignKey.ecdb"));
    auto result = m_ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas());
    ASSERT_EQ(BentleyStatus::SUCCESS, result);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaManagerTests, ForeignKeyOnWrongSideUsingDefaultStrengthDirection)
    {
    // Variation of test above "ForeignKeyOnWrongSide", but not specifying the strengthDirection, instead using default.
    Utf8CP schemaXml = R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="NavPropertyBug354429" alias="cbg" version="01.00.00" displayLabel="TFS Bug 354429 reproduce schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
          <ECEntityClass typeName="A">
            <ECNavigationProperty propertyName="NavD" relationshipName="TheRelationship" direction="forward" />
          </ECEntityClass>
          <ECEntityClass typeName="D" />
          <ECRelationshipClass typeName="TheRelationship" strength="embedding" modifier="Sealed">
            <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true"><Class class="A"/></Source>
            <Target multiplicity="(0..1)" roleLabel="is owned by parent" polymorphic="true"><Class class="D"/></Target>
          </ECRelationshipClass>
        </ECSchema>
        )schema";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *readContext));


    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("bug354429foreignKey.ecdb"));
    auto result = m_ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas());
    ASSERT_EQ(BentleyStatus::ERROR, result); // strength direction defaults to forward, so this should fail same as scenario #1
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
static void createV3_2SchemaWithAllItemTypes(ECSchemaPtr& schema)
    {
    ASSERT_EQ(ECN::ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "TestSchema3_2", "ts_tt", 1, 0, 0, ECVersion::V3_2))
        << "Should successfully create a schema";
    schema->SetOriginalECXmlVersion(3, 2);
    schema->AddReferencedSchema(*ECDbTestFixture::GetUnitsSchema());
    schema->AddReferencedSchema(*ECDbTestFixture::GetFormatsSchema());

    ECEntityClassP baseEntityClass;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(baseEntityClass, "baseEntityClass"));

    ECEntityClassP entityClass;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(entityClass, "entityClass"));
    entityClass->SetClassModifier(ECClassModifier::Sealed);
    entityClass->SetDisplayLabel("ExampleEntity");
    entityClass->SetDescription("An example entity class.");
    entityClass->AddBaseClass(*baseEntityClass);

    ECEntityClassP mixinA;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateMixinClass(mixinA, "mixinA", *entityClass));
    entityClass->AddBaseClass(*mixinA);

    ECEntityClassP mixinB;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateMixinClass(mixinB, "mixinB", *entityClass));
    entityClass->AddBaseClass(*mixinB);

    ECStructClassP structClass;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateStructClass(structClass, "structClass"));
    structClass->SetClassModifier(ECClassModifier::Sealed);

    PrimitiveArrayECPropertyP primArrProp;
    ASSERT_EQ(ECObjectsStatus::Success, structClass->CreatePrimitiveArrayProperty(primArrProp, "primArrProp"));
    primArrProp->SetPrimitiveElementType(ECN::PRIMITIVETYPE_Integer);
    primArrProp->SetMinimumValue(ECValue((int32_t) 7));
    primArrProp->SetMaximumValue(ECValue((int32_t) 20));
    primArrProp->SetMinOccurs(10);
    primArrProp->SetMaxOccurs(25);
    primArrProp->SetExtendedTypeName("FooBar");
    primArrProp->SetDisplayLabel("ExPrimitiveArray");

    PrimitiveECPropertyP primProp;
    ASSERT_EQ(ECObjectsStatus::Success, structClass->CreatePrimitiveProperty(primProp, "primProp", PRIMITIVETYPE_String));

    ECEntityClassP sourceClass;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(sourceClass, "sourceClass"));

    ECEntityClassP targetClass;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(targetClass, "targetClass"));

    ECRelationshipClassP relationshipClass;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateRelationshipClass(relationshipClass, "relationshipClass"));
    relationshipClass->SetClassModifier(ECClassModifier::Sealed);
    relationshipClass->SetStrength(StrengthType::Embedding);
    relationshipClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationshipClass->GetSource().AddClass(*sourceClass);
    relationshipClass->GetSource().SetRoleLabel("source roleLabel");
    relationshipClass->GetSource().SetIsPolymorphic(true);
    relationshipClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relationshipClass->GetTarget().AddClass(*targetClass);
    relationshipClass->GetTarget().SetRoleLabel("target roleLabel");
    relationshipClass->GetSource().SetIsPolymorphic(true);
    relationshipClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroOne());

    ECCustomAttributeClassP customAttrClass;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateCustomAttributeClass(customAttrClass, "customAttrClass"));
    customAttrClass->SetClassModifier(ECClassModifier::Sealed);
    customAttrClass->SetContainerType(ECN::CustomAttributeContainerType::Schema | ECN::CustomAttributeContainerType::AnyProperty);

    UnitSystemP unitSystem;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateUnitSystem(unitSystem, "unitSystem", "ExampleUnitSystemLabel", "ExampleUnitSystemDescription"));

    PhenomenonP phenom;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreatePhenomenon(phenom, "phenom", "LENGTH", "ExamplePhenomenonLabel", "ExamplePhenomenonDescription"));

    ECUnitP unit;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateUnit(unit, "unit", "[MILLI]M", *phenom, *unitSystem, 10.0, 1.0, 1.0, "ExampleUnitLabel", "ExampleUnitDescription"));

    ECUnitP constant;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateConstant(constant, "constant", "ONE", *phenom, 10.0, "ExampleConstantLabel", "ExampleConstantDescription"));

    ECUnitP invertedUnit;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateInvertedUnit(invertedUnit, *unit, "invertedUnit", *unitSystem, "ExampleInvertedUnitLabel", "ExampleInvertedUnitDescription"));

    ECFormatP format;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateFormat(format, "format", "ExampleFormatLabel", "ExampleFormatDescription"));
    Formatting::NumericFormatSpec numspec;
    format->SetNumericSpec(numspec);
    ASSERT_FALSE(format->IsProblem());

    KindOfQuantityP koq;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateKindOfQuantity(koq, "koq"));
    koq->SetPersistenceUnit(*unit);
    koq->SetDefaultPresentationFormat(*ECDbTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU"), nullptr, unit, "example");
    koq->SetRelativeError(3);

    ECEnumerationP enumeration;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEnumeration(enumeration, "enumeration", PrimitiveType::PRIMITIVETYPE_Integer));
    enumeration->SetIsStrict(true);

    ECEnumeratorP enumeratorA, enumeratorB, enumeratorC;
    ASSERT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumeratorA, "enumeratorA", 1));
    enumeratorA->SetDisplayLabel("EnumA");
    ASSERT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumeratorB, "enumeratorB", 2));
    enumeratorB->SetDisplayLabel("EnumB");
    ASSERT_EQ(ECObjectsStatus::Success, enumeration->CreateEnumerator(enumeratorC, "enumeratorC", 3));
    enumeratorC->SetDisplayLabel("EnumC");

    PropertyCategoryP propCateg;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreatePropertyCategory(propCateg, "propCateg"));
    propCateg->SetPriority(5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaManagerTests, PruneImportedNamelessPropertyCategories)
    {
    ECSchemaPtr schema;
    createV3_2SchemaWithAllItemTypes(schema);
        {
        PropertyCategoryP namelessPropCateg;
        ASSERT_EQ(ECObjectsStatus::Success, schema->CreatePropertyCategory(namelessPropCateg, ""));
        namelessPropCateg->SetPriority(5);

        ECClassP structClass = schema->GetClassP("structClass");

        PrimitiveECPropertyP namelessReferencingProp;
        ASSERT_EQ(ECObjectsStatus::Success, structClass->CreatePrimitiveProperty(namelessReferencingProp, "namelessReferencingProp", PRIMITIVETYPE_Integer));
        namelessReferencingProp->SetCategory(namelessPropCateg);

        PropertyCategoryCP propCateg = schema->GetPropertyCategoryCP("propCateg");
        ASSERT_NE(nullptr, propCateg);

        PrimitiveECPropertyP namedReferencingProp;
        ASSERT_EQ(ECObjectsStatus::Success, structClass->CreatePrimitiveProperty(namedReferencingProp, "namedReferencingProp", PRIMITIVETYPE_String));
        namedReferencingProp->SetCategory(propCateg);
        }

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("prune_imported_nameless_property_categories.ecdb"));
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(schema)) << "Importing schema should succeed";
    ECSchemaCP storedSchema = m_ecdb.Schemas().GetSchema("TestSchema3_2");
    ECClassCP structClass = storedSchema->GetClassCP("structClass");
        {
        ASSERT_NE(nullptr, storedSchema) << "schema should be found after import";

        ASSERT_NE(nullptr, storedSchema->GetClassCP("baseEntityClass"));
        ASSERT_NE(nullptr, storedSchema->GetClassCP("entityClass"));
        ASSERT_NE(nullptr, storedSchema->GetClassCP("mixinA"));
        ASSERT_NE(nullptr, storedSchema->GetClassCP("mixinB"));
        ASSERT_NE(nullptr, storedSchema->GetClassCP("sourceClass"));
        ASSERT_NE(nullptr, storedSchema->GetClassCP("targetClass"));

        ASSERT_NE(nullptr, structClass);

        ASSERT_NE(nullptr, structClass->GetPropertyP("primArrProp"));
        ASSERT_NE(nullptr, structClass->GetPropertyP("primProp"));

        ASSERT_NE(nullptr, storedSchema->GetClassCP("relationshipClass"));

        ASSERT_NE(nullptr, storedSchema->GetClassCP("customAttrClass"));

        ASSERT_NE(nullptr, storedSchema->GetUnitSystemCP("unitSystem"));

        ASSERT_NE(nullptr, storedSchema->GetPhenomenonCP("phenom"));

        ASSERT_NE(nullptr, storedSchema->GetUnitCP("unit"));
        ASSERT_NE(nullptr, storedSchema->GetUnitCP("constant"));
        ASSERT_NE(nullptr, storedSchema->GetUnitCP("invertedUnit"));

        ASSERT_NE(nullptr, storedSchema->GetFormatCP("format"));

        ASSERT_NE(nullptr, storedSchema->GetKindOfQuantityCP("koq"));

        ECEnumerationCP enumeration = storedSchema->GetEnumerationCP("enumeration");
        ASSERT_NE(nullptr, enumeration);

        ASSERT_NE(nullptr, enumeration->FindEnumeratorByName("enumeratorA"));
        ASSERT_NE(nullptr, enumeration->FindEnumeratorByName("enumeratorB"));
        ASSERT_NE(nullptr, enumeration->FindEnumeratorByName("enumeratorC"));
        }

    ASSERT_NE(nullptr, storedSchema->GetPropertyCategoryCP("propCateg")) << "the non-empty named property category should not have been pruned during import";
    ASSERT_EQ(nullptr, storedSchema->GetPropertyCategoryCP("")) << "the empty named property category should have been pruned during import";
    ASSERT_EQ(nullptr, structClass->GetPropertyP("namelessReferencingProp")->GetCategory()) << "the empty named property category reference should have been pruned during import";
    ASSERT_NE(nullptr, structClass->GetPropertyP("namedReferencingProp")->GetCategory()) << "the unnamed property category should not have been pruned during import";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerTests, SchemaOwnershipCheck) {
    Utf8CP schemaXml = R"schema(<?xml version='1.0' encoding='utf-8' ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" displayLabel="test schema" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u" />
            <ECSchemaReference name="Formats" version="01.00.00" alias="f" />
            <UnitSystem typeName="MyMetric" displayLabel="Metric" description="Metric Units of measure" />
            <UnitSystem typeName="MyImperial" displayLabel="Imperial" description="Units of measure from the British Empire" />
            <Phenomenon typeName="MyArea" displayLabel="Area" definition="LENGTH*LENGTH" />
            <Unit typeName="MySquareM" displayLabel="Square Meter" definition="M*M" numerator="1.0" phenomenon="MyArea" unitSystem="MyMetric" />
            <Unit typeName="MySquareFt" displayLabel="Square Feet" definition="Ft*Ft" numerator="10.0" offset="0.4" phenomenon="MyArea" unitSystem="MyImperial" />
            <Format typeName="MyFormat" displayLabel="My Format" roundFactor="0.3" type="Fractional" showSignOption="OnlyNegative" formatTraits="TrailZeroes|KeepSingleZero"
                    precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
            </Format>
            <Format typeName="MyFormatWithComposite" displayLabel="My Format with composite" type="Decimal" formatTraits="keepSingleZero|keepDecimalPoint|showUnitLabel" precision="2" >
                <Composite spacer="-">
                    <Unit label="hour">u:HR</Unit>
                    <Unit label="min">u:MIN</Unit>
                </Composite>
            </Format>
            <KindOfQuantity typeName="KoqWithCustomFormat" persistenceUnit="u:M" presentationUnits="MyFormat[u:M]" relativeError="0.1"/>
            <KindOfQuantity typeName="KoqWithCustomUnit" persistenceUnit="MySquareM" presentationUnits="f:DefaultRealU(4)[MySquareM]" relativeError="0.2"/>
            <KindOfQuantity typeName="KoqWithCustomUnitAndFormat" persistenceUnit="MySquareFt" presentationUnits="MyFormat[MySquareFt]" relativeError="0.3"/>
            <ECEnumeration typeName="StatusEnum" displayLabel="Int Enumeration with enumerators without display label" description="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                <ECEnumerator name="On" value="0"/>
                <ECEnumerator name="Off" value="1"/>
                <ECEnumerator name="Unknown" value="2"/>
            </ECEnumeration>
            <PropertyCategory typeName="Core" description="Core" displayLabel="Core" priority="1" />
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="Code" typeName="int" />
                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="KoqWithCustomFormat" />
                <ECProperty propertyName="Status" typeName="StatusEnum" />
                <ECProperty propertyName="Length" typeName="double" category="Core" />
            </ECEntityClass>
        </ECSchema>
        )schema";

    auto readSchema = [](ECDbR ecdb, Utf8CP xml) {
        ECSchemaPtr schema;
        ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
        readContext->AddSchemaLocater(ecdb.GetSchemaLocater());
        ECSchema::ReadFromXmlString(schema, xml, *readContext);
        return schema;
    };
    enum IdAssertVal{HasId = true, HasNoId = false};
    
    auto assertHasNoIdSet = [](ECSchemaCR schema, IdAssertVal hasId) {
        ASSERT_EQ((bool)hasId, schema.HasId());

        for(auto v : schema.GetClasses()) {
            ASSERT_EQ((bool)hasId, v->HasId());
            for (auto p : v->GetProperties(false))
                ASSERT_EQ((bool)hasId, p->HasId());
        }
        
        for (auto v : schema.GetEnumerations())
            ASSERT_EQ((bool)hasId, v->HasId());
        
        for (auto v: schema.GetPropertyCategories())
            ASSERT_EQ((bool)hasId, v->HasId());
        
        for (auto v : schema.GetKindOfQuantities())
            ASSERT_EQ((bool)hasId, v->HasId());
       
        for (auto v : schema.GetUnitSystems())
            ASSERT_EQ((bool)hasId, v->HasId());

        for (auto v :  schema.GetUnits())
            ASSERT_EQ((bool)hasId, v->HasId());

        for (auto v : schema.GetPhenomena())
            ASSERT_EQ((bool)hasId, v->HasId());
        
        for (auto v : schema.GetFormats())
            ASSERT_EQ((bool)hasId, v->HasId());
    };

    ECDb a,b;
    ASSERT_EQ(BE_SQLITE_OK, CreateECDb(a, "test1.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, CreateECDb(b, "test2.ecdb"));

    ASSERT_NE(a.GetId().ToString(),b.GetId().ToString());

    auto testSchema = readSchema(a, schemaXml);

    assertHasNoIdSet(*testSchema , IdAssertVal::HasNoId);
    a.Schemas().ImportSchemas({testSchema.get()});
    assertHasNoIdSet(*testSchema, IdAssertVal::HasNoId);
    ECSchemaPtr aTestSchema = const_cast<ECSchemaP>(a.Schemas().GetSchema("TestSchema"));
    ASSERT_TRUE(aTestSchema.IsValid());
    assertHasNoIdSet(*aTestSchema, IdAssertVal::HasId);
    ASSERT_FALSE(a.Schemas().OwnsSchema(*testSchema));

    // after clear cache the schema ownership should change and ids are cleared.
    a.ClearECDbCache();
    assertHasNoIdSet(*aTestSchema, IdAssertVal::HasNoId);
    ASSERT_FALSE(a.Schemas().OwnsSchema(*aTestSchema));


    b.Schemas().ImportSchemas({testSchema.get()});
    assertHasNoIdSet(*testSchema, IdAssertVal::HasNoId);
    ECSchemaPtr bTestSchema = const_cast<ECSchemaP>(b.Schemas().GetSchema("TestSchema"));
    ASSERT_TRUE(aTestSchema.IsValid());
    assertHasNoIdSet(*bTestSchema, IdAssertVal::HasId);
    ASSERT_TRUE(b.Schemas().OwnsSchema(*bTestSchema));
    ASSERT_FALSE(b.Schemas().OwnsSchema(*testSchema));

    // after clear cache the schema ownership should change and ids are cleared.
    b.ClearECDbCache();
    assertHasNoIdSet(*bTestSchema, IdAssertVal::HasNoId);
    ASSERT_FALSE(a.Schemas().OwnsSchema(*bTestSchema));

    a.SaveChanges();    
    b.SaveChanges();
}

END_ECDBUNITTESTS_NAMESPACE
