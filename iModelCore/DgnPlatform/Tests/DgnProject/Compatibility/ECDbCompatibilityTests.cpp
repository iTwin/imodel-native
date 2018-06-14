/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/ECDbCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"
#include "Profiles.h"
#include "TestECDbCreators.h"
#include "TestHelper.h"

USING_NAMESPACE_BENTLEY_EC

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct ECDbCompatibilityTestFixture : CompatibilityTestFixture
    {
    protected:
        DbResult OpenTestFile(ECDb& ecdb, TestFile const& testFile) { return ecdb.OpenBeSQLiteDb(testFile.GetPath(), ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::Upgrade)); }

        void SetUp() override { ASSERT_EQ(SUCCESS, TestECDbCreation::Run()); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, BuiltinSchemaVersions)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EMPTY))
        {
        ProfileState::Age originalFileAge = testFile.GetAge();
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile)) << testFile.ToString();

        ProfileState actualProfileState = ecdb.CheckProfileVersion();

        TestHelper helper(testFile, ecdb);
        helper.AssertLoadSchemas();
        const int schemaCount = helper.GetSchemaCount();

        switch (originalFileAge)
            {
                case ProfileState::Age::UpToDate:
                {
                EXPECT_TRUE(actualProfileState.IsUpToDate()) << testFile.ToString();
                EXPECT_EQ(ECDbProfile::Get().GetExpectedVersion(), ecdb.GetECDbProfileVersion()) << testFile.ToString();

                EXPECT_EQ(5, schemaCount) << testFile.ToString();
                for (ECSchemaCP schema : ecdb.Schemas().GetSchemas(false))
                    {
                    EXPECT_EQ((int) ECVersion::V3_2, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << testFile.ToString();;
                    }

                //ECDb built-in schema versions
                EXPECT_EQ(SchemaVersion(2, 0, 1), helper.GetSchemaVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), helper.GetSchemaItemCounts("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), helper.GetSchemaItemCounts("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(4, 0, 1), helper.GetSchemaVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":38, "enumcount": 9})js"), helper.GetSchemaItemCounts("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(5, 0, 1), helper.GetSchemaVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), helper.GetSchemaItemCounts("ECDbSystem")) << testFile.ToString();

                //Standard schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 1), helper.GetOriginalECXmlVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), helper.GetSchemaItemCounts("CoreCustomAttributes")) << testFile.ToString();
                break;
                }

                case ProfileState::Age::Older:
                {
                EXPECT_TRUE(actualProfileState.IsUpToDate()) << "File is expected to be auto-upgraded. " << testFile.ToString();
                EXPECT_EQ(ECDbProfile::Get().GetExpectedVersion(), ecdb.GetECDbProfileVersion()) << "File is expected to be auto-upgraded. " << testFile.ToString();

                EXPECT_EQ(5, schemaCount) << testFile.ToString();
                for (ECSchemaCP schema : ecdb.Schemas().GetSchemas(false))
                    {
                    EXPECT_EQ((int) ECVersion::V3_2, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    }

                //ECDb built-in schema versions
                EXPECT_EQ(SchemaVersion(2, 0, 1), helper.GetSchemaVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbFileInfo")) << "Schema has enums which were upgraded to EC32 format, therefore original XML version was set to 3.2 | " << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), helper.GetSchemaItemCounts("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(BeVersion(), helper.GetOriginalECXmlVersion("ECDbMap")) << "Schema has no enums, so was not upgraded to EC32, so no original ECXML persisted yet | " << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), helper.GetSchemaItemCounts("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(4, 0, 1), helper.GetSchemaVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMeta")) << "Schema has enums which were upgraded to EC32 format, therefore original XML version was set to 3.2 | " << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":24, "enumcount": 8})js"), helper.GetSchemaItemCounts("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(5, 0, 1), helper.GetSchemaVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(BeVersion(), helper.GetOriginalECXmlVersion("ECDbSystem")) << "Schema has no enums, so was not upgraded to EC32, so no original ECXML persisted yet | " << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), helper.GetSchemaItemCounts("ECDbSystem")) << testFile.ToString();

                //Standard schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_EQ(BeVersion(), helper.GetOriginalECXmlVersion("CoreCustomAttributes")) << "Schema has no enums, so was not upgraded to EC32, so no original ECXML persisted yet | " << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), helper.GetSchemaItemCounts("CoreCustomAttributes")) << testFile.ToString();
                break;
                }

                case ProfileState::Age::Newer:
                {
                EXPECT_TRUE(actualProfileState.IsNewer()) << testFile.ToString();
                EXPECT_LT(ECDbProfile::Get().GetExpectedVersion(), ecdb.GetECDbProfileVersion()) << testFile.ToString();

                EXPECT_EQ(5, schemaCount) << testFile.ToString();

                for (ECSchemaCP schema : ecdb.Schemas().GetSchemas(false))
                    {
                    EXPECT_LE((int) ECVersion::V3_2, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    }

                EXPECT_LE(SchemaVersion(2, 0, 1), helper.GetSchemaVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbMap")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMap")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(4, 0, 1), helper.GetSchemaVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(5, 0, 1), helper.GetSchemaVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbSystem")) << testFile.ToString();
                //Standard schema versions
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 1), helper.GetOriginalECXmlVersion("CoreCustomAttributes")) << testFile.ToString();

                break;
                }
            }
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, PreEC32Enums)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_PREEC32ENUMS))
        {
        ProfileState::Age originalFileAge = testFile.GetAge();

        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile)) << testFile.ToString();

        ProfileState actualProfileState = ecdb.CheckProfileVersion();
        switch (originalFileAge)
            {
                case ProfileState::Age::Older:
                    EXPECT_TRUE(actualProfileState.IsUpToDate()) << "File is expected to be auto-upgraded. " << testFile.ToString();
                    EXPECT_EQ(ECDbProfile::Get().GetExpectedVersion(), ecdb.GetECDbProfileVersion()) << "File is expected to be auto-upgraded. " << testFile.ToString();
                    break;
                case ProfileState::Age::UpToDate:
                    EXPECT_TRUE(actualProfileState.IsUpToDate()) << testFile.ToString();
                    EXPECT_EQ(ECDbProfile::Get().GetExpectedVersion(), ecdb.GetECDbProfileVersion()) << testFile.ToString();
                    break;
                case ProfileState::Age::Newer:
                    EXPECT_TRUE(actualProfileState.IsNewer()) << testFile.ToString();
                    EXPECT_LT(ECDbProfile::Get().GetExpectedVersion(), ecdb.GetECDbProfileVersion()) << testFile.ToString();
                    break;

                default:
                    FAIL() << "unhandled enum value";
            }

        TestHelper helper(testFile, ecdb);
        helper.AssertLoadSchemas();

        helper.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                    {{"Unspecified", ECValue("Unspecified"), nullptr},
                    {"Utc", ECValue("Utc"), nullptr},
                    {"Local", ECValue("Local"), nullptr}});

        helper.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                {{"None", ECValue(0), "None"},
                {"Abstract", ECValue(1), "Abstract"},
                {"Sealed", ECValue(2), "Sealed"}});

        helper.AssertEnum("PreEC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{"IntEnum_EnumeratorsWithoutDisplayLabel0", ECValue(0), nullptr},
                {"IntEnum_EnumeratorsWithoutDisplayLabel1", ECValue(1), nullptr},
                {"IntEnum_EnumeratorsWithoutDisplayLabel2", ECValue(2), nullptr}});

        helper.AssertEnum("PreEC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                {{"On", ECValue("On"), "Turned On"},
                {"Off", ECValue("Off"), "Turned Off"}});
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32Enums)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32ENUMS))
        {
        ProfileState::Age originalFileAge = testFile.GetAge();

        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile)) << testFile.ToString();

        ProfileState actualProfileState = ecdb.CheckProfileVersion();
        switch (originalFileAge)
            {
                case ProfileState::Age::Older:
                    EXPECT_TRUE(actualProfileState.IsUpToDate()) << "File is expected to be auto-upgraded. " << testFile.ToString();
                    EXPECT_EQ(ECDbProfile::Get().GetExpectedVersion(), ecdb.GetECDbProfileVersion()) << "File is expected to be auto-upgraded. " << testFile.ToString();
                    break;
                case ProfileState::Age::UpToDate:
                    EXPECT_TRUE(actualProfileState.IsUpToDate()) << testFile.ToString();
                    EXPECT_EQ(ECDbProfile::Get().GetExpectedVersion(), ecdb.GetECDbProfileVersion()) << testFile.ToString();
                    break;
                case ProfileState::Age::Newer:
                    EXPECT_TRUE(actualProfileState.IsNewer()) << testFile.ToString();
                    EXPECT_LT(ECDbProfile::Get().GetExpectedVersion(), ecdb.GetECDbProfileVersion()) << testFile.ToString();
                    break;

                default:
                    FAIL() << "unhandled enum value";
            }

        TestHelper helper(testFile, ecdb);
        helper.AssertLoadSchemas();

        helper.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
            {{"Unspecified", ECValue("Unspecified"), nullptr},
            {"Utc", ECValue("Utc"), nullptr},
            {"Local", ECValue("Local"), nullptr}});

        helper.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
            {{"None", ECValue(0), "None"},
            {"Abstract", ECValue(1), "Abstract"},
            {"Sealed", ECValue(2), "Sealed"}});

        helper.AssertEnum("EC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
            {{"Unknown", ECValue(0), nullptr},
            {"On", ECValue(1), nullptr},
            {"Off", ECValue(2), nullptr}});

        helper.AssertEnum("EC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
            {{"On", ECValue("On"), "Turned On"},
            {"Off", ECValue("Off"), "Turned Off"}});
        }

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, PreEC32KindOfQuantities)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_PREEC32KOQS))
        {
        ProfileState::Age originalFileAge = testFile.GetAge();

        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile)) << testFile.ToString();

        ProfileState actualProfileState = ecdb.CheckProfileVersion();
        switch (originalFileAge)
            {
                case ProfileState::Age::Older:
                    EXPECT_TRUE(actualProfileState.IsUpToDate()) << "File is expected to be auto-upgraded. " << testFile.ToString();
                    EXPECT_EQ(ECDbProfile::Get().GetExpectedVersion(), ecdb.GetECDbProfileVersion()) << "File is expected to be auto-upgraded. " << testFile.ToString();
                    break;
                case ProfileState::Age::UpToDate:
                    EXPECT_TRUE(actualProfileState.IsUpToDate()) << testFile.ToString();
                    EXPECT_EQ(ECDbProfile::Get().GetExpectedVersion(), ecdb.GetECDbProfileVersion()) << testFile.ToString();
                    break;
                case ProfileState::Age::Newer:
                    EXPECT_TRUE(actualProfileState.IsNewer()) << testFile.ToString();
                    EXPECT_LT(ECDbProfile::Get().GetExpectedVersion(), ecdb.GetECDbProfileVersion()) << testFile.ToString();
                    break;

                default:
                    FAIL() << "unhandled enum value";
            }

        TestHelper helper(testFile, ecdb);
        helper.AssertLoadSchemas();

        helper.AssertKindOfQuantity("PreEC32Koqs", "ANGLE", "Angle", nullptr, "u:RAD", JsonValue(R"json(["f:DefaultRealU(2)[u:ARC_DEG]", "f:DefaultReal[u:RAD]"])json"), 0.0001);
        helper.AssertKindOfQuantity("PreEC32Koqs", "POWER", "Power", nullptr, "u:W", JsonValue(R"json(["f:DefaultRealU(4)[u:W]", "f:DefaultRealU(4)[u:KW]", "f:DefaultRealU(4)[u:MEGAW]", "f:DefaultRealU(4)[u:BTU_PER_HR]", "f:DefaultRealU(4)[u:KILOBTU_PER_HR]", "f:DefaultRealU(4)[u:HP]"])json"), 0.001);
        helper.AssertKindOfQuantity("PreEC32Koqs", "LIQUID_VOLUME", "Liquid Volume", nullptr, "u:CUB_M", JsonValue(R"json(["f:DefaultRealU(4)[u:LITRE]", "f:DefaultRealU(4)[u:GALLON]"])json"), 0.0001);
        helper.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_NoPresUnit", nullptr, nullptr, "u:W_PER_M_K", JsonValue(), 0.4);
        helper.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormat_NoPresUnit", nullptr, nullptr, "u:W_PER_M_K", JsonValue(R"json(["f:DefaultReal[u:W_PER_M_K]"])json"), 0.5);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32KindOfQuantities)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32KOQS))
        {
        ProfileState::Age originalFileAge = testFile.GetAge();

        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile)) << testFile.ToString();

        ProfileState actualProfileState = ecdb.CheckProfileVersion();
        switch (originalFileAge)
            {
                case ProfileState::Age::Older:
                    EXPECT_TRUE(actualProfileState.IsUpToDate()) << "File is expected to be auto-upgraded. " << testFile.ToString();
                    EXPECT_EQ(ECDbProfile::Get().GetExpectedVersion(), ecdb.GetECDbProfileVersion()) << "File is expected to be auto-upgraded. " << testFile.ToString();
                    break;
                case ProfileState::Age::UpToDate:
                    EXPECT_TRUE(actualProfileState.IsUpToDate()) << testFile.ToString();
                    EXPECT_EQ(ECDbProfile::Get().GetExpectedVersion(), ecdb.GetECDbProfileVersion()) << testFile.ToString();
                    break;
                case ProfileState::Age::Newer:
                    EXPECT_TRUE(actualProfileState.IsNewer()) << testFile.ToString();
                    EXPECT_LT(ECDbProfile::Get().GetExpectedVersion(), ecdb.GetECDbProfileVersion()) << testFile.ToString();
                    break;

                default:
                    FAIL() << "unhandled enum value";
            }

        TestHelper helper(testFile, ecdb);
        helper.AssertLoadSchemas();

        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithMandatoryComposite", "My first test KOQ", nullptr, "u:CM", JsonValue(R"js(["f:DefaultRealU(4)[u:M]"])js"), 0.1);
        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithOptionalComposite", nullptr, "My second test KOQ", "u:CM", JsonValue(R"js(["f:AmerFI[u:FT|feet][u:IN|inches]"])js"), 0.2);
        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithoutComposite", nullptr, nullptr, "u:CM", JsonValue(R"js(["f:AmerFI]"])js"), 0.3);
        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_NoPresFormat", nullptr, nullptr, "u:KG", JsonValue(), 0.4);
        }
    }
