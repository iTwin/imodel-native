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
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_PREEC32ENUMS))
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
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32ENUMS))
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
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_PREEC32KOQS))
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
        helper.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormatWithUnit_NoPresUnit", nullptr, nullptr, "FT(AmerFI8)", JsonValue(), 0.6);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32KindOfQuantities)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32KOQS))
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, OpenTestFile(ecdb, testFile)) << testFile.ToString();

        TestHelper helper(testFile, ecdb);
        helper.AssertLoadSchemas();

        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithMandatoryComposite", "My first test KOQ", nullptr, "u:CM", JsonValue(R"js(["TBD"])js"), 0.1);
        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithOptionalComposite", nullptr, "My second test KOQ", "u:CM", JsonValue(R"js(["TBD"])js"), 0.2);
        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithoutComposite", nullptr, nullptr, "u:CM", JsonValue(R"js(["TBD"])js"), 0.3);
        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_NoPresFormat", nullptr, nullptr, "u:KG", JsonValue(), 0.4);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, EC32Units)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EC32UNITS))
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

        helper.AssertKindOfQuantity("EC32Units", "KoqWithCustomFormat", nullptr, nullptr, "u:M", JsonValue(R"js(["MyFormat[u:M]"])js"), 0.1);
        helper.AssertKindOfQuantity("EC32Units", "KoqWithCustomUnit", nullptr, nullptr, "MySquareM", JsonValue(R"js(["f:DefaultRealU(4)[MySquareM]"])js"), 0.2);
        helper.AssertKindOfQuantity("EC32Units", "KoqWithCustomUnitAndFormat", nullptr, nullptr, "MySquareFt", JsonValue(R"js(["MyFormat[MySquareFt]"])js"), 0.3);

        helper.AssertUnitSystem("EC32Units", "MyMetric", "Metric", "Metric Units of measure");
        helper.AssertUnitSystem("EC32Units", "MyImperial", "Imperial", "Units of measure from the British Empire");
        helper.AssertUnitSystem("Units", "SI", nullptr, nullptr);
        helper.AssertUnitSystem("Units", "CONSTANT", nullptr, nullptr);

        helper.AssertPhenomenon("EC32Units", "MyArea", "Area", nullptr, "LENGTH*LENGTH");
        helper.AssertPhenomenon("Units", "AREA", "Area", nullptr, "LENGTH(2)");
        helper.AssertPhenomenon("Units", "TORQUE", "Torque", nullptr, "FORCE*LENGTH*ANGLE(-1)");
        helper.AssertPhenomenon("Units", "LUMINOSITY", "Luminosity", nullptr, "LUMINOSITY");

        helper.AssertUnit("EC32Units", "MySquareM", "Square Meter", nullptr, "M*M", 1.0, nullptr, nullptr, QualifiedName("EC32Units", "MyMetric"), QualifiedName("EC32Units", "MyArea"), false, QualifiedName());
        helper.AssertUnit("EC32Units", "MySquareFt", "Square Feet", nullptr, "Ft*Ft", 10.0, nullptr, 0.4, QualifiedName("EC32Units", "MyImperial"), QualifiedName("EC32Units", "MyArea"), false, QualifiedName());
        helper.AssertUnit("Units", "COULOMB", "C", nullptr, "A*S", nullptr, nullptr, nullptr, QualifiedName("Units", "SI"), QualifiedName("Units", "ELECTRIC_CHARGE"), false, QualifiedName());
        helper.AssertUnit("Units", "PI", "Pi", nullptr, "ONE", 3.1415926535897932384626433832795, nullptr, nullptr, QualifiedName(), QualifiedName("Units", "LENGTH_RATIO"), true, QualifiedName());
        helper.AssertUnit("Units", "QUARTER_PI", "Pi/4", nullptr, "PI", 1.0, 4.0, nullptr, QualifiedName(), QualifiedName("Units", "LENGTH_RATIO"), true, QualifiedName());
        helper.AssertUnit("Units", "MILLI", "milli", nullptr, "ONE", .001, nullptr, nullptr, QualifiedName(), QualifiedName("Units", "NUMBER"), true, QualifiedName());
        helper.AssertUnit("Units", "HORIZONTAL_PER_VERTICAL", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, QualifiedName("Units", "INTERNATIONAL"), QualifiedName("Units", "SLOPE"), false, QualifiedName("Units", "VERTICAL_PER_HORIZONTAL"));

        helper.AssertFormat("EC32Units", "MyFormat", "My Format", nullptr, JsonValue(R"json({"roundFactor":0.3, "type": "Fractional", "showSignOption": "OnlyNegative", "formatTraits": ["trailZeroes", "keepSingleZero"], "precision": 4, "decimalSeparator": ".", "thousandSeparator": ",", "uomSeparator": " "})json"), JsonValue());
        helper.AssertFormat("EC32Units", "MyFormatWithComposite", "My Format with composite", nullptr, JsonValue(R"json({"type": "Decimal", "formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"], "precision": 2})json"),
                            JsonValue(R"json({"includeZero":true, "spacer":"-", "units": [{"name":"HR", "label":"hour"}, {"name":"MIN", "label":"min"}]})json"));
        helper.AssertFormat("Formats", "DefaultReal", "real", nullptr, JsonValue(R"json({"type": "Decimal", "formatTraits": ["keepSingleZero", "keepDecimalPoint"], "precision": 6})json"), JsonValue());
        helper.AssertFormat("Formats", "AmerFI", "FeetInches", nullptr, JsonValue(R"json({"type": "Fractional", "formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"], "precision": 8, "uomSeparator":""})json"),
                            JsonValue(R"json({"includeZero":true, "spacer":"", "units": [{"name":"FT", "label":"'"}, {"name":"IN", "label":"\""}]})json"));
        }
    }
