/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/Format.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>
#include "../../PrivateAPI/Formatting/FormattingParsing.h"
#include <regex>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
Format::Format(FormatCR other)
    : m_specType(other.m_specType), m_explicitlyDefinedComposite(false), m_problem(other.m_problem)
    {
    if (other.HasNumeric())
        m_numericSpec = other.m_numericSpec;
    if (other.HasComposite())
        { 
        m_explicitlyDefinedComposite = true;
        m_compositeSpec = other.m_compositeSpec;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
Format::Format(NumericFormatSpecCR numSpec)
    : m_specType(FormatSpecType::None), m_explicitlyDefinedComposite(false), m_numericSpec(numSpec), m_problem(FormatProblemCode::NoProblems)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Kyle.Abramowitz                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
bool Format::SetCompositeSpec(CompositeValueSpec spec) 
    {
    if (spec.IsProblem())
        {
        LOG.warningv("Failed to set composite spec because it has problem '%s'", spec.GetProblemDescription().c_str());
        return false;
        }
    m_explicitlyDefinedComposite = true;
    m_compositeSpec = spec;
    m_specType = static_cast<FormatSpecType>(m_compositeSpec.GetUnitCount());
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Kyle.Abramowitz                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
bool Format::SetNumericSpec(NumericFormatSpec spec) 
    {
    m_problem = FormatProblemCode::NoProblems;
    m_numericSpec = spec;
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
Format::Format( NumericFormatSpecCR numSpec, CompositeValueSpecCR compSpec)
    : Format(numSpec)
    {
    m_compositeSpec = compSpec;
    if (m_compositeSpec.IsProblem())
        {
        m_problem.UpdateProblemCode(FormatProblemCode::NotInitialized);
        }
    else
        {
        switch (m_compositeSpec.GetUnitCount())
            {
            case 1:
                m_specType = FormatSpecType::Single;
                break;
            case 2:
                m_specType = FormatSpecType::Double;
                break;
            case 3:
                m_specType = FormatSpecType::Triple;
                break;
            case 4:
                m_specType = FormatSpecType::Quad;
                break;
            default:
                m_problem.UpdateProblemCode(FormatProblemCode::NotInitialized);
            }
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz
//----------------------------------------------------------------------------------------
void Format::FromJson(Utf8CP jsonString, BEU::IUnitsContextCP context)
    {
    Json::Value jval (Json::objectValue);
    Json::Reader::Parse(jsonString, jval);
    FromJson(jval, context);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
void Format::FromJson(Json::Value jval, BEU::IUnitsContextCP context)
    {
    *this = Format();
    m_problem = FormatProblemCode::NoProblems;
    if (jval.empty())
        {
        m_problem.UpdateProblemCode(FormatProblemCode::NFS_InvalidJsonObject);
        return;
        }

    Utf8CP paramName;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (BeStringUtilities::StricmpAscii(paramName, json_NumericFormat()) == 0)
            m_numericSpec = NumericFormatSpec(val);
        else if (BeStringUtilities::StricmpAscii(paramName, json_CompositeFormat()) == 0)
            m_compositeSpec.FromJson(val, context);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
bool Format::IsIdentical(FormatCR other) const
    {
    if (m_specType != other.m_specType)
        return false;
    if (HasNumeric() && !m_numericSpec.IsIdentical(other.m_numericSpec))
        return false;
    if (HasComposite() && !m_compositeSpec.IsIdentical(other.m_compositeSpec))
        return false;
    if (m_problem.GetProblemCode() != other.m_problem.GetProblemCode())
        return false;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String Format::FormatQuantity(BEU::QuantityCR qty, BEU::UnitCP useUnit, Utf8CP space, double round)
    {
    if (!qty.IsNullQuantity())
        return Utf8String();
    BEU::UnitCP unitQ = qty.GetUnit();
    BEU::Quantity temp = qty.ConvertTo(unitQ);
    Utf8String str = m_numericSpec.Format(temp.GetMagnitude(), round);
    if(nullptr == useUnit || !m_numericSpec.IsShowUnitLabel())
        return str;
    Utf8String txt = Utils::AppendUnitName(str.c_str(), useUnit->GetLabel().c_str(), space);
    return txt;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String Format::StdFormatQuantity(FormatCR nfs, BEU::QuantityCR qty, BEU::UnitCP useUnit, Utf8CP space, Utf8CP useLabel, double round)
    {
    // there are two major options here: the format is a pure Numeric or it has a composite specification
    NumericFormatSpecCP fmtP = nfs.GetNumericSpec();
    BEU::Quantity temp = qty.ConvertTo(useUnit);
    Utf8CP uomLabel = Utf8String::IsNullOrEmpty(useLabel) ? ((nullptr == useUnit) ? qty.GetUnitLabel() : useUnit->GetLabel().c_str()) : useLabel;
    Utf8String majT, midT, minT, subT;

    if (nfs.HasComposite())  // procesing composite parts
        {
        CompositeValueSpecCP compS = nfs.GetCompositeSpec();
        CompositeValue dval = compS->DecomposeValue(temp.GetMagnitude(), temp.GetUnit());
        Utf8String pref = dval.GetSignPrefix();
        Utf8String suff = dval.GetSignSuffix();
        Utf8CP spacer = Utf8String::IsNullOrEmpty(space) ? compS->GetSpacer().c_str() : space;
        // for all parts but the last one we need to format an integer 
        NumericFormatSpec fmtI;
        fmtI.SetDecimalPrecision(DecimalPrecision::Precision0);
        fmtI.SetKeepSingleZero(false);

        switch (compS->GetUnitCount())
            {
            case 1: // there is only one value to report
                majT = pref + fmtP->Format(dval.GetMajor(), round);
                // if this composite only defines a single component then use format traits to determine if unit label is shown. This allows
                // support for SuppressUnitLable options in DgnClientFx. Also in this single component situation use the define UomSeparator.
                if (fmtP->IsShowUnitLabel())
                    majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel().c_str(), Utils::SubstituteNull(space, fmtP->GetUomSeparator())) + suff;
                break;

            case 2:
                majT = pref + fmtI.Format(dval.GetMajor(), round);
                if (fmtP->IsShowUnitLabel())
                    majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel().c_str(), spacer);
                midT = fmtP->Format(dval.GetMiddle(), round);
                if (fmtP->IsShowUnitLabel())
                    midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel().c_str(), spacer);
                majT += " " + midT + suff;
                break;

            case 3:
                majT = pref + fmtI.Format(dval.GetMajor(), round);
                if (fmtP->IsShowUnitLabel())
                    majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel().c_str(), spacer);
                midT = fmtI.Format(dval.GetMiddle(), round);
                if (fmtP->IsShowUnitLabel())
                    midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel().c_str(), spacer);
                minT = fmtP->Format(dval.GetMinor(), round);
                if (fmtP->IsShowUnitLabel())
                    minT = Utils::AppendUnitName(minT.c_str(), compS->GetMinorLabel().c_str(), spacer);
                majT += " " + midT + " " + minT + suff;
                break;

            case 4:
                majT = pref + fmtI.Format(dval.GetMajor(), round);
                if (fmtP->IsShowUnitLabel())
                    majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel().c_str(), spacer);
                midT = fmtI.Format(dval.GetMiddle(), round);
                if (fmtP->IsShowUnitLabel())
                    midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel().c_str(), spacer);
                minT = fmtI.Format(dval.GetMinor(), round);
                if (fmtP->IsShowUnitLabel())
                    minT = Utils::AppendUnitName(minT.c_str(), compS->GetMinorLabel().c_str(), spacer);
                subT = fmtP->Format(dval.GetSub(), round);
                if (fmtP->IsShowUnitLabel())
                    subT = Utils::AppendUnitName(subT.c_str(), compS->GetSubLabel().c_str(), spacer);
                majT += midT + " " + minT + " " + subT + suff;
                break;
            }
        }
    else
        {
        if (nullptr == fmtP)  // invalid name
            fmtP = &NumericFormatSpec::DefaultFormat();
        if (nullptr == fmtP)
            return "";
        majT = fmtP->Format(temp.GetMagnitude(), round);
        if(fmtP->IsShowUnitLabel())
           majT = Utils::AppendUnitName(majT.c_str(), uomLabel, Utils::SubstituteNull(space, fmtP->GetUomSeparator()));
        }
    return majT;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
// static
BentleyStatus Format::ParseFormatString(FormatR nfs, Utf8StringCR formatString, std::function<FormatCP(Utf8StringCR)> defaultFormatMapper, BEU::IUnitsContextCP unitContext)
    {
    Utf8String formatName;
    Nullable<unsigned> parsedPrecision;
    bvector<Utf8String> unitNames;
    bvector<Nullable<Utf8String>> unitLabels;
    if (BentleyStatus::SUCCESS != ParseFormatString(formatName, parsedPrecision, unitNames, unitLabels, formatString))
        return BentleyStatus::ERROR;

    Utf8String const namedFormat(formatName);
    FormatCP defaultFormat = defaultFormatMapper(namedFormat);
    if (nullptr == defaultFormat)
        {
        LOG.errorv("failed to map a format name to a Format");
        return BentleyStatus::ERROR;
        }

    nfs = *defaultFormat;
    if (parsedPrecision.IsValid())
        {
        uint64_t precision = parsedPrecision.Value();
        switch (nfs.m_numericSpec.GetPresentationType())
            {
            case PresentationType::Decimal:        /* intentional fallthrough */
            case PresentationType::Scientific:     /* intentional fallthrough */
            case PresentationType::Station: /* intentional fallthrough */
                DecimalPrecision prec;
                Utils::DecimalPrecisionByIndex(prec, precision);
                nfs.m_numericSpec.SetDecimalPrecision(prec);
                break;
            case PresentationType::Fractional:
                FractionalPrecision frac;
                Utils::FractionalPrecisionByDenominator(frac, precision);
                nfs.m_numericSpec.SetFractionalPrecision(frac);
                break;
            default:
                LOG.errorv("unknown presentation type");
                return BentleyStatus::ERROR;
            }
        }

    // Handle Unit Override
    if (!unitNames.empty())
        {
        BeAssert(4 >= unitNames.size());
        bvector<BEU::UnitCP> units;
        for (const auto& u : unitNames)
            {
            BEU::UnitCP inputUnit = unitContext->LookupUnit(u.c_str());
            if (nullptr == inputUnit)
                {
                LOG.errorv("Failed to resolve the input Unit %s from format string, %s.", u.c_str(), formatString.c_str());
                return BentleyStatus::ERROR;
                }
            units.push_back(inputUnit);
            }
        auto compSpec = nfs.GetCompositeSpecP();
        if (nullptr == compSpec)
            nfs.SetCompositeSpec(CompositeValueSpec(units));
        if (nfs.GetCompositeSpec()->IsProblem())
            {
            LOG.errorv("Invalid format string, %s. %s ", formatString.c_str(), compSpec->GetProblemDescription());
            return BentleyStatus::ERROR;
            }
        }

    if (!unitLabels.empty())
        {
        auto comp = nfs.GetCompositeSpecP();
        BeAssert(nullptr != comp);
        switch (unitLabels.size())
            {
            case 4:
                if (unitLabels[3].IsValid())
                    comp->SetSubLabel(unitLabels[3].Value());
            case 3:
                if (unitLabels[2].IsValid())
                    comp->SetMinorLabel(unitLabels[2].Value());
            case 2:
                if (unitLabels[1].IsValid())
                    comp->SetMiddleLabel(unitLabels[1].Value());
            case 1:
                if (unitLabels[0].IsValid())
                    comp->SetMajorLabel(unitLabels[0].Value());
            default:
                break;
            }
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Kyle.Abramowitz                  04/18
//---------------+---------------+---------------+---------------+---------------+-------
// static
BentleyStatus Format::ParseFormatString(Utf8StringR formatName, Nullable<unsigned>& precision, bvector<Utf8String>& unitNames, bvector<Nullable<Utf8String>>& labels, Utf8StringCR formatString)
    {
    static size_t const precisionOverrideIndx = 0;
    static std::regex const rgx(R"REGEX(([\w,:]+)(\(([^\)]+)\))?(\[([^\|\]]+)([\|])?([^\|\]]+)?\])?(\[([^\|\]]+)([\|])?([^\|\]]+)?\])?(\[([^\|\]]+)([\|])?([^\|\]]+)?\])?(\[([^\|\]]+)([\|])?([^\|\]]+)?\])?)REGEX", std::regex::optimize);
    std::cmatch match;

    
    if (!std::regex_match(formatString.c_str(), match, rgx))
        return BentleyStatus::ERROR;

    size_t numOfRegexes = match.size();
    if (0 == numOfRegexes)
        return BentleyStatus::ERROR;

    // Handle format first to fail fast.
    if (!match[1].matched)
        {
        LOG.errorv("failed to map a format name to a Format");
        return BentleyStatus::ERROR;
        }
    // Get format name. Should always be the first match
    formatName = match[1].str().c_str();

    
    if (match[2].matched && match[3].matched)
        {
        Utf8String const overrideStr(match[2].str().c_str());
        // BeStringUtilities::Split ignores empty tokens. Since overrides are
        // position dependent, we actually need to count tokens even if they are
        // the empty string. This function does just that using ',' as a separator.
        bvector<Utf8String> overrides = [](Utf8StringCR str) -> bvector<Utf8String>
            {
            bvector<Utf8String> tokens;
            size_t prevPos = 1; // Initial position is the character directly after the opening '(' in the override string.
            size_t currPos;
            while (str.npos != (currPos = str.find_first_of(",)", prevPos)))
                {
                tokens.push_back(Utf8String(str.substr(prevPos, currPos - prevPos).c_str()).Trim());
                prevPos = currPos + 1;
                }
            return tokens;
            }(overrideStr);

        // It is considered an error to pass in a format string with empty
        // override brackets. If no overrides are needed, the user should instead
        // leave the brackets off altogether. As an example the incorrect format
        // string "SomeFormat<>" should instead be written as "SomeFormat".
        // Additionally, if a format would be specified using an override string
        // With no items actually overridden such as "SomeFormat<,,,,>" the string
        // is also erroneous.
        if (!overrideStr.empty()
            && overrides.end() == std::find_if_not(overrides.begin(), overrides.end(),
                [](Utf8StringCR ovrstr) -> bool
            {
            return std::all_of(ovrstr.begin(), ovrstr.end(), ::isspace);
            }))
            {
            LOG.errorv("override list must contain at least one override");
            return BentleyStatus::ERROR;
            }

        // The first override parameter overrides the default precision for the format.
        if (overrides.size() >= precisionOverrideIndx + 1) // Bail if the user didn't include this override.
            {
            if (!overrides[precisionOverrideIndx].empty())
                {
                uint64_t localPrecision;
                BentleyStatus status = BeStringUtilities::ParseUInt64(localPrecision, overrides[precisionOverrideIndx].c_str());
                if (BentleyStatus::SUCCESS != status)
                    {
                    LOG.errorv("failed to parse integer for precision override");
                    return status;
                    }
                precision = static_cast<unsigned>(localPrecision);
                }
            }
        }

    int i = 4;
    while (i < match.size())
        {
        if (!match[i].matched)
            break;
        // Unit override: required;
        if (!match[i+1].matched)
            return ERROR;
        unitNames.push_back(match[i+1].str().c_str());
        // Label override; optional
        if (match[i+2].matched) // matches a bar
            {
            labels.push_back(Nullable<Utf8String>(match[i+3].str().c_str()));
            }
        else // no label. ok
            labels.push_back(nullptr);
        i+=4;
        }

    return BentleyStatus::SUCCESS;
    }

//----------------------------------------------------------------------------------------
//  The text string has format <unitName>(<formatName>)
// @bsimethod                                                   Kyle.Abramowitz    02/2018
//----------------------------------------------------------------------------------------
void Format::ParseUnitFormatDescriptor(Utf8StringR unitName, Utf8StringR formatString, Utf8CP description)
    {
    FormattingScannerCursor curs = FormattingScannerCursor(description, -1, FormatConstant::FUSDividers());
    FormattingWord fnam;
    FormattingWord unit;
    FormatDividerInstance fdt;
    FormatDividerInstance fdi = FormatDividerInstance(description, '|'); // check if this is a new format
    int n = fdi.GetDivCount();
    if (n == 2 && fdi.IsDivLast())
        {
        fnam = curs.ExtractLastEnclosure();
        unit = curs.ExtractBeforeEnclosure();
        }
    else if (n == 1 && !fdi.IsDivLast())
        {
        int loc = fdi.GetFirstLocation();
        if (loc > 0)
            {
            unit = curs.ExtractSegment(0, (size_t)loc - 1);
            fnam = curs.ExtractSegment((size_t)loc + 1, curs.GetTotalLength());
            }
        }
    else
        {
        fdi = FormatDividerInstance(description, '(');
        n = fdi.GetDivCount();
        if (n == 0)
            unit = curs.ExtractSegment(0, curs.GetTotalLength());
        if (fdi.BracketsMatched() && fdi.IsDivLast()) // there is a candidate for the format in parethesis
            {
            unit = curs.ExtractLastEnclosure();
            fdt = FormatDividerInstance(unit.GetText(), "/*");
            if (fdt.GetDivCount() == 0) // it can be a format name
                {
                fnam = unit;
                unit = curs.ExtractBeforeEnclosure();
                }
            else
                {
                unit = curs.ExtractSegment(0, curs.GetTotalLength());
                }
            }
        // dividers are not found - we assume a Unit name only
        }
    formatString = fnam.GetText();
    unitName = unit.GetText();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Kyle.Abramowitz    04/2018
//----------------------------------------------------------------------------------------
FormatCR Format::DefaultFormat()
    {
    static Format format(NumericFormatSpec::DefaultFormat());
    return format;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Json::Value Format::ToJson(bool verbose) const
    {
    Json::Value jNFS;
    jNFS[json_NumericFormat()] = m_numericSpec.ToJson(verbose);
    Json::Value jcs = m_compositeSpec.ToJson();
    if (!jcs.empty())
        jNFS[json_CompositeFormat()] = jcs;
    return jNFS;
    }

END_BENTLEY_FORMATTING_NAMESPACE
