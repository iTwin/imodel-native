/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/StdFormatSet.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>
#include "../../PrivateAPI/Formatting/AliasMappings.h"

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
StdFormatSet::StdFormatSet() : m_problem(FormatProblemCode::NoProblems)
    {
    StdInit();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
NumericFormatSpecCP StdFormatSet::AddFormat(Utf8CP name, NumericFormatSpecCR fmtP, CompositeValueSpecCR compS)
    {
    if (IsFormatDefined(name))
        {
        m_problem.UpdateProblemCode(FormatProblemCode::NFS_DuplicateSpecNameOrAlias);
        return nullptr;
        }
    FormatCP nfs = new Format(name, fmtP, compS);
    if (nullptr == nfs || nfs->IsProblem())
        {
        if (nullptr != nfs)
            delete nfs;
        return nullptr;
        }
    m_formatSet.push_back(*nfs);
    return nfs->GetNumericSpec();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
NumericFormatSpecCP StdFormatSet::AddFormat(Utf8CP name, NumericFormatSpecCR fmtP)
    {
    if (IsFormatDefined(name))
        {
        m_problem.UpdateProblemCode(FormatProblemCode::NFS_DuplicateSpecNameOrAlias);
        return nullptr;
        }
    FormatP nfs = new Format(name, fmtP);
    if (nullptr == nfs || nfs->IsProblem())
        {
        if (nullptr != nfs)
            delete nfs;
        return nullptr;
        }
    m_formatSet.push_back(*nfs);
    return nfs->GetNumericSpec();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
NumericFormatSpecCP StdFormatSet::AddFormat(Utf8CP jsonString, BEU::IUnitsContextCR context)
    {
    FormatCP nfs = AddNamedFormat(jsonString, context);
    if (nullptr == nfs)
        return nullptr;
    return nfs->GetNumericSpec();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
FormatCP StdFormatSet::AddNamedFormat(Utf8CP jsonString, BEU::IUnitsContextCR context)
    {
    Json::Value jval (Json::objectValue);
    Json::Reader::Parse(jsonString, jval);
    FormatP nfs = new Format();
    nfs->FromJson(jval, &context);

    if (nfs->IsProblem())
        return nullptr;

    if (IsFormatDefined(nfs->GetName().c_str()))
        return nullptr;

    m_formatSet.push_back(*nfs);
    return nfs;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
bool StdFormatSet::IsFormatDefined(Utf8CP name)
    {
    return m_formatSet.end() != std::find_if(m_formatSet.begin(), m_formatSet.end(),
        [name](FormatCR pNamedFmtSpec) -> bool {return pNamedFmtSpec.GetName() == name;});
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
FormatCP StdFormatSet::FindFormat(Utf8StringCR name) const
    {
    for (auto const& fmt : m_formatSet)
        {
        if (fmt.GetName() == name)
            return &fmt;
        }

    return nullptr;
    }

// Specialized "Constructor" to replace the now removed constructor in NumericFormatSpec.
NumericFormatSpec CreateNewNumericFormatSpec(PresentationType presentationType, ShowSignOption signOption, FormatTraits formatTraits, size_t const precision, Utf8CP uomSeparator = nullptr)
    {
    NumericFormatSpec nfs;

    // Replacement for NumericFormatSpec::Init
    nfs.SetRoundingFactor(0.0);
    nfs.SetPresentationType(presentationType);
    nfs.SetSignOption(signOption);
    nfs.SetFormatTraits(formatTraits);
    if (PresentationType::Fractional == presentationType)
        {
        nfs.SetDecimalPrecision(FormatConstant::DefaultDecimalPrecision());
        FractionalPrecision prec;
        Utils::FractionalPrecisionByDenominator(prec, precision);
        nfs.SetFractionalPrecision(prec);
        }
    else
        {
        DecimalPrecision prec;
        Utils::DecimalPrecisionByIndex(prec, precision );
        nfs.SetDecimalPrecision(prec);
        }
    nfs.SetDecimalSeparator(FormatConstant::FPV_DecimalSeparator());
    nfs.SetThousandSeparator(FormatConstant::FPV_ThousandSeparator());
    nfs.SetUomSeparator(FormatConstant::BlankString());
    nfs.SetStationSeparator('+');
    nfs.SetMinWidth(0);

    if (uomSeparator)
        nfs.SetUomSeparator(uomSeparator);
    return nfs;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
void StdFormatSet::StdInit()
    {
    m_formatSet.clear();
    FormatTraits traits = FormatConstant::DefaultFormatTraits();
    FormatTraits traitsU = FormatConstant::UnitizedFormatTraits();
    AddFormat("DefaultReal", NumericFormatSpec::DefaultFormat());
    AddFormat("DefaultRealU", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, FormatConstant::DefaultDecimalPrecisionIndex()));
    AddFormat("Real2", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, 2));
    AddFormat("Real3", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, 3));
    AddFormat("Real4", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, 4));

    AddFormat("Real2U", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 2));
    AddFormat("Real3U", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 3));
    AddFormat("Real4U", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 4));
    AddFormat("Real6U", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 6));

    // Add options for no space between value and unit label, common for ' (ft) and " (inch)
    AddFormat("Real2UNS", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 2, FormatConstant::EmptyString()));
    AddFormat("Real3UNS", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 3, FormatConstant::EmptyString()));
    AddFormat("Real4UNS", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 4, FormatConstant::EmptyString()));
    AddFormat("Real6UNS", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 6, FormatConstant::EmptyString()));

    NumericFormatSpec stop = CreateNewNumericFormatSpec(PresentationType::Station, ShowSignOption::OnlyNegative, traitsU, 2);
    stop.SetStationOffsetSize(2);
    AddFormat("Stop100-2u", stop);
    stop = CreateNewNumericFormatSpec(PresentationType::Station, ShowSignOption::OnlyNegative, traitsU, 2);
    stop.SetMinWidth(4);
    AddFormat("Stop100-2-4u", stop);

    stop = CreateNewNumericFormatSpec(PresentationType::Station, ShowSignOption::OnlyNegative, traitsU, 2);
    stop.SetMinWidth(4);
    stop.SetStationOffsetSize(3);
    AddFormat("Stop1000-2-4u", stop);

    stop = CreateNewNumericFormatSpec(PresentationType::Station, ShowSignOption::OnlyNegative, traitsU, 2);
    AddFormat("Stop1000-2u", stop);

    //=========================== Stoppers w/o units
    stop = CreateNewNumericFormatSpec(PresentationType::Station, ShowSignOption::OnlyNegative, traits, 2);
    stop.SetStationOffsetSize(2);
    AddFormat("Stop100-2", stop);
    stop = CreateNewNumericFormatSpec(PresentationType::Station, ShowSignOption::OnlyNegative, traits, 2);
    stop.SetStationOffsetSize(3);
    stop.SetMinWidth(4);
    AddFormat("Stop100-2-4", stop);

    stop = CreateNewNumericFormatSpec(PresentationType::Station, ShowSignOption::OnlyNegative, traits, 2);
    stop.SetStationOffsetSize(3);
    stop.SetMinWidth(4);
    AddFormat("Stop1000-2-4", stop);

    stop = CreateNewNumericFormatSpec(PresentationType::Station, ShowSignOption::OnlyNegative, traits, 2);
    stop.SetStationOffsetSize(3);
    AddFormat("Stop1000-2", stop);

    AddFormat("SignedReal", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()));
    AddFormat("ParenthsReal", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::NegativeParentheses, traits, FormatConstant::DefaultDecimalPrecisionIndex()));
    AddFormat("DefaultFractional", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultFractionalDenominator()));
    AddFormat("DefaultFractionalU", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, FormatConstant::DefaultFractionalDenominator()));
    AddFormat("SignedFractional", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::SignAlways, traits, FormatConstant::DefaultFractionalDenominator()));
    AddFormat("DefaultExp", CreateNewNumericFormatSpec(PresentationType::Scientific, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()));
    AddFormat("SignedExp", CreateNewNumericFormatSpec(PresentationType::Scientific, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()));
    auto format = CreateNewNumericFormatSpec(PresentationType::Scientific, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex());
    format.SetScientificType(ScientificType::Normal);
    AddFormat("NormalizedExp", format);
    AddFormat("DefaultInt", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()));
    AddFormat("Fractional4", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 4));
    AddFormat("Fractional8", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8));
    AddFormat("Fractional16", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 16));
    AddFormat("Fractional32", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 32));
    AddFormat("Fractional64", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 64));
    AddFormat("Fractional128", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 128));

    AddFormat("Fractional4U", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 4));
    AddFormat("Fractional8U", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 8));
    AddFormat("Fractional16U", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 16));
    AddFormat("Fractional32U", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 32));
    AddFormat("Fractional32U", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 64));
    AddFormat("Fractional128U", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 128));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus StdFormatSet::AddCompositeSpecs(BEU::IUnitsContextCR unitContext)
    {
    FormatTraits traits = FormatConstant::DefaultFormatTraits();
    FormatTraits traitsU = FormatConstant::UnitizedFormatTraits();

    BEU::UnitCP arcDeg = unitContext.LookupUnit("ARC_DEG");
    BEU::UnitCP arcMinute = unitContext.LookupUnit("ARC_MINUTE");
    BEU::UnitCP arcSecond = unitContext.LookupUnit("ARC_SECOND");

    CompositeValueSpecP cvs = new CompositeValueSpec(*arcDeg, *arcMinute, *arcSecond);
    cvs->SetUnitLabels("\xC2\xB0", u8"'", u8"\"");
    AddFormat("AngleDMS", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 0), *cvs);
    AddFormat("AngleDMS8", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), *cvs);
    cvs = new CompositeValueSpec(*arcDeg, *arcMinute);
    cvs->SetUnitLabels("\xC2\xB0", u8"'");
    AddFormat("AngleDM8", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), *cvs);

    BEU::UnitCP mi = unitContext.LookupUnit("MILE");
    BEU::UnitCP yrd = unitContext.LookupUnit("YRD");
    BEU::UnitCP ft = unitContext.LookupUnit("FT");
    BEU::UnitCP in = unitContext.LookupUnit("IN");
    BEU::UnitCP meter = unitContext.LookupUnit("M");

    cvs = new CompositeValueSpec(*mi, *yrd, *ft, *in);
    cvs->SetUnitLabels("mile(s)", "yrd(s)", "'", "\"");
    AddFormat("AmerMYFI4", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 4), *cvs);
    cvs = new CompositeValueSpec(*ft, *in);
    cvs->SetUnitLabels("'", "\"");
    AddFormat("AmerFI8", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), *cvs);
    AddFormat("AmerFI16", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 16), *cvs);
    AddFormat("AmerFI32", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 32), *cvs);

    cvs = new CompositeValueSpec(*mi, *ft, *in);
    cvs->SetUnitLabels("yrd(s)", "'", "\"");
    AddFormat("AmerYFI8", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), *cvs);

    cvs = new CompositeValueSpec(*meter);
    cvs->SetUnitLabels("m");
    AddFormat("Meters4u", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 4), *cvs);

    cvs = new CompositeValueSpec(*ft);
    cvs->SetUnitLabels("'");
    AddFormat("Feet4u", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 4, FormatConstant::EmptyString()), *cvs);

    cvs = new CompositeValueSpec(*in);
    cvs->SetUnitLabels("\"");
    AddFormat("Inches4u", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 4, FormatConstant::EmptyString()), *cvs);

    cvs = new CompositeValueSpec(*in);
    cvs->SetUnitLabels("\"");
    AddFormat("Inches18u", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, 8, FormatConstant::EmptyString()), *cvs);

    cvs = new CompositeValueSpec(*arcDeg);
    cvs->SetUnitLabels("\xC2\xB0");
    AddFormat("DecimalDeg4", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 4, FormatConstant::EmptyString()), *cvs);

    cvs = new CompositeValueSpec(*ft);

    cvs->SetUnitLabels("'");
    NumericFormatSpec stop = CreateNewNumericFormatSpec(PresentationType::Station, ShowSignOption::OnlyNegative, traitsU, 2, FormatConstant::EmptyString());
    stop.SetStationOffsetSize(2);
    stop.SetMinWidth(2);
    AddFormat("StationFt2", stop, *cvs);

    cvs = new CompositeValueSpec(*meter);
    cvs->SetUnitLabels("m");
    stop = CreateNewNumericFormatSpec(PresentationType::Station, ShowSignOption::OnlyNegative, traitsU, 2);
    stop.SetStationOffsetSize(3);
    stop.SetMinWidth(4);
    AddFormat("StationM4", stop, *cvs);

    // Adds trailZeroes to "Stop100-2u"
    AddFormat("{\"NumericFormat\":{\"decPrec\":2, \"formatTraits\" : {\"AppendUnitName\":\"true\",\"TrailZeroes\":\"true\"}, \"presentType\" : \"Stop100\"}, \"SpecAlias\" : \"stop100-2uz\", \"SpecName\" : \"Stop100-2uz\", \"SpecType\" : \"numeric\"}",
        unitContext);
    
    // Adds trailZeroes to "Stop100-2"
    AddFormat("{\"NumericFormat\":{\"decPrec\":2, \"formatTraits\" : {\"TrailZeroes\":\"true\"}, \"minWidth\" : 2, \"presentType\" : \"Stop100\"}, \"SpecAlias\" : \"stop100-2-2z\", \"SpecName\" : \"Stop100-2-2z\", \"SpecType\" : \"numeric\"}",
        unitContext);

    // "Stop1000-2-3z"
    AddFormat("{\"NumericFormat\":{\"decPrec\":2, \"formatTraits\" : {\"TrailZeroes\":\"true\"}, \"minWidth\" : 3, \"presentType\" : \"Stop1000\"}, \"SpecAlias\" : \"stop1000-2-3z\", \"SpecName\" : \"stop1000-2-3z\", \"SpecType\" : \"numeric\"}",
        unitContext);

    // "hms"
    AddFormat("{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"hour(s)\", \"unitName\" : \"HR\"}, \"MiddleUnit\" : {\"unitLabel\":\"min\", \"unitName\" : \"MIN\"}, \"MinorUnit\" : {\"unitLabel\":\"sec\", \"unitName\" : \"S\"}, \"includeZero\" : true}, \"NumericFormat\" : {\"decPrec\":2, \"presentType\" : \"Decimal\"}, \"SpecAlias\" : \"hms\", \"SpecName\" : \"HMS\", \"SpecType\" : \"composite\"}",
        unitContext);
    
    // Duplicate of "dms", just different name.
    AddFormat("{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"\xC2\xB0\", \"unitName\" : \"ARC_DEG\"}, \"MiddleUnit\" : {\"unitLabel\":\"'\", \"unitName\" : \"ARC_MINUTE\"}, \"MinorUnit\" : {\"unitLabel\":\"\\\"\", \"unitName\" : \"ARC_SECOND\"}, \"includeZero\" : true}, \"NumericFormat\" : {\"fractPrec\":1, \"presentType\" : \"Fractional\"}, \"SpecAlias\" : \"cdms\", \"SpecName\" : \"CAngleDMS\", \"SpecType\" : \"composite\"}",
        unitContext);

    // Duplicate of "dms8", just different name.
    AddFormat("{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"\xC2\xB0\", \"unitName\" : \"ARC_DEG\"}, \"MiddleUnit\" : {\"unitLabel\":\"'\", \"unitName\" : \"ARC_MINUTE\"}, \"MinorUnit\" : {\"unitLabel\":\"\\\"\", \"unitName\" : \"ARC_SECOND\"}, \"includeZero\" : true}, \"NumericFormat\" : {\"fractPrec\":8, \"presentType\" : \"Fractional\"}, \"SpecAlias\" : \"cdms8\", \"SpecName\" : \"CAngleDMS8\", \"SpecType\" : \"composite\"}",
        unitContext);

    // Duplicate of "dm8", just different name.
    AddFormat("{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"\xC2\xB0\", \"unitName\" : \"ARC_DEG\"}, \"MiddleUnit\" : {\"unitLabel\":\"'\", \"unitName\" : \"ARC_MINUTE\"}, \"includeZero\" : true}, \"NumericFormat\" : {\"fractPrec\":8, \"presentType\" : \"Fractional\"}, \"SpecAlias\" : \"cdm8\", \"SpecName\" : \"CAngleDM8\", \"SpecType\" : \"composite\"}",
        unitContext);

    return SUCCESS;
    }

//=======================================================================================
// AliasMappings
//=======================================================================================

AliasMappings * AliasMappings::s_mappings = nullptr;

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
AliasMappings::AliasMappings()
    {
    AddMappings();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
// static
AliasMappings * AliasMappings::GetMappings()
    {
    if (nullptr == s_mappings)
        s_mappings = new AliasMappings();
    return s_mappings;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
// static
Utf8CP AliasMappings::TryGetAliasFromName(Utf8CP name)
    {
    auto iter = GetMappings()->m_nameAliasMapping.find(name);
    if (iter == GetMappings()->m_nameAliasMapping.end())
        return nullptr;
    return iter->second.c_str();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
// static
Utf8CP AliasMappings::TryGetNameFromAlias(Utf8CP alias)
    {
    auto iter = GetMappings()->m_aliasNameMapping.find(alias);
    if (iter == GetMappings()->m_aliasNameMapping.end())
        return nullptr;
    return iter->second.c_str();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
void AliasMappings::AddMappings()
    {
    AddMapping("cdm8", "CAngleDM8");
    AddMapping("cdms", "CAngleDMS");
    AddMapping("cdms8", "CAngleDMS8");

    AddMapping("decimalDeg4", "DecimalDeg4");
    AddMapping("dm8", "AngleDM8");
    AddMapping("dms", "AngleDMS");
    AddMapping("dms8", "AngleDMS8");
    AddMapping("feet4u", "Feet4u");
    AddMapping("fi8", "AmerFI8");
    AddMapping("fi16", "AmerFI16");
    AddMapping("fi32", "AmerFI32");
    AddMapping("fract", "DefaultFractional");
    AddMapping("fractu", "DefaultFractionalU");
    AddMapping("fract4", "Fractional4");
    AddMapping("fract4u", "Fractional4U");
    AddMapping("fract8", "Fractional8");
    AddMapping("fract8u", "Fractional8U");
    AddMapping("fract16", "Fractional16");
    AddMapping("fract16u", "Fractional16U");
    AddMapping("fract32", "Fractional32");
    AddMapping("fract32u", "Fractional32U");
    AddMapping("fract64", "Fractional64");
    AddMapping("fract64u", "Fractional64U");
    AddMapping("fract128", "Fractional128");
    AddMapping("fract128u", "Fractional128U");

    AddMapping("fractSign", "SignedFractional");

    AddMapping("yfi8", "AmerYFI8");
    AddMapping("meters4u", "Meters4u");
        
    AddMapping("inches4u", "Inches4u");
    AddMapping("Inches18u", "Inches18u");
    AddMapping("int", "DefaultInt");

    AddMapping("myfi4", "AmerMYFI4");

    AddMapping("hms", "HMS");

    AddMapping("real", "DefaultReal");
    AddMapping("realu", "DefaultRealU");
    AddMapping("real2", "Real2");
    AddMapping("real2u", "Real2U");
    AddMapping("real2uns", "Real2UNS");
    AddMapping("real3", "Real3");
    AddMapping("real3u", "Real3U");
    AddMapping("real3uns", "Real3UNS");
    AddMapping("real4", "Real4");
    AddMapping("real4u", "Real4U");
    AddMapping("real4uns", "Real4UNS");
    AddMapping("real6u", "Real6U");
    AddMapping("real6uns", "Real6UNS");

    AddMapping("realPth", "ParenthsReal");
    AddMapping("realSign", "SignedReal");

    AddMapping("sci", "DefaultExp");
    AddMapping("sciN", "NormalizedExp");
    AddMapping("sciSign", "SignedExp");

    AddMapping("stationFt2", "StationFt2");
    AddMapping("stationM4", "StationM4");

    AddMapping("stop100-2-2z", "Stop100-2-2z");
    AddMapping("stop100-2-4", "Stop100-2-4");
    AddMapping("stop100-2-4u", "Stop100-2-4u");
    AddMapping("stop100-2", "Stop100-2");
    AddMapping("stop100-2u", "Stop100-2u");
    AddMapping("stop100-2uz", "Stop100-2uz");

    AddMapping("stop1000-2-3z", "Stop1000-2-3z");
    AddMapping("stop1000-2-4", "Stop1000-2-4");
    AddMapping("stop1000-2-4u", "Stop1000-2-4u");
    AddMapping("stop1000-2", "Stop1000-2");
    AddMapping("stop1000-2u", "Stop1000-2u");
    }

END_BENTLEY_FORMATTING_NAMESPACE
