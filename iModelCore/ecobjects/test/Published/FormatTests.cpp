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


//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, BasicUnitFormatCreation)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    FormatP ufmt;
    Formatting::NumericFormatSpec format;
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
// @bsimethod                                   Victor.Cushman                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, BasicRoundTripTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" version="01.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00" alias="u"/>
            <Format typeName="AmerMYFI4" displayLabel="myfi4" description="" roundFactor="0.0" type="fractional" signOption="onlyNegative" formatTraits="trailZeroes|prependUnitName" precision="4" fracBarType="diagonal" decSeparator="." thousandSeparator="," uomSeparator=" ">
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
        ASSERT_EQ(Formatting::FractionBarType::Diagonal, ufmt->GetNumericSpec()->GetFractionalBarType());
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
// @bsimethod                                   Victor.Cushman                  02/2018
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, SerializeStandaloneUnitFormat)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5);

    FormatP ufmt;
    EC_ASSERT_SUCCESS(schema->CreateFormat(ufmt, "TestFormat", "TestDisplayLabel", "a test unit format"));
    ASSERT_NE(nullptr, ufmt);

    Json::Value schemaJson;
    ASSERT_EQ(SchemaWriteStatus::Success, ufmt->WriteJson(schemaJson, true));

    Json::Value testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneFormat.ecschema.json"));
    ASSERT_EQ(BentleyStatus::SUCCESS, ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile));

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

END_BENTLEY_ECN_TEST_NAMESPACE