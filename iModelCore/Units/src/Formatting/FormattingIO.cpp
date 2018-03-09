/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/FormattingIO.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>
#include "../../PrivateAPI/Units/UnitRegistry.h"

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
StdFormatSetP StdFormatSet::Set() 
    { 
    static StdFormatSetP set = nullptr; 
    if (nullptr == set)
        {
        set = new StdFormatSet();
        set->m_unitsRegistry = &BEU::UnitRegistry::Get();
        set->StdInit();
        set->CustomInit();
        }
    return set;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
size_t StdFormatSet::StdInit()
    {
    // Specialized "Constructor" to replace the now removed constructor in NumericFormatSpec.
    static auto const CreateNewNumericFormatSpec = [](
        PresentationType presentationType,
        ShowSignOption signOption,
        FormatTraits formatTraits,
        size_t const precision,
        Utf8CP uomSeparator = nullptr
    ) -> NumericFormatSpecP
        {
        NumericFormatSpecP nfs = new NumericFormatSpec;

        // Replacement for NumericFormatSpec::Init
        nfs->SetRoundingFactor(0.0);
        nfs->SetPresentationType(presentationType);
        nfs->SetSignOption(signOption);
        nfs->SetFormatTraits(formatTraits);
        nfs->SetFractionalBarType(FractionBarType::Diagonal);
        if (PresentationType::Fractional == presentationType)
            {
            nfs->SetDecimalPrecision(FormatConstant::DefaultDecimalPrecision());
            nfs->SetFractionaPrecision(Utils::FractionalPrecisionByDenominator(precision));
            }
        else
            {
            nfs->SetDecimalPrecision(Utils::DecimalPrecisionByIndex(precision));
            nfs->SetFractionaPrecision(FormatConstant::DefaultFractionalPrecision());
            }
        nfs->SetDecimalSeparator(FormatConstant::FPV_DecimalSeparator());
        nfs->SetThousandSeparator(FormatConstant::FPV_ThousandSeparator());
        nfs->SetUomSeparator(FormatConstant::BlankString());
        nfs->SetStopSeparator('+');
        nfs->SetMinWidth(0);

        if (uomSeparator)
            nfs->SetUomSeparator(uomSeparator);
        return nfs;
        };

    m_formatSet.clear();
    FormatTraits traits = FormatConstant::DefaultFormatTraits();
    FormatTraits traitsU = FormatConstant::UnitizedFormatTraits();
    //AddFormat("DefaultReal", CreateNewNumericFormatSpec( PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()), "real");
    AddFormat(FormatConstant::DefaultFormatName(), new NumericFormatSpec(NumericFormatSpec::DefaultFormat()), FormatConstant::DefaultFormatAlias());
    AddFormat("DefaultRealU", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, FormatConstant::DefaultDecimalPrecisionIndex()), "realu");
    AddFormat("Real2", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, 2), "real2");
    AddFormat("Real3", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, 3), "real3");
    AddFormat("Real4", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, 4), "real4");

    AddFormat("Real2U", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 2), "real2u");
    AddFormat("Real3U", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 3), "real3u");
    AddFormat("Real4U", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 4), "real4u");
    AddFormat("Real6U", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 6), "real6u");

    // Add options for no space between value and unit label, common for ' (ft) and " (inch)
    AddFormat("Real2UNS", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 2, FormatConstant::EmptyString()), "real2uns");
    AddFormat("Real3UNS", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 3, FormatConstant::EmptyString()), "real3uns");
    AddFormat("Real4UNS", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 4, FormatConstant::EmptyString()), "real4uns");
    AddFormat("Real6UNS", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 6, FormatConstant::EmptyString()), "real6uns");

    NumericFormatSpec stop = CreateNewNumericFormatSpec(PresentationType::Stop100, ShowSignOption::OnlyNegative, traitsU, 2);
    AddFormat("Stop100-2u", stop, "stop100-2u");
    stop = CreateNewNumericFormatSpec(PresentationType::Stop100, ShowSignOption::OnlyNegative, traitsU, 2);
    stop.SetMinWidth(4);
    AddFormat("Stop100-2-4u", stop, "stop100-2-4u");

    stop = CreateNewNumericFormatSpec(PresentationType::Stop1000, ShowSignOption::OnlyNegative, traitsU, 2);
    stop.SetMinWidth(4);
    AddFormat("Stop1000-2-4u", stop, "stop1000-2-4u");

    stop = CreateNewNumericFormatSpec(PresentationType::Stop1000, ShowSignOption::OnlyNegative, traitsU, 2);
    AddFormat("Stop1000-2u", stop, "stop1000-2u");

    //=========================== Stoppers w/o units
    stop = CreateNewNumericFormatSpec(PresentationType::Stop100, ShowSignOption::OnlyNegative, traits, 2);
    AddFormat("Stop100-2", stop, "stop100-2");
    stop = CreateNewNumericFormatSpec(PresentationType::Stop100, ShowSignOption::OnlyNegative, traits, 2);
    stop.SetMinWidth(4);
    AddFormat("Stop100-2-4", stop, "stop100-2-4");

    stop = CreateNewNumericFormatSpec(PresentationType::Stop1000, ShowSignOption::OnlyNegative, traits, 2);
    stop.SetMinWidth(4);
    AddFormat("Stop1000-2-4", stop, "stop1000-2-4");

    stop = CreateNewNumericFormatSpec(PresentationType::Stop1000, ShowSignOption::OnlyNegative, traits, 2);
    AddFormat("Stop1000-2", stop, "stop1000-2");

    // %f  %.04f
    //AddFormat("RoadStatF", CreateNewNumericFormatSpec(PresentationType::Station100, ShowSignOption::OnlyNegative, traits, 2), "rsf2");
    //AddFormat("RoadStatM", CreateNewNumericFormatSpec(PresentationType::Station1000, ShowSignOption::OnlyNegative, traits, 2), "rsm2");

    AddFormat("SignedReal", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()), "realSign");
    AddFormat("ParenthsReal", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::NegativeParentheses, traits, FormatConstant::DefaultDecimalPrecisionIndex()), "realPth");
    AddFormat("DefaultFractional", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultFractionalDenominator()), "fract");
    AddFormat("DefaultFractionalU", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, FormatConstant::DefaultFractionalDenominator()), "fractu");
    AddFormat("SignedFractional", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::SignAlways, traits, FormatConstant::DefaultFractionalDenominator()), "fractSign");
    AddFormat("DefaultExp", CreateNewNumericFormatSpec(PresentationType::Scientific, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()), "sci");
    AddFormat("SignedExp", CreateNewNumericFormatSpec(PresentationType::Scientific, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()), "sciSign");
    AddFormat("NormalizedExp", CreateNewNumericFormatSpec(PresentationType::ScientificNorm, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()), "sciN");
    AddFormat("DefaultInt", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()), "int");
    AddFormat("Fractional4", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 4), "fract4");
    AddFormat("Fractional8", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), "fract8");
    AddFormat("Fractional16", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 16), "fract16");
    AddFormat("Fractional32", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 32), "fract32");
	AddFormat("Fractional64", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 64), "fract64");
    AddFormat("Fractional128", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 128), "fract128");

    AddFormat("Fractional4U", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 4), "fract4u");
    AddFormat("Fractional8U", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 8), "fract8u");
    AddFormat("Fractional16U", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 16), "fract16u");
    AddFormat("Fractional32U", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 32), "fract32u");
	AddFormat("Fractional32U", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 64), "fract64u");
    AddFormat("Fractional128U", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 128), "fract128u");

    BEU::UnitCP arcDeg = m_unitsRegistry->LookupUnit("ARC_DEG");
    BEU::UnitCP arcMinute = m_unitsRegistry->LookupUnit("ARC_MINUTE");
    BEU::UnitCP arcSecond = m_unitsRegistry->LookupUnit("ARC_SECOND");

    CompositeValueSpecP cvs = new CompositeValueSpec(arcDeg, arcMinute, arcSecond, nullptr);
    cvs->SetUnitLabels("\xC2\xB0", u8"'", u8"\"");
    AddFormat("AngleDMS", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 0), *cvs, "dms");
    AddFormat("AngleDMS8", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), *cvs, "dms8");
    cvs = new CompositeValueSpec(arcDeg, arcMinute);
    cvs->SetUnitLabels("\xC2\xB0", u8"'");
    AddFormat("AngleDM8", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), *cvs, "dm8");

    BEU::UnitCP mi = m_unitsRegistry->LookupUnit("MILE");
    BEU::UnitCP yrd = m_unitsRegistry->LookupUnit("YRD");
    BEU::UnitCP ft = m_unitsRegistry->LookupUnit("FT");
    BEU::UnitCP in = m_unitsRegistry->LookupUnit("IN");
    BEU::UnitCP meter = m_unitsRegistry->LookupUnit("M");

    cvs = new CompositeValueSpec(mi, yrd, ft, in);
    cvs->SetUnitLabels("mile(s)", "yrd(s)", "'", "\"");
    AddFormat("AmerMYFI4", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 4), *cvs, "myfi4");
    cvs = new CompositeValueSpec(ft, in);
    cvs->SetUnitLabels("'", "\"");
    AddFormat("AmerFI8", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), *cvs, "fi8");
    AddFormat("AmerFI16", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 16), *cvs, "fi16");
    AddFormat("AmerFI32", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 32), *cvs, "fi32");

    cvs = new CompositeValueSpec(mi, ft, in);
    cvs->SetUnitLabels("yrd(s)", "'", "\"");
    AddFormat("AmerYFI8", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), *cvs, "yfi8");

    cvs = new CompositeValueSpec(meter);
    cvs->SetUnitLabels("m");
    AddFormat("Meters4u", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 4), *cvs, "meters4u");

    cvs = new CompositeValueSpec(ft);
    cvs->SetUnitLabels("'");
    AddFormat("Feet4u", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 4, FormatConstant::EmptyString()), *cvs, "feet4u");

    cvs = new CompositeValueSpec(in);
    cvs->SetUnitLabels("\"");
    AddFormat("Inches4u", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 4, FormatConstant::EmptyString()), *cvs, "inches4u");

    cvs = new CompositeValueSpec(in);
    cvs->SetUnitLabels("\"");
    AddFormat("Inches18u", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 8, FormatConstant::EmptyString()), *cvs, "Inches18u");

    cvs = new CompositeValueSpec(arcDeg);
    cvs->SetUnitLabels("\xC2\xB0");
    AddFormat("DecimalDeg4", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 4, FormatConstant::EmptyString()), *cvs, "decimalDeg4");

    cvs = new CompositeValueSpec(ft);
    cvs->SetUnitLabels("'");
    stop = CreateNewNumericFormatSpec(PresentationType::Stop100, ShowSignOption::OnlyNegative, traitsU, 2, FormatConstant::EmptyString());
    stop.SetMinWidth(2);
    AddFormat("StationFt2", stop, *cvs, "stationFt2");

    cvs = new CompositeValueSpec(meter);
    cvs->SetUnitLabels("m");
    stop = CreateNewNumericFormatSpec(PresentationType::Stop1000, ShowSignOption::OnlyNegative, traitsU, 2);
    stop.SetMinWidth(4);
    AddFormat("StationM4", stop, *cvs, "stationM4");

    size_t s = m_formatSet.size();
    return s;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void StdFormatSet::CustomInit()
    {
    // Adds trailZeroes to "Stop100-2u"
    AddFormat("{\"NumericFormat\":{\"decPrec\":2, \"formatTraits\" : {\"AppendUnitName\":\"true\",\"TrailZeroes\":\"true\"}, \"presentType\" : \"Stop100\"}, \"SpecAlias\" : \"stop100-2uz\", \"SpecName\" : \"Stop100-2uz\", \"SpecType\" : \"numeric\"}");
    
    // Adds trailZeroes to "Stop100-2"
    AddFormat("{\"NumericFormat\":{\"decPrec\":2, \"formatTraits\" : {\"TrailZeroes\":\"true\"}, \"minWidth\" : 2, \"presentType\" : \"Stop100\"}, \"SpecAlias\" : \"stop100-2-2z\", \"SpecName\" : \"Stop100-2-2z\", \"SpecType\" : \"numeric\"}");

    // "Stop1000-2-3z"
    AddFormat("{\"NumericFormat\":{\"decPrec\":2, \"formatTraits\" : {\"TrailZeroes\":\"true\"}, \"minWidth\" : 3, \"presentType\" : \"Stop1000\"}, \"SpecAlias\" : \"stop1000-2-3z\", \"SpecName\" : \"Stop1000-2-3z\", \"SpecType\" : \"numeric\"}");

    // "hms"
    AddFormat("{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"hour(s)\", \"unitName\" : \"HR\"}, \"MiddleUnit\" : {\"unitLabel\":\"min\", \"unitName\" : \"MIN\"}, \"MinorUnit\" : {\"unitLabel\":\"sec\", \"unitName\" : \"S\"}, \"includeZero\" : true}, \"NumericFormat\" : {\"decPrec\":2, \"presentType\" : \"Decimal\"}, \"SpecAlias\" : \"hms\", \"SpecName\" : \"HMS\", \"SpecType\" : \"composite\"}");
    
    // Duplicate of "dms", just different name.
    AddFormat("{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"\xC2\xB0\", \"unitName\" : \"ARC_DEG\"}, \"MiddleUnit\" : {\"unitLabel\":\"'\", \"unitName\" : \"ARC_MINUTE\"}, \"MinorUnit\" : {\"unitLabel\":\"\\\"\", \"unitName\" : \"ARC_SECOND\"}, \"includeZero\" : true}, \"NumericFormat\" : {\"fractPrec\":1, \"presentType\" : \"Fractional\"}, \"SpecAlias\" : \"cdms\", \"SpecName\" : \"CAngleDMS\", \"SpecType\" : \"composite\"}");

    // Duplicate of "dms8", just different name.
    AddFormat("{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"\xC2\xB0\", \"unitName\" : \"ARC_DEG\"}, \"MiddleUnit\" : {\"unitLabel\":\"'\", \"unitName\" : \"ARC_MINUTE\"}, \"MinorUnit\" : {\"unitLabel\":\"\\\"\", \"unitName\" : \"ARC_SECOND\"}, \"includeZero\" : true}, \"NumericFormat\" : {\"fractPrec\":8, \"presentType\" : \"Fractional\"}, \"SpecAlias\" : \"cdms8\", \"SpecName\" : \"CAngleDMS8\", \"SpecType\" : \"composite\"}");

    // Duplicate of "dm8", just different name.
    AddFormat("{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"\xC2\xB0\", \"unitName\" : \"ARC_DEG\"}, \"MiddleUnit\" : {\"unitLabel\":\"'\", \"unitName\" : \"ARC_MINUTE\"}, \"includeZero\" : true}, \"NumericFormat\" : {\"fractPrec\":8, \"presentType\" : \"Fractional\"}, \"SpecAlias\" : \"cdm8\", \"SpecName\" : \"CAngleDM8\", \"SpecType\" : \"composite\"}");
    }

//===================================================
//
// NumericFormatSpec Methods
//
//===================================================

void NumericFormatSpec::LoadJson(Json::Value jval)
    {
    DefaultInit(FormatConstant::DefaultDecimalPrecisionIndex());
    if (jval.empty())
        return;

    Utf8CP paramName;
    Utf8String str;
    Utf8String jStr = jval.ToString();
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (BeStringUtilities::StricmpAscii(paramName, json_presentType()) == 0)
            {
            m_presentationType = Utils::NameToPresentationType(val.asCString());
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_roundFactor()) == 0)
            {
            m_roundFactor = val.asDouble();
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_decPrec()) == 0)
            {
            m_decPrecision = Utils::DecimalPrecisionByIndex(val.asInt64());
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_fractPrec()) == 0)
            {
            m_fractPrecision = Utils::FractionalPrecisionByDenominator(val.asInt64());
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_signOpt()) == 0)
            {
            m_signOption = Utils::NameToSignOption(val.asCString());
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_barType()) == 0)
            {
            m_barType = Utils::NameToFractionBarType(val.asCString());
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_decimalSeparator()) == 0)
            {
            str = val.asString();
            m_decimalSeparator = str.c_str()[0];
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_thousandSeparator()) == 0)
            {
            str = val.asString();
            m_thousandsSeparator = str.c_str()[0];
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_uomSeparator()) == 0)
            {
            m_uomSeparator = val.asString();
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_statSeparator()) == 0)
            {
            str = val.asString();
            m_statSeparator = str.c_str()[0];
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_minWidth()) == 0)
            {
            m_minWidth = val.asInt();
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_formatTraits()) == 0)
            {
            TraitsFromJson(val);
            }
        } // for
    }

NumericFormatSpec::NumericFormatSpec(Json::Value jval)
    {
    LoadJson(jval);
    }

NumericFormatSpec::NumericFormatSpec(Utf8CP jsonString)
    {
    Json::Value jval (Json::objectValue);
    Json::Reader::Parse(jsonString, jval);
    LoadJson(jval);
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
        if (BeStringUtilities::StricmpAscii(paramName, json_LeadZeroes()) == 0)
            SetUseLeadingZeroes(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_TrailZeroes()) == 0)
            SetKeepTrailingZeroes(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_KeepDecPnt()) == 0)
            SetKeepDecimalPoint(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_KeepSingleZero()) == 0)
            SetKeepSingleZero(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_ExponentZero()) == 0)
            SetExponentZero(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_ZeroEmpty()) == 0)
            SetZeroEmpty(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_Use1000Separator()) == 0)
            SetUse1000Separator(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_ApplyRounding()) ==0)
            SetApplyRounding(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_FractionDash()) == 0)
            SetFractionDash(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_UseFractSymbol()) == 0)
            SetUseFractSymbol(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_AppendUnitName()) == 0)
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
        jNFC[json_fractPrec()] = Utils::FractionalPrecisionDenominator(m_fractPrecision);
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
    if (verbose || m_minWidth != defSpec.m_minWidth)
        jNFC[json_minWidth()] = m_minWidth;
    return jNFC;
    }

//===================================================
//
// NamedFormatSpec Methods
//
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
void NamedFormatSpec::Init(FormatProblemCode prob)
    {
    m_name.clear();
    m_alias.clear();
    m_description.clear();
    m_displayLabel.clear();
    m_numericSpec = NumericFormatSpec();
    m_compositeSpec.Init();
    m_specType = FormatSpecType::Undefined;
    m_problem.UpdateProblemCode(prob);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Json::Value NamedFormatSpec::ToJson(bool verbose) const
    {
    Json::Value jNFS;
    jNFS[json_SpecName()] = m_name;
    jNFS[json_SpecAlias()] = m_alias;
    if (!m_description.empty())
        jNFS[json_SpecDescript()] = m_description;
    if (!m_displayLabel.empty())
        jNFS[json_SpecLabel()] = m_displayLabel;

    jNFS[json_SpecType()] = Utils::FormatSpecTypeToName(m_specType);
    jNFS[json_NumericFormat()] = m_numericSpec.ToJson(verbose);
    Json::Value jcs = m_compositeSpec.ToJson();
    if (!jcs.empty())
        jNFS[json_CompositeFormat()] = jcs; // m_compositeSpec.ToJson();
    return jNFS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
void NamedFormatSpec::LoadJson(Json::Value jval, BEU::IUnitsContextCP context)
    {
    Utf8CP paramName;
    Init();
    if (jval.empty())
        {
        m_problem.UpdateProblemCode(FormatProblemCode::NFS_InvalidJsonObject);
        return;
        }

    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (BeStringUtilities::StricmpAscii(paramName, json_SpecName()) == 0)
            m_name = val.asString();
        else if (BeStringUtilities::StricmpAscii(paramName, json_SpecAlias()) == 0)
            m_alias = val.asString();
        else if (BeStringUtilities::StricmpAscii(paramName, json_SpecDescript()) == 0)
            m_description = val.asString();
        else if (BeStringUtilities::StricmpAscii(paramName, json_SpecLabel()) == 0)
            m_displayLabel = val.asString();
        else if (BeStringUtilities::StricmpAscii(paramName, json_SpecType()) == 0)
            m_specType = Utils::NameToFormatSpecType(val.asCString());
        else if (BeStringUtilities::StricmpAscii(paramName, json_NumericFormat()) == 0)
            m_numericSpec = NumericFormatSpec(val);
        else if (BeStringUtilities::StricmpAscii(paramName, json_CompositeFormat()) == 0)
            m_compositeSpec.LoadJsonData(val, context);
        }
    }

void NamedFormatSpec::LoadJson(Utf8CP jsonString, BEU::IUnitsContextCP context)
    {
    Json::Value jval (Json::objectValue);
    Json::Reader::Parse(jsonString, jval);
    LoadJson(jval, context);
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
NamedFormatSpec::NamedFormatSpec(Json::Value jval, BEU::IUnitsContextCP context)
    {
    LoadJson(jval, context);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
NamedFormatSpec::NamedFormatSpec(Utf8CP jsonString, BEU::IUnitsContextCP context)
    {
    Json::Value jval (Json::objectValue);
    Json::Reader::Parse(jsonString, jval);
    LoadJson(jval, context);
    }

//===================================================
//
// FormatUnitSet Methods
//
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Json::Value FormatUnitSet::ToJson(bool useAlias, bool verbose) const
    {
    Json::Value jval;
    if (!m_fusName.empty())
        jval[json_fusName()] = m_fusName.c_str();
    jval[json_unitName()] = m_unit->GetName();
    if(verbose)
        jval[json_formatSpec()] = m_formatSpec->ToJson(true);
    else
        jval[json_formatName()] = useAlias? m_formatSpec->GetAlias() : m_formatSpec->GetName();
    return jval;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Utf8String FormatUnitSet::ToJsonString(bool useAlias, bool verbose) const
    {
    Utf8String str;
    Json::Value jval = ToJson(useAlias, verbose);
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
        if (BeStringUtilities::StricmpAscii(paramName, json_unitName()) == 0)
            {
            m_unitName = val.asString();
            m_unit = BEU::UnitRegistry::Get().LookupUnit(m_unitName.c_str());
            if (nullptr == m_unit)
                m_problem.UpdateProblemCode(FormatProblemCode::UnknownUnitName);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_formatName()) == 0)
            {
            formatName = val.asString();
            m_formatSpec = StdFormatSet::FindFormatSpec(formatName.c_str());
            if (nullptr == m_formatSpec)
                m_problem.UpdateProblemCode(FormatProblemCode::UnknownStdFormatName);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_fusName()) == 0)
            {
            m_fusName = val.asString();
            }
        }
    }

END_BENTLEY_FORMATTING_NAMESPACE