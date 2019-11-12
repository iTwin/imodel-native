/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>

#include <Formatting/FormattingApi.h>
#include "../../PrivateAPI/Formatting/FormattingParsing.h"
#include "../../PrivateAPI/Formatting/NumericFormatUtils.h"

#include <locale>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/18
//----------------------------------------------------------------------------------------
bool NumericFormatSpec::ImbueLocale(Utf8CP name) // en-US en-UK   en-GB
    {
    std::locale loc = std::locale(name);
    const std::numpunct<char>& myfacet(std::use_facet < std::numpunct<char> >(loc));

    m_decimalSeparator = myfacet.decimal_point();
    m_thousandsSeparator = myfacet.thousands_sep(); 
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
NumericFormatSpec::NumericFormatSpec()
    : m_explicitlyDefinedDecimalSeparator(false)
    , m_explicitlyDefinedMinWidth(false)
    , m_explicitlyDefinedPrecision(false)
    , m_explicitlyDefinedRoundFactor(false)
    , m_explicitlyDefinedShowSign(false)
    , m_explicitlyDefinedStatSeparator(false)
    , m_explicitlyDefinedThousandsSeparator(false)
    , m_explicitlyDefinedUOMSeparator(false)
    , m_roundFactor(FormatConstant::DefaultRoundingFactor())
    , m_presentationType(FormatConstant::DefaultPresentaitonType())
    , m_signOption(FormatConstant::DefaultSignOption())
    , m_formatTraits(FormatConstant::DefaultFormatTraits())
    , m_decPrecision(FormatConstant::DefaultDecimalPrecision())
    , m_fractPrecision(FormatConstant::DefaultFractionalPrecision())
    , m_decimalSeparator(FormatConstant::FPV_DecimalSeparator())
    , m_thousandsSeparator(FormatConstant::FPV_ThousandSeparator())
    , m_uomSeparator(FormatConstant::BlankString())
    , m_statSeparator(FormatConstant::DefaultStationSeparator())
    , m_minWidth(FormatConstant::DefaultMinWidth())
    , m_stationSize(0)
    , m_scientificType(ScientificType::Normalized)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz
//----------------------------------------------------------------------------------------
bool NumericFormatSpec::FromJson(NumericFormatSpecR out, JsonValueCR jval)
    {
    if (jval.empty())
        return false;

    if (Json::objectValue != jval.type())
        return false;

    NumericFormatSpec spec;
    // Presentation Type needs to be read first since reading the precision depends on it.
    JsonValueCR presType = jval[json_type()];
    if (Json::nullValue != presType.type())
        Utils::ParsePresentationType(spec.m_presentationType, presType.asCString());

    Utf8CP paramName;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (BeStringUtilities::StricmpAscii(paramName, json_roundFactor()) == 0)
            {
            double rf = val.asDouble();
            spec.SetRoundingFactor(rf);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_precision()) == 0)
            {
            switch (spec.m_presentationType)
                {
                case PresentationType::Fractional:
                    FractionalPrecision fracPrec;
                    Utils::FractionalPrecisionByDenominator(fracPrec, (int32_t)val.asInt64());
                    spec.SetPrecision(fracPrec);
                    break;
                case PresentationType::Decimal:
                case PresentationType::Scientific:
                case PresentationType::Station:
                default:
                    DecimalPrecision decPrec;
                    Utils::GetDecimalPrecisionByInt(decPrec, (int32_t)val.asInt64());
                    spec.SetPrecision(decPrec);
                }
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_scientificType()) == 0)
            {
            ScientificType type;
            Utils::ParseScientificType(type, val.asCString());
            spec.SetScientificType(type);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_showSignOption()) == 0)
            {
            SignOption option;
            Utils::ParseSignOption(option, val.asCString());
            spec.SetSignOption(option);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_decimalSeparator()) == 0)
            {
            Utf8Char sep;
            sep = val.asString().c_str()[0];
            spec.SetDecimalSeparator(sep);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_thousandSeparator()) == 0)
            {
            Utf8Char sep;
            sep = val.asString().c_str()[0];
            spec.SetThousandSeparator(sep);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_uomSeparator()) == 0)
            {
            Utf8String sep;
            sep = val.asString().c_str();
            spec.SetUomSeparator(sep.c_str());
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_stationSeparator()) == 0)
            {
            Utf8Char sep;
            sep = val.asString().c_str()[0];
            spec.SetStationSeparator(sep);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_stationOffsetSize()) == 0)
            {
            uint32_t size;
            size = (uint32_t) val.asUInt();
            spec.SetStationOffsetSize(size);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_minWidth()) == 0)
            {
            uint32_t width;
            width = (uint32_t) val.asUInt();
            spec.SetMinWidth(width);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_formatTraits()) == 0)
            spec.SetFormatTraits(val); //Handles both string and array
        }
    out = spec;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//---------------------------------------------------------------------------------------
bool NumericFormatSpec::ToJson(Json::Value& out, bool verbose) const
    {
    out[json_type()] = Utils::GetPresentationTypeString(GetPresentationType());
    
    // Always show ScientificType if the type is Scientific.
    if (PresentationType::Scientific == GetPresentationType())
        out[json_scientificType()] = Utils::GetScientificTypeString(GetScientificType());

    if (PresentationType::Station == GetPresentationType())
        {
        // Always serialize offsetSize for station.
        out[json_stationOffsetSize()] = GetStationOffsetSize();
        if (verbose || HasStationSeparator())
            out[json_stationSeparator()] = Utf8String(1, GetStationSeparator());
        }

    // Common between all Types
    if (verbose || HasPrecision())
        {
        switch (m_presentationType)
            {
            case PresentationType::Fractional:
                out[json_precision()] = Utils::FractionalPrecisionDenominator(m_fractPrecision);
                break;
            case PresentationType::Decimal:
            case PresentationType::Scientific:
            case PresentationType::Station:
            default:
                out[json_precision()] = Utils::DecimalPrecisionToInt(m_decPrecision);
                break;
            }
        }

    if(verbose || HasSignOption())
        out[json_showSignOption()] = Utils::GetSignOptionString(m_signOption);
    if (verbose || HasRoundingFactor())
        out[json_roundFactor()] = GetRoundingFactor();
    if (verbose || HasFormatTraits())
        out[json_formatTraits()] = FormatTraitsToJson();
    if (verbose || HasDecimalSeparator())
        out[json_decimalSeparator()] = Utf8String(1, GetDecimalSeparator());
    if (verbose || HasThousandsSeparator())
        out[json_thousandSeparator()] = Utf8String(1, GetThousandSeparator());
    if (verbose || HasUomSeparator())
        out[json_uomSeparator()] = GetUomSeparator();
    if (verbose || HasMinWidth())
        out[json_minWidth()] = GetMinWidth();

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz
//----------------------------------------------------------------------------------------
bool NumericFormatSpec::IsIdentical(NumericFormatSpecCR other) const
    {
    if (fabs(m_roundFactor - other.m_roundFactor) > 0.01) return false;
    if (m_presentationType != other.m_presentationType) return false;
    if (m_signOption != other.m_signOption) return false;
    if (m_formatTraits != other.m_formatTraits) return false;
    if (m_decPrecision != other.m_decPrecision) return false;
    if (m_fractPrecision != other.m_fractPrecision) return false;
    if (m_decimalSeparator != other.m_decimalSeparator) return false;
    if (m_thousandsSeparator != other.m_thousandsSeparator) return false;
    if (!m_uomSeparator.Equals(other.m_uomSeparator)) return false;
    if (m_statSeparator != other.m_statSeparator) return false;
    if (m_stationSize != other.m_stationSize) return false;
    if (m_scientificType != other.m_scientificType) return false;
    if (m_minWidth != other.m_minWidth) return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
// static
NumericFormatSpecCR NumericFormatSpec::DefaultFormat()
    {
    static NumericFormatSpec nfs;
    nfs.SetKeepSingleZero(true);
    nfs.SetKeepDecimalPoint(true);
    nfs.SetShowUnitLabel(true);
    return nfs;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                    03/18
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String NumericFormatSpec::GetFormatTraitsString() const
    {
    Utf8String ret;
    bvector<Utf8String> strings;
    if (GetTraitBit(FormatTraits::TrailingZeroes))
        ret += FormatConstant::FPN_TrailZeroes() + "|";
    if (GetTraitBit(FormatTraits::KeepSingleZero))
        ret += FormatConstant::FPN_KeepSingleZero() + "|";
    if (GetTraitBit(FormatTraits::ZeroEmpty))
        ret += FormatConstant::FPN_ZeroEmpty() + "|";
    if (GetTraitBit(FormatTraits::KeepDecimalPoint))
        ret += FormatConstant::FPN_KeepDecimalPoint() + "|";
    if (GetTraitBit(FormatTraits::ApplyRounding))
        ret += FormatConstant::FPN_ApplyRounding() + "|";
    if (GetTraitBit(FormatTraits::FractionDash))
        ret += FormatConstant::FPN_FractionDash() + "|";
    if (GetTraitBit(FormatTraits::ShowUnitLabel))
        ret += FormatConstant::FPN_ShowUnitLabel() + "|";
    if (GetTraitBit(FormatTraits::PrependUnitLabel))
        ret += FormatConstant::FPN_PrependUnitLabel() + "|";
    if (GetTraitBit(FormatTraits::Use1000Separator))
        ret += FormatConstant::FPN_Use1000Separator() + "|";
    if (GetTraitBit(FormatTraits::ExponenentOnlyNegative))
        ret += FormatConstant::FPN_ExponentOnlyNegative() + "|";
    if ('|' == *(ret.end() - 1))
        ret = ret.substr(0, ret.size()-1);

    return ret;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    David Fox-Rabinovitz
//---------------+---------------+---------------+---------------+---------------+-------
// static
FormatTraits NumericFormatSpec::SetTraitsBit(FormatTraits traits, FormatTraits bit, bool setTo)
    {
    std::underlying_type<FormatTraits>::type traitsBitField = static_cast<std::underlying_type<FormatTraits>::type>(traits);
    if (setTo)
        traitsBitField |= static_cast<std::underlying_type<FormatTraits>::type>(bit);
    else
        traitsBitField &= ~static_cast<std::underlying_type<FormatTraits>::type>(bit);
    return static_cast<FormatTraits>(traitsBitField);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                    03/18
//---------------+---------------+---------------+---------------+---------------+-------
bool NumericFormatSpec::SetFormatTraits(Utf8CP input)
    {
    if (Utf8String::IsNullOrEmpty(input))
        return true;
    bvector<Utf8String> splitStrings;
    BeStringUtilities::Split(input, ",;|", splitStrings);
    this->m_formatTraits = FormatTraits::None;
    int numProcessed = 0;
    for (auto const& s : splitStrings)
        {
        if (BeStringUtilities::StricmpAscii(s.c_str(), FormatConstant::FPN_TrailZeroes().c_str()) == 0)
            this->SetTraitsBit(FormatTraits::TrailingZeroes, true);
        else if (BeStringUtilities::StricmpAscii(s.c_str(), FormatConstant::FPN_KeepSingleZero().c_str()) == 0)
            this->SetTraitsBit(FormatTraits::KeepSingleZero, true);
        else if (BeStringUtilities::StricmpAscii(s.c_str(), FormatConstant::FPN_ZeroEmpty().c_str()) == 0)
            this->SetTraitsBit(FormatTraits::ZeroEmpty, true);
        else if (BeStringUtilities::StricmpAscii(s.c_str(), FormatConstant::FPN_KeepDecimalPoint().c_str()) == 0)
            this->SetTraitsBit(FormatTraits::KeepDecimalPoint, true);
        else if (BeStringUtilities::StricmpAscii(s.c_str(), FormatConstant::FPN_ApplyRounding().c_str()) == 0)
            this->SetTraitsBit(FormatTraits::ApplyRounding, true);
        else if (BeStringUtilities::StricmpAscii(s.c_str(), FormatConstant::FPN_FractionDash().c_str()) == 0)
            this->SetTraitsBit(FormatTraits::FractionDash, true);
        else if (BeStringUtilities::StricmpAscii(s.c_str(), FormatConstant::FPN_ShowUnitLabel().c_str()) == 0)
            this->SetTraitsBit(FormatTraits::ShowUnitLabel, true);
        else if (BeStringUtilities::StricmpAscii(s.c_str(), FormatConstant::FPN_PrependUnitLabel().c_str()) == 0)
            this->SetTraitsBit(FormatTraits::PrependUnitLabel, true);
        else if (BeStringUtilities::StricmpAscii(s.c_str(), FormatConstant::FPN_Use1000Separator().c_str()) == 0)
            this->SetTraitsBit(FormatTraits::Use1000Separator, true);
        else if (BeStringUtilities::StricmpAscii(s.c_str(), FormatConstant::FPN_ExponentOnlyNegative().c_str()) == 0)
            this->SetTraitsBit(FormatTraits::ExponenentOnlyNegative, true);
        else 
            continue;
        ++numProcessed;
        }

    if (numProcessed != splitStrings.size())
        return false;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    David Fox-Rabinovitz
//---------------+---------------+---------------+---------------+---------------+-------
bool NumericFormatSpec::SetFormatTraits(JsonValueCR jval)
    {
    if (jval.empty())
        return true;

    if (jval.isString())
        return SetFormatTraits(jval.asCString());

    if (!jval.isArray())
        return false;

    Utf8CP paramName;
    for (Json::ValueIterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = (*iter).asCString();
        if (BeStringUtilities::StricmpAscii(paramName, json_trailZeroes()) == 0)
            SetKeepTrailingZeroes(true);
        else if (BeStringUtilities::StricmpAscii(paramName, json_keepSingleZero()) == 0)
            SetKeepSingleZero(true);
        else if (BeStringUtilities::StricmpAscii(paramName, json_zeroEmpty()) == 0)
            SetZeroEmpty(true);
        else if (BeStringUtilities::StricmpAscii(paramName, json_keepDecimalPoint()) == 0)
            SetKeepDecimalPoint(true);
        else if (BeStringUtilities::StricmpAscii(paramName, json_applyRounding()) == 0)
            SetApplyRounding(true);
        else if (BeStringUtilities::StricmpAscii(paramName, json_fractionDash()) == 0)
            SetFractionDash(true);
        else if (BeStringUtilities::StricmpAscii(paramName, json_showUnitLabel()) == 0)
            SetShowUnitLabel(true);
        else if (BeStringUtilities::StricmpAscii(paramName, json_prependUnitLabel()) == 0)
            SetPrependUnitLabel(true);
        else if (BeStringUtilities::StricmpAscii(paramName, json_use1000Separator()) == 0)
            SetUse1000Separator(true);
        else if (BeStringUtilities::StricmpAscii(paramName, json_exponentOnlyNegative()) == 0)
            SetExponentOnlyNegative(true);
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    David Fox-Rabinovitz
//---------------+---------------+---------------+---------------+---------------+-------
Json::Value NumericFormatSpec::FormatTraitsToJson() const
    {
    Json::Value jTraits = Json::arrayValue;
    if (IsKeepTrailingZeroes())
        jTraits.append(json_trailZeroes());
    if (IsKeepSingleZero())
        jTraits.append(json_keepSingleZero());
    if (IsZeroEmpty())
        jTraits.append(json_zeroEmpty());
    if (IsKeepDecimalPoint())
        jTraits.append(json_keepDecimalPoint());
    if (IsApplyRounding())
        jTraits.append(json_applyRounding());
    if (IsFractionDash())
        jTraits.append(json_fractionDash());
    if (IsShowUnitLabel())
        jTraits.append(json_showUnitLabel());
    if (IsPrependUnitLabel())
        jTraits.append(json_prependUnitLabel());
    if (IsUse1000Separator())
        jTraits.append(json_use1000Separator());
    if (IsExponenentOnlyNegative())
        jTraits.append(json_exponentOnlyNegative());

    return jTraits;
    }

// Formatting methods

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
 Utf8String NumericFormatSpec::Format(int32_t ival) const
    {
    char buf[64];
    FormatInt(ival, buf, sizeof(buf));
    return Utf8String(buf);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
Utf8String NumericFormatSpec::Format(double dval) const
    {
    char buf[64];
    FormatDouble(dval, buf, sizeof(buf));
    return Utf8String(buf);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
int NumericFormatSpec::IntPartToText(double n, Utf8P bufOut, int bufLen, bool useSeparator) const
    {
    char sign = '+';
    char buf[64];
    double n1;
    int ind = 0;

    if (bufLen < 2)  // if output buffer is too short make it empty and return
        {
        if (nullptr == bufOut)
            *bufOut = 0;
        return 0;
        }

    if (n == 0)
        {
        bufOut[ind++] = '0';
        bufOut[ind] = 0;
        return ind;
        }

    if (n < 0)
        {
        n = -n;
        sign = IsNegativeParentheses() ? '(' : '-';
        }

    ind = sizeof(buf);
    memset(buf, 0, ind--);
    //if (sign == '(')
    //    buf[--ind] = ')';
    int digs = 0;
    int rem;
    do {
        n1 = floor(n / 10.0);
        rem = (int)(n - 10.0 * n1);
        buf[--ind] = (char)rem + '0';
        if (IsInsertSeparator(useSeparator))
            digs++;
        n = n1;
        if (n > 0 && digs > 2)
            {
            buf[--ind] = m_thousandsSeparator;
            digs = 0;
            }
        } while (n > 0 && ind >= 0);

    //if (IsSignAlways() || ((IsOnlyNegative() || IsNegativeParentheses()) && sign != '+'))
    //        buf[--ind] = sign;

    int textLen = sizeof(buf) - ind;
    if (textLen > (--bufLen))
        textLen = bufLen;
    memcpy(bufOut, &buf[ind], textLen--);
    return textLen;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 07/17
//---------------------------------------------------------------------------------------
Utf8String NumericFormatSpec::FormatToString(int n, int minSize) const
    {
    const int bufLen = 64;  // this's overkill, but does not hurt
    char dig[bufLen+2];
    char str[bufLen+2];
    char sign = '+';
    memset(dig, 0, sizeof(dig));
    if (minSize > bufLen)
        minSize = bufLen;
    int rem;
    if (n < 0)
        {
        sign = '-';
        n = -n;
        }
    int tmp = n;
    int i = 0;
    do {
        rem = tmp / 10;
        dig[i++] = '0' + (char)(tmp - rem * 10);
        tmp = rem;
        } while (rem > 0);
    int extraZ = minSize - i;
    int k = 0;
    if (IsSignAlways() || (IsOnlyNegative() && sign == '-'))
        str[k++] = sign;
    while (extraZ > 0 && k < bufLen)
        {
        str[k++] = '0';
        extraZ--;
        }
    while (i > 0 && k < bufLen)
        {
        str[k++] = (char)dig[--i];
        str[k] = '\0';
        }
    return Utf8String(str);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
int NumericFormatSpec::FormatSimple(int n, Utf8P bufOut, int bufLen, bool showSign)
    {
    char sign = '+';
    char buf[64];
    int n1;
    int ind = 0;

    if (n < 0)
        {
        n = -n;
        sign = '-';
        }

    if (n == 0)  // the buffer is at least sufficient to take two bytes for the '0' value
        {
        if (showSign)
            *bufOut++= sign;
        *bufOut++= '0';
        *bufOut = 0;
        return 2;
        }



    ind = sizeof(buf);
    memset(buf, 0, ind--);

    do {
        n1 = n / 10;
        buf[--ind] = (char)(n - 10 * n1) + '0';
        n = n1;
        } while (n > 0 && ind >= 0);

    if (showSign || sign != '+')
        buf[--ind] = sign;

    int textLen = sizeof(buf) - ind;
    if (textLen > (--bufLen))
        textLen = bufLen;
    memcpy(bufOut, &buf[ind], textLen--);
    return textLen;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
int NumericFormatSpec::TrimTrailingZeroes(Utf8P buf, int index) const
    {
    if (nullptr == buf)
        return index;
    int i = (0 > index) ? (int)strlen(buf) - 1 : index;

    while (buf[i] == '0' && index > 0)
        {
        if (m_decimalSeparator == buf[i - 1])
            {
            if (IsKeepSingleZero()) // preserve decimal separator and a single zero after it
                break;
            i--;
            if (IsKeepDecimalPoint())
                break;
            i--;
            break;
            }
        i--;
        }

    return i + 1;  // first position after last zero
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
size_t NumericFormatSpec::InsertChar(Utf8P buf, size_t index, char c, int num) const
    {
    if (nullptr != buf && 0 < num)
        {
        for(size_t i = 0; i < static_cast<size_t>(num); buf[index++] = c, i++){}
        }
    return index;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
int NumericFormatSpec::FormatInt(int n, Utf8P bufOut,  int bufLen) const
    {
    // If the type is scientific, we're going to get a double out
    if (PresentationType::Scientific == GetPresentationType())
        return static_cast<int>(FormatDouble(n, bufOut, bufLen));

    char sign = '+';
    int numIgnoredChars = 1;
    int n1;

    char buf[64];
    uint32_t ind = sizeof(buf);
    memset(buf, 0, ind--);

    if (bufLen < 2)  // if output buffer is too short make it empty and return
        {
        if(nullptr != bufOut)
            *bufOut = 0;
        return 0;
        }

    if (IsZeroEmpty() && n == 0)
        { 
        *bufOut = 0;
        return 0;
        }

    if (n < 0)
        {
        n = -n;
        sign = IsNegativeParentheses() ? '(' : '-';
        numIgnoredChars++;
        }

    if ('(' == sign)
        {
        buf[--ind] = ')';
        numIgnoredChars++;
        }

    if (IsSignAlways() && sign == '+')
        numIgnoredChars++;

    int digs = 0;
    uint32_t numberLength = 0; // The number of digits + separators
    do {
        n1 = n / 10;
        buf[--ind] = (char)(n - 10 * n1) + '0';
        if (IsUse1000Separator())
            digs++;
        n = n1;
        if (n > 0 && digs > 2)
            {
            digs = 0;
            buf[--ind] = m_thousandsSeparator;
            ++numberLength;
            }
        ++numberLength;
        } while (n > 0);

    uint32_t minWidth = GetMinWidth();
    if (GetMinWidth() > sizeof(buf) - numIgnoredChars)
        {
        minWidth = sizeof(buf) - numIgnoredChars;
        LOG.warningv("A minWidth of %d is too large. Setting min width to %d", GetMinWidth(), minWidth);
        }

    while (numberLength < minWidth) 
        {
        buf[--ind] = '0';
        ++numberLength;
        }

    if (IsSignAlways() || ((IsOnlyNegative() || IsNegativeParentheses()) && sign != '+'))
        buf[--ind] = sign;

    int textLen = sizeof(buf) - ind;
    if (textLen > bufLen)
        textLen = bufLen;
    memcpy(bufOut, &buf[ind], textLen--);
    return textLen;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
size_t NumericFormatSpec::FormatDouble(double dval, Utf8P buf, size_t bufLen) const
    {
    double ival;
    Utf8Char sign = '+';
    double precScale = GetDecimalPrecisionFactor();
    int totFractLen = Utils::DecimalPrecisionToInt(m_decPrecision);
    double expInt = 0.0;
    double fract;
    char intBuf[64];
    char fractBuf[64];
    char locBuf[128];
    size_t ind = 0;
    if (IsZeroEmpty() && (std::abs(dval - 0.0) < 0.0001))
        { 
        *buf = 0;
        return 0;
        }
    memset(locBuf, 0, sizeof(locBuf));
    if (dval < 0.0)
        {
        dval = -dval;
        sign = (m_signOption == SignOption::NegativeParentheses) ? '(' : '-';
        }
    bool sci = ((dval > 1.0e12) ||m_presentationType == PresentationType::Scientific);
    bool decimal = (sci || m_presentationType == PresentationType::Decimal);
    bool fractional = (!decimal && m_presentationType == PresentationType::Fractional);
    bool stops = m_presentationType == PresentationType::Station;

    if (IsApplyRounding() || !FormatConstant::IsIgnored(m_roundFactor))
        dval = RoundDouble(dval, EffectiveRoundFactor(m_roundFactor));

    if (sci && dval > 0.0001)
        {
        double exp = log10(dval);
        bool negativeExp = false;
        if (exp < 0.0)
            {
            exp = -exp;
            negativeExp = true;
            }

        expInt = floor(exp);
        if (m_presentationType == PresentationType::Scientific)
            {
            if (m_scientificType == ScientificType::ZeroNormalized && dval > 1.0)
                expInt += 1.0;
            else if (m_scientificType == ScientificType::Normalized && dval < 1.0)
                expInt += 1.0;
            }
        if (negativeExp)
            expInt = -expInt;
        double factor = pow(10.0, -expInt);
        dval *= factor;
        }

    if (decimal)
        {
        double actval = IsPrecisionZero() ? dval + FormatConstant::FPV_RoundFactor() : dval + FormatConstant::FPV_MinTreshold();
        fract = modf(actval , &ival);
        if (!IsPrecisionZero())
            {
            fract = fabs(fract) * precScale + FormatConstant::FPV_RoundFactor();
            if (fract >= precScale)
                {
                ival += 1;
                fract -= precScale;
                }
            }

        int iLen = IntPartToText(ival, intBuf, sizeof(intBuf), true);

        memcpy(&locBuf[ind], intBuf, iLen);
        ind += iLen;

        if (IsPrecisionZero())
            {
            if (IsKeepSingleZero())
                {
                locBuf[ind++] = m_decimalSeparator;
                locBuf[ind++] = '0';
                }
            }
        else
            {
            int fLen = IntPartToText(fract, fractBuf, sizeof(fractBuf), false);
            locBuf[ind++] = m_decimalSeparator;
            ind = InsertChar(locBuf, ind, '0', totFractLen - fLen);
            memcpy(&locBuf[ind], fractBuf, fLen);
            ind += fLen;
            // handling trailing zeroes
            if (!IsKeepTrailingZeroes() && locBuf[ind - 1] == '0')
                ind = TrimTrailingZeroes(locBuf, static_cast<int>(ind)-1);
            }
        if (sci)
            {
            char expBuf[32];
            int expLen = FormatSimple ((int)expInt, expBuf, sizeof(expBuf), true);
            locBuf[ind++] = 'e';
            memcpy(&locBuf[ind], expBuf, expLen);
            ind += expLen;
            }

        locBuf[ind++] = FormatConstant::EndOfLine();

        if (ind > bufLen)
            ind = bufLen;

        auto charCountsTowardsMinWidth = [](Utf8Char c)
            {
            return ((c >= '0' && c <= '9') || c == ',' || c == '.');
            };

        uint32_t curWidth = 0;
        for (int i = 0; i < ind; i++)
            {
            if(charCountsTowardsMinWidth(locBuf[i]))
                ++curWidth;
            }
        auto* bufView = buf;
        memset(buf, 0, bufLen);
        if (m_signOption == SignOption::SignAlways || 
            ((m_signOption == SignOption::OnlyNegative || m_signOption == SignOption::NegativeParentheses) && sign != '+'))
            { 
            *bufView = sign;
            ++bufView;
            }
        if (curWidth < GetMinWidth())
            {
            uint32_t missingZeroes = GetMinWidth() - curWidth;
            memset(bufView, '0', missingZeroes);
            bufView+= missingZeroes;
            }
        memcpy(bufView, locBuf, ind);
        bufView+=ind;
        // closing formatting
        if ('(' == sign)
            strcat(buf, ")");
        } // decimal
    else if (fractional)
        {
        FractionalNumeric fn = FractionalNumeric(dval, m_fractPrecision);
        fn.FormTextParts(true);
        size_t locBufL = sizeof(locBuf);
        if (!fn.IsZero())
            {
            if (m_signOption == SignOption::SignAlways ||
                ((m_signOption == SignOption::OnlyNegative || m_signOption == SignOption::NegativeParentheses) && sign != '+'))
                locBuf[ind++] = sign;
            }
        ind = StringUtils::AppendText(locBuf, locBufL, ind, fn.GetIntegralString().c_str());
        if (fn.HasFractionPart())
            {
            if (ind < locBufL && IsFractionDash())
                ind = StringUtils::AppendText(locBuf, locBufL, ind, "-");
            else
                ind = StringUtils::AppendText(locBuf, locBufL, ind, " ");
            if (ind < locBufL)
                ind = StringUtils::AppendText(locBuf, locBufL, ind, fn.GetNumeratorString().c_str());
            if (ind < locBufL)
                ind = StringUtils::AppendText(locBuf, locBufL, ind, "/");
            if (ind < locBufL)
                ind = StringUtils::AppendText(locBuf, locBufL, ind, fn.GetDenominatorString().c_str());
            }
        ind++;
        if (ind > bufLen)
            ind = bufLen;
        PUSH_MSVC_IGNORE(6385 6386) // Static analysis thinks that ind can exceed buflen
        memcpy(buf, locBuf, ind);
        POP_MSVC_IGNORE
        } // end fractional
    else if (stops) // we assume that stopping value is always positive 
        {
        int denom = static_cast<int>(pow(10, m_stationSize));
        int tval = static_cast<int>(dval); // this is the integer part only
        int hiPart = tval / denom;
        int loPart = tval - hiPart * denom;
        fract = modf(dval, &ival);
        int frPart = (int)(0.5 + fract * GetDecimalPrecisionFactor());
        size_t k = 0;
        if (hiPart > 0)
            {
            Utf8String hiS = FormatToString(hiPart, 0);
            memcpy(locBuf, hiS.c_str(), hiS.length());
            k += hiS.length();
            }
        else
            locBuf[k++] = '0';
        
        locBuf[k++] = GetStationSeparator();
        Utf8String loS = FormatToString(loPart, m_minWidth);
        memcpy(&locBuf[k], loS.c_str(), loS.length());
        k += loS.length();
        if (frPart > 0)
            {
            Utf8String frS = FormatToString(frPart, 0);
            locBuf[k++] = GetDecimalSeparator();
            memcpy(&locBuf[k], frS.c_str(), frS.length());
            k += frS.length();
            }
        else if (IsKeepTrailingZeroes())
            {
            int aft = GetDecimalPrecisionIndex();
            if (aft > 0)
                {
                locBuf[k++] = GetDecimalSeparator();
                while (aft > 0)
                    {
                    locBuf[k++] = '0';
                    aft--;
                    }
                }
            else if(IsKeepDecimalPoint())
                locBuf[k++] = GetDecimalSeparator();
            }
        locBuf[k++] = '\0';
        if (k > bufLen)
            k = bufLen;
        PUSH_MSVC_IGNORE(6385 6386) // Static analysis thinks that ind can exceed buflen
            memcpy(buf, locBuf, k);
        POP_MSVC_IGNORE
        }

    return ind;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//----------------------------------------------------------------------------------------
double NumericFormatSpec::RoundDouble(double dval, double roundTo)
    {
    if (FormatConstant::IsNegligible(roundTo))
        return dval;
    roundTo = fabs(roundTo);
    double rnd = FormatConstant::FPV_RoundFactor() + (fabs(dval) / roundTo);
    double ival;
    modf(rnd, &ival);
    rnd = ival * roundTo;
    return (dval < 0.0) ? -rnd : rnd;
    }

END_BENTLEY_FORMATTING_NAMESPACE
