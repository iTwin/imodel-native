/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/NumericFormatSpec.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>

#include <Formatting/FormattingApi.h>
#include "../../PrivateAPI/Formatting/FormattingParsing.h"
#include "../../PrivateAPI/Formatting/NumericFormatUtils.h"

#include <locale>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

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
int NumericFormatSpec::FormatInteger(int n, Utf8P bufOut,  int bufLen)
    {
    char sign = '+';
    char buf[64];
    int n1;
    int ind = 0;

    if (bufLen < 2)  // if output buffer is too short make it empty and return
        {
        if(nullptr == bufOut)
            *bufOut = 0;
        return 0;
        }

    if (n == 0)  // the buffer is at least sufficient to take two bytes for the '0' value
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
    if ('(' == sign)
        buf[--ind] = ')';

    int digs = 0;
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
            }
        } while (n > 0 && ind >= 0);

    if (IsSignAlways() || ((IsOnlyNegative() || IsNegativeParentheses()) && sign != '+'))
        buf[--ind] = sign;

    int textLen = sizeof(buf) - ind;
    if (textLen > (--bufLen))
        textLen = bufLen;
    memcpy(bufOut, &buf[ind], textLen--);
    return textLen;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
size_t NumericFormatSpec::FormatDoubleBuf(double dval, Utf8P buf, size_t bufLen, int prec, double round) const
    {
    double ival;
    Utf8Char sign = '+';
    double precScale = NumericFormatSpec::GetDecimalPrecisionFactor(prec);
    int totFractLen = NumericFormatSpec::GetDecimalPrecisionIndex(prec);
    double expInt = 0.0;
    double fract;
    char intBuf[64];
    char fractBuf[64];
    char locBuf[128];
    size_t ind = 0;
    memset(locBuf, 0, sizeof(locBuf));
    if (dval < 0.0)
        {
        dval = -dval;
        sign = (m_signOption == ShowSignOption::NegativeParentheses) ? '(' : '-';
        }
    bool sci = ((dval > 1.0e12) ||m_presentationType == PresentationType::Scientific);
    bool decimal = (sci || m_presentationType == PresentationType::Decimal);
    bool fractional = (!decimal && m_presentationType == PresentationType::Fractional);
    bool stops = m_presentationType == PresentationType::Station;

    if (IsApplyRounding() || !FormatConstant::IsIgnored(round))
        dval = RoundDouble(dval, EffectiveRoundFactor(round));

    if (sci)
        {
        double exp = log10(dval);
        bool negativeExp = false;
        if (exp < 0.0)
            {
            exp = -exp;
            negativeExp = true;
            }

        expInt = floor(exp);
        if (m_presentationType == PresentationType::Scientific && m_scientificType == ScientificType::Normal)
            expInt += 1.0;
        if (negativeExp)
            expInt = -expInt;
        //double expFract = modf(exp, &expInt);
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
        if (m_signOption == ShowSignOption::SignAlways || 
            ((m_signOption == ShowSignOption::OnlyNegative || m_signOption == ShowSignOption::NegativeParentheses) && sign != '+'))
            locBuf[ind++] = sign;

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
           /* if (fract < 0.0)
                fract = -fract;
            fract = fract * precScale + 0.501;*/
            int fLen = IntPartToText(fract, fractBuf, sizeof(fractBuf), false);

            locBuf[ind++] = m_decimalSeparator;
            ind = InsertChar(locBuf, ind, '0', totFractLen - fLen);
           /* while (fLen < totFractLen)
            {
                locBuf[ind++] = '0';
                fLen++;
            }*/
            memcpy(&locBuf[ind], fractBuf, fLen);
            ind += fLen;
            // handling trailing zeroes
            if (!IsKeepTrailingZeroes() && locBuf[ind - 1] == '0')
                ind = TrimTrailingZeroes(locBuf, static_cast<int>(ind)-1);
            }
        if (sci && expInt != 0)
            {
            char expBuf[32];
            int expLen = FormatIntegerSimple ((int)expInt, expBuf, sizeof(expBuf), true, (IsExponentZero() ? true : false));
            locBuf[ind++] = 'e';
            //if (IsExponentZero())
            //    locBuf[ind++] = '0';
            memcpy(&locBuf[ind], expBuf, expLen);
            ind += expLen;
            }
        // closing formatting
        if ('(' == sign)
            locBuf[ind++] = ')';
        locBuf[ind++] = FormatConstant::EndOfLine();

        if (ind > bufLen)
            ind = bufLen;
        memcpy(buf, locBuf, ind);
        } // decimal
    else if (fractional)
        {
        FractionalNumeric fn = FractionalNumeric(dval, m_fractPrecision);
        fn.FormTextParts(true);
        size_t locBufL = sizeof(locBuf);
        if (!fn.IsZero())
            {
            if (m_signOption == ShowSignOption::SignAlways ||
                ((m_signOption == ShowSignOption::OnlyNegative || m_signOption == ShowSignOption::NegativeParentheses) && sign != '+'))
                locBuf[ind++] = sign;
            }
        ind = Utils::AppendText(locBuf, locBufL, ind, fn.GetIntegralString().c_str());
        if (fn.HasFractionPart())
            {
            if (ind < locBufL)
                ind = Utils::AppendText(locBuf, locBufL, ind, " ");
            if (ind < locBufL)
                ind = Utils::AppendText(locBuf, locBufL, ind, fn.GetNumeratorString().c_str());
            if (ind < locBufL)
                ind = Utils::AppendText(locBuf, locBufL, ind, "/");
            if (ind < locBufL)
                ind = Utils::AppendText(locBuf, locBufL, ind, fn.GetDenominatorString().c_str());
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
        int denom = 10*m_stationSize;
        int tval = static_cast<int>(dval); // this is the integer part only
        int hiPart = tval / denom;
        int loPart = tval - hiPart * denom;
        fract = modf(dval, &ival);
        int frPart = (int)(0.5 + fract * GetDecimalPrecisionFactor());
        size_t k = 0;
        if (hiPart > 0)
            {
            Utf8String hiS = FormatIntegerToString(hiPart, 0);
            memcpy(locBuf, hiS.c_str(), hiS.length());
            k += hiS.length();
            }
        else
            locBuf[k++] = '0';
        
        locBuf[k++] = GetStatSeparator();
        Utf8String loS = FormatIntegerToString(loPart, m_minWidth);
        memcpy(&locBuf[k], loS.c_str(), loS.length());
        k += loS.length();
        if (frPart > 0)
            {
            Utf8String frS = FormatIntegerToString(frPart, 0);
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
int NumericFormatSpec::GetDecimalPrecisionIndex(int prec) const
    { 
    if (0 <= prec && prec < Utils::DecimalPrecisionToInt(DecimalPrecision::Precision12))
        return prec;
    return Utils::DecimalPrecisionToInt(m_decPrecision); 
    }

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

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/18
//----------------------------------------------------------------------------------------
bool NumericFormatSpec::ImbueLocaleProperties(LocalePropertiesCR locProp)
    {
    m_decimalSeparator = locProp.GetDecimalSeparator();
    m_thousandsSeparator = locProp.GetThousandSeparator();
    return true;
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
Utf8String NumericFormatSpec::FormatIntegerToString(int n, int minSize) const
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
int NumericFormatSpec::FormatIntegerSimple(int n, Utf8P bufOut, int bufLen, bool showSign, bool extraZero)
    {
    char sign = '+';
    char buf[64];
    int n1;
    int ind = 0;

    if (n == 0)  // the buffer is at least sufficient to take two bytes for the '0' value
        {
        *bufOut++= '0';
        *bufOut = 0;
        return ind;
        }

    if (n < 0)
        {
        n = -n;
        sign = '-';
        }

    ind = sizeof(buf);
    memset(buf, 0, ind--);

    do {
        n1 = n / 10;
        buf[--ind] = (char)(n - 10 * n1) + '0';
        n = n1;
        } while (n > 0 && ind >= 0);

        if (showSign || sign != '+')
            {
            if (extraZero)
                buf[--ind] = '0';
            buf[--ind] = sign;
            }

        int textLen = sizeof(buf) - ind;
        if (textLen > (--bufLen))
            textLen = bufLen;
        memcpy(bufOut, &buf[ind], textLen--);
        return textLen;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
NumericFormatSpec::NumericFormatSpec()
    : m_explicitlyDefinedDecimalSeparator(false)
    , m_explicitlyDefinedPrecision(false)
    , m_explicitlyDefinedRoundFactor(false)
    , m_explicitlyDefinedShowSign(false)
    , m_explicitlyDefinedStatSeparator(false)
    , m_explicitlyDefinedThousandsSeparator(false)
    , m_explicitlyDefinedUOMSeparator(false)
    , m_roundFactor(0.0)
    , m_presentationType(FormatConstant::DefaultPresentaitonType())
    , m_signOption(FormatConstant::DefaultSignOption())
    , m_formatTraits(FormatConstant::DefaultFormatTraits())
    , m_decPrecision(FormatConstant::DefaultDecimalPrecision())
    , m_fractPrecision(FormatConstant::DefaultFractionalPrecision())
    , m_decimalSeparator(FormatConstant::FPV_DecimalSeparator())
    , m_thousandsSeparator(FormatConstant::FPV_ThousandSeparator())
    , m_uomSeparator(FormatConstant::BlankString())
    , m_statSeparator(FormatConstant::DefaultStationSeparator())
    , m_minWidth(0)
    , m_stationSize(0)
    , m_scientificType(ScientificType::Standard)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz
//----------------------------------------------------------------------------------------
NumericFormatSpec::NumericFormatSpec(Json::Value jval) : NumericFormatSpec()
    {
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
            Utils::NameToPresentationType(val.asCString(), m_presentationType);
        else if (BeStringUtilities::StricmpAscii(paramName, json_roundFactor()) == 0)
            m_roundFactor = val.asDouble();
        else if (BeStringUtilities::StricmpAscii(paramName, json_decPrec()) == 0)
            Utils::DecimalPrecisionByIndex(val.asInt64(), m_decPrecision);
        else if (BeStringUtilities::StricmpAscii(paramName, json_fractPrec()) == 0)
            Utils::FractionalPrecisionByDenominator(val.asInt64(), m_fractPrecision);
        else if (BeStringUtilities::StricmpAscii(paramName, json_signOpt()) == 0)
            Utils::NameToSignOption(val.asCString(), m_signOption);
        else if (BeStringUtilities::StricmpAscii(paramName, json_decimalSeparator()) == 0)
            m_decimalSeparator = val.asString().c_str()[0];
        else if (BeStringUtilities::StricmpAscii(paramName, json_thousandSeparator()) == 0)
            m_thousandsSeparator = val.asString().c_str()[0];
        else if (BeStringUtilities::StricmpAscii(paramName, json_uomSeparator()) == 0)
            m_uomSeparator = val.asString();
        else if (BeStringUtilities::StricmpAscii(paramName, json_statSeparator()) == 0)
            m_statSeparator = val.asString().c_str()[0];
        else if (BeStringUtilities::StricmpAscii(paramName, json_minWidth()) == 0)
            m_minWidth = val.asInt();
        else if (BeStringUtilities::StricmpAscii(paramName, json_formatTraits()) == 0)
            SetFormatTraitsFromJson(val);
        }
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
    return nfs;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                    03/18
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String NumericFormatSpec::GetFormatTraitsString() const
    {
    Utf8String ret;
    bvector<Utf8String> strings;
    if (GetTraitsBit(FormatTraits::ApplyRounding))
        strings.push_back(FormatConstant::FPN_ApplyRounding());
    if (GetTraitsBit(FormatTraits::ExponenentOnlyNegative))
        strings.push_back(FormatConstant::FPN_ExponentOnlyNegative());
    if (GetTraitsBit(FormatTraits::FractionDash))
        strings.push_back(FormatConstant::FPN_FractionDash());
    if (GetTraitsBit(FormatTraits::KeepDecimalPoint))
        strings.push_back(FormatConstant::FPN_KeepDecimalPoint());
    if (GetTraitsBit(FormatTraits::KeepSingleZero))
        strings.push_back(FormatConstant::FPN_KeepSingleZero());
    if (GetTraitsBit(FormatTraits::PrependUnitLabel))
        strings.push_back(FormatConstant::FPN_PrependUnitName());
    if (GetTraitsBit(FormatTraits::ShowUnitLabel))
        strings.push_back(FormatConstant::FPN_ShowUnitName());
    if (GetTraitsBit(FormatTraits::TrailingZeroes))
        strings.push_back(FormatConstant::FPN_TrailZeroes());
    if (GetTraitsBit(FormatTraits::Use1000Separator))
        strings.push_back(FormatConstant::FPN_Use1000Separator());
    if (GetTraitsBit(FormatTraits::ZeroEmpty))
        strings.push_back(FormatConstant::FPN_ZeroEmpty());

    int i = 0;
    for (auto const& s : strings)
        {
        ret += s;
        if (i < strings.size() - 1)
            { 
            ret += "|";
            ++i;
            }
        }
    return ret;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Kyle.Abramowitz                    03/18
//---------------+---------------+---------------+---------------+---------------+-------
bool NumericFormatSpec::SetFormatTraitsFromString(Utf8StringCR input)
    {
    bvector<Utf8String> splitStrings;
    BeStringUtilities::Split(input.c_str(),",;|", splitStrings);
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
        else if (BeStringUtilities::StricmpAscii(s.c_str(), FormatConstant::FPN_ShowUnitName().c_str()) == 0)
            this->SetTraitsBit(FormatTraits::ShowUnitLabel, true);
        else if (BeStringUtilities::StricmpAscii(s.c_str(), FormatConstant::FPN_PrependUnitName().c_str()) == 0)
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
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
// static
bool NumericFormatSpec::GetTraitsBit(FormatTraits traits, FormatTraits bit)
    {
    return 0 != (static_cast<std::underlying_type<FormatTraits>::type>(traits)
        & static_cast<std::underlying_type<FormatTraits>::type>(bit));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    David Fox-Rabinovitz
//---------------+---------------+---------------+---------------+---------------+-------
void NumericFormatSpec::TraitsBitToJson(JsonValueR outValue, Utf8CP bitIndex, FormatTraits bit, FormatTraits* ref, bool verbose) const
    {
    if (ref == nullptr)
        verbose = true;
    if ((nullptr == ref) || !FormatConstant::IsBoolEqual(GetTraitsBit(bit), GetTraitsBit(*ref, bit)))  //.IsKeepTrailingZeroes())
        outValue[bitIndex] = FormatConstant::BoolText(GetTraitsBit(bit));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    David Fox-Rabinovitz
//---------------+---------------+---------------+---------------+---------------+-------
void NumericFormatSpec::SetFormatTraitsFromJson(JsonValueCR jval)
    {
    Utf8CP paramName;
    Utf8String str;
    if (jval.empty())
        return;

    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (BeStringUtilities::StricmpAscii(paramName, json_leadZeroes()) == 0)
            SetUseLeadingZeroes(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_trailZeroes()) == 0)
            SetKeepTrailingZeroes(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_keepDecimalPrecision()) == 0)
            SetKeepDecimalPoint(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_keepSingleZero()) == 0)
            SetKeepSingleZero(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_zeroEmpty()) == 0)
            SetZeroEmpty(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_use1000Separator()) == 0)
            SetUse1000Separator(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_applyRounding()) ==0)
            SetApplyRounding(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_fractionDash()) == 0)
            SetFractionDash(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_prependUnitLabel()) == 0)
            SetPrependUnitLabel(val.asBool());
        else if (BeStringUtilities::StricmpAscii(paramName, json_showUnitLabel()) == 0)
            SetShowUnitLabel(val.asBool());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    David Fox-Rabinovitz
//---------------+---------------+---------------+---------------+---------------+-------
Json::Value NumericFormatSpec::FormatTraitsToJson(bool verbose) const
    {
    Json::Value jTraits;
    FormatTraits ref = FormatConstant::DefaultFormatTraits();

    TraitsBitToJson(jTraits, json_trailZeroes(), FormatTraits::TrailingZeroes, &ref, verbose);
    TraitsBitToJson(jTraits, json_leadZeroes(),  FormatTraits::LeadingZeroes, &ref, verbose);
    TraitsBitToJson(jTraits, json_keepDecimalPrecision(), FormatTraits::KeepDecimalPoint, &ref, verbose);
    TraitsBitToJson(jTraits, json_keepSingleZero(), FormatTraits::KeepSingleZero, &ref, verbose);
    TraitsBitToJson(jTraits, json_zeroEmpty(), FormatTraits::ZeroEmpty, &ref, verbose);
    TraitsBitToJson(jTraits, json_use1000Separator(), FormatTraits::Use1000Separator, &ref, verbose);
    TraitsBitToJson(jTraits, json_applyRounding(), FormatTraits::ApplyRounding, &ref, verbose);
    TraitsBitToJson(jTraits, json_prependUnitLabel(), FormatTraits::PrependUnitLabel, &ref, verbose);
    TraitsBitToJson(jTraits, json_exponentOnlyNegative(), FormatTraits::ExponenentOnlyNegative, &ref, verbose);
    TraitsBitToJson(jTraits, json_fractionDash(), FormatTraits::FractionDash, &ref, verbose);
    TraitsBitToJson(jTraits, json_showUnitLabel(), FormatTraits::ShowUnitLabel, &ref, verbose);
    return jTraits;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
 Utf8String NumericFormatSpec::FormatInteger(int32_t ival)
    {
    char buf[64];
    FormatInteger(ival, buf, sizeof(buf));
    return Utf8String(buf);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
Utf8String NumericFormatSpec::FormatDouble(double dval, int prec, double round) const
    {
    char buf[64];
    FormatDoubleBuf(dval, buf, sizeof(buf), prec, round);
    return Utf8String(buf);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                 03/18
//---------------+---------------+---------------+---------------+---------------+-------
// static
Utf8String NumericFormatSpec::FormatDouble(NumericFormatSpecCR nfs, double dval, int prec, double round)
    {
    return nfs.FormatDouble(dval, prec, round);
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
        jNFC[json_formatTraits()] = FormatTraitsToJson(verbose);
    if (verbose || m_decimalSeparator != defSpec.m_decimalSeparator)
        jNFC[json_decimalSeparator()] = m_decimalSeparator;
    if (verbose || m_thousandsSeparator != defSpec.m_thousandsSeparator)
        jNFC[json_thousandSeparator()] = m_thousandsSeparator;
    if (verbose || !m_uomSeparator.Equals(defSpec.m_uomSeparator.c_str()))
        jNFC[json_uomSeparator()] = m_uomSeparator;
    if (verbose || m_statSeparator != defSpec.m_statSeparator)
        jNFC[json_statSeparator()] = m_statSeparator;
    if (verbose || m_minWidth != defSpec.m_minWidth)
        jNFC[json_minWidth()] = m_minWidth;
    return jNFC;
    }

END_BENTLEY_FORMATTING_NAMESPACE
