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
#include "TestDb.h"

USING_NAMESPACE_BENTLEY_EC

//======================================================================================
// @bsiclass                                               Krischan.Eberle      06/2018
//======================================================================================
struct ECDbCompatibilityTestFixture : CompatibilityTestFixture
    {
    protected:
        void SetUp() override 
            { 
            CompatibilityTestFixture::SetUp();
            ASSERT_EQ(SUCCESS, TestECDbCreation::Run()); 
            }
    };

//---------------------------------------------------------------------------------------
// Runs basic tests on all available test files. This is a basic test to cover tests and test files
// which are added in the future, and to which existing test runners cannot be adjusted to.
// @bsimethod                                  Krischan.Eberle                      07/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, BasicTestsOnAllPulledFiles)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfAllPulledTestFiles())
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            // Run SELECT statements against all classes
            for (ECSchemaCP schema : testDb.GetDb().Schemas().GetSchemas())
                {
                if (schema->GetName().Equals("ECDbSystem"))
                    continue; //doesn't have mapped classes

                for (ECClassCP cl : schema->GetClasses())
                    {
                    if (!cl->IsEntityClass() && !cl->IsRelationshipClass())
                        continue;

                    Utf8String ecsql("SELECT ECInstanceId,ECClassId");
                    for (ECPropertyCP prop : cl->GetProperties())
                        {
                        ecsql.append(",[").append(prop->GetName()).append("]");
                        }
                    ecsql.append(" FROM ").append(cl->GetECSqlName());

                    ECSqlStatement stmt;
                    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), ecsql.c_str())) << ecsql << " | " << testDb.GetDescription();
                    const DbResult stepStat = stmt.Step();
                    ASSERT_TRUE(BE_SQLITE_DONE == stepStat || BE_SQLITE_ROW == stepStat) << ecsql << " | " << testDb.GetDescription();
                    }
                }
            }
        }
    }

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

            switch (testDb.GetAge())
                {
                    case ProfileState::Age::Older:
                    case ProfileState::Age::UpToDate:
                    {
                    EXPECT_EQ(5, testDb.GetSchemaCount()) << testDb.GetDescription();

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

                    //Standard schema versions
                    EXPECT_EQ(SchemaVersion(1, 0, 0), testDb.GetSchemaVersion("CoreCustomAttributes")) << testDb.GetDescription();
                    EXPECT_EQ(BeVersion(), testDb.GetOriginalECXmlVersion("CoreCustomAttributes")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue(R"js({"classcount":14, "enumcount": 2})js"), testDb.GetSchemaItemCounts("CoreCustomAttributes")) << testDb.GetDescription();
                    break;
                    }

                    case ProfileState::Age::Newer:
                    {
                    EXPECT_LE(5, testDb.GetSchemaCount()) << testDb.GetDescription();

                    //ECDb built-in schema versions
                    //ECDbFileInfo version was incremented in next profile, so must be higher in newer file
                    EXPECT_LT(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbFileInfo")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbFileInfo")) << testDb.GetDescription();

                    EXPECT_LE(SchemaVersion(2, 0, 0), testDb.GetSchemaVersion("ECDbMap")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbMap")) << testDb.GetDescription();

                    //ECDbMeta version was incremented in next profile, so must be higher in newer file
                    EXPECT_LT(SchemaVersion(4, 0, 0), testDb.GetSchemaVersion("ECDbMeta")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbMeta")) << testDb.GetDescription();

                    //ECDbSystem version was incremented in next profile, so must be higher in newer file
                    EXPECT_LT(SchemaVersion(5, 0, 0), testDb.GetSchemaVersion("ECDbSystem")) << testDb.GetDescription();
                    EXPECT_LE(BeVersion(3, 1), testDb.GetOriginalECXmlVersion("ECDbSystem")) << testDb.GetDescription();

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
// @bsimethod                                  Krischan.Eberle                      07/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, UpgradingPreEC32EnumsToEC32AfterProfileUpgrade)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_UPGRADEDEC32ENUMS))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //regardless of whether the actual test file is 3.1 and not upgraded yet or it is newer and upgraded,
            //the upgrade does not affect the EC31 enum API. So the test code is generic
            testDb.AssertEnum("UpgradedEC32Enums", "IntEnum_EnumeratorsWithoutDisplayLabel", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
            {{ECValue(0), nullptr},
            {ECValue(1), nullptr},
            {ECValue(2), nullptr}});

            testDb.AssertEnum("UpgradedEC32Enums", "StringEnum_EnumeratorsWithDisplayLabel", "String Enumeration with enumerators with display label", nullptr, PRIMITIVETYPE_String, false,
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
            ASSERT_TRUE(testDb.SupportsFeature(ECDbFeature::NamedEnumerators)) << testDb.GetDescription();

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

            switch (testDb.GetAge())
                {
                    case ProfileState::Age::Older:
                    case ProfileState::Age::UpToDate:
                        testDb.AssertKindOfQuantity("PreEC32Koqs", "ANGLE", "Angle", nullptr, "RAD(DefaultReal)", JsonValue(R"json(["ARC_DEG(Real2U)", "ARC_DEG(AngleDMS)"])json"), 0.0001);
                        testDb.AssertKindOfQuantity("PreEC32Koqs", "POWER", "Power", nullptr, "W(DefaultReal)", JsonValue(R"json(["W(Real4U)", "KW(Real4U)", "MEGAW(Real4U)", "BTU/HR(Real4U)", "KILOBTU/HR(Real4U)", "HP(Real4U)"])json"), 0.001);
                        testDb.AssertKindOfQuantity("PreEC32Koqs", "LIQUID_VOLUME", "Liquid Volume", nullptr, "CUB.M(DefaultReal)", JsonValue(R"json(["LITRE(Real4U)", "GALLON(Real4U)"])json"), 0.0001);
                        //A bug in bim02dev persisted a format along with the unit. This will be obsolete once EC32 is available
                        testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.4);
                        testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormat_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.5);
                        testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormatWithUnit_NoPresUnit", nullptr, nullptr, "FT(AmerFI8)", JsonValue(), 0.6);

                        testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_M_Mfi8", nullptr, nullptr, "M(DefaultReal)", JsonValue(R"json(["M(AmerFI8)"])json"), 0.7);
                        testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_Mfi8", nullptr, nullptr, "M(AmerFI8)", JsonValue(), 0.8);
                        testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_SQFTfi8", nullptr, nullptr, "SQ.FT(AmerFI8)", JsonValue(), 0.9);
                        testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_SQFTfi8_SQFTreal4u", nullptr, nullptr, "SQ.FT(AmerFI8)", JsonValue(R"json(["SQ.FT(Real4U)"])json"), 1.0);
                        break;

                    case ProfileState::Age::Newer:
                    {
                    testDb.AssertKindOfQuantity("PreEC32Koqs", "ANGLE", "Angle", nullptr, "RAD(DefaultReal)", JsonValue(R"json(["ARC_DEG(Real2U)", "ARC_DEG(AngleDMS)"])json"), 0.0001);
                    testDb.AssertKindOfQuantity("PreEC32Koqs", "POWER", "Power", nullptr, "W(DefaultReal)", JsonValue(R"json(["W(Real4U)", "KW(Real4U)", "MEGAW(Real4U)", "BTU/HR(Real4U)", "KILOBTU/HR(Real4U)", "HP(Real4U)"])json"), 0.001);
                    testDb.AssertKindOfQuantity("PreEC32Koqs", "LIQUID_VOLUME", "Liquid Volume", nullptr, "CUB.M(DefaultReal)", JsonValue(R"json(["LITRE(Real4U)", "GALLON(Real4U)"])json"), 0.0001);
                    if (TestDb::VersionSupportsFeature(testDb.GetECDbInitialVersion(), ECDbFeature::UnitsAndFormats))
                        testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(), 0.4);
                    else
                        {
                        //The original KOQ was serialized to disk in bim02dev in a wrong way, where it did persist the format along with the unit,
                        //although it shouldn't have one. This will not be fixed, as EC32 will make this obsolete anyways.
                        testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(R"json(["W/(M*K)(DefaultReal)"])json"), 0.4);
                        }

                    testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormat_NoPresUnit", nullptr, nullptr, "W/(M*K)(DefaultReal)", JsonValue(R"json(["W/(M*K)(DefaultReal)"])json"), 0.5);
                    testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_PersUnitWithFormatWithUnit_NoPresUnit", nullptr, nullptr, "FT(DefaultReal)", JsonValue(R"json(["FT(AmerFI8)"])json"), 0.6);

                    testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_M_Mfi8", nullptr, nullptr, "M(DefaultReal)", JsonValue(R"json(["FT(AmerFI8)"])json"), 0.7);
                    testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_Mfi8", nullptr, nullptr, "M(DefaultReal)", JsonValue(R"json(["FT(AmerFI8)"])json"), 0.8);
                    testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_SQFTfi8", nullptr, nullptr, "SQ.FT(DefaultReal)", JsonValue(), 0.9);
                    testDb.AssertKindOfQuantity("PreEC32Koqs", "TestKoq_SQFTfi8_SQFTreal4u", nullptr, nullptr, "SQ.FT(DefaultReal)", JsonValue(R"json(["SQ.FT(Real4U)"])json"), 1.0);
                    break;
                    }
                }
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
            ASSERT_TRUE(testDb.SupportsFeature(ECDbFeature::UnitsAndFormats)) << testDb.GetDescription();

            testDb.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithMandatoryComposite", "My first test KOQ", nullptr, "CM(DefaultReal)", JsonValue(R"js(["M(Real4U)"])js"), 0.1);
            testDb.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithOptionalComposite", nullptr, "My second test KOQ", "CM(DefaultReal)", JsonValue(R"js(["FT(AmerFI8)"])js"), 0.2);
            testDb.AssertKindOfQuantity("EC32Koqs", "TestKoq_PresFormatWithoutComposite", nullptr, nullptr, "CM(DefaultReal)", JsonValue(R"js(["FT(AmerFI8)"])js"), 0.3);
            testDb.AssertKindOfQuantity("EC32Koqs", "TestKoq_NoPresFormat", nullptr, nullptr, "KG(DefaultReal)", JsonValue(), 0.4);
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

            testDb.AssertKindOfQuantity("EC32Units", "KoqWithCustomFormat", nullptr, nullptr, "M(DefaultReal)", JsonValue(R"js(["M(DefaultRealU)"])js"), 0.1);
            testDb.AssertKindOfQuantity("EC32Units", "KoqWithCustomUnit", nullptr, nullptr, "MySquareM(DefaultReal)", JsonValue(R"js(["MySquareM(Real4U)"])js"), 0.2);
            testDb.AssertKindOfQuantity("EC32Units", "KoqWithCustomUnitAndFormat", nullptr, nullptr, "MySquareFt(DefaultReal)", JsonValue(R"js(["MySquareFt(DefaultRealU)"])js"), 0.3);

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

//---------------------------------------------------------------------------------------
// Performs an import of a pre EC 3.2 schema
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, PreEC32SchemaImport)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_EMPTY))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
                        <ECSchemaReference name="ECDbFileInfo" version="02.00.00" alias="ecdbf" />
                        <ECEntityClass typeName="Foo">
                                <ECCustomAttributes>
                                    <ClassMap xmlns="ECDbMap.02.00.00">
                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                    </ClassMap>
                                </ECCustomAttributes>
                                <ECProperty propertyName="Code" typeName="int" />
                                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                                <ECProperty propertyName="Status" typeName="StatusEnum" />
                        </ECEntityClass>
                        <ECEntityClass typeName="MyFileInfo">
                            <BaseClass>ecdbf:FileInfo</BaseClass>
                            <ECProperty propertyName="Path" typeName="string" />
                        </ECEntityClass>
                        <KindOfQuantity typeName="ANGLE" displayLabel="Angle" persistenceUnit="RAD(DefaultReal)" presentationUnits="ARC_DEG(real2u);ARC_DEG(dms)" relativeError="0.0001"/>
                        <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="SQ.M(DefaultReal)" presentationUnits="SQ.M(real4u);SQ.FT(real4u)" relativeError="0.0001"/>
                        <KindOfQuantity typeName="TEMPERATURE" displayLabel="Temperature" persistenceUnit="K(DefaultReal)" presentationUnits="CELSIUS(real4u);FAHRENHEIT(real4u);K(real4u)" relativeError="0.01"/>
                        <ECEnumeration typeName="StatusEnum" displayLabel="Int Enumeration with enumerators without display label" description="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                            <ECEnumerator value="0"/>
                            <ECEnumerator value="1"/>
                            <ECEnumerator value="2"/>
                        </ECEnumeration>
                     </ECSchema>)xml"));
            ASSERT_TRUE(deserializationCtx != nullptr);
            const BentleyStatus schemaImportStat = testDb.GetDb().Schemas().ImportSchemas(deserializationCtx->GetCache().GetSchemas());
            switch (testDb.GetAge())
                {
                    case ProfileState::Age::UpToDate:
                    {
                    EXPECT_EQ(SUCCESS, schemaImportStat) << testDb.GetDescription();
                    //No units or formats schema from EC3.2 must creep into the file when importing KOQs in an 4.0.0.1 file
                    EXPECT_EQ(JsonValue("[{\"cnt\": 0}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef WHERE Name IN ('Units','Formats')")) <<  testDb.GetDescription();
                    EXPECT_EQ(JsonValue("[{\"cnt\": 2}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef s JOIN meta.SchemaHasSchemaReferences ref ON s.ECInstanceId=ref.SourceECInstanceId WHERE s.Name='TestSchema'")) << testDb.GetDescription();

                    ECInstanceKey fooKey;
                    ECSqlStatement stmt;
                    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), "INSERT INTO ts.Foo(Code,Size,Status) VALUES(1,3.0,2)")) << testDb.GetDescription();
                    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(fooKey)) << testDb.GetDescription();
                    stmt.Finalize();
                    EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id": "%s", "Code": 1, "Size": 3.0, "Status": 2}])json", fooKey.GetInstanceId().ToHexStr().c_str())), testDb.ExecuteECSqlSelect("SELECT ECInstanceId, Code, Size, Status FROM ts.Foo")) << testDb.GetDescription();

                    ECClassCP fooClass = testDb.GetDb().Schemas().GetClass("TestSchema", "Foo");
                    ASSERT_TRUE(fooClass != nullptr && fooClass->IsEntityClass()) << testDb.GetDescription();
                    {
                    ECPropertyCP sizeProp = fooClass->GetPropertyP("Size");
                    ASSERT_TRUE(sizeProp != nullptr && sizeProp->GetIsPrimitive()) << testDb.GetDescription();
                    KindOfQuantityCP koq = sizeProp->GetAsPrimitiveProperty()->GetKindOfQuantity();
                    ASSERT_TRUE(koq != nullptr) << testDb.GetDescription();
                    testDb.AssertKindOfQuantity(*koq, "TestSchema", "AREA", "Area", nullptr, "SQ.M(DefaultReal)", JsonValue(R"json(["SQ.M(Real4U)", "SQ.FT(Real4U)"])json"), 0.0001);
                    }
                    {
                    ECPropertyCP statusProp = fooClass->GetPropertyP("Status");
                    ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << testDb.GetDescription();
                    ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
                    ASSERT_TRUE(ecenum != nullptr) << testDb.GetDescription();
                    testDb.AssertEnum(*ecenum, "TestSchema", "StatusEnum", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                                    {{ECValue(0), nullptr},
                                    {ECValue(1), nullptr},
                                    {ECValue(2), nullptr}});
                    }

                    testDb.AssertKindOfQuantity("TestSchema", "ANGLE", "Angle", nullptr, "RAD(DefaultReal)", JsonValue(R"json(["ARC_DEG(Real2U)", "ARC_DEG(AngleDMS)"])json"), 0.0001);
                    testDb.AssertKindOfQuantity("TestSchema", "AREA", "Area", nullptr, "SQ.M(DefaultReal)", JsonValue(R"json(["SQ.M(Real4U)", "SQ.FT(Real4U)"])json"), 0.0001);
                    testDb.AssertKindOfQuantity("TestSchema", "TEMPERATURE", "Temperature", nullptr, "K(DefaultReal)", JsonValue(R"json(["CELSIUS(Real4U)","FAHRENHEIT(Real4U)","K(Real4U)"])json"), 0.01);

                    testDb.AssertEnum("TestSchema", "StatusEnum", "Int Enumeration with enumerators without display label", "Int Enumeration with enumerators without display label", PRIMITIVETYPE_Integer, true,
                    {{ECValue(0), nullptr},
                    {ECValue(1), nullptr},
                    {ECValue(2), nullptr}});
                    break;
                    }

                    case ProfileState::Age::Older:
                    case ProfileState::Age::Newer:
                    {
                    EXPECT_EQ(ERROR, schemaImportStat) << testDb.GetDescription();
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
// Performs an update of a pre EC 3.2 schema
// @bsimethod                                  Krischan.Eberle                      08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbCompatibilityTestFixture, PreEC32SchemaUpdate)
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(TESTECDB_PREEC32SCHEMAUPDATE))
        {
        for (std::unique_ptr<TestECDb> testDbPtr : TestECDb::GetPermutationsFor(testFile))
            {
            TestECDb& testDb = *testDbPtr;
            if (testDb.GetOpenParams().IsReadonly())
                continue;

            ASSERT_EQ(BE_SQLITE_OK, testDb.Open()) << testDb.GetDescription();
            testDb.AssertProfileVersion();
            testDb.AssertLoadSchemas();

            //Schema changes:
            //- bumped up version to 1.0.1
            //- KOQ AREA: 
            //    - Added presentation unit SQ.CM(real4u)
            //    - Changed relativeError to 0.001
            // - Enum StatusEnum
            //    - Changed display label to "Status"
            //    - Added enumerator 3
            // - Added subclasses SubA, SubB, SubC
            ECSchemaReadContextPtr deserializationCtx = TestFileCreator::DeserializeSchema(testDb.GetDb(), SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
            <ECSchema schemaName="SchemaUpdateTest" alias="su" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
              <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />

             <KindOfQuantity typeName="AREA" displayLabel="Area" persistenceUnit="SQ.M(DefaultReal)" presentationUnits="SQ.M(real4u);SQ.FT(real4u);SQ.CM(real4u)" relativeError="0.001"/>
             <ECEnumeration typeName="StatusEnum" displayLabel="Status" backingTypeName="int" isStrict="true">
                <ECEnumerator value="0"/>
                <ECEnumerator value="1"/>
                <ECEnumerator value="2"/>
                <ECEnumerator value="3"/>
             </ECEnumeration>
             <ECEntityClass typeName="BaseA">
                <ECProperty propertyName="Code" typeName="int" />
                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                <ECProperty propertyName="Status" typeName="StatusEnum" />
             </ECEntityClass>
             <ECEntityClass typeName="SubA">
                <BaseClass>BaseA</BaseClass>
                <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
                <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
             </ECEntityClass>
             <ECEntityClass typeName="BaseB">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                <ECProperty propertyName="Status" typeName="StatusEnum" />
             </ECEntityClass>
             <ECEntityClass typeName="SubB">
                <BaseClass>BaseB</BaseClass>
                <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
                <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
             </ECEntityClass>
             <ECEntityClass typeName="BaseC">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>6</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="int" />
                <ECProperty propertyName="Size" typeName="double" kindOfQuantity="AREA" />
                <ECProperty propertyName="Status" typeName="StatusEnum" />
             </ECEntityClass>
             <ECEntityClass typeName="SubC">
                <BaseClass>BaseC</BaseClass>
                <ECArrayProperty propertyName="Sizes" typeName="double" kindOfQuantity="AREA" />
                <ECArrayProperty propertyName="Statuses" typeName="StatusEnum" />
             </ECEntityClass>
            </ECSchema>)xml"));
            ASSERT_TRUE(deserializationCtx != nullptr);
            const BentleyStatus schemaImportStat = testDb.GetDb().Schemas().ImportSchemas(deserializationCtx->GetCache().GetSchemas());
            switch (testDb.GetAge())
                {
                    case ProfileState::Age::UpToDate:
                    {
                    EXPECT_EQ(SUCCESS, schemaImportStat) << testDb.GetDescription();
                    //No units or formats schema from EC3.2 must creep into the file when importing KOQs in an 4.0.0.1 file
                    EXPECT_EQ(JsonValue("[{\"cnt\": 0}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef WHERE Name IN ('Units','Formats')")) << testDb.GetDescription();
                    EXPECT_EQ(JsonValue("[{\"cnt\": 1}]"), testDb.ExecuteECSqlSelect("SELECT count(*) cnt FROM meta.ECSchemaDef s JOIN meta.SchemaHasSchemaReferences ref ON s.ECInstanceId=ref.SourceECInstanceId WHERE s.Name='SchemaUpdateTest'")) << testDb.GetDescription();

                    for (Utf8CP className : {"SubA", "SubB", "SubC"})
                        {
                        ECInstanceKey key;
                        ECSqlStatement stmt;
                        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(testDb.GetDb(), Utf8PrintfString("INSERT INTO su.%s(Code,Size,Sizes,Status,Statuses) VALUES(?,?,?,?,?)", className).c_str())) << className << " | " << testDb.GetDescription();
                        ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 1)) << className << " | " << testDb.GetDescription();
                        ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(2, 2.2)) << className << " | " << testDb.GetDescription();

                        IECSqlBinder& sizesBinder = stmt.GetBinder(3);
                        ASSERT_EQ(ECSqlStatus::Success, sizesBinder.AddArrayElement().BindDouble(3.3)) << className << " | " << testDb.GetDescription();
                        ASSERT_EQ(ECSqlStatus::Success, sizesBinder.AddArrayElement().BindDouble(33.3)) << className << " | " << testDb.GetDescription();
                        ASSERT_EQ(ECSqlStatus::Success, sizesBinder.AddArrayElement().BindDouble(333.3)) << className << " | " << testDb.GetDescription();

                        ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(4, 3)) << className << " | " << testDb.GetDescription();
                        
                        IECSqlBinder& statusBinder = stmt.GetBinder(5);
                        ASSERT_EQ(ECSqlStatus::Success, statusBinder.AddArrayElement().BindInt(0)) << className << " | " << testDb.GetDescription();
                        ASSERT_EQ(ECSqlStatus::Success, statusBinder.AddArrayElement().BindInt(1)) << className << " | " << testDb.GetDescription();
                        ASSERT_EQ(ECSqlStatus::Success, statusBinder.AddArrayElement().BindInt(2)) << className << " | " << testDb.GetDescription();
                        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << className << " | " << testDb.GetDescription();
                        stmt.Finalize();
                        EXPECT_EQ(JsonValue(Utf8PrintfString(R"json([{"id": "%s", "Code": 1, "Size": 2.2, "Sizes" : [3.3,33.3,333.3], "Status": 3, "Statuses": [0,1,2]}])json", key.GetInstanceId().ToHexStr().c_str())), 
                                  testDb.ExecuteECSqlSelect(Utf8PrintfString("SELECT ECInstanceId,Code,Size,Sizes,Status,Statuses FROM su.%s", className).c_str())) << className << " | " << testDb.GetDescription();

                        ECClassCP cl = testDb.GetDb().Schemas().GetClass("SchemaUpdateTest", className);
                        ASSERT_TRUE(cl != nullptr && cl->IsEntityClass()) << className << " | " << testDb.GetDescription();
                        {
                        ECPropertyCP sizeProp = cl->GetPropertyP("Size");
                        ASSERT_TRUE(sizeProp != nullptr && sizeProp->GetIsPrimitive()) << className << " | " << testDb.GetDescription();
                        KindOfQuantityCP koq = sizeProp->GetAsPrimitiveProperty()->GetKindOfQuantity();
                        ASSERT_TRUE(koq != nullptr) << className << " | " << testDb.GetDescription();
                        testDb.AssertKindOfQuantity(*koq, "SchemaUpdateTest", "AREA", "Area", nullptr, "SQ.M(DefaultReal)", JsonValue(R"json(["SQ.M(Real4U)", "SQ.FT(Real4U)", "SQ.CM(Real4U)"])json"), 0.001);
                        }

                        {
                        ECPropertyCP sizesProp = cl->GetPropertyP("Sizes");
                        ASSERT_TRUE(sizesProp != nullptr && sizesProp->GetIsPrimitiveArray()) << className << " | " << testDb.GetDescription();
                        KindOfQuantityCP koq = sizesProp->GetAsPrimitiveArrayProperty()->GetKindOfQuantity();
                        ASSERT_TRUE(koq != nullptr) << className << " | " << testDb.GetDescription();
                        testDb.AssertKindOfQuantity(*koq, "SchemaUpdateTest", "AREA", "Area", nullptr, "SQ.M(DefaultReal)", JsonValue(R"json(["SQ.M(Real4U)", "SQ.FT(Real4U)", "SQ.CM(Real4U)"])json"), 0.001);
                        }

                        {
                        ECPropertyCP statusProp = cl->GetPropertyP("Status");
                        ASSERT_TRUE(statusProp != nullptr && statusProp->GetIsPrimitive()) << className << " | " << testDb.GetDescription();
                        ECEnumerationCP ecenum = statusProp->GetAsPrimitiveProperty()->GetEnumeration();
                        ASSERT_TRUE(ecenum != nullptr) << className << " | " << testDb.GetDescription();
                        testDb.AssertEnum(*ecenum, "SchemaUpdateTest", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                        {{ECValue(0), nullptr},
                        {ECValue(1), nullptr},
                        {ECValue(2), nullptr},
                        {ECValue(3), nullptr}});
                        }

                        {
                        ECPropertyCP statusesProp = cl->GetPropertyP("Statuses");
                        ASSERT_TRUE(statusesProp != nullptr && statusesProp->GetIsPrimitiveArray()) << className << " | " << testDb.GetDescription();
                        ECEnumerationCP ecenum = statusesProp->GetAsPrimitiveArrayProperty()->GetEnumeration();
                        ASSERT_TRUE(ecenum != nullptr) << className << " | " << testDb.GetDescription();
                        testDb.AssertEnum(*ecenum, "SchemaUpdateTest", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                        {{ECValue(0), nullptr},
                        {ECValue(1), nullptr},
                        {ECValue(2), nullptr},
                        {ECValue(3), nullptr}});
                        }

                        }

                    testDb.AssertKindOfQuantity("SchemaUpdateTest", "AREA", "Area", nullptr, "SQ.M(DefaultReal)", JsonValue(R"json(["SQ.M(Real4U)", "SQ.FT(Real4U)", "SQ.CM(Real4U)"])json"), 0.001);

                    testDb.AssertEnum("SchemaUpdateTest", "StatusEnum", "Status", nullptr, PRIMITIVETYPE_Integer, true,
                    {{ECValue(0), nullptr},
                    {ECValue(1), nullptr},
                    {ECValue(2), nullptr},
                    {ECValue(3), nullptr}});
                    break;
                    }

                    case ProfileState::Age::Older:
                    case ProfileState::Age::Newer:
                    {
                    EXPECT_EQ(ERROR, schemaImportStat) << testDb.GetDescription();
                    break;
                    }
                    default:
                        FAIL() << "Unhandled ProfileState::Age enum value | " << testDb.GetDescription();
                        break;
                }
            }
        }
    }