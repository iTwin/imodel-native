/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/FormattingIO.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
void StdFormatSet::StdInit()
    {
    FormatTraits traits = FormatConstant::DefaultFormatTraits();
    FormatTraits traitsU = FormatConstant::UnitizedFormatTraits();
    //AddFormat("DefaultReal", new NumericFormatSpec( PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()), "real");
    AddFormat(FormatConstant::DefaultFormatName(), new NumericFormatSpec(NumericFormatSpec::DefaultFormat()), FormatConstant::DefaultFormatAlias());
    AddFormat("DefaultRealU", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, FormatConstant::DefaultDecimalPrecisionIndex()), "realu");
    AddFormat("Real2", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, 2), "real2");
    AddFormat("Real3", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, 3), "real3");
    AddFormat("Real4", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, 4), "real4");

    AddFormat("Real2U", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 2), "real2u");
    AddFormat("Real3U", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 3), "real3u");
    AddFormat("Real4U", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 4), "real4u");
    AddFormat("Real6U", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 6), "real6u");

    NumericFormatSpec stop = new NumericFormatSpec(PresentationType::Stop100, ShowSignOption::OnlyNegative, traitsU, 2);
    AddFormat("Stop100-2u", stop, "stop100-2u");
    stop = new NumericFormatSpec(PresentationType::Stop100, ShowSignOption::OnlyNegative, traitsU, 2);
    stop.SetMinWidth(4);
    AddFormat("Stop100-2-4u", stop, "stop100-2-4u");

    stop = new NumericFormatSpec(PresentationType::Stop1000, ShowSignOption::OnlyNegative, traitsU, 2);
    stop.SetMinWidth(4);
    AddFormat("Stop1000-2-4u", stop, "stop1000-2-4u");

    stop = new NumericFormatSpec(PresentationType::Stop1000, ShowSignOption::OnlyNegative, traitsU, 2);
    AddFormat("Stop1000-2u", stop, "stop1000-2u");

    //=========================== Stoppers w/o units
    stop = new NumericFormatSpec(PresentationType::Stop100, ShowSignOption::OnlyNegative, traits, 2);
    AddFormat("Stop100-2", stop, "stop100-2");
    stop = new NumericFormatSpec(PresentationType::Stop100, ShowSignOption::OnlyNegative, traits, 2);
    stop.SetMinWidth(4);
    AddFormat("Stop100-2-4", stop, "stop100-2-4");

    stop = new NumericFormatSpec(PresentationType::Stop1000, ShowSignOption::OnlyNegative, traits, 2);
    stop.SetMinWidth(4);
    AddFormat("Stop1000-2-4", stop, "stop1000-2-4");

    stop = new NumericFormatSpec(PresentationType::Stop1000, ShowSignOption::OnlyNegative, traits, 2);
    AddFormat("Stop1000-2", stop, "stop1000-2");


    //AddFormat("RoadStatF", new NumericFormatSpec(PresentationType::Station100, ShowSignOption::OnlyNegative, traits, 2), "rsf2");
    //AddFormat("RoadStatM", new NumericFormatSpec(PresentationType::Station1000, ShowSignOption::OnlyNegative, traits, 2), "rsm2");

    AddFormat("SignedReal", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()), "realSign");
    AddFormat("ParenthsReal", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::NegativeParentheses, traits, FormatConstant::DefaultDecimalPrecisionIndex()), "realPth");
    AddFormat("DefaultFractional", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultFractionalDenominator()), "fract");
    AddFormat("DefaultFractionalU", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, FormatConstant::DefaultFractionalDenominator()), "fractu");
    AddFormat("SignedFractional", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::SignAlways, traits, FormatConstant::DefaultFractionalDenominator()), "fractSign");
    AddFormat("DefaultExp", new NumericFormatSpec(PresentationType::Scientific, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()), "sci");
    AddFormat("SignedExp", new NumericFormatSpec(PresentationType::Scientific, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()), "sciSign");
    AddFormat("NormalizedExp", new NumericFormatSpec(PresentationType::ScientificNorm, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()), "sciN");
    AddFormat("DefaultInt", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()), "int");
    AddFormat("Fractional4", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 4), "fract4");
    AddFormat("Fractional8", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), "fract8");
    AddFormat("Fractional16", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 16), "fract16");
    AddFormat("Fractional32", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 32), "fract32");
    AddFormat("Fractional128", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 128), "fract128");

    AddFormat("Fractional4U", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 4), "fract4u");
    AddFormat("Fractional8U", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 8), "fract8u");
    AddFormat("Fractional16U", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 16), "fract16u");
    AddFormat("Fractional32U", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 32), "fract32u");
    AddFormat("Fractional128U", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 128), "fract128u");

    CompositeValueSpecP cvs = new CompositeValueSpec("ARC_DEG", "ARC_MINUTE", "ARC_SECOND", nullptr);
    cvs->SetUnitLabels("\xC2\xB0", u8"'", u8"\"");
    AddFormat("AngleDMS", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 0), cvs, "dms");
    AddFormat("AngleDMS8", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), cvs, "dms8");
    cvs = new CompositeValueSpec("ARC_DEG", "ARC_MINUTE");
    cvs->SetUnitLabels("\xC2\xB0", u8"'");
    AddFormat("AngleDM8", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), cvs, "dm8");
    cvs = new CompositeValueSpec("MILE", "YRD", "FT", "IN");
    cvs->SetUnitLabels("mile(s)", "yrd(s)", "'", "\"");
    AddFormat("AmerMYFI4", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 4), cvs, "myfi4");
    cvs = new CompositeValueSpec("FT", "IN");
    cvs->SetUnitLabels("'", "\"");
    AddFormat("AmerFI8", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), cvs, "fi8");
    AddFormat("AmerFI16", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 16), cvs, "fi16");
    cvs = new CompositeValueSpec("YRD", "FT", "IN");
    cvs->SetUnitLabels("yrd(s)", "'", "\"");
    AddFormat("AmerYFI8", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), cvs, "yfi8");

    }

//===================================================
//
// NumericFormatSpec Methods
//
//===================================================

FormatTraits NumericFormatSpec::SetTraitsBit(FormatTraits bit, FormatTraits traits, bool set)
    {
    size_t temp = static_cast<int>(traits);

    if (set)
        temp |= static_cast<int>(bit);
    else
        temp &= ~static_cast<int>(bit);
    return  static_cast<FormatTraits>(temp);
    }

void NumericFormatSpec::TraitsBitToJson(JsonValueR outValue, Utf8CP bitIndex, FormatTraits bit, FormatTraits ref, bool verbose) const
    {
    if (verbose || FormatConstant::IsBoolEqual(CheckTraitsBit(bit), CheckTraitsBit(ref, bit)))  //.IsKeepTrailingZeroes())
        outValue[bitIndex] = FormatConstant::BoolText(CheckTraitsBit(bit));
    }

void NumericFormatSpec::TraitsBitToJsonKey(JsonValueR outValue, Utf8CP bitIndex, FormatTraits bit, FormatTraits traits)
    {
    outValue[bitIndex] = FormatConstant::BoolText((static_cast<int>(traits) & static_cast<int>(bit)) != 0);
    }

Json::Value NumericFormatSpec::JsonFormatTraits(bool verbose) const
    {
    Json::Value jTraits;
    FormatTraits ref = FormatConstant::DefaultFormatTraits();
    
    TraitsBitToJson(jTraits, json_TrailZeroes(), FormatTraits::TrailingZeroes, ref, verbose);
    TraitsBitToJson(jTraits, json_LeadZeroes(),  FormatTraits::LeadingZeroes, ref, verbose);
    TraitsBitToJson(jTraits, json_KeepDecPnt(), FormatTraits::KeepDecimalPoint, ref, verbose);
    TraitsBitToJson(jTraits, json_KeepSingleZero(), FormatTraits::KeepSingleZero, ref, verbose);
    TraitsBitToJson(jTraits, json_ExponentZero(), FormatTraits::ExponentZero, ref, verbose);
    TraitsBitToJson(jTraits, json_ZeroEmpty(), FormatTraits::ZeroEmpty, ref, verbose);
    TraitsBitToJson(jTraits, json_Use1000Separator(), FormatTraits::Use1000Separator, ref, verbose);
    TraitsBitToJson(jTraits, json_ApplyRounding(), FormatTraits::ApplyRounding, ref, verbose);
    TraitsBitToJson(jTraits, json_AppendUnitName(), FormatTraits::AppendUnitName, ref, verbose);
    TraitsBitToJson(jTraits, json_UseFractSymbol(), FormatTraits::UseFractSymbol, ref, verbose);
    TraitsBitToJson(jTraits, json_FractionDash(), FormatTraits::FractionDash, ref, verbose);
    return jTraits;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//---------------------------------------------------------------------------------------
Json::Value NumericFormatSpec::ToJson(bool verbose)const
    {
    Json::Value jNFC;
    jNFC[json_presentType()] = Utils::PresentationTypeName(m_presentationType);
    jNFC[json_signOpt()] = Utils::SignOptionName(m_signOption);
    jNFC[json_roundFactor()] = m_roundFactor;
    jNFC[json_decPrec()] = Utils::DecimalPrecisionToInt(m_decPrecision);
    jNFC[json_fractPrec()] = Utils::FractionallPrecisionName(m_fractPrecision);
    jNFC[json_formatTraits()] = JsonFormatTraits(verbose);
    if (m_barType != FractionBarType::None)
        jNFC[json_barType()] = Utils::FractionBarName(m_barType);
    if (m_decimalSeparator != '\0')
        jNFC[json_decimalSeparator()] = Utils::CharToString(m_decimalSeparator);
    if (m_thousandsSeparator != '\0')
        jNFC[json_thousandSeparator()] = Utils::CharToString(m_thousandsSeparator);
    if (nullptr != m_uomSeparator)
        jNFC[json_uomSeparator()] = m_uomSeparator;
    if (m_stopSeparator != '\0')
        jNFC[json_statSeparator()] = Utils::CharToString(m_stopSeparator);
    if (m_minWIdth != 0)
        jNFC[json_minWidth()] = m_minWIdth;
    return jNFC;
    }

//===================================================
//
// UnitProxy Methods
//
//===================================================
Json::Value UnitProxy::ToJson(bool verbose) const
    {
    Json::Value jUP;
    Utf8CP uN = Utils::SubstituteEmptyOrNull(m_unitName.c_str(), "");
    Utf8CP uL = Utils::SubstituteEmptyOrNull(m_unitLabel.c_str(), "");
    if (verbose)
        {
        jUP[json_unitName()] = uN;
        jUP[json_unitLabel()] = uL;
        }
    else
        {
        jUP[0] = uN;
        jUP[1] = uL;
        }

    return jUP;
    }

//===================================================
//
//UnitProxySet Methods
//
//===================================================
Json::Value UnitProxySet::ToJson() const
    {
    Json::Value jUP;

    if(m_proxys.empty())
        return jUP;
    UnitProxy prox;
    for(int i = 0; i < m_proxys.size(); i++)
        {
        prox = m_proxys[i];
        jUP[i] = prox.ToJson(false);
        }
    return jUP;
    }


//===================================================
//
// CompositeValueSpec Methods
//
//===================================================

Json::Value CompositeValueSpec::ToJson() const
    {
    Json::Value jCVS;
    bool valid = false;
    switch (m_type)
        {
        case CompositeSpecType::Quatro:
            jCVS[json_SubUnit()] = m_unitProx.GetProxy(indxSub)->ToJson(false);
        case CompositeSpecType::Triple:
            jCVS[json_MinorUnit()] = m_unitProx.GetProxy(indxMinor)->ToJson(false);
        case CompositeSpecType::Double:
            jCVS[json_MiddleUnit()] = m_unitProx.GetProxy(indxMiddle)->ToJson(false);
        case CompositeSpecType::Single: // smallQ already has the converted value
            jCVS[json_MajorUnit()] = m_unitProx.GetProxy(indxMajor)->ToJson(false);
            valid = true;
            break;
        }

    if (valid)
        {
        if (IsIncludeZero())
            jCVS[json_includeZero()] = true;
        if (m_spacer.length() > 0)
            jCVS[json_spacer()] = m_spacer.c_str();
        }

    return jCVS;
    }

    //friend struct CompositeValue;
    //protected:
    //    static const size_t  indxMajor = 0;
    //    static const size_t  indxMiddle = 1;
    //    static const size_t  indxMinor = 2;
    //    static const size_t  indxSub = 3;
    //    static const size_t  indxInput = 4;
    //    static const size_t  indxLimit = 5;
    //    size_t m_ratio[indxSub];
    //    //BEU::UnitCP m_units[indxLimit];
    //    UnitProxySet m_unitProx = UnitProxySet(indxLimit);
    //    //Utf8CP m_unitLabel[indxLimit];
    //    FormatProblemDetail m_problem;
    //    CompositeSpecType m_type;
    //    bool m_includeZero;
    //    Utf8String m_spacer;





//===================================================
//
// NamedFormatSpec Methods
//
//===================================================
Json::Value NamedFormatSpec::ToJson(bool verbose) const
    {
    Json::Value jNFS;
    jNFS[json_SpecName()] = m_name;
    jNFS[json_SpecAlias()] = m_alias;
    if (m_specType == FormatSpecType::Numeric)
        jNFS[json_SpecType()] = "numeric";
    if (m_specType == FormatSpecType::Composite)
        jNFS[json_SpecType()] = "composite";
    jNFS[json_NumericFormat()] = m_numericSpec.ToJson(verbose);
    jNFS[json_CompositeFormat()] = m_compositeSpec.ToJson();
    return jNFS;
    }

//===================================================
//
// FormatUnitSet Methods
//
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Json::Value FormatUnitSet::ToJson(bool useAlias) const
    {
    Json::Value jval = Json::objectValue;
    if (useAlias)
        jval[FormatConstant::FUSJsonAlias()] = m_formatSpec->GetAlias();
    else
        jval[FormatConstant::FUSJsonName()] = m_formatSpec->GetName();
    jval[FormatConstant::FUSJsonUnit()] = m_unit->GetName();
    return jval;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Utf8String FormatUnitSet::ToJsonString(bool useAlias) const
    {
    Utf8String str;
    Json::Value jval = ToJson(useAlias);
    str = jval.ToString();
    return str;
    }


//===================================================
//
// FormatDictionary Methods
//
//===================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void FormatDictionary::InitLoad()
    {
    AddParameter(FormatParameter(FormatConstant::FPN_NoSign(), ParameterCategory::Sign, ParameterCode::NoSign, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_OnlyNegative(), ParameterCategory::Sign, ParameterCode::OnlyNegative, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_SignAlways(), ParameterCategory::Sign, ParameterCode::SignAlways, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_NegativeParenths(), ParameterCategory::Sign, ParameterCode::NegativeParenths, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Decimal(), ParameterCategory::Presentation, ParameterCode::Decimal, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Fractional(), ParameterCategory::Presentation, ParameterCode::Fractional, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Scientific(), ParameterCategory::Presentation, ParameterCode::Scientific, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_ScientificNorm(), ParameterCategory::Presentation, ParameterCode::ScientificNorm, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Binary(), ParameterCategory::Presentation, ParameterCode::Binary, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_DefaultZeroes(), ParameterCategory::Zeroes, ParameterCode::DefaultZeroes, Utils::FormatTraitsBit(FormatTraits::DefaultZeroes)));
    AddParameter(FormatParameter(FormatConstant::FPN_LeadingZeroes(), ParameterCategory::Zeroes, ParameterCode::LeadingZeroes, Utils::FormatTraitsBit(FormatTraits::LeadingZeroes)));
    AddParameter(FormatParameter(FormatConstant::FPN_TrailingZeroes(), ParameterCategory::Zeroes, ParameterCode::TrailingZeroes, Utils::FormatTraitsBit(FormatTraits::TrailingZeroes)));
    AddParameter(FormatParameter(FormatConstant::FPN_KeepDecimalPoint(), ParameterCategory::Zeroes, ParameterCode::KeepDecimalPoint, Utils::FormatTraitsBit(FormatTraits::KeepDecimalPoint)));
    AddParameter(FormatParameter(FormatConstant::FPN_ZeroEmpty(), ParameterCategory::Zeroes, ParameterCode::ZeroEmpty, Utils::FormatTraitsBit(FormatTraits::ZeroEmpty)));
    AddParameter(FormatParameter(FormatConstant::FPN_KeepSingleZero(), ParameterCategory::Zeroes, ParameterCode::KeepSingleZero, Utils::FormatTraitsBit(FormatTraits::KeepSingleZero)));
    AddParameter(FormatParameter(FormatConstant::FPN_ExponentZero(), ParameterCategory::Zeroes, ParameterCode::ExponentZero, Utils::FormatTraitsBit(FormatTraits::ExponentZero)));
    AddParameter(FormatParameter(FormatConstant::FPN_Precision0(), ParameterCategory::DecPrecision, ParameterCode::DecPrec0, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Precision1(), ParameterCategory::DecPrecision, ParameterCode::DecPrec1, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Precision2(), ParameterCategory::DecPrecision, ParameterCode::DecPrec2, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Precision3(), ParameterCategory::DecPrecision, ParameterCode::DecPrec3, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Precision4(), ParameterCategory::DecPrecision, ParameterCode::DecPrec4, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Precision5(), ParameterCategory::DecPrecision, ParameterCode::DecPrec5, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Precision6(), ParameterCategory::DecPrecision, ParameterCode::DecPrec6, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Precision7(), ParameterCategory::DecPrecision, ParameterCode::DecPrec7, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Precision8(), ParameterCategory::DecPrecision, ParameterCode::DecPrec8, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Precision9(), ParameterCategory::DecPrecision, ParameterCode::DecPrec9, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Precision10(), ParameterCategory::DecPrecision, ParameterCode::DecPrec10, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Precision11(), ParameterCategory::DecPrecision, ParameterCode::DecPrec11, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_Precision12(), ParameterCategory::DecPrecision, ParameterCode::DecPrec12, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_FractPrec1(), ParameterCategory::FractPrecision, ParameterCode::FractPrec1, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_FractPrec2(), ParameterCategory::FractPrecision, ParameterCode::FractPrec2, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_FractPrec4(), ParameterCategory::FractPrecision, ParameterCode::FractPrec4, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_FractPrec8(), ParameterCategory::FractPrecision, ParameterCode::FractPrec8, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_FractPrec16(), ParameterCategory::FractPrecision, ParameterCode::FractPrec16, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_FractPrec32(), ParameterCategory::FractPrecision, ParameterCode::FractPrec32, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_FractPrec64(), ParameterCategory::FractPrecision, ParameterCode::FractPrec64, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_FractPrec128(), ParameterCategory::FractPrecision, ParameterCode::FractPrec128, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_FractPrec256(), ParameterCategory::FractPrecision, ParameterCode::FractPrec256, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_DecimalComma(), ParameterCategory::Separator, ParameterCode::DecimalComma, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_DecimalPoint(), ParameterCategory::Separator, ParameterCode::DecimalPoint, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_DecimalSepar(), ParameterCategory::Separator, ParameterCode::DecimalSepar, ParameterDataType::Symbol));
    AddParameter(FormatParameter(FormatConstant::FPN_ThousandSepComma(), ParameterCategory::Separator, ParameterCode::ThousandSepComma, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_ThousandSepPoint(), ParameterCategory::Separator, ParameterCode::ThousandSepPoint, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_ThousandsSepar(), ParameterCategory::Separator, ParameterCode::ThousandsSepar, ParameterDataType::Symbol));
    AddParameter(FormatParameter(FormatConstant::FPN_RoundUp(), ParameterCategory::RoundType, ParameterCode::RoundUp, ParameterDataType::Double));
    AddParameter(FormatParameter(FormatConstant::FPN_RoundDown(), ParameterCategory::RoundType, ParameterCode::RoundDown, ParameterDataType::Double));
    AddParameter(FormatParameter(FormatConstant::FPN_RoundToward0(), ParameterCategory::RoundType, ParameterCode::RoundToward0, ParameterDataType::Double));
    AddParameter(FormatParameter(FormatConstant::FPN_RoundAwayFrom0(), ParameterCategory::RoundType, ParameterCode::RoundAwayFrom0, ParameterDataType::Double));
    AddParameter(FormatParameter(FormatConstant::FPN_FractBarHoriz(), ParameterCategory::FractionBar, ParameterCode::FractBarHoriz, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_FractBarOblique(), ParameterCategory::FractionBar, ParameterCode::FractBarOblique, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_FractBarDiagonal(), ParameterCategory::FractionBar, ParameterCode::FractBarDiagonal, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_AngleRegular(), ParameterCategory::AngleFormat, ParameterCode::AngleRegular, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_AngleDegMin(), ParameterCategory::AngleFormat, ParameterCode::AngleDegMin, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_AngleDegMinSec(), ParameterCategory::AngleFormat, ParameterCode::AngleDegMinSec, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_PaddingSymbol(), ParameterCategory::Padding, ParameterCode::PaddingSymbol, ParameterDataType::Symbol));
    AddParameter(FormatParameter(FormatConstant::FPN_CenterAlign(), ParameterCategory::Alignment, ParameterCode::CenterAlign, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_LeftAlign(), ParameterCategory::Alignment, ParameterCode::LeftAlign, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_RightAlign(), ParameterCategory::Alignment, ParameterCode::RightAlign, ParameterDataType::Flag));
    AddParameter(FormatParameter(FormatConstant::FPN_MapName(), ParameterCategory::Mapping, ParameterCode::MapName, ParameterDataType::String));
    }

END_BENTLEY_FORMATTING_NAMESPACE