/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/IModelCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <DgnPlatform/DgnCoreAPI.h>
#include <DgnPlatform/FunctionalDomain.h>
#include "Profiles.h"
#include "TestIModelCreators.h"
#include "TestDb.h"
#include "TestDomain.h"

USING_NAMESPACE_BENTLEY_EC

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct IModelCompatibilityTestFixture : CompatibilityTestFixture
    {
    protected:
        ScopedDgnHost m_host;
        void SetUp() override 
            {
            CompatibilityTestFixture::SetUp();
            ASSERT_EQ(SUCCESS, TestIModelCreation::Run()); 
            }
    };

//---------------------------------------------------------------------------------------
// Runs basic tests on all available test files. This is a basic test to cover tests and test files
// which are added in the future, and to which existing test runners cannot be adjusted to.
// @bsimethod                                  Krischan.Eberle                      07/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, BasicTestsOnAllPulledFiles)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfAllPulledTestFiles())
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            // Run SELECT statements against all classes
            for (ECSchemaCP schema : testDb.GetDb().Schemas().GetSchemas())
                {
                if (schema->GetName().Equals("ECDbSystem"))
                    continue; //doesn't have mapped classes

                for (ECClassCP cl : schema->GetClasses())
                    {
                    if (!cl->IsEntityClass() && !cl->IsRelationshipClass())
                        continue;
                    
                    Utf8String ecsql("SELECT ECInstanceId,ECClassId");
                    for (ECPropertyCP prop : cl->GetProperties())
                        {
                        ecsql.append(",[").append(prop->GetName()).append("]");
                        }
                    ecsql.append(" FROM ").append(cl->GetECSqlName());

                    ECSqlStatement stmt;
                    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), ecsql.c_str())) << ecsql << " | " << testDb.GetDescription();
                    const DbResult stepStat = stmt.Step();
                    ASSERT_TRUE(BE_SQLITE_DONE == stepStat || BE_SQLITE_ROW == stepStat) << ecsql << " | " << testDb.GetDescription();
                    }
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, BuiltinSchemaVersions)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();

            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            switch (testDb.GetAge())
                {
                    case ProfileState::Age::Older:
                    case ProfileState::Age::UpToDate:
                    {
                    EXPECT_EQ(8, testDb.GetSchemaCount()) << testDb.GetDescription();

                    //DgnDb built-in schema versions
                    EXPECT_EQ(SchemaVersion(1, 0, 1), testDb.GetSchemaVersion("BisCore")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("BisCore")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":164, "enumcount": 2})js"), testDb.GetSchemaItemCounts("BisCore")) << testDb.GetDescription();

                    EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("Generic")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("Generic")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":17})js"), testDb.GetSchemaItemCounts("Generic")) << testDb.GetDescription();

                    EXPECT_EQ(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), testDb.GetSchemaItemCounts("ECDbFileInfo")) << testDb.GetDescription();

                    EXPECT_EQ(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbMap")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), testDb.GetSchemaItemCounts("ECDbMap")) << testDb.GetDescription();

                    EXPECT_EQ(SchemaVersion(4, 0, 0), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbMeta")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":24, "enumcount": 8})js"), testDb.GetSchemaItemCounts("ECDbMeta")) << testDb.GetDescription();

                    EXPECT_EQ(SchemaVersion(5, 0, 0), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbSystem")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), testDb.GetSchemaItemCounts("ECDbSystem")) << testDb.GetDescription();

                    EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("ECDbSchemaPolicies")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbSchemaPolicies")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":3})js"), testDb.GetSchemaItemCounts("ECDbSchemaPolicies")) << testDb.GetDescription();

                    //standard schema versions
                    EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("CoreCustomAttributes")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), testDb.GetSchemaItemCounts("CoreCustomAttributes")) << testDb.GetDescription();
                    break;
                    }
                    case ProfileState::Age::Newer:
                    {
                    EXPECT_EQ(8, testDb.GetSchemaCount()) << testDb.GetDescription();

                    //DgnDb built-in schema versions
                    EXPECT_LE(SchemaVersion(1, 0, 1), testDb.GetSchemaVersion("BisCore")) << testDb.GetDescription();
                    if (TestDb::VersionSupportsFeature(testDb.GetECDbInitialVersion(), ECDbFeature::PersistedECVersions))
                        EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("BisCore")) << testDb.GetDescription();
                    else
                        EXPECT_LE(BeVersion(), testDb.GetOriginalECXmlVersion("BisCore")) << testDb.GetDescription();


                    EXPECT_LE(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("Generic")) << testDb.GetDescription();
                    if (TestDb::VersionSupportsFeature(testDb.GetECDbInitialVersion(), ECDbFeature::PersistedECVersions))
                        EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("Generic")) << testDb.GetDescription();
                    else
                        EXPECT_LE(BeVersion(), testDb.GetOriginalECXmlVersion("Generic")) << testDb.GetDescription();


                    //ECDb built-in schema versions
                    //ECDbFileInfo version was incremented in next profile, so must be higher in newer file
                    EXPECT_LT(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << testDb.GetDescription();

                    EXPECT_LE(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbMap")) << testDb.GetDescription();

                    //ECDbMeta version was incremented in next profile, so must be higher in newer file
                    EXPECT_LT(SchemaVersion(4, 0, 0), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbMeta")) << testDb.GetDescription();

                    //ECDbSystem version was incremented in next profile, so must be higher in newer file
                    EXPECT_LT(SchemaVersion(5, 0, 0), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbSystem")) << testDb.GetDescription();

                    EXPECT_LE(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("ECDbSchemaPolicies")) << testDb.GetDescription();
                    if (TestDb::VersionSupportsFeature(testDb.GetECDbInitialVersion(), ECDbFeature::PersistedECVersions))
                        EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbSchemaPolicies")) << testDb.GetDescription();
                    else
                        EXPECT_LE(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbSchemaPolicies")) << testDb.GetDescription();

                    //standard schema versions
                    EXPECT_LE(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("CoreCustomAttributes")) << testDb.GetDescription();

                    break;
                    }

                    default:
                        FAIL() << "Unhandled ProfileState::Age enum value | " << testDb.GetDescription();
                        break;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC31Enums)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31ENUMS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();

            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            testDb.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
            {{ECValue("Unspecified"), nullptr},
            {ECValue("Utc"), nullptr},
            {ECValue("Local"), nullptr}});

            testDb.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
            {{ECValue(0), "None"},
            {ECValue(1), "Abstract"},
            {ECValue(2), "Sealed"}});

            testDb.AssertEnum("TestSchema", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
            {{ECValue(0), nullptr},
            {ECValue(1), nullptr},
            {ECValue(2), nullptr}});

            testDb.AssertEnum("TestSchema", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
            {{ECValue("On"), "Turned On"},
            {ECValue("Off"), "Turned Off"}});
            }
        }
    }
   
//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      07/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, UpgradingEC31EnumsToEC32AfterProfileUpgrade)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC32ENUMS_PROFILEUPGRADED))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //regardless of whether the actual test file is 3.1 and not upgraded yet or it is newer and upgraded,
            //the upgrade does not affect the EC31 enum API. So the test code is generic
            testDb.AssertEnum("TestSchema", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
            {{ECValue(0), nullptr},
            {ECValue(1), nullptr},
            {ECValue(2), nullptr}});

            testDb.AssertEnum("TestSchema", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
            {{ECValue("On"), "Turned On"},
            {ECValue("Off"), "Turned Off"}});
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC32Enums)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC32ENUMS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            testDb.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
            {{ECValue("Unspecified"), nullptr},
            {ECValue("Utc"), nullptr},
            {ECValue("Local"), nullptr}});

            testDb.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
            {{ECValue(0), "None"},
            {ECValue(1), "Abstract"},
            {ECValue(2), "Sealed"}});

            testDb.AssertEnum("TestSchema", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
            {{ECValue(0), nullptr},
            {ECValue(1), nullptr},
            {ECValue(2), nullptr}});

            testDb.AssertEnum("TestSchema", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
            {{ECValue("On"), "Turned On"},
            {ECValue("Off"), "Turned Off"}});
            }
        }
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC31KindOfQuantities)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31KOQS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();

            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            switch (testDb.GetAge())
                {
                    case ProfileState::Age::Older:
                    case ProfileState::Age::UpToDate:
                        testDb.AssertKindOfQuantity("TestSchema", "ANGLE", "Angle", nullptr, "RAD(DefaultReal)", JsonValue(R"json(["ARC_DEG(Real2U)", "ARC_DEG(AngleDMS)"])json"), 0.0001);
                        testDb.AssertKindOfQuantity("TestSchema", "POWER", "Power", nullptr, "W(DefaultReal)", JsonValue(R"json(["W(Real4U)", "KW(Real4U)", "MEGAW(Real4U)", "BTU/HR(Real4U)", "KILOBTU/HR(Real4U)", "HP(Real4U)"])json"), 0.001);
                        testDb.AssertKindOfQuantity("TestSchema", "LIQUID_VOLUME", "Liquid Volume", nullptr, "CUB.M(DefaultReal)", JsonValue(R"json(["LITRE(Real4U)", "GALLON(Real4U)"])json"), 0.0001);
                        //A bug in bim02dev persisted a format along with the unit. This will be obsolete once EC32 is available
                        testDb.AssertKindOfQuantity("TestSchema", "TestKoq_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.4);
                        testDb.AssertKindOfQuantity("TestSchema", "TestKoq_PersUnitWithFormat_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.5);
                        testDb.AssertKindOfQuantity("TestSchema", "TestKoq_PersUnitWithFormatWithUnit_NoPresUnit", nullptr, nullptr, "FT(AmerFI8)", JsonValue(), 0.6);

                        testDb.AssertKindOfQuantity("TestSchema", "TestKoq_M_Mfi8", nullptr, nullptr, "M(DefaultReal)", JsonValue(R"json(["M(AmerFI8)"])json"), 0.7);
                        testDb.AssertKindOfQuantity("TestSchema", "TestKoq_Mfi8", nullptr, nullptr, "M(AmerFI8)", JsonValue(), 0.8);
                        testDb.AssertKindOfQuantity("TestSchema", "TestKoq_SQFTfi8", nullptr, nullptr, "SQ.FT(AmerFI8)", JsonValue(), 0.9);
                        testDb.AssertKindOfQuantity("TestSchema", "TestKoq_SQFTfi8_SQFTreal4u", nullptr, nullptr, "SQ.FT(AmerFI8)", JsonValue(R"json(["SQ.FT(Real4U)"])json"), 1.0);

                        break;

                    case ProfileState::Age::Newer:
                    {
                    testDb.AssertKindOfQuantity("TestSchema", "ANGLE", "Angle", nullptr, "RAD(DefaultReal)", JsonValue(R"json(["ARC_DEG(Real2U)", "ARC_DEG(AngleDMS)"])json"), 0.0001);
                    testDb.AssertKindOfQuantity("TestSchema", "POWER", "Power", nullptr, "W(DefaultReal)", JsonValue(R"json(["W(Real4U)", "KW(Real4U)", "MEGAW(Real4U)", "BTU/HR(Real4U)", "KILOBTU/HR(Real4U)", "HP(Real4U)"])json"), 0.001);
                    testDb.AssertKindOfQuantity("TestSchema", "LIQUID_VOLUME", "Liquid Volume", nullptr, "CUB.M(DefaultReal)", JsonValue(R"json(["LITRE(Real4U)", "GALLON(Real4U)"])json"), 0.0001);
                    if (TestDb::VersionSupportsFeature(testDb.GetECDbInitialVersion(), ECDbFeature::UnitsAndFormats))
                        testDb.AssertKindOfQuantity("TestSchema", "TestKoq_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.4);
                    else
                        {
                        //The original KOQ was serialized to disk in bim02dev in a wrong way, where it did persist the format along with the unit,
                        //although it shouldn't have one. This will not be fixed, as EC32 will make this obsolete anyways.
                        testDb.AssertKindOfQuantity("TestSchema", "TestKoq_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(R"json(["W/(M*K)(DefaultReal)"])json"), 0.4);
                        }

                    testDb.AssertKindOfQuantity("TestSchema", "TestKoq_PersUnitWithFormat_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(R"json(["W/(M*K)(DefaultReal)"])json"), 0.5);
                    testDb.AssertKindOfQuantity("TestSchema", "TestKoq_PersUnitWithFormatWithUnit_NoPresUnit", nullptr, nullptr, "FT(DefaultReal)", JsonValue(R"json(["FT(AmerFI8)"])json"), 0.6);

                    testDb.AssertKindOfQuantity("TestSchema", "TestKoq_M_Mfi8", nullptr, nullptr, "M(DefaultReal)", JsonValue(R"json(["FT(AmerFI8)"])json"), 0.7);
                    testDb.AssertKindOfQuantity("TestSchema", "TestKoq_Mfi8", nullptr, nullptr, "M(DefaultReal)", JsonValue(R"json(["FT(AmerFI8)"])json"), 0.8);
                    testDb.AssertKindOfQuantity("TestSchema", "TestKoq_SQFTfi8", nullptr, nullptr, "SQ.FT(DefaultReal)", JsonValue(), 0.9);
                    testDb.AssertKindOfQuantity("TestSchema", "TestKoq_SQFTfi8_SQFTreal4u", nullptr, nullptr, "SQ.FT(DefaultReal)", JsonValue(R"json(["SQ.FT(Real4U)"])json"), 1.0);
                    break;
                    }
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, SchemaManager_EC31KindOfQuantities)
    {
    Utf8CP koq1Name = "TestKoq_M_Mfi8";
    Utf8CP koq2Name = "TestKoq_SQFTfi8_SQFTreal4u";

    auto assertReferencedUnitsAndFormatsSchema = [] (TestIModel const& testDb, ECSchemaCR koqSchema)
        {
        const bool fileSupportsUnitsAndFormats = testDb.SupportsFeature(ECDbFeature::UnitsAndFormats);
        auto it = koqSchema.GetReferencedSchemas().Find(SchemaKey("Units", 1, 0), ECN::SchemaMatchType::Identical);
        ASSERT_EQ(fileSupportsUnitsAndFormats, it != koqSchema.GetReferencedSchemas().end()) << testDb.GetDescription();
        if (fileSupportsUnitsAndFormats)
            ASSERT_TRUE(it->second->HasId()) << testDb.GetDescription();

        it = koqSchema.GetReferencedSchemas().Find(SchemaKey("Formats", 1, 0), ECN::SchemaMatchType::Identical);
        ASSERT_EQ(fileSupportsUnitsAndFormats, it != koqSchema.GetReferencedSchemas().end()) << testDb.GetDescription();
        if (fileSupportsUnitsAndFormats)
            ASSERT_TRUE(it->second->HasId()) << testDb.GetDescription();
        };

    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31KOQS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            testDb.GetDb().ClearECDbCache();

            //load KOQs with empty cache
            KindOfQuantityCP koq1 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq1Name);
            ASSERT_TRUE(koq1 != nullptr) << testDb.GetDescription();

            KindOfQuantityCP koq2 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq2Name);
            ASSERT_TRUE(koq2 != nullptr) << testDb.GetDescription();
            testDb.GetDb().ClearECDbCache();

            //load KOQs after schema stub was loaded (but without elements)
            ASSERT_TRUE(testDb.GetDb().Schemas().GetSchema("TestSchema", false) != nullptr) << testDb.GetDescription();

            koq1 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq1Name);
            ASSERT_TRUE(koq1 != nullptr) << testDb.GetDescription();

            koq2 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq2Name);
            ASSERT_TRUE(koq2 != nullptr) << testDb.GetDescription();
            testDb.GetDb().ClearECDbCache();

            //load KOQs after schema was fully loaded
            ASSERT_TRUE(testDb.GetDb().Schemas().GetSchema("TestSchema", true) != nullptr) << testDb.GetDescription();

            koq1 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq1Name);
            ASSERT_TRUE(koq1 != nullptr) << testDb.GetDescription();

            koq2 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq2Name);
            ASSERT_TRUE(koq2 != nullptr) << testDb.GetDescription();
            testDb.GetDb().ClearECDbCache();

            //load KOQs after all schema stubs were loaded (no elements)
            testDb.GetDb().Schemas().GetSchemas(false);
            koq1 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq1Name);
            ASSERT_TRUE(koq1 != nullptr) << testDb.GetDescription();

            koq2 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq2Name);
            ASSERT_TRUE(koq2 != nullptr) << testDb.GetDescription();
            testDb.GetDb().ClearECDbCache();

            //load KOQs after all schema were fully loaded
            testDb.GetDb().Schemas().GetSchemas(true);
            koq1 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq1Name);
            ASSERT_TRUE(koq1 != nullptr) << testDb.GetDescription();

            koq2 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq2Name);
            ASSERT_TRUE(koq2 != nullptr) << testDb.GetDescription();
            testDb.GetDb().ClearECDbCache();

            //Load schema elements after a KOQ was loaded (for 4.0.0.1 files, temporary schemas are expected to be deserialized now.
            //This must ignore the temporary schema references as they don't have a schema id).
            koq1 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq1Name);
            ASSERT_TRUE(koq1 != nullptr) << testDb.GetDescription();
            assertReferencedUnitsAndFormatsSchema(testDb, koq1->GetSchema());

            koq2 = testDb.GetDb().Schemas().GetKindOfQuantity("TestSchema", koq2Name);
            ASSERT_TRUE(koq2 != nullptr) << testDb.GetDescription();
            assertReferencedUnitsAndFormatsSchema(testDb, koq2->GetSchema());

            bvector<ECSchemaCP> schemas = testDb.GetDb().Schemas().GetSchemas(true);
            const bool fileSupportsUnitsAndFormats = testDb.SupportsFeature(ECDbFeature::UnitsAndFormats);
            if (fileSupportsUnitsAndFormats)
                ASSERT_EQ(11, schemas.size()) << testDb.GetDescription();
            else
                ASSERT_EQ(9, schemas.size()) << testDb.GetDescription();

            bool containsUnitsSchema = false, containsFormatsSchema = false;
            for (ECSchemaCP schema : schemas)
                {
                if (schema->GetName().Equals("Units"))
                    containsUnitsSchema = true;
                if (schema->GetName().Equals("Formats"))
                    containsFormatsSchema = true;
                }
            ASSERT_EQ(fileSupportsUnitsAndFormats, containsUnitsSchema) << testDb.GetDescription();
            ASSERT_EQ(fileSupportsUnitsAndFormats, containsFormatsSchema) << testDb.GetDescription();

            testDb.GetDb().ClearECDbCache();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC32KindOfQuantities)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC32KOQS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            ASSERT_TRUE(testDb.SupportsFeature(ECDbFeature::UnitsAndFormats)) << testDb.GetDescription();

            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_PresFormatWithMandatoryComposite", "My first test KOQ", nullptr, "CM(DefaultReal)", JsonValue(R"js(["M(Real4U)"])js"), 0.1);
            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_PresFormatWithOptionalComposite", nullptr, "My second test KOQ", "CM(DefaultReal)", JsonValue(R"js(["FT(AmerFI8)"])js"), 0.2);
            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_PresFormatWithoutComposite", nullptr, nullptr, "CM(DefaultReal)", JsonValue(R"js(["FT(AmerFI8)"])js"), 0.3);
            testDb.AssertKindOfQuantity("TestSchema", "TestKoq_NoPresFormat", nullptr, nullptr, "KG(DefaultReal)", JsonValue(), 0.4);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC31Units)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            //this test only tests files which do not support EC32 units yet
            if (testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                continue;

            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            testDb.GetDb().ClearECDbCache();

            EXPECT_EQ(8, testDb.GetDb().Schemas().GetSchemas(false).size()) << testDb.GetDescription();

            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Units") == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("u", false, SchemaLookupMode::ByAlias) == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Formats") == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("f", false, SchemaLookupMode::ByAlias) == nullptr) << testDb.GetDescription();

            if (testDb.GetOpenParams().IsReadonly())
                continue;

            // PreEC3.1 code does not trigger deserializing the units/formats schema from disk
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <KindOfQuantity typeName="ANGLE" displayLabel="Angle" persistenceUnit="RAD(DefaultReal)" presentationUnits="ARC_DEG(real2u);ARC_DEG(dms)" relativeError="0.0001"/>
                     </ECSchema>)xml"));
            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            ASSERT_EQ(SchemaStatus::Success, testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas())) << testDb.GetDescription();

            EXPECT_EQ(9, testDb.GetDb().Schemas().GetSchemas(false).size()) << testDb.GetDescription();

            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Units") == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("u", false, SchemaLookupMode::ByAlias) == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Formats") == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("f", false, SchemaLookupMode::ByAlias) == nullptr) << testDb.GetDescription();
            }
        }

    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31KOQS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            //this test only tests files which do not support EC32 units yet
            if (testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                continue;

            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            testDb.GetDb().ClearECDbCache();

            EXPECT_EQ(9, testDb.GetDb().Schemas().GetSchemas(false).size()) << testDb.GetDescription();

            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Units") == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("u", false, SchemaLookupMode::ByAlias) == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Formats") == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("f", false, SchemaLookupMode::ByAlias) == nullptr) << testDb.GetDescription();

            if (testDb.GetOpenParams().IsReadonly())
                continue;

            // now import another schema. PreEC3.1 code does not trigger deserializing the units/formats schema from disk
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="NewSchema" alias="ns" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <KindOfQuantity typeName="ANGLE" displayLabel="Angle" persistenceUnit="RAD(DefaultReal)" presentationUnits="ARC_DEG(real2u);ARC_DEG(dms)" relativeError="0.0001"/>
                     </ECSchema>)xml"));
            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            ASSERT_EQ(SchemaStatus::Success, testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas())) << testDb.GetDescription();

            EXPECT_EQ(10, testDb.GetDb().Schemas().GetSchemas(false).size()) << testDb.GetDescription();

            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Units") == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("u", false, SchemaLookupMode::ByAlias) == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Formats") == nullptr) << testDb.GetDescription();
            EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("f", false, SchemaLookupMode::ByAlias) == nullptr) << testDb.GetDescription();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC32Units)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC32UNITS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            ASSERT_TRUE(testDb.SupportsFeature(ECDbFeature::UnitsAndFormats)) << testDb.GetDescription();

            testDb.AssertKindOfQuantity("TestSchema", "KoqWithCustomFormat", nullptr, nullptr, "M(DefaultReal)", JsonValue(R"js(["M(DefaultRealU)"])js"), 0.1);
            testDb.AssertKindOfQuantity("TestSchema", "KoqWithCustomUnit", nullptr, nullptr, "MySquareM(DefaultReal)", JsonValue(R"js(["MySquareM(Real4U)"])js"), 0.2);
            testDb.AssertKindOfQuantity("TestSchema", "KoqWithCustomUnitAndFormat", nullptr, nullptr, "MySquareFt(DefaultReal)", JsonValue(R"js(["MySquareFt(DefaultRealU)"])js"), 0.3);

            testDb.AssertUnitSystem("TestSchema", "MyMetric", "Metric", "Metric Units of measure");
            testDb.AssertUnitSystem("TestSchema", "MyImperial", "Imperial", "Units of measure from the British Empire");
            testDb.AssertUnitSystem("Units", "SI", nullptr, nullptr);
            testDb.AssertUnitSystem("Units", "CONSTANT", nullptr, nullptr);

            testDb.AssertPhenomenon("TestSchema", "MyArea", "Area", nullptr, "LENGTH*LENGTH");
            testDb.AssertPhenomenon("Units", "AREA", "Area", nullptr, "LENGTH(2)");
            testDb.AssertPhenomenon("Units", "TORQUE", "Torque", nullptr, "FORCE*LENGTH*ANGLE(-1)");
            testDb.AssertPhenomenon("Units", "LUMINOSITY", "Luminosity", nullptr, "LUMINOSITY");

            testDb.AssertUnit("TestSchema", "MySquareM", "Square Meter", nullptr, "M*M", 1.0, nullptr, nullptr, QualifiedName("TestSchema", "MyMetric"), QualifiedName("TestSchema", "MyArea"), false, QualifiedName());
            testDb.AssertUnit("TestSchema", "MySquareFt", "Square Feet", nullptr, "Ft*Ft", 10.0, nullptr, 0.4, QualifiedName("TestSchema", "MyImperial"), QualifiedName("TestSchema", "MyArea"), false, QualifiedName());
            testDb.AssertUnit("Units", "COULOMB", "C", nullptr, "A*S", nullptr, nullptr, nullptr, QualifiedName("Units", "SI"), QualifiedName("Units", "ELECTRIC_CHARGE"), false, QualifiedName());
            testDb.AssertUnit("Units", "PI", "Pi", nullptr, "ONE", 3.1415926535897932384626433832795, nullptr, nullptr, QualifiedName(), QualifiedName("Units", "LENGTH_RATIO"), true, QualifiedName());
            testDb.AssertUnit("Units", "QUARTER_PI", "Pi/4", nullptr, "PI", 1.0, 4.0, nullptr, QualifiedName(), QualifiedName("Units", "LENGTH_RATIO"), true, QualifiedName());
            testDb.AssertUnit("Units", "MILLI", "milli", nullptr, "ONE", .001, nullptr, nullptr, QualifiedName(), QualifiedName("Units", "NUMBER"), true, QualifiedName());
            testDb.AssertUnit("Units", "HORIZONTAL_PER_VERTICAL", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, QualifiedName("Units", "INTERNATIONAL"), QualifiedName("Units", "SLOPE"), false, QualifiedName("Units", "VERTICAL_PER_HORIZONTAL"));

            testDb.AssertFormat("TestSchema", "MyFormat", "My Format", nullptr, JsonValue(R"json({"roundFactor":0.3, "type": "Fractional", "showSignOption": "OnlyNegative", "formatTraits": ["trailZeroes", "keepSingleZero"], "precision": 4, "decimalSeparator": ".", "thousandSeparator": ",", "uomSeparator": " "})json"), JsonValue());
            testDb.AssertFormat("TestSchema", "MyFormatWithComposite", "My Format with composite", nullptr, JsonValue(R"json({"type": "Decimal", "formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"], "precision": 2})json"),
                                JsonValue(R"json({"includeZero":true, "spacer":"-", "units": [{"name":"HR", "label":"hour"}, {"name":"MIN", "label":"min"}]})json"));
            testDb.AssertFormat("Formats", "DefaultReal", "real", nullptr, JsonValue(R"json({"type": "Decimal", "formatTraits": ["keepSingleZero", "keepDecimalPoint"], "precision": 6})json"), JsonValue());
            testDb.AssertFormat("Formats", "AmerFI", "FeetInches", nullptr, JsonValue(R"json({"type": "Fractional", "formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"], "precision": 8, "uomSeparator":""})json"),
                                JsonValue(R"json({"includeZero":true, "spacer":"", "units": [{"name":"FT", "label":"'"}, {"name":"IN", "label":"\""}]})json"));
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC31SchemaImport)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>

                        <ECEntityClass typeName="MyDomainClass">
                                <BaseClass>bis:PhysicalElement</BaseClass>
                                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                                <ECProperty propertyName="Status" typeName="StatusEnum" />
                            </ECEntityClass>
                        <KindOfQuantity typeName="ANGLE" displayLabel="Angle" persistenceUnit="RAD(DefaultReal)" presentationUnits="ARC_DEG(real2u);ARC_DEG(dms)" relativeError="0.0001"/>
                        <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="SQ.M(DefaultReal)" presentationUnits="SQ.M(real4u);SQ.FT(real4u)" relativeError="0.0001"/>
                        <KindOfQuantity typeName="TEMPERATURE" displayLabel="Temperature" persistenceUnit="K(DefaultReal)" presentationUnits="CELSIUS(real4u);FAHRENHEIT(real4u);K(real4u)" relativeError="0.01"/>
                        <ECEnumeration typeName="StatusEnum" displayLabel="Int Enumeration with enumerators without display label" description="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                            <ECEnumerator value="0"/>
                            <ECEnumerator value="1"/>
                            <ECEnumerator value="2"/>
                        </ECEnumeration>
                     </ECSchema>)xml"));
            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());
            switch (testDb.GetAge())
                {
                    case ProfileState::Age::UpToDate:
                    {
                    EXPECT_EQ(SchemaStatus::Success, schemaImportStat) << testDb.GetDescription();
                    //No units or formats schema from EC3.2 must creep into the file when importing KOQs in an 4.0.0.1 file
                    EXPECT_EQ(JsonValue("[{\"cnt\": 0}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef WHERE Name IN ('Units','Formats')")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue("[{\"cnt\": 1}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef s JOIN meta.SchemaHasSchemaReferences ref ON s.ECInstanceId=ref.SourceECInstanceId WHERE s.Name='TestSchema'")) << testDb.GetDescription();

                    EXPECT_EQ(JsonValue("[]"), testDb.ExecuteECSqlSelect("SELECT ECInstanceId, Size, Status FROM ts.MyDomainClass")) << testDb.GetDescription();

                    ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", "MyDomainClass");
                    ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << testDb.GetDescription();
                    {
                    ECPropertyCP sizeProp = cl->GetPropertyP("Size");
                    ASSERT_TRUE(sizeProp != nullptr && sizeProp->GetIsPrimitive()) << testDb.GetDescription();
                    KindOfQuantityCP koq = sizeProp->GetAsPrimitiveProperty()->GetKindOfQuantity();
                    ASSERT_TRUE(koq != nullptr) << testDb.GetDescription();
                    testDb.AssertKindOfQuantity(*koq, "TestSchema", "AREA", "Area", nullptr, "SQ.M(DefaultReal)", JsonValue(R"json(["SQ.M(Real4U)", "SQ.FT(Real4U)"])json"), 0.0001);
                    }
                    {
                    ECPropertyCP statusProp = cl->GetPropertyP("Status");
                    ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << testDb.GetDescription();
                    ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
                    ASSERT_TRUE(ecenum != nullptr) << testDb.GetDescription();
                    testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                    {{ECValue(0), nullptr},
                    {ECValue(1), nullptr},
                    {ECValue(2), nullptr}});
                    }

                    testDb.AssertKindOfQuantity("TestSchema", "ANGLE", "Angle", nullptr, "RAD(DefaultReal)", JsonValue(R"json(["ARC_DEG(Real2U)", "ARC_DEG(AngleDMS)"])json"), 0.0001);
                    testDb.AssertKindOfQuantity("TestSchema", "AREA", "Area", nullptr, "SQ.M(DefaultReal)", JsonValue(R"json(["SQ.M(Real4U)", "SQ.FT(Real4U)"])json"), 0.0001);
                    testDb.AssertKindOfQuantity("TestSchema", "TEMPERATURE", "Temperature", nullptr, "K(DefaultReal)", JsonValue(R"json(["CELSIUS(Real4U)","FAHRENHEIT(Real4U)","K(Real4U)"])json"), 0.01);

                    testDb.AssertEnum("TestSchema", "StatusEnum", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                    {{ECValue(0), nullptr},
                    {ECValue(1), nullptr},
                    {ECValue(2), nullptr}});
                    break;
                    }

                    case ProfileState::Age::Older:
                    case ProfileState::Age::Newer:
                    {
                    EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                    break;
                    }
                    default:
                        FAIL() << "Unhandled ProfileState::Age enum value | " << testDb.GetDescription();
                        break;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC32SchemaImport_Enums)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>

                        <ECEntityClass typeName="MyDomainClass">
                                <BaseClass>bis:PhysicalElement</BaseClass>
                                <ECProperty propertyName="Status" typeName="StatusEnum" />
                        </ECEntityClass>
                        <ECEnumeration typeName="StatusEnum" displayLabel="Int Enumeration with enumerators without display label" description="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                            <ECEnumerator name="On" value="0"/>
                            <ECEnumerator name="Off" value="1"/>
                            <ECEnumerator name="Unknown" value="2"/>
                        </ECEnumeration>
                     </ECSchema>)xml"));
            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());
            switch (testDb.GetAge())
                {
                    case ProfileState::Age::UpToDate:
                    {
                    EXPECT_EQ(JsonValue("[]"), testDb.ExecuteECSqlSelect("SELECT ECInstanceId, Status FROM ts.MyDomainClass")) << testDb.GetDescription();

                    ECClassCP myDomainClass = testDb.GetDb().Schemas().GetClass("TestSchema", "MyDomainClass");
                    ASSERT_TRUE(myDomainClass != nullptr && myDomainClass->IsEntityClass()) << testDb.GetDescription();
                    {
                    ECPropertyCP statusProp = myDomainClass->GetPropertyP("Status");
                    ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << testDb.GetDescription();
                    ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
                    ASSERT_TRUE(ecenum != nullptr) << testDb.GetDescription();
                    testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                    {{ECValue(0), nullptr},
                    {ECValue(1), nullptr},
                    {ECValue(2), nullptr}});
                    }

                    testDb.AssertEnum("TestSchema", "StatusEnum", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                    {{ECValue(0), nullptr},
                    {ECValue(1), nullptr},
                    {ECValue(2), nullptr}});
                    break;
                    }

                    case ProfileState::Age::Older:
                    case ProfileState::Age::Newer:
                    {
                    EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                    break;
                    }
                    default:
                        FAIL() << "Unhandled ProfileState::Age enum value | " << testDb.GetDescription();
                        break;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC32SchemaImport_Koqs)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
                        <ECSchemaReference name="Units" version="01.00.00" alias="u" />
                        <ECSchemaReference name="Formats" version="01.00.00" alias="f" />

                        <ECEntityClass typeName="MyDomainClass">
                                <BaseClass>bis:PhysicalElement</BaseClass>
                                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                        </ECEntityClass>
                        <KindOfQuantity typeName="ANGLE" displayLabel="Angle" persistenceUnit="u:RAD" presentationUnits="f:DefaultRealU(2)[u:ARC_DEG]" relativeError="0.0001"/>
                        <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="u:SQ_M" presentationUnits="f:DefaultRealU(4)[u:SQ_M];f:DefaultRealU(4)[u:SQ_FT]" relativeError="0.0001"/>
                        <KindOfQuantity typeName="TEMPERATURE" displayLabel="Temperature" persistenceUnit="u:K" presentationUnits="f:DefaultRealU(4)[u:CELSIUS];f:DefaultRealU(4)[u:FAHRENHEIT];f:DefaultRealU(4)[u:K]" relativeError="0.01"/>
                     </ECSchema>)xml"));

            // The schema could even be deserialized, but the referenced Units and Formats schemas are not available (neither on disk
            // because this is the EC3.1 code stream, nor in the file).
            ASSERT_TRUE(deserializationCtx == nullptr) << testDb.GetDescription();
            continue;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC31Enum_SchemaUpgrade)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31ENUMS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //Schema changes:
            //- bumped up version to 1.0.1
            // - Enum StatusEnum
            //    - Changed display label to "Status"
            //    - Added enumerator 3
            // - Added subclass SubDomainClass
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>

             <ECEnumeration typeName="StatusEnum" displayLabel="Status" backingTypeName="int" isStrict="true">
                <ECEnumerator value="0"/>
                <ECEnumerator value="1"/>
                <ECEnumerator value="2"/>
                <ECEnumerator value="3"/>
             </ECEnumeration>
             <ECEntityClass typeName="MyDomainClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
                <ECProperty propertyName="Status" typeName="StatusEnum" />
             </ECEntityClass>
             <ECEntityClass typeName="SubDomainClass">
                <BaseClass>MyDomainClass</BaseClass>
                <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
             </ECEntityClass>
            </ECSchema>)xml"));
            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());
            switch (testDb.GetAge())
                {
                    case ProfileState::Age::UpToDate:
                    {
                    EXPECT_EQ(SchemaStatus::Success, schemaImportStat) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue("[]"), testDb.ExecuteECSqlSelect("SELECT ECInstanceId,Code,Status,Statuses FROM ts.SubDomainClass")) << testDb.GetDescription();

                    ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", "SubDomainClass");
                    ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << "SubDomainClass | " << testDb.GetDescription();

                    {
                    ECPropertyCP statusProp = cl->GetPropertyP("Status");
                    ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << "SubDomainClass | "  << testDb.GetDescription();
                    ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
                    ASSERT_TRUE(ecenum != nullptr) << "SubDomainClass | "  << testDb.GetDescription();
                    testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                        {{ECValue(0), nullptr},
                        {ECValue(1), nullptr},
                        {ECValue(2), nullptr},
                        {ECValue(3), nullptr}});
                    }

                    {
                    ECPropertyCP statusesProp = cl->GetPropertyP("Statuses");
                    ASSERT_TRUE(statusesProp != nullptr && statusesProp->GetIsPrimitiveArray()) << "SubDomainClass | "  << testDb.GetDescription();
                    ECEnumerationCP ecenum = statusesProp->GetAsPrimitiveArrayProperty()->GetEnumeration();
                    ASSERT_TRUE(ecenum != nullptr) << "SubDomainClass | "  << testDb.GetDescription();
                    testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                        {{ECValue(0), nullptr},
                        {ECValue(1), nullptr},
                        {ECValue(2), nullptr},
                        {ECValue(3), nullptr}});
                    }

                    testDb.AssertEnum("TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                        {{ECValue(0), nullptr},
                        {ECValue(1), nullptr},
                        {ECValue(2), nullptr},
                        {ECValue(3), nullptr}});
                    break;
                    }

                    case ProfileState::Age::Older:
                    case ProfileState::Age::Newer:
                    {
                    EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                    break;
                    }
                    default:
                        FAIL() << "Unhandled ProfileState::Age enum value | " << testDb.GetDescription();
                        break;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC31Koqs_SchemaUpgrade)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31KOQS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //Schema changes:
            //- bumped up version to 1.0.1
            //- KOQ AREA: 
            //    - Added presentation unit SQ.CM(real4u)
            //    - Changed relativeError to 0.001
            // - Added subclass SubDomainClass
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>

             <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="SQ.M(DefaultReal)" presentationUnits="SQ.M(real4u);SQ.FT(real4u);SQ.CM(real4u)" relativeError="0.001"/>
             <ECEntityClass typeName="MyDomainClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
             </ECEntityClass>
             <ECEntityClass typeName="SubDomainClass">
                <BaseClass>MyDomainClass</BaseClass>
                <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
             </ECEntityClass>
            </ECSchema>)xml"));
            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());
            switch (testDb.GetAge())
                {
                    case ProfileState::Age::UpToDate:
                    {
                    EXPECT_EQ(SchemaStatus::Success, schemaImportStat) << testDb.GetDescription();
                    //No units or formats schema from EC3.2 must creep into the file when importing KOQs in an 4.0.0.1 file
                    EXPECT_EQ(JsonValue("[{\"cnt\": 0}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef WHERE Name IN ('Units','Formats')")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue("[{\"cnt\": 1}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef s JOIN meta.SchemaHasSchemaReferences ref ON s.ECInstanceId=ref.SourceECInstanceId WHERE s.Name='TestSchema'")) << testDb.GetDescription();

                    EXPECT_EQ(JsonValue("[]"), testDb.ExecuteECSqlSelect("SELECT ECInstanceId,Size,Sizes FROM ts.SubDomainClass")) << testDb.GetDescription();

                    ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", "SubDomainClass");
                    ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << "SubDomainClass | " << testDb.GetDescription();
                    {
                    ECPropertyCP sizeProp = cl->GetPropertyP("Size");
                    ASSERT_TRUE(sizeProp != nullptr && sizeProp->GetIsPrimitive()) << "SubDomainClass | " << testDb.GetDescription();
                    KindOfQuantityCP koq = sizeProp->GetAsPrimitiveProperty()->GetKindOfQuantity();
                    ASSERT_TRUE(koq != nullptr) << "SubDomainClass | " << testDb.GetDescription();
                    testDb.AssertKindOfQuantity(*koq, "TestSchema", "AREA", "Area", nullptr, "SQ.M(DefaultReal)", JsonValue(R"json(["SQ.M(Real4U)", "SQ.FT(Real4U)", "SQ.CM(Real4U)"])json"), 0.001);
                    }

                    {
                    ECPropertyCP sizesProp = cl->GetPropertyP("Sizes");
                    ASSERT_TRUE(sizesProp != nullptr && sizesProp->GetIsPrimitiveArray()) << "SubDomainClass | " << testDb.GetDescription();
                    KindOfQuantityCP koq = sizesProp->GetAsPrimitiveArrayProperty()->GetKindOfQuantity();
                    ASSERT_TRUE(koq != nullptr) << "SubDomainClass | " << testDb.GetDescription();
                    testDb.AssertKindOfQuantity(*koq, "TestSchema", "AREA", "Area", nullptr, "SQ.M(DefaultReal)", JsonValue(R"json(["SQ.M(Real4U)", "SQ.FT(Real4U)", "SQ.CM(Real4U)"])json"), 0.001);
                    }

                    testDb.AssertKindOfQuantity("TestSchema", "AREA", "Area", nullptr, "SQ.M(DefaultReal)", JsonValue(R"json(["SQ.M(Real4U)", "SQ.FT(Real4U)", "SQ.CM(Real4U)"])json"), 0.001);
                    break;
                    }

                    case ProfileState::Age::Older:
                    case ProfileState::Age::Newer:
                    {
                    EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                    break;
                    }
                    default:
                        FAIL() << "Unhandled ProfileState::Age enum value | " << testDb.GetDescription();
                        break;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC31ToEC32SchemaUpgrade_Enums)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31ENUMS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //Schema changes:
            //- bumped up version to 1.0.1
            // - Enum StatusEnum
            //    - Changed display label to "Status"
            //    - add meaningful names to enumerators
            //    - Added enumerator 3
            // - Added subclass SubDomainClass
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
          <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.00" alias="bis" />

            <ECEnumeration typeName="StatusEnum" displayLabel="Status" backingTypeName="int" isStrict="true">
                <ECEnumerator name="On" value="0"/>
                <ECEnumerator name="Off" value="1"/>
                <ECEnumerator name="Unknown" value="2"/>
                <ECEnumerator name="Halfhalf" value="3"/>
            </ECEnumeration>
            <ECEntityClass typeName="MyDomainClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
                <ECProperty propertyName="Status" typeName="StatusEnum" />
             </ECEntityClass>
             <ECEntityClass typeName="SubDomainClass">
                <BaseClass>MyDomainClass</BaseClass>
                <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
             </ECEntityClass>
           </ECSchema>)xml"));
           ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
           const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());

           switch (testDb.GetAge())
               {
                   case ProfileState::Age::UpToDate:
                   {
                   EXPECT_EQ(SchemaStatus::Success, schemaImportStat) << testDb.GetDescription();
                   EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();

                   EXPECT_EQ(JsonValue("[]"), testDb.ExecuteECSqlSelect("SELECT ECInstanceId,Status,Statuses FROM ts.SubDomainClass")) << testDb.GetDescription();

                   ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", "SubDomainClass");
                   ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << "SubDomainClass | " << testDb.GetDescription();

                   {
                   ECPropertyCP statusProp = cl->GetPropertyP("Status");
                   ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << "SubDomainClass | " << testDb.GetDescription();
                   ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
                   ASSERT_TRUE(ecenum != nullptr) << "SubDomainClass | " << testDb.GetDescription();
                   testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                       {{ECValue(0), nullptr},
                       {ECValue(1), nullptr},
                       {ECValue(2), nullptr},
                       {ECValue(3), nullptr}});
                   }

                   {
                   ECPropertyCP statusesProp = cl->GetPropertyP("Statuses");
                   ASSERT_TRUE(statusesProp != nullptr && statusesProp->GetIsPrimitiveArray()) << "SubDomainClass | " << testDb.GetDescription();
                   ECEnumerationCP ecenum = statusesProp->GetAsPrimitiveArrayProperty()->GetEnumeration();
                   ASSERT_TRUE(ecenum != nullptr) << "SubDomainClass | " << testDb.GetDescription();
                   testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                       {{ECValue(0), nullptr},
                       {ECValue(1), nullptr},
                       {ECValue(2), nullptr},
                       {ECValue(3), nullptr}});
                   }

                   testDb.AssertEnum("TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                       {{ECValue(0), nullptr},
                       {ECValue(1), nullptr},
                       {ECValue(2), nullptr},
                       {ECValue(3), nullptr}});
                   break;
                   }

                   case ProfileState::Age::Older:
                   case ProfileState::Age::Newer:
                   {
                   EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                   break;
                   }
                   default:
                       FAIL() << "Unhandled ProfileState::Age enum value | " << testDb.GetDescription();
                       break;
               }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC31ToEC32SchemaUpgrade_Koqs)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC31KOQS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //Schema changes:
            //- bumped up version to 1.0.1
            //- KOQ AREA: 
            //    - Added presentation unit f:DefaultRealU(4)[u:SQ_CM]
            //    - Changed relativeError to 0.001
            // - Added subclass SubDomainClass
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
          <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
            <ECSchemaReference name="Units" version="01.00.00" alias="u" />
            <ECSchemaReference name="Formats" version="01.00.00" alias="f" />

            <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="u:SQ_M" presentationUnits="f:DefaultRealU(4)[u:SQ_M];f:DefaultRealU(4)[u:SQ_FT];f:DefaultRealU(4)[u:SQ_CM]" relativeError="0.001"/>
            <ECEntityClass typeName="MyDomainClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
             </ECEntityClass>
             <ECEntityClass typeName="SubDomainClass">
                <BaseClass>MyDomainClass</BaseClass>
                <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
             </ECEntityClass>
           </ECSchema>)xml"));

            if (!testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                {
                // The schema could even be deserialized, but the referenced Units and Formats schemas are not available unless,
                // they have been previously imported into an 4.0.0.2 file.
                ASSERT_TRUE(deserializationCtx == nullptr) << testDb.GetDescription();
                continue;
                }

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());
            switch (testDb.GetAge())
                {
                    case ProfileState::Age::Older:
                    case ProfileState::Age::UpToDate:
                    {
                    FAIL() << "Shouldn't get here, because the test schema cannot be deserialized for a 4.0.0.1 file because the units/formats schemas are not there | " << testDb.GetDescription();
                    break;
                    }

                    case ProfileState::Age::Newer:
                    {
                    EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                    break;
                    }
                    default:
                        FAIL() << "Unhandled ProfileState::Age enum value | " << testDb.GetDescription();
                        break;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC32SchemaUpgrade_Enums)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC32ENUMS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //Schema changes:
            //- bumped up version to 1.0.1
            // - Enum StatusEnum
            //    - Changed display label to "Status"
            //    - Added enumerator 3
            // - Added subclasses SubA, SubB, SubC
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
                 <ECEnumeration typeName="StatusEnum" displayLabel="Status" backingTypeName="int" isStrict="true">
                    <ECEnumerator name="On" value="0"/>
                    <ECEnumerator name="Off" value="1"/>
                    <ECEnumerator name="Unknown" value="2"/>
                    <ECEnumerator name="Halfhalf" value="3"/>
                 </ECEnumeration>
                  <ECEntityClass typeName="MyDomainClass">
                    <BaseClass>bis:PhysicalElement</BaseClass>
                    <ECProperty propertyName="Status" typeName="StatusEnum" />
                 </ECEntityClass>
                 <ECEntityClass typeName="SubDomainClass">
                    <BaseClass>MyDomainClass</BaseClass>
                    <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
                 </ECEntityClass>
                </ECSchema>)xml"));
            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());
            switch (testDb.GetAge())
                {
                    case ProfileState::Age::UpToDate:
                    {
                    // This test works on 4.0.0.1 files because the EC3.2 schema gets downgraded to EC3.1 by ECObjects during deserialization
                    EXPECT_EQ(SchemaStatus::Success, schemaImportStat) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("TestSchema")) << testDb.GetDescription();

                    EXPECT_EQ(JsonValue("[]"), testDb.ExecuteECSqlSelect("SELECT ECInstanceId,Code,Status,Statuses FROM ts.SubDomainClass")) << testDb.GetDescription();

                    ECClassCP cl = testDb.GetDb().Schemas().GetClass("TestSchema", "SubDomainClass");
                    ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << "SubDomainClass | " << testDb.GetDescription();

                    {
                    ECPropertyCP statusProp = cl->GetPropertyP("Status");
                    ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << "SubDomainClass | " << testDb.GetDescription();
                    ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
                    ASSERT_TRUE(ecenum != nullptr) << "SubDomainClass | " << testDb.GetDescription();
                    testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                    {{ECValue(0), nullptr},
                    {ECValue(1), nullptr},
                    {ECValue(2), nullptr},
                    {ECValue(3), nullptr}});
                    }

                    {
                    ECPropertyCP statusesProp = cl->GetPropertyP("Statuses");
                    ASSERT_TRUE(statusesProp != nullptr && statusesProp->GetIsPrimitiveArray()) << "SubDomainClass | " << testDb.GetDescription();
                    ECEnumerationCP ecenum = statusesProp->GetAsPrimitiveArrayProperty()->GetEnumeration();
                    ASSERT_TRUE(ecenum != nullptr) << "SubDomainClass | " << testDb.GetDescription();
                    testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                    {{ECValue(0), nullptr},
                    {ECValue(1), nullptr},
                    {ECValue(2), nullptr},
                    {ECValue(3), nullptr}});
                    }

                    testDb.AssertEnum("TestSchema", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                    {{ECValue(0), nullptr},
                    {ECValue(1), nullptr},
                    {ECValue(2), nullptr},
                    {ECValue(3), nullptr}});
                    break;
                    }

                    case ProfileState::Age::Older:
                    case ProfileState::Age::Newer:
                    {
                    EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                    break;
                    }
                    default:
                        FAIL() << "Unhandled ProfileState::Age enum value | " << testDb.GetDescription();
                        break;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC32SchemaUpgrade_Koqs)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC32KOQS_SCHEMAUPGRADE))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //Schema changes:
            //- bumped up version to 1.0.1
            //- KOQ AREA: 
            //    - Added presentation unit f:DefaultRealU(4)[u:SQ_CM]
            //    - Changed relativeError to 0.001
            // - Added subclass MyDomainClass
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
            <ECSchemaReference name="Units" version="01.00.00" alias="u" />
            <ECSchemaReference name="Formats" version="01.00.00" alias="f" />

            <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="u:SQ_M" presentationUnits="f:DefaultRealU(4)[u:SQ_M];f:DefaultRealU(4)[u:SQ_FT];f:DefaultRealU(4)[u:SQ_CM]" relativeError="0.001"/>
            <ECEntityClass typeName="MyDomainClass">
                <BaseClass>bis:PhysicalElement</BaseClass>
                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
             </ECEntityClass>
             <ECEntityClass typeName="SubDomainClass">
                <BaseClass>MyDomainClass</BaseClass>
                <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
             </ECEntityClass>
            </ECSchema>)xml"));
            if (!testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                {
                // The schema could even be deserialized, but the referenced Units and Formats schemas are not available unless,
                // they have been previously imported into an 4.0.0.2 file.
                ASSERT_TRUE(deserializationCtx == nullptr) << testDb.GetDescription();
                continue;
                }

            ASSERT_TRUE(deserializationCtx != nullptr) << testDb.GetDescription();
            const SchemaStatus schemaImportStat = testDb.GetDgnDb().ImportSchemas(deserializationCtx->GetCache().GetSchemas());
            switch (testDb.GetAge())
                {
                    case ProfileState::Age::Older:
                    case ProfileState::Age::UpToDate:
                    {
                    FAIL() << "Shouldn't get here, because the test schema cannot be deserialized for a 4.0.0.1 file because the units/formats schemas are not there | " << testDb.GetDescription();
                    break;
                    }

                    case ProfileState::Age::Newer:
                    {
                    EXPECT_EQ(SchemaStatus::SchemaImportFailed, schemaImportStat) << testDb.GetDescription();
                    break;
                    }
                    default:
                        FAIL() << "Unhandled ProfileState::Age enum value | " << testDb.GetDescription();
                        break;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, AddDomain)
    {
    ASSERT_EQ(SUCCESS, DgnDomains::RegisterDomain(FunctionalDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No));
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            const DbResult openStat = testDb.Open();
            DgnDb::OpenParams const& params = static_cast<DgnDb::OpenParams const&> (testDb.GetOpenParams());

            if (params.IsReadonly() || params.GetSchemaUpgradeOptions().GetDomainUpgradeOptions() != SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade)
                {
                ASSERT_EQ(BE_SQLITE_ERROR_SchemaUpgradeRequired, openStat) << testDb.GetDescription();
                continue;
                }

            if (testFile.GetAge() == ProfileState::Age::Newer)
                {
                //schema import not possible in newer files
                ASSERT_EQ(BE_SQLITE_ERROR_SchemaUpgradeFailed, openStat) << testDb.GetDescription();
                continue;
                }

            ASSERT_EQ(BE_SQLITE_OK, openStat) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            EXPECT_TRUE(testDb.GetDb().Schemas().ContainsSchema("Functional")) << testDb.GetDescription();
            EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("Functional")) << testDb.GetDescription();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, OpenDomainIModel)
    {
    ASSERT_EQ(SUCCESS, IModelEvolutionTestsDomain::Register(SchemaVersion(1, 0, 0), DgnDomain::Required::Yes, DgnDomain::Readonly::No));
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_TESTDOMAIN))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            EXPECT_TRUE(testDb.GetDb().Schemas().ContainsSchema(TESTDOMAIN_NAME)) << testDb.GetDescription();
            EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion(TESTDOMAIN_NAME)) << testDb.GetDescription();
            ECSqlStatement stmt;
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "SELECT Name,Material FROM me.Toy")) << testDb.GetDescription();
            stmt.Finalize();
            testDb.AssertEnum(TESTDOMAIN_NAME, "Material", nullptr, nullptr, PRIMITIVETYPE_Integer, true, 
                {{ECValue(0), "Wood"},
                 {ECValue(1), "Plastic"},
                 {ECValue(2), "Metal"}});
            }
        }
    IModelEvolutionTestsDomain::GetDomain().SetRequired(DgnDomain::Required::No);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, UpgradeDomainIModel)
    {
    // bump up domain schema version to trigger upgrade
    ASSERT_EQ(SUCCESS, IModelEvolutionTestsDomain::Register(SchemaVersion(1, 0, 1), DgnDomain::Required::Yes, DgnDomain::Readonly::No));
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_TESTDOMAIN))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            const DbResult openStat = testDb.Open();
            DgnDb::OpenParams const& params = static_cast<DgnDb::OpenParams const&> (testDb.GetOpenParams());

            if (testFile.GetAge() == ProfileState::Age::Newer)
                {
                //schema import not possible in newer files
                ASSERT_EQ(BE_SQLITE_ERROR_SchemaUpgradeFailed, openStat) << testDb.GetDescription();
                continue;
                }

            if (params.IsReadonly() || params.GetSchemaUpgradeOptions().GetDomainUpgradeOptions() != SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade)
                {
                //opens but schema is not upgraded
                ASSERT_EQ(BE_SQLITE_OK, openStat) << testDb.GetDescription();
                testDb.AssertProfileVersion();
                testDb.AssertLoadSchemas();

                EXPECT_TRUE(testDb.GetDb().Schemas().ContainsSchema(TESTDOMAIN_NAME)) << testDb.GetDescription();
                EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion(TESTDOMAIN_NAME)) << testDb.GetDescription();
                ECSqlStatement stmt;
                EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "SELECT Name,Material FROM me.Toy")) << testDb.GetDescription();
                stmt.Finalize();
                testDb.AssertEnum(TESTDOMAIN_NAME, "Material", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                {{ECValue(0), "Wood"},
                {ECValue(1), "Plastic"},
                {ECValue(2), "Metal"}});
                continue;
                }

            // opens and upgrades the schema
            ASSERT_EQ(BE_SQLITE_OK, openStat) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            EXPECT_TRUE(testDb.GetDb().Schemas().ContainsSchema(TESTDOMAIN_NAME)) << testDb.GetDescription();
            EXPECT_EQ(SchemaVersion(1, 0, 1), testDb.GetSchemaVersion(TESTDOMAIN_NAME)) << testDb.GetDescription();
            ECSqlStatement stmt;
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "SELECT Name,NickName,Material FROM me.Toy")) << testDb.GetDescription();
            stmt.Finalize();
            testDb.AssertEnum(TESTDOMAIN_NAME, "Material", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
            {{ECValue(0), "Wood"},
            {ECValue(1), "Plastic"},
            {ECValue(2), "Metal"},
            {ECValue(3), "Mix"}});
            }
        }

    IModelEvolutionTestsDomain::GetDomain().SetRequired(DgnDomain::Required::No);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      09/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, UpgradeDomainIModelToEC32)
    {
    // bump up domain schema version to trigger upgrade
    ASSERT_EQ(SUCCESS, IModelEvolutionTestsDomain::Register(SchemaVersion(1, 0, 2), DgnDomain::Required::Yes, DgnDomain::Readonly::No));
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_TESTDOMAIN))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            const DbResult openStat = testDb.Open();
            DgnDb::OpenParams const& params = static_cast<DgnDb::OpenParams const&> (testDb.GetOpenParams());

            if (testFile.GetAge() == ProfileState::Age::Newer)
                {
                //schema import not possible in newer files
                ASSERT_EQ(BE_SQLITE_ERROR_SchemaUpgradeFailed, openStat) << testDb.GetDescription();
                continue;
                }

            if (params.IsReadonly() || params.GetSchemaUpgradeOptions().GetDomainUpgradeOptions() != SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade)
                {
                //opens but schema is not upgraded
                ASSERT_EQ(BE_SQLITE_OK, openStat) << testDb.GetDescription();
                testDb.AssertProfileVersion();
                testDb.AssertLoadSchemas();

                EXPECT_TRUE(testDb.GetDb().Schemas().ContainsSchema(TESTDOMAIN_NAME)) << testDb.GetDescription();
                EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion(TESTDOMAIN_NAME)) << testDb.GetDescription();
                ECSqlStatement stmt;
                EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "SELECT Name,Material FROM me.Toy")) << testDb.GetDescription();
                stmt.Finalize();
                testDb.AssertEnum(TESTDOMAIN_NAME, "Material", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                {{ECValue(0), "Wood"},
                {ECValue(1), "Plastic"},
                {ECValue(2), "Metal"}});
                continue;
                }

            //opens and upgrades the schema
            ASSERT_EQ(BE_SQLITE_OK, openStat) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            EXPECT_TRUE(testDb.GetDb().Schemas().ContainsSchema(TESTDOMAIN_NAME)) << testDb.GetDescription();
            EXPECT_EQ(SchemaVersion(1, 0, 2), testDb.GetSchemaVersion(TESTDOMAIN_NAME)) << testDb.GetDescription();
            ECSqlStatement stmt;
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "SELECT Name,NickName,Material FROM me.Toy")) << testDb.GetDescription();
            stmt.Finalize();
            testDb.AssertEnum(TESTDOMAIN_NAME, "Material", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
            {{ECValue(0), "Wood"},
            {ECValue(1), "Plastic"},
            {ECValue(2), "Metal"},
            {ECValue(3), "Mix"}});
            }
        }

    IModelEvolutionTestsDomain::GetDomain().SetRequired(DgnDomain::Required::No);
    }