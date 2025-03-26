/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <set>
#include <ECObjects/SchemaComparer.h>
#include <Bentley/BeDirectoryIterator.h>
#include <optional>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

#define ENABLE_CONSOLE_LOGGING 1
//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct SchemaGraphTestFixture : public ECDbTestFixture
    {
public:
    void SetUp() override
    {
        ECDbTestFixture::SetUp();
        #if ENABLE_CONSOLE_LOGGING
            NativeLogging::Logging::SetLogger(&NativeLogging::ConsoleLogger::GetLogger());
            NativeLogging::ConsoleLogger::GetLogger().SetSeverity("ECDb", BentleyApi::NativeLogging::LOG_TRACE);
            NativeLogging::ConsoleLogger::GetLogger().SetSeverity("ECObjectsNative", BentleyApi::NativeLogging::LOG_TRACE);
        #endif
    }
    std::optional<SchemaKey> ReadSchemaKeyFromDb(Utf8StringCR schemaName)
        {
        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(m_ecdb, SqlPrintfString("SELECT VersionMajor,VersionWrite,VersionMinor FROM meta.ECSchemaDef WHERE Name='%s'", schemaName.c_str())))
            {
            return std::nullopt;
            }
    
        if (stmt.Step() != BE_SQLITE_ROW)
            return std::nullopt;

        return SchemaKey(schemaName.c_str(), stmt.GetValueInt(0), stmt.GetValueInt(1), stmt.GetValueInt(2));
        }

    void AssertECDbSchemaVersion(SchemaKeyCR expectedKey)
        {
        auto foundKey = ReadSchemaKeyFromDb(expectedKey.GetName());
        ASSERT_TRUE(foundKey.has_value()) << "Failed to read schema key from the database for schema: " << expectedKey.GetName();
        ASSERT_TRUE(expectedKey.Matches(foundKey.value(), SchemaMatchType::Exact)) << "Schema key mismatch. Expected: " << expectedKey.GetFullSchemaName().c_str() << ", Found: " << foundKey.value().GetFullSchemaName().c_str();
        }
    
    ECSchemaP GetReferencedSchema(Utf8StringCR name, ECSchemaCR schema)
        {
        for (auto& pair : schema.GetReferencedSchemas())
            {
            if(pair.first.GetName() == name)
                return pair.second.get();
            }

        return nullptr;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaGraphTestFixture, MissingImportSchemaInTheMiddle)
    {
    // Test produces a real bug that we had with RoadRailPhysical and transformer.
    // The db contains an older version of BisCore and LinearReferecing, and the
    // incoming schemas are an updated BisCore plus RoadRailPhysical, but not LinearReferencing.
    // This caused an unclean schema graph with 2 BisCore versions in it.

    // before the fix, this test logs the following relevant messages:
    // WARNING  ECObjectsNative      ECSchemaCache: Adding schema 'LinearReferencing.01.00.00' which references schema 'CoreCustomAttributes.01.00.04'. However, a different in-memory instance of this referenced schema already exists in the cache. This may indicate an issue with the schema graph.
    // WARNING  ECObjectsNative      ECSchemaCache: Adding schema 'BisCore.01.00.00' to the cache while schema 'BisCore.01.00.01' already exists. Typically, only one version of a schema with the same name is expected. This may indicate an issue with the schema graph.
    // ERROR    ECObjectsNative      Cannot add 'BisCore:ISubModeledElement' as a base class to 'RoadRailPhysical:Corridor' because the base class is a mixin and the derived class does not derive from 'BisCore:Element' which is the applies to constraint

    SchemaItem initialBisCoreXml(
    R"schema(<?xml version='1.0' encoding='utf-8' ?>
    <ECSchema schemaName="BisCore" alias="bis" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
        <ECEntityClass typeName="Element" />
        <ECEntityClass typeName="ISubModeledElement" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.01.00.03">
                    <AppliesToEntityClass>Element</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
        </ECEntityClass>
    </ECSchema>)schema");

    SchemaItem initialLinearReferencingXml(
    R"schema(<?xml version='1.0' encoding='utf-8' ?>
    <ECSchema schemaName="LinearReferencing" alias="lr" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
        <ECEntityClass typeName="LinearPhysicalElement">
            <BaseClass>bis:Element</BaseClass>
        </ECEntityClass>
        <ECEntityClass typeName="ILinearElementSource" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.01.00.03">
                    <AppliesToEntityClass>bis:Element</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
        </ECEntityClass>
    </ECSchema>)schema");

    std::vector<SchemaItem> initialSchemas { initialBisCoreXml, initialLinearReferencingXml };
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    ASSERT_EQ(BentleyStatus::SUCCESS, GetHelper().ImportSchemas(initialSchemas));

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    StringSchemaLocater locater;
    SchemaKey bisCoreKey ("BisCore", 1, 0, 1);
    
    locater.AddSchemaString(bisCoreKey, 
    R"schema(<?xml version='1.0' encoding='utf-8' ?>
    <ECSchema schemaName="BisCore" alias="bis" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
        <ECEntityClass typeName="Element" />
        <ECEntityClass typeName="ISubModeledElement" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.01.00.03">
                    <AppliesToEntityClass>Element</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
        </ECEntityClass>
    </ECSchema>)schema");

    SchemaKey roadRailPhysicalKey ("RoadRailPhysical", 1, 0, 0);
    locater.AddSchemaString(roadRailPhysicalKey,
    R"schema(<?xml version='1.0' encoding='utf-8' ?>
    <ECSchema schemaName="RoadRailPhysical" alias="rrphys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
        <ECSchemaReference name = "LinearReferencing" version = "01.00.00" alias = "lr"/>
        <ECEntityClass typeName="Corridor">
            <BaseClass>lr:LinearPhysicalElement</BaseClass>
            <BaseClass>bis:ISubModeledElement</BaseClass>
            <BaseClass>lr:ILinearElementSource</BaseClass>
        </ECEntityClass>
    </ECSchema>)schema");
    context->AddSchemaLocater(locater);
    context->SetFinalSchemaLocater(m_ecdb.GetSchemaLocater());

    // Load schemas through the context
    SchemaKey bisCoreRequestedKey("BisCore", 1, 0, 0);
    ECSchemaPtr bisCore = context->LocateSchema(bisCoreRequestedKey, SchemaMatchType::LatestReadCompatible);
    ASSERT_TRUE(bisCore.IsValid());
    SchemaKey roadRailPhysicalRequestedKey("RoadRailPhysical", 1, 0, 0);
    ECSchemaPtr roadRailPhysical = context->LocateSchema(roadRailPhysicalRequestedKey, SchemaMatchType::LatestReadCompatible);

    // Verify the schema graph
    SchemaKey linearReferencingKey("LinearReferencing", 1, 0, 0);
    ASSERT_TRUE(roadRailPhysical.IsValid());
    ASSERT_TRUE(roadRailPhysical->GetSchemaKey().Matches(roadRailPhysicalKey, SchemaMatchType::Exact));
    ASSERT_TRUE(bisCore->GetSchemaKey().Matches(bisCoreKey, SchemaMatchType::Exact));
    ASSERT_TRUE(roadRailPhysical->IsSchemaReferenced(*roadRailPhysical, *bisCore));
    ECSchemaP linearReferencing = GetReferencedSchema("LinearReferencing", *roadRailPhysical);
    ASSERT_TRUE(linearReferencing != nullptr);
    ASSERT_TRUE(linearReferencing->GetSchemaKey().Matches(linearReferencingKey, SchemaMatchType::Exact));
    ECSchemaP bisCoreOfLinearReferencing = GetReferencedSchema("BisCore", *linearReferencing);
    ASSERT_TRUE(bisCoreOfLinearReferencing != nullptr);
    ASSERT_TRUE(bisCoreOfLinearReferencing->GetSchemaKey().Matches(bisCoreKey, SchemaMatchType::Exact)); // This was wrong before the fix, returned 1.0.0, should be 1.0.1
    
    // Import the schemas
    SchemaImportResult result = m_ecdb.Schemas().ImportSchemas({ bisCore.get(), roadRailPhysical.get() }, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade);
    ASSERT_EQ(SchemaImportResult::OK, result);
    m_ecdb.SaveChanges();
    AssertECDbSchemaVersion(bisCoreKey);
    AssertECDbSchemaVersion(linearReferencingKey);
    AssertECDbSchemaVersion(roadRailPhysicalKey);
    }

TEST_F(SchemaGraphTestFixture, CircularSchemaReference)
    {
    SchemaKey fooKey("Foo", 1, 0, 0);
    Utf8String fooXml(
    R"schema(<?xml version='1.0' encoding='utf-8' ?>
    <ECSchema schemaName="Foo" alias="foo" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="Bar" version="01.00.00" alias="bar"/>
        <ECEntityClass typeName="FooEntity">
            <BaseClass>bar:BarEntity</BaseClass>
        </ECEntityClass>
    </ECSchema>)schema");

    SchemaKey barKey("Bar", 1, 0, 0);
    Utf8String barXml(
    R"schema(<?xml version='1.0' encoding='utf-8' ?>
    <ECSchema schemaName="Bar" alias="bar" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="Foo" version="01.00.00" alias="foo"/>
        <ECEntityClass typeName="BarEntity">
            <BaseClass>foo:FooEntity</BaseClass>
        </ECEntityClass>
    </ECSchema>)schema");

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    StringSchemaLocater locater;
    locater.AddSchemaString(fooKey, fooXml);
    locater.AddSchemaString(barKey, barXml);
    context->AddSchemaLocater(locater);

    // Currently we log an error and refuse to load. That's fine, this test is to make sure we fail gracefully.
    ECSchemaPtr foo = context->LocateSchema(fooKey, SchemaMatchType::LatestReadCompatible);
    ASSERT_FALSE(foo.IsValid());
    ECSchemaPtr bar = context->LocateSchema(barKey, SchemaMatchType::LatestReadCompatible);
    ASSERT_FALSE(bar.IsValid());
    }

TEST_F(SchemaGraphTestFixture, CircularEmptySchemaReference)
    {
    SchemaKey fooKey("Foo", 1, 0, 0);
    Utf8String fooXml(
    R"schema(<?xml version='1.0' encoding='utf-8' ?>
    <ECSchema schemaName="Foo" alias="foo" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="Bar" version="01.00.00" alias="bar" />
    </ECSchema>)schema");

    SchemaKey barKey("Bar", 1, 0, 0);
    Utf8String barXml(
    R"schema(<?xml version='1.0' encoding='utf-8' ?>
    <ECSchema schemaName="Bar" alias="bar" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="Foo" version="01.00.00" alias="foo" />
    </ECSchema>)schema");

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    StringSchemaLocater locater;
    locater.AddSchemaString(fooKey, fooXml);
    locater.AddSchemaString(barKey, barXml);
    context->AddSchemaLocater(locater);

    // Currently we log an error and refuse to load. That's fine, this test is to make sure we fail gracefully.
    TestIssueListener issues;
    context->Issues().AddListener(issues);

    ECSchemaPtr foo = context->LocateSchema(fooKey, SchemaMatchType::LatestReadCompatible);
    ASSERT_FALSE(foo.IsValid());
    ECSchemaPtr bar = context->LocateSchema(barKey, SchemaMatchType::LatestReadCompatible);
    ASSERT_FALSE(bar.IsValid());

    bvector<Utf8String> expectedIssues {
        "Failed to read Schema. The attempt to load from XML ended up in a circular reference. Schemaname (if available): Foo.01.00.00",
        "Failed to read Schema. The attempt to load from XML ended up in a circular reference. Schemaname (if available): Bar.01.00.00",
    };
    issues.CompareIssues(expectedIssues);
    }


    TEST_F(SchemaGraphTestFixture, CircularEmptySchemaReferenceInDb)
    {
    // Provokes the circular schema reference scenario inside ecdb
    SetupECDbForCurrentTest();
    SchemaKey fooKey("Foo", 1, 0, 0);
    Utf8String fooXml(
    R"schema(<?xml version='1.0' encoding='utf-8' ?>
    <ECSchema schemaName="Foo" alias="foo" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="Bar" version="01.00.00" alias="bar" />
    </ECSchema>)schema");

    SchemaKey barKey("Bar", 1, 0, 0);
    Utf8String barXml(
    R"schema(<?xml version='1.0' encoding='utf-8' ?>
    <ECSchema schemaName="Bar" alias="bar" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
    </ECSchema>)schema");

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    StringSchemaLocater locater;
    locater.AddSchemaString(fooKey, fooXml);
    locater.AddSchemaString(barKey, barXml);
    context->AddSchemaLocater(locater);

    ECSchemaPtr foo = context->LocateSchema(fooKey, SchemaMatchType::LatestReadCompatible);
    ASSERT_TRUE(foo.IsValid());
    ECSchemaPtr bar = context->LocateSchema(barKey, SchemaMatchType::LatestReadCompatible);
    ASSERT_TRUE(bar.IsValid());

    SchemaImportResult result = m_ecdb.Schemas().ImportSchemas({ foo.get(), bar.get() });
    ASSERT_EQ(SchemaImportResult::OK, result);
    m_ecdb.SaveChanges();
    AssertECDbSchemaVersion(fooKey);
    AssertECDbSchemaVersion(barKey);
    
    ECSchemaId  fooId, barId;
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT Id FROM ec_Schema WHERE Name='%s'", "Foo")));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    fooId = stmt.GetValueId<ECSchemaId>(0);
    }
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, SqlPrintfString("SELECT Id FROM ec_Schema WHERE Name='%s'", "Bar")));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    barId = stmt.GetValueId<ECSchemaId>(0);
    }
    { // Add a reference to foo from bar
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "INSERT INTO ec_SchemaReference(SchemaId, ReferencedSchemaId) VALUES(?, ?)"));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindId(1, barId));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindId(2, fooId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    // Now we have a circular reference in the db, try and load the schemas
    ECSchemaReadContextPtr context2 = ECSchemaReadContext::CreateContext();
    context2->SetFinalSchemaLocater(m_ecdb.GetSchemaLocater());
    ECSchemaPtr foo2 = context2->LocateSchema(fooKey, SchemaMatchType::LatestReadCompatible);
    ASSERT_FALSE(foo2.IsValid());
    ECSchemaPtr bar2 = context2->LocateSchema(barKey, SchemaMatchType::LatestReadCompatible);
    ASSERT_FALSE(bar2.IsValid()); // TODO: THis actually returns true at the moment, schema is incompletely loaded.
    }

#define ENABLE_IMPORT_SCHEMAS_FROM_FILES_TEST 1

#if ENABLE_IMPORT_SCHEMAS_FROM_FILES_TEST
TEST_F(SchemaGraphTestFixture, ImportSchemasFromFiles)
    {
    // Test is intended to be commented by default. It can be used to analyze schema import calls
    // It simulates the same procedure that happens during JsInterop.ImportSchemas().
    // The other tests use SchemaStringLocater, but behavior with files is slightly different as
    // the files' directory gets added to search paths.
    BeFileName fileName("/home/rob/test/TargetiModel.bim");
    OpenECDb(fileName);

    std::vector<Utf8String> fileNames = {
      "/home/rob/test/ImportSchemas/AdskCivil3dSchema.ecschema.xml",
      "/home/rob/test/ImportSchemas/BisCore.ecschema.xml",
      "/home/rob/test/ImportSchemas/DocumentMetadata.ecschema.xml",
      "/home/rob/test/ImportSchemas/DwgAttributeDefinitions_6039__x0020__Perry__x0020__Worth__x0020__Bldg__x0020__footprint__x0020____x002D____x0020__Copy.ecschema.xml",
      "/home/rob/test/ImportSchemas/DwgSourceInfo.ecschema.xml",
      "/home/rob/test/ImportSchemas/ProjectwiseDynamic.ecschema.xml",
      "/home/rob/test/ImportSchemas/QuantityTakeoffsAspects.ecschema.xml",
      "/home/rob/test/ImportSchemas/RailPhysical.ecschema.xml",
      "/home/rob/test/ImportSchemas/Raster.ecschema.xml",
      "/home/rob/test/ImportSchemas/RoadPhysical.ecschema.xml",
      "/home/rob/test/ImportSchemas/RoadRailPhysical.ecschema.xml"
    };

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false /*=acceptLegacyImperfectLatestCompatibleMatch*/, true /*=includeFilesWithNoVerExt*/);
    schemaContext->SetFinalSchemaLocater(m_ecdb.GetSchemaLocater());
    // In the node addon, we also use the platform assets directory to find the schema files.
    // iModelJsNodeAddon/JsInterop.cpp AddFallbackSchemaLocaters()

    bvector<ECSchemaCP> schemas;
    for (Utf8String schemaSource : fileNames)
        {
        ECSchemaPtr schema;
        SchemaReadStatus schemaStatus;
        BeFileName schemaFile(schemaSource.c_str(), BentleyCharEncoding::Utf8);
        ASSERT_TRUE(schemaFile.DoesPathExist());

        schema = ECSchema::LocateSchema(schemaSource.c_str(), *schemaContext, SchemaMatchType::Exact, &schemaStatus);

        if (SchemaReadStatus::DuplicateSchema == schemaStatus)
            continue;

        ASSERT_EQ(SchemaReadStatus::Success, schemaStatus);

        schemas.push_back(schema.get());
        }

    SchemaImportResult result = m_ecdb.Schemas().ImportSchemas(schemas, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade);
    ASSERT_EQ(SchemaImportResult::OK, result);
    m_ecdb.SaveChanges();
    }
#endif
END_ECDBUNITTESTS_NAMESPACE