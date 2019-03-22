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
static BEU::UnitCP s_mile = nullptr;
static BEU::UnitCP s_yrd = nullptr;
static BEU::UnitCP s_ft = nullptr;
static BEU::UnitCP s_inch = nullptr;
struct FormatTest : FormattingTestFixture {};
struct FormatJsonTest : FormattingTestFixture 
{
    void SetUp() override
        {
        FormattingTestFixture::SetUp();

        if (nullptr == s_mile)
            s_mile = s_unitsContext->LookupUnit("MILE");
        if (nullptr == s_yrd)
            s_yrd = s_unitsContext->LookupUnit("YRD");
        if (nullptr == s_ft)
            s_ft = s_unitsContext->LookupUnit("FT");
        if (nullptr == s_inch)
            s_inch = s_unitsContext->LookupUnit("IN");
        }
};
struct FormatStringTest : FormatTest
{
    // Valid format strings.
    Utf8String const fmtStrBasic = Utf8String("ExampleFmt(9)");
    Utf8String const fmtStrBasicTrailingComma = Utf8String("ExampleFmt(9,)");
    Utf8String const fmtStrBasicNoOverrides = Utf8String("ExampleFmt");
    Utf8String const fmtStrBasicUnitOverrideNoLabel = Utf8String("ExampleFmt[u:M]");
    Utf8String const fmtStrBasicUnitOverrideEmptyLabel = Utf8String("ExampleFmt[u:M|]");
    Utf8String const fmtStrBasicUnitOverrideWithLabel = Utf8String("ExampleFmt[u:|m]");

    Utf8String const fmtStrFutureAddition = Utf8String("ExampleFmt(9,banana)");
    Utf8String const fmtStrFutureAdditionWhiteSpace = Utf8String("ExampleFmt \n ( 9 \t , banana ) ");
    Utf8String const fmtStrFutureAdditionTrailingComma = Utf8String("ExampleFmt(9,banana,)");
    Utf8String const fmtStrFutureAdditionNoFirstOverride = Utf8String("ExampleFmt(,banana)");

    //Evil format strings that should work
    Utf8String const fmtStrDoubleBar = Utf8String("ExampleFmt[M||]");
    Utf8String const fmtStrLeftBracket = Utf8String("ExampleFmt[M|[]");
    Utf8String const fmtStrParens = Utf8String("ExampleFmt[M|()]");
    Utf8String const fmtStrSemicolon = Utf8String("ExampleFmt[M|;]");

    // Invalid format strings.
    Utf8String const fmtStrBasicNoOverridesButStillHasBrackets = Utf8String("ExampleFmt()");
    Utf8String const fmtStrBasicHasBananaForPrecision = Utf8String("ExampleFmt(banana)");
    Utf8String const fmtStrBasicNoOverridesButStillHasBracketsWithCommas = Utf8String("ExampleFmt(,,,,)");
    Utf8String const fmtStrWithEmptySqBrackets = Utf8String("ExampleFmt[]");
    Utf8String const fmtStrWithOnlyEmptySqBrackets = Utf8String("ExampleFmt[|]");
    Utf8String const fmtStrUnit5Overrides = Utf8String("ExampleFmt[M|label][DM|label2][CM|label3][MM|label4][UM|label5]");
    Utf8String const fmtStrInvalidUnit = Utf8String("ExampleFmt[banana|label]");
    Utf8String const fmtStrWrongOrderOfUnits = Utf8String("ExampleFmt[FT][Mile]");
    Utf8String const fmtStrEmptyUnit = Utf8String("ExampleFmt[|banana]");

    // Invalid and evil strings
    Utf8String const fmtStrRightBracket = Utf8String("ExampleFmt[u:M|]]");
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
    EXPECT_TRUE(namedFmtSpec.GetCompositeSpec()->IsIdentical(compValSpec));
    }

    // Copy constructor
    {
    NumericFormatSpec numFmtSpec;
    CompositeValueSpec compValSpec(*s_unitsContext->LookupUnit("MILE"));
    Format namedFmtSpec(numFmtSpec, compValSpec);

    EXPECT_EQ(FormatSpecType::Single, namedFmtSpec.GetSpecType());
    EXPECT_FALSE(namedFmtSpec.IsProblem()) << namedFmtSpec.GetProblemDescription();
    ASSERT_NE(nullptr, namedFmtSpec.GetNumericSpec());
    EXPECT_TRUE(namedFmtSpec.GetNumericSpec()->IsIdentical(numFmtSpec));
    ASSERT_NE(nullptr, namedFmtSpec.GetCompositeSpec());
    EXPECT_TRUE(namedFmtSpec.GetCompositeSpec()->IsIdentical(compValSpec));

    Format copied = Format(namedFmtSpec);
    EXPECT_TRUE(copied.IsIdentical(namedFmtSpec));
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
    EXPECT_TRUE(namedFmtSpecComposite.IsIdentical(namedFmtSpecComposite)) << "Format is not identical to itself.";;

    EXPECT_FALSE(namedFmtSpecUndefined.IsIdentical(namedFmtSpecNumeric));
    EXPECT_FALSE(namedFmtSpecUndefined.IsIdentical(namedFmtSpecComposite));

    EXPECT_FALSE(namedFmtSpecNumeric.IsIdentical(namedFmtSpecUndefined));
    EXPECT_FALSE(namedFmtSpecNumeric.IsIdentical(namedFmtSpecComposite));
    EXPECT_FALSE(namedFmtSpecComposite.IsIdentical(namedFmtSpecUndefined));
    EXPECT_FALSE(namedFmtSpecComposite.IsIdentical(namedFmtSpecNumeric));
    }

    // Formats with differing NumericFormatSpecs.
    {
    NumericFormatSpec numFmtSpecA;
    numFmtSpecA.SetPrecision(DecimalPrecision::Precision8);
    NumericFormatSpec numFmtSpecB;
    numFmtSpecB.SetPrecision(DecimalPrecision::Precision9);

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

//===================================================
// FormatStringTest
//===================================================

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
    exampleNumericFmtSpec.SetPrecision(DecimalPrecision::Precision9);
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
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrUnit5Overrides, mapper));
    EXPECT_EQ(FormatProblemCode::NotInitialized, parsedNfs.GetProblem());
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrInvalidUnit, mapper, s_unitsContext));
    EXPECT_EQ(FormatProblemCode::NotInitialized, parsedNfs.GetProblem());
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrWrongOrderOfUnits, mapper, s_unitsContext));
    EXPECT_EQ(FormatProblemCode::NotInitialized, parsedNfs.GetProblem());
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrBasicHasBananaForPrecision, mapper));
    EXPECT_EQ(FormatProblemCode::NotInitialized, parsedNfs.GetProblem());
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "", mapper));
    EXPECT_EQ(FormatProblemCode::NotInitialized, parsedNfs.GetProblem());
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrEmptyUnit, mapper));
    EXPECT_EQ(FormatProblemCode::NotInitialized, parsedNfs.GetProblem());
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrRightBracket, mapper));
    EXPECT_EQ(FormatProblemCode::NotInitialized, parsedNfs.GetProblem());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(FormatStringTest, ParseFormatString)
    {
    NumericFormatSpec exampleNumericFmtSpecDec;
    exampleNumericFmtSpecDec.SetPresentationType(PresentationType::Decimal);
    exampleNumericFmtSpecDec.SetPrecision(DecimalPrecision::Precision5);
    Format const exampleNamedFmtSpecDec(exampleNumericFmtSpecDec);

    NumericFormatSpec exampleNumericFmtSpecFrac;
    exampleNumericFmtSpecFrac.SetPresentationType(PresentationType::Fractional);
    exampleNumericFmtSpecFrac.SetPrecision(FractionalPrecision::Over_128);
    Format const exampleNamedFmtSpecFrac(exampleNumericFmtSpecFrac);

    NumericFormatSpec exampleNumericFmtSpecSci;
    exampleNumericFmtSpecSci.SetPrecision(DecimalPrecision::Precision4);
    exampleNumericFmtSpecSci.SetPresentationType(PresentationType::Scientific);
    Format const exampleNamedFmtSpecSci(exampleNumericFmtSpecSci);

    NumericFormatSpec exampleNumericFmtSpecSciNorm;
    exampleNumericFmtSpecSciNorm.SetPrecision(DecimalPrecision::Precision7);
    exampleNumericFmtSpecSciNorm.SetPresentationType(PresentationType::Scientific);
    exampleNumericFmtSpecSciNorm.SetScientificType(ScientificType::ZeroNormalized);
    Format const exampleNamedFmtSpecSciNorm(exampleNumericFmtSpecSciNorm);

    NumericFormatSpec exampleNumericFmtSpecStation;
    exampleNumericFmtSpecStation.SetPresentationType(PresentationType::Station);
    exampleNumericFmtSpecStation.SetStationOffsetSize(2);
    exampleNumericFmtSpecStation.SetPrecision(DecimalPrecision::Precision9);
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

    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExDec(5)", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecDec));
    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExFrac(128)", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecFrac));
    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExSci(4)", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecSci));
    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExSciNorm(7)", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecSciNorm));
    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExStation(9)", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecStation));
    ASSERT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, "ExStation(9)", mapper));
    EXPECT_TRUE(parsedNfs.IsIdentical(exampleNamedFmtSpecStation));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
TEST_F(FormatStringTest, InvalidUnitOverrides)
    {
    Format const exampleNamedFmtSpec;
    auto const mapper = [exampleNamedFmtSpec](Utf8StringCR name) -> FormatCP
        {
        return name == "ExampleFmt" ? &exampleNamedFmtSpec : nullptr;
        };

    Format parsedNfs;
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrWithEmptySqBrackets, mapper, s_unitsContext));
    EXPECT_NE(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrWithOnlyEmptySqBrackets, mapper, s_unitsContext));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(FormatStringTest, UnitAndLabelOverride)
    {
    Format const exampleNamedFmtSpec;
    auto const mapper = [exampleNamedFmtSpec](Utf8StringCR name) -> FormatCP
        {
        return name == "ExampleFmt" ? &exampleNamedFmtSpec : nullptr;
        };

    Utf8String const fmtStrWithMeter("ExampleFmt[M]");
    Utf8String const fmtStrWithMeterNoLabel("ExampleFmt[M|]");
    Utf8String const fmtStrUnitOverride("ExampleFmt[M|label]");
    Utf8String const fmtStrUnit2Overrides("ExampleFmt[M|label][DM|label2]");
    Utf8String const fmtStrUnit3Overrides("ExampleFmt[M|label][DM|label2][CM|label3]");
    Utf8String const fmtStrUnit4Overrides("ExampleFmt[M|label][DM|label2][CM|label3][MM|label4]");

    Format parsedNfs;
    EXPECT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrWithMeter, mapper, s_unitsContext));
    CompositeValueSpecP spec = parsedNfs.GetCompositeSpecP();
    ASSERT_NE(nullptr, spec);
    EXPECT_STRCASEEQ(spec->GetMajorUnit()->GetName().c_str(), "M");
    EXPECT_FALSE(spec->HasMajorLabel());
    EXPECT_STRCASEEQ(spec->GetMajorLabel().c_str(), "M");

    parsedNfs = Format();
    EXPECT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrWithMeterNoLabel, mapper, s_unitsContext));
    spec = parsedNfs.GetCompositeSpecP();
    ASSERT_NE(nullptr, spec);
    EXPECT_STRCASEEQ(spec->GetMajorUnit()->GetName().c_str(), "M");
    EXPECT_TRUE(spec->HasMajorLabel());
    EXPECT_STRCASEEQ("", spec->GetMajorLabel().c_str());

    parsedNfs = Format();
    EXPECT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrUnitOverride, mapper, s_unitsContext));
    spec = parsedNfs.GetCompositeSpecP();
    ASSERT_NE(nullptr, spec);
    EXPECT_STRCASEEQ(spec->GetMajorUnit()->GetName().c_str(), "M");
    EXPECT_TRUE(spec->HasMajorLabel());
    EXPECT_STRCASEEQ("label", spec->GetMajorLabel().c_str());

    parsedNfs = Format();
    EXPECT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrUnit2Overrides, mapper, s_unitsContext));
    spec = parsedNfs.GetCompositeSpecP();
    ASSERT_NE(nullptr, spec);
    EXPECT_STRCASEEQ(spec->GetMajorUnit()->GetName().c_str(), "M");
    EXPECT_TRUE(spec->HasMajorLabel());
    EXPECT_STRCASEEQ("label", spec->GetMajorLabel().c_str());
    EXPECT_STRCASEEQ(spec->GetMiddleUnit()->GetName().c_str(), "DM");
    EXPECT_TRUE(spec->HasMiddleLabel());
    EXPECT_STRCASEEQ("label2", spec->GetMiddleLabel().c_str());

    parsedNfs = Format();
    EXPECT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrUnit3Overrides, mapper, s_unitsContext));
    spec = parsedNfs.GetCompositeSpecP();
    ASSERT_NE(nullptr, spec);
    EXPECT_STRCASEEQ(spec->GetMajorUnit()->GetName().c_str(), "M");
    EXPECT_TRUE(spec->HasMajorLabel());
    EXPECT_STRCASEEQ("label", spec->GetMajorLabel().c_str());
    EXPECT_STRCASEEQ(spec->GetMiddleUnit()->GetName().c_str(), "DM");
    EXPECT_TRUE(spec->HasMiddleLabel());
    EXPECT_STRCASEEQ("label2", spec->GetMiddleLabel().c_str());
    EXPECT_STRCASEEQ(spec->GetMinorUnit()->GetName().c_str(), "CM");
    EXPECT_TRUE(spec->HasMinorLabel());
    EXPECT_STRCASEEQ("label3", spec->GetMinorLabel().c_str());

    parsedNfs = Format();
    EXPECT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrUnit4Overrides, mapper, s_unitsContext));
    spec = parsedNfs.GetCompositeSpecP();
    ASSERT_NE(nullptr, spec);
    EXPECT_STRCASEEQ(spec->GetMajorUnit()->GetName().c_str(), "M");
    EXPECT_TRUE(spec->HasMajorLabel());
    EXPECT_STRCASEEQ("label", spec->GetMajorLabel().c_str());
    EXPECT_STRCASEEQ(spec->GetMiddleUnit()->GetName().c_str(), "DM");
    EXPECT_TRUE(spec->HasMiddleLabel());
    EXPECT_STRCASEEQ("label2", spec->GetMiddleLabel().c_str());
    EXPECT_STRCASEEQ(spec->GetMinorUnit()->GetName().c_str(), "CM");
    EXPECT_TRUE(spec->HasMinorLabel());
    EXPECT_STRCASEEQ("label3", spec->GetMinorLabel().c_str());
    EXPECT_STRCASEEQ(spec->GetSubUnit()->GetName().c_str(), "MM");
    EXPECT_TRUE(spec->HasSubLabel());
    EXPECT_STRCASEEQ("label4", spec->GetSubLabel().c_str());

    parsedNfs = Format();
    EXPECT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrDoubleBar, mapper, s_unitsContext));
    spec = parsedNfs.GetCompositeSpecP();
    ASSERT_NE(nullptr, spec);
    EXPECT_STRCASEEQ(spec->GetMajorUnit()->GetName().c_str(), "M");
    EXPECT_TRUE(spec->HasMajorLabel());
    EXPECT_STRCASEEQ(spec->GetMajorLabel().c_str(), "|");

    parsedNfs = Format();
    EXPECT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrLeftBracket, mapper, s_unitsContext));
    spec = parsedNfs.GetCompositeSpecP();
    ASSERT_NE(nullptr, spec);
    EXPECT_STRCASEEQ(spec->GetMajorUnit()->GetName().c_str(), "M");
    EXPECT_TRUE(spec->HasMajorLabel());
    EXPECT_STRCASEEQ(spec->GetMajorLabel().c_str(), "[");

    parsedNfs = Format();
    EXPECT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrParens, mapper, s_unitsContext));
    spec = parsedNfs.GetCompositeSpecP();
    ASSERT_NE(nullptr, spec);
    EXPECT_STRCASEEQ(spec->GetMajorUnit()->GetName().c_str(), "M");
    EXPECT_TRUE(spec->HasMajorLabel());
    EXPECT_STRCASEEQ(spec->GetMajorLabel().c_str(), "()");

    parsedNfs = Format();
    EXPECT_EQ(BentleyStatus::SUCCESS, Format::ParseFormatString(parsedNfs, fmtStrSemicolon, mapper, s_unitsContext));
    spec = parsedNfs.GetCompositeSpecP();
    ASSERT_NE(nullptr, spec);
    EXPECT_STRCASEEQ(spec->GetMajorUnit()->GetName().c_str(), "M");
    EXPECT_TRUE(spec->HasMajorLabel());
    EXPECT_STRCASEEQ(spec->GetMajorLabel().c_str(), ";");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
TEST_F(FormatJsonTest, FormatDecimal)
    {
    NumericFormatSpec numericSpec = NumericFormatSpec();
    CompositeValueSpec spec(*s_mile, *s_yrd, *s_ft, *s_inch);
    spec.SetMajorLabel("apple");
    spec.SetMiddleLabel("banana");
    spec.SetMinorLabel("cactus pear");
    spec.SetSubLabel("dragonfruit");
    spec.SetSpacer("-");
    Json::Value json;
    spec.ToJson(json);
    Format f = Format(numericSpec, spec);
    auto expectedJson = R"json({
                             "decimalSeparator" : ".",
                             "formatTraits" : [],
                             "minWidth" : 0,
                             "precision" : 6,
                             "roundFactor" : 0.0,
                             "showSignOption" : "OnlyNegative",
                             "thousandSeparator" : ",",
                             "type" : "Decimal",
                             "uomSeparator" : " ",
                             "composite": {
                                "includeZero": true,
                                "spacer": "-",
                                "units": [
                                        {
                                        "name": "MILE",
                                        "label": "apple"
                                        },
                                        {
                                        "name": "YRD",
                                        "label": "banana"
                                        },
                                        {
                                        "name": "FT",
                                        "label": "cactus pear"
                                        },
                                        {
                                        "name": "IN",
                                        "label": "dragonfruit"
                                        }
                                    ]
                                 }
                             })json";
    Json::Value root;
    Json::Reader::Parse(expectedJson, root);
    Json::Value jval;
    f.ToJson(jval, true);
    EXPECT_TRUE(root.ToString() == jval.ToString()) << FormattingTestUtils::JsonComparisonString(jval, root);

    // FromJson
    Format newF;
    Format::FromJson(newF, root, s_unitsContext);
    Json::Value newJval;
    newF.ToJson(newJval, true);
    EXPECT_TRUE(newJval.ToString() == root.ToString()) << FormattingTestUtils::JsonComparisonString(newJval, root);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
TEST_F(FormatJsonTest, FormatFractional)
    {
    NumericFormatSpec numericSpec = NumericFormatSpec();
    numericSpec.SetPresentationType(PresentationType::Fractional);
    numericSpec.SetPrecision(FractionalPrecision::Over_256);
    CompositeValueSpec spec(*s_mile, *s_yrd, *s_ft, *s_inch);
    spec.SetMajorLabel("apple");
    spec.SetMiddleLabel("banana");
    spec.SetMinorLabel("cactus pear");
    spec.SetSubLabel("dragonfruit");
    spec.SetSpacer("-");
    Json::Value json;
    spec.ToJson(json);
    Format f = Format(numericSpec, spec);
    auto expectedJson = R"json({
                             "decimalSeparator" : ".",
                             "formatTraits" : [],
                             "minWidth" : 0,
                             "precision" : 256,
                             "roundFactor" : 0.0,
                             "showSignOption" : "OnlyNegative",
                             "thousandSeparator" : ",",
                             "type" : "Fractional",
                             "uomSeparator" : " ",
                             "composite": {
                                "includeZero": true,
                                "spacer": "-",
                                "units": [
                                        {
                                        "name": "MILE",
                                        "label": "apple"
                                        },
                                        {
                                        "name": "YRD",
                                        "label": "banana"
                                        },
                                        {
                                        "name": "FT",
                                        "label": "cactus pear"
                                        },
                                        {
                                        "name": "IN",
                                        "label": "dragonfruit"
                                        }
                                    ]
                                 }
                             })json";
    Json::Value root;
    Json::Reader::Parse(expectedJson, root);
    Json::Value jval;
    f.ToJson(jval, true);
    EXPECT_TRUE(root.ToString() == jval.ToString()) << FormattingTestUtils::JsonComparisonString(jval, root);

    // FromJson
    Format newF;
    Format::FromJson(newF, root, s_unitsContext);
    Json::Value newJval;
    newF.ToJson(newJval, true);
    EXPECT_TRUE(newJval.ToString() == root.ToString()) << FormattingTestUtils::JsonComparisonString(newJval, root);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
TEST_F(FormatJsonTest, FormatScientific)
    {
    NumericFormatSpec numericSpec = NumericFormatSpec();
    numericSpec.SetPresentationType(PresentationType::Scientific);
    numericSpec.SetScientificType(ScientificType::ZeroNormalized);
    CompositeValueSpec spec(*s_mile, *s_yrd, *s_ft, *s_inch);
    spec.SetMajorLabel("apple");
    spec.SetMiddleLabel("banana");
    spec.SetMinorLabel("cactus pear");
    spec.SetSubLabel("dragonfruit");
    spec.SetSpacer("-");
    Json::Value json;
    spec.ToJson(json);
    Format f = Format(numericSpec, spec);
    auto expectedJson = R"json({
                             "decimalSeparator" : ".",
                             "formatTraits" : [],
                             "minWidth" : 0,
                             "precision" : 6,
                             "roundFactor" : 0.0,
                             "showSignOption" : "OnlyNegative",
                             "thousandSeparator" : ",",
                             "type" : "Scientific",
                             "scientificType" : "ZeroNormalized",
                             "uomSeparator" : " ",
                             "composite": {
                                "includeZero": true,
                                "spacer": "-",
                                "units": [
                                        {
                                        "name": "MILE",
                                        "label": "apple"
                                        },
                                        {
                                        "name": "YRD",
                                        "label": "banana"
                                        },
                                        {
                                        "name": "FT",
                                        "label": "cactus pear"
                                        },
                                        {
                                        "name": "IN",
                                        "label": "dragonfruit"
                                        }
                                    ]
                                 }
                             })json";
    Json::Value root;
    Json::Reader::Parse(expectedJson, root);
    Json::Value jval;
    f.ToJson(jval, true);
    EXPECT_TRUE(root.ToString() == jval.ToString()) << FormattingTestUtils::JsonComparisonString(jval, root);

    // FromJson
    Format newF;
    Format::FromJson(newF, root, s_unitsContext);
    Json::Value newJval;
    newF.ToJson(newJval, true);
    EXPECT_TRUE(newJval.ToString() == root.ToString()) << FormattingTestUtils::JsonComparisonString(newJval, root);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  04/2018
//--------------------------------------------------------------------------------------
TEST_F(FormatJsonTest, FormatStation)
    {
    NumericFormatSpec numericSpec = NumericFormatSpec();
    numericSpec.SetPresentationType(PresentationType::Station);
    numericSpec.SetStationOffsetSize(4);
    numericSpec.SetStationSeparator('(');
    CompositeValueSpec spec(*s_mile, *s_yrd, *s_ft, *s_inch);
    spec.SetMajorLabel("apple");
    spec.SetMiddleLabel("banana");
    spec.SetMinorLabel("cactus pear");
    spec.SetSubLabel("dragonfruit");
    spec.SetSpacer("-");
    Json::Value json;
    spec.ToJson(json);
    Format f = Format(numericSpec, spec);
    auto expectedJson = R"json({
                             "decimalSeparator" : ".",
                             "formatTraits" : [],
                             "minWidth" : 0,
                             "precision" : 6,
                             "roundFactor" : 0.0,
                             "showSignOption" : "OnlyNegative",
                             "thousandSeparator" : ",",
                             "type" : "Station",
                             "stationOffsetSize" : 4,
                             "stationSeparator" : "(",
                             "uomSeparator" : " ",
                             "composite": {
                                "includeZero": true,
                                "spacer": "-",
                                "units": [
                                        {
                                        "name": "MILE",
                                        "label": "apple"
                                        },
                                        {
                                        "name": "YRD",
                                        "label": "banana"
                                        },
                                        {
                                        "name": "FT",
                                        "label": "cactus pear"
                                        },
                                        {
                                        "name": "IN",
                                        "label": "dragonfruit"
                                        }
                                    ]
                                 }
                             })json";
    Json::Value root;
    Json::Reader::Parse(expectedJson, root);
    Json::Value jval;
    f.ToJson(jval, true);
    EXPECT_TRUE(root.ToString() == jval.ToString()) << FormattingTestUtils::JsonComparisonString(jval, root);

    // FromJson
    Format newF;
    Format::FromJson(newF, root, s_unitsContext);
    Json::Value newJval;
    newF.ToJson(newJval, true);
    EXPECT_TRUE(newJval.ToString() == root.ToString()) << FormattingTestUtils::JsonComparisonString(newJval, root);
    }
END_BENTLEY_FORMATTEST_NAMESPACE