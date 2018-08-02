/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/IModelCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include "Profiles.h"
#include "TestIModelCreators.h"
#include "TestDb.h"

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
                    {
                    EXPECT_EQ(8, testDb.GetSchemaCount()) << testDb.GetDescription();

                    //iModel built-in schema versions
                    EXPECT_EQ(SchemaVersion(1, 0, 1), testDb.GetSchemaVersion("BisCore")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("BisCore")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":164, "enumcount": 2})js"), testDb.GetSchemaItemCounts("BisCore")) << testDb.GetDescription();

                    EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("Generic")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("Generic")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":17})js"), testDb.GetSchemaItemCounts("Generic")) << testDb.GetDescription();

                    //ECDb built-in schema versions
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

                    //Standard schema versions
                    EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("CoreCustomAttributes")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), testDb.GetSchemaItemCounts("CoreCustomAttributes")) << testDb.GetDescription();
                    break;
                    }

                    case ProfileState::Age::UpToDate:
                    {
                    EXPECT_EQ(8, testDb.GetSchemaCount()) << testDb.GetDescription();
                    //iModel built-in schema versions
                    EXPECT_EQ(SchemaVersion(1, 0, 1), testDb.GetSchemaVersion("BisCore")) << testDb.GetDescription();
                    if (testDb.GetOpenParams().GetProfileUpgradeOptions() == ECDb::ProfileUpgradeOptions::Upgrade || testDb.GetTestFile().IsUpgraded())
                        {
                        // if the file was upgraded, it doesn't have the original versions yet, as the BisCore schema is not upgraded automatically
                        EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("BisCore")) << testDb.GetDescription();
                        }
                    else
                        EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("BisCore")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":164, "enumcount": 2})js"), testDb.GetSchemaItemCounts("BisCore")) << testDb.GetDescription();

                    EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("Generic")) << testDb.GetDescription();
                    if (testDb.GetOpenParams().GetProfileUpgradeOptions() == ECDb::ProfileUpgradeOptions::Upgrade || testDb.GetTestFile().IsUpgraded())
                        {
                        // if the file was upgraded, it doesn't have the original versions yet, as the BisCore schema is not upgraded automatically
                        EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("Generic")) << testDb.GetDescription();
                        }
                    else
                        EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("Generic")) << testDb.GetDescription();

                    EXPECT_EQ(JsonValue(R"js({"classcount":17})js"), testDb.GetSchemaItemCounts("Generic")) << testDb.GetDescription();

                    //ECDb built-in schema versions
                    EXPECT_EQ(SchemaVersion(2, 0, 1), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), testDb.GetSchemaItemCounts("ECDbFileInfo")) << testDb.GetDescription();
                    
                    EXPECT_EQ(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbMap")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), testDb.GetSchemaItemCounts("ECDbMap")) << testDb.GetDescription();
                    
                    EXPECT_EQ(SchemaVersion(4, 0, 1), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbMeta")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":38, "enumcount": 8})js"), testDb.GetSchemaItemCounts("ECDbMeta")) << testDb.GetDescription();
                    
                    EXPECT_EQ(SchemaVersion(5, 0, 1), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbSystem")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), testDb.GetSchemaItemCounts("ECDbSystem")) << testDb.GetDescription();
                    
                    EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("ECDbSchemaPolicies")) << testDb.GetDescription();
                    if (testDb.GetOpenParams().GetProfileUpgradeOptions() == ECDb::ProfileUpgradeOptions::Upgrade || testDb.GetTestFile().IsUpgraded())
                        {
                        // if the file was upgraded, it doesn't have the original versions yet, as the BisCore schema is not upgraded automatically
                        EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("ECDbSchemaPolicies")) << testDb.GetDescription();
                        }
                    else
                        EXPECT_EQ(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbSchemaPolicies")) << testDb.GetDescription();

                    EXPECT_EQ(JsonValue(R"js({"classcount":3})js"), testDb.GetSchemaItemCounts("ECDbSchemaPolicies")) << testDb.GetDescription();

                    //Standard schema versions
                    EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("CoreCustomAttributes")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), testDb.GetSchemaItemCounts("CoreCustomAttributes")) << testDb.GetDescription();
                    break;
                    }

                    case ProfileState::Age::Newer:
                    {
                    EXPECT_LE(8, testDb.GetSchemaCount()) << testDb.GetDescription();

                    //iModel built-in schema versions
                    EXPECT_LE(SchemaVersion(1, 0, 1), testDb.GetSchemaVersion("BisCore")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("BisCore")) << testDb.GetDescription();

                    EXPECT_LE(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("Generic")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("Generic")) << testDb.GetDescription();

                    //ECDb built-in schema versions
                    EXPECT_LE(SchemaVersion(2, 0, 1), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << testDb.GetDescription();

                    EXPECT_LE(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbMap")) << testDb.GetDescription();

                    EXPECT_LE(SchemaVersion(4, 0, 1), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbMeta")) << testDb.GetDescription();

                    EXPECT_LT(SchemaVersion(5, 0, 1), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbSystem")) << testDb.GetDescription();

                    EXPECT_LE(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("ECDbSchemaPolicies")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 2), testDb.GetOriginalECXmlVersion("ECDbSchemaPolicies")) << testDb.GetDescription();

                    //Standard schema versions
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
TEST_F(IModelCompatibilityTestFixture, PreEC32Enums)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_PREEC32ENUMS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            if (!testDb.SupportsFeature(ECDbFeature::NamedEnumerators))
                {
                testDb.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                {{"Unspecified", ECValue("Unspecified"), nullptr},
                {"Utc", ECValue("Utc"), nullptr},
                {"Local", ECValue("Local"), nullptr}});

                testDb.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                {{"ECClassModifier0", ECValue(0), "None"},
                {"ECClassModifier1", ECValue(1), "Abstract"},
                {"ECClassModifier2", ECValue(2), "Sealed"}});

                testDb.AssertEnum("PreEC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{"IntEnum_EnumeratorsWithoutDisplayLabel0", ECValue(0), nullptr},
                {"IntEnum_EnumeratorsWithoutDisplayLabel1", ECValue(1), nullptr},
                {"IntEnum_EnumeratorsWithoutDisplayLabel2", ECValue(2), nullptr}});

                testDb.AssertEnum("PreEC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                {{"On", ECValue("On"), "Turned On"},
                {"Off", ECValue("Off"), "Turned Off"}});
                }
            else
                {
                testDb.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                {{"Unspecified", ECValue("Unspecified"), nullptr},
                {"Utc", ECValue("Utc"), nullptr},
                {"Local", ECValue("Local"), nullptr}});

                testDb.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                {{"None", ECValue(0), "None"},
                {"Abstract", ECValue(1), "Abstract"},
                {"Sealed", ECValue(2), "Sealed"}});

                testDb.AssertEnum("PreEC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{"IntEnum_EnumeratorsWithoutDisplayLabel0", ECValue(0), nullptr},
                {"IntEnum_EnumeratorsWithoutDisplayLabel1", ECValue(1), nullptr},
                {"IntEnum_EnumeratorsWithoutDisplayLabel2", ECValue(2), nullptr}});

                testDb.AssertEnum("PreEC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                {{"On", ECValue("On"), "Turned On"},
                {"Off", ECValue("Off"), "Turned Off"}});
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      07/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, UpgradingPreEC32EnumsToEC32AfterProfileUpgrade)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_UPGRADEDEC32ENUMS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            // older files for which the schema upgrade wasn't run must have the auto-generated enumerator names
            if (testDb.GetOriginalECXmlVersion("UpgradedEC32Enums") <= BeVersion(3, 1))
                {
                testDb.AssertEnum("UpgradedEC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{"IntEnum_EnumeratorsWithoutDisplayLabel0", ECValue(0), nullptr},
                {"IntEnum_EnumeratorsWithoutDisplayLabel1", ECValue(1), nullptr},
                {"IntEnum_EnumeratorsWithoutDisplayLabel2", ECValue(2), nullptr}});

                testDb.AssertEnum("UpgradedEC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                {{"On", ECValue("On"), "Turned On"},
                {"Off", ECValue("Off"), "Turned Off"}});
                }
            else
                {
                testDb.AssertEnum("UpgradedEC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{"Unknown", ECValue(0), nullptr},
                {"On", ECValue(1), nullptr},
                {"Off", ECValue(2), nullptr}});

                testDb.AssertEnum("UpgradedEC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                {{"An", ECValue("On"), "Turned On"},
                {"Aus", ECValue("Off"), "Turned Off"}});
                }

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
            ASSERT_TRUE(testDb.SupportsFeature(ECDbFeature::NamedEnumerators)) << testDb.GetDescription();

            testDb.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
            {{"Unspecified", ECValue("Unspecified"), nullptr},
            {"Utc", ECValue("Utc"), nullptr},
            {"Local", ECValue("Local"), nullptr}});

            testDb.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
            {{"None", ECValue(0), "None"},
            {"Abstract", ECValue(1), "Abstract"},
            {"Sealed", ECValue(2), "Sealed"}});

            testDb.AssertEnum("EC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
            {{"Unknown", ECValue(0), nullptr},
            {"On", ECValue(1), nullptr},
            {"Off", ECValue(2), nullptr}});

            testDb.AssertEnum("EC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
            {{"On", ECValue("On"), "Turned On"},
            {"Off", ECValue("Off"), "Turned Off"}});
            }
        }
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, PreEC32KindOfQuantities)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_PREEC32KOQS))
        {
        for (std::unique_ptr<TestIModel> testDbPtr : TestIModel::GetPermutationsFor(testFile))
            {
            TestIModel& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            testDb.AssertKindOfQuantity("PreEC32Koqs", "ANGLE", "Angle", nullptr, "u:RAD", JsonValue(R"json(["f:DefaultRealU(2)[u:ARC_DEG]", "f:AngleDMS"])json"), 0.0001);
            testDb.AssertKindOfQuantity("PreEC32Koqs", "POWER", "Power", nullptr, "u:W", JsonValue(R"json(["f:DefaultRealU(4)[u:W]", "f:DefaultRealU(4)[u:KW]", "f:DefaultRealU(4)[u:MEGAW]", "f:DefaultRealU(4)[u:BTU_PER_HR]", "f:DefaultRealU(4)[u:KILOBTU_PER_HR]", "f:DefaultRealU(4)[u:HP]"])json"), 0.001);
            testDb.AssertKindOfQuantity("PreEC32Koqs", "LIQUID_VOLUME", "Liquid Volume", nullptr, "u:CUB_M", JsonValue(R"json(["f:DefaultRealU(4)[u:LITRE]", "f:DefaultRealU(4)[u:GALLON]"])json"), 0.0001);

            if (TestDb::VersionSupportsFeature(testDb.GetECDbInitialVersion(), ECDbFeature::UnitsAndFormats))
                {
                testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_NoPresUnit", nullptr, nullptr, "u:W_PER_M_K", JsonValue(), 0.4);

                // KOQs which are actually invalid in EC3.1 and could not be deserialized in pre EC3.2 code. In EC3.2 code this is tolerated
                // but invalid pres formats are dropped.
                // So these tests may only be run if the file was created with code that supports EC3.2 or later
                testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_LUX_M", nullptr, nullptr, "u:LUX", JsonValue(), 1.1);
                testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_M_LUX", nullptr, nullptr, "u:M", JsonValue(), 1.2);
                testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_M_SQFTreal4u", nullptr, nullptr, "u:M", JsonValue(), 1.3);
                testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_M_CM_LUX", nullptr, nullptr, "u:M", JsonValue(R"json(["f:DefaultReal[u:CM]"])json"), 1.4);
                testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_LUX_CM_MM", nullptr, nullptr, "u:LUX", JsonValue(), 1.5);
                testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_LUXreal4u_CM_MM", nullptr, nullptr, "u:LUX", JsonValue(R"json(["f:DefaultRealU(4)[u:LUX]"])json"), 1.6);
                }
            else
                {
                //The original KOQ was serialized to disk in bim02dev in a wrong way, where it did persist the format along with the unit,
                //although it shouldn't have one. This will not be fixed, as EC32 will make this obsolete anyways.
                testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_NoPresUnit", nullptr, nullptr, "u:W_PER_M_K", JsonValue(R"json(["f:DefaultReal[u:W_PER_M_K]"])json"), 0.4);
                }

            if (!testDb.SupportsFeature(ECDbFeature::UnitsAndFormats))
                {
                EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Units", false) == nullptr) << testDb.GetDescription();
                EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Formats", false) == nullptr) << testDb.GetDescription();
                EXPECT_TRUE(testDb.GetDb().Schemas().GetUnit("Units", "COULOMB") == nullptr) << testDb.GetDescription();
                EXPECT_TRUE(testDb.GetDb().Schemas().GetUnitSystem("Units", "SI") == nullptr) << testDb.GetDescription();
                EXPECT_TRUE(testDb.GetDb().Schemas().GetPhenomenon("Units", "LUMINOSITY") == nullptr) << testDb.GetDescription();
                EXPECT_TRUE(testDb.GetDb().Schemas().GetFormat("Formats", "AmerFI") == nullptr) << testDb.GetDescription();

                ECSqlStatement stmt;
                EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(testDb.GetDb(), "SELECT * FROM meta.UnitDef")) << testDb.GetDescription();
                stmt.Finalize();
                EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(testDb.GetDb(), "SELECT * FROM meta.UnitSystemDef")) << testDb.GetDescription();
                stmt.Finalize();
                EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(testDb.GetDb(), "SELECT * FROM meta.PhenomenonDef")) << testDb.GetDescription();
                stmt.Finalize();
                EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(testDb.GetDb(), "SELECT * FROM meta.FormatDef")) << testDb.GetDescription();
                stmt.Finalize();
                EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(testDb.GetDb(), "SELECT * FROM meta.FormatCompositeUnitDef")) << testDb.GetDescription();
                stmt.Finalize();
                }
            else
                {
                EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Units", false) != nullptr) << testDb.GetDescription();
                EXPECT_TRUE(testDb.GetDb().Schemas().GetSchema("Formats", false) != nullptr) << testDb.GetDescription();

                testDb.AssertUnit("Units", "COULOMB", "C", nullptr, "A*S", nullptr, nullptr, nullptr, QualifiedName("Units", "SI"), QualifiedName("Units", "ELECTRIC_CHARGE"), false, QualifiedName());
                testDb.AssertUnitSystem("Units", "SI", nullptr, nullptr);
                testDb.AssertPhenomenon("Units", "LUMINOSITY", "Luminosity", nullptr, "LUMINOSITY");
                testDb.AssertFormat("Formats", "AmerFI", "FeetInches", nullptr, JsonValue(R"json({"type": "Fractional", "formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"], "precision": 8, "uomSeparator":""})json"),
                                    JsonValue(R"json({"includeZero":true, "spacer":"", "units": [{"name":"FT", "label":"'"}, {"name":"IN", "label":"\""}]})json"));

                }
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

            testDb.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithMandatoryComposite", "My first test KOQ", nullptr, "u:CM", JsonValue(R"js(["f:DefaultRealU(4)[u:M]"])js"), 0.1);
            testDb.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithOptionalComposite", nullptr, "My second test KOQ", "u:CM", JsonValue(R"js(["f:AmerFI[u:FT|feet][u:IN|inches]"])js"), 0.2);
            testDb.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithoutComposite", nullptr, nullptr, "u:CM", JsonValue(R"js(["f:AmerFI"])js"), 0.3);
            testDb.AssertKindOfQuantity("EC32Koqs", "TestKoq_NoPresFormat", nullptr, nullptr, "u:KG", JsonValue(), 0.4);
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

            testDb.AssertKindOfQuantity("EC32Units", "KoqWithCustomFormat", nullptr, nullptr, "u:M", JsonValue(R"js(["MyFormat[u:M]"])js"), 0.1);
            testDb.AssertKindOfQuantity("EC32Units", "KoqWithCustomUnit", nullptr, nullptr, "MySquareM", JsonValue(R"js(["f:DefaultRealU(4)[MySquareM]"])js"), 0.2);
            testDb.AssertKindOfQuantity("EC32Units", "KoqWithCustomUnitAndFormat", nullptr, nullptr, "MySquareFt", JsonValue(R"js(["MyFormat[MySquareFt]"])js"), 0.3);

            testDb.AssertUnitSystem("EC32Units", "MyMetric", "Metric", "Metric Units of measure");
            testDb.AssertUnitSystem("EC32Units", "MyImperial", "Imperial", "Units of measure from the British Empire");
            testDb.AssertUnitSystem("Units", "SI", nullptr, nullptr);
            testDb.AssertUnitSystem("Units", "CONSTANT", nullptr, nullptr);

            testDb.AssertPhenomenon("EC32Units", "MyArea", "Area", nullptr, "LENGTH*LENGTH");
            testDb.AssertPhenomenon("Units", "AREA", "Area", nullptr, "LENGTH(2)");
            testDb.AssertPhenomenon("Units", "TORQUE", "Torque", nullptr, "FORCE*LENGTH*ANGLE(-1)");
            testDb.AssertPhenomenon("Units", "LUMINOSITY", "Luminosity", nullptr, "LUMINOSITY");

            testDb.AssertUnit("EC32Units", "MySquareM", "Square Meter", nullptr, "M*M", 1.0, nullptr, nullptr, QualifiedName("EC32Units", "MyMetric"), QualifiedName("EC32Units", "MyArea"), false, QualifiedName());
            testDb.AssertUnit("EC32Units", "MySquareFt", "Square Feet", nullptr, "Ft*Ft", 10.0, nullptr, 0.4, QualifiedName("EC32Units", "MyImperial"), QualifiedName("EC32Units", "MyArea"), false, QualifiedName());
            testDb.AssertUnit("Units", "COULOMB", "C", nullptr, "A*S", nullptr, nullptr, nullptr, QualifiedName("Units", "SI"), QualifiedName("Units", "ELECTRIC_CHARGE"), false, QualifiedName());
            testDb.AssertUnit("Units", "PI", "Pi", nullptr, "ONE", 3.1415926535897932384626433832795, nullptr, nullptr, QualifiedName(), QualifiedName("Units", "LENGTH_RATIO"), true, QualifiedName());
            testDb.AssertUnit("Units", "QUARTER_PI", "Pi/4", nullptr, "PI", 1.0, 4.0, nullptr, QualifiedName(), QualifiedName("Units", "LENGTH_RATIO"), true, QualifiedName());
            testDb.AssertUnit("Units", "MILLI", "milli", nullptr, "ONE", .001, nullptr, nullptr, QualifiedName(), QualifiedName("Units", "NUMBER"), true, QualifiedName());
            testDb.AssertUnit("Units", "HORIZONTAL_PER_VERTICAL", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, QualifiedName("Units", "INTERNATIONAL"), QualifiedName("Units", "SLOPE"), false, QualifiedName("Units", "VERTICAL_PER_HORIZONTAL"));

            testDb.AssertFormat("EC32Units", "MyFormat", "My Format", nullptr, JsonValue(R"json({"roundFactor":0.3, "type": "Fractional", "showSignOption": "OnlyNegative", "formatTraits": ["trailZeroes", "keepSingleZero"], "precision": 4, "decimalSeparator": ".", "thousandSeparator": ",", "uomSeparator": " "})json"), JsonValue());
            testDb.AssertFormat("EC32Units", "MyFormatWithComposite", "My Format with composite", nullptr, JsonValue(R"json({"type": "Decimal", "formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"], "precision": 2})json"),
                                JsonValue(R"json({"includeZero":true, "spacer":"-", "units": [{"name":"HR", "label":"hour"}, {"name":"MIN", "label":"min"}]})json"));
            testDb.AssertFormat("Formats", "DefaultReal", "real", nullptr, JsonValue(R"json({"type": "Decimal", "formatTraits": ["keepSingleZero", "keepDecimalPoint"], "precision": 6})json"), JsonValue());
            testDb.AssertFormat("Formats", "AmerFI", "FeetInches", nullptr, JsonValue(R"json({"type": "Fractional", "formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"], "precision": 8, "uomSeparator":""})json"),
                                JsonValue(R"json({"includeZero":true, "spacer":"", "units": [{"name":"FT", "label":"'"}, {"name":"IN", "label":"\""}]})json"));
            }
        }
    }
