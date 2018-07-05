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
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            switch (testDb.GetState())
                {
                    case TestDb::State::Older:
                    case TestDb::State::Upgraded:
                    case TestDb::State::UpToDate:
                    {
                    EXPECT_EQ(5, testDb.GetSchemaCount()) << testDb.GetDescription();

                    //ECDb built-in schema versions
                    EXPECT_EQ(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
                    EXPECT_EQ((int) ECVersion::V3_1, (int) testDb.GetECVersion("ECDbFileInfo")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), testDb.GetSchemaItemCounts("ECDbFileInfo")) << testDb.GetDescription();

                    EXPECT_EQ(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
                    EXPECT_EQ((int) ECVersion::V3_1, (int) testDb.GetECVersion("ECDbMap")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbMap")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), testDb.GetSchemaItemCounts("ECDbMap")) << testDb.GetDescription();

                    EXPECT_EQ(SchemaVersion(4, 0, 0), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
                    EXPECT_EQ((int) ECVersion::V3_1, (int) testDb.GetECVersion("ECDbMeta")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbMeta")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":24, "enumcount": 8})js"), testDb.GetSchemaItemCounts("ECDbMeta")) << testDb.GetDescription();

                    EXPECT_EQ(SchemaVersion(5, 0, 0), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
                    EXPECT_EQ((int) ECVersion::V3_1, (int) testDb.GetECVersion("ECDbSystem")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbSystem")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), testDb.GetSchemaItemCounts("ECDbSystem")) << testDb.GetDescription();

                    //Standard schema versions
                    EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
                    EXPECT_EQ((int) ECVersion::V3_1, (int) testDb.GetECVersion("CoreCustomAttributes")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("CoreCustomAttributes")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), testDb.GetSchemaItemCounts("CoreCustomAttributes")) << testDb.GetDescription();
                    break;
                    }

                    case TestDb::State::Newer:
                    {
                    EXPECT_LE(5, testDb.GetSchemaCount()) << testDb.GetDescription();

                    //ECDb built-in schema versions
                    //ECDbFileInfo version was incremented in next profile, so must be higher in newer file
                    EXPECT_LT(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
                    EXPECT_LE((int) ECVersion::V3_1, (int) testDb.GetECVersion("ECDbFileInfo")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << testDb.GetDescription();

                    EXPECT_LE(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
                    EXPECT_LE((int) ECVersion::V3_1, (int) testDb.GetECVersion("ECDbMap")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbMap")) << testDb.GetDescription();

                    //ECDbMeta version was incremented in next profile, so must be higher in newer file
                    EXPECT_LT(SchemaVersion(4, 0, 0), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
                    EXPECT_LE((int) ECVersion::V3_1, (int) testDb.GetECVersion("ECDbMeta")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbMeta")) << testDb.GetDescription();

                    //ECDbSystem version was incremented in next profile, so must be higher in newer file
                    EXPECT_LT(SchemaVersion(5, 0, 0), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
                    EXPECT_LE((int) ECVersion::V3_1, (int) testDb.GetECVersion("ECDbSystem")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbSystem")) << testDb.GetDescription();

                    //Standard schema versions
                    EXPECT_LE(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
                    EXPECT_LE((int) ECVersion::V3_1, (int) testDb.GetECVersion("CoreCustomAttributes")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("CoreCustomAttributes")) << testDb.GetDescription();
                    break;
                    }
                    default:
                        FAIL() << "Unhandled TestDb::State enum value | " << testDb.GetDescription();
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
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
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
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32Enums)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32ENUMS))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();

            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            ASSERT_TRUE(testDb.SupportsFeature(Feature::NamedEnumerators)) << testDb.GetDescription();

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
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            testDb.AssertKindOfQuantity("PreEC32Koqs", "ANGLE", "Angle", nullptr, "RAD(DefaultReal)", JsonValue(R"json(["ARC_DEG(Real2U)", "ARC_DEG(AngleDMS)"])json"), 0.0001);
            testDb.AssertKindOfQuantity("PreEC32Koqs", "POWER", "Power", nullptr, "W(DefaultReal)", JsonValue(R"json(["W(Real4U)", "KW(Real4U)", "MEGAW(Real4U)", "BTU/HR(Real4U)", "KILOBTU/HR(Real4U)", "HP(Real4U)"])json"), 0.001);
            testDb.AssertKindOfQuantity("PreEC32Koqs", "LIQUID_VOLUME", "Liquid Volume", nullptr, "CUB.M(DefaultReal)", JsonValue(R"json(["LITRE(Real4U)", "GALLON(Real4U)"])json"), 0.0001);
            testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.4);
            testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormat_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.5);
            testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormatWithUnit_NoPresUnit", nullptr, nullptr, "FT(AmerFI8)", JsonValue(), 0.6);
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
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();

            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();
            ASSERT_TRUE(testDb.SupportsFeature(Feature::UnitsAndFormats)) << testDb.GetDescription();

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
TEST_F(ECDbCompatibilityTestFixture, EC32Units)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32UNITS))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
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
            }
        }
    }
