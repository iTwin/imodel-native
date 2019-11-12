/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
        SetNumericSpec(other.m_numericSpec);
    if (other.HasComposite())
        SetCompositeSpec(other.m_compositeSpec);
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
Format::Format(NumericFormatSpecCR numSpec, CompositeValueSpecCR compSpec)
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
// static
bool Format::FromJson(FormatR out, Utf8CP jsonString, BEU::IUnitsContextCP context)
    {
    Json::Value jval (Json::objectValue);
    if (!Json::Reader::Parse(jsonString, jval))
        return false;
    return FromJson(out, jval, context);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
// static
bool Format::FromJson(FormatR out, Json::Value jval, BEU::IUnitsContextCP context)
    {
    Format f = Format();
    f.m_problem = FormatProblemCode::NoProblems;
    if (jval.empty())
        {
        f.m_problem.UpdateProblemCode(FormatProblemCode::NFS_InvalidJsonObject);
        return false;
        }

    if (!NumericFormatSpec::FromJson(f.m_numericSpec, jval))
        return false;
    Utf8CP paramName;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (BeStringUtilities::StricmpAscii(paramName, json_composite()) == 0)
            {
            CompositeValueSpec spec;
            if (!CompositeValueSpec::FromJson(spec, val, context))
                return false;
            f.SetCompositeSpec(spec);
            }
        }
    out = f;
    return true;
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
Utf8String Format::FormatQuantity(BEU::QuantityCR qty, BEU::UnitCP useUnit, Utf8CP space, Utf8CP useLabel) const
    {
    // there are two major options here: the format is a pure Numeric or it has a composite specification
    NumericFormatSpecCP fmtP = GetNumericSpec();
    BEU::Quantity temp = qty.ConvertTo(useUnit);
    Utf8CP uomLabel = Utf8String::IsNullOrEmpty(useLabel) ? ((nullptr == useUnit) ? qty.GetUnitLabel() : useUnit->GetDisplayLabel().c_str()) : useLabel;
    Utf8String majT, midT, minT, subT;

    if (HasComposite())  // procesing composite parts
        {
        CompositeValueSpecCP compS = GetCompositeSpec();
        auto dval = compS->DecomposeValue(temp.GetMagnitude(), temp.GetUnit());
        Utf8String pref = dval.GetSignPrefix();
        Utf8String suff = dval.GetSignSuffix();
        Utf8String uomSeparator;
        if (compS->HasSpacer())
            uomSeparator = compS->GetSpacer();

        // if the composite was auto created just to provide an override label for the numeric format then use the specified UomSeparator.
        if (!HasExplicitlyDefinedComposite())
            uomSeparator = fmtP->GetUomSeparator();

        // if caller explicity defines space parameter when calling this method use it, else use what is defined in format specification
        Utf8CP spacer = Utf8String::IsNullOrEmpty(space) ? uomSeparator.c_str() : space;

        // for all parts but the last one we need to format an integer 
        NumericFormatSpec fmtI;
        fmtI.SetPrecision(DecimalPrecision::Precision0);
        fmtI.SetKeepSingleZero(false);

        switch (compS->GetUnitCount())
            {
            case 1: // there is only one value to report
                majT = pref + fmtP->Format(dval.GetMajor());
                // if this composite only defines a single component then use format traits to determine if unit label is shown. This allows
                // support for SuppressUnitLable options in DgnClientFx. Also in this single component situation use the define UomSeparator.
                if (fmtP->IsShowUnitLabel())
                    majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel().c_str(), compS->HasSpacer()? spacer : fmtP->GetUomSeparator()) + suff;
                break;

            case 2:
                majT = pref + fmtI.Format(dval.GetMajor());
                if (fmtP->IsShowUnitLabel())
                    majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel().c_str(), spacer);
                midT = fmtP->Format(dval.GetMiddle());
                if (fmtP->IsShowUnitLabel())
                    midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel().c_str(), spacer);
                majT += " " + midT + suff;
                break;

            case 3:
                majT = pref + fmtI.Format(dval.GetMajor());
                if (fmtP->IsShowUnitLabel())
                    majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel().c_str(), spacer);
                midT = fmtI.Format(dval.GetMiddle());
                if (fmtP->IsShowUnitLabel())
                    midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel().c_str(), spacer);
                minT = fmtP->Format(dval.GetMinor());
                if (fmtP->IsShowUnitLabel())
                    minT = Utils::AppendUnitName(minT.c_str(), compS->GetMinorLabel().c_str(), spacer);
                majT += " " + midT + " " + minT + suff;
                break;

            case 4:
                majT = pref + fmtI.Format(dval.GetMajor());
                if (fmtP->IsShowUnitLabel())
                    majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel().c_str(), spacer);
                midT = fmtI.Format(dval.GetMiddle());
                if (fmtP->IsShowUnitLabel())
                    midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel().c_str(), spacer);
                minT = fmtI.Format(dval.GetMinor());
                if (fmtP->IsShowUnitLabel())
                    minT = Utils::AppendUnitName(minT.c_str(), compS->GetMinorLabel().c_str(), spacer);
                subT = fmtP->Format(dval.GetSub());
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
        majT = fmtP->Format(temp.GetMagnitude());
        if(fmtP->IsShowUnitLabel())
           majT = Utils::AppendUnitName(majT.c_str(), uomLabel, (nullptr == space) ? fmtP->GetUomSeparator() : space);
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
    Nullable<int32_t> parsedPrecision;
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
        int32_t precision = parsedPrecision.Value();
        switch (nfs.m_numericSpec.GetPresentationType())
            {
            case PresentationType::Decimal:        /* intentional fallthrough */
            case PresentationType::Scientific:     /* intentional fallthrough */
            case PresentationType::Station: /* intentional fallthrough */
                DecimalPrecision prec;
                Utils::GetDecimalPrecisionByInt(prec, precision);
                nfs.m_numericSpec.SetPrecision(prec);
                break;
            case PresentationType::Fractional:
                FractionalPrecision frac;
                Utils::FractionalPrecisionByDenominator(frac, precision);
                nfs.m_numericSpec.SetPrecision(frac);
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
            {
            CompositeValueSpec comp;
            CompositeValueSpec::CreateCompositeSpec(comp, units);
            nfs.SetCompositeSpec(comp);
            }
        compSpec = nfs.GetCompositeSpecP();
        if (nullptr == compSpec || compSpec->IsProblem())
            {
            LOG.errorv("Invalid format string, %s. Failed to create composite spec ", formatString.c_str());
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
BentleyStatus Format::ParseFormatString(Utf8StringR formatName, Nullable<int32_t>& precision, bvector<Utf8String>& unitNames, bvector<Nullable<Utf8String>>& labels, Utf8StringCR formatString)
    {
    static size_t const precisionOverrideIndx = 0;
    static std::regex const rgx(R"REGEX(([\w,:]+)(\(([^\)]+)\))?(\[([^\|\]]+)([\|])?([^\]]+)?\])?(\[([^\|\]]+)([\|])?([^\]]+)?\])?(\[([^\|\]]+)([\|])?([^\]]+)?\])?(\[([^\|\]]+)([\|])?([^\]]+)?\])?)REGEX", std::regex::optimize);
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
                    LOG.errorv("Invalid FormatString: Failed to parse integer for precision override of FormatString, %s", formatString.c_str());
                    return status;
                    }
                precision = static_cast<int32_t>(localPrecision);
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
    FormattingScannerCursor curs = FormattingScannerCursor(description, -1, FormatConstant::Dividers());
    Utf8String fnam;
    Utf8String unit;
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
            fdt = FormatDividerInstance(unit.c_str(), "/*");
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
        else if (fdi.BracketsMatched())// There are parentheses but they are not at the end. ex. (N*M)/DEG
            unit = curs.ExtractSegment(0, curs.GetTotalLength());
        // dividers are not found - we assume a Unit name only
        }
    formatString = fnam;
    unitName = unit;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
bool Format::_ToJson(Json::Value& out, bool verbose) const
    {
    if (!m_numericSpec.ToJson(out, verbose))
        return false;

    if (HasComposite())
        {
        auto& compNode = out[json_composite()] = Json::objectValue;
        if (!m_compositeSpec.ToJson(compNode, verbose))
            return false;
        }

    return true;
    }

END_BENTLEY_FORMATTING_NAMESPACE
