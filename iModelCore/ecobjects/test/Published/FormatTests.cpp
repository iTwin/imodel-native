/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/FormatTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct FormatTest : ECTestFixture {};
struct FormatRequiredAttributesTest : ECTestFixture {};
struct FormatOptionalAttributesTest : ECTestFixture {};
struct CompositeTests : ECTestFixture {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, BasicUnitFormatCreation)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    FormatP ufmt;
    Formatting::NumericFormatSpec format = Formatting::NumericFormatSpec();
    format.SetApplyRounding(true);
    format.SetPresentationType(Formatting::PresentationType::Fractional);
    format.SetFractionalPrecision(Formatting::FractionalPrecision::Over_128);
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
// @bsimethod                                 Kyle.Abramowitz                   03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, BasicRoundTripTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" displayLabel="myfi4" description="" roundFactor="0.0" type="fractional" signOption="onlyNegative" formatTraits="trailZeroes|prependUnitLabel" precision="4" decSeparator="." thousandSeparator="," uomSeparator=" ">
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
        FormatCP ufmt = schema->GetFormatCP("AmerMYFI4");
        ASSERT_NE(nullptr, ufmt);
        ASSERT_EQ(schema.get(), &ufmt->GetSchema());
        ASSERT_STREQ("AmerMYFI4", ufmt->GetName().c_str());
        ASSERT_STREQ("myfi4", ufmt->GetDisplayLabel().c_str());
        ASSERT_STREQ("myfi4", ufmt->GetInvariantDisplayLabel().c_str());
        ASSERT_STREQ("", ufmt->GetDescription().c_str());
        ASSERT_STREQ("", ufmt->GetInvariantDescription().c_str());
        ASSERT_EQ(Formatting::PresentationType::Fractional, ufmt->GetNumericSpec()->GetPresentationType());
        ASSERT_EQ(Formatting::ShowSignOption::OnlyNegative, ufmt->GetNumericSpec()->GetSignOption());
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
        ASSERT_STRCASEEQ("M", ufmt->GetCompositeSpec()->GetInputUnit()->GetName().c_str());
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
    schema->AddReferencedSchema(*GetUnitsSchema(true));

    FormatP ufmt;
    EC_ASSERT_SUCCESS(schema->CreateFormat(ufmt, "AmerMYFI4", "myfi4", ""));
    ASSERT_NE(nullptr, ufmt);
    using namespace Formatting;
    NumericFormatSpec numeric = NumericFormatSpec();
    numeric.SetPresentationType(PresentationType::Fractional);
    numeric.SetRoundingFactor(0.0);
    numeric.SetSignOption(ShowSignOption::OnlyNegative);
    numeric.SetUseLeadingZeroes(true);
    numeric.SetKeepTrailingZeroes(true);
    numeric.SetFractionalPrecision(FractionalPrecision::Quarter);
    numeric.SetDecimalSeparator('.');
    numeric.SetThousandSeparator(',');
    numeric.SetUomSeparator(" ");
    ufmt->SetNumericSpec(numeric);
    CompositeValueSpec comp = CompositeValueSpec(*schema->GetUnitsContext().LookupUnit("u:MILE"), *schema->GetUnitsContext().LookupUnit("u:YRD"), *schema->GetUnitsContext().LookupUnit("u:FT"), *schema->GetUnitsContext().LookupUnit("u:IN"));
    comp.SetSpacer("-");
    comp.SetInputUnit(schema->GetUnitsContext().LookupUnit("u:M"));
    comp.SetMajorLabel("mile(s)");
    comp.SetMiddleLabel("yrd(s)");
    comp.SetMinorLabel("'");
    comp.SetSubLabel("\"");
    ufmt->SetCompositeSpec(comp);

    Json::Value schemaJson;
    ASSERT_EQ(SchemaWriteStatus::Success, ufmt->WriteJson(schemaJson, true));

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
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal"/>
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
    EXPECT_FALSE(nfs->HasPrecision());
    EXPECT_EQ(Formatting::DecimalPrecision::Precision6, nfs->GetDecimalPrecision());
    EXPECT_EQ(Formatting::FractionalPrecision::Over_64, nfs->GetFractionalPrecision());
    EXPECT_FALSE(nfs->HasMinWidth());
    EXPECT_DOUBLE_EQ(0.0, nfs->GetMinWidth());
    EXPECT_FALSE(nfs->HasSignOption());
    EXPECT_EQ(Formatting::ShowSignOption::OnlyNegative, nfs->GetSignOption());
    EXPECT_FALSE(nfs->HasFormatTraits());
    EXPECT_EQ(Formatting::FormatTraits::None, nfs->GetFormatTraits());
    EXPECT_FALSE(nfs->HasDecimalSeparator());
    EXPECT_EQ('.', nfs->GetDecimalSeparator());
    EXPECT_FALSE(nfs->HasThousandsSeparator());
    EXPECT_EQ(',', nfs->GetThousandSeparator());
    EXPECT_FALSE(nfs->HasUomSeparator());
    EXPECT_STRCASEEQ(" ", nfs->GetUomSeparator());
    EXPECT_FALSE(nfs->HasPrefixPadChar());
    EXPECT_EQ('\0', nfs->GetPrefixPadChar());
    EXPECT_FALSE(nfs->HasStationSeparator());
    EXPECT_EQ('+', nfs->GetStationSeparator());
    EXPECT_FALSE(format->HasComposite());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatRequiredAttributesTest, MissingOrInvalidType)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format  typeName="AmerMYFI4" displayLabel="myfi4" description="" roundFactor="0.0" signOption="onlyNegative" formatTraits="trailZeroes|prependUnitName" precision="4" decSeparator="." thousandSeparator="," uomSeparator=" ">
                <Composite spacer="-" inputUnit="u:M">
                  <Unit label="mile(s)">u:MILE</Unit>
                  <Unit label="yrd(s)">u:YRD</Unit>
                  <Unit label="'">u:FT</Unit>
                  <Unit label="&quot;">u:IN</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with missing presentation type");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format  typeName="AmerMYFI4" displayLabel="myfi4" description="" roundFactor="0.0" type="" signOption="onlyNegative" formatTraits="trailZeroes|prependUnitName" precision="4" decSeparator="." thousandSeparator="," uomSeparator=" ">
                <Composite spacer="-" inputUnit="u:M">
                  <Unit label="mile(s)">u:MILE</Unit>
                  <Unit label="yrd(s)">u:YRD</Unit>
                  <Unit label="'">u:FT</Unit>
                  <Unit label="&quot;">u:IN</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with empty presentation type");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" displayLabel="myfi4" description="" roundFactor="0.0" type="bananas" signOption="onlyNegative" formatTraits="trailZeroes|prependUnitName" precision="4" decSeparator="." thousandSeparator="," uomSeparator=" ">
                <Composite spacer="-" inputUnit="u:M">
                  <Unit label="mile(s)">u:MILE</Unit>
                  <Unit label="yrd(s)">u:YRD</Unit>
                  <Unit label="'">u:FT</Unit>
                  <Unit label="&quot;">u:IN</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid presentation type");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatRequiredAttributesTest, VerifyAllowedPresentationTypes)
    {
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" displayLabel="myfi4" description="" roundFactor="0.0" type="fractional"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::PresentationType::Fractional, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetPresentationType());
    }
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" displayLabel="myfi4" description="" roundFactor="0.0" type="decimal"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::PresentationType::Decimal, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetPresentationType());
    }
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" displayLabel="myfi4" description="" roundFactor="0.0" type="scientific" scientificType="standard"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::PresentationType::Scientific, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetPresentationType());
    }
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" displayLabel="myfi4" description="" roundFactor="0.0" type="station" stationOffsetSize="2"/>
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
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format displayLabel="myfi4" description="" roundFactor="0.0" type="fractional" signOption="onlyNegative" formatTraits="trailZeroes|prependUnitName" precision="4" decSeparator="." thousandSeparator="," uomSeparator=" ">
                <Composite spacer="-" inputUnit="u:M">
                  <Unit label="mile(s)">u:MILE</Unit>
                  <Unit label="yrd(s)">u:YRD</Unit>
                  <Unit label="'">u:FT</Unit>
                  <Unit label="&quot;">u:IN</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with missing typeName");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="" displayLabel="myfi4" description="" roundFactor="0.0" type="fractional" signOption="onlyNegative" formatTraits="trailZeroes|prependUnitName" precision="4" decSeparator="." thousandSeparator="," uomSeparator=" ">
                <Composite spacer="-" inputUnit="u:M">
                  <Unit label="mile(s)">u:MILE</Unit>
                  <Unit label="yrd(s)">u:YRD</Unit>
                  <Unit label="'">u:FT</Unit>
                  <Unit label="&quot;">u:IN</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with empty typeName");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="...." displayLabel="myfi4" description="" roundFactor="0.0" type="fractional" signOption="onlyNegative" formatTraits="trailZeroes|prependUnitName" precision="4" decSeparator="." thousandSeparator="," uomSeparator=" ">
                <Composite spacer="-" inputUnit="u:M">
                  <Unit label="mile(s)">u:MILE</Unit>
                  <Unit label="yrd(s)">u:YRD</Unit>
                  <Unit label="'">u:FT</Unit>
                  <Unit label="&quot;">u:IN</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid typeName");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatRequiredAttributesTest, ScientificTypeShouldBeRequiredWhenTypeIsScientific)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" displayLabel="myfi4" description="" roundFactor="0.0" type="scientific">
                <Composite spacer="-" inputUnit="u:M">
                  <Unit label="mile(s)">u:MILE</Unit>
                  <Unit label="yrd(s)">u:YRD</Unit>
                  <Unit label="'">u:FT</Unit>
                  <Unit label="&quot;">u:IN</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with missing scientific type when type is scientific");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" displayLabel="myfi4" description="" roundFactor="0.0" type="scientific" scientificType="bananas">
                <Composite spacer="-" inputUnit="u:M">
                  <Unit label="mile(s)">u:MILE</Unit>
                  <Unit label="yrd(s)">u:YRD</Unit>
                  <Unit label="'">u:FT</Unit>
                  <Unit label="&quot;">u:IN</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid scientific type when type is scientific");
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" displayLabel="myfi4" description="" roundFactor="0.0" type="scientific" scientificType="normal">
                <Composite spacer="-" inputUnit="u:M">
                  <Unit label="mile(s)">u:MILE</Unit>
                  <Unit label="yrd(s)">u:YRD</Unit>
                  <Unit label="'">u:FT</Unit>
                  <Unit label="&quot;">u:IN</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::ScientificType::Normal, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetScientificType());
    }
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" displayLabel="myfi4" description="" roundFactor="0.0" type="scientific" scientificType="engineering">
                <Composite spacer="-" inputUnit="u:M">
                  <Unit label="mile(s)">u:MILE</Unit>
                  <Unit label="yrd(s)">u:YRD</Unit>
                  <Unit label="'">u:FT</Unit>
                  <Unit label="&quot;">u:IN</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::ScientificType::Engineering, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetScientificType());
    }
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" displayLabel="myfi4" description="" roundFactor="0.0" type="scientific" scientificType="standard">
                <Composite spacer="-" inputUnit="u:M">
                  <Unit label="mile(s)">u:MILE</Unit>
                  <Unit label="yrd(s)">u:YRD</Unit>
                  <Unit label="'">u:FT</Unit>
                  <Unit label="&quot;">u:IN</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::ScientificType::Standard, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetScientificType());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatRequiredAttributesTest, StationOffsetSizeShouldBeRequiredWhenTypeIsStation)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="station"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with missing stationoffsetsize when type is station");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="station" stationOffsetSize="bananas"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid stationoffsetsize when type is scientific");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="station" stationOffsetSize="-1"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid stationoffsetsize when type is scientific");
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="station" stationOffsetSize="3"/>
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
TEST_F(FormatOptionalAttributesTest, VerifyAllowedShowSignOptions)
    {
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="fractional" showSignOption="noSign"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::ShowSignOption::NoSign, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetSignOption());
    }
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="fractional" showSignOption="onlyNegative"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::ShowSignOption::OnlyNegative, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetSignOption());
    }
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="fractional" showSignOption="signAlways"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::ShowSignOption::SignAlways, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetSignOption());
    }
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="fractional" showSignOption="negativeParentheses"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    EXPECT_EQ(Formatting::ShowSignOption::NegativeParentheses, schema->GetFormatCP("AmerMYFI4")->GetNumericSpec()->GetSignOption());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatOptionalAttributesTest, VerifyValidFormatTraits)
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="fractional" formatTraits="leadZeroes|trailZeroes|keepSingleZero|zeroEmpty|keepDecimalPoint|applyRounding|fractionDash|showUnitLabel|prependUnitLabel|use1000Separator|exponentOnlyNegative"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    auto nfs = schema->GetFormatCP("AmerMYFI4")->GetNumericSpec();
    EXPECT_TRUE(nfs->GetTraitsBit(Formatting::FormatTraits::LeadingZeroes));
    EXPECT_TRUE(nfs->GetTraitsBit(Formatting::FormatTraits::TrailingZeroes));
    EXPECT_TRUE(nfs->GetTraitsBit(Formatting::FormatTraits::KeepSingleZero));
    EXPECT_TRUE(nfs->GetTraitsBit(Formatting::FormatTraits::ZeroEmpty));
    EXPECT_TRUE(nfs->GetTraitsBit(Formatting::FormatTraits::KeepDecimalPoint));
    EXPECT_TRUE(nfs->GetTraitsBit(Formatting::FormatTraits::ApplyRounding));
    EXPECT_TRUE(nfs->GetTraitsBit(Formatting::FormatTraits::FractionDash));
    EXPECT_TRUE(nfs->GetTraitsBit(Formatting::FormatTraits::ShowUnitLabel));
    EXPECT_TRUE(nfs->GetTraitsBit(Formatting::FormatTraits::PrependUnitLabel));
    EXPECT_TRUE(nfs->GetTraitsBit(Formatting::FormatTraits::Use1000Separator));
    EXPECT_TRUE(nfs->GetTraitsBit(Formatting::FormatTraits::ExponenentOnlyNegative));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatOptionalAttributesTest, VerifyValidDecimalPrecision)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="-1"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with negative decimal precision");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="bananas"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid decimal precision");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" precision="13"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with decimal precision higher than max");
    for (int i = 0; i <= 12; ++i)
        { 
        Utf8String goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="Units" version="01.00" alias="u"/>
                <Format typeName="AmerMYFI4" type="decimal" precision=")xml";
        goodSchemaXml += std::to_string(i).c_str();
        goodSchemaXml += R"xml(" /> </ECSchema>)xml";
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        ECSchemaPtr schema;
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml.c_str(), *context));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatOptionalAttributesTest, VerifyValidFractionalPrecision)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="fractional" precision="-1"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with negative fractional precision");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="fractional" precision="13"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid fractional precision");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="fractional" precision="bananas"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid fractional precision");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="fractional" precision="512"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with fractional precision higher than max");
    int i = 0;
    while(i <= 256)
        { 
        Utf8String goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="Units" version="01.00" alias="u"/>
                <Format typeName="AmerMYFI4" type="fractional" precision=")xml";
        goodSchemaXml += std::to_string(i).c_str();
        goodSchemaXml += R"xml(" /> </ECSchema>)xml";
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        ECSchemaPtr schema;
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml.c_str(), *context));
        if (0 == i)
            i+=2;
        else
            i<<=1;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatOptionalAttributesTest, InvalidOrEmptyRoundingFactor)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" roundFactor=""/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with empty round factor");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" roundFactor="bananas"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid round factor");
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal"/>
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
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" minWidth=""/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with empty minwidth");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" minWidth="bananas"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid minwidth");
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal"/>
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
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" decSeparator=""/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with empty decSeparator");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" decSeparator="bananas"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid decSeparator");
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal"/>
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
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" thousandSeparator=""/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with empty thousandSeparator");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" thousandSeparator="bananas"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid thousandSeparator");
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal"/>
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
TEST_F(FormatOptionalAttributesTest, InvalidOrEmptyPrefixPadChar)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" prefixPadChar=""/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with empty prefixPadChar");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" prefixPadChar="bananas"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid prefixPadChar");
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal"/>
        </ECSchema>)xml";
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, goodSchemaXml, *context));
    auto nfs = schema->GetFormatCP("AmerMYFI4")->GetNumericSpec();
    EXPECT_FALSE(nfs->HasPrefixPadChar());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatOptionalAttributesTest, InvalidOrEmptyStationSeparator)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" statSeparator=""/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with empty statSeparator");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal" statSeparator="bananas"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail with invalid statSeparator");
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal"/>
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
TEST_F(CompositeTests, VerifyDefaults)
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
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
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
                <Composite inputUnit="u:M">
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with no units in a composite");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
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
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
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
TEST_F(CompositeTests, InvalidInputUnit)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
                <Composite inputUnit="u:BANANA">
                    <Unit>u:MILE</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid input unit");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
                <Composite inputUnit="M">
                    <Unit>u:MILE</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with unqualified input unit");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeTests, InvalidUnits)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
                <Composite inputUnit="u:M">
                    <Unit>u:BANANA</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid major unit");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
                <Composite inputUnit="u:M">
                    <Unit>u:MILE</Unit>
                    <Unit>u:BANANA</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid middle unit");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
                <Composite inputUnit="u:M">
                    <Unit>u:MILE</Unit>
                    <Unit>u:YRD</Unit>
                    <Unit>u:BANANA</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with invalid minor unit");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
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
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
                <Composite inputUnit="u:M">
                    <Unit>MILE</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with unqualified major unit");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
                <Composite inputUnit="u:M">
                    <Unit>u:MILE</Unit>
                    <Unit>YRD</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with unqualified middle unit");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
                <Composite inputUnit="u:M">
                    <Unit>u:MILE</Unit>
                    <Unit>u:YRD</Unit>
                    <Unit>FT</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with unqualified minor unit");
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
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
TEST_F(CompositeTests, UseLocallyDefinedInputUnit)
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Unit typeName="SMOOT" definition="u:M" phenomenon="u:LENGTH" unitSystem="u:SI"/>
            <Format typeName="AmerMYFI4" type="decimal">
                <Composite inputUnit="SMOOT">
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
    EXPECT_STRCASEEQ("SMOOT", cfs->GetInputUnit()->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeTests, UseLocallyDefinedUnits)
    {
    Utf8CP goodSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Unit typeName="SMOOT" definition="u:MILE" phenomenon="u:LENGTH" unitSystem="u:SI"/>
            <Unit typeName="CSMOOT" definition="u:YRD" phenomenon="u:LENGTH" unitSystem="u:SI"/>
            <Unit typeName="MSMOOT" definition="u:FT" phenomenon="u:LENGTH" unitSystem="u:SI"/>
            <Unit typeName="USMOOT" definition="u:IN" phenomenon="u:LENGTH" unitSystem="u:SI"/>
            <Format typeName="AmerMYFI4" type="decimal">
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
    EXPECT_NE(nullptr, cfs->GetMiddleUnit());
    EXPECT_NE(nullptr, cfs->GetMinorUnit());
    EXPECT_NE(nullptr, cfs->GetSubUnit());
    EXPECT_STRCASEEQ("M", cfs->GetInputUnit()->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeTests, IncompatibleInputAndUnit)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
                <Composite inputUnit="u:M">
                    <Unit>u:ACRE</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with incompaitlbe input unit and major unit");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                               Kyle.Abramowitz                     03/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(CompositeTests, IncompatibleUnits)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" type="decimal">
                <Composite>
                    <Unit>u:ACRE</Unit>
                    <Unit>u:M</Unit>
                </Composite>
            </Format>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with incompatible units");
    }
END_BENTLEY_ECN_TEST_NAMESPACE