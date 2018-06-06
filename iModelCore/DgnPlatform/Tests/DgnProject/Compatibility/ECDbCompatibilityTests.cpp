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
TEST_F(ECDbCompatibilityTestFixture, PreEC32Enums)
    {
    for (TestFile const& testFile : Profile().GetAllVersionsOfTestFile(TESTECDB_PREEC32ENUMS))
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile.GetPath())) << testFile.GetPath().GetNameUtf8();

        TestHelper::AssertEnum(ecdb, "CoreCustomAttributes", "DateTimeKind", nullptr, nullptr, PRIMITIVETYPE_String, true,
                    {{"Unspecified", ECValue("Unspecified"), nullptr},
                    {"Utc", ECValue("Utc"), nullptr},
                    {"Local", ECValue("Local"), nullptr}});

        TestHelper::AssertEnum(ecdb, "ECDbMeta", "ECClassModifier", nullptr, nullptr, PRIMITIVETYPE_Integer, true,
                {{"None", ECValue(0), "None"},
                {"Abstract", ECValue(1), "Abstract"},
                {"Sealed", ECValue(2), "Sealed"}});

        TestHelper::AssertEnum(ecdb, "PreEC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                {{"IntEnum_EnumeratorsWithoutDisplayLabel0", ECValue(0), nullptr},
                {"IntEnum_EnumeratorsWithoutDisplayLabel1", ECValue(1), nullptr},
                {"IntEnum_EnumeratorsWithoutDisplayLabel2", ECValue(2), nullptr}});

        TestHelper::AssertEnum(ecdb, "PreEC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
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
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile.GetPath())) << testFile.GetPath().GetNameUtf8();

        TestHelper::AssertKindOfQuantity(ecdb, "PreEC32Koqs", "ANGLE", "Angle", nullptr, "RAD(DefaultReal)", JsonValue(R"json(["ARC_DEG(Real2U)", "ARC_DEG(AngleDMS)"])json"), 0.0001);
        TestHelper::AssertKindOfQuantity(ecdb, "PreEC32Koqs", "POWER", "Power", nullptr, "W(DefaultReal)", JsonValue(R"json(["W(Real4U)", "KW(Real4U)", "MEGAW(Real4U)", "BTU/HR(Real4U)", "KILOBTU/HR(Real4U)", "HP(Real4U)"])json"), 0.001);
        TestHelper::AssertKindOfQuantity(ecdb, "PreEC32Koqs", "LIQUID_VOLUME", "Liquid Volume", nullptr, "CUB.M(DefaultReal)", JsonValue(R"json(["LITRE(Real4U)", "GALLON(Real4U)"])json"), 0.0001);
        TestHelper::AssertKindOfQuantity(ecdb, "PreEC32Koqs", "TestKoqWithoutPresUnits", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.5);
        }
    }
