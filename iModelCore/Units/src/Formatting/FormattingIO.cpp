/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/FormattingIO.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//===================================================
//
// StdFormatSet Methods
//
//===================================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
StdFormatSetP StdFormatSet::Set() 
    { 
    static StdFormatSetP set = nullptr; 
    if (nullptr == set)
        {
        set = new StdFormatSet();
        set->StdInit();
        }
    return set;
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
    nfs.SetFractionalBarType(FractionBarType::Diagonal);
    if (PresentationType::Fractional == presentationType)
    {
        nfs.SetDecimalPrecision(FormatConstant::DefaultDecimalPrecision());
        nfs.SetFractionaPrecision(Utils::FractionalPrecisionByDenominator(precision));
    }
    else
    {
        nfs.SetDecimalPrecision(Utils::DecimalPrecisionByIndex(precision));
        nfs.SetFractionaPrecision(FormatConstant::DefaultFractionalPrecision());
    }
    nfs.SetDecimalSeparator(FormatConstant::FPV_DecimalSeparator());
    nfs.SetThousandSeparator(FormatConstant::FPV_ThousandSeparator());
    nfs.SetUomSeparator(FormatConstant::BlankString());
    nfs.SetStopSeparator('+');
    nfs.SetMinWidth(0);

    if (uomSeparator)
        nfs.SetUomSeparator(uomSeparator);
    return nfs;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
size_t StdFormatSet::StdInit()
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

    NumericFormatSpec stop = CreateNewNumericFormatSpec(PresentationType::Stop100, ShowSignOption::OnlyNegative, traitsU, 2);
    AddFormat("Stop100-2u", stop);
    stop = CreateNewNumericFormatSpec(PresentationType::Stop100, ShowSignOption::OnlyNegative, traitsU, 2);
    stop.SetMinWidth(4);
    AddFormat("Stop100-2-4u", stop);

    stop = CreateNewNumericFormatSpec(PresentationType::Stop1000, ShowSignOption::OnlyNegative, traitsU, 2);
    stop.SetMinWidth(4);
    AddFormat("Stop1000-2-4u", stop);

    stop = CreateNewNumericFormatSpec(PresentationType::Stop1000, ShowSignOption::OnlyNegative, traitsU, 2);
    AddFormat("Stop1000-2u", stop);

    //=========================== Stoppers w/o units
    stop = CreateNewNumericFormatSpec(PresentationType::Stop100, ShowSignOption::OnlyNegative, traits, 2);
    AddFormat("Stop100-2", stop);
    stop = CreateNewNumericFormatSpec(PresentationType::Stop100, ShowSignOption::OnlyNegative, traits, 2);
    stop.SetMinWidth(4);
    AddFormat("Stop100-2-4", stop);

    stop = CreateNewNumericFormatSpec(PresentationType::Stop1000, ShowSignOption::OnlyNegative, traits, 2);
    stop.SetMinWidth(4);
    AddFormat("Stop1000-2-4", stop);

    stop = CreateNewNumericFormatSpec(PresentationType::Stop1000, ShowSignOption::OnlyNegative, traits, 2);
    AddFormat("Stop1000-2", stop);

    // %f  %.04f
    //AddFormat("RoadStatF", CreateNewNumericFormatSpec(PresentationType::Station100, ShowSignOption::OnlyNegative, traits, 2), "rsf2");
    //AddFormat("RoadStatM", CreateNewNumericFormatSpec(PresentationType::Station1000, ShowSignOption::OnlyNegative, traits, 2), "rsm2");

    AddFormat("SignedReal", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()));
    AddFormat("ParenthsReal", CreateNewNumericFormatSpec(PresentationType::Decimal, ShowSignOption::NegativeParentheses, traits, FormatConstant::DefaultDecimalPrecisionIndex()));
    AddFormat("DefaultFractional", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultFractionalDenominator()));
    AddFormat("DefaultFractionalU", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, FormatConstant::DefaultFractionalDenominator()));
    AddFormat("SignedFractional", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::SignAlways, traits, FormatConstant::DefaultFractionalDenominator()));
    AddFormat("DefaultExp", CreateNewNumericFormatSpec(PresentationType::Scientific, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()));
    AddFormat("SignedExp", CreateNewNumericFormatSpec(PresentationType::Scientific, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()));
    AddFormat("NormalizedExp", CreateNewNumericFormatSpec(PresentationType::ScientificNorm, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()));
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

    size_t s = m_formatSet.size();
    return s;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void StdFormatSet::CompositeSpecsInit(BEU::IUnitsContextCP unitContext)
    {
    if (nullptr == unitContext)
        return;

    FormatTraits traits = FormatConstant::DefaultFormatTraits();
    FormatTraits traitsU = FormatConstant::UnitizedFormatTraits();

    BEU::UnitCP arcDeg = unitContext->LookupUnit("ARC_DEG");
    BEU::UnitCP arcMinute = unitContext->LookupUnit("ARC_MINUTE");
    BEU::UnitCP arcSecond = unitContext->LookupUnit("ARC_SECOND");

    CompositeValueSpecP cvs = new CompositeValueSpec(arcDeg, arcMinute, arcSecond, nullptr);
    cvs->SetUnitLabels("\xC2\xB0", u8"'", u8"\"");
    AddFormat("AngleDMS", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 0), *cvs, "dms");
    AddFormat("AngleDMS8", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), *cvs, "dms8");
    cvs = new CompositeValueSpec(arcDeg, arcMinute);
    cvs->SetUnitLabels("\xC2\xB0", u8"'");
    AddFormat("AngleDM8", CreateNewNumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, 8), *cvs, "dm8");

    BEU::UnitCP mi = unitContext->LookupUnit("MILE");
    BEU::UnitCP yrd = unitContext->LookupUnit("YRD");
    BEU::UnitCP ft = unitContext->LookupUnit("FT");
    BEU::UnitCP in = unitContext->LookupUnit("IN");
    BEU::UnitCP meter = unitContext->LookupUnit("M");

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
    NumericFormatSpec stop = CreateNewNumericFormatSpec(PresentationType::Stop100, ShowSignOption::OnlyNegative, traitsU, 2, FormatConstant::EmptyString());
    stop.SetMinWidth(2);
    AddFormat("StationFt2", stop, *cvs, "stationFt2");

    cvs = new CompositeValueSpec(meter);
    cvs->SetUnitLabels("m");
    stop = CreateNewNumericFormatSpec(PresentationType::Stop1000, ShowSignOption::OnlyNegative, traitsU, 2);
    stop.SetMinWidth(4);
    AddFormat("StationM4", stop, *cvs, "stationM4");

    // Adds trailZeroes to "Stop100-2u"
    AddFormat("{\"NumericFormat\":{\"decPrec\":2, \"formatTraits\" : {\"AppendUnitName\":\"true\",\"TrailZeroes\":\"true\"}, \"presentType\" : \"Stop100\"}, \"SpecAlias\" : \"stop100-2uz\", \"SpecName\" : \"Stop100-2uz\", \"SpecType\" : \"numeric\"}",
        *unitContext);
    
    // Adds trailZeroes to "Stop100-2"
    AddFormat("{\"NumericFormat\":{\"decPrec\":2, \"formatTraits\" : {\"TrailZeroes\":\"true\"}, \"minWidth\" : 2, \"presentType\" : \"Stop100\"}, \"SpecAlias\" : \"stop100-2-2z\", \"SpecName\" : \"Stop100-2-2z\", \"SpecType\" : \"numeric\"}",
        *unitContext);

    // "Stop1000-2-3z"
    AddFormat("{\"NumericFormat\":{\"decPrec\":2, \"formatTraits\" : {\"TrailZeroes\":\"true\"}, \"minWidth\" : 3, \"presentType\" : \"Stop1000\"}, \"SpecAlias\" : \"stop1000-2-3z\", \"SpecName\" : \"Stop1000-2-3z\", \"SpecType\" : \"numeric\"}",
        *unitContext);

    // "hms"
    AddFormat("{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"hour(s)\", \"unitName\" : \"HR\"}, \"MiddleUnit\" : {\"unitLabel\":\"min\", \"unitName\" : \"MIN\"}, \"MinorUnit\" : {\"unitLabel\":\"sec\", \"unitName\" : \"S\"}, \"includeZero\" : true}, \"NumericFormat\" : {\"decPrec\":2, \"presentType\" : \"Decimal\"}, \"SpecAlias\" : \"hms\", \"SpecName\" : \"HMS\", \"SpecType\" : \"composite\"}",
        *unitContext);
    
    // Duplicate of "dms", just different name.
    AddFormat("{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"\xC2\xB0\", \"unitName\" : \"ARC_DEG\"}, \"MiddleUnit\" : {\"unitLabel\":\"'\", \"unitName\" : \"ARC_MINUTE\"}, \"MinorUnit\" : {\"unitLabel\":\"\\\"\", \"unitName\" : \"ARC_SECOND\"}, \"includeZero\" : true}, \"NumericFormat\" : {\"fractPrec\":1, \"presentType\" : \"Fractional\"}, \"SpecAlias\" : \"cdms\", \"SpecName\" : \"CAngleDMS\", \"SpecType\" : \"composite\"}",
        *unitContext);

    // Duplicate of "dms8", just different name.
    AddFormat("{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"\xC2\xB0\", \"unitName\" : \"ARC_DEG\"}, \"MiddleUnit\" : {\"unitLabel\":\"'\", \"unitName\" : \"ARC_MINUTE\"}, \"MinorUnit\" : {\"unitLabel\":\"\\\"\", \"unitName\" : \"ARC_SECOND\"}, \"includeZero\" : true}, \"NumericFormat\" : {\"fractPrec\":8, \"presentType\" : \"Fractional\"}, \"SpecAlias\" : \"cdms8\", \"SpecName\" : \"CAngleDMS8\", \"SpecType\" : \"composite\"}",
        *unitContext);

    // Duplicate of "dm8", just different name.
    AddFormat("{\"CompositeFormat\":{\"MajorUnit\":{\"unitLabel\":\"\xC2\xB0\", \"unitName\" : \"ARC_DEG\"}, \"MiddleUnit\" : {\"unitLabel\":\"'\", \"unitName\" : \"ARC_MINUTE\"}, \"includeZero\" : true}, \"NumericFormat\" : {\"fractPrec\":8, \"presentType\" : \"Fractional\"}, \"SpecAlias\" : \"cdm8\", \"SpecName\" : \"CAngleDM8\", \"SpecType\" : \"composite\"}",
        *unitContext);
    }

END_BENTLEY_FORMATTING_NAMESPACE
