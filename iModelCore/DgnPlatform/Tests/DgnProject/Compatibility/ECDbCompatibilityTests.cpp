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
        DbResult OpenTestFile(ECDb& ecdb, BeFileNameCR path) { return ecdb.OpenBeSQLiteDb(path, ECDb::OpenParams(ECDb::OpenMode::Readonly)); }

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
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile.GetPath())) << testFile.ToString();

        TestHelper helper(testFile, ecdb);
        helper.AssertLoadSchemas();
        const int schemaCount = helper.GetSchemaCount();

        switch (testFile.GetProfileState())
            {
                case ProfileState::Current:
                {
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

                case ProfileState::Older:
                {
                EXPECT_EQ(Profile().GetExpectedVersion(), Profile().ReadProfileVersion(ecdb)) << "File is expected to be auto-upgraded. " << testFile.ToString();

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

                case ProfileState::Newer:
                {
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
    for (TestFile const& testFile : Profile().GetAllVersionsOfTestFile(TESTECDB_PREEC32ENUMS))
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile.GetPath())) << testFile.ToString();

        if (testFile.GetProfileState() == ProfileState::Older)
            EXPECT_EQ(Profile().GetExpectedVersion(), Profile().ReadProfileVersion(ecdb)) << "File is expected to be auto-upgraded | " << testFile.ToString();

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
    for (TestFile const& testFile : Profile().GetAllVersionsOfTestFile(TESTECDB_EC32ENUMS))
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile.GetPath())) << testFile.ToString();

        if (testFile.GetProfileState() == ProfileState::Older)
            EXPECT_EQ(Profile().GetExpectedVersion(), Profile().ReadProfileVersion(ecdb)) << "File is expected to be auto-upgraded | " << testFile.ToString();

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
    for (TestFile const& testFile : Profile().GetAllVersionsOfTestFile(TESTECDB_PREEC32KOQS))
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile.GetPath())) << testFile.ToString();

        if (testFile.GetProfileState() == ProfileState::Older)
            EXPECT_EQ(Profile().GetExpectedVersion(), Profile().ReadProfileVersion(ecdb)) << "File is expected to be auto-upgraded | " << testFile.ToString();

        TestHelper helper(testFile, ecdb);
        helper.AssertLoadSchemas();

        helper.AssertKindOfQuantity("PreEC32Koqs", "ANGLE", "Angle", nullptr, "u:RAD", JsonValue(R"json(["f:DefaultRealU(2)[u:ARC_DEG]", "f:AngleDMS"])json"), 0.0001);
        helper.AssertKindOfQuantity("PreEC32Koqs", "POWER", "Power", nullptr, "u:W", JsonValue(R"json(["f:DefaultRealU(4)[u:W]", "f:DefaultRealU(4)[u:KW]", "f:DefaultRealU(4)[u:MEGAW]", "f:DefaultRealU(4)[u:BTU_PER_HR]", "f:DefaultRealU(4)[u:KILOBTU_PER_HR]", "f:DefaultRealU(4)[u:HP]"])json"), 0.001);
        helper.AssertKindOfQuantity("PreEC32Koqs", "LIQUID_VOLUME", "Liquid Volume", nullptr, "u:CUB_M", JsonValue(R"json(["f:DefaultRealU(4)[u:LITRE]", "f:DefaultRealU(4)[u:GALLON]"])json"), 0.0001);
        helper.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_NoPresUnit", nullptr, nullptr, "u:W_PER_M_K", JsonValue(), 0.4);
        helper.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormat_NoPresUnit", nullptr, nullptr, "u:W_PER_M_K", JsonValue(), 0.5);
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
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile.GetPath())) << testFile.ToString();

        if (testFile.GetProfileState() == ProfileState::Older)
            EXPECT_EQ(Profile().GetExpectedVersion(), Profile().ReadProfileVersion(ecdb)) << "File is expected to be auto-upgraded | " << testFile.ToString();

        TestHelper helper(testFile, ecdb);
        helper.AssertLoadSchemas();

        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatNoComposite", nullptr, "KOQ with presentation formats without composite units", "u:CM", JsonValue(R"json(["f:DefaultRealU", "f:DefaultReal"])json"), 0.5);
        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithComposite", nullptr, nullptr, "u:KG", JsonValue(R"js(["f:DefaultRealU[u:G]"])js"), 0.6);
        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_NoPresFormat", nullptr, nullptr, "u:KG", JsonValue(), 0.5);
        }
    }
