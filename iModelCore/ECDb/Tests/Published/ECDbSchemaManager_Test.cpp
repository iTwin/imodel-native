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

static Utf8CP const TEST_SCHEMA_NAME = "ECDbSchemaManagerTest";

void SetupTestECDb (BeFileNameCR filePath);
ECSchemaPtr CreateTestSchema ();

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, IncrementalLoading)
    {
    BeFileName testFilePath(ECDbTestUtility::BuildECDbPath("ecschemamanagertest.ecdb"));
    SetupTestECDb(testFilePath);

    const int expectedClassCount = CreateTestSchema()->GetClassCount();

    //GetECSchema with ensureAllClassesLoaded = false
    {
    ECDb ecdb;
    auto stat = ecdb.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly));
    ASSERT_EQ(BE_SQLITE_OK, stat);

    auto const& schemaManager = ecdb.Schemas();

    ECSchemaCP schema = schemaManager.GetECSchema(TEST_SCHEMA_NAME, false);
    ASSERT_TRUE(schema != nullptr);

    ASSERT_EQ(0, schema->GetClassCount()) << "ECDbSchemaManager::GetECSchema (..., false) is expected to return an empty schema";
    }

    //GetECSchema with ensureAllClassesLoaded = true
    {
    ECDb ecdb;
    auto stat = ecdb.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly));
    ASSERT_EQ(BE_SQLITE_OK, stat);

    auto const& schemaManager = ecdb.Schemas();

    ECSchemaCP schema = schemaManager.GetECSchema(TEST_SCHEMA_NAME, true);
    ASSERT_TRUE(schema != nullptr);

    ASSERT_EQ(expectedClassCount, schema->GetClassCount()) << "ECDbSchemaManager::GetECSchema (..., true) is expected to return a fully populated schema";
    }

    //GetECClass from a different schema first and then GetECSchema with ensureAllClassesLoaded = true
    {
    ECDb ecdb;
    auto stat = ecdb.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly));
    ASSERT_EQ(BE_SQLITE_OK, stat);

    auto const& schemaManager = ecdb.Schemas();

    ECClassCP ecClass = schemaManager.GetECClass("ECDb_System", "PrimitiveArray");
    ASSERT_TRUE(ecClass != nullptr) << "ECDbSchemaManager::GetECClass ('ECDbSystem', 'PrimitiveArray') is expected to succeed as the class exists in the ecdb file.";

    ECSchemaCP schema = schemaManager.GetECSchema(TEST_SCHEMA_NAME, false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(0, schema->GetClassCount()) << "ECDbSchemaManager::GetECSchema (..., false) is expected to return a schema with only the classes already loaded.";

    schema = schemaManager.GetECSchema(TEST_SCHEMA_NAME, true);
    ASSERT_TRUE(schema != nullptr);

    ASSERT_EQ(expectedClassCount, schema->GetClassCount()) << "ECDbSchemaManager::GetECSchema (..., true) is expected to return a fully populated schema even if ECDbSchemaManager::GetECClass was called before for a class in a different ECSchema.";
    }

    //GetECClass from same schema first and then GetECSchema with ensureAllClassesLoaded = true
    {
    ECDb ecdb;
    auto stat = ecdb.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly));
    ASSERT_EQ(BE_SQLITE_OK, stat);

    auto const& schemaManager = ecdb.Schemas();

    ECClassCP ecClass = schemaManager.GetECClass(TEST_SCHEMA_NAME, "Base");
    ASSERT_TRUE(ecClass != nullptr) << "ECDbSchemaManager::GetECClass ('Base') is expected to succeed as the class exists in the ecdb file.";

    ECSchemaCP schema = schemaManager.GetECSchema(TEST_SCHEMA_NAME, false);
    ASSERT_TRUE(schema != nullptr);
    ASSERT_EQ(1, schema->GetClassCount()) << "ECDbSchemaManager::GetECSchema (..., false) is expected to return a schema with only the classes already loaded.";

    schema = schemaManager.GetECSchema(TEST_SCHEMA_NAME, true);
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

    ecclass = ecdb.Schemas().GetECClass("ecSqL", "P", ResolveSchema::BySchemaNamespacePrefix);
    ASSERT_TRUE(ecclass != nullptr && BeStringUtilities::StricmpAscii(ecclass->GetFullName(), "ECSqlTest:P") == 0);

    ecclass = ecdb.Schemas().GetECClass("ecsql", "p", ResolveSchema::BySchemaNamespacePrefix);
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
    BeFileName testFilePath(ECDbTestUtility::BuildECDbPath("ecschemamanagertest.ecdb"));
    SetupTestECDb(testFilePath);

    ECDb testFile;
    ASSERT_EQ(BE_SQLITE_OK, testFile.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(Db::OpenMode::Readonly))) << "Could not open test file " << testFilePath.GetNameUtf8().c_str();

    auto const& schemaManager = testFile.Schemas();

    ECClassCP baseClass = schemaManager.GetECClass(TEST_SCHEMA_NAME, "Base");
    ASSERT_TRUE(baseClass != nullptr) << "Could not retrieve base class";

    //derived classes are not loaded when calling ECClass::GetDerivedClasses
    ASSERT_TRUE(baseClass->GetDerivedClasses().empty()) << "ECClass::GetDerivedClasses is expected to not load subclasses.";

    //derived classes are expected to be loaded when calling ECDbSchemaManager::GetDerivedClasses
    ASSERT_EQ(2, schemaManager.GetDerivedECClasses(*baseClass).size()) << "Unexpected derived class count with derived classes now being loaded";

    //now ECClass::GetDerivedClasses can also be called
    ASSERT_EQ(2, baseClass->GetDerivedClasses().size()) << "Unexpected derived class count after derived classes were loaded";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, GetDerivedECClassesWithoutIncrementalLoading)
    {
    BeFileName testFilePath(ECDbTestUtility::BuildECDbPath("ecschemamanagertest.ecdb"));
    SetupTestECDb(testFilePath);

    ECDb testFile;
    ASSERT_EQ(BE_SQLITE_OK, testFile.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(Db::OpenMode::Readonly))) << "Could not open test file " << testFilePath.GetNameUtf8().c_str();

    auto const& schemaManager = testFile.Schemas();
    ECSchemaCP testSchema = schemaManager.GetECSchema(TEST_SCHEMA_NAME, true);
    ASSERT_TRUE(testSchema != nullptr);

    ECClassCP baseClass = schemaManager.GetECClass(TEST_SCHEMA_NAME, "Base");
    ASSERT_TRUE(baseClass != nullptr) << "Could not retrieve base class";

    ASSERT_EQ(2, baseClass->GetDerivedClasses().size()) << "Unexpected derived class count. Derived classes are expected to already be loaded along with having loaded the schema";

    //derived classes are expected to be loaded when calling ECDbSchemaManager::GetDerivedClasses
    ASSERT_EQ(2, schemaManager.GetDerivedECClasses(*baseClass).size()) << "Unexpected derived class count";
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
    ASSERT_TRUE(prop != nullptr && prop->GetAsArrayProperty() != nullptr);
    ASSERT_FALSE(prop->HasExtendedType());

    prop = fooClass->GetPropertyP("Favorites");
    ASSERT_TRUE(prop != nullptr && prop->GetAsArrayProperty() != nullptr);
    ASSERT_TRUE(prop->HasExtendedType());
    ASSERT_STREQ("URL", prop->GetAsArrayProperty()->GetExtendedTypeName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/14
//+---------------+---------------+---------------+---------------+---------------+------
void SetupTestECDb(BeFileNameCR filePath)
    {
    ECDbTestFixture::Initialize();

    if (filePath.DoesPathExist())
        {
        ASSERT_EQ((int) BeFileNameStatus::Success, (int) BeFileName::BeDeleteFile(filePath.c_str())) << "Could not delete existing test ECDb file " << filePath.GetNameUtf8().c_str();
        }

    ECDb ecdb;
    auto stat = ecdb.CreateNewDb(filePath);
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Could not create test ECDb file for path " << filePath.GetNameUtf8().c_str();

    auto testSchema = CreateTestSchema();
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    schemaCache->AddSchema(*testSchema);
    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(*schemaCache)) << "Could not import test ECSchema.";

    ecdb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaPtr CreateTestSchema()
    {
    ECSchemaPtr testSchema = nullptr;
    auto stat = ECSchema::CreateSchema(testSchema, TEST_SCHEMA_NAME, 1, 0);
    EXPECT_EQ(ECObjectsStatus::Success, stat);
    testSchema->SetNamespacePrefix("test");
    if (testSchema == nullptr)
        return nullptr;

    ECEntityClassP baseClass = nullptr;
    stat = testSchema->CreateEntityClass(baseClass, "Base");
    EXPECT_EQ(ECObjectsStatus::Success, stat);
    if (baseClass == nullptr)
        return nullptr;

    PrimitiveECPropertyP prop = nullptr;
    baseClass->CreatePrimitiveProperty(prop, "bprop", PrimitiveType::PRIMITIVETYPE_Double);

    ECEntityClassP sub1Class = nullptr;
    stat = testSchema->CreateEntityClass(sub1Class, "Sub1");
    EXPECT_EQ(ECObjectsStatus::Success, stat);
    if (sub1Class == nullptr)
        return nullptr;

    sub1Class->CreatePrimitiveProperty(prop, "s1prop", PrimitiveType::PRIMITIVETYPE_Long);
    sub1Class->AddBaseClass(*baseClass);

    ECEntityClassP sub1Sub1Class = nullptr;
    stat = testSchema->CreateEntityClass(sub1Sub1Class, "Sub1Sub1");
    EXPECT_EQ(ECObjectsStatus::Success, stat);
    if (sub1Sub1Class == nullptr)
        return nullptr;

    sub1Sub1Class->CreatePrimitiveProperty(prop, "s1s1prop", PrimitiveType::PRIMITIVETYPE_Long);
    sub1Sub1Class->AddBaseClass(*sub1Class);

    ECEntityClassP sub1Sub2Class = nullptr;
    stat = testSchema->CreateEntityClass(sub1Sub2Class, "Sub1Sub2");
    EXPECT_EQ(ECObjectsStatus::Success, stat);
    if (sub1Sub2Class == nullptr)
        return nullptr;

    sub1Sub2Class->CreatePrimitiveProperty(prop, "s1s2prop", PrimitiveType::PRIMITIVETYPE_Long);
    sub1Sub2Class->AddBaseClass(*sub1Class);

    ECEntityClassP sub2Class = nullptr;
    stat = testSchema->CreateEntityClass(sub2Class, "Sub2");
    EXPECT_EQ(ECObjectsStatus::Success, stat);
    if (sub2Class == nullptr)
        return nullptr;

    sub2Class->CreatePrimitiveProperty(prop, "s2prop", PrimitiveType::PRIMITIVETYPE_Long);
    sub2Class->AddBaseClass(*baseClass);

    ECEntityClassP sub2Sub1Class = nullptr;
    stat = testSchema->CreateEntityClass(sub2Sub1Class, "Sub2Sub1");
    EXPECT_EQ(ECObjectsStatus::Success, stat);
    if (sub2Sub1Class == nullptr)
        return nullptr;

    sub2Sub1Class->CreatePrimitiveProperty(prop, "s2s1prop", PrimitiveType::PRIMITIVETYPE_Long);
    sub2Sub1Class->AddBaseClass(*sub2Class);

    return testSchema;
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
    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(*schemacache)) << "could not import test ecschema.";
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

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(*schemaCache));

    ECClassCP ecclass = ecdb.Schemas().GetECClass("BaseSchemaA", "Address");
    ASSERT_TRUE(ecclass != nullptr) << "Class with the specified name doesn't exist :- ecclass is empty";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, ImportSchemaWithReferenceToECDbMap10)
    {
            {
            SchemaItem testSchema(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "            <SchemaMap xmlns='ECDbMap.02.00'>"
                "                <TablePrefix>mytables_</TablePrefix>"
                "            </SchemaMap>"
                "</ECSchema>");

            ECDb ecdb;
            bool asserted = false;
            AssertSchemaImport(ecdb, asserted, testSchema, "ecdbmap10schemareferencetest.ecdb");
            ASSERT_FALSE(asserted);

            Utf8String sql;
            sql.Sprintf("SELECT c.Name, c.CustomAttributeContainerType FROM ec_Class c, ec_Schema s WHERE c.SchemaId=s.Id AND s.Name='ECDbMap' AND c.Type=%d", (int) ECClassType::CustomAttribute);
            Statement stmt;
            ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, sql.c_str()));
            while (BE_SQLITE_ROW == stmt.Step())
                {
                ASSERT_FALSE(stmt.IsColumnNull(1)) << "CustomAttributeContainerType must not be NULL for ECDbMap CAs";

                Utf8CP caClassName = stmt.GetValueText(0);
                CustomAttributeContainerType actualType = (CustomAttributeContainerType) stmt.GetValueInt(1);
                if (BeStringUtilities::StricmpAscii(caClassName, "SchemaMap") == 0)
                    ASSERT_EQ(CustomAttributeContainerType::Schema, actualType);
                else if (BeStringUtilities::StricmpAscii(caClassName, "ClassMap") == 0)
                    ASSERT_EQ(CustomAttributeContainerType::EntityClass | CustomAttributeContainerType::RelationshipClass, actualType);
                else if (BeStringUtilities::StricmpAscii(caClassName, "PropertyMap") == 0)
                    ASSERT_EQ(CustomAttributeContainerType::PrimitiveProperty, actualType);
                else if (BeStringUtilities::StricmpAscii(caClassName, "LinkTableRelationshipMap") == 0 ||
                         BeStringUtilities::StricmpAscii(caClassName, "ForeignKeyRelationshipMap") == 0)
                    ASSERT_EQ(CustomAttributeContainerType::RelationshipClass, actualType);
                else if (BeStringUtilities::StricmpAscii(caClassName, "DisableECInstanceIdAutogeneration") == 0)
                    ASSERT_EQ(CustomAttributeContainerType::EntityClass, actualType);
                else
                    FAIL() << "New CA was added to ECDbMap ECSchema. Adjust this ATP accordingly";
                }
            }

            {
            SchemaItem testSchema(
                "<?xml version='1.0' encoding='utf-8'?>"
                "<ECSchema schemaName='TestSchema2' nameSpacePrefix='ts2' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                "    <ECEntityClass typeName='ClassA'>"
                "        <ECCustomAttributes>"
                "            <SchemaMap xmlns='ECDbMap.02.00'>"
                "                <TablePrefix>mytables_</TablePrefix>"
                "            </SchemaMap>"
                "        </ECCustomAttributes>"
                "        <ECProperty propertyName='Price' typeName='double' />"
                "    </ECEntityClass>"
                "</ECSchema>",
                false, "SchemaMap CA cannot be applied to an ECClass");

            AssertSchemaImport(testSchema, "ecdbmap10schemareferencetest.ecdb");
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, SchemaCache)
    {
    ECDbTestFixture::Initialize();
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    ECSchemaPtr schemaPtr = NULL;
    ECSchemaReadContextPtr  context = nullptr;

    ECDbTestUtility::ReadECSchemaFromDisk(schemaPtr, context, L"BaseSchemaA.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr != NULL);
    schemaCache->AddSchema(*schemaPtr);
    ECDbTestUtility::ReadECSchemaFromDisk(schemaPtr, context, L"DSCacheSchema.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr != NULL);
    schemaCache->AddSchema(*schemaPtr);
    ECDbTestUtility::ReadECSchemaFromDisk(schemaPtr, context, L"TestSchema.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr != NULL);
    schemaCache->AddSchema(*schemaPtr);

    EXPECT_EQ(5, schemaCache->GetCount()) << "Number of schema doesn't matches the number of schema read form the disk";

    SchemaKeyCR key = schemaPtr->GetSchemaKey();
    EXPECT_EQ(4, schemaPtr->GetClassCount());

    ECSchemaPtr schemaPtr1 = schemaCache->GetSchema(key);
    EXPECT_TRUE(schemaPtr1 != NULL);
    ASSERT_TRUE(schemaPtr1.IsValid());

    ASSERT_EQ(ECObjectsStatus::Success, schemaCache->DropSchema(key));
    EXPECT_EQ(4, schemaCache->GetCount()) << "Number of schema doesn't matches the number of schema read form the disk";
    schemaCache->Clear();
    EXPECT_EQ(0, schemaCache->GetCount()) << "Couldn't clear the cache";
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

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(*Schemacache)) << "could not import test ecschema.";
    ecdb.SaveChanges();

    ASSERT_EQ(ERROR, testecdb.Schemas().ImportECSchemas(*Schemacache)) << "could not import test ecschema in the 2nd ecdb file.";
    testecdb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, ImportMultipleSupplementalSchemas)
    {
    ECDbTestFixture::Initialize();
    ECDb testecdb;
    auto stat = ECDbTestUtility::CreateECDb(testecdb, nullptr, L"supplementalSchematest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    ECSchemaReadContextPtr context = nullptr;

    ECSchemaPtr schemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(schemaptr, context, L"SchoolSchema.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaptr != NULL);
    ECSchemaPtr supple;
    ECDbTestUtility::ReadECSchemaFromDisk(supple, context, L"SchoolSchema_Supplemental_Localization.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(supple != NULL);
    ECSchemaPtr supple1;
    ECDbTestUtility::ReadECSchemaFromDisk(supple1, context, L"SchoolSchema_Supplemental_Units.01.01.ecschema.xml", nullptr);
    ASSERT_TRUE(supple1 != NULL);

    ECSchemaCachePtr schemacache = ECSchemaCache::Create();
    schemacache->AddSchema(*schemaptr);
    schemacache->AddSchema(*supple);
    schemacache->AddSchema(*supple1);

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemacache)) << "couldn't import the schema";
    ECSchemaCP SchoolSupplSchema = testecdb.Schemas().GetECSchema("SchoolSchema", true);
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


    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemaCache)) << "couldn't import the schema";
    ECSchemaCP SchoolSupplSchema = testecdb.Schemas().GetECSchema("SchoolSchema", true);
    ASSERT_TRUE(SchoolSupplSchema != NULL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  1/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, ImportReferenceSchemaReferedByMultipleSchemas)
    {
    ECDbTestFixture::Initialize();
    ECDb testecdb;
    auto stat = ECDbTestUtility::CreateECDb(testecdb, nullptr, L"referancedSchematest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);
    ECSchemaReadContextPtr  context = nullptr;

    ECSchemaPtr schemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(schemaptr, context, L"StartupCompany.02.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaptr != NULL);
    ECSchemaPtr supplementalSchemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(supplementalSchemaptr, context, L"StartupCompany_Supplemental_ECDbTest.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(supplementalSchemaptr != NULL);

    ECSchemaCachePtr schemacache = ECSchemaCache::Create();
    schemacache->AddSchema(*schemaptr);
    schemacache->AddSchema(*supplementalSchemaptr);

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemacache)) << "couldn't import the schema";
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

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemacache)) << "couldn't import the schema";
    ECSchemaCP SchoolSupplSchema = testecdb.Schemas().GetECSchema("SchoolSchema", true);
    ASSERT_TRUE(SchoolSupplSchema != NULL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbSchemaManagerTests, TestGetClassResolver)
    {
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create("ecschemamanagertest.ecdb", L"TestSchema.01.00.ecschema.xml", true);
    ECClassCP ecClass = ecdbr.Schemas().GetECClass("TestSchema", "DerivedTestClass");
    EXPECT_TRUE(ecClass != nullptr);
    ecClass = ecdbr.Schemas().GetECClass("TS", "DerivedTestClass", ResolveSchema::BySchemaNamespacePrefix);
    EXPECT_TRUE(ecClass != nullptr);

    ecClass = ecdbr.Schemas().GetECClass("TestSchema", "DerivedTestClass", ResolveSchema::AutoDetect);
    EXPECT_TRUE(ecClass != nullptr);

    ecClass = ecdbr.Schemas().GetECClass("TS", "DerivedTestClass", ResolveSchema::AutoDetect);
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

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemacache)) << "couldn't import the schema";
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

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemacache)) << "couldn't import the schema";
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
TEST_F(ECDbSchemaManagerTests, CreateECClassViews)
    {
    ECDbR ecdb = SetupECDb("CreateECClassViews.ecdb");
    ASSERT_TRUE(ecdb.IsDbOpen());

    ASSERT_EQ(SUCCESS, ecdb.Schemas().CreateECClassViewsInDb());
    ecdb.SaveChanges();
    std::map<Utf8String, std::set<Utf8String>> schemasWithECClassViews = GetECViewNamesByPrefix(ecdb);
    ASSERT_EQ(2, schemasWithECClassViews.size()) << "Unexpected number of schemas with ECClassViews";
    ASSERT_EQ(4, schemasWithECClassViews["ecdbf"].size()) << "Unexpected number of ECClassViews";
    ECSchemaReadContextPtr  context = nullptr;
    ECSchemaPtr schemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(schemaptr, context, L"StartupCompany.02.00.ecschema.xml", nullptr);

    ECSchemaCachePtr schemacache = ECSchemaCache::Create();
    schemacache->AddSchema(*schemaptr);

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(*schemacache)) << "couldn't import the schema";
    ASSERT_EQ(SUCCESS, ecdb.Schemas().CreateECClassViewsInDb());
    ecdb.SaveChanges();
    schemasWithECClassViews = GetECViewNamesByPrefix(ecdb);
    ASSERT_EQ(3, schemasWithECClassViews.size()) << "Unexpected number of schemas with ECClassViews";
    ASSERT_EQ(4, schemasWithECClassViews["ecdbf"].size()) << "Unexpected number of ECClassViews";
    ASSERT_EQ(38, schemasWithECClassViews["stco"].size()) << "Unexpected number of ECClassViews";
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

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemacache)) << "couldn't import the schema";
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

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemacache)) << "couldn't import the schema";
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
    auto setup = [] (ECInstanceKey& activityKey, ECDbCR ecdb, bool clearCacheAfterFirstImport)
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
        ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(context1->GetCache()));

        if (clearCacheAfterFirstImport)
            ecdb.ClearECDbCache();

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
        ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(context2->GetCache()));

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

    //Import two ECSchemas separately without clearing the cache before the second import
    {
    ECDbTestProject testProject;
    ECDbR ecdb = testProject.Create("importschemawithsubclassestoexistingschema1.ecdb");

    ECInstanceKey activityKey;
    setup(activityKey, ecdb, false);
    ASSERT_TRUE(activityKey.IsValid());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT PlanId, OutlineIndex FROM p.Activity WHERE ECInstanceId=?"));
    stmt.BindId(1, activityKey.GetECInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_FALSE(stmt.IsValueNull(0)) << "This should start to fail if ECDb still caching horizontal paratition even after import a second schema";
    ASSERT_FALSE(stmt.IsValueNull(1)) << "This should start to fail if ECDb still caching horizontal paratition even after import a second schema";

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    //Import two ECSchemas separately with clearing the cache before the second import
    {
    ECDbTestProject testProject;
    ECDbR ecdb = testProject.Create("importschemawithsubclassestoexistingschema2.ecdb");

    ECInstanceKey activityKey;
    setup(activityKey, ecdb, true);
    ASSERT_TRUE(activityKey.IsValid());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT PlanId, OutlineIndex FROM p.Activity WHERE ECInstanceId=?"));
    stmt.BindId(1, activityKey.GetECInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_TRUE(!stmt.IsValueNull(0));
    ASSERT_EQ(100ULL, stmt.GetValueInt64(0));
    ASSERT_TRUE(!stmt.IsValueNull(1));
    ASSERT_EQ(100, stmt.GetValueInt(1));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
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

END_ECDBUNITTESTS_NAMESPACE

