/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/ECDbCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"
#include "ProfileManager.h"
#include "TestECDbCreators.h"
#include "TestHelper.h"

USING_NAMESPACE_BENTLEY_EC

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct ECDbCompatibilityTestFixture : CompatibilityTestFixture
    {
    protected:
        Profile& Profile() const { return ProfileManager::Get().GetProfile(ProfileType::ECDb); }
        DbResult OpenTestFile(ECDb& ecdb, TestFile const& testFile) { return ecdb.OpenBeSQLiteDb(testFile.GetPath(), ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::Upgrade)); }

        void SetUp() override { ASSERT_EQ(SUCCESS, TestECDbCreation::Run()); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, BuiltinSchemaVersions)
    {
    for (TestFile const& testFile : Profile().GetAllVersionsOfTestFile(TESTECDB_EMPTY))
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile)) << testFile.ToString();

        ProfileState profileState = ecdb.CheckProfileVersion();

        TestHelper helper(testFile, ecdb);
        helper.AssertLoadSchemas();
        const int schemaCount = helper.GetSchemaCount();

        switch (profileState.GetAge())
            {
                case ProfileState::Age::UpToDate:
                case ProfileState::Age::Older:
                {
                EXPECT_EQ(5, schemaCount) << testFile.ToString();
                for (ECSchemaCP schema : ecdb.Schemas().GetSchemas(false))
                    {
                    //ECVersion not persisted by ECDb, so ECObjects defaults to 3.1
                    EXPECT_EQ((int) ECVersion::V3_1, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    //OriginalECXML version not persisted in ECDb pre 4.0.0.2, so ECObjects defaults to 3.1
                    EXPECT_EQ(3, schema->GetOriginalECXmlVersionMajor()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    EXPECT_EQ(1, schema->GetOriginalECXmlVersionMinor()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    }

                //ECDb built-in schema versions
                EXPECT_EQ(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), helper.GetSchemaItemCounts("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), helper.GetSchemaItemCounts("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(4, 0, 0), helper.GetSchemaVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":24, "enumcount": 8})js"), helper.GetSchemaItemCounts("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(5, 0, 0), helper.GetSchemaVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), helper.GetSchemaItemCounts("ECDbSystem")) << testFile.ToString();

                //Standard schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), helper.GetSchemaItemCounts("CoreCustomAttributes")) << testFile.ToString();
                break;
                }

                case ProfileState::Age::Newer:
                {
                EXPECT_EQ(5, schemaCount) << testFile.ToString();

                for (ECSchemaCP schema : ecdb.Schemas().GetSchemas(false))
                    {
                    //ECVersion not read by ECDb, so ECObjects defaults to 3.1
                    EXPECT_EQ((int) ECVersion::V3_1, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    //OriginalECXML version not read by ECDb, so ECObjects defaults to 3.1
                    EXPECT_EQ(3, schema->GetOriginalECXmlVersionMajor()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    EXPECT_EQ(1, schema->GetOriginalECXmlVersionMinor()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    }

                //ECDb built-in schema versions
                //ECDbFileInfo version was incremented in next profile, so must be higher in newer file
                EXPECT_LT(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbMap")) << testFile.ToString();
                //ECDbMeta version was incremented in next profile, so must be higher in newer file
                EXPECT_LT(SchemaVersion(4, 0, 0), helper.GetSchemaVersion("ECDbMeta")) << testFile.ToString();
                //ECDbSystem version was incremented in next profile, so must be higher in newer file
                EXPECT_LT(SchemaVersion(5, 0, 0), helper.GetSchemaVersion("ECDbSystem")) << testFile.ToString();
                //Standard schema versions
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("CoreCustomAttributes")) << testFile.ToString();

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
    for (TestFile const& testFile : Profile().GetAllVersionsOfTestFile(TESTECDB_PREEC32ENUMS))
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile)) << testFile.ToString();

        TestHelper helper(testFile, ecdb);
        helper.AssertLoadSchemas();

        helper.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                    {{ECValue("Unspecified"), nullptr},
                    {ECValue("Utc"), nullptr},
                    {ECValue("Local"), nullptr}});

        helper.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                {{ECValue(0), "None"},
                {ECValue(1), "Abstract"},
                {ECValue(2), "Sealed"}});

        helper.AssertEnum("PreEC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{ECValue(0), nullptr},
                {ECValue(1), nullptr},
                {ECValue(2), nullptr}});

        helper.AssertEnum("PreEC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                {{ECValue("On"), "Turned On"},
                {ECValue("Off"), "Turned Off"}});
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32Enums)
    {
    for (TestFile const& testFile : Profile().GetAllVersionsOfTestFile(TESTECDB_EC32ENUMS))
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile)) << testFile.ToString();

        TestHelper helper(testFile, ecdb);
        helper.AssertLoadSchemas();

        helper.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
            {{ECValue("Unspecified"), nullptr},
            {ECValue("Utc"), nullptr},
            {ECValue("Local"), nullptr}});

        helper.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
            {{ECValue(0), "None"},
            {ECValue(1), "Abstract"},
            {ECValue(2), "Sealed"}});

        helper.AssertEnum("EC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
            {{ECValue(0), nullptr},
            {ECValue(1), nullptr},
            {ECValue(2), nullptr}});

        helper.AssertEnum("EC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
            {{ECValue("On"), "Turned On"},
            {ECValue("Off"), "Turned Off"}});
        }

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, PreEC32KindOfQuantities)
    {
    for (TestFile const& testFile : Profile().GetAllVersionsOfTestFile(TESTECDB_PREEC32KOQS))
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile)) << testFile.ToString();

        TestHelper helper(testFile, ecdb);
        helper.AssertLoadSchemas();

        helper.AssertKindOfQuantity("PreEC32Koqs", "ANGLE", "Angle", nullptr, "RAD(DefaultReal)", JsonValue(R"json(["ARC_DEG(Real2U)", "ARC_DEG(AngleDMS)"])json"), 0.0001);
        helper.AssertKindOfQuantity("PreEC32Koqs", "POWER", "Power", nullptr, "W(DefaultReal)", JsonValue(R"json(["W(Real4U)", "KW(Real4U)", "MEGAW(Real4U)", "BTU/HR(Real4U)", "KILOBTU/HR(Real4U)", "HP(Real4U)"])json"), 0.001);
        helper.AssertKindOfQuantity("PreEC32Koqs", "LIQUID_VOLUME", "Liquid Volume", nullptr, "CUB.M(DefaultReal)", JsonValue(R"json(["LITRE(Real4U)", "GALLON(Real4U)"])json"), 0.0001);
        helper.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.4);
        helper.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormat_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.5);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32KindOfQuantities)
    {
    for (TestFile const& testFile : Profile().GetAllVersionsOfTestFile(TESTECDB_EC32KOQS))
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile)) << testFile.ToString();

        TestHelper helper(testFile, ecdb);
        helper.AssertLoadSchemas();

        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatNoComposite", nullptr, "KOQ with presentation formats without composite units", "u:CM", JsonValue(R"json(["f:DefaultRealU", "f:DefaultReal"])json"), 0.5);
        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithComposite", nullptr, nullptr, "u:KG", JsonValue(R"js(["f:DefaultRealU[u:G]"])js"), 0.6);
        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_NoPresFormat", nullptr, nullptr, "u:KG", JsonValue(), 0.5);
        }
    }