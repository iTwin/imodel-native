/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include "ECObjects/ECQuantityFormatting.h"
#include <Formatting/FormattingApi.h>

namespace BEU = BentleyApi::Units;
namespace BEF = BentleyApi::Formatting;

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ECQuantityFormattingTest : ECTestFixture {};

static void CheckFormattedQuantity(Utf8CP input, Utf8CP formatName, Utf8CP unitName, Utf8CP expectedOutput)
    {
    ECUnitCP unit = ECTestFixture::GetUnitsSchema()->GetUnitCP(unitName);
    ECFormatCP format = ECTestFixture::GetFormatsSchema()->GetFormatCP(formatName);
    EXPECT_TRUE(unit);
    EXPECT_TRUE(format);

    NamedFormat namedFormat(format->GetName() + "[" + unit->GetName().c_str() + "]", format);
    if (!namedFormat.HasComposite())
        {
        Formatting::CompositeValueSpec compositeSpec = Formatting::CompositeValueSpec(*unit);
        EXPECT_FALSE(compositeSpec.IsProblem()) << "Composite spec of " << formatName << " has problem " << compositeSpec.GetProblemDescription().c_str();
        if (compositeSpec.IsProblem())
            return;

        namedFormat.SetCompositeSpec(compositeSpec);
        EXPECT_FALSE(compositeSpec.IsProblem()) << "NamedFormat " << namedFormat.GetName().c_str() << " has problem " << compositeSpec.GetProblemDescription().c_str();
        if (compositeSpec.IsProblem())
            return;
        }

    Formatting::FormatProblemCode code;
    BEU::Quantity qty = ECQuantityFormatting::CreateQuantity(input, namedFormat, &code);
    Utf8String formattedQuantity = namedFormat.FormatQuantity(qty);

    EXPECT_STREQ(expectedOutput, formattedQuantity.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECQuantityFormattingTest, FormatVariousQuantities)
    {
    CheckFormattedQuantity("3ft 4in", "DefaultReal", "IN", "40.0");
    CheckFormattedQuantity("3ft 4in", "DefaultRealU", "IN", "40.0 in");
    CheckFormattedQuantity("3ft4in", "DefaultRealU", "IN", "40.0 in");
    CheckFormattedQuantity("3ft4 1/8in", "DefaultRealU", "IN", "40.125 in");
    CheckFormattedQuantity("3.6 FT", "DefaultReal", "IN", "43.2");
    CheckFormattedQuantity("3.75 FT", "DefaultRealU", "IN", "45.0 in");
    CheckFormattedQuantity("3 1/3ft", "DefaultRealU", "IN", "40.0 in");
    CheckFormattedQuantity("3 1/3ft", "DefaultRealU", "M", "1.016 m");
    CheckFormattedQuantity("3 1/3ft", "DefaultRealU", "MM", "1016.0 mm");
    CheckFormattedQuantity("5:6", "AmerFI", "MM", "5' 6\"");
    CheckFormattedQuantity("5:6", "AmerFI", "IN", "5' 6\"");
    CheckFormattedQuantity("5:", "AmerFI", "MM", "5' 0\"");
    CheckFormattedQuantity("5:", "AmerFI", "IN", "5' 0\"");
    CheckFormattedQuantity(":6", "AmerFI", "MM", "0' 6\"");
    CheckFormattedQuantity(":6", "AmerFI", "IN", "0' 6\"");
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(ECQuantityFormattingTest, TestWithOnlyMajorUnit)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());
    schema->AddReferencedSchema(*ECTestFixture::GetFormatsSchema());

    ECUnitCP ft = ECTestFixture::GetUnitsSchema()->GetUnitCP("FT");
    ECFormatCP fi = ECTestFixture::GetFormatsSchema()->GetFormatCP("AmerFi");

    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "Test");
    koq->SetPersistenceUnit(*ft);
    koq->SetDefaultPresentationFormat(*fi);

    Formatting::FormatProblemCode problem;
    BEU::Quantity newQuantity = ECQuantityFormatting::CreateQuantity("5", *fi, &problem);
    EXPECT_TRUE(ECUnit::AreEqual(ft, newQuantity.GetUnit()));
    EXPECT_EQ(5, newQuantity.GetMagnitude());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(ECQuantityFormattingTest, FormatPersistedValueUsingKoQPersistenceUnit)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());

    ECUnitCP meter = ECTestFixture::GetUnitsSchema()->GetUnitCP("M");

    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKindOfQuantity");
    koq->SetPersistenceUnit(*meter);

    ECQuantityFormattingStatus status;
    // TODO where to handle case where we have no presentation units but we have a persistence unit that we'd like to use as an input unit 
    // for defaultrealU
    BEF::NumericFormatSpec format = BEF::NumericFormatSpec::DefaultFormat();
    Utf8String formatString = ECQuantityFormatting::FormatPersistedValue(5, koq, &status);
    Utf8String expectedFormat = Utf8PrintfString("5%c0 m", format.GetDecimalSeparator());
    EXPECT_STREQ(expectedFormat.c_str(), formatString.c_str());
    }


//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(ECQuantityFormattingTest, FormatDoesNotShowNegativeZero)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    schema->AddReferencedSchema(*ECTestFixture::GetUnitsSchema());

    ECUnitCP pascal = ECTestFixture::GetUnitsSchema()->GetUnitCP("PA");
    ECUnitCP psig = ECTestFixture::GetUnitsSchema()->GetUnitCP("PSIG");
    ECFormatCP defaultRealU = ECTestFixture::GetFormatsSchema()->GetFormatCP("DefaultRealU");

    KindOfQuantityP koq;
    schema->CreateKindOfQuantity(koq, "TestKindOfQuantity");
    koq->SetPersistenceUnit(*pascal);
    koq->SetDefaultPresentationFormat(*defaultRealU, nullptr, psig, "");
    NamedFormatCP format = koq->GetDefaultPresentationFormat();


    ECQuantityFormattingStatus status;
    BEF::NumericFormatSpec defFormat = BEF::NumericFormatSpec::DefaultFormat();
    Utf8String expectedFormat = Utf8PrintfString("0%c0", defFormat.GetDecimalSeparator());
    Utf8String formatString = ECQuantityFormatting::FormatPersistedValue(101324.99999999997, koq, &status);
    EXPECT_STREQ(expectedFormat.c_str(), formatString.c_str());
    formatString = ECQuantityFormatting::FormatPersistedValue(101324.99999999997, koq, *format->GetCompositeMajorUnit(), *format, &status);
    EXPECT_STREQ(expectedFormat.c_str(), formatString.c_str());
    expectedFormat = Utf8PrintfString("-13%c231068", defFormat.GetDecimalSeparator());
    formatString = ECQuantityFormatting::FormatPersistedValue(10100.0, koq, &status);
    EXPECT_STREQ(expectedFormat.c_str(), formatString.c_str());
    formatString = ECQuantityFormatting::FormatPersistedValue(10100.0, koq, *format->GetCompositeMajorUnit(), *format, &status);
    EXPECT_STREQ(expectedFormat.c_str(), formatString.c_str());
    }

END_BENTLEY_ECN_TEST_NAMESPACE
