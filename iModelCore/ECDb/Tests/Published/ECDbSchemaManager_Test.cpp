/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbSchemaManager_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestProject.h"
#include "SchemaImportTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE


struct ECDbSchemaManagerTests : SchemaImportTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, ImportDifferentInMemorySchemaVersions)
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
            ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(schemas)) << "ECVersion " << ECSchema::GetECVersionString(version);
        else
            ASSERT_EQ(ERROR, ecdb.Schemas().ImportECSchemas(schemas)) << "ECVersion " << ECSchema::GetECVersionString(version);

        ecdb.AbandonChanges();
        };

    importSchema(ecdb, ECVersion::V2_0, false);
    importSchema(ecdb, ECVersion::V3_0, false);
    importSchema(ecdb, ECVersion::V3_1, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, ImportToken)
    {
    auto assertImport = [this] (Utf8CP ecschemaXml, bool expectedToSucceedInRestrictedMode, BeFileNameCR seedFilePath)
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, CloneECDb(ecdb, (Utf8String(seedFilePath.GetFileNameWithoutExtension()) + "_unrestricted.ecdb").c_str(), seedFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));
        bool asserted = false;
        AssertSchemaImport(asserted, ecdb, SchemaItem(ecschemaXml, true));
        ASSERT_FALSE(asserted) << "SchemaImport into unrestricted ECDb failed unexpectedly for: " << ecschemaXml;
        ecdb.CloseDb();

        NoDbSchemaModificationsECDb restrictedECDb;
        ASSERT_EQ(BE_SQLITE_OK, CloneECDb(restrictedECDb, (Utf8String(seedFilePath.GetFileNameWithoutExtension()) + "_restricted.ecdb").c_str(), seedFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));

        asserted = false;
        //Until enforced finally, the token validation does not fail the schema import, but just logs a warning.
        expectedToSucceedInRestrictedMode = true;
        AssertSchemaImport(asserted, restrictedECDb, SchemaItem(ecschemaXml, expectedToSucceedInRestrictedMode));
        ASSERT_FALSE(asserted) << "SchemaImport into restricted ECDb. Expected to succeed: " << expectedToSucceedInRestrictedMode << " for: " << ecschemaXml;
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
                 "</ECSchema>", false, seedFileName);


    SetupECDb("importtokentests.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                                                  "<ECSchema schemaName='BaseSchema' alias='base' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                                  "    <ECSchemaReference name='ECDbMap' version='02.00' alias='ecdbMap' />"
                                                  "    <ECEntityClass typeName='BaseNoTph' modifier='Abstract'>"
                                                  "        <ECProperty propertyName='BaseProp1' typeName='long' />"
                                                  "        <ECProperty propertyName='BaseProp2' typeName='string' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='Foo' modifier='Sealed'>"
                                                  "        <ECProperty propertyName='SubNoTphProp1' typeName='long' />"
                                                  "        <ECProperty propertyName='SubNoTphProp2' typeName='string' />"
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
                                                  "    <ECEntityClass typeName='SubSharedCols' modifier='None'>"
                                                  "          <ECCustomAttributes>"
                                                  "            <ShareColumns xmlns='ECDbMap.02.00' >"
                                                  "               <SharedColumnCount>5</SharedColumnCount>"
                                                  "               <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
                                                  "            </ShareColumns>"
                                                  "          </ECCustomAttributes>"
                                                  "        <BaseClass>TphBase</BaseClass>"
                                                  "        <ECProperty propertyName='SubSharedColsProp1' typeName='long' />"
                                                  "        <ECProperty propertyName='SubSharedColsProp2' typeName='string' />"
                                                  "    </ECEntityClass>"
                                                  "    <ECEntityClass typeName='SubNotSharedCols' modifier='None'>"
                                                  "        <BaseClass>TphBase</BaseClass>"
                                                  "        <ECProperty propertyName='SubNotSharedColsProp1' typeName='long' />"
                                                  "        <ECProperty propertyName='SubNotSharedColsProp2' typeName='string' />"
                                                  "    </ECEntityClass>"
                                                  "   <ECRelationshipClass typeName='FKRel' strength='Referencing' modifier='None'>"
                                                  "      <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent'>"
                                                  "          <Class class ='Foo' />"
                                                  "      </Source>"
                                                  "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Children'>"
                                                  "          <Class class ='TphBase' />"
                                                  "      </Target>"
                                                  "   </ECRelationshipClass>"
                                                  "   <ECRelationshipClass typeName='LinkTableRel' strength='Referencing' modifier='None'>"
                                                  "      <Source multiplicity='(0..*)' polymorphic='True' roleLabel='Lhs'>"
                                                  "          <Class class ='Foo' />"
                                                  "      </Source>"
                                                  "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Rhs'>"
                                                  "          <Class class ='TphBase' />"
                                                  "      </Target>"
                                                  "   </ECRelationshipClass>"
                                                  "</ECSchema>", true));
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
                 "</ECSchema>", false, seedFileName);
    
    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='SubClassGetsJoinedTable' alias='sub' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='BaseSchema' version='01.00' alias='base' />"
                 "    <ECEntityClass typeName='Sub2' modifier='None'>"
                 "        <BaseClass>base:TphBase</BaseClass>"
                 "        <ECProperty propertyName='Sub2Prop1' typeName='long' />"
                 "        <ECProperty propertyName='Sub2Prop2' typeName='string' />"
                 "    </ECEntityClass>"
                 "</ECSchema>", false, seedFileName);

    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='NotSharedCols' alias='sub' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='BaseSchema' version='01.00' alias='base' />"
                 "    <ECEntityClass typeName='SubSub' modifier='None'>"
                 "        <BaseClass>base:SubNotSharedCols</BaseClass>"
                 "        <ECProperty propertyName='SubSubProp1' typeName='long' />"
                 "        <ECProperty propertyName='SubSubProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "</ECSchema>", false, seedFileName);

    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='SharedCols' alias='sub' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='BaseSchema' version='01.00' alias='base' />"
                 "    <ECEntityClass typeName='SubSub' modifier='None'>"
                 "        <BaseClass>base:SubSharedCols</BaseClass>"
                 "        <ECProperty propertyName='SubSubProp1' typeName='long' />"
                 "        <ECProperty propertyName='SubSubProp2' typeName='string' />"
                 "    </ECEntityClass>"
                 "</ECSchema>", false, seedFileName); //Once overflow work was merged in, this is expected to succeed

    assertImport("<?xml version='1.0' encoding='utf-8' ?>"
                 "<ECSchema schemaName='RootFkRel' alias='sub' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                 "    <ECSchemaReference name='BaseSchema' version='01.00' alias='base' />"
                 "   <ECRelationshipClass typeName='NewFKRel' strength='Referencing' modifier='None'>"
                 "      <Source multiplicity='(0..1)' polymorphic='True' roleLabel='Parent'>"
                 "          <Class class ='base:Foo' />"
                 "      </Source>"
                 "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Children'>"
                 "          <Class class ='base:TphBase' />"
                 "      </Target>"
                 "   </ECRelationshipClass>"
                 "</ECSchema>", false, seedFileName);

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
                 "</ECSchema>", true, seedFileName);

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
                 "</ECSchema>", false, seedFileName);

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
                 "</ECSchema>", true, seedFileName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  10/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, ImportWithLocalizationSchemas)
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

    ECDbSchemaManager const& schemas = GetECDb().Schemas();
    ECSchemaCP testSchema = schemas.GetECSchema("TestSchema", false);
    ASSERT_TRUE(testSchema != nullptr);
    ASSERT_STRCASEEQ("Test Schema", testSchema->GetDisplayLabel().c_str());

    ECEnumerationCP animalEnum = schemas.GetECEnumeration("TestSchema", "Animal");
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

    ECClassCP spatialElementClass = schemas.GetECClass("TestSchema", "SpatialElement");
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
TEST_F(ECDbSchemaManagerTests, IncrementalLoading)
    {
    SetupECDb("ecdbschemamanagertests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    BeFileName testFilePath(GetECDb().GetDbFileName());

    const int expectedClassCount = GetECDb().Schemas().GetECSchema("ECSqlTest")->GetClassCount();
    GetECDb().CloseDb();

    {
    //GetECSchema with ensureAllClassesLoaded = false
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));
    ECSchemaCP schema = ecdb.Schemas().GetECSchema("ECSqlTest", false);
    ASSERT_TRUE(schema != nullptr);

    ASSERT_EQ(0, schema->GetClassCount()) << "ECDbSchemaManager::GetECSchema (..., false) is expected to return an empty schema";
    }

    //GetECSchema with ensureAllClassesLoaded = true
    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));
    ECSchemaCP schema = ecdb.Schemas().GetECSchema("ECSqlTest", true);
    ASSERT_TRUE(schema != nullptr);

    ASSERT_EQ(expectedClassCount, schema->GetClassCount()) << "ECDbSchemaManager::GetECSchema (..., true) is expected to return a fully populated schema";
    }

    //GetECClass from a different schema first and then GetECSchema with ensureAllClassesLoaded = true
    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));
    ECClassCP ecClass = ecdb.Schemas().GetECClass("ECDb_System", "PrimitiveArray");
    ASSERT_TRUE(ecClass != nullptr) << "ECDbSchemaManager::GetECClass ('ECDbSystem', 'PrimitiveArray') is expected to succeed as the class exists in the ecdb file.";

    ECSchemaCP schema = ecdb.Schemas().GetECSchema("ECSqlTest", false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(0, schema->GetClassCount()) << "ECDbSchemaManager::GetECSchema (..., false) is expected to return a schema with only the classes already loaded.";

    schema = ecdb.Schemas().GetECSchema("ECSqlTest", true);
    ASSERT_TRUE(schema != nullptr);

    ASSERT_EQ(expectedClassCount, schema->GetClassCount()) << "ECDbSchemaManager::GetECSchema (..., true) is expected to return a fully populated schema even if ECDbSchemaManager::GetECClass was called before for a class in a different ECSchema.";
    }

    //GetECClass from same schema first and then GetECSchema with ensureAllClassesLoaded = true
    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));

    ECClassCP ecClass = ecdb.Schemas().GetECClass("ECSqlTest", "THBase");
    ASSERT_TRUE(ecClass != nullptr) << "ECDbSchemaManager::GetECClass ('THBase') is expected to succeed as the class exists in the ecdb file.";

    ECSchemaCP schema = ecdb.Schemas().GetECSchema("ECSqlTest", false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(1, schema->GetClassCount()) << "ECDbSchemaManager::GetECSchema (..., false) is expected to return a schema with only the classes already loaded.";

    schema = ecdb.Schemas().GetECSchema("ECSqlTest", true);
    ASSERT_TRUE(schema != nullptr);

    ASSERT_EQ(expectedClassCount, schema->GetClassCount()) << "ECDbSchemaManager::GetECSchema (..., true) is expected to return a fully populated schema even if ECDbSchemaManager::GetECClass was called before for a class in the same schema.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, CasingTests)
    {
    ECDbCR ecdb = SetupECDb("schemamanagercasingtests.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSchemaCP schema = ecdb.Schemas().GetECSchema("ECDB_FILEinfo");
    ASSERT_TRUE(schema != nullptr && schema->GetName().EqualsI("ECDb_FileInfo"));

    schema = ecdb.Schemas().GetECSchema("ecsqltest");
    ASSERT_TRUE(schema != nullptr && schema->GetName().EqualsI("ECSqlTest"));

    ECClassCP ecclass = nullptr;
    ecclass = ecdb.Schemas().GetECClass("ecsqltest", "P");
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = ecdb.Schemas().GetECClass("ECSqlTest", "p");
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = ecdb.Schemas().GetECClass("ecSqL", "P", ResolveSchema::BySchemaAlias);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = ecdb.Schemas().GetECClass("ecsql", "p", ResolveSchema::BySchemaAlias);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = ecdb.Schemas().GetECClass("ecsqlTest", "P", ResolveSchema::AutoDetect);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = ecdb.Schemas().GetECClass("ecsqL", "P", ResolveSchema::AutoDetect);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = ecdb.Schemas().GetECClass("ECSqlTest", "p", ResolveSchema::AutoDetect);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = ecdb.Schemas().GetECClass("ecsql", "p", ResolveSchema::AutoDetect);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, GetDerivedECClasses)
    {
    ECDbCR ecdb = SetupECDb("ecschemamanagertest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassCP baseClass = ecdb.Schemas().GetECClass("ECSqlTest", "THBase");
    ASSERT_TRUE(baseClass != nullptr) << "Could not retrieve base class";

    //derived classes are not loaded when calling ECClass::GetDerivedClasses
    ASSERT_TRUE(baseClass->GetDerivedClasses().empty()) << "ECClass::GetDerivedClasses is expected to not load subclasses.";

    //derived classes are expected to be loaded when calling ECDbSchemaManager::GetDerivedClasses
    ASSERT_EQ(1, ecdb.Schemas().GetDerivedECClasses(*baseClass).size()) << "Unexpected derived class count with derived classes now being loaded";

    //now ECClass::GetDerivedClasses can also be called
    ASSERT_EQ(1, baseClass->GetDerivedClasses().size()) << "Unexpected derived class count after derived classes were loaded";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, GetDerivedECClassesWithoutIncrementalLoading)
    {
    ECDbCR ecdb = SetupECDb("ecschemamanagertest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSchemaCP testSchema = ecdb.Schemas().GetECSchema("ECSqlTest", true);
    ASSERT_TRUE(testSchema != nullptr);

    ECClassCP baseClass = ecdb.Schemas().GetECClass("ECSqlTest", "THBase");
    ASSERT_TRUE(baseClass != nullptr) << "Could not retrieve base class";

    ASSERT_EQ(1, baseClass->GetDerivedClasses().size()) << "Unexpected derived class count. Derived classes are expected to already be loaded along with having loaded the schema";

    //derived classes are expected to be loaded when calling ECDbSchemaManager::GetDerivedClasses
    ASSERT_EQ(1, ecdb.Schemas().GetDerivedECClasses(*baseClass).size()) << "Unexpected derived class count";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, GetEnumeration)
    {
    ECDbCR ecdb = SetupECDb("getenumeration.ecdb", SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                                     "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "<ECSchemaReference name='ECDb_FileInfo' version='02.00.00' prefix='ecdbf' />"
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

    ECEnumerationCP ecEnum = ecdb.Schemas().GetECEnumeration("ECDb_FileInfo", "StandardRootFolderType");
    ASSERT_TRUE(ecEnum != nullptr);
    ASSERT_EQ(4, ecEnum->GetEnumeratorCount());
    }

    {
    ECDb ecdb;
    ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath.c_str(), ECDb::OpenParams(Db::OpenMode::Readonly))) << "Could not open test file " << ecdbPath.c_str();

    ECSchemaCP schema = ecdb.Schemas().GetECSchema("ECDb_FileInfo", false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(0, schema->GetEnumerationCount());
    ECClassCP classWithEnum = ecdb.Schemas().GetECClass("ECDb_FileInfo", "ExternalFileInfo");
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

    ECSchemaCP schema = ecdb.Schemas().GetECSchema("ECDb_FileInfo", false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(0, schema->GetEnumerationCount());
    ECClassCP classWithEnum = ecdb.Schemas().GetECClass("TestSchema", "Foo");
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

    ECSchemaCP schema = ecdb.Schemas().GetECSchema("ECDb_FileInfo", true);
    ASSERT_TRUE(schema != nullptr);
    ECEnumerationCP ecEnum = schema->GetEnumerationCP("StandardRootFolderType");
    ASSERT_TRUE(ecEnum != nullptr);
    ASSERT_EQ(4, ecEnum->GetEnumeratorCount());
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, GetKindOfQuantity)
    {
    auto assertKoq = [] (KindOfQuantityCR actualKoq)
        {
        ASSERT_STREQ("My KindOfQuantity", actualKoq.GetDisplayLabel().c_str());
        ASSERT_STREQ("My KindOfQuantity", actualKoq.GetDescription().c_str());
        ASSERT_STREQ("CENTIMETRE", actualKoq.GetPersistenceUnit().c_str());
        ASSERT_STREQ("FOOT", actualKoq.GetDefaultPresentationUnit().c_str());
        bvector<Utf8String> const& actualAltUnits = actualKoq.GetAlternativePresentationUnitList();
        ASSERT_EQ(2, actualAltUnits.size());
        ASSERT_STREQ("INCH", actualAltUnits[0].c_str());
        ASSERT_STREQ("YARD", actualAltUnits[1].c_str());
        };

    Utf8String ecdbPath;
    {
    std::vector<SchemaItem> testSchemas;
    testSchemas.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                                     "<ECSchema schemaName='Schema1' nameSpacePrefix='s1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "<KindOfQuantity typeName='MyKindOfQuantity' description='My KindOfQuantity'"
                                     "                displayLabel='My KindOfQuantity' persistenceUnit='CENTIMETRE' precision='10'"
                                     "                defaultPresentationUnit='FOOT' alternativePresentationUnits='INCH;YARD' />"
                                     "</ECSchema>"));

    testSchemas.push_back(SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                                     "<ECSchema schemaName='Schema2' nameSpacePrefix='s2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                     "<ECSchemaReference name='Schema1' version='01.00.00' prefix='s1' />"
                                     "  <ECEntityClass typeName='Foo' >"
                                     "    <ECProperty propertyName='Length' typeName='double' kindOfQuantity='s1:MyKindOfQuantity' />"
                                     "    <ECProperty propertyName='Homepage' typeName='string' extendedTypeName='URL' />"
                                     "    <ECArrayProperty propertyName='AlternativeLengths' typeName='double' minOccurs='0' maxOccurs='unbounded' kindOfQuantity='s1:MyKindOfQuantity'/>"
                                     "    <ECArrayProperty propertyName='Favorites' typeName='string' extendedTypeName='URL' minOccurs='0' maxOccurs='unbounded' />"
                                     "  </ECEntityClass>"
                                     "</ECSchema>"));

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

    ECSchemaCP schema = ecdb.Schemas().GetECSchema("Schema1", false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(0, schema->GetKindOfQuantityCount());
    ECClassCP classWithKoq = ecdb.Schemas().GetECClass("Schema2", "Foo");
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

    ECSchemaCP schema = ecdb.Schemas().GetECSchema("Schema1", true);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(1, schema->GetKindOfQuantityCount());
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, GetPropertyWithExtendedType)
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

    ECClassCP fooClass = ecdb.Schemas().GetECClass("TestSchema", "Foo");
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
TEST_F(ECDbSchemaManagerTests, GetRelationshipWithAbstractConstraintClass)
    {
    ECDbR ecdb = SetupECDb("relationshipwithabstractconstraintclass.ecdb",
                           SchemaItem("<?xml version='1.0' encoding='utf-8' ?>"
                                      "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                                      "  <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' />"
                                      "  <ECEntityClass typeName='Model' >"
                                      "    <ECProperty propertyName='Name' typeName='string' />"
                                      "  </ECEntityClass>"
                                      "  <ECEntityClass typeName='Element' modifier='Abstract' >"
                                      "        <ECCustomAttributes>"
                                      "            <ClassMap xmlns='ECDbMap.02.00'>"
                                      "                <MapStrategy>TablePerHierarchy</MapStrategy>"
                                      "            </ClassMap>"
                                      "        </ECCustomAttributes>"
                                      "    <ECProperty propertyName='Code' typeName='string' />"
                                      "  </ECEntityClass>"
                                      "  <ECEntityClass typeName='BaseElement' modifier='Abstract' >"
                                      "      <BaseClass>Element</BaseClass>"
                                      "  </ECEntityClass>"
                                      "  <ECEntityClass typeName='FooElement' modifier='Sealed' >"
                                      "      <BaseClass>BaseElement</BaseClass>"
                                      "  </ECEntityClass>"
                                      "  <ECEntityClass typeName='GooElement' modifier='Sealed' >"
                                      "      <BaseClass>BaseElement</BaseClass>"
                                      "  </ECEntityClass>"
                                      "  <ECRelationshipClass typeName='ModelHasFooOrGooElements' modifier='Sealed' >"
                                      "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Model'>"
                                      "          <Class class ='Model' />"
                                      "      </Source>"
                                      "      <Target multiplicity='(0..*)' polymorphic='False' abstractConstraint='BaseElement' roleLabel='Foo or Goo Elements'>"
                                      "          <Class class ='FooElement' />"
                                      "          <Class class ='GooElement' />"
                                      "      </Target>"
                                      "  </ECRelationshipClass>"
                                      "  <ECRelationshipClass typeName='ModelHasElements' modifier='Sealed' >"
                                      "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='Model'>"
                                      "          <Class class ='Model' />"
                                      "      </Source>"
                                      "      <Target multiplicity='(0..*)' polymorphic='False' roleLabel='Elements'>"
                                      "          <Class class ='Element' />"
                                      "      </Target>"
                                      "  </ECRelationshipClass>"
                                      "</ECSchema>"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECClassCP ecclass = ecdb.Schemas().GetECClass("TestSchema", "ModelHasFooOrGooElements");
    ASSERT_TRUE(ecclass != nullptr);
    ECRelationshipClassCP relWithAbstractConstraint = ecclass->GetRelationshipClassCP();
    ASSERT_TRUE(relWithAbstractConstraint != nullptr);
    
    ASSERT_EQ(ecdb.Schemas().GetECClass("TestSchema", "Model"), relWithAbstractConstraint->GetSource().GetAbstractConstraint());
    ASSERT_EQ(ecdb.Schemas().GetECClass("TestSchema", "BaseElement"), relWithAbstractConstraint->GetTarget().GetAbstractConstraint());

    ecclass = ecdb.Schemas().GetECClass("TestSchema", "ModelHasElements");
    ASSERT_TRUE(ecclass != nullptr);
    ECRelationshipClassCP relWithoutAbstractConstraint = ecclass->GetRelationshipClassCP();
    ASSERT_TRUE(relWithoutAbstractConstraint != nullptr);
    ASSERT_EQ(ecdb.Schemas().GetECClass("TestSchema", "Model"), relWithoutAbstractConstraint->GetSource().GetAbstractConstraint());
    ASSERT_EQ(ecdb.Schemas().GetECClass("TestSchema", "Element"), relWithoutAbstractConstraint->GetTarget().GetAbstractConstraint());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, AddDuplicateECSchemaInCache)
    {
    ECDbTestFixture::Initialize();

    ECDb ecdb;
    auto stat = ECDbTestUtility::CreateECDb(ecdb, nullptr, L"ecschemamanagertest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    ECSchemaPtr schemaPtr = ECDbTestUtility::ReadECSchemaFromDisk(L"BaseSchemaA.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr != NULL);
    ECSchemaPtr schemaPtr1 = ECDbTestUtility::ReadECSchemaFromDisk(L"BaseSchemaA.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr1 != NULL);

    ECSchemaCachePtr schemacache = ECSchemaCache::Create();
    ASSERT_EQ(ECObjectsStatus::Success, schemacache->AddSchema(*schemaPtr)) << "couldn't add schema to the cache" << schemaPtr->GetName().c_str();
    ASSERT_EQ(ECObjectsStatus::DuplicateSchema, schemacache->AddSchema(*schemaPtr1));
    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(schemacache->GetSchemas())) << "could not import test ecschema.";
    ecdb.SaveChanges();

    ECClassCP ecclass = ecdb.Schemas().GetECClass("BaseSchemaA", "Address");
    EXPECT_TRUE(ecclass != NULL) << "ecclass with the specified name does not exist";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, ImportDuplicateSchema)
    {
    ECDbR ecdb = SetupECDb("ecschemamanagertest.ecdb", BeFileName(L"BaseSchemaA.01.00.ecschema.xml"), 3);

    ECSchemaPtr schema = ECDbTestUtility::ReadECSchemaFromDisk(L"BaseSchemaA.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schema != nullptr);

    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    schemaCache->AddSchema(*schema);

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(schemaCache->GetSchemas()));

    ECClassCP ecclass = ecdb.Schemas().GetECClass("BaseSchemaA", "Address");
    ASSERT_TRUE(ecclass != nullptr) << "Class with the specified name doesn't exist :- ecclass is empty";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, ImportingSchemaInDifferentECDB)
    {
    ECDbTestFixture::Initialize();
    ECDb ecdb, testecdb;
    auto stat = ECDbTestUtility::CreateECDb(ecdb, nullptr, L"testecdbSchema.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);
    stat = ECDbTestUtility::CreateECDb(testecdb, nullptr, L"testecdbSchema1.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    ECSchemaPtr schemaPtr = ECDbTestUtility::ReadECSchemaFromDisk(L"BaseSchemaA.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr != NULL);

    ECSchemaCachePtr Schemacache = ECSchemaCache::Create();
    Schemacache->AddSchema(*schemaPtr);

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(Schemacache->GetSchemas())) << "could not import test ecschema.";
    ecdb.SaveChanges();

    ASSERT_EQ(ERROR, testecdb.Schemas().ImportECSchemas(Schemacache->GetSchemas())) << "could not import test ecschema in the 2nd ecdb file.";
    testecdb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, ImportMultipleSupplementalSchemas)
    {
    ECDbCR ecdb = SetupECDb("supplementalSchematest.ecdb");
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());

    ECSchemaPtr schemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(schemaptr, context, L"SchoolSchema.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaptr != NULL);
    ECSchemaPtr supple;
    ECDbTestUtility::ReadECSchemaFromDisk(supple, context, L"SchoolSchema_Supplemental_Localization.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(supple != NULL);
    ECSchemaPtr supple1;
    ECDbTestUtility::ReadECSchemaFromDisk(supple1, context, L"SchoolSchema_Supplemental_Units.01.01.ecschema.xml", nullptr);
    ASSERT_TRUE(supple1 != NULL);

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(context->GetCache().GetSchemas())) << "couldn't import the schema";
    ECSchemaCP SchoolSupplSchema = ecdb.Schemas().GetECSchema("SchoolSchema", true);
    ASSERT_TRUE(SchoolSupplSchema != NULL);

    ECClassCP ecclassCourse = SchoolSupplSchema->GetClassCP("Course");
    ASSERT_TRUE(ecclassCourse != NULL);
    ECClassCP ecclassCourseTitle = SchoolSupplSchema->GetClassCP("CourseTitle");
    ASSERT_TRUE(ecclassCourseTitle != NULL);
    //get custom attributes from base class (false)
    ECCustomAttributeInstanceIterable iterator2 = ecclassCourseTitle->GetCustomAttributes(false);
    uint32_t i = 0;
    for (IECInstancePtr instance : iterator2)
        {
        i++;
        }
    EXPECT_EQ(1, i) << "the number of custom attributes on the Class CourseTitle do not match the original";
    //get custom attributes from base class (false)
    ECCustomAttributeInstanceIterable iterator1 = ecclassCourseTitle->GetCustomAttributes(true);
    i = 0;
    for (IECInstancePtr instance : iterator1)
        {
        i++;
        }
    EXPECT_EQ(4, i) << "the number of custom attributes on the Class CourseTitle do not match the original";

    ECClassCP relationshipClass = SchoolSupplSchema->GetClassCP("SchoolDepartmentRelation");
    ASSERT_TRUE(relationshipClass != NULL);
    ECCustomAttributeInstanceIterable iterator = relationshipClass->GetCustomAttributes(false);
    i = 0;
    for (IECInstancePtr instance : iterator)
        {
        i++;
        }
    EXPECT_EQ(5, i) << "the number of custom attributes on the Class relationshipClass do not match the original";

    ECClassCP ecclasscp = SchoolSupplSchema->GetClassCP("Department");
    ASSERT_TRUE(ecclasscp != NULL) << "couldn't read the class Department from schema";
    IECInstancePtr iecinstancePtr = ecclasscp->GetCustomAttribute("ChangeManagement");
    ASSERT_TRUE(iecinstancePtr.IsValid()) << "couldn't retrieve the custom attribute from the class Department";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, ImportLowPrioritySupplementalSchama)
    {
    ECDbTestFixture::Initialize();
    ECDb testecdb;
    auto stat = ECDbTestUtility::CreateECDb(testecdb, nullptr, L"supplementalSchematest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);
    ECSchemaReadContextPtr  context = nullptr;

    ECSchemaPtr schemaPtr;
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();

    ECDbTestUtility::ReadECSchemaFromDisk(schemaPtr, context, L"SchoolSchema.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr != NULL);
    schemaCache->AddSchema(*schemaPtr);
    ECDbTestUtility::ReadECSchemaFromDisk(schemaPtr, context, L"SchoolSchema_Supplemental_Localization.01.00.ecschema.xml", nullptr);
    schemaCache->AddSchema(*schemaPtr);


    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(schemaCache->GetSchemas())) << "couldn't import the schema";
    ECSchemaCP SchoolSupplSchema = testecdb.Schemas().GetECSchema("SchoolSchema", true);
    ASSERT_TRUE(SchoolSupplSchema != NULL);
    }

    
 //---------------------------------------------------------------------------------------
 // @bsimethod                                    Muhammad Hassan                  10/14
 //+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, ImportHighPrioritySupplementalSchama)
    {
    ECDbTestFixture::Initialize();
    ECDb testecdb;
    auto stat = ECDbTestUtility::CreateECDb(testecdb, nullptr, L"supplementalSchematest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);
    ECSchemaReadContextPtr context = nullptr;

    ECSchemaPtr schemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(schemaptr, context, L"SchoolSchema.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaptr != NULL);
    ECSchemaPtr supplementalSchemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(supplementalSchemaptr, context, L"SchoolSchema_Supplemental_Units.01.01.ecschema.xml", nullptr);
    ASSERT_TRUE(supplementalSchemaptr != NULL);

    ECSchemaCachePtr schemacache = ECSchemaCache::Create();
    schemacache->AddSchema(*schemaptr);
    schemacache->AddSchema(*supplementalSchemaptr);

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(schemacache->GetSchemas())) << "couldn't import the schema";
    ECSchemaCP SchoolSupplSchema = testecdb.Schemas().GetECSchema("SchoolSchema", true);
    ASSERT_TRUE(SchoolSupplSchema != NULL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, TestGetClassResolver)
    {
    ECDbCR ecdb = SetupECDb("ecschemamanagertest.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());
    ECClassCP ecClass = ecdb.Schemas().GetECClass("ECSqlTest", "PSA");
    EXPECT_TRUE(ecClass != nullptr);
    ecClass = ecdb.Schemas().GetECClass("ecsql", "PSA", ResolveSchema::BySchemaAlias);
    EXPECT_TRUE(ecClass != nullptr);

    ecClass = ecdb.Schemas().GetECClass("ECSqlTest", "PSA", ResolveSchema::AutoDetect);
    EXPECT_TRUE(ecClass != nullptr);

    ecClass = ecdb.Schemas().GetECClass("ecsql", "PSA", ResolveSchema::AutoDetect);
    EXPECT_TRUE(ecClass != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
// A primary schema should be supplemented with the latest available supplemental schema
TEST_F(ECDbSchemaManagerTests, SupplementWithLatestCompatibleSupplementalSchema)
    {
    ECDbTestFixture::Initialize();
    ECDb testecdb;
    auto stat = ECDbTestUtility::CreateECDb(testecdb, nullptr, L"supplementalSchematest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    ECSchemaReadContextPtr context = nullptr;
    ECSchemaCachePtr schemacache = ECSchemaCache::Create();

    ECSchemaPtr schemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(schemaptr, context, L"BasicSchema.01.70.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaptr != NULL);
    schemacache->AddSchema(*schemaptr);

    ECSchemaPtr supple = nullptr;

    ECDbTestUtility::ReadECSchemaFromDisk(supple, context, L"BasicSchema_Supplemental_Localization.01.90.ecschema.xml", nullptr);
    ASSERT_TRUE(supple != NULL);
    schemacache->AddSchema(*supple);

    ECDbTestUtility::ReadECSchemaFromDisk(supple, context, L"BasicSchema_Supplemental_Localization.01.60.ecschema.xml", nullptr);
    ASSERT_TRUE(supple != NULL);
    schemacache->AddSchema(*supple);

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(schemacache->GetSchemas())) << "couldn't import the schema";
    ECSchemaCP basicSupplSchema = testecdb.Schemas().GetECSchema("BasicSchema", true);
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
TEST_F(ECDbSchemaManagerTests, SupplementSchemaWhoseTargetedPrimaryHasGreaterMajorVersion)
    {
    ECDbTestFixture::Initialize();
    ECDb testecdb;
    auto stat = ECDbTestUtility::CreateECDb(testecdb, nullptr, L"supplementalSchematest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    ECSchemaReadContextPtr context = nullptr;
    ECSchemaCachePtr schemacache = ECSchemaCache::Create();

    ECSchemaPtr schemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(schemaptr, context, L"BasicSchema.01.70.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaptr != NULL);
    schemacache->AddSchema(*schemaptr);

    ECSchemaPtr supple;
    ECDbTestUtility::ReadECSchemaFromDisk(supple, context, L"BasicSchema_Supplemental_Localization.01.90.ecschema.xml", nullptr);
    ASSERT_TRUE(supple != NULL);
    schemacache->AddSchema(*supple);

    ECDbTestUtility::ReadECSchemaFromDisk(supple, context, L"BasicSchema_Supplemental_Localization.02.10.ecschema.xml", nullptr);
    ASSERT_TRUE(supple != NULL);
    schemacache->AddSchema(*supple);

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(schemacache->GetSchemas())) << "couldn't import the schema";
    ECSchemaCP basicSupplSchema = testecdb.Schemas().GetECSchema("BasicSchema", true);
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
TEST_F(ECDbSchemaManagerTests, CreateECClassViews)
    {
    ECDbR ecdb = SetupECDb("createecclassviews.ecdb");
    ASSERT_TRUE(ecdb.IsDbOpen());

    ASSERT_EQ(SUCCESS, ecdb.Schemas().CreateECClassViewsInDb());
    ecdb.SaveChanges();
    bmap<Utf8String, bset<Utf8String>> schemasWithECClassViews;
    bool validationFailed = false;
    AssertECClassViews(schemasWithECClassViews, validationFailed, ecdb);
    ASSERT_FALSE(validationFailed);

    ASSERT_EQ(2, schemasWithECClassViews.size()) << "Unexpected number of schemas with ECClassViews";
    ASSERT_EQ(4, schemasWithECClassViews["ecdbf"].size()) << "Unexpected number of ECClassViews";

    ECSchemaReadContextPtr  context = ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());
    ECSchemaPtr schemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(schemaptr, context, L"StartupCompany.02.00.ecschema.xml", nullptr);

    ECSchemaCachePtr schemacache = ECSchemaCache::Create();
    schemacache->AddSchema(*schemaptr);

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(schemacache->GetSchemas())) << "couldn't import the schema";
    ASSERT_EQ(SUCCESS, ecdb.Schemas().CreateECClassViewsInDb());
    ecdb.SaveChanges();
    AssertECClassViews(schemasWithECClassViews, validationFailed, ecdb);
    ASSERT_FALSE(validationFailed);
    ASSERT_EQ(3, schemasWithECClassViews.size()) << "Unexpected number of schemas with ECClassViews";
    ASSERT_EQ(4, schemasWithECClassViews["ecdbf"].size()) << "Unexpected number of ECClassViews";
    ASSERT_EQ(37, schemasWithECClassViews["stco"].size()) << "Unexpected number of ECClassViews";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                   12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, CreateECClassViewsForSubsetOfClasses)
    {
    ECDbR ecdb = SetupECDb("createecclassviewspartially.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId FROM ec.ECClassDef WHERE Name IN ('FileInfo', 'FileInfoOwnership', 'AAA','Cubicle', 'Foo_has_Bars', 'RelationWithLinkTableMapping')"));
    bvector<ECClassId> classIds;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ECClassId id = stmt.GetValueId<ECClassId>(0);
        ASSERT_TRUE(id.IsValid());
        classIds.push_back(id);
        }

    ASSERT_EQ(SUCCESS, ecdb.Schemas().CreateECClassViewsInDb(classIds));
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
    ASSERT_EQ(4, stcoViews.size());
    ASSERT_TRUE(stcoViews.find(Utf8String("[stco.AAA]")) != stcoViews.end());
    ASSERT_TRUE(stcoViews.find(Utf8String("[stco.Cubicle]")) != stcoViews.end());
    ASSERT_TRUE(stcoViews.find(Utf8String("[stco.Foo_has_Bars]")) != stcoViews.end());
    ASSERT_TRUE(stcoViews.find(Utf8String("[stco.RelationWithLinkTableMapping]")) != stcoViews.end());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                   12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, CreateECClassViewsForInvalidClasses)
    {
    ECDbCR ecdb = SetupECDb("createecclassviewsforinvalidclasses.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId FROM ec.ECClassDef WHERE Name IN ('AnglesStruct', 'ClassMap', 'AClassThatDoesNotGetMappedToDb')"));
    bvector<ECClassId> classIds;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ECClassId id = stmt.GetValueId<ECClassId>(0);
        ASSERT_TRUE(id.IsValid());
        classIds.push_back(id);
        }

    ASSERT_EQ(ERROR, ecdb.Schemas().CreateECClassViewsInDb(classIds));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad.Hassan                   12/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, CreateECClassViewsForCombinationofValidInvalidClasses)
    {
    ECDbR ecdb = SetupECDb("createecclassviewsforvalidinvalidclasses.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId FROM ec.ECClassDef WHERE Name IN ('AAA', 'AnglesStruct', 'AClassThatDoesNotGetMappedToDb', 'Foo_has_Bars')"));
    bvector<ECClassId> classIds;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ECClassId id = stmt.GetValueId<ECClassId>(0);
        ASSERT_TRUE(id.IsValid());
        classIds.push_back(id);
        }

    ASSERT_EQ(ERROR, ecdb.Schemas().CreateECClassViewsInDb(classIds));
    ecdb.SaveChanges();

    // Class view will be created for the provided list of ECClassIds untill it get the first invalid one
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
//supplemental schema whose Targeted primary schema's minor version is less then minor Version of schema to supplement.
TEST_F(ECDbSchemaManagerTests, SupplementSchemaWhoseTargetedPrimaryHasLowerMinorVersion)
    {
    ECDbTestFixture::Initialize();
    ECDb testecdb;
    auto stat = ECDbTestUtility::CreateECDb(testecdb, nullptr, L"supplementalSchematest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    ECSchemaReadContextPtr context = nullptr;
    ECSchemaCachePtr schemacache = ECSchemaCache::Create();

    ECSchemaPtr schemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(schemaptr, context, L"BasicSchema.01.70.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaptr != NULL);
    schemacache->AddSchema(*schemaptr);

    ECSchemaPtr supple;
    //With new supplementation Behaviour, this one will not be ignored though it is not targeting the primary schema's exact version.
    ECDbTestUtility::ReadECSchemaFromDisk(supple, context, L"BasicSchema_Supplemental_Localization.01.69.ecschema.xml", nullptr);
    ASSERT_TRUE(supple != NULL);
    schemacache->AddSchema(*supple);

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(schemacache->GetSchemas())) << "couldn't import the schema";
    ECSchemaCP basicSupplSchema = testecdb.Schemas().GetECSchema("BasicSchema", true);
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
    EXPECT_EQ(1, i) << "the number of custom attributes on the Class Base do not match the original";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
//supplemental schema whose Targeted primary schema's minor version is greater then minor Version of schema to supplement.
TEST_F(ECDbSchemaManagerTests, SupplementSchemaWhoseTargetedPrimaryHasGreaterMinorVersion)
    {
    ECDbTestFixture::Initialize();
    ECDb testecdb;
    auto stat = ECDbTestUtility::CreateECDb(testecdb, nullptr, L"supplementalSchematest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    ECSchemaReadContextPtr context = nullptr;
    ECSchemaCachePtr schemacache = ECSchemaCache::Create();

    ECSchemaPtr schemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(schemaptr, context, L"BasicSchema.01.69.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaptr != NULL);
    schemacache->AddSchema(*schemaptr);

    ECSchemaPtr supple;
    ECDbTestUtility::ReadECSchemaFromDisk(supple, context, L"BasicSchema_Supplemental_Localization.01.69.ecschema.xml", nullptr);
    ASSERT_TRUE(supple != NULL);
    schemacache->AddSchema(*supple);

    //With new supplementation Behaviour, this one will not be ignored though it is not targeting the primary schema's exact version.
    ECDbTestUtility::ReadECSchemaFromDisk(supple, context, L"BasicSchema_Supplemental_Localization.01.90.ecschema.xml", nullptr);
    ASSERT_TRUE(supple != NULL);
    schemacache->AddSchema(*supple);

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(schemacache->GetSchemas())) << "couldn't import the schema";
    ECSchemaCP basicSupplSchema = testecdb.Schemas().GetECSchema("BasicSchema", true);
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
TEST_F(ECDbSchemaManagerTests, ImportSchemaWithSubclassesToBaseClassInExistingSchema)
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
        ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(context1->GetCache().GetSchemas()));

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
        ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(context2->GetCache().GetSchemas()));

        ECInstanceKey newKey;
        ECSqlStatement insStmt;
        ASSERT_EQ(ECSqlStatus::Success, insStmt.Prepare(ecdb, "INSERT INTO c.Activity (Code, Name) VALUES ('ConstructionActivity-1', 'Do something')"));
        ASSERT_EQ(BE_SQLITE_DONE, insStmt.Step(newKey));

        ECSqlStatement updStmt;
        ASSERT_EQ(ECSqlStatus::Success, updStmt.Prepare(ecdb, "UPDATE p.Activity SET PlanId=100, OutlineIndex=100 WHERE ECInstanceId=?"));
        updStmt.BindId(1, newKey.GetECInstanceId());
        ASSERT_EQ(BE_SQLITE_DONE, updStmt.Step());

        activityKey = newKey;
        };

    ECDbTestProject testProject;
    ECDbR ecdb = testProject.Create("importschemawithsubclassestoexistingschema1.ecdb");

    ECInstanceKey activityKey;
    setup(activityKey, ecdb);
    ASSERT_TRUE(activityKey.IsValid());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT PlanId, OutlineIndex FROM p.Activity WHERE ECInstanceId=?"));
    stmt.BindId(1, activityKey.GetECInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_FALSE(stmt.IsValueNull(0)) << "This should start to fail if ECDb still caching horizontal paratition even after import a second schema";
    ASSERT_FALSE(stmt.IsValueNull(1)) << "This should start to fail if ECDb still caching horizontal paratition even after import a second schema";

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
   
    }


//-------------------------------------------------------------------------------------- -
// @bsimethod                                     Krischan.Eberle           11/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, IGeometryTypes)
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

    ECClassCP cl = ecdb.Schemas().GetECClass("TestSchema", "Foo");
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_TRUE(stmt.GetValueGeometry(0)->IsSameStructureAndGeometry(*g1));
    ASSERT_TRUE(stmt.GetValueGeometry(1)->IsSameStructureAndGeometry(*g2));
    ASSERT_TRUE(stmt.GetValueGeometry(2)->IsSameStructureAndGeometry(*g3));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad.Hassan                 02/16
//+---------------+---------------+---------------+---------------+---------------+-----
TEST_F(ECDbSchemaManagerTests, EnforceECEnumeration)
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
TEST_F(ECDbSchemaManagerTests, DuplicateInMemorySchemaTest)
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
    ASSERT_EQ(BentleyStatus::SUCCESS, ecdb.Schemas().ImportECSchemas(readContext->GetCache().GetSchemas()));

    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(usr, usrXml, *readContext));
    ASSERT_EQ(BentleyStatus::SUCCESS, ecdb.Schemas().ImportECSchemas(readContext->GetCache().GetSchemas())) << "Failed because locater was not added for schemas that already exist in ECDb";
    }

END_ECDBUNITTESTS_NAMESPACE

