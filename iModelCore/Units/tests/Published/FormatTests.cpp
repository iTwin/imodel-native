/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/Published/FormatTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../FormattingTestsPch.h"
#include "../TestFixture/FormattingTestFixture.h"

USING_BENTLEY_FORMATTING
BEGIN_BENTLEY_FORMATTEST_NAMESPACE

struct FormatTest : FormattingTestFixture {};
struct FormatStringTest : FormatTest
{
    // Valid format strings.
    Utf8String const fmtStrBasic = Utf8String("ExampleFmt<9>");
    Utf8String const fmtStrBasicTrailingComma = Utf8String("ExampleFmt<9,>");
    Utf8String const fmtStrBasicNoOverrides = Utf8String("ExampleFmt");
    Utf8String const fmtStrBasicUnitOverrideNoLabel = Utf8String("ExampleFmt[u:M]");
    Utf8String const fmtStrBasicUnitOverrideEmptyLabel = Utf8String("ExampleFmt[u:M|]");
    Utf8String const fmtStrBasicUnitOverrideWithLabel = Utf8String("ExampleFmt[u:|m]");

    Utf8String const fmtStrFutureAddition = Utf8String("ExampleFmt<9,banana>");
    Utf8String const fmtStrFutureAdditionWhiteSpace = Utf8String("ExampleFmt \n < 9 \t , banana > ");
    Utf8String const fmtStrFutureAdditionTrailingComma = Utf8String("ExampleFmt<9,banana,>");
    Utf8String const fmtStrFutureAdditionNoFirstOverride = Utf8String("ExampleFmt<,banana>");

    // Invalid format strings.
    Utf8String const fmtStrBasicNoOverridesButStillHasBrackets = Utf8String("ExampleFmt<>");
    Utf8String const fmtStrBasicNoOverridesButStillHasBracketsWithCommas = Utf8String("ExampleFmt<,,,,>");
    Utf8String const fmtStrWithEmptySqBrackets = Utf8String("ExampleFmt[]");
    Utf8String const fmtStrWithOnlyEmptySqBrackets = Utf8String("ExampleFmt[|]");
};

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, Constructors)
    {
    // Default constructed.
    {
    Format namedFmtSpec;

    EXPECT_EQ(FormatSpecType::None, namedFmtSpec.GetSpecType());
    EXPECT_TRUE(namedFmtSpec.IsProblem());
    EXPECT_EQ(nullptr, namedFmtSpec.GetNumericSpec());
    EXPECT_EQ(nullptr, namedFmtSpec.GetCompositeSpec());
    }

    // Constructed with NumericFormatSpec
    {
    NumericFormatSpec numFmtSpec;
    Format namedFmtSpec(numFmtSpec);

    EXPECT_EQ(FormatSpecType::None, namedFmtSpec.GetSpecType());
    EXPECT_FALSE(namedFmtSpec.IsProblem());
    ASSERT_NE(nullptr, namedFmtSpec.GetNumericSpec());
    EXPECT_TRUE(namedFmtSpec.GetNumericSpec()->IsIdentical(numFmtSpec));
    EXPECT_EQ(nullptr, namedFmtSpec.GetCompositeSpec());
    }

    // Constructed with name, NumericFormatSpec, and CompositeValueSpec
    {
    NumericFormatSpec numFmtSpec;
    CompositeValueSpec compValSpec(*s_unitsContext->LookupUnit("MILE"));
    Format namedFmtSpec(numFmtSpec, compValSpec);

    EXPECT_EQ(FormatSpecType::Single, namedFmtSpec.GetSpecType());
    EXPECT_FALSE(namedFmtSpec.IsProblem()) << namedFmtSpec.GetProblemDescription();
    ASSERT_NE(nullptr, namedFmtSpec.GetNumericSpec());
    EXPECT_TRUE(namedFmtSpec.GetNumericSpec()->IsIdentical(numFmtSpec));
    ASSERT_NE(nullptr, namedFmtSpec.GetCompositeSpec());
    // TODO: CompositeValueSpec::IsIdentical has a hacky unit proxy work around for now.
    //       Until that's cleaned up there's no point in testing the method or testing methods that
    //       call the method.
    //EXPECT_TRUE(namedFmtSpec.GetCompositeSpec()->IsIdentical(compValSpec));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, IsIdentical)
    {
    // Test format specs of varying value types.
    // In these cases, only NumericFormatSpecs that are compared against themselves
    // should be concidered identical by the identity principal.
    {
    NumericFormatSpec numFmtSpec;
    CompositeValueSpec compValSpec(*s_unitsContext->LookupUnit("MILE"));

    Format namedFmtSpecUndefined;
    Format namedFmtSpecNumeric(numFmtSpec);
    Format namedFmtSpecComposite(numFmtSpec, compValSpec);

    // Identity checks.
    EXPECT_TRUE(namedFmtSpecUndefined.IsIdentical(namedFmtSpecUndefined))  << "Format is not identical to itself.";
    EXPECT_TRUE(namedFmtSpecNumeric.IsIdentical(namedFmtSpecNumeric)) << "Format is not identical to itself.";
    // TODO: CompositeValueSpec::IsIdentical has a hacky unit proxy work around for now.
    //       Until that's cleaned up there's no point in testing the method or testing methods that
    //       call the method.
    //EXPECT_TRUE(namedFmtSpecComposite.IsIdentical(namedFmtSpecComposite)) << "Format is not identical to itself.";;

    EXPECT_FALSE(namedFmtSpecUndefined.IsIdentical(namedFmtSpecNumeric));
    EXPECT_FALSE(namedFmtSpecUndefined.IsIdentical(namedFmtSpecComposite));

    EXPECT_FALSE(namedFmtSpecNumeric.IsIdentical(namedFmtSpecUndefined));
    EXPECT_FALSE(namedFmtSpecNumeric.IsIdentical(namedFmtSpecComposite));

    // TODO: CompositeValueSpec::IsIdentical has a hacky unit proxy work around for now.
    //       Until that's cleaned up there's no point in testing the method or testing methods that
    //       call the method.
    //EXPECT_FALSE(namedFmtSpecComposite.IsIdentical(namedFmtSpecUndefined));
    //EXPECT_FALSE(namedFmtSpecComposite.IsIdentical(namedFmtSpecNumeric));
    }

    // Formats with differing NumericFormatSpecs.
    {
    NumericFormatSpec numFmtSpecA;
    numFmtSpecA.SetDecimalPrecision(DecimalPrecision::Precision8);
    NumericFormatSpec numFmtSpecB;
    numFmtSpecB.SetDecimalPrecision(DecimalPrecision::Precision9);

    Format namedFmtSpecA(numFmtSpecA);
    Format namedFmtSpecB(numFmtSpecB);
    EXPECT_FALSE(namedFmtSpecA.IsIdentical(namedFmtSpecB));
    EXPECT_FALSE(namedFmtSpecB.IsIdentical(namedFmtSpecA));
    }

    // Formats with differing CompositeValueSpecs.
    {
    NumericFormatSpec numFmtSpec;
    BEU::UnitCP mile = s_unitsContext->LookupUnit("MILE");
    CompositeValueSpec compValSpecA(*mile);
    CompositeValueSpec compValSpecB;
    compValSpecB.SetSpacer(" # ");

    Format namedFmtSpecA(numFmtSpec, compValSpecA);
    Format namedFmtSpecB(numFmtSpec, compValSpecB);
    EXPECT_FALSE(namedFmtSpecA.IsIdentical(namedFmtSpecB));
    EXPECT_FALSE(namedFmtSpecB.IsIdentical(namedFmtSpecA));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatTest, StdFormatQuantityUsesThousandSeparatorForAllUnits)
    {
    NumericFormatSpec numericFormatSpec;
    numericFormatSpec.SetThousandSeparator('\'');
    numericFormatSpec.SetUse1000Separator(true);
    numericFormatSpec.SetKeepSingleZero(true);

    BEU::UnitCP mile = s_unitsContext->LookupUnit("MILE");
    BEU::UnitCP inch = s_unitsContext->LookupUnit("IN");
    CompositeValueSpec compositeValueSpec(*mile, *inch);
    ASSERT_EQ(2, compositeValueSpec.GetUnitCount());
    Format Format(numericFormatSpec, compositeValueSpec);

    // 1500.5 miles == 1,500 miles and 31,680 inches
    BEU::Quantity quantity(1500.5, *compositeValueSpec.GetMajorUnit());
    EXPECT_STREQ("1'500.0 31'680.0", Format::StdFormatQuantity(Format, quantity).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatStringTest, FailUnknownFormatName)
    {
    static auto const nullMapper = [](Utf8StringCR name) -> FormatCP {return nullptr; };
    Format parsedNfs;

    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasic, nullMapper));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicTrailingComma, nullMapper));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicNoOverrides, nullMapper));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicUnitOverrideNoLabel, nullMapper, s_unitsContext));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicUnitOverrideEmptyLabel, nullMapper, s_unitsContext));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicUnitOverrideWithLabel, nullMapper, s_unitsContext));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAddition, nullMapper));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAdditionWhiteSpace, nullMapper));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAdditionTrailingComma, nullMapper));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAdditionNoFirstOverride, nullMapper));

    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicNoOverridesButStillHasBrackets, nullMapper));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicNoOverridesButStillHasBracketsWithCommas, nullMapper));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatStringTest, SuccessfullyParseKnownFormat)
    {
    NumericFormatSpec exampleNumericFmtSpec;
    exampleNumericFmtSpec.SetPresentationType(PresentationType::Decimal);
    exampleNumericFmtSpec.SetDecimalPrecision(DecimalPrecision::Precision9);
    Format const exampleNamedFmtSpec(exampleNumericFmtSpec);
    auto const mapper = [exampleNamedFmtSpec](Utf8StringCR name) -> FormatCP
        {
        return name == "ExampleFmt" ? &exampleNamedFmtSpec : nullptr;
        };
    Format parsedNfs;

    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasic, mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpec));

    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicTrailingComma, mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpec));

    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicNoOverrides, mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpec));

    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAddition, mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpec));

    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAdditionWhiteSpace, mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpec));

    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAdditionTrailingComma, mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpec));

    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrFutureAdditionNoFirstOverride, mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpec));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatStringTest, FailWithInvalidOverride)
    {
    Format const exampleNamedFmtSpec;
    auto const mapper = [exampleNamedFmtSpec](Utf8StringCR name) -> FormatCP
        {
        return name == "ExampleFmt" ? &exampleNamedFmtSpec : nullptr;
        };

    Format parsedNfs;
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicNoOverridesButStillHasBrackets, mapper));
    EXPECT_EQ(FormatProblemCode::NotInitialized, parsedNfs.GetProblem());
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicNoOverridesButStillHasBracketsWithCommas, mapper));
    EXPECT_EQ(FormatProblemCode::NotInitialized, parsedNfs.GetProblem());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatStringTest, ParseFormatString)
    {
    NumericFormatSpec exampleNumericFmtSpecDec;
    exampleNumericFmtSpecDec.SetPresentationType(PresentationType::Decimal);
    exampleNumericFmtSpecDec.SetDecimalPrecision(DecimalPrecision::Precision5);
    Format const exampleNamedFmtSpecDec(exampleNumericFmtSpecDec);

    NumericFormatSpec exampleNumericFmtSpecFrac;
    exampleNumericFmtSpecFrac.SetPresentationType(PresentationType::Fractional);
    exampleNumericFmtSpecFrac.SetFractionalPrecision(FractionalPrecision::Over_128);
    Format const exampleNamedFmtSpecFrac(exampleNumericFmtSpecFrac);

    NumericFormatSpec exampleNumericFmtSpecSci;
    exampleNumericFmtSpecSci.SetDecimalPrecision(DecimalPrecision::Precision4);
    exampleNumericFmtSpecSci.SetPresentationType(PresentationType::Scientific);
    Format const exampleNamedFmtSpecSci(exampleNumericFmtSpecSci);

    NumericFormatSpec exampleNumericFmtSpecSciNorm;
    exampleNumericFmtSpecSciNorm.SetDecimalPrecision(DecimalPrecision::Precision7);
    exampleNumericFmtSpecSciNorm.SetPresentationType(PresentationType::Scientific);
    exampleNumericFmtSpecSciNorm.SetScientificType(ScientificType::Normal);
    Format const exampleNamedFmtSpecSciNorm(exampleNumericFmtSpecSciNorm);

    NumericFormatSpec exampleNumericFmtSpecStation;
    exampleNumericFmtSpecStation.SetPresentationType(PresentationType::Station);
    exampleNumericFmtSpecStation.SetStationOffsetSize(2);
    exampleNumericFmtSpecStation.SetDecimalPrecision(DecimalPrecision::Precision9);
    Format const exampleNamedFmtSpecStation(exampleNumericFmtSpecStation);
    auto const mapper = [&](Utf8StringCR name) -> FormatCP
        {
        if (name == "ExDec")
            return &exampleNamedFmtSpecDec;
        if (name == "ExFrac")
            return &exampleNamedFmtSpecFrac;
        if (name == "ExSci")
            return &exampleNamedFmtSpecSci;
        if (name == "ExSciNorm")
            return &exampleNamedFmtSpecSciNorm;
        if (name == "ExStation")
            return &exampleNamedFmtSpecStation;
        return nullptr;
        };
    Format parsedNfs;

    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExDec<5>", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecDec));
    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExFrac<128>", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecFrac));
    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExSci<4>", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecSci));
    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExSciNorm<7>", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecSciNorm));
    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExStation<9>", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecStation));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
//TEST_F(FormatStringTest, InvalidUnitOverrides)
//    {
//    
//    }

END_BENTLEY_FORMATTEST_NAMESPACE