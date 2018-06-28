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
        switch (originalFileAge)
            {
                case ProfileState::Age::UpToDate:
                {
                TestECDb testDb(testFile, ECDb::OpenParams(ECDb::OpenMode::Readonly));
                ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
                testDb.AssertProfileVersion();
                testDb.AssertLoadSchemas();
                EXPECT_EQ(5, testDb.GetSchemaCount()) << testDb.GetDescription();
                for (ECSchemaCP schema : testDb.GetDb().Schemas().GetSchemas(false))
                    {
                    //ECVersion not persisted by ECDb, so ECObjects defaults to 3.1
                    EXPECT_EQ((int) ECVersion::V3_1, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << testDb.GetDescription();
                    //OriginalECXML version not persisted in ECDb pre 4.0.0.2, so ECObjects defaults to 3.1
                    EXPECT_EQ(3, schema->GetOriginalECXmlVersionMajor()) << schema->GetFullSchemaName() << " | " << testDb.GetDescription();
                    EXPECT_EQ(1, schema->GetOriginalECXmlVersionMinor()) << schema->GetFullSchemaName() << " | " << testDb.GetDescription();
                    }

                //ECDb built-in schema versions
                EXPECT_EQ(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
                EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), testDb.GetSchemaItemCounts("ECDbFileInfo")) << testDb.GetDescription();
                EXPECT_EQ(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
                EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), testDb.GetSchemaItemCounts("ECDbMap")) << testDb.GetDescription();
                EXPECT_EQ(SchemaVersion(4, 0, 0), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
                EXPECT_EQ(JsonValue(R"js({"classcount":24, "enumcount": 8})js"), testDb.GetSchemaItemCounts("ECDbMeta")) << testDb.GetDescription();
                EXPECT_EQ(SchemaVersion(5, 0, 0), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
                EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), testDb.GetSchemaItemCounts("ECDbSystem")) << testDb.GetDescription();

                //Standard schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
                EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), testDb.GetSchemaItemCounts("CoreCustomAttributes")) << testDb.GetDescription();
                break;
                }

                case ProfileState::Age::Older:
                {
                //first open file without upgrade
                TestECDb notUpgradedDb(testFile, ECDb::OpenParams(ECDb::OpenMode::Readonly));
                ASSERT_EQ(BE_SQLITE_OK, notUpgradedDb.Open()) << notUpgradedDb.GetDescription();
                const bool isUpgradable = notUpgradedDb.GetDb().CheckProfileVersion().IsUpgradable();
                notUpgradedDb.AssertProfileVersion();
                notUpgradedDb.AssertLoadSchemas();
                EXPECT_EQ(5, notUpgradedDb.GetSchemaCount()) << notUpgradedDb.GetDescription();
                for (ECSchemaCP schema : notUpgradedDb.GetDb().Schemas().GetSchemas(false))
                    {
                    //ECVersion not persisted by ECDb, so ECObjects defaults to 3.1
                    EXPECT_EQ((int) ECVersion::V3_1, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << notUpgradedDb.GetDescription();
                    //OriginalECXML version not persisted in ECDb pre 4.0.0.2, so ECObjects defaults to 3.1
                    EXPECT_EQ(3, schema->GetOriginalECXmlVersionMajor()) << schema->GetFullSchemaName() << " | " << notUpgradedDb.GetDescription();
                    EXPECT_EQ(1, schema->GetOriginalECXmlVersionMinor()) << schema->GetFullSchemaName() << " | " << notUpgradedDb.GetDescription();
                    }

                //ECDb built-in schema versions
                EXPECT_EQ(SchemaVersion(2, 0, 0), notUpgradedDb.GetSchemaVersion("ECDbFileInfo")) << notUpgradedDb.GetDescription();
                EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), notUpgradedDb.GetSchemaItemCounts("ECDbFileInfo")) << notUpgradedDb.GetDescription();
                EXPECT_EQ(SchemaVersion(2, 0, 0), notUpgradedDb.GetSchemaVersion("ECDbMap")) << notUpgradedDb.GetDescription();
                EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), notUpgradedDb.GetSchemaItemCounts("ECDbMap")) << notUpgradedDb.GetDescription();
                EXPECT_EQ(SchemaVersion(4, 0, 0), notUpgradedDb.GetSchemaVersion("ECDbMeta")) << notUpgradedDb.GetDescription();
                EXPECT_EQ(JsonValue(R"js({"classcount":24, "enumcount": 8})js"), notUpgradedDb.GetSchemaItemCounts("ECDbMeta")) << notUpgradedDb.GetDescription();
                EXPECT_EQ(SchemaVersion(5, 0, 0), notUpgradedDb.GetSchemaVersion("ECDbSystem")) << notUpgradedDb.GetDescription();
                EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), notUpgradedDb.GetSchemaItemCounts("ECDbSystem")) << notUpgradedDb.GetDescription();

                //Standard schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), notUpgradedDb.GetSchemaVersion("CoreCustomAttributes")) << notUpgradedDb.GetDescription();
                EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), notUpgradedDb.GetSchemaItemCounts("CoreCustomAttributes")) << notUpgradedDb.GetDescription();
                notUpgradedDb.Close();

                //now upgrade the file
                TestECDb upgradedDb(testFile, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::Upgrade));
                if (!isUpgradable)
                    {
                    ASSERT_NE(BE_SQLITE_OK, upgradedDb.Open()) << "File is not upgradable | " << upgradedDb.GetDescription();
                    break;
                    }

                ASSERT_EQ(BE_SQLITE_OK, upgradedDb.Open()) << upgradedDb.GetDescription();
                upgradedDb.AssertProfileVersion();
                upgradedDb.AssertLoadSchemas();
                EXPECT_EQ(5, upgradedDb.GetSchemaCount()) << upgradedDb.GetDescription();
                for (ECSchemaCP schema : upgradedDb.GetDb().Schemas().GetSchemas(false))
                    {
                    //ECVersion not persisted by ECDb, so ECObjects defaults to 3.1
                    EXPECT_EQ((int) ECVersion::V3_1, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << upgradedDb.GetDescription();
                    //OriginalECXML version not persisted in ECDb pre 4.0.0.2, so ECObjects defaults to 3.1
                    EXPECT_EQ(3, schema->GetOriginalECXmlVersionMajor()) << schema->GetFullSchemaName() << " | " << upgradedDb.GetDescription();
                    EXPECT_EQ(1, schema->GetOriginalECXmlVersionMinor()) << schema->GetFullSchemaName() << " | " << upgradedDb.GetDescription();
                    }

                //ECDb built-in schema versions
                EXPECT_EQ(SchemaVersion(2, 0, 1), upgradedDb.GetSchemaVersion("ECDbFileInfo")) << upgradedDb.GetDescription();
                EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), upgradedDb.GetSchemaItemCounts("ECDbFileInfo")) << upgradedDb.GetDescription();
                EXPECT_EQ(SchemaVersion(2, 0, 0), upgradedDb.GetSchemaVersion("ECDbMap")) << upgradedDb.GetDescription();
                EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), upgradedDb.GetSchemaItemCounts("ECDbMap")) << upgradedDb.GetDescription();
                EXPECT_EQ(SchemaVersion(4, 0, 1), upgradedDb.GetSchemaVersion("ECDbMeta")) << upgradedDb.GetDescription();
                EXPECT_EQ(JsonValue(R"js({"classcount":38, "enumcount": 9})js"), upgradedDb.GetSchemaItemCounts("ECDbMeta")) << upgradedDb.GetDescription();
                EXPECT_EQ(SchemaVersion(5, 0, 1), upgradedDb.GetSchemaVersion("ECDbSystem")) << upgradedDb.GetDescription();
                EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), upgradedDb.GetSchemaItemCounts("ECDbSystem")) << upgradedDb.GetDescription();

                //Standard schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), upgradedDb.GetSchemaVersion("CoreCustomAttributes")) << upgradedDb.GetDescription();
                EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), upgradedDb.GetSchemaItemCounts("CoreCustomAttributes")) << upgradedDb.GetDescription();
                break;
                }

                case ProfileState::Age::Newer:
                {
                TestECDb testDb(testFile, ECDb::OpenParams(ECDb::OpenMode::Readonly));
                ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
                testDb.AssertProfileVersion();
                testDb.AssertLoadSchemas();
                EXPECT_EQ(5, testDb.GetSchemaCount()) << testDb.GetDescription();

                for (ECSchemaCP schema : testDb.GetDb().Schemas().GetSchemas(false))
                    {
                    //ECVersion not read by ECDb, so ECObjects defaults to 3.1
                    EXPECT_EQ((int) ECVersion::V3_1, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << testDb.GetDescription();
                    //OriginalECXML version not read by ECDb, so ECObjects defaults to 3.1
                    EXPECT_EQ(3, schema->GetOriginalECXmlVersionMajor()) << schema->GetFullSchemaName() << " | " << testDb.GetDescription();
                    EXPECT_EQ(1, schema->GetOriginalECXmlVersionMinor()) << schema->GetFullSchemaName() << " | " << testDb.GetDescription();
                    }

                //ECDb built-in schema versions
                //ECDbFileInfo version was incremented in next profile, so must be higher in newer file
                EXPECT_LT(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
                EXPECT_LE(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
                //ECDbMeta version was incremented in next profile, so must be higher in newer file
                EXPECT_LT(SchemaVersion(4, 0, 0), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
                //ECDbSystem version was incremented in next profile, so must be higher in newer file
                EXPECT_LT(SchemaVersion(5, 0, 0), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
                //Standard schema versions
                EXPECT_LE(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
                break;
                }

                default:
                    FAIL() << "Unhandled ProfileState::Age enum value | " << testFile.ToString();
                    break;
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
        if (originalFileAge != ProfileState::Age::Older)
            {
            TestECDb testDb(testFile, ECDb::OpenParams(ECDb::OpenMode::Readonly));
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

            testDb.AssertEnum("PreEC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
            {{ECValue(0), nullptr},
            {ECValue(1), nullptr},
            {ECValue(2), nullptr}});

            testDb.AssertEnum("PreEC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
            {{ECValue("On"), "Turned On"},
            {ECValue("Off"), "Turned Off"}});
            continue;
            }

        BeAssert(originalFileAge == ProfileState::Age::Older);
        TestECDb notUpgradedDb(testFile, ECDb::OpenParams(ECDb::OpenMode::Readonly));
        ASSERT_EQ(BE_SQLITE_OK, notUpgradedDb.Open()) << notUpgradedDb.GetDescription();
        notUpgradedDb.AssertProfileVersion();
        notUpgradedDb.AssertLoadSchemas();
        const bool isUpgradable = notUpgradedDb.GetDb().CheckProfileVersion().IsUpgradable();

        notUpgradedDb.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                    {{ECValue("Unspecified"), nullptr},
                    {ECValue("Utc"), nullptr},
                    {ECValue("Local"), nullptr}});

        notUpgradedDb.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                {{ECValue(0), "None"},
                {ECValue(1), "Abstract"},
                {ECValue(2), "Sealed"}});

        notUpgradedDb.AssertEnum("PreEC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{ECValue(0), nullptr},
                {ECValue(1), nullptr},
                {ECValue(2), nullptr}});

        notUpgradedDb.AssertEnum("PreEC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                {{ECValue("On"), "Turned On"},
                {ECValue("Off"), "Turned Off"}});

        notUpgradedDb.Close();

        TestECDb upgradedDb(testFile, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::Upgrade));
        if (!isUpgradable)
            {
            ASSERT_NE(BE_SQLITE_OK, upgradedDb.Open()) << "File is not upgradable | " << upgradedDb.GetDescription();
            break;
            }

        ASSERT_EQ(BE_SQLITE_OK, upgradedDb.Open()) << upgradedDb.GetDescription();
        upgradedDb.AssertProfileVersion();
        upgradedDb.AssertLoadSchemas();

        upgradedDb.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
        {{ECValue("Unspecified"), nullptr},
        {ECValue("Utc"), nullptr},
        {ECValue("Local"), nullptr}});

        upgradedDb.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
        {{ECValue(0), "None"},
        {ECValue(1), "Abstract"},
        {ECValue(2), "Sealed"}});

        upgradedDb.AssertEnum("PreEC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
        {{ECValue(0), nullptr},
        {ECValue(1), nullptr},
        {ECValue(2), nullptr}});

        upgradedDb.AssertEnum("PreEC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
        {{ECValue("On"), "Turned On"},
        {ECValue("Off"), "Turned Off"}});
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32Enums)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32ENUMS))
        {
        switch (testFile.GetAge())
            {
                case ProfileState::Age::UpToDate:
                case ProfileState::Age::Newer:
                {
                TestECDb testDb(testFile);
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

                testDb.AssertEnum("EC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{ECValue(0), nullptr},
                {ECValue(1), nullptr},
                {ECValue(2), nullptr}});

                testDb.AssertEnum("EC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                {{ECValue("On"), "Turned On"},
                {ECValue("Off"), "Turned Off"}});
                break;
                }

                case ProfileState::Age::Older:
                {
                TestECDb notUpgradedDb(testFile);
                ASSERT_EQ(BE_SQLITE_OK, notUpgradedDb.Open()) << notUpgradedDb.GetDescription();
                const bool isUpgradable = notUpgradedDb.GetDb().CheckProfileVersion().IsUpgradable();
                notUpgradedDb.AssertProfileVersion();
                notUpgradedDb.AssertLoadSchemas();

                notUpgradedDb.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                {{ECValue("Unspecified"), nullptr},
                {ECValue("Utc"), nullptr},
                {ECValue("Local"), nullptr}});

                notUpgradedDb.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                {{ECValue(0), "None"},
                {ECValue(1), "Abstract"},
                {ECValue(2), "Sealed"}});

                notUpgradedDb.AssertEnum("EC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{ECValue(0), nullptr},
                {ECValue(1), nullptr},
                {ECValue(2), nullptr}});

                notUpgradedDb.AssertEnum("EC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                {{ECValue("On"), "Turned On"},
                { ECValue("Off"), "Turned Off"}});
                notUpgradedDb.Close();

                TestECDb upgradedDb(testFile, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::Upgrade));
                if (!isUpgradable)
                    {
                    ASSERT_NE(BE_SQLITE_OK, upgradedDb.Open()) << upgradedDb.GetDescription();
                    break;
                    }

                ASSERT_EQ(BE_SQLITE_OK, upgradedDb.Open()) << upgradedDb.GetDescription();
                upgradedDb.AssertProfileVersion();
                upgradedDb.AssertLoadSchemas();

                upgradedDb.AssertEnum("CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                {{ECValue("Unspecified"), nullptr},
                {ECValue("Utc"), nullptr},
                {ECValue("Local"), nullptr}});

                upgradedDb.AssertEnum("ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                {{ECValue(0), "None"},
                {ECValue(1), "Abstract"},
                {ECValue(2), "Sealed"}});

                upgradedDb.AssertEnum("EC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{ECValue(0), nullptr},
                {ECValue(1), nullptr},
                {ECValue(2), nullptr}});

                upgradedDb.AssertEnum("EC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
                {{ECValue("On"), "Turned On"},
                {ECValue("Off"), "Turned Off"}});

                break;
                }

                default:
                    FAIL() << "Unhandled ProfileState::Age enum value | " << testFile.ToString();
                    break;
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, PreEC32KindOfQuantities)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_PREEC32KOQS))
        {
        switch (testFile.GetAge())
            {
                case ProfileState::Age::UpToDate:
                case ProfileState::Age::Newer:
                {
                TestECDb testDb(testFile);
                ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
                testDb.AssertProfileVersion();
                testDb.AssertLoadSchemas();
                testDb.AssertKindOfQuantity("PreEC32Koqs", "ANGLE", "Angle", nullptr, "RAD(DefaultReal)", JsonValue(R"json(["ARC_DEG(Real2U)", "ARC_DEG(AngleDMS)"])json"), 0.0001);
                testDb.AssertKindOfQuantity("PreEC32Koqs", "POWER", "Power", nullptr, "W(DefaultReal)", JsonValue(R"json(["W(Real4U)", "KW(Real4U)", "MEGAW(Real4U)", "BTU/HR(Real4U)", "KILOBTU/HR(Real4U)", "HP(Real4U)"])json"), 0.001);
                testDb.AssertKindOfQuantity("PreEC32Koqs", "LIQUID_VOLUME", "Liquid Volume", nullptr, "CUB.M(DefaultReal)", JsonValue(R"json(["LITRE(Real4U)", "GALLON(Real4U)"])json"), 0.0001);
                testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.4);
                testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormat_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.5);
                testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormatWithUnit_NoPresUnit", nullptr, nullptr, "FT(AmerFI8)", JsonValue(), 0.6);
                break;
                }

                case ProfileState::Age::Older:
                {
                TestECDb notUpgradedDb(testFile);
                ASSERT_EQ(BE_SQLITE_OK, notUpgradedDb.Open()) << notUpgradedDb.GetDescription();
                const bool isUpgradable = notUpgradedDb.GetDb().CheckProfileVersion().IsUpgradable();
                notUpgradedDb.AssertProfileVersion();
                notUpgradedDb.AssertLoadSchemas();
                notUpgradedDb.AssertKindOfQuantity("PreEC32Koqs", "ANGLE", "Angle", nullptr, "RAD(DefaultReal)", JsonValue(R"json(["ARC_DEG(Real2U)", "ARC_DEG(AngleDMS)"])json"), 0.0001);
                notUpgradedDb.AssertKindOfQuantity("PreEC32Koqs", "POWER", "Power", nullptr, "W(DefaultReal)", JsonValue(R"json(["W(Real4U)", "KW(Real4U)", "MEGAW(Real4U)", "BTU/HR(Real4U)", "KILOBTU/HR(Real4U)", "HP(Real4U)"])json"), 0.001);
                notUpgradedDb.AssertKindOfQuantity("PreEC32Koqs", "LIQUID_VOLUME", "Liquid Volume", nullptr, "CUB.M(DefaultReal)", JsonValue(R"json(["LITRE(Real4U)", "GALLON(Real4U)"])json"), 0.0001);
                notUpgradedDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.4);
                notUpgradedDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormat_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.5);
                notUpgradedDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormatWithUnit_NoPresUnit", nullptr, nullptr, "FT(AmerFI8)", JsonValue(), 0.6);
                notUpgradedDb.Close();

                TestECDb upgradedDb(testFile, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::Upgrade));
                if (!isUpgradable)
                    {
                    ASSERT_NE(BE_SQLITE_OK, upgradedDb.Open()) << upgradedDb.GetDescription();
                    break;
                    }

                ASSERT_EQ(BE_SQLITE_OK, upgradedDb.Open()) << upgradedDb.GetDescription();
                upgradedDb.AssertProfileVersion();
                upgradedDb.AssertLoadSchemas();
                upgradedDb.AssertKindOfQuantity("PreEC32Koqs", "ANGLE", "Angle", nullptr, "RAD(DefaultReal)", JsonValue(R"json(["ARC_DEG(Real2U)", "ARC_DEG(AngleDMS)"])json"), 0.0001);
                upgradedDb.AssertKindOfQuantity("PreEC32Koqs", "POWER", "Power", nullptr, "W(DefaultReal)", JsonValue(R"json(["W(Real4U)", "KW(Real4U)", "MEGAW(Real4U)", "BTU/HR(Real4U)", "KILOBTU/HR(Real4U)", "HP(Real4U)"])json"), 0.001);
                upgradedDb.AssertKindOfQuantity("PreEC32Koqs", "LIQUID_VOLUME", "Liquid Volume", nullptr, "CUB.M(DefaultReal)", JsonValue(R"json(["LITRE(Real4U)", "GALLON(Real4U)"])json"), 0.0001);
                upgradedDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.4);
                upgradedDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormat_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.5);
                upgradedDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormatWithUnit_NoPresUnit", nullptr, nullptr, "FT(AmerFI8)", JsonValue(), 0.6);
                break;
                }
                default:
                    FAIL() << "Unhandled ProfileState::Age enum value | " << testFile.ToString();
                    break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32KindOfQuantities)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32KOQS))
        {
        switch (testFile.GetAge())
            {
                case ProfileState::Age::UpToDate:
                case ProfileState::Age::Newer:
                {
                TestECDb testDb(testFile);
                ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
                testDb.AssertProfileVersion();
                testDb.AssertLoadSchemas();

                testDb.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithMandatoryComposite", "My first test KOQ", nullptr, "u:CM", JsonValue(R"js(["f:DefaultRealU(4)[u:M]"])js"), 0.1);
                testDb.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithOptionalComposite", nullptr, "My second test KOQ", "u:CM", JsonValue(R"js(["f:AmerFI[u:FT|feet][u:IN|inches]"])js"), 0.2);
                testDb.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithoutComposite", nullptr, nullptr, "u:CM", JsonValue(R"js(["f:AmerFI"])js"), 0.3);
                testDb.AssertKindOfQuantity("EC32Koqs", "TestKoq_NoPresFormat", nullptr, nullptr, "u:KG", JsonValue(), 0.4);
                break;
                }
                case ProfileState::Age::Older:
                {
                TestECDb notUpgradedDb(testFile);
                ASSERT_EQ(BE_SQLITE_OK, notUpgradedDb.Open()) << notUpgradedDb.GetDescription();
                const bool isUpgradable = notUpgradedDb.GetDb().CheckProfileVersion().IsUpgradable();
                notUpgradedDb.AssertProfileVersion();
                notUpgradedDb.AssertLoadSchemas();
                notUpgradedDb.AssertKindOfQuantity("PreEC32Koqs", "ANGLE", "Angle", nullptr, "u:RAD", JsonValue(R"json(["f:DefaultRealU(2)[u:ARC_DEG]", "f:AngleDMS"])json"), 0.0001);
                notUpgradedDb.AssertKindOfQuantity("PreEC32Koqs", "POWER", "Power", nullptr, "u:W", JsonValue(R"json(["f:DefaultRealU(4)[u:W]", "f:DefaultRealU(4)[u:KW]", "f:DefaultRealU(4)[u:MEGAW]", "f:DefaultRealU(4)[u:BTU_PER_HR]", "f:DefaultRealU(4)[u:KILOBTU_PER_HR]", "f:DefaultRealU(4)[u:HP]"])json"), 0.001);
                notUpgradedDb.AssertKindOfQuantity("PreEC32Koqs", "LIQUID_VOLUME", "Liquid Volume", nullptr, "u:CUB_M", JsonValue(R"json(["f:DefaultRealU(4)[u:LITRE]", "f:DefaultRealU(4)[u:GALLON]"])json"), 0.0001);
                notUpgradedDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_NoPresUnit", nullptr, nullptr, "u:W_PER_M_K", JsonValue(), 0.4);
                notUpgradedDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormat_NoPresUnit", nullptr, nullptr, "u:W_PER_M_K", JsonValue(R"json(["f:DefaultReal[u:W_PER_M_K]"])json"), 0.5);
                notUpgradedDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormatWithUnit_NoPresUnit", nullptr, nullptr, "u:FT", JsonValue(R"json(["f:AmerFI"])json"), 0.6);
                notUpgradedDb.Close();

                TestECDb upgradedDb(testFile, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::Upgrade));
                if (!isUpgradable)
                    {
                    ASSERT_NE(BE_SQLITE_OK, upgradedDb.Open()) << upgradedDb.GetDescription();
                    break;
                    }

                ASSERT_EQ(BE_SQLITE_OK, upgradedDb.Open()) << upgradedDb.GetDescription();
                upgradedDb.AssertProfileVersion();
                upgradedDb.AssertLoadSchemas();
                upgradedDb.AssertKindOfQuantity("PreEC32Koqs", "ANGLE", "Angle", nullptr, "u:RAD", JsonValue(R"json(["f:DefaultRealU(2)[u:ARC_DEG]", "f:AngleDMS"])json"), 0.0001);
                upgradedDb.AssertKindOfQuantity("PreEC32Koqs", "POWER", "Power", nullptr, "u:W", JsonValue(R"json(["f:DefaultRealU(4)[u:W]", "f:DefaultRealU(4)[u:KW]", "f:DefaultRealU(4)[u:MEGAW]", "f:DefaultRealU(4)[u:BTU_PER_HR]", "f:DefaultRealU(4)[u:KILOBTU_PER_HR]", "f:DefaultRealU(4)[u:HP]"])json"), 0.001);
                upgradedDb.AssertKindOfQuantity("PreEC32Koqs", "LIQUID_VOLUME", "Liquid Volume", nullptr, "u:CUB_M", JsonValue(R"json(["f:DefaultRealU(4)[u:LITRE]", "f:DefaultRealU(4)[u:GALLON]"])json"), 0.0001);
                upgradedDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_NoPresUnit", nullptr, nullptr, "u:W_PER_M_K", JsonValue(), 0.4);
                upgradedDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormat_NoPresUnit", nullptr, nullptr, "u:W_PER_M_K", JsonValue(R"json(["f:DefaultReal[u:W_PER_M_K]"])json"), 0.5);
                upgradedDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormatWithUnit_NoPresUnit", nullptr, nullptr, "u:FT", JsonValue(R"json(["f:AmerFI"])json"), 0.6);
                break;
                }

                default:
                    FAIL() << "Unhandled ProfileState::Age enum value | " << testFile.ToString();
                    break;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32Units)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32UNITS))
        {
        switch (testFile.GetAge())
            {
                case ProfileState::Age::UpToDate:
                case ProfileState::Age::Newer:
                {
                TestECDb testDb(testFile);
                ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
                testDb.AssertProfileVersion();
                testDb.AssertLoadSchemas();

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
                break;
                }

                case ProfileState::Age::Older:
                {
                TestECDb notUpgradedDb(testFile);
                ASSERT_EQ(BE_SQLITE_OK, notUpgradedDb.Open()) << notUpgradedDb.GetDescription();
                const bool isUpgradable = notUpgradedDb.GetDb().CheckProfileVersion().IsUpgradable();
                notUpgradedDb.AssertProfileVersion();
                notUpgradedDb.AssertLoadSchemas();

                notUpgradedDb.AssertKindOfQuantity("EC32Units", "KoqWithCustomFormat", nullptr, nullptr, "u:M", JsonValue(R"js(["MyFormat[u:M]"])js"), 0.1);
                notUpgradedDb.AssertKindOfQuantity("EC32Units", "KoqWithCustomUnit", nullptr, nullptr, "MySquareM", JsonValue(R"js(["f:DefaultRealU(4)[MySquareM]"])js"), 0.2);
                notUpgradedDb.AssertKindOfQuantity("EC32Units", "KoqWithCustomUnitAndFormat", nullptr, nullptr, "MySquareFt", JsonValue(R"js(["MyFormat[MySquareFt]"])js"), 0.3);

                notUpgradedDb.AssertUnitSystem("EC32Units", "MyMetric", "Metric", "Metric Units of measure");
                notUpgradedDb.AssertUnitSystem("EC32Units", "MyImperial", "Imperial", "Units of measure from the British Empire");
                notUpgradedDb.AssertUnitSystem("Units", "SI", nullptr, nullptr);
                notUpgradedDb.AssertUnitSystem("Units", "CONSTANT", nullptr, nullptr);

                notUpgradedDb.AssertPhenomenon("EC32Units", "MyArea", "Area", nullptr, "LENGTH*LENGTH");
                notUpgradedDb.AssertPhenomenon("Units", "AREA", "Area", nullptr, "LENGTH(2)");
                notUpgradedDb.AssertPhenomenon("Units", "TORQUE", "Torque", nullptr, "FORCE*LENGTH*ANGLE(-1)");
                notUpgradedDb.AssertPhenomenon("Units", "LUMINOSITY", "Luminosity", nullptr, "LUMINOSITY");

                notUpgradedDb.AssertUnit("EC32Units", "MySquareM", "Square Meter", nullptr, "M*M", 1.0, nullptr, nullptr, QualifiedName("EC32Units", "MyMetric"), QualifiedName("EC32Units", "MyArea"), false, QualifiedName());
                notUpgradedDb.AssertUnit("EC32Units", "MySquareFt", "Square Feet", nullptr, "Ft*Ft", 10.0, nullptr, 0.4, QualifiedName("EC32Units", "MyImperial"), QualifiedName("EC32Units", "MyArea"), false, QualifiedName());
                notUpgradedDb.AssertUnit("Units", "COULOMB", "C", nullptr, "A*S", nullptr, nullptr, nullptr, QualifiedName("Units", "SI"), QualifiedName("Units", "ELECTRIC_CHARGE"), false, QualifiedName());
                notUpgradedDb.AssertUnit("Units", "PI", "Pi", nullptr, "ONE", 3.1415926535897932384626433832795, nullptr, nullptr, QualifiedName(), QualifiedName("Units", "LENGTH_RATIO"), true, QualifiedName());
                notUpgradedDb.AssertUnit("Units", "QUARTER_PI", "Pi/4", nullptr, "PI", 1.0, 4.0, nullptr, QualifiedName(), QualifiedName("Units", "LENGTH_RATIO"), true, QualifiedName());
                notUpgradedDb.AssertUnit("Units", "MILLI", "milli", nullptr, "ONE", .001, nullptr, nullptr, QualifiedName(), QualifiedName("Units", "NUMBER"), true, QualifiedName());
                notUpgradedDb.AssertUnit("Units", "HORIZONTAL_PER_VERTICAL", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, QualifiedName("Units", "INTERNATIONAL"), QualifiedName("Units", "SLOPE"), false, QualifiedName("Units", "VERTICAL_PER_HORIZONTAL"));

                notUpgradedDb.AssertFormat("EC32Units", "MyFormat", "My Format", nullptr, JsonValue(R"json({"roundFactor":0.3, "type": "Fractional", "showSignOption": "OnlyNegative", "formatTraits": ["trailZeroes", "keepSingleZero"], "precision": 4, "decimalSeparator": ".", "thousandSeparator": ",", "uomSeparator": " "})json"), JsonValue());
                notUpgradedDb.AssertFormat("EC32Units", "MyFormatWithComposite", "My Format with composite", nullptr, JsonValue(R"json({"type": "Decimal", "formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"], "precision": 2})json"),
                                    JsonValue(R"json({"includeZero":true, "spacer":"-", "units": [{"name":"HR", "label":"hour"}, {"name":"MIN", "label":"min"}]})json"));
                notUpgradedDb.AssertFormat("Formats", "DefaultReal", "real", nullptr, JsonValue(R"json({"type": "Decimal", "formatTraits": ["keepSingleZero", "keepDecimalPoint"], "precision": 6})json"), JsonValue());
                notUpgradedDb.AssertFormat("Formats", "AmerFI", "FeetInches", nullptr, JsonValue(R"json({"type": "Fractional", "formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"], "precision": 8, "uomSeparator":""})json"),
                                    JsonValue(R"json({"includeZero":true, "spacer":"", "units": [{"name":"FT", "label":"'"}, {"name":"IN", "label":"\""}]})json"));

                notUpgradedDb.Close();

                TestECDb upgradedDb(testFile, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::Upgrade));
                if (!isUpgradable)
                    {
                    ASSERT_NE(BE_SQLITE_OK, upgradedDb.Open()) << upgradedDb.GetDescription();
                    break;
                    }

                ASSERT_EQ(BE_SQLITE_OK, upgradedDb.Open()) << upgradedDb.GetDescription();
                upgradedDb.AssertProfileVersion();
                upgradedDb.AssertLoadSchemas();
                upgradedDb.AssertKindOfQuantity("EC32Units", "KoqWithCustomFormat", nullptr, nullptr, "u:M", JsonValue(R"js(["MyFormat[u:M]"])js"), 0.1);
                upgradedDb.AssertKindOfQuantity("EC32Units", "KoqWithCustomUnit", nullptr, nullptr, "MySquareM", JsonValue(R"js(["f:DefaultRealU(4)[MySquareM]"])js"), 0.2);
                upgradedDb.AssertKindOfQuantity("EC32Units", "KoqWithCustomUnitAndFormat", nullptr, nullptr, "MySquareFt", JsonValue(R"js(["MyFormat[MySquareFt]"])js"), 0.3);

                upgradedDb.AssertUnitSystem("EC32Units", "MyMetric", "Metric", "Metric Units of measure");
                upgradedDb.AssertUnitSystem("EC32Units", "MyImperial", "Imperial", "Units of measure from the British Empire");
                upgradedDb.AssertUnitSystem("Units", "SI", nullptr, nullptr);
                upgradedDb.AssertUnitSystem("Units", "CONSTANT", nullptr, nullptr);

                upgradedDb.AssertPhenomenon("EC32Units", "MyArea", "Area", nullptr, "LENGTH*LENGTH");
                upgradedDb.AssertPhenomenon("Units", "AREA", "Area", nullptr, "LENGTH(2)");
                upgradedDb.AssertPhenomenon("Units", "TORQUE", "Torque", nullptr, "FORCE*LENGTH*ANGLE(-1)");
                upgradedDb.AssertPhenomenon("Units", "LUMINOSITY", "Luminosity", nullptr, "LUMINOSITY");

                upgradedDb.AssertUnit("EC32Units", "MySquareM", "Square Meter", nullptr, "M*M", 1.0, nullptr, nullptr, QualifiedName("EC32Units", "MyMetric"), QualifiedName("EC32Units", "MyArea"), false, QualifiedName());
                upgradedDb.AssertUnit("EC32Units", "MySquareFt", "Square Feet", nullptr, "Ft*Ft", 10.0, nullptr, 0.4, QualifiedName("EC32Units", "MyImperial"), QualifiedName("EC32Units", "MyArea"), false, QualifiedName());
                upgradedDb.AssertUnit("Units", "COULOMB", "C", nullptr, "A*S", nullptr, nullptr, nullptr, QualifiedName("Units", "SI"), QualifiedName("Units", "ELECTRIC_CHARGE"), false, QualifiedName());
                upgradedDb.AssertUnit("Units", "PI", "Pi", nullptr, "ONE", 3.1415926535897932384626433832795, nullptr, nullptr, QualifiedName(), QualifiedName("Units", "LENGTH_RATIO"), true, QualifiedName());
                upgradedDb.AssertUnit("Units", "QUARTER_PI", "Pi/4", nullptr, "PI", 1.0, 4.0, nullptr, QualifiedName(), QualifiedName("Units", "LENGTH_RATIO"), true, QualifiedName());
                upgradedDb.AssertUnit("Units", "MILLI", "milli", nullptr, "ONE", .001, nullptr, nullptr, QualifiedName(), QualifiedName("Units", "NUMBER"), true, QualifiedName());
                upgradedDb.AssertUnit("Units", "HORIZONTAL_PER_VERTICAL", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, QualifiedName("Units", "INTERNATIONAL"), QualifiedName("Units", "SLOPE"), false, QualifiedName("Units", "VERTICAL_PER_HORIZONTAL"));

                upgradedDb.AssertFormat("EC32Units", "MyFormat", "My Format", nullptr, JsonValue(R"json({"roundFactor":0.3, "type": "Fractional", "showSignOption": "OnlyNegative", "formatTraits": ["trailZeroes", "keepSingleZero"], "precision": 4, "decimalSeparator": ".", "thousandSeparator": ",", "uomSeparator": " "})json"), JsonValue());
                upgradedDb.AssertFormat("EC32Units", "MyFormatWithComposite", "My Format with composite", nullptr, JsonValue(R"json({"type": "Decimal", "formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"], "precision": 2})json"),
                                           JsonValue(R"json({"includeZero":true, "spacer":"-", "units": [{"name":"HR", "label":"hour"}, {"name":"MIN", "label":"min"}]})json"));
                upgradedDb.AssertFormat("Formats", "DefaultReal", "real", nullptr, JsonValue(R"json({"type": "Decimal", "formatTraits": ["keepSingleZero", "keepDecimalPoint"], "precision": 6})json"), JsonValue());
                upgradedDb.AssertFormat("Formats", "AmerFI", "FeetInches", nullptr, JsonValue(R"json({"type": "Fractional", "formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"], "precision": 8, "uomSeparator":""})json"),
                                           JsonValue(R"json({"includeZero":true, "spacer":"", "units": [{"name":"FT", "label":"'"}, {"name":"IN", "label":"\""}]})json"));

                break;
                }
                default:
                    FAIL() << "Unhandled ProfileState::Age enum value | " << testFile.ToString();
                    break;
            }
        }
    }
