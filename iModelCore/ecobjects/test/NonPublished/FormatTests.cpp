/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct FormatTest : ECTestFixture {};
struct FormatRequiredAttributesTest : ECTestFixture {};
struct FormatOptionalAttributesTest : ECTestFixture {};
struct FormatJsonTests : ECTestFixture {};
struct CompositeTests : ECTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, BasicUnitFormatCreation)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    ECFormatP ufmt;
    Formatting::NumericFormatSpec format = Formatting::NumericFormatSpec();
    format.SetApplyRounding(true);
    format.SetPresentationType(Formatting::PresentationType::Fractional);
    format.SetPrecision(Formatting::FractionalPrecision::Over_128);
    EC_ASSERT_SUCCESS(schema->CreateFormat(ufmt, "TestFormat", "TestDisplayLabel", "a test format", &format));

    ASSERT_NE(nullptr, ufmt);
    EXPECT_STREQ("TestFormat", ufmt->GetName().c_str());
    EXPECT_STREQ("TestDisplayLabel", ufmt->GetDisplayLabel().c_str());
    EXPECT_STREQ("TestDisplayLabel", ufmt->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("a test format", ufmt->GetDescription().c_str());
    EXPECT_STREQ("a test format", ufmt->GetInvariantDescription().c_str());

    EXPECT_EQ(ufmt, schema->GetFormatP("TestFormat"));
    EXPECT_EQ(ufmt, schema->GetFormatCP("TestFormat"));
    ASSERT_NE(nullptr, ufmt->GetNumericSpec());
    EXPECT_EQ(Formatting::FractionalPrecision::Over_128, ufmt->GetNumericSpec()->GetFractionalPrecision());
    EXPECT_EQ(Formatting::PresentationType::Fractional, ufmt->GetNumericSpec()->GetPresentationType());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                      01/2019
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, EmptySpacerRoundTrips)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="MrNoSpacer" type="fractional" precision="4">
                <Composite spacer="" inputUnit="u:M">
                  <Unit label="mile(s)">u:MILE</Unit>
                  <Unit label="yrd(s)">u:YRD</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml";
    Utf8String serializedSchemaXml;

    static auto const verify = [](ECSchemaPtr schema) -> void
        {
        ECFormatCP ufmt = schema->GetFormatCP("MrNoSpacer");
        ASSERT_NE(nullptr, ufmt);
        ASSERT_EQ(schema.get(), &ufmt->GetSchema());
        ASSERT_STREQ("MrNoSpacer", ufmt->GetName().c_str());
        ASSERT_EQ(Formatting::PresentationType::Fractional, ufmt->GetNumericSpec()->GetPresentationType());
        ASSERT_STREQ("", ufmt->GetCompositeSpec()->GetSpacer().c_str());   
        };

    // Deserialize original XML and serialize it back out.
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    verify(schema);
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml, ECVersion::Latest));
    }
    // Deserialize roundtriped schema XML.
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, serializedSchemaXml.c_str(), *context));
    verify(schema);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                   03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, BasicRoundTripTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" displayLabel="myfi4" description="" roundFactor="0.0" type="fractional" signOption="onlyNegative" formatTraits="trailZeroes|prependUnitLabel" precision="4" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                <Composite spacer="-" inputUnit="u:M">
                  <Unit label="mile(s)">u:MILE</Unit>
                  <Unit label="yrd(s)">u:YRD</Unit>
                  <Unit label="'">u:FT</Unit>
                  <Unit label="&quot;">u:IN</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml";
    Utf8String serializedSchemaXml;

    static auto const verify = [](ECSchemaPtr schema) -> void
        {
        ECFormatCP ufmt = schema->GetFormatCP("AmerMYFI4");
        ASSERT_NE(nullptr, ufmt);
        ASSERT_EQ(schema.get(), &ufmt->GetSchema());
        ASSERT_STREQ("AmerMYFI4", ufmt->GetName().c_str());
        ASSERT_STREQ("myfi4", ufmt->GetDisplayLabel().c_str());
        ASSERT_STREQ("myfi4", ufmt->GetInvariantDisplayLabel().c_str());
        ASSERT_STREQ("", ufmt->GetDescription().c_str());
        ASSERT_STREQ("", ufmt->GetInvariantDescription().c_str());
        ASSERT_EQ(Formatting::PresentationType::Fractional, ufmt->GetNumericSpec()->GetPresentationType());
        ASSERT_EQ(Formatting::SignOption::OnlyNegative, ufmt->GetNumericSpec()->GetSignOption());
        ASSERT_EQ(Formatting::FractionalPrecision::Quarter, ufmt->GetNumericSpec()->GetFractionalPrecision());
        ASSERT_EQ('.', ufmt->GetNumericSpec()->GetDecimalSeparator());
        ASSERT_EQ(',', ufmt->GetNumericSpec()->GetThousandSeparator());
        ASSERT_DOUBLE_EQ(0.0, ufmt->GetNumericSpec()->GetRoundingFactor());
        ASSERT_STRCASEEQ(" ", ufmt->GetNumericSpec()->GetUomSeparator());

        bvector<Utf8String> traits;
        BeStringUtilities::Split(ufmt->GetNumericSpec()->GetFormatTraitsString().c_str(), "|", traits);
        ASSERT_EQ(2, traits.size());
        ASSERT_NE(nullptr, std::find(traits.begin(), traits.end(), "trailZeroes"));
        ASSERT_NE(nullptr, std::find(traits.begin(), traits.end(), "prependUnitName"));
        ASSERT_TRUE(ufmt->HasComposite());
        ASSERT_EQ(4, ufmt->GetCompositeSpec()->GetUnitCount());
        ASSERT_STRCASEEQ("MILE", ufmt->GetCompositeMajorUnit()->GetName().c_str());
        ASSERT_STRCASEEQ("mile(s)", ufmt->GetCompositeSpec()->GetMajorLabel().c_str());
        ASSERT_STRCASEEQ("YRD", ufmt->GetCompositeMiddleUnit()->GetName().c_str());
        ASSERT_STRCASEEQ("yrd(s)", ufmt->GetCompositeSpec()->GetMiddleLabel().c_str());
        ASSERT_STRCASEEQ("FT", ufmt->GetCompositeMinorUnit()->GetName().c_str());
        ASSERT_STRCASEEQ("'", ufmt->GetCompositeSpec()->GetMinorLabel().c_str());
        ASSERT_STRCASEEQ("IN", ufmt->GetCompositeSubUnit()->GetName().c_str());
        ASSERT_STRCASEEQ("\"", ufmt->GetCompositeSpec()->GetSubLabel().c_str());
        };

    // Deserialize original XML and serialize it back out.
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    verify(schema);
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml, ECVersion::Latest));
    }
    // Deserialize roundtriped schema XML.
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, serializedSchemaXml.c_str(), *context));
    verify(schema);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, SerializeStandaloneUnitFormat)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "es", 3, 2, 0);
    schema->AddReferencedSchema(*GetUnitsSchema());

    ECFormatP ufmt;
    EC_ASSERT_SUCCESS(schema->CreateFormat(ufmt, "AmerMYFI4", "myfi4", ""));
    ASSERT_NE(nullptr, ufmt);
    using namespace Formatting;
    NumericFormatSpec numeric = NumericFormatSpec();
    numeric.SetPresentationType(PresentationType::Fractional);
    numeric.SetRoundingFactor(0.0);
    numeric.SetSignOption(SignOption::OnlyNegative);
    numeric.SetKeepSingleZero(true);
    numeric.SetKeepTrailingZeroes(true);
    numeric.SetPrecision(FractionalPrecision::Quarter);
    numeric.SetDecimalSeparator('.');
    numeric.SetThousandSeparator(',');
    numeric.SetUomSeparator(" ");
    ufmt->SetNumericSpec(numeric);
    CompositeValueSpec comp = CompositeValueSpec(*schema->GetUnitsContext().LookupUnit("u:MILE"), *schema->GetUnitsContext().LookupUnit("u:YRD"), *schema->GetUnitsContext().LookupUnit("u:FT"), *schema->GetUnitsContext().LookupUnit("u:IN"));
    comp.SetSpacer("-");
    comp.SetMajorLabel("mile(s)");
    comp.SetMiddleLabel("yrd(s)");
    comp.SetMinorLabel("'");
    comp.SetSubLabel("\"");
    ufmt->SetCompositeSpec(comp);

    Json::Value schemaJson;
    ufmt->ToJson(schemaJson, false);

    Json::Value testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneFormat.ecschema.json"));
    ASSERT_EQ(BentleyStatus::SUCCESS, ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile));

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, VerifyDefaults)
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    auto format = schema->GetFormatCP("AmerMYFI4");
    auto nfs = format->GetNumericSpec();
    EXPECT_STRCASEEQ("AmerMYFI4", format->GetName().c_str());
    EXPECT_FALSE(format->GetIsDisplayLabelDefined());
    EXPECT_FALSE(format->GetIsDescriptionDefined());
    EXPECT_STRCASEEQ(format->GetName().c_str(), format->GetInvariantDisplayLabel().c_str());
    EXPECT_EQ(Formatting::PresentationType::Decimal, nfs->GetPresentationType());
    EXPECT_FALSE(nfs->HasRoundingFactor());
    EXPECT_DOUBLE_EQ(0.0, nfs->GetRoundingFactor());
    EXPECT_TRUE(nfs->HasPrecision());
    EXPECT_EQ(Formatting::DecimalPrecision::Precision4, nfs->GetDecimalPrecision());
    EXPECT_EQ(Formatting::FractionalPrecision::Over_64, nfs->GetFractionalPrecision());
    EXPECT_FALSE(nfs->HasMinWidth());
    EXPECT_DOUBLE_EQ(0.0, nfs->GetMinWidth());
    EXPECT_FALSE(nfs->HasSignOption());
    EXPECT_EQ(Formatting::SignOption::OnlyNegative, nfs->GetSignOption());
    EXPECT_FALSE(nfs->HasFormatTraits());
    EXPECT_EQ(Formatting::FormatTraits::None, nfs->GetFormatTraits());
    EXPECT_FALSE(nfs->HasDecimalSeparator());
    EXPECT_EQ(std::use_facet<std::numpunct<Utf8Char>>(std::locale("")).decimal_point(), nfs->GetDecimalSeparator());
    EXPECT_FALSE(nfs->HasThousandsSeparator());
    EXPECT_EQ(std::use_facet<std::numpunct<Utf8Char>>(std::locale("")).thousands_sep(), nfs->GetThousandSeparator());
    EXPECT_FALSE(nfs->HasUomSeparator());
    EXPECT_STRCASEEQ(" ", nfs->GetUomSeparator());
    EXPECT_FALSE(nfs->HasStationSeparator());
    EXPECT_EQ('+', nfs->GetStationSeparator());
    EXPECT_FALSE(format->HasComposite());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     04/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, VerifyRoundTripMappings)
    {
    auto schema = ECTestFixture::GetFormatsSchema();
    bvector<Utf8String> ignoredNames = {"FORMATS:StationUNS_100"};
    for (const auto& f : schema->GetFormats())
        {
        if (ignoredNames.end() == std::find(ignoredNames.begin(), ignoredNames.end(), f->GetFullName()))
            continue;
        auto legacy = Formatting::LegacyNameMappings::TryGetLegacyNameFromFormatString(f->GetFullName().c_str());
        EXPECT_NE(nullptr, legacy);
        auto alias = Formatting::AliasMappings::TryGetAliasFromName(legacy);
        EXPECT_NE(nullptr, alias);
        legacy = Formatting::AliasMappings::TryGetNameFromAlias(alias);
        auto roundTrippedName = Formatting::LegacyNameMappings::TryGetFormatStringFromLegacyName(legacy);
        ASSERT_NE(nullptr, roundTrippedName) << "Failed to round trip name " << f->GetFullName().c_str();
        Utf8String _alias;
        Utf8String name;
        ECClass::ParseClassName(_alias, name, roundTrippedName);
        auto format = schema->GetFormatCP(name.c_str());
        EXPECT_TRUE(f->IsIdentical(*format));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                  05/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, LookupFormatTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Formats" version="1.0.0" alias="f"/>
            <Format typeName="myformat" type="decimal" precision="4"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    auto shouldBeNull = schema->LookupFormat("");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupFormat("banana");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupFormat("Formats:AmerFI");
    ASSERT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupFormat("f:AmerFI", true);
    ASSERT_EQ(nullptr, shouldBeNull);
    auto shouldNotBeNull = schema->LookupFormat("myformat");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("myformat", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupFormat("ts:myformat");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("myformat", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupFormat("TestSchema:myformat", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("myformat", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupFormat("f:AmerFI");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("AmerFI", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupFormat("Formats:AmerFI", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("AmerFI", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupFormat("TS:myformat");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("myformat", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupFormat("TESTSCHEMA:myformat", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("myformat", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupFormat("F:AmerFI");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("AmerFI", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupFormat("FORMATS:AmerFI", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("AmerFI", shouldNotBeNull->GetName().c_str());
    ASSERT_EQ(1, schema->GetFormatCount());

    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatRequiredAttributesTest, MissingOrInvalidType)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format  typeName="AmerMYFI4" precision="4" />
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with missing presentation type");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format  typeName="AmerMYFI4" type="" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with empty presentation type");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="bananas" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid presentation type");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatRequiredAttributesTest, VerifyAllowedPresentationTypes)
    {
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="fractional" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::PresentationType::Fractional, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetPresentationType());
    }
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::PresentationType::Decimal, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetPresentationType());
    }
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="scientific" scientificType="normalized" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::PresentationType::Scientific, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetPresentationType());
    EXPECT_EQ(Formatting::ScientificType::Normalized, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetScientificType());
    }
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="station" stationOffsetSize="2" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::PresentationType::Station, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetPresentationType());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatRequiredAttributesTest, MissingOrInvalidName)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format type="fractional" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with missing typeName");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="" type="fractional" precision="4" />
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with empty typeName");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u" precision="4"/>
            <Format typeName="...." type="fractional"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid typeName");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatRequiredAttributesTest, ScientificTypeShouldBeRequiredWhenTypeIsScientific)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="scientific" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with missing scientific type when type is scientific");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="scientific" scientificType="bananas" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid scientific type when type is scientific");
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="scientific" scientificType="zeroNormalized" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::ScientificType::ZeroNormalized, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetScientificType());
    }
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="scientific" scientificType="normalized" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::ScientificType::Normalized, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetScientificType());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatRequiredAttributesTest, StationOffsetSizeShouldBeRequiredWhenTypeIsStation)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="station" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with missing stationoffsetsize when type is station");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="station" stationOffsetSize="bananas" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid stationoffsetsize when type is station");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="station" stationOffsetSize="-1" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid stationoffsetsize when type is station");
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="station" stationOffsetSize="3" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(3, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetStationOffsetSize());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatRequiredAttributesTest, VerifyValidDecimalPrecision)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with missing decimal precision");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" precision=""/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with empty decimal precision");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" precision="-1"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with negative decimal precision");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" precision="bananas"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid decimal precision");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" precision="13"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with decimal precision higher than max");
    for (int i = 0; i <= 12; ++i)
        { 
        Utf8String goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <Format typeName="AmerMYFI4" type="decimal" precision=")xml";
        goodSchemaXml += std::to_string(i).c_str();
        goodSchemaXml += R"xml(" /> </ECSchema>)xml";
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        ECSchemaPtr schema;
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml.c_str(), *context));
        ASSERT_EQ(static_cast<Formatting::DecimalPrecision>(i), schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetDecimalPrecision());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatRequiredAttributesTest, VerifyValidFractionalPrecision)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="fractional"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with missing fractional precision");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="fractional" precision=""/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with empty fractional precision");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="fractional" precision="-1"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with negative fractional precision");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="fractional" precision="13"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid fractional precision");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="fractional" precision="bananas"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid fractional precision");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="fractional" precision="512"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with fractional precision higher than max");
    int i = 1;
    int j = 0;
    while(i <= 256)
        { 
        Utf8String goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
                <Format typeName="AmerMYFI4" type="fractional" precision=")xml";
        goodSchemaXml += std::to_string(i).c_str();
        goodSchemaXml += R"xml(" /> </ECSchema>)xml";
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        ECSchemaPtr schema;
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml.c_str(), *context));
        ASSERT_EQ(static_cast<Formatting::FractionalPrecision>(j), schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetFractionalPrecision());
        i<<=1;
        j++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatOptionalAttributesTest, VerifyAllowedShowSignOptions)
    {
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="fractional" showSignOption="noSign" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::SignOption::NoSign, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetSignOption());
    }
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="fractional" showSignOption="onlyNegative" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::SignOption::OnlyNegative, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetSignOption());
    }
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="fractional" showSignOption="signAlways" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::SignOption::SignAlways, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetSignOption());
    }
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="fractional" showSignOption="negativeParentheses" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::SignOption::NegativeParentheses, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetSignOption());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatOptionalAttributesTest, VerifyValidFormatTraits)
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="fractional" formatTraits="trailZeroes|keepSingleZero|zeroEmpty|keepDecimalPoint|applyRounding|fractionDash|showUnitLabel|prependUnitLabel|use1000Separator|exponentOnlyNegative" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    auto nfs = schema->GetFormatCP("AmerMYFI4")->GetNumericSpec();
    EXPECT_TRUE(nfs->GetTraitBit(Formatting::FormatTraits::TrailingZeroes));
    EXPECT_TRUE(nfs->GetTraitBit(Formatting::FormatTraits::KeepSingleZero));
    EXPECT_TRUE(nfs->GetTraitBit(Formatting::FormatTraits::ZeroEmpty));
    EXPECT_TRUE(nfs->GetTraitBit(Formatting::FormatTraits::KeepDecimalPoint));
    EXPECT_TRUE(nfs->GetTraitBit(Formatting::FormatTraits::ApplyRounding));
    EXPECT_TRUE(nfs->GetTraitBit(Formatting::FormatTraits::FractionDash));
    EXPECT_TRUE(nfs->GetTraitBit(Formatting::FormatTraits::ShowUnitLabel));
    EXPECT_TRUE(nfs->GetTraitBit(Formatting::FormatTraits::PrependUnitLabel));
    EXPECT_TRUE(nfs->GetTraitBit(Formatting::FormatTraits::Use1000Separator));
    EXPECT_TRUE(nfs->GetTraitBit(Formatting::FormatTraits::ExponenentOnlyNegative));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatOptionalAttributesTest, InvalidOrEmptyRoundingFactor)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" roundFactor="" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with empty round factor");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" roundFactor="bananas" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid round factor");
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    auto nfs = schema->GetFormatCP("AmerMYFI4")->GetNumericSpec();
    EXPECT_FALSE(nfs->HasRoundingFactor());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatOptionalAttributesTest, InvalidOrEmptyMinWidth)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" minWidth="" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with empty minwidth");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" minWidth="bananas" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid minwidth");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" minWidth="-1" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid minwidth");
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    auto nfs = schema->GetFormatCP("AmerMYFI4")->GetNumericSpec();
    EXPECT_FALSE(nfs->HasMinWidth());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatOptionalAttributesTest, InvalidOrEmptyDecimalSeparator)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" decimalSeparator="" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with empty decSeparator");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" decimalSeparator="bananas" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid decSeparator");
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    auto nfs = schema->GetFormatCP("AmerMYFI4")->GetNumericSpec();
    EXPECT_FALSE(nfs->HasDecimalSeparator());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatOptionalAttributesTest, InvalidOrEmptyThousandSeparator)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" thousandSeparator="" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with empty thousandSeparator");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" thousandSeparator="bananas" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid thousandSeparator");
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    auto nfs = schema->GetFormatCP("AmerMYFI4")->GetNumericSpec();
    EXPECT_FALSE(nfs->HasThousandsSeparator());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatOptionalAttributesTest, InvalidOrEmptyStationSeparator)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" stationSeparator="" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with empty statSeparator");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" stationSeparator="bananas" precision="4"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid statSeparator");
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Format typeName="AmerMYFI4" type="decimal" precision="4"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    auto nfs = schema->GetFormatCP("AmerMYFI4")->GetNumericSpec();
    EXPECT_FALSE(nfs->HasStationSeparator());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatJsonTests, FromJson)
    {
    Utf8String json = R"json({"composite":{"includeZero":true,"spacer":" ","units":[{"label":"'","name":"Units.FT"}]},"formatTraits":"KeepSingleZero|KeepDecimalPoint|ShowUnitLabel","precision":4,"type":"Decimal","uomSeparator":""})json";

    auto jsonValue = Json::Value::From(json);
    ECSchemaPtr out;
    ECSchema::CreateSchema(out, "test", "t", 1, 0, 0);
    out->AddReferencedSchema(*GetUnitsSchema());
    NamedFormat format;
    ASSERT_TRUE(NamedFormat::FromJson(format, jsonValue, &out->GetUnitsContext()));
    ASSERT_TRUE(format.HasCompositeMajorUnit());
    EXPECT_STRCASEEQ("FT", format.GetCompositeMajorUnit()->GetName().c_str());
    EXPECT_STRCASEEQ("Units.FT", ECJsonUtilities::ECNameToJsonName(*(ECUnitCP)format.GetCompositeMajorUnit()).c_str());
    EXPECT_EQ(Formatting::PresentationType::Decimal, format.GetNumericSpec()->GetPresentationType());
    EXPECT_EQ(Formatting::DecimalPrecision::Precision4, format.GetNumericSpec()->GetDecimalPrecision());
    EXPECT_STRCASEEQ("", format.GetNumericSpec()->GetUomSeparator());
    EXPECT_TRUE(format.GetNumericSpec()->IsKeepSingleZero());
    EXPECT_TRUE(format.GetNumericSpec()->IsKeepDecimalPoint());
    EXPECT_TRUE(format.GetNumericSpec()->IsShowUnitLabel());
    EXPECT_TRUE(format.GetCompositeSpec()->IsIncludeZero());
    EXPECT_STRCASEEQ(" ", format.GetCompositeSpec()->GetSpacer().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     07/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatJsonTests, JsonRoundTrip)
    {
    Utf8String json = R"json({"composite":{"includeZero":true,"spacer":" ","units":[{"label":"'","name":"Units.FT"}]},"formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"],"precision":4,"type":"Decimal","uomSeparator":""})json";
    auto original = Json::Value::From(json);
    ECSchemaPtr out;
    ECSchema::CreateSchema(out, "test", "t", 1, 0, 0);
    out->AddReferencedSchema(*GetUnitsSchema());
    NamedFormat format;
    ASSERT_TRUE(NamedFormat::FromJson(format, original, &out->GetUnitsContext()));
    Json::Value roundTripped;
    format.ToJson(roundTripped, false);
    Json::Reader::Parse(json, original);
    ASSERT_TRUE(ECTestUtility::JsonDeepEqual(original, roundTripped)) << ECTestUtility::JsonSchemasComparisonString(roundTripped, original);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     07/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatJsonTests, InvalidJson)
    {

    {
    Utf8String json = R"json({"composite":{"includeZero":true,"spacer":" ","units":[{"label":"'","name":"u.FT"}]},"formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"],"precision":4,"type":"Decimal","uomSeparator":""})json";
    auto original = Json::Value::From(json);
    ECSchemaPtr out;
    ECSchema::CreateSchema(out, "test", "t", 1, 0, 0);
    out->AddReferencedSchema(*GetUnitsSchema());
    NamedFormat format;
    ASSERT_FALSE(NamedFormat::FromJson(format, original, &out->GetUnitsContext()));
    }

    {
    Utf8String json = R"json({"composite":{"includeZero":true,"spacer":" ","units":[{"label":"'","name":"Units:FT"}]},"formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"],"precision":4,"type":"Decimal","uomSeparator":""})json";
    auto original = Json::Value::From(json);
    ECSchemaPtr out;
    ECSchema::CreateSchema(out, "test", "t", 1, 0, 0);
    out->AddReferencedSchema(*GetUnitsSchema());
    NamedFormat format;
    ASSERT_FALSE(NamedFormat::FromJson(format, original, &out->GetUnitsContext()));
    }

    {
    Utf8String json = R"json({"composite":{"includeZero":true,"spacer":" ","units":[{"label":"'","name":"u:FT"}]},"formatTraits": ["keepSingleZero", "keepDecimalPoint", "showUnitLabel"],"precision":4,"type":"Decimal","uomSeparator":""})json";
    auto original = Json::Value::From(json);
    ECSchemaPtr out;
    ECSchema::CreateSchema(out, "test", "t", 1, 0, 0);
    out->AddReferencedSchema(*GetUnitsSchema());
    NamedFormat format;
    ASSERT_FALSE(NamedFormat::FromJson(format, original, &out->GetUnitsContext()));
    }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeTests, VerifyDefaults)
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="4">
                <Composite inputUnit="u:M">
                    <Unit>u:FT</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    auto format = schema->GetFormatCP("AmerMYFI4");
    ASSERT_TRUE(format->HasComposite());
    auto cfs = format->GetCompositeSpec();
    ASSERT_EQ(1, cfs->GetUnitCount());
    EXPECT_FALSE(cfs->HasSpacer());
    EXPECT_STRCASEEQ(" ", cfs->GetSpacer().c_str());
    EXPECT_NE(nullptr, cfs->GetMajorUnit());
    EXPECT_FALSE(cfs->HasMajorLabel());
    EXPECT_STRCASEEQ("FT", cfs->GetMajorLabel().c_str());
    EXPECT_FALSE(cfs->HasMiddleLabel());
    EXPECT_FALSE(cfs->HasMinorLabel());
    EXPECT_FALSE(cfs->HasSubLabel());
    EXPECT_EQ(nullptr, cfs->GetMiddleUnit());
    EXPECT_EQ(nullptr, cfs->GetMinorUnit());
    EXPECT_EQ(nullptr, cfs->GetSubUnit());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeTests, InvalidNumberOfUnits)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="4">
                <Composite inputUnit="u:M">
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with no units in a composite");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="4">
                <Composite inputUnit="u:M">
                    <Unit>u:MILE</Unit>
                    <Unit>u:YRD</Unit>
                    <Unit>u:FT</Unit>
                    <Unit>u:IN</Unit>
                    <Unit>u:CM</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with more than 4 units in a composite");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeTests, DuplicateUnits)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="4">
                <Composite inputUnit="u:M">
                    <Unit>u:MILE</Unit>
                    <Unit>u:MILE</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with more than 4 units in a composite");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeTests, InvalidUnits)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="4">
                <Composite inputUnit="u:M">
                    <Unit>u:BANANA</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid major unit");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="4">
                <Composite inputUnit="u:M">
                    <Unit>u:MILE</Unit>
                    <Unit>u:BANANA</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid middle unit");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="4">
                <Composite inputUnit="u:M">
                    <Unit>u:MILE</Unit>
                    <Unit>u:YRD</Unit>
                    <Unit>u:BANANA</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid minor unit");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="4">
                <Composite inputUnit="u:M">
                    <Unit>u:MILE</Unit>
                    <Unit>u:YRD</Unit>
                    <Unit>u:FT</Unit>
                    <Unit>u:BANANA</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid sub unit");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeTests, UnqualifiedUnits)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="4">
                <Composite inputUnit="u:M">
                    <Unit>MILE</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with unqualified major unit");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="4">
                <Composite inputUnit="u:M">
                    <Unit>u:MILE</Unit>
                    <Unit>YRD</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with unqualified middle unit");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="4">
                <Composite inputUnit="u:M">
                    <Unit>u:MILE</Unit>
                    <Unit>u:YRD</Unit>
                    <Unit>FT</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with unqualified minor unit");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="4">
                <Composite inputUnit="u:M">
                    <Unit>u:MILE</Unit>
                    <Unit>u:YRD</Unit>
                    <Unit>u:FT</Unit>
                    <Unit>IN</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with unqualified sub unit");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeTests, UseLocallyDefinedUnits)
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Unit typeName="SMOOT" definition="u:MILE" phenomenon="u:LENGTH" unitSystem="u:SI"/>
            <Unit typeName="CSMOOT" definition="u:YRD" phenomenon="u:LENGTH" unitSystem="u:SI"/>
            <Unit typeName="MSMOOT" definition="u:FT" phenomenon="u:LENGTH" unitSystem="u:SI"/>
            <Unit typeName="USMOOT" definition="u:IN" phenomenon="u:LENGTH" unitSystem="u:SI"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="4">
                <Composite inputUnit="u:M">
                    <Unit>SMOOT</Unit>
                    <Unit>CSMOOT</Unit>
                    <Unit>MSMOOT</Unit>
                    <Unit>USMOOT</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    auto format = schema->GetFormatCP("AmerMYFI4");
    ASSERT_TRUE(format->HasComposite());
    auto cfs = format->GetCompositeSpec();
    ASSERT_EQ(4, cfs->GetUnitCount());
    EXPECT_FALSE(cfs->HasMajorLabel());
    EXPECT_FALSE(cfs->HasMiddleLabel());
    EXPECT_FALSE(cfs->HasMinorLabel());
    EXPECT_FALSE(cfs->HasSubLabel());
    EXPECT_NE(nullptr, cfs->GetMajorUnit());
    EXPECT_NE(nullptr, schema->GetUnitCP(cfs->GetMajorUnit()->GetName().c_str()));
    EXPECT_NE(nullptr, cfs->GetMiddleUnit());
    EXPECT_NE(nullptr, schema->GetUnitCP(cfs->GetMiddleUnit()->GetName().c_str()));
    EXPECT_NE(nullptr, cfs->GetMinorUnit());
    EXPECT_NE(nullptr, schema->GetUnitCP(cfs->GetMinorUnit()->GetName().c_str()));
    EXPECT_NE(nullptr, cfs->GetSubUnit());
    EXPECT_NE(nullptr, schema->GetUnitCP(cfs->GetSubUnit()->GetName().c_str()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeTests, IncompatibleUnits)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="4">
                <Composite>
                    <Unit>u:ACRE</Unit>
                    <Unit>u:M</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with incompatible units");
    }
END_BENTLEY_ECN_TEST_NAMESPACE