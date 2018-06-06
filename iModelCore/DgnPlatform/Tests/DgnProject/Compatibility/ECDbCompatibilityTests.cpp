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
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile.GetPath())) << testFile.GetPath().GetNameUtf8();

        JsonValue schemaCountJson = TestHelper::ExecuteECSqlSelect(ecdb, "SELECT count(*) schemaCount FROM meta.ECSchemaDef");
        ASSERT_TRUE(schemaCountJson.m_value.isArray() && schemaCountJson.m_value.size() == 1 && schemaCountJson.m_value[0].isMember("schemaCount")) << schemaCountJson.ToString();
        const int schemaCount = schemaCountJson.m_value[0]["schemaCount"].asInt();

        switch (testFile.GetProfileState())
            {
                case ProfileState::Current:
                case ProfileState::Older:
                {
                EXPECT_EQ(5, schemaCount) << testFile.GetPath().GetNameUtf8();
                for (ECSchemaCP schema : ecdb.Schemas().GetSchemas(false))
                    {
                    //ECVersion not persisted by ECDb, so ECObjects defaults to 3.1
                    EXPECT_EQ((int) ECVersion::V3_1, (int) schema->GetECVersion()) << schema->GetFullSchemaName();
                    //OriginalECXML version not persisted in ECDb pre 4.0.0.2, so ECObjects defaults to 3.1
                    EXPECT_EQ(3, schema->GetOriginalECXmlVersionMajor()) << schema->GetFullSchemaName();
                    EXPECT_EQ(1, schema->GetOriginalECXmlVersionMinor()) << schema->GetFullSchemaName();
                    }

                //ECDb built-in schema versions
                EXPECT_EQ(SchemaVersion(2, 0, 0), TestHelper::GetSchemaVersion(ecdb, "ECDbFileInfo"));
                EXPECT_EQ(SchemaVersion(2, 0, 0), TestHelper::GetSchemaVersion(ecdb, "ECDbMap"));
                EXPECT_EQ(SchemaVersion(4, 0, 0), TestHelper::GetSchemaVersion(ecdb, "ECDbMeta"));
                EXPECT_EQ(SchemaVersion(5, 0, 0), TestHelper::GetSchemaVersion(ecdb, "ECDbSystem"));

                //Standard schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), TestHelper::GetSchemaVersion(ecdb, "CoreCustomAttributes"));
                break;
                }

                case ProfileState::Newer:
                {
                EXPECT_EQ(5, schemaCount) << testFile.GetPath().GetNameUtf8();

                for (ECSchemaCP schema : ecdb.Schemas().GetSchemas(false))
                    {
                    //ECVersion not read by ECDb, so ECObjects defaults to 3.1
                    EXPECT_EQ((int) ECVersion::V3_1, (int) schema->GetECVersion()) << schema->GetFullSchemaName();
                    //OriginalECXML version not read by ECDb, so ECObjects defaults to 3.1
                    EXPECT_EQ(3, schema->GetOriginalECXmlVersionMajor()) << schema->GetFullSchemaName();
                    EXPECT_EQ(1, schema->GetOriginalECXmlVersionMinor()) << schema->GetFullSchemaName();
                    }

                //ECDb built-in schema versions
                //ECDbFileInfo version was incremented in next profile, so must be higher in newer file
                EXPECT_LT(SchemaVersion(2, 0, 0), TestHelper::GetSchemaVersion(ecdb, "ECDbFileInfo"));
                EXPECT_LE(SchemaVersion(2, 0, 0), TestHelper::GetSchemaVersion(ecdb, "ECDbMap"));
                //ECDbMeta version was incremented in next profile, so must be higher in newer file
                EXPECT_LT(SchemaVersion(4, 0, 0), TestHelper::GetSchemaVersion(ecdb, "ECDbMeta"));
                //ECDbSystem version was incremented in next profile, so must be higher in newer file
                EXPECT_LT(SchemaVersion(5, 0, 0), TestHelper::GetSchemaVersion(ecdb, "ECDbSystem"));
                //Standard schema versions
                EXPECT_LE(SchemaVersion(1, 0, 0), TestHelper::GetSchemaVersion(ecdb, "CoreCustomAttributes"));

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
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile.GetPath())) << testFile.GetPath().GetNameUtf8();

        TestHelper::AssertEnum(ecdb, "CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                    {{ECValue("Unspecified"), nullptr},
                    {ECValue("Utc"), nullptr},
                    {ECValue("Local"), nullptr}});

        TestHelper::AssertEnum(ecdb, "ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                {{ECValue(0), "None"},
                {ECValue(1), "Abstract"},
                {ECValue(2), "Sealed"}});

        TestHelper::AssertEnum(ecdb, "PreEC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{ECValue(0), nullptr},
                {ECValue(1), nullptr},
                {ECValue(2), nullptr}});

        TestHelper::AssertEnum(ecdb, "PreEC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
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
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile.GetPath())) << testFile.GetPath().GetNameUtf8();

        TestHelper::AssertKindOfQuantity(ecdb, "PreEC32Koqs", "ANGLE", "Angle", nullptr, "RAD(DefaultReal)", JsonValue(R"json(["ARC_DEG(Real2U)", "ARC_DEG(AngleDMS)"])json"), 0.0001);
        TestHelper::AssertKindOfQuantity(ecdb, "PreEC32Koqs", "POWER", "Power", nullptr, "W(DefaultReal)", JsonValue(R"json(["W(Real4U)", "KW(Real4U)", "MEGAW(Real4U)", "BTU/HR(Real4U)", "KILOBTU/HR(Real4U)", "HP(Real4U)"])json"), 0.001);
        TestHelper::AssertKindOfQuantity(ecdb, "PreEC32Koqs", "LIQUID_VOLUME", "Liquid Volume", nullptr, "CUB.M(DefaultReal)", JsonValue(R"json(["LITRE(Real4U)", "GALLON(Real4U)"])json"), 0.0001);
        TestHelper::AssertKindOfQuantity(ecdb, "PreEC32Koqs", "TestKoqWithoutPresUnits", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.5);
        }
    }
