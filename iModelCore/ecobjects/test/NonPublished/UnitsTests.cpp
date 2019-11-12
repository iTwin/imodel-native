/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include <fstream>
#include <sstream>
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct UnitConversionTests : ECTestFixture {};
struct UnitsTests : ECTestFixture {};
struct InvertedUnitsTests : ECTestFixture {};
struct ConstantTests : ECTestFixture {};
struct UnitsDeserializationTests : ECTestFixture {};
struct InvertedUnitsDeserializationTests: ECTestFixture {};
struct ConstantDeserializationTests: ECTestFixture {};

template<class T> typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
static almost_equal(const T x, const T y, int ulp)
    {
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return fabs(x - y) < std::numeric_limits<T>::epsilon() * fabs(x + y) * ulp
        // unless the result is subnormal
        || fabs(x - y) < std::numeric_limits<T>::min();
    }

static void CompareValues(double expected, double actual, int ulpPower, Utf8CP message)
    {
    if (!almost_equal<double>(expected, actual, int(pow(10,ulpPower))))
        {
        Utf8PrintfString formattedText("%s\nExpected: %.17g \nActual:   %.17g \nDiff:     %.17g   Diff/Exp: %.17g   ULP: %d\n",
                                       message, expected, actual, actual - expected, (actual - expected) / expected, pow(10,ulpPower));
        EXPECT_FALSE(true) << formattedText;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, UnitConversionsMatchOldConversions)
    {
    auto path = ECTestFixture::GetTestDataPath(L"ec31UnitConversions.csv");
    std::ifstream ifs = std::ifstream(Utf8String(path.c_str()).c_str(), std::ifstream::in);
    std::string line;
    auto s = GetUnitsSchema();
    auto toDouble = [](Utf8StringCR d)
        {
        std::istringstream iss(d.c_str());
        double val = 0.0;
        iss >> val;
        return val;
        };
    while (std::getline(ifs, line))
        {
        bvector<Utf8String> split;
        BeStringUtilities::Split(line.c_str(), ",", split);
        ASSERT_EQ(4, split.size());
        auto fromName = split[0];
        auto toName = split[2];
        auto orig = toDouble(split[1]);
        auto conv = toDouble(split[3]);
        auto from = Units::UnitNameMappings::TryGetECNameFromNewName(fromName.c_str());
        auto to = Units::UnitNameMappings::TryGetECNameFromNewName(toName.c_str());
        ASSERT_NE(nullptr, from);
        ASSERT_NE(nullptr, to);

        // From Schema
        Utf8String alias;
        Utf8String name;
        ECClass::ParseClassName(alias, name, from);
        auto fromSchema = s->GetUnitCP(name.c_str());
        ECClass::ParseClassName(alias, name, to);
        auto toSchema = s->GetUnitCP(name.c_str());
        double converted;
        fromSchema->Convert(converted, orig, toSchema);
        ASSERT_DOUBLE_EQ(converted, conv);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  05/2019
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, VolumetricFlowUnitConversions)
    {
    ECUnitCP gallonPerHour = ECTestFixture::GetUnitsSchema()->GetUnitCP("GALLON_PER_HR");
    ECUnitCP imperialGallonPerHour = ECTestFixture::GetUnitsSchema()->GetUnitCP("GALLON_IMPERIAL_PER_HR");
    ECUnitCP cubicMeterPerSecond = ECTestFixture::GetUnitsSchema()->GetUnitCP("CUB_M_PER_SEC");
    double expected = 1.0515032733e-6; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    double actual;
    gallonPerHour->Convert(actual, 1.0, cubicMeterPerSecond);
    CompareValues(expected, actual, 6, "Conversion from Gallon per Hour to cubic meter per second not as expected");

    expected = 1.2628027778e-6; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    imperialGallonPerHour->Convert(actual, 1.0, cubicMeterPerSecond);
    CompareValues(expected, actual, 6, "Conversion from Imperial Gallon per Hour to cubic meter per second not as expected");

    expected = 60;
    ECTestFixture::GetUnitsSchema()->GetUnitCP("GALLON_PER_MIN")->Convert(actual, 1.0, gallonPerHour);
    EXPECT_DOUBLE_EQ(expected, actual) << "Conversion from Gallon per Hour to gallon per minute not as expected";
    ECTestFixture::GetUnitsSchema()->GetUnitCP("GALLON_IMPERIAL_PER_MIN")->Convert(actual, 1.0, imperialGallonPerHour);
    EXPECT_DOUBLE_EQ(expected, actual) << "Conversion from imperial gallon per Hour to imperial gallon per minute not as expected";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  09/2019
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, ProbabilityUnitConversions)
    {
    ECUnitCP probFraction = ECTestFixture::GetUnitsSchema()->GetUnitCP("PROBABILITY_FRACTION");
    ECUnitCP propPercent = ECTestFixture::GetUnitsSchema()->GetUnitCP("PROBABILITY_PERCENT");
    double expected = 100.0; // 1
    double actual;
    probFraction->Convert(actual, 1.0, propPercent);
    CompareValues(expected, actual, 15, "Conversion from Probability Fraction to Probability Percent not as expected");

    expected = 0.42; // 42%
    propPercent->Convert(actual, 42.0, probFraction);
    CompareValues(expected, actual, 15, "Conversion from Probability Percent to Probability Fraction not as expected");

    expected = 1.0;
    propPercent->Convert(actual, 1.0, propPercent);
    CompareValues(expected, actual, 15, "Conversion from Probability Percent to itself not as expected");

    probFraction->Convert(actual, 1.0, probFraction);
    CompareValues(expected, actual, 15, "Conversion from Probability Fraction to itself not as expected");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsTests, BasicECUnitCreation)
    {
    ECSchemaPtr schema;
    ECUnitP unit;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    EC_EXPECT_SUCCESS(schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, 10.0, 1.0, 1.0, "ExampleUnitLabel", "ExampleUnitDescription")); 
    
    EXPECT_STREQ("ExampleUnitDescription", unit->GetInvariantDescription().c_str());
    EXPECT_TRUE(unit->GetIsDescriptionDefined());
    EXPECT_STREQ("ExampleUnitLabel", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());
    EXPECT_STREQ("ExampleUnit", unit->GetName().c_str());
    EXPECT_EQ(phenom, unit->GetPhenomenon());
    EXPECT_EQ(system, unit->GetUnitSystem());
    EXPECT_EQ(10.0, unit->GetNumerator());
    EXPECT_TRUE(unit->HasNumerator());
    EXPECT_EQ(1.0, unit->GetDenominator());
    EXPECT_TRUE(unit->HasDenominator());
    EXPECT_EQ(1.0, unit->GetOffset());
    EXPECT_TRUE(unit->HasOffset());
    EXPECT_TRUE(unit->HasUnitSystem());
    EXPECT_TRUE(unit->HasDefinition());
    auto testECUnit = schema->GetUnitCP("ExampleUnit");
    EXPECT_EQ(unit, testECUnit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsTests, LookupUnitTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Unit typeName="TestUnit" phenomenon="u:LENGTH" unitSystem="u:SI" definition="u:M"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    auto shouldBeNull = schema->LookupUnit("");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupUnit("banana:M");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupUnit("Units:M");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupUnit("u:M", true);
    EXPECT_EQ(nullptr, shouldBeNull);
    auto shouldNotBeNull = schema->LookupUnit("TestUnit");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("TestUnit", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("ts:TestUnit");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("TestUnit", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("TestSchema:TestUnit", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("TestUnit", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("u:M");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("M", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("Units:M", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("M", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("TS:TestUnit");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("TestUnit", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("TESTSCHEMA:TestUnit", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("TestUnit", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("U:M");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("M", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("UNITS:M", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("M", shouldNotBeNull->GetName().c_str());
    bvector<Units::UnitCP> units;
    schema->GetUnitsContext().AllUnits(units);
    ASSERT_EQ(schema->GetUnitCount(), units.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsTests, ECUnitContainerTest)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    
    {
    ECUnitP unit;
    EXPECT_EQ(ECObjectsStatus::Success,schema->CreateUnit(unit, "ExampleUnit1", "M", *phenom, *system, 10.0, "ExampleUnitLabel1", "ExampleUnitDescription1"));
    EXPECT_TRUE(nullptr != unit);
    }
    {
    ECUnitP unit;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateUnit(unit, "ExampleUnit2", "MM", *phenom, *system, 1.0, "ExampleUnitLabel2", "ExampleUnitDescription2"));
    EXPECT_TRUE(nullptr != unit);
    }
    {
    ECUnitP unit;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateUnit(unit, "ExampleUnit3", "MMM", *phenom, *system));
    EXPECT_TRUE(nullptr != unit);
    }
    ECUnitP unitToBeInverted;
    {
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateUnit(unitToBeInverted, "ExampleUnit4", "MMMM", *phenom, *system, nullptr, "ExampleUnitLabel4", "ExampleUnitDescription4"));
    EXPECT_TRUE(nullptr != unitToBeInverted);
    }
    {
    ECUnitP invUnit;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateInvertedUnit(invUnit, *unitToBeInverted, "InvertedUnit", *system, "Inverted Unit", "Inverted Unit"));
    EXPECT_TRUE(nullptr != invUnit);
    }
    {
    ECUnitP constant;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateConstant(constant, "Constant", "M", *phenom, 10.0, 1.0, "Constant", "Constant"));
    EXPECT_TRUE(nullptr != constant);
    }

    EXPECT_EQ(6, schema->GetUnitCount());
    int curCount = 0;
    for (ECUnitCP unit : schema->GetUnits())
        {
        EXPECT_TRUE(nullptr != unit);
        switch (curCount)
            {
            case 0:
                ASSERT_TRUE(unit->IsConstant());
                EXPECT_STREQ("Constant", unit->GetInvariantDescription().c_str());
                EXPECT_STREQ("Constant", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_TRUE(unit->GetIsDescriptionDefined());
                EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
                EXPECT_STREQ("Constant", unit->GetName().c_str());
                EXPECT_STREQ("M", unit->GetDefinition().c_str());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(nullptr, unit->GetUnitSystem());
                EXPECT_DOUBLE_EQ(10.0, unit->GetNumerator());
                EXPECT_TRUE(unit->HasNumerator());
                EXPECT_DOUBLE_EQ(1.0, unit->GetDenominator());
                EXPECT_TRUE(unit->HasDenominator());
                EXPECT_FALSE(unit->HasUnitSystem());
                break;
            case 1:
                EXPECT_STREQ("ExampleUnitDescription1", unit->GetInvariantDescription().c_str());
                EXPECT_STREQ("ExampleUnitLabel1", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_TRUE(unit->GetIsDescriptionDefined());
                EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
                EXPECT_STREQ("M", unit->GetDefinition().c_str());
                EXPECT_STREQ("ExampleUnit1", unit->GetName().c_str());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(system, unit->GetUnitSystem());
                EXPECT_TRUE(unit->HasNumerator());
                EXPECT_EQ(10.0, unit->GetNumerator());
                EXPECT_FALSE(unit->HasDenominator());
                EXPECT_EQ(1.0, unit->GetDenominator());
                EXPECT_FALSE(unit->HasOffset());
                EXPECT_EQ(0.0, unit->GetOffset());
                EXPECT_TRUE(unit->HasUnitSystem());
                break;
            case 2:
                EXPECT_STREQ("ExampleUnitDescription2", unit->GetInvariantDescription().c_str());
                EXPECT_STREQ("ExampleUnitLabel2", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_STREQ("MM", unit->GetDefinition().c_str());
                EXPECT_STREQ("ExampleUnit2", unit->GetName().c_str());
                EXPECT_TRUE(unit->GetIsDescriptionDefined());
                EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(system, unit->GetUnitSystem());
                EXPECT_TRUE(unit->HasNumerator());
                EXPECT_EQ(1.0, unit->GetNumerator());
                EXPECT_FALSE(unit->HasDenominator());
                EXPECT_EQ(1.0, unit->GetDenominator());
                EXPECT_FALSE(unit->HasOffset());
                EXPECT_EQ(0.0, unit->GetOffset());
                EXPECT_TRUE(unit->HasUnitSystem());
                break;
            case 3:
                EXPECT_STREQ("ExampleUnit3", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_STREQ("MMM", unit->GetDefinition().c_str());
                EXPECT_STREQ("ExampleUnit3", unit->GetName().c_str());
                EXPECT_FALSE(unit->GetIsDescriptionDefined());
                EXPECT_FALSE(unit->GetIsDisplayLabelDefined());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(system, unit->GetUnitSystem());
                EXPECT_FALSE(unit->HasNumerator());
                EXPECT_EQ(1.0, unit->GetNumerator());
                EXPECT_FALSE(unit->HasDenominator());
                EXPECT_EQ(1.0, unit->GetDenominator());
                EXPECT_FALSE(unit->HasOffset());
                EXPECT_EQ(0.0, unit->GetOffset());
                EXPECT_TRUE(unit->HasUnitSystem());
                break;
            case 4:
                EXPECT_STREQ("ExampleUnitDescription4", unit->GetInvariantDescription().c_str());
                EXPECT_STREQ("ExampleUnitLabel4", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_STREQ("MMMM", unit->GetDefinition().c_str());
                EXPECT_STREQ("ExampleUnit4", unit->GetName().c_str());
                EXPECT_TRUE(unit->GetIsDescriptionDefined());
                EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(system, unit->GetUnitSystem());
                EXPECT_FALSE(unit->HasNumerator());
                EXPECT_EQ(1.0, unit->GetNumerator());
                EXPECT_FALSE(unit->HasDenominator());
                EXPECT_EQ(1.0, unit->GetDenominator());
                EXPECT_FALSE(unit->HasOffset());
                EXPECT_EQ(0.0, unit->GetOffset());
                EXPECT_TRUE(unit->HasUnitSystem());
                break;
            case 5:
                ASSERT_TRUE(unit->IsInvertedUnit());
                EXPECT_FALSE(unit->HasDefinition());
                EXPECT_STREQ("Inverted Unit", unit->GetInvariantDescription().c_str());
                EXPECT_STREQ("Inverted Unit", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_TRUE(unit->GetIsDescriptionDefined());
                EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
                EXPECT_STREQ("InvertedUnit", unit->GetName().c_str());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(system, unit->GetUnitSystem());
                EXPECT_TRUE(unit->HasUnitSystem());
                EXPECT_NE(nullptr, unit->GetInvertingUnit());
                EXPECT_FALSE(unit->HasNumerator());
                EXPECT_FALSE(unit->HasDenominator());
                EXPECT_FALSE(unit->HasOffset());
                break;
            }
        curCount++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Kyle.Abramowitz                          02/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitsTests, StandaloneSchemaChildECUnit)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    ECUnitP unit;
    schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, 10.0, 1.0, 1.0, "ExampleUnitLabel", "ExampleUnitDescription");

    Json::Value schemaJson;
    EXPECT_TRUE(unit->ToJson(schemaJson, true));

    Json::Value testDataJson;

    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneUnit.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Kyle.Abramowitz                          03/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitsTests, WritingToPre32VersionShouldNotWriteUnit)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    ECUnitP unit;
    schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, 10.0, 1.0, 1.0, "ExampleUnitLabel", "ExampleUnitDescription");

    Utf8String out;
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(out, ECVersion::V3_1));
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, out.c_str(), *context);
    ASSERT_EQ(nullptr, schema->GetUnitCP("ExampleUnit"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsTests, AllUnitsInStandardUnitsSchemaHaveValidDefinitions)
    {
    ECSchemaPtr schema = ECTestFixture::GetUnitsSchema();

    bvector<Units::UnitCP> allUnits;
    schema->GetUnitsContext().AllUnits(allUnits);

    for(auto const& unit: allUnits)
        {
        Utf8StringCR expression = ((ECUnitCP)unit)->GetParsedUnitExpression();
        ASSERT_FALSE(expression.empty());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, BasicRoundTripTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit" offset="10.0" numerator="10.0" denominator="10.0"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(1, schema->GetUnitCount());
    ECUnitCP unit = schema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != unit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());

    EXPECT_STREQ("Unit", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", unit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.0, unit->GetOffset());
    EXPECT_TRUE(unit->HasOffset());
    EXPECT_DOUBLE_EQ(10.0, unit->GetNumerator());
    EXPECT_TRUE(unit->HasNumerator());
    EXPECT_DOUBLE_EQ(10.0, unit->GetDenominator());
    EXPECT_TRUE(unit->HasDenominator());

    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(1, serializedSchema->GetUnitCount());
    ECUnitCP serializedECUnit = serializedSchema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != serializedECUnit);

    EXPECT_STREQ("Unit", serializedECUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", serializedECUnit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", serializedECUnit->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.0, serializedECUnit->GetOffset());
    EXPECT_TRUE(serializedECUnit->HasOffset());
    EXPECT_DOUBLE_EQ(10.0, serializedECUnit->GetNumerator());
    EXPECT_TRUE(serializedECUnit->HasNumerator());
    EXPECT_DOUBLE_EQ(10.0, serializedECUnit->GetDenominator());
    EXPECT_TRUE(serializedECUnit->HasDenominator());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, RoundTripWithReferencedSchemaForPhenomenonAndUnitSystem)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00.00" alias="rs"/>
            <Unit typeName="TestUnit" phenomenon="rs:TestPhenomenon" unitSystem="rs:TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(1, schema->GetUnitCount());
    ECUnitCP unit = schema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != unit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());
        
    EXPECT_STREQ("Unit", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", unit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());
    EXPECT_STREQ("LENGTH*LENGTH", unit->GetPhenomenon()->GetDefinition().c_str());
    EXPECT_STREQ("Unit System", ((ECN::UnitSystemCP)unit->GetUnitSystem())->GetInvariantDisplayLabel().c_str());
    EXPECT_FALSE(unit->HasOffset());
    EXPECT_FALSE(unit->HasNumerator());
    EXPECT_FALSE(unit->HasDenominator());
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    EXPECT_EQ(SchemaWriteStatus::Success, refSchema->WriteToXmlString(serializedRefSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaPtr serializedRefSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedRefSchema, serializedRefSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(1, serializedSchema->GetUnitCount());
    ECUnitCP serializedUnit = serializedSchema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != serializedUnit);

    EXPECT_STREQ("Unit", serializedUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", serializedUnit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", serializedUnit->GetDefinition().c_str());
    EXPECT_STREQ("LENGTH*LENGTH", serializedUnit->GetPhenomenon()->GetDefinition().c_str());
    EXPECT_STREQ("Unit System", ((ECN::UnitSystemCP)serializedUnit->GetUnitSystem())->GetInvariantDisplayLabel().c_str());
    EXPECT_FALSE(serializedUnit->HasOffset());
    EXPECT_FALSE(serializedUnit->HasNumerator());
    EXPECT_FALSE(serializedUnit->HasDenominator());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, ShouldFailWithoutAliases)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00.00" alias="rs"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, ShouldFailWithoutPhenomenonAndUnitSystemBeingDefined)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit without properly defined unit system and phenomenon");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, DuplicateUnitNames)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize schema with two units with the same name");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, DuplicateSchemaChildNames)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <PropertyCategory typeName="TestUnit"/>
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize schema with duplicate schema child names, one being a unit");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, ShouldFailWithBadUnitSystemName)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem:" displayLabel="Unit" definition="M" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with bad unit system name (fails parseClassName because of colon at end");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="badalias:TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with bad unit system alias");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="" displayLabel="Unit" definition="M" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with empty unit system name");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, ShouldFailWithBadPhenomName)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="BadPhenom:" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with a bad phenom name (fails parseclassname because of colon at end");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="badAlias:TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with a phenom with a bad alias");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with an empty phenom name");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, MissingOrInvalidName)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with missing name");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with empty name");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, MissingOrInvalidNumerator)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ECUnitCP unit = schema->GetUnitCP("Smoot");
    ASSERT_FALSE(unit->HasNumerator());
    ASSERT_DOUBLE_EQ(1.0, unit->GetNumerator());
    }

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="Smoots" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with non numeric numerator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with empty numerator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="0.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with 0.0 numerator");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, MissingOrInvalidDenominator)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ECUnitCP unit = schema->GetUnitCP("Smoot");
    ASSERT_FALSE(unit->HasDenominator());
    ASSERT_DOUBLE_EQ(1.0, unit->GetDenominator());
    }

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" denominator="Smoots" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an invalid (non-numeric) denominator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" denominator="0.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with a 0 denominator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" denominator="" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an empty denominator");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, MissingOrInvalidOffset)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    EXPECT_DOUBLE_EQ(schema->GetUnitCP("TestUnit")->GetOffset(), 0.0);

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit" offset="bananas"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an invalid (non-numeric) offset");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit" offset=""/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an empty offset");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, MissingDisplayLabel)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    ECUnitCP unit = schema->GetUnitCP("TestUnit");
    EXPECT_STREQ("TestUnit", unit->GetInvariantDisplayLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, MissingOrEmptyDefinition)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with missing definition");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" definition="" displayLabel="Unit" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with empty definintion");
    }

//=======================================================================================
//! InvertedUnitsTests
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InvertedUnitsTests, BasicCreation)
    {
    ECSchemaPtr schema;
    ECUnitP unit;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    EC_EXPECT_SUCCESS(schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, 10.0, 1.0, 1.0, "ExampleUnitLabel", "ExampleUnitDescription")); 
    ECUnitP invertedUnit;
    EC_EXPECT_SUCCESS(schema->CreateInvertedUnit(invertedUnit, *unit, "ExampleInvertedUnit", *system, "ExampleInvertedUnitLabel", "ExampleInvertedUnitDescription"));
    auto testECUnit = schema->GetUnitCP("ExampleUnit");
    EXPECT_EQ(unit, testECUnit);
    auto schemaInvertedUnit = schema->GetInvertedUnitCP("ExampleInvertedUnit");
    EXPECT_EQ(invertedUnit, schemaInvertedUnit);
    EXPECT_FALSE(invertedUnit->HasOffset());
    EXPECT_FALSE(invertedUnit->HasNumerator());
    EXPECT_FALSE(invertedUnit->HasDenominator());
    EXPECT_STRCASEEQ("ExampleInvertedUnit", invertedUnit->GetName().c_str());
    EXPECT_EQ(system, invertedUnit->GetUnitSystem());
    EXPECT_STRCASEEQ("ExampleInvertedUnitLabel", invertedUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STRCASEEQ("ExampleInvertedUnitDescription", invertedUnit->GetInvariantDescription().c_str());
    EXPECT_TRUE(invertedUnit->GetIsDisplayLabelDefined());
    EXPECT_TRUE(invertedUnit->GetIsDescriptionDefined());
    EXPECT_EQ(unit, invertedUnit->GetInvertingUnit());
    EXPECT_FALSE(invertedUnit->HasDefinition());
    EXPECT_STREQ(unit->GetDefinition().c_str(), invertedUnit->GetDefinition().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Kyle.Abramowitz                          02/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InvertedUnitsTests, StandaloneSchemaChild)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    ECUnitP unit;
    schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, 10.0, 1.0, 1.0, "ExampleUnitLabel", "ExampleUnitDescription");
    ECUnitP invUnit;
    schema->CreateInvertedUnit(invUnit, *unit, "ExampleInvertedUnit", *system, "ExampleUnitLabel", "ExampleUnitDescription");

    Json::Value schemaJson;
    EXPECT_TRUE(invUnit->ToJson(schemaJson, true));
    Json::Value testDataJson;

    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneInvertedUnit.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Kyle.Abramowitz                          03/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitsTests, WritingToPre32VersionShouldNotWriteInvertedUnit)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    ECUnitP unit;
    ECUnitP inv;
    schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, 10.0, 1.0, 1.0, "ExampleUnitLabel", "ExampleUnitDescription");
    schema->CreateInvertedUnit(inv, *unit, "Inv", *system);

    Utf8String out;
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(out, ECVersion::V3_1));
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, out.c_str(), *context);
    ASSERT_EQ(nullptr, schema->GetInvertedUnitCP("Inv"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InvertedUnitsDeserializationTests, RoundTripWithReferencedSchema)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00.00" alias="rs"/>
            <InvertedUnit typeName="TestInvertedUnit" invertsUnit="rs:TestUnit" unitSystem="rs:TestUnitSystem" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(1, schema->GetUnitCount());
    ECUnitCP unit = schema->GetInvertedUnitCP("TestInvertedUnit");
    ASSERT_TRUE(nullptr != unit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());
        
    EXPECT_STREQ("InvertedUnitLabel", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("InvertedUnitDescription", unit->GetInvariantDescription().c_str());
    EXPECT_TRUE(unit->GetIsDescriptionDefined());
    EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
    EXPECT_FALSE(unit->HasOffset());
    EXPECT_FALSE(unit->HasNumerator());
    EXPECT_FALSE(unit->HasDenominator());
    EXPECT_STREQ("LENGTH*LENGTH", unit->GetPhenomenon()->GetDefinition().c_str());
    EXPECT_STREQ("Unit System", ((ECN::UnitSystemCP)unit->GetUnitSystem())->GetInvariantDisplayLabel().c_str());
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    EXPECT_EQ(SchemaWriteStatus::Success, refSchema->WriteToXmlString(serializedRefSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaPtr serializedRefSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedRefSchema, serializedRefSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(1, serializedSchema->GetUnitCount());
    ECUnitCP serializedUnit = serializedSchema->GetInvertedUnitCP("TestInvertedUnit");
    ASSERT_TRUE(nullptr != serializedUnit);

    EXPECT_STREQ("InvertedUnitLabel", serializedUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("InvertedUnitDescription", serializedUnit->GetInvariantDescription().c_str());
    EXPECT_STREQ("LENGTH*LENGTH", serializedUnit->GetPhenomenon()->GetDefinition().c_str());
    EXPECT_STREQ("Unit System", ((ECN::UnitSystemCP)serializedUnit->GetUnitSystem())->GetInvariantDisplayLabel().c_str());
    EXPECT_TRUE(serializedUnit->GetIsDescriptionDefined());
    EXPECT_TRUE(serializedUnit->GetIsDisplayLabelDefined());
    EXPECT_FALSE(serializedUnit->HasOffset());
    EXPECT_FALSE(serializedUnit->HasNumerator());
    EXPECT_FALSE(serializedUnit->HasDenominator());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InvertedUnitsDeserializationTests, BasicRoundTripTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" numerator="1.0" definition="M" description="This is an awesome new Unit"/>
            <InvertedUnit typeName="TestInvertedUnit" invertsUnit="TestUnit" unitSystem="TestUnitSystem" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(2, schema->GetUnitCount());
    ECUnitCP unit = schema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != unit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());

    EXPECT_STREQ("Unit", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", unit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());

    ECUnitCP invUnit = schema->GetInvertedUnitCP("TestInvertedUnit");
    ASSERT_TRUE(nullptr != invUnit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());

    EXPECT_STREQ("TestUnitSystem",((ECN::UnitSystemCP)invUnit->GetUnitSystem())->GetName().c_str());
    EXPECT_STREQ("InvertedUnitDescription", invUnit->GetDescription().c_str());
    EXPECT_STREQ("InvertedUnitLabel", invUnit->GetInvariantDisplayLabel().c_str());
    ASSERT_TRUE(invUnit->IsInvertedUnit());
    EXPECT_TRUE(unit->HasUnitSystem());
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(2, serializedSchema->GetUnitCount());
    ECUnitCP serializedECUnit = serializedSchema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != serializedECUnit);
    ECUnitCP serializedInvertedUnit = serializedSchema->GetInvertedUnitCP("TestInvertedUnit");
    ASSERT_TRUE(nullptr != serializedInvertedUnit);

    EXPECT_STREQ("Unit", serializedECUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", serializedECUnit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", serializedECUnit->GetDefinition().c_str());

    EXPECT_STREQ("InvertedUnitLabel", serializedInvertedUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("InvertedUnitDescription", serializedInvertedUnit->GetDescription().c_str());
    ASSERT_TRUE(serializedInvertedUnit->IsInvertedUnit());
    EXPECT_TRUE(serializedInvertedUnit->HasUnitSystem());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InvertedUnitsDeserializationTests, ShouldFailWithoutAliases)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00.00" alias="rs"/>
            <InvertedUnit typeName="TestInvertedUnit" invertsUnit="TestUnit" unitSystem="TestUnitSystem" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InvertedUnitsDeserializationTests, ShouldFailWithoutUnitSystemDefined)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <InvertedUnit typeName="TestInvertedUnit" invertsUnit="TestUnit" unitSystem="bananas" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize inverted unit with a unit system that doesn't exist");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InvertedUnitsDeserializationTests, ShouldFailWithoutInvertsUnitDefinedOrEmptyOrMissingInvertsUnit)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <InvertedUnit typeName="TestInvertedUnit" invertsUnit="TestUnit" unitSystem="TestUnitSystem" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize an InvertedUnit with unit that doesn't exist");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <InvertedUnit typeName="TestInvertedUnit" invertsUnit="" unitSystem="TestUnitSystem" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize an Inverted unit with empty invertsUnit");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <InvertedUnit typeName="TestInvertedUnit" unitSystem="TestUnitSystem" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize an inverted unit with missing invertsUnit");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InvertedUnitsDeserializationTests, ShouldFailWithBadSchemaAliases)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="1.0.0" alias="u"/>
            <InvertedUnit typeName="TestInvertedUnit" invertsUnit="units:M" unitSystem="u:SI" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with bad schema alias on invertsUnit");
    }

//=======================================================================================
//! ConstantTests
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantTests, BasicCreation)
    {
    ECSchemaPtr schema;
    ECUnitP unit;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    PhenomenonP phenom;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    EC_EXPECT_SUCCESS(schema->CreateConstant(unit, "CONSTANT", "M", *phenom, 10, 10.0, "ConstantLabel", "ConstantDescription")); 
    
    EXPECT_STREQ("ConstantDescription", unit->GetInvariantDescription().c_str());
    EXPECT_TRUE(unit->GetIsDescriptionDefined());
    EXPECT_STREQ("ConstantLabel", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());
    EXPECT_STREQ("CONSTANT", unit->GetName().c_str());
    EXPECT_EQ(phenom, unit->GetPhenomenon());
    EXPECT_EQ(nullptr, unit->GetUnitSystem());
    EXPECT_EQ(10.0, unit->GetNumerator());
    EXPECT_TRUE(unit->HasNumerator());
    EXPECT_EQ(10.0, unit->GetDenominator());
    EXPECT_TRUE(unit->HasDenominator());
    EXPECT_FALSE(unit->HasOffset());
    EXPECT_FALSE(unit->HasUnitSystem());
    EXPECT_TRUE(unit->HasDefinition());

    auto testECUnit = schema->GetUnitCP("CONSTANT");
    EXPECT_EQ(unit, testECUnit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Kyle.Abramowitz                          02/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConstantTests, StandaloneSchemaChild)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    ECUnitP unit;
    schema->CreateConstant(unit, "ExampleConstant", "M", *phenom, 10.0, 1.0, "ExampleConstantLabel", "ExampleConstantDescription");

    Json::Value schemaJson;
    EXPECT_TRUE(unit->ToJson(schemaJson, true));

    Json::Value testDataJson;

    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneConstant.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Kyle.Abramowitz                          03/2018
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConstantTests, WritingToPre32VersionShouldNotWriteConstant)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);
    PhenomenonP phenom;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    ECUnitP unit;
    schema->CreateConstant(unit, "Constant", "M", *phenom, 10.0);

    Utf8String out;
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(out, ECVersion::V3_1));
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, out.c_str(), *context);
    ASSERT_EQ(nullptr, schema->GetConstantCP("Constant"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, BasicRoundTripTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0" denominator="1.0" />
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(1, schema->GetUnitCount());
    ECUnitCP unit = schema->GetConstantCP("TestConstant");
    ASSERT_TRUE(nullptr != unit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());

    EXPECT_STREQ("Constant", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Constant", unit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.0, unit->GetNumerator());
    EXPECT_DOUBLE_EQ(1.0, unit->GetDenominator());
    EXPECT_FALSE(unit->HasUnitSystem());

    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(1, serializedSchema->GetUnitCount());
    ECUnitCP serializedECUnit = serializedSchema->GetConstantCP("TestConstant");
    ASSERT_TRUE(nullptr != serializedECUnit);

    ASSERT_TRUE(serializedECUnit->IsConstant());
    EXPECT_STREQ("Constant", serializedECUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Constant", serializedECUnit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", serializedECUnit->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.0, serializedECUnit->GetNumerator());
    EXPECT_DOUBLE_EQ(1.0, serializedECUnit->GetDenominator());
    EXPECT_FALSE(serializedECUnit->HasUnitSystem());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, RoundTripWithReferencedSchemaForPhenomenonAndUnitSystem)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00.00" alias="rs"/>
            <Constant typeName="TestConstant" phenomenon="rs:TestPhenomenon" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(1, schema->GetUnitCount());
    ECUnitCP unit = schema->GetUnitCP("TestConstant");
    ASSERT_TRUE(nullptr != unit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());
        
    EXPECT_STREQ("Constant", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Constant", unit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());
    EXPECT_STREQ("LENGTH*LENGTH", unit->GetPhenomenon()->GetDefinition().c_str());
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    EXPECT_EQ(SchemaWriteStatus::Success, refSchema->WriteToXmlString(serializedRefSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaPtr serializedRefSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedRefSchema, serializedRefSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(1, serializedSchema->GetUnitCount());
    ECUnitCP serializedUnit = serializedSchema->GetUnitCP("TestConstant");
    ASSERT_TRUE(nullptr != serializedUnit);

    EXPECT_STREQ("Constant", serializedUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Constant", serializedUnit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", serializedUnit->GetDefinition().c_str());
    EXPECT_STREQ("LENGTH*LENGTH", serializedUnit->GetPhenomenon()->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.0, serializedUnit->GetNumerator());
    EXPECT_DOUBLE_EQ(1.0, serializedUnit->GetDenominator());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, ShouldFailWithoutAliasesOrBadAliases)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00.00" alias="rs"/>
            <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, ShouldFailWithoutPhenomenonAndUnitSystemBeingDefined)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant"  numerator="10.0"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, DuplicateConstantNames)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
            <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize schema with two constants with same name");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, ShouldFailWithInvalidDefinition)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="" description="This is an awesome new Constant" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should Fail to derserialize constatnt with empty defintion");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, MissingOrInvalidNumerator)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize Constant with missing numerator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="bananas"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize Constant with non numeric numerator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="bananas"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize Constant with empty numerator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="0.0"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize Constant with 0 numerator");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                    03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, MissingOrInvalidDenominator)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_DOUBLE_EQ(1.0, schema->GetConstantCP("Constant")->GetDenominator());
    }

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0" denominator="bananas"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialized with invalid denominator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0" denominator="0.0"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialized with 0 denominator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0" denominator=""/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialized with invalid denominator");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, MissingOrInvalidName)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize constant with missing name");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize constant with empty name");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, MissingDisplayLabel)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
    </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    ECUnitCP unit = schema->GetUnitCP("TestConstant");
    EXPECT_STREQ("TestConstant", unit->GetInvariantDisplayLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, MissingOrEmptyDefinition)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" description="This is an awesome new Constant" numerator="10.0"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize constant with missing definition");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="" description="This is an awesome new Constant" numerator="10.0"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize constant with empty defintion");
    }

END_BENTLEY_ECN_TEST_NAMESPACE
