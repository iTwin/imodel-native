/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/NamedFormatSpec.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>
#include <regex>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
NamedFormatSpec::NamedFormatSpec() : m_specType(FormatSpecType::None), m_problem(FormatProblemCode::NotInitialized) 
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
NamedFormatSpec::NamedFormatSpec(NamedFormatSpecCR other)
    : m_name(other.m_name), m_description(other.m_description),
    m_displayLabel(other.m_displayLabel), m_specType(other.m_specType), m_problem(other.m_problem)
    {
    if (other.HasNumeric())
        m_numericSpec = other.m_numericSpec;
    if (other.HasComposite())
        m_compositeSpec = other.m_compositeSpec;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
NamedFormatSpec::NamedFormatSpec(Utf8StringCR name, NumericFormatSpecCR numSpec)
    : m_name(name), m_specType(FormatSpecType::None), m_numericSpec(numSpec), m_problem(FormatProblemCode::NoProblems)
    {
    if (name.empty())
        m_problem.UpdateProblemCode(FormatProblemCode::NFS_InvalidSpecName);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
NamedFormatSpec::NamedFormatSpec(Utf8StringCR name, NumericFormatSpecCR numSpec, CompositeValueSpecCR compSpec)
    : NamedFormatSpec(name, numSpec)
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
void NamedFormatSpec::FromJson(Utf8CP jsonString, BEU::IUnitsContextCP context)
    {
    Json::Value jval (Json::objectValue);
    Json::Reader::Parse(jsonString, jval);
    FromJson(jval, context);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
void NamedFormatSpec::FromJson(Json::Value jval, BEU::IUnitsContextCP context)
    {
    *this = NamedFormatSpec();
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
        if (BeStringUtilities::StricmpAscii(paramName, json_SpecName()) == 0)
            m_name = val.asString();
        else if (BeStringUtilities::StricmpAscii(paramName, json_SpecDescript()) == 0)
            m_description = val.asString();
        else if (BeStringUtilities::StricmpAscii(paramName, json_SpecLabel()) == 0)
            m_displayLabel = val.asString();
        else if (BeStringUtilities::StricmpAscii(paramName, json_NumericFormat()) == 0)
            m_numericSpec = NumericFormatSpec(val);
        else if (BeStringUtilities::StricmpAscii(paramName, json_CompositeFormat()) == 0)
            m_compositeSpec.LoadJsonData(val, context);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
bool NamedFormatSpec::IsIdentical(NamedFormatSpecCR other) const
    {
    if (m_name != other.m_name)
        return false;
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
Utf8String NamedFormatSpec::FormatQuantity(BEU::QuantityCR qty, BEU::UnitCP useUnit, Utf8CP space, int prec, double round)
    {
    if (!qty.IsNullQuantity())
        return Utf8String();
    BEU::UnitCP unitQ = qty.GetUnit();
    BEU::Quantity temp = qty.ConvertTo(unitQ);
    Utf8String str = m_numericSpec.FormatDouble(temp.GetMagnitude(), prec, round);
    if(nullptr == useUnit || !m_numericSpec.IsShowUnitLabel())
        return str;
    Utf8String txt = Utils::AppendUnitName(str.c_str(), useUnit->GetLabel().c_str(), space);
    return txt;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String NamedFormatSpec::StdFormatQuantity(NamedFormatSpecCR nfs, BEU::QuantityCR qty, BEU::UnitCP useUnit, Utf8CP space, Utf8CP useLabel, int prec, double round)
    {
    // there are two major options here: the format is a pure Numeric or it has a composite specification
    NumericFormatSpecCP fmtP = nfs.GetNumericSpec();
    bool composite = nfs.HasComposite();
    BEU::Quantity temp = qty.ConvertTo(useUnit);
    Utf8CP uomLabel = Utf8String::IsNullOrEmpty(useLabel) ? ((nullptr == useUnit) ? qty.GetUnitLabel() : useUnit->GetLabel().c_str()) : useLabel;
    Utf8String majT, midT, minT, subT;

    if (composite)  // procesing composite parts
        {
        CompositeValueSpecP compS = (CompositeValueSpecP)nfs.GetCompositeSpec();
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
                majT = pref + fmtP->FormatDouble(dval.GetMajor(), prec, round);
                // if this composite only defines a single component then use format traits to determine if unit label is shown. This allows
                // support for SuppressUnitLable options in DgnClientFx. Also in this single component situation use the define UomSeparator.
                if (fmtP->IsShowUnitLabel())
                    majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel().c_str(), Utils::SubstituteNull(space, fmtP->GetUomSeparator())) + suff;
                break;

            case 2:
                majT = pref + fmtI.FormatDouble(dval.GetMajor(), prec, round);
                majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel().c_str(), spacer);
                midT = fmtP->FormatDouble(dval.GetMiddle(), prec, round);
                midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel().c_str(), spacer);
                majT += " " + midT + suff;
                break;

            case 3:
                majT = pref + fmtI.FormatDouble(dval.GetMajor(), prec, round);
                majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel().c_str(), spacer);
                midT = fmtI.FormatDouble(dval.GetMiddle(), prec, round);
                midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel().c_str(), spacer);
                minT = fmtP->FormatDouble(dval.GetMinor(), prec, round);
                minT = Utils::AppendUnitName(minT.c_str(), compS->GetMinorLabel().c_str(), spacer);
                majT += " " + midT + " " + minT + suff;
                break;

            case 4:
                majT = pref + fmtI.FormatDouble(dval.GetMajor(), prec, round);
                majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel().c_str(), spacer);
                midT = fmtI.FormatDouble(dval.GetMiddle(), prec, round);
                midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel().c_str(), spacer);
                minT = fmtI.FormatDouble(dval.GetMinor(), prec, round);
                minT = Utils::AppendUnitName(minT.c_str(), compS->GetMinorLabel().c_str(), spacer);
                subT = fmtP->FormatDouble(dval.GetSub(), prec, round);
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
        majT = fmtP->FormatDouble(temp.GetMagnitude(), prec, round);
        if(fmtP->IsShowUnitLabel())
           majT = Utils::AppendUnitName(majT.c_str(), uomLabel, Utils::SubstituteNull(space, fmtP->GetUomSeparator()));
        }
    return majT;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus NamedFormatSpec::ParseFormatString(NamedFormatSpecR nfs, Utf8StringCR formatString, std::function<NamedFormatSpecCP(Utf8StringCR)> defaultNamedFormatSpecMapper)
    {
    static size_t const precisionOverrideIndx = 0;
    static std::regex const rgx(R"REGEX((\w+)(<([^,<>]*,?)+>)?)REGEX", std::regex::optimize);
    std::cmatch match;

    if (!std::regex_match(formatString.c_str(), match, rgx))
        return BentleyStatus::ERROR;

    Utf8String const namedFormat(match[1].str().c_str());
    Utf8String const overrideStr(match[2].str().c_str());
    // BeStringUtilities::Split ignores empty tokens. Since overrides are
    // position dependent, we actually need to count tokens even if they are
    // the empty string. This function does just that using ',' as a separator.
    bvector<Utf8String> overrides = [](Utf8StringCR str) -> bvector<Utf8String>
        {
        bvector<Utf8String> tokens;
        size_t prevPos = 1; // Initial position is the character directly after the opening '<' in the override string.
        size_t currPos;
        while (str.npos != (currPos = str.find_first_of(",>", prevPos)))
            {
            tokens.push_back(Utf8String(str.substr(prevPos, currPos-prevPos).c_str()).Trim());
            prevPos = currPos+1;
            }
        return tokens;
        }(overrideStr);

    NamedFormatSpecCP defaultNamedFormatSpec = defaultNamedFormatSpecMapper(namedFormat);
    if (nullptr == defaultNamedFormatSpec)
        {
        LOG.errorv("failed to map a format name to a NamedFormatSpec");
        return BentleyStatus::ERROR;
        }
    nfs = *defaultNamedFormatSpec;

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
    if (overrides.size() < precisionOverrideIndx+1) // Bail if the user didn't include this override.
        return BentleyStatus::SUCCESS;
    if (!overrides[precisionOverrideIndx].empty())
        {
        uint64_t precision;
        BentleyStatus status = BeStringUtilities::ParseUInt64(precision, overrides[precisionOverrideIndx].c_str());
        if (BentleyStatus::SUCCESS != status)
            {
            LOG.errorv("failed to parse integer for precision override");
            return status;
            }
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

    return BentleyStatus::SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Json::Value NamedFormatSpec::ToJson(bool verbose) const
    {
    Json::Value jNFS;
    jNFS[json_SpecName()] = m_name;
    if (!m_description.empty())
        jNFS[json_SpecDescript()] = m_description;
    if (!m_displayLabel.empty())
        jNFS[json_SpecLabel()] = m_displayLabel;

    jNFS[json_NumericFormat()] = m_numericSpec.ToJson(verbose);
    Json::Value jcs = m_compositeSpec.ToJson();
    if (!jcs.empty())
        jNFS[json_CompositeFormat()] = jcs;
    return jNFS;
    }

END_BENTLEY_FORMATTING_NAMESPACE
