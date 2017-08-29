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

NumericFormatSpec::NumericFormatSpec(Json::Value jval)
    {
    DefaultInit(FormatConstant::DefaultDecimalPrecisionIndex());
    if (!jval.empty())
        {
        Utf8CP paramName;
        Utf8String str;
        for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
            {
            paramName = iter.memberName();
            JsonValueCR val = *iter;
            if (stricmp(paramName, json_roundFactor()) == 0)
                {
                m_roundFactor = val.asDouble();
                }
            else if (stricmp(paramName, json_decPrec()) == 0)
                {
                m_decPrecision = Utils::DecimalPrecisionByIndex(val.asInt64());
                }
            else if (stricmp(paramName, json_fractPrec()) == 0)
                {
                m_fractPrecision = Utils::FractionalPrecisionByDenominator(val.asInt64());
                }
            else if (stricmp(paramName, json_barType()) == 0)
                {
                m_barType = Utils::NameToFractionBarType(val.asCString());
                }
            else if (stricmp(paramName, json_decimalSeparator()) == 0)
                {
                str = val.asString();
                m_decimalSeparator = str.c_str()[0];
                }
            else if (stricmp(paramName, json_thousandSeparator()) == 0)
                {
                str = val.asString();
                m_thousandsSeparator = str.c_str()[0];
                }
            else if (stricmp(paramName, json_uomSeparator()) == 0)
                {
                m_uomSeparator = val.asString();
                }
            else if (stricmp(paramName, json_statSeparator()) == 0)
                {
                str = val.asString();
                m_statSeparator = str.c_str()[0];
                }
            else if (stricmp(paramName, json_minWidth()) == 0)
                {
                m_minWIdth = val.asInt();
                }
            else if (stricmp(paramName, json_formatTraits()) == 0)
                {
                TraitsFromJson(val);
                }
            }
        }
    }

FormatTraits NumericFormatSpec::TraitsFromJson(JsonValueCR jval)
    {
    Utf8CP paramName;
    Utf8String str;
    if (jval.empty())
        return m_formatTraits;

    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (stricmp(paramName, json_LeadZeroes()) == 0)
            SetUseLeadingZeroes(val.asBool());
        else if (stricmp(paramName, json_TrailZeroes()) == 0)
            SetKeepTrailingZeroes(val.asBool());
        else if (stricmp(paramName, json_KeepDecPnt()) == 0)
            SetKeepDecimalPoint(val.asBool());
        else if (stricmp(paramName, json_KeepSingleZero()) == 0)
            SetKeepSingleZero(val.asBool());
        else if (stricmp(paramName, json_ExponentZero()) == 0)
            SetExponentZero(val.asBool());
        else if (stricmp(paramName, json_ZeroEmpty()) == 0)
            SetZeroEmpty(val.asBool());
        else if (stricmp(paramName, json_Use1000Separator()) == 0)
            SetUse1000Separator(val.asBool());
        else if (stricmp(paramName, json_ApplyRounding()) ==0)
            SetApplyRounding(val.asBool());
        else if (stricmp(paramName, json_FractionDash()) == 0)
            SetFractionDash(val.asBool());
        else if (stricmp(paramName, json_UseFractSymbol()) == 0)
            SetUseFractSymbol(val.asBool());
        else if (stricmp(paramName, json_AppendUnitName()) == 0)
            SetAppendUnit(val.asBool());
        }
    return m_formatTraits;
    }

FormatTraits NumericFormatSpec::SetTraitsBit(FormatTraits bit, FormatTraits traits, bool set)
    {
    size_t temp = static_cast<int>(traits);

    if (set)
        temp |= static_cast<int>(bit);
    else
        temp &= ~static_cast<int>(bit);
    return  static_cast<FormatTraits>(temp);
    }

void NumericFormatSpec::TraitsBitToJson(JsonValueR outValue, Utf8CP bitIndex, FormatTraits bit, FormatTraits* ref, bool verbose) const
    {
    if (ref == nullptr)
        verbose = true;
    if ((nullptr == ref) || !FormatConstant::IsBoolEqual(CheckTraitsBit(bit), CheckTraitsBit(*ref, bit)))  //.IsKeepTrailingZeroes())
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

    TraitsBitToJson(jTraits, json_TrailZeroes(), FormatTraits::TrailingZeroes, &ref, verbose);
    TraitsBitToJson(jTraits, json_LeadZeroes(),  FormatTraits::LeadingZeroes, &ref, verbose);
    TraitsBitToJson(jTraits, json_KeepDecPnt(), FormatTraits::KeepDecimalPoint, &ref, verbose);
    TraitsBitToJson(jTraits, json_KeepSingleZero(), FormatTraits::KeepSingleZero, &ref, verbose);
    TraitsBitToJson(jTraits, json_ExponentZero(), FormatTraits::ExponentZero, &ref, verbose);
    TraitsBitToJson(jTraits, json_ZeroEmpty(), FormatTraits::ZeroEmpty, &ref, verbose);
    TraitsBitToJson(jTraits, json_Use1000Separator(), FormatTraits::Use1000Separator, &ref, verbose);
    TraitsBitToJson(jTraits, json_ApplyRounding(), FormatTraits::ApplyRounding, &ref, verbose);
    TraitsBitToJson(jTraits, json_AppendUnitName(), FormatTraits::AppendUnitName, &ref, verbose);
    TraitsBitToJson(jTraits, json_UseFractSymbol(), FormatTraits::UseFractSymbol, &ref, verbose);
    TraitsBitToJson(jTraits, json_FractionDash(), FormatTraits::FractionDash, &ref, verbose);
    return jTraits;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//---------------------------------------------------------------------------------------
Json::Value NumericFormatSpec::ToJson(bool verbose)const
    {
    NumericFormatSpec defSpec = NumericFormatSpec();
    Json::Value jNFC;

    jNFC[json_presentType()] = Utils::PresentationTypeName(m_presentationType);
    if(verbose || m_signOption != defSpec.m_signOption)
        jNFC[json_signOpt()] = Utils::SignOptionName(m_signOption);
    if (verbose || fabs(m_roundFactor - defSpec.m_roundFactor) > 0.01)
        jNFC[json_roundFactor()] = m_roundFactor;
    if (verbose || m_decPrecision != defSpec.m_decPrecision)
        jNFC[json_decPrec()] = Utils::DecimalPrecisionToInt(m_decPrecision);
    if (verbose || m_fractPrecision != defSpec.m_fractPrecision)
        jNFC[json_fractPrec()] = Utils::FractionallPrecisionName(m_fractPrecision);
    if (verbose || m_formatTraits != defSpec.m_formatTraits)
        jNFC[json_formatTraits()] = JsonFormatTraits(verbose);
    if (verbose || m_barType != defSpec.m_barType)
        jNFC[json_barType()] = Utils::FractionBarName(m_barType);
    if (verbose || m_decimalSeparator != defSpec.m_decimalSeparator)
        jNFC[json_decimalSeparator()] = Utils::CharToString(m_decimalSeparator);
    if (verbose || m_thousandsSeparator != defSpec.m_thousandsSeparator)
        jNFC[json_thousandSeparator()] = Utils::CharToString(m_thousandsSeparator);
    if (verbose || !m_uomSeparator.Equals(defSpec.m_uomSeparator.c_str()))
        jNFC[json_uomSeparator()] = m_uomSeparator;
    if (verbose || m_statSeparator != defSpec.m_statSeparator)
        jNFC[json_statSeparator()] = Utils::CharToString(m_statSeparator);
    if (verbose || m_minWIdth != defSpec.m_minWIdth)
        jNFC[json_minWidth()] = m_minWIdth;
    return jNFC;
    }

//===================================================
//
// UnitProxy Methods
//
//===================================================
Json::Value UnitProxy::ToJson() const
    {
    Json::Value jUP;

    Utf8CP uN = Utils::GetCharsOrNull(m_unitName);
    Utf8CP uL = Utils::GetCharsOrNull(m_unitLabel);

    if(nullptr != uN)
        jUP[json_unitName()] = uN;
    if (nullptr != uL)
        jUP[json_unitLabel()] = uL;
    return jUP;
    }

//===================================================
//
//UnitProxySet Methods
//
//===================================================
Json::Value UnitProxySet::ToJson(bvector<Utf8CP> keyNames) const
    {
    Json::Value jUP;

    if(m_proxys.empty())
        return jUP;
    UnitProxy prox;
    size_t num = Utils::MinInt(keyNames.size(), m_proxys.size());

    for(int i = 0; i < num; i++)
        {
        jUP[keyNames[i]] = m_proxys[i].ToJson();
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
    bvector<Utf8CP> keyNames;
    UnitProxyCP proxP;
    switch (m_type)
        {
        case CompositeSpecType::Quatro:
            proxP = m_unitProx.GetProxy(indxSub);
            if(!proxP->IsEmpty())
                jCVS[json_SubUnit()] = proxP->ToJson();
        case CompositeSpecType::Triple:
            proxP = m_unitProx.GetProxy(indxMinor);
            if (!proxP->IsEmpty())
                jCVS[json_MinorUnit()] = proxP->ToJson();
        case CompositeSpecType::Double:
            proxP = m_unitProx.GetProxy(indxMiddle);
            if (!proxP->IsEmpty())
                jCVS[json_MiddleUnit()] = proxP->ToJson();
        case CompositeSpecType::Single: // smallQ already has the converted value
            proxP = m_unitProx.GetProxy(indxMajor);
            if (!proxP->IsEmpty())
                {
                jCVS[json_MajorUnit()] = proxP->ToJson();
                valid = true;
                }
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

void CompositeValueSpec::LoadJsonData(JsonValueCR jval)
    {
    Utf8CP paramName;
    Utf8String str;
    if (jval.empty())
        return;
    Utf8String major;
    Utf8String middle;
    Utf8String minor;
    Utf8String sub;
    Utf8String input;
    
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (stricmp(paramName, json_MajorUnit()) == 0)
            major = val.asString();
        else if (stricmp(paramName, json_MiddleUnit()) == 0)
            middle = val.asString();
        else if (stricmp(paramName, json_MinorUnit()) == 0)
            minor = val.asString();
        else if (stricmp(paramName, json_SubUnit()) == 0)
            sub = val.asString();
        else if (stricmp(paramName, json_InputUnit()) == 0)
            input = val.asString();
        else if (stricmp(paramName, json_includeZero()) == 0)
            m_includeZero = val.asBool();
        else if (stricmp(paramName, json_spacer()) == 0)
            m_spacer = val.asString();
        }

    SetUnitNames(Utils::GetCharsOrNull(major), Utils::GetCharsOrNull(middle), Utils::GetCharsOrNull(minor), Utils::GetCharsOrNull(sub));
    if (!input.empty())
        {
        BEU::UnitCP inputUnit = BEU::UnitRegistry::Instance().LookupUnit(input.c_str());
        SetInputUnit(inputUnit);
        }    
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
    Json::Value jcs = m_compositeSpec.ToJson();
    if(!jcs.empty())
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
    Json::Value jval;
    jval[json_formatName()] = useAlias? m_formatSpec->GetAlias() : m_formatSpec->GetName();
    jval[json_unitName()] = m_unit->GetName();
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
void FormatUnitSet::LoadJsonData(Json::Value jval)
    {
    if (jval.empty())
        return;
    Utf8CP paramName;
    Utf8String formatName;
    m_problem = FormatProblemDetail();
    m_unit = nullptr;
    m_formatSpec = nullptr;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (stricmp(paramName, json_unitName()) == 0)
            {
            m_unitName = val.asString();
            m_unit = BEU::UnitRegistry::Instance().LookupUnit(m_unitName.c_str());
            if (nullptr == m_unit)
                m_problem.UpdateProblemCode(FormatProblemCode::UnknownUnitName);
            }
        else if (stricmp(paramName, json_formatName()) == 0)
            {
            formatName = val.asString();
            m_formatSpec = StdFormatSet::FindFormatSpec(formatName.c_str());
            if (nullptr == m_formatSpec)
                m_problem.UpdateProblemCode(FormatProblemCode::UnknownStdFormatName);
            }
        }
    }

//===================================================
//
// FormatUnitGroup Methods
//
//===================================================
Json::Value FormatUnitGroup::ToJson(bool useAlias)
    {
    Json::Value jval;
    FormatUnitSetCP fus = GetPersistenceFUS();
    if (nullptr == fus)
        return jval;
    jval[json_KOQName()] = m_name.c_str();
    jval[json_persistFUS()] = fus->ToJson(useAlias);
    size_t num = GetPresentationFUSCount();
    if (num == 0)
        return jval;
    Json::Value jarr;
    for (size_t i = 0; i < num; i++)
        {
        fus = GetPresentationFUS(i);
        jarr.append(fus->ToJson(useAlias));
        }
    if(!jarr.empty())
       jval[json_presentFUS()] = jarr;
    return jval;
    }

FormatUnitGroup::FormatUnitGroup(JsonValueCR jval)
    {
    m_problem = FormatProblemDetail();
    if (!jval.empty())
        {
        Utf8CP paramName;
        Utf8String formatName;
        FormatUnitSet fus;
        for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
            {
            paramName = iter.memberName();
            JsonValueCR val = *iter;
            if (stricmp(paramName, json_KOQName()) == 0)
                {
                m_name = val.asString();
                }
            else if (stricmp(paramName, json_persistFUS()) == 0)
                {
                fus = FormatUnitSet();
                fus.LoadJsonData(val);
                m_group.push_back(fus);
                }
            else if (stricmp(paramName, json_presentFUS()) == 0)
                {
                for (Json::Value::iterator iter = val.begin(); iter != val.end(); iter++)
                    {
                    fus = FormatUnitSet();
                    JsonValueCR val = *iter;
                    fus.LoadJsonData(val);
                    m_group.push_back(fus);
                    }
                }
            }
        }
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