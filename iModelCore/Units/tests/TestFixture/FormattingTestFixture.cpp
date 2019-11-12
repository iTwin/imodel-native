/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#if defined (BENTLEY_WIN32)
#include <windows.h>
#include <iostream>
#endif

#include <locale>

#include "FormattingTestFixture.h"

USING_BENTLEY_FORMATTING

BEGIN_BENTLEY_FORMATTEST_NAMESPACE

BEU::UnitRegistry* FormattingTestFixture::s_unitsContext = nullptr;
bmap<Utf8String, Format> FormattingTestFixture::s_stdFormats = bmap<Utf8String, Format>();

void FormattingTestFixture::CreateStdFormats()
    {
    NumericFormatSpec spec = NumericFormatSpec(NumericFormatSpec::DefaultFormat());
    Format defaultRealU = Format(spec);
    s_stdFormats["DefaultRealU"] = defaultRealU;

    Format defaultReal = Format(spec);
    defaultReal.SetSuppressUnitLabel();
    s_stdFormats["DefaultReal"] = defaultReal;

    Format defaultRealUNS = defaultRealU;
    defaultRealUNS.GetNumericSpecP()->SetUomSeparator("");
    s_stdFormats["DefaultRealUNS"] = defaultRealUNS;

    Format signedReal = defaultReal;
    signedReal.GetNumericSpecP()->SetSignOption(SignOption::SignAlways);
    s_stdFormats["SignedReal"] = signedReal;

    Format parensReal = defaultReal;
    parensReal.GetNumericSpecP()->SetSignOption(SignOption::NegativeParentheses);
    s_stdFormats["ParensReal"] = parensReal;

    Format fractional = defaultReal;
    fractional.GetNumericSpecP()->SetPresentationType(PresentationType::Fractional);
    s_stdFormats["Fractional"] = fractional;

    Format signedFractional = fractional;
    signedFractional.GetNumericSpecP()->SetSignOption(SignOption::SignAlways);
    s_stdFormats["SignedFractional"] = signedFractional;

    Format fractionalU = fractional;
    fractionalU.GetNumericSpecP()->SetShowUnitLabel(true);
    s_stdFormats["FractionalU"] = fractionalU;

    Format scientific = defaultReal;
    scientific.GetNumericSpecP()->SetPresentationType(PresentationType::Scientific);
    scientific.GetNumericSpecP()->SetScientificType(ScientificType::Normalized);
    s_stdFormats["Scientific"] = scientific;

    Format signedScientific = scientific;
    signedScientific.GetNumericSpecP()->SetSignOption(SignOption::SignAlways);
    s_stdFormats["SignedScientific"] = signedScientific;

    Format scientificNormal = scientific;
    scientificNormal.GetNumericSpecP()->SetScientificType(ScientificType::ZeroNormalized);
    s_stdFormats["ScientificNormal"] = scientificNormal;

    Format station100 = defaultReal;
    station100.GetNumericSpecP()->SetPresentationType(PresentationType::Station);
    station100.GetNumericSpecP()->SetPrecision(DecimalPrecision::Precision2);
    station100.GetNumericSpecP()->SetStationOffsetSize(2);
    s_stdFormats["Station_100"] = station100;

    Format stationU100 = station100;
    stationU100.GetNumericSpecP()->SetShowUnitLabel(true);
    s_stdFormats["StationU_100"] = stationU100;

    Format stationUNS100 = stationU100;
    stationUNS100.GetNumericSpecP()->SetUomSeparator("");
    s_stdFormats["StationUNS_100"] = stationUNS100;

    Format stationUZ100 = stationU100;
    stationUZ100.GetNumericSpecP()->SetKeepTrailingZeroes(true);
    s_stdFormats["StationUZ_100"] = stationUZ100;

    Format station1000 = station100;
    station1000.GetNumericSpecP()->SetStationOffsetSize(3);
    s_stdFormats["Station_1000"] = station1000;

    Format stationU1000 = station1000;
    stationU1000.GetNumericSpecP()->SetShowUnitLabel(true);
    s_stdFormats["StationU_1000"] = stationU1000;

    Format station1004 = station100;
    station1004.GetNumericSpecP()->SetMinWidth(4);
    s_stdFormats["Station_100_4"] = station1004;

    Format stationU1004 = station1004;
    stationU1004.GetNumericSpecP()->SetShowUnitLabel(true);
    s_stdFormats["StationU_100_4"] = stationU1004;

    Format station10004 = station1004;
    station10004.GetNumericSpecP()->SetStationOffsetSize(3);
    s_stdFormats["Station_1000_4"] = station10004;

    Format stationU10004 = station10004;
    stationU10004.GetNumericSpecP()->SetShowUnitLabel(true);
    s_stdFormats["StationU_1000_4"] = stationU10004;

    Format stationz1002 = station100;
    stationz1002.GetNumericSpecP()->SetKeepTrailingZeroes(true);
    s_stdFormats["StationZ_100_2"] = stationz1002;

    Format stationz10003 = stationz1002;
    stationz10003.GetNumericSpecP()->SetStationOffsetSize(3);
    stationz10003.GetNumericSpecP()->SetMinWidth(3);
    s_stdFormats["StationZ_1000_3"] = stationz10003;

    Format AngleDMS = fractional;
    AngleDMS.GetNumericSpecP()->SetPrecision(FractionalPrecision::Whole);
    auto comp = CompositeValueSpec(*s_unitsContext->LookupUnit("ARC_DEG"), *s_unitsContext->LookupUnit("ARC_MINUTE"), *s_unitsContext->LookupUnit("ARC_SECOND"));
    AngleDMS.SetCompositeSpec(comp);
    s_stdFormats["AngleDMS"] = AngleDMS;

    Format AmerFI = fractional;
    AmerFI.GetNumericSpecP()->SetPrecision(FractionalPrecision::Eighth);
    comp = CompositeValueSpec(*s_unitsContext->LookupUnit("FT"), *s_unitsContext->LookupUnit("IN"));
    AmerFI.SetCompositeSpec(comp);
    s_stdFormats["AmerFI"] = AmerFI;

    Format HMS = defaultReal;
    HMS.GetNumericSpecP()->SetPrecision(DecimalPrecision::Precision2);
    comp = CompositeValueSpec(*s_unitsContext->LookupUnit("HR"), *s_unitsContext->LookupUnit("MIN"), *s_unitsContext->LookupUnit("S"));
    HMS.SetCompositeSpec(comp);
    s_stdFormats["HMS"] = HMS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                  Bill Steinbock 12/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::SetUp()
    {
    if (nullptr == s_unitsContext)
        s_unitsContext = new BEU::UnitRegistry();
    if (s_stdFormats.empty())
        CreateStdFormats();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                  Bill Steinbock 12/17
//----------------------------------------------------------------------------------------
void FormattingTestFixture::TearDown()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
bvector<TraitJsonKeyMap> TraitJsonKeyMap::TraitJsonKeySet()
    {
    static bvector<TraitJsonKeyMap> vec;
    if (vec.size() == 0)
        {
        vec.push_back(TraitJsonKeyMap(FormatTraits::TrailingZeroes, json_trailZeroes()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::KeepDecimalPoint, json_keepDecimalPoint()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::KeepSingleZero, json_keepSingleZero()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::ZeroEmpty, json_zeroEmpty()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::Use1000Separator, json_use1000Separator()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::ApplyRounding, json_applyRounding()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::ShowUnitLabel, json_showUnitLabel()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::FractionDash, json_fractionDash()));
        vec.push_back(TraitJsonKeyMap(FormatTraits::PrependUnitLabel, json_prependUnitLabel()));
        }
    return vec;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
void FormattingTestUtils::NumericFormatSpecJsonTest(NumericFormatSpecCR nfs)
    {
    Json::Value jval;
    EXPECT_TRUE(nfs.ToJson(jval, true));
    Utf8String str = jval.ToString();
    NumericFormatSpec nfs1;
    NumericFormatSpec::FromJson(nfs1, jval);
    EXPECT_TRUE(nfs.IsIdentical(nfs1));
    EXPECT_TRUE(nfs.ToJson(jval, false));
    NumericFormatSpec::FromJson(nfs1, jval);
    EXPECT_TRUE(nfs.IsIdentical(nfs1));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
Utf8String FormattingTestUtils::SetLocale(Utf8CP name)
    {
    Utf8String locName;
    //const std::locale& loc = *(locale::global); // (name);
    std::locale currLoc("");

    std::locale loc(name);
    const std::numpunct<char>& myfacet = std::use_facet < numpunct<char> >(loc);
    Utf8Char buf[3];
    buf[0] = myfacet.decimal_point();
    buf[1] = myfacet.thousands_sep();
    buf[2] = 0;
    locName.assign(buf);
    LOG.infov("Current system locale decpnt= %s name %s   switched from %s", locName.c_str(), loc.name().c_str(), currLoc.name().c_str());

    return locName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String FormattingTestUtils::JsonComparisonString(Json::Value const& created, Json::Value const& test)
    {
    return "Created   (minified): " + created.ToString() + '\n' +
           "Test Data (minified): " + test.ToString() + '\n' +
           "Created   (pretty):\n"  + created.toStyledString() + '\n' +
           "Test Data (pretty):\n"  + test.toStyledString();
    }

END_BENTLEY_FORMATTEST_NAMESPACE
