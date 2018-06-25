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
#include "TestHelper.h"

USING_NAMESPACE_BENTLEY_EC

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct IModelCompatibilityTestFixture : CompatibilityTestFixture
    {
    protected:
        ScopedDgnHost m_host;

        DgnDbPtr OpenTestFile(DbResult* stat, TestFile const& testFile)
            {
            DgnDb::OpenParams params(Db::OpenMode::ReadWrite);
            params.SetProfileUpgradeOptions(Db::ProfileUpgradeOptions::Upgrade);
            return DgnDb::OpenDgnDb(stat, testFile.GetPath(), params);
            }

        void SetUp() override { ASSERT_EQ(SUCCESS, TestIModelCreation::Run()); }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, BuiltinSchemaVersions)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EMPTY))
        {
        ProfileState::Age originalFileAge = testFile.GetAge();

        DbResult stat = BE_SQLITE_ERROR;
        DgnDbPtr bim = OpenTestFile(&stat, testFile);
        ASSERT_EQ(BE_SQLITE_OK, stat) << testFile.ToString();
        ASSERT_TRUE(bim != nullptr) << testFile.ToString();

        ProfileState actualProfileState = bim->CheckProfileVersion();
        TestHelper helper(testFile, *bim);
        helper.AssertLoadSchemas();
        const int schemaCount = helper.GetSchemaCount();

        switch (originalFileAge)
            {
                case ProfileState::Age::UpToDate:
                {
                EXPECT_TRUE(actualProfileState.IsUpToDate()) << testFile.ToString();
                EXPECT_EQ(DgnDbProfile::Get().GetExpectedVersion(), bim->GetProfileVersion()) << testFile.ToString();

                EXPECT_EQ(8, schemaCount) << testFile.ToString();

                for (ECSchemaCP schema : bim->Schemas().GetSchemas(false))
                    {
                    EXPECT_EQ((int) ECVersion::V3_2, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    }

                //DgnDb built-in schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 1), helper.GetSchemaVersion("BisCore")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 1), helper.GetOriginalECXmlVersion("BisCore")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":164, "enumcount": 2})js"), helper.GetSchemaItemCounts("BisCore")) << testFile.ToString();

                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("Generic")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 1), helper.GetOriginalECXmlVersion("Generic")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":17})js"), helper.GetSchemaItemCounts("Generic")) << testFile.ToString();

                //ECDb built-in schema versions
                EXPECT_EQ(SchemaVersion(2, 0, 1), helper.GetSchemaVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbFileInfo")) << "Schema has enums which were upgraded to EC32 format, therefore original XML version was set to 3.2 | " << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), helper.GetSchemaItemCounts("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMap")) << "Schema has no enums, so was not upgraded to EC32, so no original ECXML persisted yet | " << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), helper.GetSchemaItemCounts("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(4, 0, 1), helper.GetSchemaVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMeta")) << "Schema has enums which were upgraded to EC32 format, therefore original XML version was set to 3.2 | " << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":38, "enumcount": 9})js"), helper.GetSchemaItemCounts("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(5, 0, 1), helper.GetSchemaVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbSystem")) << "Schema has no enums, so was not upgraded to EC32, so no original ECXML persisted yet | " << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), helper.GetSchemaItemCounts("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("ECDbSchemaPolicies")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbSchemaPolicies")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":3})js"), helper.GetSchemaItemCounts("ECDbSchemaPolicies")) << testFile.ToString();

                //standard schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 1), helper.GetOriginalECXmlVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), helper.GetSchemaItemCounts("CoreCustomAttributes")) << testFile.ToString();
                break;
                }
                case ProfileState::Age::Older:
                {
                EXPECT_TRUE(actualProfileState.IsUpToDate()) << "File is expected to be auto-upgraded" << testFile.ToString();
                EXPECT_EQ(DgnDbProfile::Get().GetExpectedVersion(), bim->GetProfileVersion()) << "File is expected to be auto-upgraded" << testFile.ToString();

                EXPECT_EQ(8, schemaCount) << testFile.ToString();

                for (ECSchemaCP schema : bim->Schemas().GetSchemas(false))
                    {
                    EXPECT_EQ((int) ECVersion::V3_2, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    }

                //DgnDb built-in schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 1), helper.GetSchemaVersion("BisCore")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 1), helper.GetOriginalECXmlVersion("BisCore")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":164, "enumcount": 2})js"), helper.GetSchemaItemCounts("BisCore")) << testFile.ToString();

                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("Generic")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 1), helper.GetOriginalECXmlVersion("Generic")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":17})js"), helper.GetSchemaItemCounts("Generic")) << testFile.ToString();

                //ECDb built-in schema versions
                EXPECT_EQ(SchemaVersion(2, 0, 1), helper.GetSchemaVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4, "enumcount": 1})js"), helper.GetSchemaItemCounts("ECDbFileInfo")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(BeVersion(), helper.GetOriginalECXmlVersion("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":9})js"), helper.GetSchemaItemCounts("ECDbMap")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(4, 0, 1), helper.GetSchemaVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":24, "enumcount": 8})js"), helper.GetSchemaItemCounts("ECDbMeta")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(5, 0, 1), helper.GetSchemaVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(BeVersion(), helper.GetOriginalECXmlVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":4})js"), helper.GetSchemaItemCounts("ECDbSystem")) << testFile.ToString();
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("ECDbSchemaPolicies")) << testFile.ToString();
                EXPECT_EQ(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbSchemaPolicies")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":3})js"), helper.GetSchemaItemCounts("ECDbSchemaPolicies")) << testFile.ToString();

                //standard schema versions
                EXPECT_EQ(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_EQ(BeVersion(), helper.GetOriginalECXmlVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), helper.GetSchemaItemCounts("CoreCustomAttributes"));
                break;
                }

                case ProfileState::Age::Newer:
                {
                EXPECT_TRUE(actualProfileState.IsNewer()) << testFile.ToString();
                EXPECT_LT(DgnDbProfile::Get().GetExpectedVersion(), bim->GetProfileVersion()) << testFile.ToString();

                EXPECT_EQ(8, schemaCount) << testFile.ToString();

                for (ECSchemaCP schema : bim->Schemas().GetSchemas(false))
                    {
                    EXPECT_LE((int) ECVersion::V3_2, (int) schema->GetECVersion()) << schema->GetFullSchemaName() << " | " << testFile.ToString();
                    }

                //DgnDb built-in schema versions
                EXPECT_LE(SchemaVersion(1, 0, 1), helper.GetSchemaVersion("BisCore")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("BisCore")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("Generic")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("Generic")) << testFile.ToString();

                //ECDb built-in schema versions
                EXPECT_LE(SchemaVersion(2, 0, 1), helper.GetSchemaVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbFileInfo")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(2, 0, 0), helper.GetSchemaVersion("ECDbMap")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMap")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(4, 0, 1), helper.GetSchemaVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbMeta")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(5, 0, 1), helper.GetSchemaVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbSystem")) << testFile.ToString();
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("ECDbSchemaPolicies")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("ECDbSchemaPolicies")) << testFile.ToString();

                //standard schema versions
                EXPECT_LE(SchemaVersion(1, 0, 0), helper.GetSchemaVersion("CoreCustomAttributes")) << testFile.ToString();
                EXPECT_LE(BeVersion(3, 2), helper.GetOriginalECXmlVersion("CoreCustomAttributes")) << testFile.ToString();

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
        DbResult stat = BE_SQLITE_ERROR;
        DgnDbPtr bim = OpenTestFile(&stat, testFile);
        ASSERT_EQ(BE_SQLITE_OK, stat) << testFile.ToString();
        ASSERT_TRUE(bim != nullptr) << testFile.ToString();

        TestHelper helper(testFile, *bim);
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
TEST_F(IModelCompatibilityTestFixture, EC32Enums)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC32ENUMS))
        {
        DbResult stat = BE_SQLITE_ERROR;
        DgnDbPtr bim = OpenTestFile(&stat, testFile);
        ASSERT_EQ(BE_SQLITE_OK, stat) << testFile.ToString();
        ASSERT_TRUE(bim != nullptr) << testFile.ToString();

        TestHelper helper(testFile, *bim);
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
TEST_F(IModelCompatibilityTestFixture, PreEC32KindOfQuantities)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_PREEC32KOQS))
        {
        DbResult stat = BE_SQLITE_ERROR;
        DgnDbPtr bim = OpenTestFile(&stat, testFile);
        ASSERT_EQ(BE_SQLITE_OK, stat) << testFile.ToString();
        ASSERT_TRUE(bim != nullptr) << testFile.ToString();

        TestHelper helper(testFile, *bim);
        helper.AssertLoadSchemas();

        helper.AssertKindOfQuantity("PreEC32Koqs", "ANGLE", "Angle", nullptr, "u:RAD", JsonValue(R"json(["f:DefaultRealU(2)[u:ARC_DEG]", "f:AngleDMS"])json"), 0.0001);
        helper.AssertKindOfQuantity("PreEC32Koqs", "POWER", "Power", nullptr, "u:W", JsonValue(R"json(["f:DefaultRealU(4)[u:W]", "f:DefaultRealU(4)[u:KW]", "f:DefaultRealU(4)[u:MEGAW]", "f:DefaultRealU(4)[u:BTU_PER_HR]", "f:DefaultRealU(4)[u:KILOBTU_PER_HR]", "f:DefaultRealU(4)[u:HP]"])json"), 0.001);
        helper.AssertKindOfQuantity("PreEC32Koqs", "LIQUID_VOLUME", "Liquid Volume", nullptr, "u:CUB_M", JsonValue(R"json(["f:DefaultRealU(4)[u:LITRE]", "f:DefaultRealU(4)[u:GALLON]"])json"), 0.0001);
        helper.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_NoPresUnit", nullptr, nullptr, "u:W_PER_M_K", JsonValue(), 0.4);
        helper.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormat_NoPresUnit", nullptr, nullptr, "u:W_PER_M_K", JsonValue(R"json(["f:DefaultReal[u:W_PER_M_K]"])json"), 0.5);
        helper.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormatWithUnit_NoPresUnit", nullptr, nullptr, "u:FT", JsonValue(R"json(["f:AmerFI"])json"), 0.6);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC32KindOfQuantities)
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC32KOQS))
        {
        DbResult stat = BE_SQLITE_ERROR;
        DgnDbPtr bim = OpenTestFile(&stat, testFile);
        ASSERT_EQ(BE_SQLITE_OK, stat) << testFile.ToString();
        ASSERT_TRUE(bim != nullptr) << testFile.ToString();

        TestHelper helper(testFile, *bim);
        helper.AssertLoadSchemas();

        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithMandatoryComposite", "My first test KOQ", nullptr, "u:CM", JsonValue(R"js(["f:DefaultRealU(4)[u:M]"])js"), 0.1);
        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithOptionalComposite", nullptr, "My second test KOQ", "u:CM", JsonValue(R"js(["f:AmerFI[u:FT|feet][u:IN|inches]"])js"), 0.2);
        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithoutComposite", nullptr, nullptr, "u:CM", JsonValue(R"js(["f:AmerFI"])js"), 0.3);
        helper.AssertKindOfQuantity("EC32Koqs", "TestKoq_NoPresFormat", nullptr, nullptr, "u:KG", JsonValue(), 0.4);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(IModelCompatibilityTestFixture, EC32Units)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTIMODEL_EC32UNITS))
        {
        ProfileState::Age originalFileAge = testFile.GetAge();

        DbResult stat = BE_SQLITE_ERROR;
        DgnDbPtr bim = OpenTestFile(&stat, testFile);
        ASSERT_EQ(BE_SQLITE_OK, stat) << testFile.ToString();
        ASSERT_TRUE(bim != nullptr) << testFile.ToString();

        ProfileState actualProfileState = bim->CheckProfileVersion();
        switch (originalFileAge)
            {
                case ProfileState::Age::Older:
                    EXPECT_TRUE(actualProfileState.IsUpToDate()) << "File is expected to be auto-upgraded. " << testFile.ToString();
                    EXPECT_EQ(DgnDbProfile::Get().GetExpectedVersion(), bim->GetProfileVersion()) << "File is expected to be auto-upgraded. " << testFile.ToString();
                    break;
                case ProfileState::Age::UpToDate:
                    EXPECT_TRUE(actualProfileState.IsUpToDate()) << testFile.ToString();
                    EXPECT_EQ(DgnDbProfile::Get().GetExpectedVersion(), bim->GetProfileVersion()) << testFile.ToString();
                    break;
                case ProfileState::Age::Newer:
                    EXPECT_TRUE(actualProfileState.IsNewer()) << testFile.ToString();
                    EXPECT_LT(DgnDbProfile::Get().GetExpectedVersion(), bim->GetProfileVersion()) << testFile.ToString();
                    break;

                default:
                    FAIL() << "unhandled enum value";
            }

        TestHelper helper(testFile, *bim);
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
