/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbSchemaManager_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

static Utf8CP const TEST_SCHEMA_NAME = "ECDbSchemaManagerTest";

void SetupTestECDb (BeFileNameCR filePath);
ECSchemaPtr CreateTestSchema ();

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbSchemaManager, IncrementalLoading)
    {
    BeFileName testFilePath (ECDbTestProject::BuildECDbPath ("ecschemamanagertest.ecdb"));
    SetupTestECDb (testFilePath);

    const int expectedClassCount = CreateTestSchema ()->GetClassCount ();

    //GetECSchema with ensureAllClassesLoaded = false
    {
    ECDb ecdb;
    auto stat = ecdb.OpenBeSQLiteDb (testFilePath, ECDb::OpenParams (ECDb::OpenMode::Readonly));
    ASSERT_EQ (BE_SQLITE_OK, stat);

    auto const& schemaManager = ecdb. Schemas ();

    ECSchemaCP schema = schemaManager.GetECSchema (TEST_SCHEMA_NAME, false);
    ASSERT_TRUE (schema != nullptr);

    ASSERT_EQ (0, schema->GetClassCount ()) << "ECDbSchemaManager::GetECSchema (..., false) is expected to return an empty schema";
    }

    //GetECSchema with ensureAllClassesLoaded = true
    {
    ECDb ecdb;
    auto stat = ecdb.OpenBeSQLiteDb (testFilePath, ECDb::OpenParams (ECDb::OpenMode::Readonly));
    ASSERT_EQ (BE_SQLITE_OK, stat);

    auto const& schemaManager = ecdb. Schemas ();

    ECSchemaCP schema = schemaManager.GetECSchema (TEST_SCHEMA_NAME, true);
    ASSERT_TRUE (schema != nullptr);

    ASSERT_EQ (expectedClassCount, schema->GetClassCount ()) << "ECDbSchemaManager::GetECSchema (..., true) is expected to return a fully populated schema";
    }

    //GetECClass from a different schema first and then GetECSchema with ensureAllClassesLoaded = true
    {
    ECDb ecdb;
    auto stat = ecdb.OpenBeSQLiteDb (testFilePath, ECDb::OpenParams (ECDb::OpenMode::Readonly));
    ASSERT_EQ (BE_SQLITE_OK, stat);

    auto const& schemaManager = ecdb. Schemas ();

    ECClassCP ecClass = schemaManager.GetECClass ("ECDbSystem", "ArrayOfPrimitives");
    ASSERT_TRUE (ecClass != nullptr) << "ECDbSchemaManager::GetECClass ('ECDbSystem', 'ArrayOfPrimitives') is expected to succeed as the class exists in the ecdb file.";

    ECSchemaCP schema = schemaManager.GetECSchema (TEST_SCHEMA_NAME, false);
    ASSERT_TRUE (schema != nullptr);
    ASSERT_EQ (0, schema->GetClassCount ()) << "ECDbSchemaManager::GetECSchema (..., false) is expected to return a schema with only the classes already loaded.";

    schema = schemaManager.GetECSchema (TEST_SCHEMA_NAME, true);
    ASSERT_TRUE (schema != nullptr);

    ASSERT_EQ (expectedClassCount, schema->GetClassCount ()) << "ECDbSchemaManager::GetECSchema (..., true) is expected to return a fully populated schema even if ECDbSchemaManager::GetECClass was called before for a class in a different ECSchema.";
    }

    //GetECClass from same schema first and then GetECSchema with ensureAllClassesLoaded = true
    {
    ECDb ecdb;
    auto stat = ecdb.OpenBeSQLiteDb (testFilePath, ECDb::OpenParams (ECDb::OpenMode::Readonly));
    ASSERT_EQ (BE_SQLITE_OK, stat);

    auto const& schemaManager = ecdb. Schemas ();

    ECClassCP ecClass = schemaManager.GetECClass (TEST_SCHEMA_NAME, "Base");
    ASSERT_TRUE (ecClass != nullptr) << "ECDbSchemaManager::GetECClass ('Base') is expected to succeed as the class exists in the ecdb file.";
    
    ECSchemaCP schema = schemaManager.GetECSchema (TEST_SCHEMA_NAME, false);
    ASSERT_TRUE (schema != nullptr);
    ASSERT_EQ (1, schema->GetClassCount ()) << "ECDbSchemaManager::GetECSchema (..., false) is expected to return a schema with only the classes already loaded.";

    schema = schemaManager.GetECSchema (TEST_SCHEMA_NAME, true);
    ASSERT_TRUE (schema != nullptr);

    ASSERT_EQ (expectedClassCount, schema->GetClassCount ()) << "ECDbSchemaManager::GetECSchema (..., true) is expected to return a fully populated schema even if ECDbSchemaManager::GetECClass was called before for a class in the same schema.";
    }
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbSchemaManager, GetDerivedECClasses)
    {
    BeFileName testFilePath (ECDbTestProject::BuildECDbPath ("ecschemamanagertest.ecdb"));
    SetupTestECDb (testFilePath);

    ECDb testFile;
    ASSERT_EQ (BE_SQLITE_OK, testFile.OpenBeSQLiteDb (testFilePath, ECDb::OpenParams (Db::OpenMode::Readonly))) << "Could not open test file " << testFilePath.GetNameUtf8 ().c_str ();

    auto const& schemaManager = testFile. Schemas ();

    ECClassCP baseClass = schemaManager.GetECClass (TEST_SCHEMA_NAME, "Base");
    ASSERT_TRUE (baseClass != nullptr) << "Could not retrieve base class";

    //derived classes are not loaded when calling ECClass::GetDerivedClasses
    ASSERT_TRUE (baseClass->GetDerivedClasses ().empty ()) << "ECClass::GetDerivedClasses is expected to not load subclasses.";

    //derived classes are expected to be loaded when calling ECDbSchemaManager::GetDerivedClasses
    ASSERT_EQ (2, schemaManager.GetDerivedECClasses (*baseClass).size ()) << "Unexpected derived class count with derived classes now being loaded";

    //now ECClass::GetDerivedClasses can also be called
    ASSERT_EQ (2, baseClass->GetDerivedClasses ().size ()) << "Unexpected derived class count after derived classes were loaded";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbSchemaManager, GetDerivedECClassesWithoutIncrementalLoading)
    {
    BeFileName testFilePath (ECDbTestProject::BuildECDbPath ("ecschemamanagertest.ecdb"));
    SetupTestECDb (testFilePath);

    ECDb testFile;
    ASSERT_EQ (BE_SQLITE_OK, testFile.OpenBeSQLiteDb (testFilePath, ECDb::OpenParams (Db::OpenMode::Readonly))) << "Could not open test file " << testFilePath.GetNameUtf8 ().c_str ();

    auto const& schemaManager = testFile. Schemas ();
    ECSchemaCP testSchema = schemaManager.GetECSchema (TEST_SCHEMA_NAME, true);
    ASSERT_TRUE (testSchema != nullptr);

    ECClassCP baseClass = schemaManager.GetECClass (TEST_SCHEMA_NAME, "Base");
    ASSERT_TRUE (baseClass != nullptr) << "Could not retrieve base class";

    ASSERT_EQ (2, baseClass->GetDerivedClasses ().size ()) << "Unexpected derived class count. Derived classes are expected to already be loaded along with having loaded the schema";

    //derived classes are expected to be loaded when calling ECDbSchemaManager::GetDerivedClasses
    ASSERT_EQ (2, schemaManager.GetDerivedECClasses (*baseClass).size ()) << "Unexpected derived class count";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/14
//+---------------+---------------+---------------+---------------+---------------+------
void SetupTestECDb (BeFileNameCR filePath)
    {
    ECDbTestProject::Initialize ();

    if (filePath.DoesPathExist ())
        {
        ASSERT_EQ ((int) BeFileNameStatus::Success, (int) BeFileName::BeDeleteFile (filePath.c_str ())) << "Could not delete existing test ECDb file " << filePath.GetNameUtf8 ().c_str();
        }

    ECDb ecdb;
    auto stat = ecdb.CreateNewDb (filePath);
    ASSERT_EQ (BE_SQLITE_OK, stat) << "Could not create test ECDb file for path " << filePath.GetNameUtf8 ().c_str ();

    auto testSchema = CreateTestSchema ();
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create ();
    schemaCache->AddSchema (*testSchema);
    ASSERT_EQ (SUCCESS, ecdb. Schemas ().ImportECSchemas (*schemaCache, ECDbSchemaManager::ImportOptions ())) << "Could not import test ECSchema.";

    ecdb.SaveChanges ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSchemaPtr CreateTestSchema ()
    {
    ECSchemaPtr testSchema = nullptr;
    auto stat = ECSchema::CreateSchema (testSchema, WString (TEST_SCHEMA_NAME, BentleyCharEncoding::Utf8), 1, 0);
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat);
    testSchema->SetNamespacePrefix (L"test");
    if (testSchema == nullptr)
        return nullptr;

    ECClassP baseClass = nullptr;
    stat = testSchema->CreateClass (baseClass, L"Base");
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat);
    if (baseClass == nullptr)
        return nullptr;

    PrimitiveECPropertyP prop = nullptr;
    baseClass->CreatePrimitiveProperty (prop, L"bprop", PrimitiveType::PRIMITIVETYPE_Double);


    ECClassP sub1Class = nullptr;
    stat = testSchema->CreateClass (sub1Class, L"Sub1");
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat);
    if (sub1Class == nullptr)
        return nullptr;

    sub1Class->CreatePrimitiveProperty (prop, L"s1prop", PrimitiveType::PRIMITIVETYPE_Long);
    sub1Class->AddBaseClass (*baseClass);

    ECClassP sub1Sub1Class = nullptr;
    stat = testSchema->CreateClass (sub1Sub1Class, L"Sub1Sub1");
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat);
    if (sub1Sub1Class == nullptr)
        return nullptr;

    sub1Sub1Class->CreatePrimitiveProperty (prop, L"s1s1prop", PrimitiveType::PRIMITIVETYPE_Long);
    sub1Sub1Class->AddBaseClass (*sub1Class);

    ECClassP sub1Sub2Class = nullptr;
    stat = testSchema->CreateClass (sub1Sub2Class, L"Sub1Sub2");
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat);
    if (sub1Sub2Class == nullptr)
        return nullptr;

    sub1Sub2Class->CreatePrimitiveProperty (prop, L"s1s2prop", PrimitiveType::PRIMITIVETYPE_Long);
    sub1Sub2Class->AddBaseClass (*sub1Class);

    ECClassP sub2Class = nullptr;
    stat = testSchema->CreateClass (sub2Class, L"Sub2");
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat);
    if (sub2Class == nullptr)
        return nullptr;

    sub2Class->CreatePrimitiveProperty (prop, L"s2prop", PrimitiveType::PRIMITIVETYPE_Long);
    sub2Class->AddBaseClass (*baseClass);

    ECClassP sub2Sub1Class = nullptr;
    stat = testSchema->CreateClass (sub2Sub1Class, L"Sub2Sub1");
    EXPECT_EQ (ECOBJECTS_STATUS_Success, stat);
    if (sub2Sub1Class == nullptr)
        return nullptr;

    sub2Sub1Class->CreatePrimitiveProperty (prop, L"s2s1prop", PrimitiveType::PRIMITIVETYPE_Long);
    sub2Sub1Class->AddBaseClass (*sub2Class);

    return testSchema;
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                   Krischan.Eberle                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
struct SchemaImportTestItem
    {
    Utf8String m_schemaXml;
    bool m_expectedToSucceed;
    Utf8String m_assertMessage;

    SchemaImportTestItem(Utf8CP schemaXml, bool expectedToSucceeed, Utf8CP assertMessage) : m_schemaXml(schemaXml), m_expectedToSucceed(expectedToSucceeed), m_assertMessage(assertMessage) {}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
void AssertSchemaImport(SchemaImportTestItem const& testItem)
    {
    ECDb ecdb;
    auto stat = ECDbTestUtility::CreateECDb(ecdb, nullptr, L"importinvalidecschema.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    auto schemaCache = ECDbTestUtility::ReadECSchemaFromString(testItem.m_schemaXml.c_str());
    ASSERT_TRUE(schemaCache != nullptr) << testItem.m_assertMessage.c_str();

    if (!testItem.m_expectedToSucceed)
        BeTest::SetFailOnAssert(false);

    {
    ASSERT_EQ(testItem.m_expectedToSucceed, SUCCESS == ecdb.Schemas().ImportECSchemas(*schemaCache)) << testItem.m_assertMessage.c_str();
    }

    BeTest::SetFailOnAssert(true);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  04/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbSchemaManager, ImportSchemaWithSchemaValidationErrors)
    {
    ECDbTestProject::Initialize ();

    std::vector <SchemaImportTestItem> testItems {
        SchemaImportTestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" >"
        "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"TESTCLASS\" >"
        "    <ECProperty propertyName=\"Property\" typeName=\"string\" />"
        "  </ECClass>"
        "</ECSchema>",
        false, "Classes with names differing only by case."),

        SchemaImportTestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" >"
        "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
        "    <ECProperty propertyName=\"TESTPROPERTY\" typeName=\"string\" />"
        "  </ECClass>"
        "</ECSchema>",
        false, "Properties only differing by case within a class."),

        SchemaImportTestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" >"
        "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"SubClass\" >"
        "    <BaseClass>TestClass</BaseClass>"
        "    <ECProperty propertyName=\"TESTPROPERTY\" typeName=\"string\" />"
        "  </ECClass>"
        "</ECSchema>",
        false, "Properties only differing by case in a sub and base class."),

        SchemaImportTestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" >"
        "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"TestClass2\" >"
        "    <ECProperty propertyName=\"TESTPROPERTY\" typeName=\"string\" />"
        "  </ECClass>"
        "</ECSchema>",
        true, "Properties differing only by case in two unrelated classes."),

        SchemaImportTestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" >"
        "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"SubClass\" >"
        "    <BaseClass>TestClass</BaseClass>"
        "    <ECProperty propertyName=\"TESTPROPERTY\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"TESTCLASS\" >"
        "    <ECProperty propertyName=\"Property\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"TESTClass\" >"
        "    <ECProperty propertyName=\"Property\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"Foo\" >"
        "    <ECProperty propertyName=\"Property\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"FOO\" >"
        "    <ECProperty propertyName=\"Property\" typeName=\"string\" />"
        "    <ECProperty propertyName=\"PROPerty\" typeName=\"string\" />"
        "    <ECProperty propertyName=\"PROPERTY\" typeName=\"string\" />"
        "    <ECProperty propertyName=\"Prop2\" typeName=\"string\" />"
        "    <ECProperty propertyName=\"PROP2\" typeName=\"string\" />"
        "  </ECClass>"
        "</ECSchema>",
        false, "Class and properties only differing by case within a class."),

        SchemaImportTestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" isStruct=\"true\" isDomainClass=\"true\">"
        "    <ECStructProperty propertyName=\"Prop1\" typeName=\"TestClass\" />"
        "  </ECClass>"
        "</ECSchema>",
        false, "Property is of same type as class."),

        SchemaImportTestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"Base\" isStruct=\"true\" isDomainClass=\"true\" >"
        "    <ECStructProperty propertyName=\"Prop1\" typeName=\"Sub\" isStruct=\"true\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"Sub\" isStruct=\"true\">"
        "     <BaseClass>Base</BaseClass>"
        "    <ECProperty propertyName=\"Prop2\" typeName=\"string\" />"
        "  </ECClass>"
        "</ECSchema>",
        false, "Property is of subtype of class."),

        SchemaImportTestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" isStruct=\"true\" isDomainClass=\"true\">"
        "    <ECArrayProperty propertyName=\"Prop1\" typeName=\"TestClass\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
        "  </ECClass>"
        "</ECSchema>",
        false, "Property is array of class."),

        SchemaImportTestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"Base\" isStruct=\"true\" isDomainClass=\"true\">"
        "    <ECArrayProperty propertyName=\"Prop1\" typeName=\"Sub\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
        "  </ECClass>"
        "  <ECClass typeName=\"Sub\" isStruct=\"true\">"
        "     <BaseClass>Base</BaseClass>"
        "    <ECProperty propertyName=\"Prop2\" typeName=\"string\" />"
        "  </ECClass>"
        "</ECSchema>",
        false, "Property is of array of subclass of class."),

        SchemaImportTestItem("<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"Base\" isStruct=\"true\" isDomainClass=\"true\">"
        "    <ECArrayProperty propertyName=\"Prop1\" typeName=\"Sub\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
        "  </ECClass>"
        "  <ECClass typeName=\"Sub\" isStruct=\"true\">"
        "     <BaseClass>Base</BaseClass>"
        "    <ECProperty propertyName=\"PROP1\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"SUB\" >"
        "    <ECProperty propertyName=\"PROP1\" typeName=\"string\" />"
        "  </ECClass>"
        "</ECSchema>",
        false, "Case-sensitive class and prop names and property is of array of subclass of class."),


        SchemaImportTestItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"A\" >"
        "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"B\" >"
        "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"Base\" >"
        "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName = \"Rel\" isDomainClass = \"True\" strength = \"holding\" strengthDirection = \"forward\">"
        "    <BaseClass>Base</BaseClass>"
        "    <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "      <Class class = \"A\" />"
        "    </Source>"
        "    <Target cardinality = \"(0, N)\" polymorphic = \"True\">"
        "      <Class class = \"B\" />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>",
        false, "RelationshipClass with non-relationship base class is not expected to be supported."),

        SchemaImportTestItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"A\" >"
        "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"B\" >"
        "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName = \"RelBase\" isDomainClass = \"True\" strength = \"holding\" strengthDirection = \"forward\">"
        "    <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "      <Class class = \"A\" />"
        "    </Source>"
        "    <Target cardinality = \"(0, N)\" polymorphic = \"True\">"
        "      <Class class = \"B\" />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "  <ECClass typeName=\"Cl\" >"
        "    <BaseClass>RelBase</BaseClass>"
        "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
        "  </ECClass>"        
        "</ECSchema>",
        false, "Non-relationship class with a relationship base class is not expected to be supported."),

        SchemaImportTestItem("<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"A\" >"
        "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"B\" >"
        "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName = \"RelBase\" isDomainClass = \"True\" strength = \"holding\" strengthDirection = \"forward\">"
        "    <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "      <Class class = \"A\" />"
        "    </Source>"
        "    <Target cardinality = \"(0, N)\" polymorphic = \"True\">"
        "      <Class class = \"B\" />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "  <ECClass typeName=\"Cl\" >"
        "    <BaseClass>RelBase</BaseClass>"
        "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
        "  </ECClass>"        
        "</ECSchema>",
        false, "Non-relationship class with a relationship base class is not expected to be supported.")
        };

        for (SchemaImportTestItem const& testItem : testItems)
            {
            AssertSchemaImport(testItem);
            }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  04/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbMap, RelationshipMapCAOnSubclasses)
    {
    ECDbTestProject::Initialize();

    ECDb ecdb;
    auto stat = ECDbTestUtility::CreateECDb(ecdb, nullptr, L"RelationshipMapCAOnSubclasses.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    //Importing the DGN ECSchema requires the dgn_RTree3d table to pre-exist (Which a DgnDb file ensures). So
    //we add the table manually here
    ASSERT_EQ(BE_SQLITE_OK, ecdb.ExecuteSql("CREATE VIRTUAL TABLE dgn_RTree3d USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ);"));

    Utf8CP testSchemaXml =
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECSchemaReference name = 'Bentley_Standard_CustomAttributes' version = '01.11' prefix = 'bsca' />"
        "  <ECSchemaReference name = 'ECDbMap' version = '01.00' prefix = 'ecdbmap' />"
        "  <ECSchemaReference name = 'dgn' version = '02.00' prefix = 'dgn' />"
        "  <ECClass typeName='MyElement' >"
        "    <BaseClass>dgn:Element</BaseClass>"
        "    <ECProperty propertyName='MyName' typeName='string' />"
        "  </ECClass>"
        "  <ECClass typeName='YourElement' >"
        "    <BaseClass>dgn:Element</BaseClass>"
        "    <ECProperty propertyName='YourName' typeName='string' />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName='MyElementHasYourElements' isDomainClass='True' strength='embedding'>"
        "    <ECCustomAttributes>"
        "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
        "            <ForeignKeyColumn>ParentId</ForeignKeyColumn>"
        "        </ForeignKeyRelationshipMap>"
        "    </ECCustomAttributes>"
        "   <BaseClass>dgn:ElementOwnsChildElements</BaseClass>"
        "    <Source cardinality='(1,1)' polymorphic='True'>"
        "      <Class class = 'MyElement' />"
        "    </Source>"
        "    <Target cardinality='(0,N)' polymorphic='True'>"
        "      <Class class = 'YourElement' />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>";

    BeFileName searchPath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(searchPath);
    searchPath.AppendToPath(L"ECSchemas");
    searchPath.AppendToPath(L"Dgn");

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    schemaContext->AddSchemaLocater(ecdb.GetSchemaLocater());
    schemaContext->AddSchemaPath(searchPath.GetName());

    ECDbTestUtility::ReadECSchemaFromString(schemaContext, testSchemaXml);
    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(schemaContext->GetCache()));

    bvector<Utf8String> columns;
    ASSERT_TRUE (ecdb.GetColumns(columns, "dgn_element"));
    ASSERT_EQ(11, columns.size()) << "dgn_element table should not contain an extra foreign key column as the relationship map specifies to use the ParentId column";
    
    auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
    auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
    ASSERT_TRUE(it == columns.end()) << "dgn_element table should not contain an extra foreign key column as the relationship map specifies to use the ParentId column";
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
void CreateECDbAndImportSchema(ECDbR ecdb, WCharCP ecdbName, Utf8CP ecschemaXml, bool schemaImportExpectedToSucceed, Utf8CP assertMessage = "")
    {
    const DbResult stat = ECDbTestUtility::CreateECDb(ecdb, nullptr, ecdbName);
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECDbTestUtility::ReadECSchemaFromString(schemaContext, ecschemaXml);
    if (schemaImportExpectedToSucceed)
        ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(schemaContext->GetCache())) << assertMessage;
    else
        ASSERT_EQ(ERROR, ecdb.Schemas().ImportECSchemas(schemaContext->GetCache())) << assertMessage;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TryGetHasForeignKey(bool& hasForeignKey, ECDbCR ecdb, Utf8CP tableName, Utf8CP foreignKeyColumnName = nullptr)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(ecdb, "SELECT sql FROM sqlite_master WHERE name=?"))
        return ERROR;

    stmt.BindText(1, tableName, Statement::MakeCopy::No);
    if (BE_SQLITE_ROW != stmt.Step())
        return ERROR;

    Utf8String ddl(stmt.GetValueText(0));

    //onlw one row expected
    if (BE_SQLITE_ROW == stmt.Step())
        return ERROR;

    Utf8String fkSearchString;
    if (Utf8String::IsNullOrEmpty(foreignKeyColumnName))
        fkSearchString = "FOREIGN KEY (";
    else
        fkSearchString.Sprintf("FOREIGN KEY ([%s", foreignKeyColumnName);

    hasForeignKey = (ddl.find(fkSearchString) != ddl.npos);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbMap, ForeignKeyMapWhereLinkTableIsRequired)
    {
    ECDbTestProject::Initialize();
    WCharCP ecdbName = L"ForeignKeyMapWhereLinkTableIsRequired.ecdb";

        {
        Utf8CP testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
            "</ECSchema>";

        ECDb ecdb;
        CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, false);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbMap, ForeignKeyMapWithKeyProperty)
    {
    ECDbTestProject::Initialize();
    WCharCP ecdbName = L"ForeignKeyMapWithKeyProp.ecdb";
    Utf8CP childTableName = "ts_Child";

        {
        Utf8CP testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
            "</ECSchema>";

        ECDb ecdb;
        CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, true);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(3, columns.size()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it == columns.end()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

        bool actualForeignKey = false;
        ASSERT_EQ(SUCCESS, TryGetHasForeignKey(actualForeignKey, ecdb, childTableName));
        ASSERT_EQ(false, actualForeignKey);
        }

        {
        Utf8CP testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
            "</ECSchema>";

        ECDb ecdb;
        CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, true);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "parent_id"));
        ASSERT_FALSE(ecdb.ColumnExists(childTableName, "ParentId"));
        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(3, columns.size()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it == columns.end()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

        bool actualForeignKey = false;
        ASSERT_EQ(SUCCESS, TryGetHasForeignKey(actualForeignKey, ecdb, childTableName));
        ASSERT_EQ(false, actualForeignKey);
        }

    {
    Utf8CP testSchemaXml =
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
        "            <CreateConstraint>True</CreateConstraint>"
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
        "</ECSchema>";

    ECDb ecdb;
    CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, true);

    ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
    bvector<Utf8String> columns;
    ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
    ASSERT_EQ(3, columns.size()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

    auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
    auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
    ASSERT_TRUE(it == columns.end()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";
    
    bool actualForeignKey = false;
    ASSERT_EQ(SUCCESS, TryGetHasForeignKey(actualForeignKey, ecdb, childTableName, "ParentId"));
    ASSERT_EQ(true, actualForeignKey);
    }

    {
    Utf8CP testSchemaXml =
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
        "</ECSchema>";

    ECDb ecdb;
    CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, false , "ForeignKeyColumn should not be specified if Key property is defined.");
    }

    {
    Utf8CP testSchemaXml =
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
        "</ECSchema>";

    ECDb ecdb;
    CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, false, "ForeignKeyColumn should not be specified if Key property is defined.");
    }

    {
    Utf8CP testSchemaXml =
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
        "                   <IsPolymorphic>True</IsPolymorphic>"
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
        "</ECSchema>";

    ECDb ecdb;
    CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, false, "Only one constraint class supported by ECDb if key properties are defined.");
    }
    
    {
    Utf8CP testSchemaXml =
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
        "</ECSchema>";

    ECDb ecdb;
    CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, false, "Only one key property is supported by ECDb.");
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbMap, ForeignKeyMapWithECInstanceIdKeyProperty)
    {
    ECDbTestProject::Initialize();
    WCharCP ecdbName = L"ForeignKeyMapWithECInstanceIdKeyProp.ecdb";
    Utf8CP childTableName = "ts_Child";

        {
        Utf8CP testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
            "</ECSchema>";

        ECDb ecdb;
        CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, true);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ECInstanceId"));
        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(2, columns.size()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies that the ECInstanceId is the foreign key";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it == columns.end()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

        bool actualForeignKey = false;
        ASSERT_EQ(SUCCESS, TryGetHasForeignKey(actualForeignKey, ecdb, childTableName));
        ASSERT_EQ(false, actualForeignKey);
        }

        {
        Utf8CP testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
            "</ECSchema>";

        ECDb ecdb;
        CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, true);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "Id"));
        ASSERT_FALSE(ecdb.ColumnExists(childTableName, "ECInstanceId"));

        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(2, columns.size()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies that the ECInstanceId is the foreign key";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it == columns.end()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

        bool actualForeignKey = false;
        ASSERT_EQ(SUCCESS, TryGetHasForeignKey(actualForeignKey, ecdb, childTableName));
        ASSERT_EQ(false, actualForeignKey);
        }

        {
        Utf8CP testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
            "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
            "            <CreateConstraint>True</CreateConstraint>"
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
            "</ECSchema>";

        ECDb ecdb;
        CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, true);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "Id"));
        ASSERT_FALSE(ecdb.ColumnExists(childTableName, "ECInstanceId"));

        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(2, columns.size()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies that the ECInstanceId is the foreign key";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it == columns.end()) << childTableName << " table should not contain an extra foreign key column as the relationship specifies a Key property";

        bool actualForeignKey = false;
        ASSERT_EQ(SUCCESS, TryGetHasForeignKey(actualForeignKey, ecdb, childTableName, "Id"));
        ASSERT_EQ(true, actualForeignKey);
        }

        {
        Utf8CP testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
            "</ECSchema>";

        ECDb ecdb;
        CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, false);
        }

        {
        Utf8CP testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
            "</ECSchema>";

        ECDb ecdb;
        CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, false);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  06/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbMap, ForeignKeyMapWithoutKeyProperty)
    {
    ECDbTestProject::Initialize();

    WCharCP ecdbName = L"ForeignKeyMapWithoutKeyProp.ecdb";
    Utf8CP childTableName = "ts_Child";

        {
        Utf8CP testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
            "</ECSchema>";

        ECDb ecdb;
        CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, true);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(3, columns.size()) << childTableName << " table should not contain an extra foreign key column as the relationship map specifies an existing column name";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it == columns.end()) << childTableName << " table should not contain an extra foreign key column as the relationship map specifies an existing column name";

        bool actualForeignKey = false;
        ASSERT_EQ(SUCCESS, TryGetHasForeignKey(actualForeignKey, ecdb, childTableName));
        ASSERT_EQ(false, actualForeignKey);
        }

        {
        Utf8CP testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
            "</ECSchema>";

        ECDb ecdb;
        CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, true);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "MyOwnParentId"));
        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(4, columns.size()) << childTableName << " table should contain an extra foreign key column as the relationship map specifies a value for ForeignKeyColumn";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it == columns.end()) << childTableName << " table should contain an extra foreign key column as the relationship map specifies a value for ForeignKeyColumn";

        bool actualForeignKey = false;
        ASSERT_EQ(SUCCESS, TryGetHasForeignKey(actualForeignKey, ecdb, childTableName));
        ASSERT_EQ(false, actualForeignKey);
        }

        {
        Utf8CP testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
            "</ECSchema>";

        ECDb ecdb;
        CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, true);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(4, columns.size()) << childTableName << " table should contain a default-name extra foreign key column as there is no relationship map CA";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it != columns.end()) << "ts_child table should contain a default-name extra foreign key column as there is no relationship map CA";

        bool actualForeignKey = false;
        ASSERT_EQ(SUCCESS, TryGetHasForeignKey(actualForeignKey, ecdb, childTableName));
        ASSERT_EQ(false, actualForeignKey);
        }

        {
        Utf8CP testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
            "</ECSchema>";

        ECDb ecdb;
        CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, true);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(4, columns.size()) << childTableName << " table should contain a default-name extra foreign key column as there is the relationship map CA doesn't specify a value for ForeignKeyColumn";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it != columns.end()) << childTableName << " table should contain a default-name extra foreign key column as there is the relationship map CA doesn't specify a value for ForeignKeyColumn";

        bool actualForeignKey = false;
        ASSERT_EQ(SUCCESS, TryGetHasForeignKey(actualForeignKey, ecdb, childTableName));
        ASSERT_EQ(false, actualForeignKey);
        }

        {
        Utf8CP testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
            "           <CreateConstraint>True</CreateConstraint>"  
            "        </ForeignKeyRelationshipMap>"
            "    </ECCustomAttributes>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class = 'Parent' />"
            "    </Source>"
            "    <Target cardinality='(0,N)' polymorphic='True'>"
            "      <Class class = 'Child' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>";

        ECDb ecdb;
        CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, true);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(4, columns.size()) << childTableName << " table should contain a default-name extra foreign key column as there is the relationship map CA doesn't specify a value for ForeignKeyColumn";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it != columns.end()) << childTableName << " table should contain a default-name extra foreign key column as there is the relationship map CA doesn't specify a value for ForeignKeyColumn";

        bool actualForeignKey = false;
        ASSERT_EQ(SUCCESS, TryGetHasForeignKey(actualForeignKey, ecdb, childTableName, "ForeignEC"));
        ASSERT_EQ(true, actualForeignKey);
        }

        {
        Utf8CP testSchemaXml =
            "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
            "                   <IsPolymorphic>True</IsPolymorphic>"
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
            "        <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
            "           <CreateConstraint>True</CreateConstraint>"
            "        </ForeignKeyRelationshipMap>"
            "    </ECCustomAttributes>"
            "    <Source cardinality='(1,1)' polymorphic='True'>"
            "      <Class class = 'Parent' />"
            "    </Source>"
            "    <Target cardinality='(0,N)' polymorphic='True'>"
            "      <Class class = 'Child' />"
            "      <Class class = 'Child2' />"
            "    </Target>"
            "  </ECRelationshipClass>"
            "</ECSchema>";

        ECDb ecdb;
        CreateECDbAndImportSchema(ecdb, ecdbName, testSchemaXml, true);

        ASSERT_TRUE(ecdb.ColumnExists(childTableName, "ParentId"));
        bvector<Utf8String> columns;
        ASSERT_TRUE(ecdb.GetColumns(columns, childTableName));
        ASSERT_EQ(6, columns.size()) << childTableName << " table should contain a default-name extra foreign key column as there is the relationship map CA doesn't specify a value for ForeignKeyColumn";

        auto containsDefaultNamedRelationalKeyColumn = [] (Utf8StringCR str) { return BeStringUtilities::Strnicmp(str.c_str(), "ForeignEC", 9) == 0; };
        auto it = std::find_if(columns.begin(), columns.end(), containsDefaultNamedRelationalKeyColumn);
        ASSERT_TRUE(it != columns.end()) << childTableName << " table should contain a default-name extra foreign key column as there is the relationship map CA doesn't specify a value for ForeignKeyColumn";

        bool actualForeignKey = false;
        ASSERT_EQ(SUCCESS, TryGetHasForeignKey(actualForeignKey, ecdb, childTableName, "ForeignEC"));
        ASSERT_EQ(true, actualForeignKey);
        }

    }


//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
//Importing a schema containing all the possible combinations of class properties and relationship classes having built-in and user defined cardinalities 
TEST(ECDbSchemaManager, ImportSchema)
    {
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create("ecschemamanagertest.ecdb", L"UserWorkBench.01.00.ecschema.xml", true);
    ECClassCP ecclass = ecdbr. Schemas ().GetECClass ("UserWorkBench", "areas");
    ASSERT_TRUE (ecclass != nullptr);
    ecclass = ecdbr. Schemas ().GetECClass ("UserWorkBench", "house_user");
    ASSERT_TRUE (ecclass != nullptr);
    }
//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemaManager, AddDuplicateECSchemaInCache)
{
    ECDbTestProject::Initialize();

    ECDb ecdb;
    auto stat = ECDbTestUtility::CreateECDb(ecdb, nullptr, L"ecschemamanagertest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    ECSchemaPtr schemaPtr = ECDbTestUtility::ReadECSchemaFromDisk(L"BaseSchemaA.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr != NULL);
    ECSchemaPtr schemaPtr1 = ECDbTestUtility::ReadECSchemaFromDisk(L"BaseSchemaA.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr1 != NULL);

    ECSchemaCachePtr schemacache = ECSchemaCache::Create();
    ASSERT_EQ(ECOBJECTS_STATUS_Success, schemacache->AddSchema(*schemaPtr)) << "couldn't add schema to the cache" << schemaPtr->GetName().c_str();
    ASSERT_EQ(ECOBJECTS_STATUS_DuplicateSchema, schemacache->AddSchema(*schemaPtr1));
    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions())) << "could not import test ecschema.";
    ecdb.SaveChanges();

    ECClassCP ecclass = ecdb.Schemas().GetECClass("BaseSchemaA", "Address");
    EXPECT_TRUE(ecclass != NULL) << "ecclass with the specified name does not exist";
}

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemaManager, ImportDuplicateSchema)
{
    ECDbTestProject::Initialize();

    ECDb ecdb;
    auto stat = ECDbTestUtility::CreateECDb(ecdb, nullptr, L"ecschemamanagertest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    ECSchemaPtr schemaPtr = ECDbTestUtility::ReadECSchemaFromDisk(L"BaseSchemaA.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr != NULL);
    ECSchemaPtr schemaPtr1 = ECDbTestUtility::ReadECSchemaFromDisk(L"BaseSchemaA.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr1 != NULL);

    ECSchemaCachePtr Schemacache = ECSchemaCache::Create();
    Schemacache->AddSchema(*schemaPtr);
    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(*Schemacache, ECDbSchemaManager::ImportOptions()));
    ECSchemaCachePtr Schemacache1 = ECSchemaCache::Create();
    Schemacache1->AddSchema(*schemaPtr1);
    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(*Schemacache1, ECDbSchemaManager::ImportOptions()));
    ecdb.SaveChanges();

    ECClassCP ecclass = ecdb.Schemas().GetECClass("BaseSchemaA", "Address");
    EXPECT_TRUE(ecclass != nullptr) << "Class with the specified name doesn't exist :- ecclass is empty";
}

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  09/14
//+---------------+---------------+---------------+---------------+---------------+------

TEST(ECDbSchemaManager, UpdateExistingSchema)
{
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create("ecschemamanagertest.ecdb", L"TestSchema.01.00.ecschema.xml", true);
    ECSchemaPtr schemaPtr = NULL;
    ECSchemaReadContextPtr  context = nullptr;
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    ECDbTestUtility::ReadECSchemaFromDisk(schemaPtr, context, L"TestSchema.01.01.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr.IsValid());
    ASSERT_TRUE(schemaPtr != NULL);
    schemaCache->AddSchema(*schemaPtr);

    BeTest::SetFailOnAssert(false);
    Savepoint s(ecdbr, "test");
    ASSERT_EQ(ERROR, ecdbr.Schemas().ImportECSchemas(*schemaCache, ECDbSchemaManager::ImportOptions(false, true))) << "couldn't update the existing schema";
    BeTest::SetFailOnAssert(true);
    s.Cancel();

    ECSchemaCP schemap = ecdbr. Schemas ().GetECSchema ("TestSchema", true);
    ASSERT_TRUE(schemap != nullptr);
    wprintf(L"%s\n", schemap->GetName().c_str());
    ASSERT_EQ(4, schemap->GetClassCount()) << "Class count doesn't match the original number of classes";

    ECClassCP ecclass = ecdbr. Schemas ().GetECClass ("TestSchema", "DerivedTestClass");
    EXPECT_TRUE (ecclass != nullptr);
    ecclass = ecdbr. Schemas ().GetECClass ("TestSchema", "DerivedClass");
    EXPECT_TRUE (ecclass == nullptr);
    ecclass = ecdbr. Schemas ().GetECClass ("TestSchema", "FileInfo");
    EXPECT_TRUE (ecclass != nullptr);
    EXPECT_EQ (3, ecclass->GetPropertyCount ());
    ECPropertyIterable iterator = ecclass->GetProperties();
    int i = 0;
    for (ECPropertyP ecpropertyp: iterator)
    {
        ASSERT_TRUE(ecpropertyp != NULL);
        ecpropertyp = NULL;
        i++;
    }
    ASSERT_EQ(3, i) << "The number of Properties returned by the iterator doesn't match the Original Number";
}

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemaManager, UpdateExsitingSchemaDifferntCache)
{

    ECDbTestProject::Initialize();
    ECDb testEcdb;
    DbResult dbresult = ECDbTestUtility::CreateECDb(testEcdb, nullptr, L"ecschemamanagertest.ecdb");
    ASSERT_TRUE(dbresult == BE_SQLITE_OK) << "Couldn't create ecdb";

    ECSchemaPtr schemaPtr = ECDbTestUtility::ReadECSchemaFromDisk(L"RSComponents.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr != NULL) << "couldn't read the schema: value = null";
    ECSchemaPtr schemaPtr1 = ECDbTestUtility::ReadECSchemaFromDisk(L"RSComponents.01.01.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr1 != NULL) << "couldn't read the schema: value = null";

    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    ASSERT_EQ(SUCCESS, schemaCache->AddSchema(*schemaPtr));
    ECSchemaCachePtr schemaCache1 = ECSchemaCache::Create();
    ASSERT_EQ(SUCCESS, schemaCache1->AddSchema(*schemaPtr1));

    ASSERT_EQ(SUCCESS, testEcdb.Schemas().ImportECSchemas(*schemaCache, ECDbSchemaManager::ImportOptions())) << "Couldn't import the first version of Schema";
    ECClassCP ecClass = testEcdb. Schemas ().GetECClass ("RSComponents", "TestClass");
    EXPECT_TRUE(ecClass == NULL);
    ASSERT_EQ(SUCCESS, testEcdb.Schemas().ImportECSchemas(*schemaCache1, ECDbSchemaManager::ImportOptions(false, true))) << "Couldn't import or update the existing schema";
    testEcdb.SaveChanges();

    //getting the updated schema
    ECSchemaCP schemap = testEcdb. Schemas ().GetECSchema ("RSComponents", true);
    ASSERT_TRUE (schemap != nullptr);
    ASSERT_EQ(16, schemap->GetClassCount()) << "Class count doesn't match the original number of classes";

    ecClass = testEcdb.Schemas().GetECClass("RSComponents", "CAMERA");
    ASSERT_TRUE(ecClass != NULL);

    //Getting property count in the class with base class (true) and without base class (false)
    ASSERT_EQ(3, ecClass->GetPropertyCount(false));
    ASSERT_EQ(8, ecClass->GetPropertyCount(true));

    //Getting property by name in the class with base class (true) and without base class (false)
    ECPropertyP ecProperty = ecClass->GetPropertyP("SENSOR_TYPE", false);
    ASSERT_TRUE(ecProperty != NULL) << "couldn't get the propety in the given class";
    ecProperty = ecClass->GetPropertyP("VERSION", true);
    ASSERT_TRUE(ecProperty != NULL) << "couldn't get the propety in the base class of the given class";

    //Property iterator
    ECPropertyIterable iterator = ecClass->GetProperties();
    int i = 0;
    for(ECPropertyP ecpropertyp: iterator)
    {
        ASSERT_TRUE(ecpropertyp != NULL);
        ecpropertyp = NULL;
        i++;
    }
    ASSERT_EQ(8, i) << "Properties returned by Iterator doesn't match the Original Number";

    ecClass = testEcdb.Schemas().GetECClass("RSComponents", "SONOR");
    ASSERT_TRUE(ecClass != NULL);
    ecClass = testEcdb.Schemas().GetECClass("RSComponents", "SC");
    ASSERT_TRUE(ecClass != NULL);
    EXPECT_EQ(1, ecClass->GetPropertyCount()) << "Property count not equal";
}

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemaManager, SchemaCache)
{
    ECDbTestProject::Initialize();
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

    ASSERT_EQ(ECOBJECTS_STATUS_Success, schemaCache->DropSchema(key));
    EXPECT_EQ(4, schemaCache->GetCount()) << "Number of schema doesn't matches the number of schema read form the disk";
    schemaCache->Clear();
    EXPECT_EQ(0, schemaCache->GetCount()) << "Couldn't clear the cache";
}

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemaManager, ImportingSchemaInDifferentECDB)
{
    ECDbTestProject::Initialize();
    ECDb ecdb, testecdb;
    auto stat = ECDbTestUtility::CreateECDb(ecdb, nullptr, L"testecdbSchema.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);
    stat = ECDbTestUtility::CreateECDb(testecdb, nullptr, L"testecdbSchema1.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    ECSchemaPtr schemaPtr = ECDbTestUtility::ReadECSchemaFromDisk(L"BaseSchemaA.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr != NULL);

    ECSchemaCachePtr Schemacache = ECSchemaCache::Create();
    Schemacache->AddSchema(*schemaPtr);

    ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportECSchemas(*Schemacache, ECDbSchemaManager::ImportOptions())) << "could not import test ecschema.";
    ecdb.SaveChanges();

    ASSERT_EQ(ERROR, testecdb.Schemas().ImportECSchemas(*Schemacache, ECDbSchemaManager::ImportOptions())) << "could not import test ecschema in the 2nd ecdb file.";
    testecdb.SaveChanges();
}

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemaManager, ImportSupplementedSchemaDoSupplementationFalse)
{
    ECDbTestProject::Initialize();
    ECDb testecdb;
    auto stat = ECDbTestUtility::CreateECDb(testecdb, nullptr, L"supplementalSchematest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);
    ECSchemaCachePtr schemaCache = ECSchemaCache::Create();
    ECSchemaPtr schemaPtr = NULL;
    ECSchemaReadContextPtr  context = nullptr;

    ECDbTestUtility::ReadECSchemaFromDisk(schemaPtr, context, L"SchoolSchema.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr != NULL);
    schemaCache->AddSchema(*schemaPtr);

    ECDbTestUtility::ReadECSchemaFromDisk(schemaPtr, context, L"SchoolSchema_Supplemental_Localization.01.00.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr != NULL);
    schemaCache->AddSchema(*schemaPtr);    
    ECDbTestUtility::ReadECSchemaFromDisk(schemaPtr, context, L"SchoolSchema_Supplemental_Units.01.01.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaPtr != NULL);
    schemaCache->AddSchema(*schemaPtr);

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemaCache, ECDbSchemaManager::ImportOptions(false, false))) << "couldn't import the schema";
    ECSchemaCP SchoolSupplSchema = testecdb.Schemas().GetECSchema("SchoolSchema", true);
    ASSERT_TRUE (SchoolSupplSchema != nullptr);

    ECClassCP ecclassCourseTitle = SchoolSupplSchema->GetClassCP(L"CourseTitle");
    ASSERT_TRUE(ecclassCourseTitle != NULL);
    ECCustomAttributeInstanceIterable iterator1 = ecclassCourseTitle->GetCustomAttributes(true);
    uint32_t i = 0;
    for (IECInstancePtr instance : iterator1)
    {
        i++;
    }
    EXPECT_EQ(2, i) << "the number of custom attributes on the Class Department do not match the original";
}

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemaManager, ImportMultipalSupplementalSchemas)
{
    ECDbTestProject::Initialize();
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

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
    ECSchemaCP SchoolSupplSchema = testecdb.Schemas().GetECSchema("SchoolSchema", true);
    ASSERT_TRUE (SchoolSupplSchema != NULL);

    ECClassCP ecclassCourse = SchoolSupplSchema->GetClassCP(L"Course");
    ASSERT_TRUE(ecclassCourse != NULL);
    ECClassCP ecclassCourseTitle = SchoolSupplSchema->GetClassCP(L"CourseTitle");
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

    ECClassCP relationshipClass = SchoolSupplSchema->GetClassCP(L"SchoolDepartmentRelation");
    ASSERT_TRUE(relationshipClass != NULL);
    ECCustomAttributeInstanceIterable iterator = relationshipClass->GetCustomAttributes(false);
    i = 0;
    for (IECInstancePtr instance : iterator)
    {
        i++;
    }
    EXPECT_EQ(5, i) << "the number of custom attributes on the Class relationshipClass do not match the original";

    ECClassCP ecclasscp = SchoolSupplSchema->GetClassCP(L"Department");
    ASSERT_TRUE(ecclasscp != NULL) << "couldn't read the class Department from schema";
    IECInstancePtr iecinstancePtr = ecclasscp->GetCustomAttribute(L"ChangeManagement");
    ASSERT_TRUE(iecinstancePtr.IsValid()) << "couldn't retreive the custom attribute from the class Department";
}

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemaManager, ImportLowPrioritySupplementalSchama)
{
    ECDbTestProject::Initialize();
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


    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemaCache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
    ECSchemaCP SchoolSupplSchema = testecdb.Schemas().GetECSchema("SchoolSchema", true);
    ASSERT_TRUE (SchoolSupplSchema != NULL);
    }

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  1/15
//+---------------+---------------+---------------+---------------+---------------+------
 TEST(ECDbSchemaManager, ImportReferenceSchemaReferedByMultipleSchemas)
    {
     ECDbTestProject::Initialize();
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

     ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
    }
 //---------------------------------------------------------------------------------------
 //                                               Muhammad Hassan                  10/14
 //+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemaManager, ImportHighPrioritySupplementalSchama)
{
    ECDbTestProject::Initialize();
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

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
    ECSchemaCP SchoolSupplSchema = testecdb.Schemas().GetECSchema("SchoolSchema", true);
    ASSERT_TRUE (SchoolSupplSchema != NULL);
    }

TEST(ECDbSchemaManager, TestGetClassResolver)
    {
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create("ecschemamanagertest.ecdb", L"TestSchema.01.00.ecschema.xml", true);
    ECClassCP ecClass = ecdbr. Schemas ().GetECClass ("TestSchema", "DerivedTestClass");
    EXPECT_TRUE (ecClass != nullptr);
    ecClass = ecdbr. Schemas ().GetECClass ("TS", "DerivedTestClass", ECDbSchemaManager::ResolveSchema::BySchemaNamespacePrefix);
    EXPECT_TRUE (ecClass != nullptr);

    ecClass = ecdbr. Schemas ().GetECClass ("TestSchema", "DerivedTestClass", ECDbSchemaManager::ResolveSchema::AutoDetect);
    EXPECT_TRUE (ecClass != nullptr);

    ecClass = ecdbr. Schemas ().GetECClass ("TS", "DerivedTestClass", ECDbSchemaManager::ResolveSchema::AutoDetect);
    EXPECT_TRUE (ecClass != nullptr);
    }
//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
// A primary schema should be supplemented with the latest available supplemental schema
TEST(ECDbSchemaManager, supplementSchemaWithLatestSupplementalSchema)
{

    ECDbTestProject::Initialize();
    ECDb testecdb;
    auto stat = ECDbTestUtility::CreateECDb(testecdb, nullptr, L"supplementalSchematest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    ECSchemaReadContextPtr context = nullptr;

    ECSchemaPtr schemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(schemaptr, context, L"BasicSchema.01.70.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaptr != NULL);
    ECSchemaPtr supple;
    ECDbTestUtility::ReadECSchemaFromDisk(supple, context, L"BasicSchema_Supplemental_Localization.01.10.ecschema.xml", nullptr);
    ASSERT_TRUE(supple != NULL);
    ECSchemaPtr supple1;
    ECDbTestUtility::ReadECSchemaFromDisk(supple1, context, L"BasicSchema_Supplemental_Localization.01.60.ecschema.xml", nullptr);
    ASSERT_TRUE(supple1 != NULL);
    ECSchemaPtr supple2;
    ECDbTestUtility::ReadECSchemaFromDisk(supple2, context, L"BasicSchema_Supplemental_Localization.01.90.ecschema.xml", nullptr);
    ASSERT_TRUE(supple2 != NULL);

    ECSchemaCachePtr schemacache = ECSchemaCache::Create();
    schemacache->AddSchema(*schemaptr);
    schemacache->AddSchema(*supple);
    schemacache->AddSchema(*supple1);
    schemacache->AddSchema(*supple2);

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
    ECSchemaCP basicSupplSchema = testecdb.Schemas().GetECSchema("BasicSchema", true);
    ASSERT_TRUE (basicSupplSchema != NULL);

    ECClassCP ecclassBase = basicSupplSchema->GetClassCP(L"Base");
    ASSERT_TRUE(ecclassBase != NULL);
    //get custom attributes from base class (false)
    ECCustomAttributeInstanceIterable iterator1 = ecclassBase->GetCustomAttributes(true);
    int i = 0;
    for (IECInstancePtr instance : iterator1)
    {
        i++;
    }
    EXPECT_EQ(3, i) << "the number of custom attributes on the Class Base do not match the original";
}
//supplement schema with a supplemental schema whose primary schema's major version is greater then the major version of current primary schema.
TEST(ECDbSchemaManager, supplementSchemaWithGreaterMajorVersionPrimary)
{

    ECDbTestProject::Initialize();
    ECDb testecdb;
    auto stat = ECDbTestUtility::CreateECDb(testecdb, nullptr, L"supplementalSchematest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    ECSchemaReadContextPtr context = nullptr;

    ECSchemaPtr schemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(schemaptr, context, L"BasicSchema.01.70.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaptr != NULL);
    ECSchemaPtr supple;
    ECDbTestUtility::ReadECSchemaFromDisk(supple, context, L"BasicSchema_Supplemental_Localization.02.10.ecschema.xml", nullptr);
    ASSERT_TRUE(supple != NULL);

    ECSchemaCachePtr schemacache = ECSchemaCache::Create();
    schemacache->AddSchema(*schemaptr);
    schemacache->AddSchema(*supple);

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
    ECSchemaCP basicSupplSchema = testecdb.Schemas().GetECSchema("BasicSchema", true);
    ASSERT_TRUE (basicSupplSchema != NULL);

    ECClassCP ecclassBase = basicSupplSchema->GetClassCP(L"Base");
    ASSERT_TRUE(ecclassBase != NULL);
    //get custom attributes from base class (false)
    ECCustomAttributeInstanceIterable iterator1 = ecclassBase->GetCustomAttributes(true);
    int i = 0;
    for (IECInstancePtr instance : iterator1)
    {
        i++;
    }
    EXPECT_EQ(0, i) << "the number of custom attributes on the Class Base do not match the original";
}
// supplement current primary schema with a supplemental schema whose primary schema's minor version is less then the current schema.
TEST(ECDbSchemaManager, supplementSchemaWithLessMinorVersionPrimarySchema)
{

    ECDbTestProject::Initialize();
    ECDb testecdb;
    auto stat = ECDbTestUtility::CreateECDb(testecdb, nullptr, L"supplementalSchematest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    ECSchemaReadContextPtr context = nullptr;

    ECSchemaPtr schemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(schemaptr, context, L"BasicSchema.01.70.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaptr != NULL);
    ECSchemaPtr supple;
    ECDbTestUtility::ReadECSchemaFromDisk(supple, context, L"BasicSchema_Supplemental_Localization.01.69.ecschema.xml", nullptr);
    ASSERT_TRUE(supple != NULL);

    ECSchemaCachePtr schemacache = ECSchemaCache::Create();
    schemacache->AddSchema(*schemaptr);
    schemacache->AddSchema(*supple);

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
    ECSchemaCP basicSupplSchema = testecdb.Schemas().GetECSchema("BasicSchema", true);
    ASSERT_TRUE (basicSupplSchema != NULL);

    ECClassCP ecclassBase = basicSupplSchema->GetClassCP(L"Base");
    ASSERT_TRUE(ecclassBase != NULL);
    //get custom attributes from base class (false)
    ECCustomAttributeInstanceIterable iterator1 = ecclassBase->GetCustomAttributes(true);
    int i = 0;
    for (IECInstancePtr instance : iterator1)
    {
        i++;
    }
    EXPECT_EQ(0, i) << "the number of custom attributes on the Class Base do not match the original";
}
// suppelement schema with a supplemental schema whose primary schema's minor version is greater then the current.
TEST(ECDbSchemaManager, supplementSchemaWithGreaterMinorVersionPrimarySchema)
{

    ECDbTestProject::Initialize();
    ECDb testecdb;
    auto stat = ECDbTestUtility::CreateECDb(testecdb, nullptr, L"supplementalSchematest.ecdb");
    ASSERT_TRUE(stat == BE_SQLITE_OK);

    ECSchemaReadContextPtr context = nullptr;

    ECSchemaPtr schemaptr;
    ECDbTestUtility::ReadECSchemaFromDisk(schemaptr, context, L"BasicSchema.01.69.ecschema.xml", nullptr);
    ASSERT_TRUE(schemaptr != NULL);
    ECSchemaPtr supple;
    ECDbTestUtility::ReadECSchemaFromDisk(supple, context, L"BasicSchema_Supplemental_Localization.01.69.ecschema.xml", nullptr);
    ASSERT_TRUE(supple != NULL);
    ECSchemaPtr supple1;
    ECDbTestUtility::ReadECSchemaFromDisk(supple1, context, L"BasicSchema_Supplemental_Localization.01.90.ecschema.xml", nullptr);
    ASSERT_TRUE(supple1 != NULL);

    ECSchemaCachePtr schemacache = ECSchemaCache::Create();
    schemacache->AddSchema(*schemaptr);
    schemacache->AddSchema(*supple);
    schemacache->AddSchema(*supple1);

    ASSERT_EQ(SUCCESS, testecdb.Schemas().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
    ECSchemaCP basicSupplSchema = testecdb. Schemas ().GetECSchema ("BasicSchema", true);
    ASSERT_TRUE (basicSupplSchema != NULL);

    ECClassCP ecclassBase = basicSupplSchema->GetClassCP(L"Base");
    ASSERT_TRUE(ecclassBase != NULL);
    //get custom attributes from base class (false)
    ECCustomAttributeInstanceIterable iterator1 = ecclassBase->GetCustomAttributes(true);
    int i = 0;
    for (IECInstancePtr instance : iterator1)
    {
        i++;
    }
    EXPECT_EQ(3, i) << "the number of custom attributes on the Class Base do not match the original";
}

TEST(ECDbSchemaManager, ECDbImportSchema_WSG2eBPluginSchemas_Succeeds)
    {
    BeFileName assetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDir);

    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    ECDb::Initialize(temporaryDir, &assetsDir);
    bvector<BeFileName> schemaPaths;
    BeFileName ecSchemaPath;
    BeTest::GetHost().GetDocumentsRoot(ecSchemaPath);
    schemaPaths.push_back(BeFileName(ecSchemaPath).AppendToPath(BeFileName(L"DgnDb\\ECDb\\Schemas\\Contents.01.00.ecschema.xml")));
    schemaPaths.push_back(BeFileName(ecSchemaPath).AppendToPath(BeFileName(L"DgnDb\\ECDb\\Schemas\\EC_to_eB_Dynamic_Schema.01.00.ecschema.xml")));
    schemaPaths.push_back(BeFileName(ecSchemaPath).AppendToPath(BeFileName(L"DgnDb\\ECDb\\Schemas\\EC_to_eB_Mapping_Custom_Attributes.01.00.ecschema.xml")));
    schemaPaths.push_back(BeFileName(ecSchemaPath).AppendToPath(BeFileName(L"DgnDb\\ECDb\\Schemas\\Forms_EC_Mapping.01.00.ecschema.xml")));
    schemaPaths.push_back(BeFileName(ecSchemaPath).AppendToPath(BeFileName(L"DgnDb\\ECDb\\Schemas\\MetaSchema.02.00.ecschema.xml")));
    schemaPaths.push_back(BeFileName(ecSchemaPath).AppendToPath(BeFileName(L"DgnDb\\ECDb\\Schemas\\Navigation.01.00.ecschema.xml")));
    schemaPaths.push_back(BeFileName(ecSchemaPath).AppendToPath(BeFileName(L"DgnDb\\ECDb\\Schemas\\Policies.01.00.ecschema.xml")));
    schemaPaths.push_back(BeFileName(ecSchemaPath).AppendToPath(BeFileName(L"DgnDb\\ECDb\\Schemas\\Views.01.00.ecschema.xml")));

    // Mimic import logic in CachingDataSource & DataSourceCache
    auto context = ECSchemaReadContext::CreateContext();
    for (BeFileNameCR schemaPath : schemaPaths)
        {
        context->AddSchemaPath(schemaPath.GetDirectoryName());
        }

    bvector<ECSchemaPtr> schemas;
    for (BeFileNameCR schemaPath : schemaPaths)
        {
        ECSchemaPtr schema;
        SchemaReadStatus status = ECSchema::ReadFromXmlFile(schema, schemaPath.GetName(), *context);
        if (SchemaReadStatus::SCHEMA_READ_STATUS_Success != status &&
            SchemaReadStatus::SCHEMA_READ_STATUS_DuplicateSchema != status)
            {
            BeAssert(false);
            return;
            }
        schemas.push_back(schema);
        }
    EXPECT_EQ(schemaPaths.size(), schemas.size());

    auto schemaCache = ECSchemaCache::Create();
    for (ECSchemaPtr schema : schemas)
        {
        schemaCache->AddSchema(*schema);
        }

    ECDb db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(":memory:"));
    EXPECT_EQ(SUCCESS, db.Schemas().ImportECSchemas(*schemaCache, ECDbSchemaManager::ImportOptions(true, true)));
    }
    
//---------------------------------------------------------------------------------------
//                                               Krischan.Eberle                  10/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbSchemaManager, ImportSchemaWithSubclassesToBaseClassInExistingSchema)
    {
    Utf8CP baseSchemaXmlTemplate =
        "<ECSchema schemaName=\"BaseSchema\" nameSpacePrefix=\"b\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECSchemaReference name = \"ECDbMap\" version = \"01.00\" prefix = \"ecdbmap\" />"
        "  <ECClass typeName=\"A\" >"
        "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"B\" >"
        "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName = \"Rel\" isDomainClass = \"True\" strength = \"holding\" strengthDirection = \"forward\">"
        "      %s"
        "    <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "      <Class class = \"A\" />"
        "    </Source>"
        "    <Target cardinality = \"(0, N)\" polymorphic = \"True\">"
        "      <Class class = \"B\" />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>";

    Utf8CP secondSchemaXml =
        "<ECSchema schemaName=\"DomainSchema\" nameSpacePrefix=\"d\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECSchemaReference name = \"BaseSchema\" version = \"01.00\" prefix = \"b\" />"
        "  <ECClass typeName=\"AA\" >"
        "    <BaseClass>b:A</BaseClass>"
        "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
        "  </ECClass>"
        "</ECSchema>";

    auto importSchema = [] (Utf8StringR ecdbFilePath, Utf8CP baseSchema, Utf8CP secondSchema)
        {
        ECDbTestProject testProject;
        ECDbR ecdb = testProject.Create ("importschemawithsubclassestoexistingschema.ecdb");
        ecdbFilePath = ecdb.GetDbFileName ();
        auto schemaCache = ECDbTestUtility::ReadECSchemaFromString (baseSchema);
        if (BSISUCCESS != ecdb. Schemas ().ImportECSchemas (*schemaCache))
            return BSIERROR;

        ecdb.SaveChanges ();

        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext ();
        context->AddSchemaLocater (ecdb. GetSchemaLocater ());
        ECDbTestUtility::ReadECSchemaFromString (context, secondSchema);
        if (BSISUCCESS != ecdb. Schemas ().ImportECSchemas (context->GetCache ()))
            return BSIERROR;

        ecdb.SaveChanges ();
        return BSISUCCESS;
        };

    //Test 1: Base schema doesn't explicitly mention class ids for source constraint -> error
        {
        Utf8String ecdbPath;
        Utf8String baseSchemaXml;
        baseSchemaXml.Sprintf (baseSchemaXmlTemplate, "");
        ASSERT_EQ (BSISUCCESS, importSchema (ecdbPath, baseSchemaXml.c_str (), secondSchemaXml));

        ECDb ecdb;
        ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (ecdbPath.c_str (), ECDb::OpenParams (Db::OpenMode::Readonly)));

        ECSqlStatement stmt;
        ASSERT_EQ ((int) ECSqlStatus::InvalidECSql, (int) stmt.Prepare (ecdb, "SELECT SourceECClassId, TargetECClassId FROM b.Rel"));
        }

    //Test 2: Base schema explicitly mentions class ids for source constraint -> correct
        {
        Utf8String ecdbPath;
        Utf8CP customAttributeXml =
            "<ECCustomAttributes>"
            "  <ForeignKeyRelationshipMap xmlns = \"ECDbMap.01.00\">"
            "    <ForeignKeyColumn>SourceECInstanceId</ForeignKeyColumn>"
            "    <ForeignKeyClassIdColumn>SourceECClassId</ForeignKeyClassIdColumn>"
            "  </ForeignKeyRelationshipMap>"
            "</ECCustomAttributes>";

        Utf8String baseSchemaXml;
        baseSchemaXml.Sprintf (baseSchemaXmlTemplate, customAttributeXml);
        ASSERT_EQ (BSISUCCESS, importSchema (ecdbPath, baseSchemaXml.c_str (), secondSchemaXml));

        ECDb ecdb;
        ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (ecdbPath.c_str (), ECDb::OpenParams (Db::OpenMode::Readonly)));

        ASSERT_TRUE (ecdb.ColumnExists ("b_B", "SourceECClassId"));

        ECSqlStatement stmt;
        ASSERT_EQ ((int) ECSqlStatus::Success, (int) stmt.Prepare (ecdb, "SELECT SourceECClassId, TargetECClassId FROM b.Rel"));
        }

    }

END_ECDBUNITTESTS_NAMESPACE
