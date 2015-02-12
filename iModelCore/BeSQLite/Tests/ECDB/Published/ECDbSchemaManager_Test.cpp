/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Published/ECDbSchemaManager_Test.cpp $
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
    auto stat = ecdb.OpenBeSQLiteDb (testFilePath, ECDb::OpenParams (ECDb::OPEN_Readonly, DefaultTxn_Yes));
    ASSERT_EQ (BE_SQLITE_OK, stat);

    auto const& schemaManager = ecdb.GetEC ().GetSchemaManager ();

    ECSchemaP schema = nullptr;
    auto status = schemaManager.GetECSchema (schema, TEST_SCHEMA_NAME, false);
    ASSERT_EQ (SUCCESS, status);

    ASSERT_EQ (0, schema->GetClassCount ()) << "ECDbSchemaManager::GetECSchema (..., false) is expected to return an empty schema";
    }

    //GetECSchema with ensureAllClassesLoaded = true
    {
    ECDb ecdb;
    auto stat = ecdb.OpenBeSQLiteDb (testFilePath, ECDb::OpenParams (ECDb::OPEN_Readonly, DefaultTxn_Yes));
    ASSERT_EQ (BE_SQLITE_OK, stat);

    auto const& schemaManager = ecdb.GetEC ().GetSchemaManager ();

    ECSchemaP schema = nullptr;
    auto status = schemaManager.GetECSchema (schema, TEST_SCHEMA_NAME, true);
    ASSERT_EQ (SUCCESS, status);

    ASSERT_EQ (expectedClassCount, schema->GetClassCount ()) << "ECDbSchemaManager::GetECSchema (..., true) is expected to return a fully populated schema";
    }

    //GetECClass from a different schema first and then GetECSchema with ensureAllClassesLoaded = true
    {
    ECDb ecdb;
    auto stat = ecdb.OpenBeSQLiteDb (testFilePath, ECDb::OpenParams (ECDb::OPEN_Readonly, DefaultTxn_Yes));
    ASSERT_EQ (BE_SQLITE_OK, stat);

    auto const& schemaManager = ecdb.GetEC ().GetSchemaManager ();

    ECClassP ecClass = nullptr;
    auto status = schemaManager.GetECClass (ecClass, "ECDbSystem", "ArrayOfPrimitives");
    ASSERT_EQ (SUCCESS, status) << "ECDbSchemaManager::GetECClass (..., 'ECDbSystem', 'ArrayOfPrimitives') is expected to succeed as the class exists in the ecdb file.";

    ECSchemaP schema = nullptr;
    status = schemaManager.GetECSchema (schema, TEST_SCHEMA_NAME, false);
    ASSERT_EQ (SUCCESS, status);
    ASSERT_EQ (0, schema->GetClassCount ()) << "ECDbSchemaManager::GetECSchema (..., false) is expected to return a schema with only the classes already loaded.";

    status = schemaManager.GetECSchema (schema, TEST_SCHEMA_NAME, true);
    ASSERT_EQ (SUCCESS, status);

    ASSERT_EQ (expectedClassCount, schema->GetClassCount ()) << "ECDbSchemaManager::GetECSchema (..., true) is expected to return a fully populated schema even if ECDbSchemaManager::GetECClass was called before for a class in a different ECSchema.";
    }

    //GetECClass from same schema first and then GetECSchema with ensureAllClassesLoaded = true
    {
    ECDb ecdb;
    auto stat = ecdb.OpenBeSQLiteDb (testFilePath, ECDb::OpenParams (ECDb::OPEN_Readonly, DefaultTxn_Yes));
    ASSERT_EQ (BE_SQLITE_OK, stat);

    auto const& schemaManager = ecdb.GetEC ().GetSchemaManager ();

    ECClassP ecClass = nullptr;
    auto status = schemaManager.GetECClass (ecClass, TEST_SCHEMA_NAME, "Base");
    ASSERT_EQ (SUCCESS, status) << "ECDbSchemaManager::GetECClass (... 'Base') is expected to succeed as the class exists in the ecdb file.";
    
    ECSchemaP schema = nullptr;
    status = schemaManager.GetECSchema (schema, TEST_SCHEMA_NAME, false);
    ASSERT_EQ (SUCCESS, status);
    ASSERT_EQ (1, schema->GetClassCount ()) << "ECDbSchemaManager::GetECSchema (..., false) is expected to return a schema with only the classes already loaded.";

    status = schemaManager.GetECSchema (schema, TEST_SCHEMA_NAME, true);
    ASSERT_EQ (SUCCESS, status);

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
    ASSERT_EQ (BE_SQLITE_OK, testFile.OpenBeSQLiteDb (testFilePath, ECDb::OpenParams (Db::OPEN_Readonly))) << "Could not open test file " << testFilePath.GetNameUtf8 ().c_str ();

    auto const& schemaManager = testFile.GetEC ().GetSchemaManager ();

    ECClassP baseClass = nullptr;
    ASSERT_EQ (SUCCESS, schemaManager.GetECClass (baseClass, TEST_SCHEMA_NAME, "Base")) << "Could not retrieve base class";

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
    ASSERT_EQ (BE_SQLITE_OK, testFile.OpenBeSQLiteDb (testFilePath, ECDb::OpenParams (Db::OPEN_Readonly))) << "Could not open test file " << testFilePath.GetNameUtf8 ().c_str ();

    auto const& schemaManager = testFile.GetEC ().GetSchemaManager ();
    ECSchemaP testSchema = nullptr;
    ASSERT_EQ (SUCCESS, schemaManager.GetECSchema (testSchema, TEST_SCHEMA_NAME, true));

    ECClassP baseClass = nullptr;
    ASSERT_EQ (SUCCESS, schemaManager.GetECClass (baseClass, TEST_SCHEMA_NAME, "Base")) << "Could not retrieve base class";

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
    ASSERT_EQ (SUCCESS, ecdb.GetEC ().GetSchemaManager ().ImportECSchemas (*schemaCache, ECDbSchemaManager::ImportOptions ())) << "Could not import test ECSchema.";

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
// @bsimethod                                   Krischan.Eberle                  04/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbSchemaManager, ImportSchemaWithSchemaValidationErrors)
    {
    ECDbTestProject::Initialize ();

    auto testFunc = [] (Utf8CP testSchemaXml, bool expectedToSucceed, Utf8CP assertMessage)
        {
        ECDb ecdb;
        auto stat = ECDbTestUtility::CreateECDb (ecdb, nullptr, L"importinvalidecschema.ecdb");
        ASSERT_TRUE (stat == BE_SQLITE_OK);

        auto schemaCache = ECDbTestUtility::ReadECSchemaFromString (testSchemaXml);
        ASSERT_TRUE (schemaCache != nullptr) << assertMessage;

        if (!expectedToSucceed)
            BeTest::SetFailOnAssert (false);

            {
            ASSERT_EQ (expectedToSucceed, SUCCESS == ecdb.GetEC ().GetSchemaManager ().ImportECSchemas (*schemaCache)) << assertMessage;
            }

        BeTest::SetFailOnAssert (true);
        };

    //***** Classes only differing by case
    Utf8CP testSchemaXml =
        "<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" >"
        "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"TESTCLASS\" >"
        "    <ECProperty propertyName=\"Property\" typeName=\"string\" />"
        "  </ECClass>"
        "</ECSchema>";
    testFunc (testSchemaXml, false, "Classes with names differing only by case.");


    //***** Properties only differing by case
    testSchemaXml =
        "<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" >"
        "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
        "    <ECProperty propertyName=\"TESTPROPERTY\" typeName=\"string\" />"
        "  </ECClass>"
        "</ECSchema>";
    testFunc (testSchemaXml, false, "Properties only differing by case within a class.");

    testSchemaXml =
        "<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" >"
        "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"SubClass\" >"
        "    <BaseClass>TestClass</BaseClass>"
        "    <ECProperty propertyName=\"TESTPROPERTY\" typeName=\"string\" />"
        "  </ECClass>"
        "</ECSchema>";
    testFunc (testSchemaXml, false, "Properties only differing by case in a sub and base class.");

    testSchemaXml =
        "<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" >"
        "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"TestClass2\" >"
        "    <ECProperty propertyName=\"TestProperty\" typeName=\"string\" />"
        "  </ECClass>"
        "</ECSchema>";
    testFunc (testSchemaXml, true, "Properties differing only by case in two unrelated classes.");

    //***** Classes and properties only differing by case
    testSchemaXml =
        "<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
        "</ECSchema>";
    testFunc (testSchemaXml, false, "Class and properties only differing by case within a class.");

    //***** Property is of same type as class
    testSchemaXml =
        "<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" isStruct=\"true\" isDomainClass=\"true\">"
        "    <ECStructProperty propertyName=\"Prop1\" typeName=\"TestClass\" />"
        "  </ECClass>"
        "</ECSchema>";
    testFunc (testSchemaXml, false, "Property is of same type as class.");

    //***** Property is of subtype of class
    testSchemaXml =
        "<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"Base\" isStruct=\"true\" isDomainClass=\"true\" >"
        "    <ECStructProperty propertyName=\"Prop1\" typeName=\"Sub\" isStruct=\"true\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"Sub\" isStruct=\"true\">"
        "     <BaseClass>Base</BaseClass>"
        "    <ECProperty propertyName=\"Prop2\" typeName=\"string\" />"
        "  </ECClass>"
        "</ECSchema>";
    testFunc (testSchemaXml, false, "Property is of subtype of class.");

    //***** Property is array of class
    testSchemaXml =
        "<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"TestClass\" isStruct=\"true\" isDomainClass=\"true\">"
        "    <ECArrayProperty propertyName=\"Prop1\" typeName=\"TestClass\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
        "  </ECClass>"
        "</ECSchema>";
    testFunc (testSchemaXml, false, "Property is array of class.");

    //***** Property is array of subclass
    testSchemaXml =
        "<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"Base\" isStruct=\"true\" isDomainClass=\"true\">"
        "    <ECArrayProperty propertyName=\"Prop1\" typeName=\"Sub\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
        "  </ECClass>"
        "  <ECClass typeName=\"Sub\" isStruct=\"true\">"
        "     <BaseClass>Base</BaseClass>"
        "    <ECProperty propertyName=\"Prop2\" typeName=\"string\" />"
        "  </ECClass>"
        "</ECSchema>";
    testFunc (testSchemaXml, false, "Property is of array of subclass of class.");

    //***** Synthesis, casing errors and property is of same type as class errors
    testSchemaXml =
        "<ECSchema schemaName=\"InvalidSchema\" nameSpacePrefix=\"is\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
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
        "</ECSchema>";
    testFunc (testSchemaXml, false, "Case-sensitive class and prop names and property is of array of subclass of class.");
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  03/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbSchemaManager, ImportSchemaWithUnsupportedECRelationships)
    {
    ECDbTestProject::Initialize ();

    ECDb ecdb;
    auto stat = ECDbTestUtility::CreateECDb (ecdb, nullptr, L"importecschemawithunsupportedecrelationship.ecdb");
    ASSERT_TRUE (stat == BE_SQLITE_OK);

    Utf8CP testSchemaXml =
        "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "  <ECClass typeName=\"A\" >"
        "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"B\" >"
        "    <ECProperty propertyName=\"Id\" typeName=\"long\" />"
        "  </ECClass>"
        "  <ECClass typeName=\"Base\" >"
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
        "  <ECRelationshipClass typeName = \"RelUnsupported1\" isDomainClass = \"True\" strength = \"holding\" strengthDirection = \"forward\">"
        "    <BaseClass>Base</BaseClass>"
        "    <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "      <Class class = \"A\" />"
        "    </Source>"
        "    <Target cardinality = \"(0, N)\" polymorphic = \"True\">"
        "      <Class class = \"B\" />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "  <ECClass typeName=\"ClassUnsupported\" >"
        "    <BaseClass>RelBase</BaseClass>"
        "    <ECProperty propertyName=\"Name\" typeName=\"string\" />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName = \"RelUnsupported2\" isDomainClass = \"True\" strength = \"holding\" strengthDirection = \"forward\">"
        "    <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "      <Class class = \"A\" />"
        "    </Source>"
        "    <Target cardinality = \"(0, N)\" polymorphic = \"True\">"
        "      <Class class = \"RelBase\" />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "  <ECRelationshipClass typeName = \"RelUnsupported3\" isDomainClass = \"True\" strength = \"holding\" strengthDirection = \"forward\">"
        "    <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "      <Class class = \"A\" />"
        "    </Source>"
        "    <Target cardinality = \"(0, N)\" polymorphic = \"True\">"
        "      <Class class = \"Base\" />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "  <ECRelationshipClass typeName = \"RelSupported1\" isDomainClass = \"True\" strength = \"holding\" strengthDirection = \"forward\">"
        "    <Source cardinality = \"(0, 1)\" polymorphic = \"True\">"
        "      <Class class = \"A\" />"
        "    </Source>"
        "    <Target cardinality = \"(0, N)\" polymorphic = \"False\">"
        "      <Class class = \"Base\" />"
        "    </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>";

    auto schemaCache = ECDbTestUtility::ReadECSchemaFromString (testSchemaXml);
    ASSERT_TRUE (schemaCache != nullptr);

    BeTest::SetFailOnAssert (false);
    ASSERT_EQ (SUCCESS, ecdb.GetEC ().GetSchemaManager ().ImportECSchemas (*schemaCache));
    BeTest::SetFailOnAssert (true);

    //For each test class, bool flag indicates whether it is expected to be mapped or not
    std::vector<std::pair<Utf8String, bool>> mappingCheckItems = {
        { "A", true },
        { "B", true },
        { "Base", true },
        { "RelBase", true },
        { "RelUnsupported1", false },
        { "ClassUnsupported", false },
        { "RelUnsupported2", false },
        { "RelUnsupported3", false },
        { "RelSupported1", true } };

    for (auto const& pair : mappingCheckItems)
        {
        //check: prepare ECSQL against test class -> must fail if class was not mapped
        ECSqlStatement stmt;
        Utf8String ecsql ("SELECT NULL FROM ts.");
        ecsql.append (pair.first);
        bool expectedIsMapped = pair.second;

        BeTest::SetFailOnAssert (false);
        EXPECT_EQ (expectedIsMapped, stmt.Prepare (ecdb, ecsql.c_str ()) == ECSqlStatus::Success) << "Unexpected map status for class " << pair.first.c_str ();
        BeTest::SetFailOnAssert (true);
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
    ECClassP ecclass = NULL;
    ASSERT_EQ(SUCCESS, ecdbr.GetEC().GetSchemaManager().GetECClass(ecclass, "UserWorkBench", "areas"));
    ecclass = NULL;
    ASSERT_EQ(SUCCESS, ecdbr.GetEC().GetSchemaManager().GetECClass(ecclass, "UserWorkBench", "house_user"));
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
    ASSERT_EQ(SUCCESS, ecdb.GetEC().GetSchemaManager().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions())) << "could not import test ecschema.";
    ecdb.SaveChanges();

    ECClassP ecclass = NULL;
    ASSERT_EQ(SUCCESS, ecdb.GetEC().GetSchemaManager().GetECClass(ecclass, "BaseSchemaA", "Address")) << "couldn't get the ecclass";
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
    ASSERT_EQ(SUCCESS, ecdb.GetEC().GetSchemaManager().ImportECSchemas(*Schemacache, ECDbSchemaManager::ImportOptions()));
    ECSchemaCachePtr Schemacache1 = ECSchemaCache::Create();
    Schemacache1->AddSchema(*schemaPtr1);
    ASSERT_EQ(SUCCESS, ecdb.GetEC().GetSchemaManager().ImportECSchemas(*Schemacache1, ECDbSchemaManager::ImportOptions()));
    ecdb.SaveChanges();

    ECClassP ecclass = NULL;
    ecdb.GetEC().GetSchemaManager().GetECClass(ecclass, "BaseSchemaA", "Address");
    EXPECT_TRUE(ecclass != NULL) << "Class with the specified name doesn't exist :- ecclass is empty";
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

    Bentley::BeTest::SetFailOnAssert(false);
    Savepoint s(ecdbr, "test");
    ASSERT_EQ(ERROR, ecdbr.GetEC().GetSchemaManager().ImportECSchemas(*schemaCache, ECDbSchemaManager::ImportOptions(false, true))) << "couldn't update the existing schema";
    Bentley::BeTest::SetFailOnAssert(true);
    s.Cancel();

    ECSchemaP schemap = NULL;
    ASSERT_EQ(SUCCESS, ecdbr.GetEC().GetSchemaManager().GetECSchema(schemap, "TestSchema", true));
    wprintf(L"%s\n", schemap->GetName().c_str());
    ASSERT_EQ(4, schemap->GetClassCount()) << "Class count doesn't match the original number of classes";

    ECClassP ecclass = NULL;
    EXPECT_EQ(SUCCESS, ecdbr.GetEC().GetSchemaManager().GetECClass(ecclass, "TestSchema", "DerivedTestClass"));
    ecclass = NULL;
    ASSERT_EQ(ERROR, ecdbr.GetEC().GetSchemaManager().GetECClass(ecclass, "TestSchema", "DerivedClass"));
    ecclass = NULL;
    ASSERT_EQ(SUCCESS, ecdbr.GetEC().GetSchemaManager().GetECClass(ecclass, "TestSchema", "FileInfo"));
    EXPECT_EQ(3, ecclass->GetPropertyCount());
    ECPropertyIterable iterator = ecclass->GetProperties();
    int i = 0;
    FOR_EACH(ECPropertyP ecpropertyp, iterator)
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

    ASSERT_EQ(SUCCESS, testEcdb.GetEC().GetSchemaManager().ImportECSchemas(*schemaCache, ECDbSchemaManager::ImportOptions())) << "Couldn't import the first version of Schema";
    ECClassP ecClass = NULL;
    EXPECT_EQ(ERROR, testEcdb.GetEC().GetSchemaManager().GetECClass(ecClass, "RSComponents", "TestClass"));
    EXPECT_TRUE(ecClass == NULL);
    ASSERT_EQ(SUCCESS, testEcdb.GetEC().GetSchemaManager().ImportECSchemas(*schemaCache1, ECDbSchemaManager::ImportOptions(false, true))) << "Couldn't import or update the existing schema";
    testEcdb.SaveChanges();

    //getting the updated schema
    ECSchemaP schemap = NULL;
    ASSERT_EQ(SUCCESS, testEcdb.GetEC().GetSchemaManager().GetECSchema(schemap, "RSComponents", true));
    ASSERT_EQ(16, schemap->GetClassCount()) << "Class count doesn't match the original number of classes";

    ecClass = NULL;
    ASSERT_EQ(SUCCESS, testEcdb.GetEC().GetSchemaManager().GetECClass(ecClass, "RSComponents", "CAMERA"));
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
    FOR_EACH(ECPropertyP ecpropertyp, iterator)
    {
        ASSERT_TRUE(ecpropertyp != NULL);
        ecpropertyp = NULL;
        i++;
    }
    ASSERT_EQ(8, i) << "Properties returned by Iterator doesn't match the Original Number";

    ecClass = NULL;
    ASSERT_EQ(SUCCESS, testEcdb.GetEC().GetSchemaManager().GetECClass(ecClass, "RSComponents", "SONOR"));
    ASSERT_TRUE(ecClass != NULL);
    ecClass = NULL;
    ASSERT_EQ(SUCCESS, testEcdb.GetEC().GetSchemaManager().GetECClass(ecClass, "RSComponents", "SC"));
    ASSERT_TRUE(ecClass != NULL);
    EXPECT_EQ(1, ecClass->GetPropertyCount()) << "Property count not equal";
}

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  09/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbSchemaManager, SchemaCacheManager)
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

    ASSERT_EQ(SUCCESS, ecdb.GetEC().GetSchemaManager().ImportECSchemas(*Schemacache, ECDbSchemaManager::ImportOptions())) << "could not import test ecschema.";
    ecdb.SaveChanges();

    ASSERT_EQ(ERROR, testecdb.GetEC().GetSchemaManager().ImportECSchemas(*Schemacache, ECDbSchemaManager::ImportOptions())) << "could not import test ecschema in the 2nd ecdb file.";
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

    ASSERT_EQ(SUCCESS, testecdb.GetEC().GetSchemaManager().ImportECSchemas(*schemaCache, ECDbSchemaManager::ImportOptions(false, false))) << "couldn't import the schema";
    ECSchemaP SchoolSupplSchema = nullptr;
    auto status = testecdb.GetEC().GetSchemaManager().GetECSchema(SchoolSupplSchema, "SchoolSchema", true);
    ASSERT_EQ(SUCCESS, status);

    ECClassP ecclassCourseTitle = SchoolSupplSchema->GetClassP(L"CourseTitle");
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

    ASSERT_EQ(SUCCESS, testecdb.GetEC().GetSchemaManager().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
    ECSchemaP SchoolSupplSchema = nullptr;
    auto status = testecdb.GetEC().GetSchemaManager().GetECSchema(SchoolSupplSchema, "SchoolSchema", true);
    ASSERT_EQ(SUCCESS, status);

    ECClassP ecclassCourse = SchoolSupplSchema->GetClassP(L"Course");
    ASSERT_TRUE(ecclassCourse != NULL);
    ECClassP ecclassCourseTitle = SchoolSupplSchema->GetClassP(L"CourseTitle");
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

    ECClassP relationshipClass = SchoolSupplSchema->GetClassP(L"SchoolDepartmentRelation");
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


    ASSERT_EQ(SUCCESS, testecdb.GetEC().GetSchemaManager().ImportECSchemas(*schemaCache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
    ECSchemaP SchoolSupplSchema = nullptr;
    auto status = testecdb.GetEC().GetSchemaManager().GetECSchema(SchoolSupplSchema, "SchoolSchema", true);
    ASSERT_EQ(SUCCESS, status);
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

     ASSERT_EQ(SUCCESS, testecdb.GetEC().GetSchemaManager().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
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

    ASSERT_EQ(SUCCESS, testecdb.GetEC().GetSchemaManager().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
    ECSchemaP SchoolSupplSchema = nullptr;
    auto status = testecdb.GetEC().GetSchemaManager().GetECSchema(SchoolSupplSchema, "SchoolSchema", true);
    ASSERT_EQ(SUCCESS, status);
}

TEST(ECDbSchemaManager, TestGetClassResolver)
    {
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create("ecschemamanagertest.ecdb", L"TestSchema.01.00.ecschema.xml", true);
    ECClassP ecClass = NULL;
    EXPECT_EQ(SUCCESS, ecdbr.GetEC().GetSchemaManager().GetECClass(ecClass, "TestSchema", "DerivedTestClass"));
    EXPECT_EQ(SUCCESS, ecdbr.GetEC().GetSchemaManager().GetECClass(ecClass, "TS", "DerivedTestClass", ECDbSchemaManager::ResolveSchema::BySchemaNamespacePrefix));

    EXPECT_EQ(SUCCESS, ecdbr.GetEC().GetSchemaManager().GetECClass(ecClass, "TestSchema", "DerivedTestClass", ECDbSchemaManager::ResolveSchema::AutoDetect));
    EXPECT_EQ(SUCCESS, ecdbr.GetEC().GetSchemaManager().GetECClass(ecClass, "TS", "DerivedTestClass", ECDbSchemaManager::ResolveSchema::AutoDetect));

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

    ASSERT_EQ(SUCCESS, testecdb.GetEC().GetSchemaManager().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
    ECSchemaP basicSupplSchema = nullptr;
    auto status = testecdb.GetEC().GetSchemaManager().GetECSchema(basicSupplSchema, "BasicSchema", true);
    ASSERT_EQ(SUCCESS, status);

    ECClassP ecclassBase = basicSupplSchema->GetClassP(L"Base");
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

    ASSERT_EQ(SUCCESS, testecdb.GetEC().GetSchemaManager().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
    ECSchemaP basicSupplSchema = nullptr;
    auto status = testecdb.GetEC().GetSchemaManager().GetECSchema(basicSupplSchema, "BasicSchema", true);
    ASSERT_EQ(SUCCESS, status);

    ECClassP ecclassBase = basicSupplSchema->GetClassP(L"Base");
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

    ASSERT_EQ(SUCCESS, testecdb.GetEC().GetSchemaManager().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
    ECSchemaP basicSupplSchema = nullptr;
    auto status = testecdb.GetEC().GetSchemaManager().GetECSchema(basicSupplSchema, "BasicSchema", true);
    ASSERT_EQ(SUCCESS, status);

    ECClassP ecclassBase = basicSupplSchema->GetClassP(L"Base");
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

    ASSERT_EQ(SUCCESS, testecdb.GetEC().GetSchemaManager().ImportECSchemas(*schemacache, ECDbSchemaManager::ImportOptions(true, false))) << "couldn't import the schema";
    ECSchemaP basicSupplSchema = nullptr;
    auto status = testecdb.GetEC().GetSchemaManager().GetECSchema(basicSupplSchema, "BasicSchema", true);
    ASSERT_EQ(SUCCESS, status);

    ECClassP ecclassBase = basicSupplSchema->GetClassP(L"Base");
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
    EXPECT_EQ(SUCCESS, db.GetEC().GetSchemaManager().ImportECSchemas(*schemaCache, ECDbSchemaManager::ImportOptions(true, true)));
    }


END_ECDBUNITTESTS_NAMESPACE
