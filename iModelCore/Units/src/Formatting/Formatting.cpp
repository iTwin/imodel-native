/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/Formatting.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>
#include <locale>
#include <BeSQLite/L10N.h>
#include "../../PrivateAPI/Formatting/FormattingParsing.h"
#include "../../PrivateAPI/Units/UnitRegistry.h" // temporary

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//===================================================
//
// NumericFormatSpec Methods
//
//===================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
NumericFormatSpec::NumericFormatSpec()
    : NumericFormatSpec(Utils::DecimalPrecisionByIndex(FormatConstant::DefaultDecimalPrecisionIndex()))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
NumericFormatSpec::NumericFormatSpec(DecimalPrecision decimalPrecision)
    : m_roundFactor(0.0)
    , m_presentationType(FormatConstant::DefaultPresentaitonType())
    , m_signOption(FormatConstant::DefaultSignOption())
    , m_formatTraits(FormatConstant::DefaultFormatTraits())
    , m_decPrecision(decimalPrecision)
    , m_fractPrecision(FormatConstant::DefaultFractionalPrecision())
    , m_barType(FractionBarType::Diagonal)
    , m_decimalSeparator(FormatConstant::FPV_DecimalSeparator())
    , m_thousandsSeparator(FormatConstant::FPV_ThousandSeparator())
    , m_uomSeparator(FormatConstant::BlankString())
    , m_statSeparator('+')
    , m_minWidth(0)
    {
    }

//===================================================
//
// LocaleProperties
//
//===================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/18
//----------------------------------------------------------------------------------------
LocaleProperties::LocaleProperties(Utf8CP localeName)
{
    std::locale loc = std::locale(localeName);
    const std::numpunct<char>& myfacet(std::use_facet < std::numpunct<char> >(loc));

    m_decimalSeparator = myfacet.decimal_point();
    m_thousandsSeparator = myfacet.thousands_sep();
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/18
//----------------------------------------------------------------------------------------
LocaleProperties LocaleProperties::DefaultAmerican()
{
    return LocaleProperties('.',',');
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/18
//----------------------------------------------------------------------------------------
LocaleProperties LocaleProperties::DefaultEuropean(bool useBlank)
{
    return LocaleProperties(',', (useBlank? ' ' : '.'));
}

LocaleProperties::LocaleProperties(Json::Value jval)
{
    if (!jval.empty())
    {
        Utf8CP paramName;
        Utf8String str;
        Utf8String jStr = jval.ToString();
        for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
            paramName = iter.memberName();
            JsonValueCR val = *iter;
            if (BeStringUtilities::StricmpAscii(paramName, json_decimalSeparator()) == 0)
            {
                str = val.asString();
                m_decimalSeparator = str.c_str()[0];
            }
            else if (BeStringUtilities::StricmpAscii(paramName, json_thousandSeparator()) == 0)
            {
                str = val.asString();
                m_thousandsSeparator = str.c_str()[0];
            }
        }
    }
    else
    {
        m_decimalSeparator = '.';
        m_thousandsSeparator = '\0';
    }
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/18
//----------------------------------------------------------------------------------------
Json::Value LocaleProperties::ToJson()
{
    Json::Value jval;
    jval[json_decimalSeparator()] = Utils::CharToString(m_decimalSeparator);
    jval[json_thousandSeparator()] = Utils::CharToString(m_thousandsSeparator);
    return jval;
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/18
//----------------------------------------------------------------------------------------
Utf8String LocaleProperties::ToText()
{
   return Utf8PrintfString("Separators: decimal |%c| thousands |%c|", m_decimalSeparator, m_thousandsSeparator);
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//----------------------------------------------------------------------------------------
void NumericFormatSpec::DefaultInit(size_t precision)
    {
    m_roundFactor = 0.0;
    m_presentationType = FormatConstant::DefaultPresentaitonType();
    m_signOption = FormatConstant::DefaultSignOption();
    m_formatTraits = FormatConstant::DefaultFormatTraits();
    m_decPrecision = Utils::DecimalPrecisionByIndex(precision);
    m_fractPrecision = FormatConstant::DefaultFractionalPrecision();
    m_barType = FractionBarType::Diagonal;
    m_decimalSeparator = FormatConstant::FPV_DecimalSeparator();
    m_thousandsSeparator = FormatConstant::FPV_ThousandSeparator();
    m_uomSeparator = FormatConstant::BlankString();
    m_statSeparator = '+';
    m_minWidth = 0;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//----------------------------------------------------------------------------------------
//void NumericFormatSpec::Init(Utf8CP name, PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, size_t precision)
void NumericFormatSpec::Init(PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, size_t precision)
    {
    m_roundFactor = 0.0;
    m_presentationType = presType;
    m_signOption = signOpt;
    m_formatTraits = formatTraits;
    m_barType = FractionBarType::Diagonal;
    if (PresentationType::Fractional == m_presentationType)
        {
        m_decPrecision = FormatConstant::DefaultDecimalPrecision();
        m_fractPrecision = Utils::FractionalPrecisionByDenominator(precision);
        }
    else
        {
        m_decPrecision = Utils::DecimalPrecisionByIndex(precision);
        m_fractPrecision = FormatConstant::DefaultFractionalPrecision();
        }

    m_decimalSeparator = FormatConstant::FPV_DecimalSeparator();
    m_thousandsSeparator = FormatConstant::FPV_ThousandSeparator();
    m_uomSeparator = FormatConstant::BlankString();
    m_statSeparator = '+';
    m_minWidth = 0;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/17
//----------------------------------------------------------------------------------------
//NumericFormatSpec& NumericFormatSpec::operator=(const NumericFormatSpec& other)
//    {
//    if (this != &other)
//        {
//        Clone(other);
//        }
//    return *this;
//    }

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
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//----------------------------------------------------------------------------------------
bool NumericFormatSpec::AcceptableDifference(double dval1, double dval2, double maxDiff)
    {
    return (fabs(maxDiff) > fabs(dval1 - dval2));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//----------------------------------------------------------------------------------------
NumericFormatSpec::NumericFormatSpec(PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, size_t precision, Utf8CP uomSeparator)
    {
    Init(presType, signOpt, formatTraits, precision);   
    if (uomSeparator)
        SetUomSeparator(uomSeparator);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//----------------------------------------------------------------------------------------
void NumericFormatSpec::SetPrecisionByValue(int prec)
    {
    if (PresentationType::Fractional == m_presentationType)
        {
        m_fractPrecision = Utils::FractionalPrecisionByDenominator(prec);
        }
    else
        {
        m_decPrecision = Utils::DecimalPrecisionByIndex(prec);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
double NumericFormatSpec::RoundedValue(double dval, double round) const
    {
    round = fabs(round);
    if (round < FormatConstant::FPV_MinTreshold())
        return dval;
    double val = floor(fabs(dval) / round) * round;
    return (0.0 < dval) ? val : -val;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
// copies bytes from one char buffer to another starting from the last char of the source
// If the destination buffer is shorter than source, only the right portion of the source 
// will be copied. The destination will be terminated by zero if so indicated by the bool flag
//  and the useful capacity of the destination buffer will be reduced by 1 
int NumericFormatSpec::RightAlignedCopy(Utf8P dest, int destLen, bool termZero, CharCP src, int srcLen)
    {
    if (nullptr == src)
        srcLen = 0;
    else if (srcLen < 0)
        srcLen = (int)strlen(src);

    if (termZero) destLen--;
    if (nullptr == dest || destLen < 1) // copying is not possible
        return -srcLen;

    int actCapacity = (destLen > srcLen) ? srcLen : destLen;
    int deficit = srcLen - actCapacity;
    if (actCapacity > 0)
        memcpy(dest, &src[deficit], (size_t)actCapacity);
    if (termZero)
        dest[actCapacity] = 0;
    return -deficit;
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

// the following methind does not perform buffer related checks and does not use
//  parenthesis for indicating negative numbers However it uses other ShowSign options
// the calling function 
//   The main purpose of this methind is to form exponent value
//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
int NumericFormatSpec::FormatIntegerSimple (int n, Utf8P bufOut, int bufLen, bool showSign, bool extraZero)
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
double NumericFormatSpec::GetDecimalPrecisionFactor(int prec = -1) const
    { 
    return Utils::DecimalPrecisionFactor(m_decPrecision, prec); 
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
int  NumericFormatSpec::GetDecimalPrecisionIndex(int prec = -1) const
    { 
    if (0 <= prec && prec < Utils::DecimalPrecisionToInt(DecimalPrecision::Precision12))
        return prec;
    return Utils::DecimalPrecisionToInt(m_decPrecision); 
    };

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
    bool sci = ((dval > 1.0e12) ||m_presentationType == PresentationType::Scientific || m_presentationType == PresentationType::ScientificNorm);
    bool decimal = (sci || m_presentationType == PresentationType::Decimal);
    bool fractional = (!decimal && m_presentationType == PresentationType::Fractional);
    bool stops = (m_presentationType == PresentationType::Stop100 || m_presentationType == PresentationType::Stop1000);

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
        if (m_presentationType == PresentationType::ScientificNorm)
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
            int expLen = FormatIntegerSimple ((int)expInt, expBuf, sizeof(expBuf), true, (IsExponentZero()?true:false));
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
        ind = Utils::AppendText(locBuf, locBufL, ind, fn.GetIntegralText());
        if (fn.HasFractionPart())
            {
            if (ind < locBufL)
                ind = Utils::AppendText(locBuf, locBufL, ind, " ");
            if (ind < locBufL)
                ind = Utils::AppendText(locBuf, locBufL, ind, fn.GetNumeratorText());
            if (ind < locBufL)
                ind = Utils::AppendText(locBuf, locBufL, ind, "/");
            if (ind < locBufL)
                ind = Utils::AppendText(locBuf, locBufL, ind, fn.GetDenominatorText());
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
        int denom = (m_presentationType == PresentationType::Stop100) ? 100 : 1000;
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
        
        locBuf[k++] = GetStopSeparator();
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
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//---------------------------------------------------------------------------------------
Utf8String NumericFormatSpec::FormatQuantity(BEU::QuantityCR qty, BEU::UnitCP useUnit, Utf8CP space, int prec, double round)
    {
    if (!qty.IsNullQuantity())
        return Utf8String();
    BEU::UnitCP unitQ = qty.GetUnit();
    BEU::Quantity temp = qty.ConvertTo(unitQ);
    char buf[64];
    FormatDoubleBuf(temp.GetMagnitude(), buf, sizeof(buf), prec, round);
    if(nullptr == useUnit || !IsAppendUnit())
        return Utf8String(buf);
    Utf8String txt = Utils::AppendUnitName(buf, useUnit->GetLabel().c_str(), space);
    return txt;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
Utf8String NumericFormatSpec::FormatRoundedDouble(double dval, double round)
    {
    return FormatDouble(RoundedValue(dval, round));
    }

//NumericFormat NumericFormat::StdNumericFormat(Utf8P stdName, int prec, double round)
//    {
//    StdFormatNameMap sfm = StdFormatNameMap();
//    StdFormatNameCP sfn = sfm.GetFormatByName(stdName);
//    if (nullptr == sfn)
//        return nullptr;
//    return NumericFormat((StdFormatNameR)sfn, prec, round);
//    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
Utf8String NumericFormatSpec::StdFormatDouble(Utf8CP stdName, double dval, int prec, double round)
    {
    NumericFormatSpecCP fmtP = StdFormatSet::GetNumericFormat(stdName);
    if (nullptr == fmtP)  // invalid name
        fmtP = StdFormatSet::DefaultDecimal();
    if (nullptr == fmtP)
        return "";
    return fmtP->FormatDouble(dval, prec, round);
    }

Utf8String NumericFormatSpec::StdFormatQuantity(NamedFormatSpecCR nfs, BEU::QuantityCR qty, BEU::UnitCP useUnit, Utf8CP space, Utf8CP useLabel, int prec, double round)
    {
    // there are two major options here: the format is a pure Numeric or it has a composite specification
    NumericFormatSpecCP fmtP = nfs.GetNumericSpec();
    bool composite = nfs.HasComposite();
    BEU::Quantity temp = qty.ConvertTo(useUnit);
    Utf8CP uomLabel = Utils::IsNameNullOrEmpty(useLabel) ? ((nullptr == useUnit) ? qty.GetUnitLabel() : useUnit->GetLabel().c_str()) : useLabel;
    Utf8String majT, midT, minT, subT;

    if (composite)  // procesing composite parts
        {
        CompositeValueSpecP compS = (CompositeValueSpecP)nfs.GetCompositeSpec();
        CompositeValue dval = compS->DecomposeValue(temp.GetMagnitude(), temp.GetUnit());
        Utf8String pref = dval.GetSignPrefix();
        Utf8String suff = dval.GetSignSuffix();
        Utf8CP spacer = Utils::IsNameNullOrEmpty(space) ? compS->GetSpacer().c_str() : space;
        // for all parts but the last one we need to format an integer 
        NumericFormatSpec fmtI = NumericFormatSpec(PresentationType::Decimal, FormatConstant::DefaultSignOption(),
            FormatConstant::DefaultFormatTraits(), 0);
        fmtI.SetKeepSingleZero(false);

        switch (compS->GetType())
            {
            case CompositeSpecType::Single: // there is only one value to report
                majT = pref + fmtP->FormatDouble(dval.GetMajor(), prec, round);
                // if this composite only defines a single component then use format traits to determine if unit label is shown. This allows
                // support for SuppressUnitLable options in DgnClientFx. Also in this single component situation use the define UomSeparator.
                if (fmtP->IsAppendUnit())
                    majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel().c_str(), Utils::SubstituteNull(space, fmtP->GetUomSeparator())) + suff;
                break;

            case CompositeSpecType::Double:
                majT = pref + fmtI.FormatDouble(dval.GetMajor(), prec, round);
                majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel().c_str(), spacer);
                midT = fmtP->FormatDouble(dval.GetMiddle(), prec, round);
                midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel().c_str(), spacer);
                majT += " " + midT + suff;
                break;

            case CompositeSpecType::Triple:
                majT = pref + fmtI.FormatDouble(dval.GetMajor(), prec, round);
                majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel().c_str(), spacer);
                midT = fmtI.FormatDouble(dval.GetMiddle(), prec, round);
                midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel().c_str(), spacer);
                minT = fmtP->FormatDouble(dval.GetMinor(), prec, round);
                minT = Utils::AppendUnitName(minT.c_str(), compS->GetMinorLabel().c_str(), spacer);
                majT += " " + midT + " " + minT + suff;
                break;

            case CompositeSpecType::Quatro:
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
            fmtP = StdFormatSet::DefaultDecimal();
        if (nullptr == fmtP)
            return "";
        majT = fmtP->FormatDouble(temp.GetMagnitude(), prec, round);
        if(fmtP->IsAppendUnit())
           majT = Utils::AppendUnitName(majT.c_str(), uomLabel, Utils::SubstituteNull(space, fmtP->GetUomSeparator()));
        }
    return majT;
    }

const NumericFormatSpecCP NumericFormatSpec::DefaultFormat()
    {
    static NumericFormatSpec nfs = NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, FormatConstant::DefaultFormatTraits(), FormatConstant::DefaultDecimalPrecisionIndex());
    return &nfs;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
 Utf8String NumericFormatSpec::FormatInteger(int ival)
    {
        char buf[64];
        FormatInteger(ival, buf, sizeof(buf));
        return Utf8String(buf);
    }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //---------------------------------------------------------------------------------------
 // the caller provided buffer must be at least 9 byte long with the 9th byte for the terminating 0
 // this function returns the number of bytes that was not populated - in case of success it will 0
 int NumericFormatSpec::FormatBinaryByte (unsigned char n, Utf8P bufOut, int bufLen)
    {
    char binBuf[8];
    unsigned char mask = 0x80;
    int i = 0;
    while (mask != 0)
        {
        binBuf[i++] = (n & mask) ? '1' : '0';
        mask >>= 1;
        }

    return RightAlignedCopy(bufOut, bufLen, true, binBuf, sizeof(binBuf));
    }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //---------------------------------------------------------------------------------------
 int NumericFormatSpec::FormatBinaryShort(short int n, Utf8P bufOut, int bufLen, bool useSeparator)
    {
    char binBuf[64];

    unsigned char c = (n & 0xFF00) >> 8;
    FormatBinaryByte (c, binBuf, 9);
    int ind = 8;
    if (IsInsertSeparator(useSeparator))
        binBuf[ind++] = m_thousandsSeparator;
    c = n & 0xFF;
    FormatBinaryByte (c, &binBuf[ind], 9);
    return RightAlignedCopy(bufOut, bufLen, true, binBuf, -1);
    }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //---------------------------------------------------------------------------------------
 int NumericFormatSpec::FormatBinaryInt(int n, Utf8P bufOut, int bufLen, bool useSeparator)
 {
     char binBuf[80];

     memset(binBuf, 0, sizeof(binBuf));
     unsigned int temp = (size_t)n;
     unsigned short int c = (temp & 0xFFFF0000) >> 16;
     FormatBinaryShort (c, binBuf, sizeof(binBuf), useSeparator);
     size_t ind = strlen(binBuf);
     if (IsInsertSeparator(useSeparator))
         binBuf[ind++] = m_thousandsSeparator;
     c = n & 0xFFFF;
     FormatBinaryShort (c, &binBuf[ind], (int)(sizeof(binBuf) - ind), useSeparator);
     return RightAlignedCopy(bufOut, bufLen, true, binBuf, -1);
 }

 int NumericFormatSpec::FormatBinaryDouble(double x, Utf8P bufOut, int bufLen, bool useSeparator)
 {
     char binBuf[80];
     union { unsigned int ival[2]; double x; }temp;
     temp.x = x;
     memset(binBuf, 0, sizeof(binBuf));
     FormatBinaryInt (temp.ival[1], binBuf, sizeof(binBuf), useSeparator);
     size_t ind = strlen(binBuf);
     if (IsInsertSeparator(useSeparator))
         binBuf[ind++] = m_thousandsSeparator;
     FormatBinaryInt (temp.ival[0], &binBuf[ind], (int)(sizeof(binBuf) - ind), useSeparator);
     return RightAlignedCopy(bufOut, bufLen, true, binBuf, -1);
 }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //---------------------------------------------------------------------------------------

 Utf8String NumericFormatSpec::ByteToBinaryText(unsigned char n)
 {
     char buf[64];
     FormatBinaryByte(n, buf, sizeof(buf));
     return Utf8String(buf);
 }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //---------------------------------------------------------------------------------------
 Utf8String NumericFormatSpec::ShortToBinaryText(short int n, bool useSeparator)
 {
     char buf[64];
     FormatBinaryShort(n, buf, sizeof(buf), useSeparator);
     return Utf8String(buf);
 }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //---------------------------------------------------------------------------------------
 Utf8String NumericFormatSpec::IntToBinaryText(int n, bool useSeparator)
 {
     char buf[80];
     FormatBinaryInt(n, buf, sizeof(buf), useSeparator);
     return Utf8String(buf);
 }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //---------------------------------------------------------------------------------------
 Utf8String NumericFormatSpec::DoubleToBinaryText(double x, bool useSeparator)
 {
     char buf[80];
     FormatBinaryDouble(x, buf, sizeof(buf), useSeparator);
     return Utf8String(buf);
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
     if (m_barType != other.m_barType) return false;
     if (m_decimalSeparator != other.m_decimalSeparator) return false;
     if (m_thousandsSeparator != other.m_thousandsSeparator) return false;
     if (!m_uomSeparator.Equals(other.m_uomSeparator)) return false;
     if (m_statSeparator != other.m_statSeparator) return false;
     if (m_minWidth != other.m_minWidth) return false;

     return true;
     }


 //---------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 12/16
 //---------------------------------------------------------------------------------------
 NumericFormatSpecCP StdFormatSet::DefaultDecimal()
     {
     NamedFormatSpecCP fmtP;

     for (auto itr = Set()->m_formatSet.begin(); itr != Set()->m_formatSet.end(); ++itr)
         {
         fmtP = *itr;
         if (PresentationType::Decimal == fmtP->GetPresentationType())
             return fmtP->GetNumericSpec();
         }
     return nullptr;
     }

//===================================================
//
// StdFormatSet Methods
//
//===================================================
//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
NumericFormatSpecCP StdFormatSet::AddFormat(Utf8CP name, NumericFormatSpecCR fmtP, CompositeValueSpecCR compS, Utf8CP alias)
    {
    if (IsFormatDefined(name, alias))
        {
        m_problem.UpdateProblemCode(FormatProblemCode::NFS_DuplicateSpecNameOrAlias);
        return nullptr;
        }
    NamedFormatSpecCP nfs = new NamedFormatSpec(name, fmtP, compS, alias);
    if (nullptr == nfs || nfs->IsProblem())
        {
        if (nullptr != nfs)
            delete nfs;
        return nullptr;
        }
    m_formatSet.push_back(nfs);
    return nfs->GetNumericSpec();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
NumericFormatSpecCP StdFormatSet::AddFormat(Utf8CP name, NumericFormatSpecCR fmtP, Utf8CP alias)
    {
    if (IsFormatDefined(name, alias))
        {
        m_problem.UpdateProblemCode(FormatProblemCode::NFS_DuplicateSpecNameOrAlias);
        return nullptr;
        }
    NamedFormatSpecP nfs = new NamedFormatSpec(name, fmtP, alias);
    if (nullptr == nfs || nfs->IsProblem())
        {
        if (nullptr != nfs)
            delete nfs;
        return nullptr;
        }
    m_formatSet.push_back(nfs);
    return nfs->GetNumericSpec();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
NumericFormatSpecCP StdFormatSet::AddFormat(Utf8CP jsonString)
    {
    NamedFormatSpecCP nfs = AddNamedFormat(jsonString);
    if (nullptr == nfs)
        return nullptr;
    return nfs->GetNumericSpec();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
NamedFormatSpecCP StdFormatSet::AddNamedFormat(Utf8CP jsonString)
    {
    Json::Value jval (Json::objectValue);
    Json::Reader::Parse(jsonString, jval);
    NamedFormatSpecP nfs = new NamedFormatSpec(jval, m_unitsRegistry);
    if (nullptr == nfs)
        return nullptr;
    Utf8String tval = jval.ToString();
    tval.empty();

    if (nfs->IsProblem())
        {
        delete nfs;
        return nullptr;
        }
    if (IsFormatDefined(nfs->GetName(), nfs->GetAlias()))
        {
        m_problem.UpdateProblemCode(FormatProblemCode::NFS_DuplicateSpecNameOrAlias);
        delete nfs;
        return nullptr;
        }
    m_formatSet.push_back(nfs);
    return nfs;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
// static
NamedFormatSpecCP StdFormatSet::AddFormat(Utf8CP jsonString, FormatProblemDetailR problem)
    {
    StdFormatSetP sp = Set();
    sp->ResetProblemCode();
    auto nfs = sp->AddNamedFormat(jsonString);
    problem.Reset();
    if (sp->HasProblem())
        problem.UpdateProblemCode(sp->GetProblemCode());

    return nfs;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
NumericFormatSpecCP StdFormatSet::GetNumericFormat(Utf8CP name)
    {
    NamedFormatSpecCP fmtP = FindFormatSpec(name);
    if(nullptr == fmtP)
        return nullptr;
    return fmtP->GetNumericSpec();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
NamedFormatSpecCP StdFormatSet::FindFormatSpec(Utf8CP name)
    {
    NamedFormatSpecCP fmtP = *Set()->m_formatSet.begin();
    if (Set()->m_formatSet.size() > 0)
        {
        for (auto itr = Set()->m_formatSet.begin(); itr != Set()->m_formatSet.end(); ++itr)
            {
            fmtP = *itr;
            if (fmtP->HasName(name) || fmtP->HasAlias(name))
                {
                return fmtP;
                }
            }
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
bool StdFormatSet::IsFormatDefined(Utf8CP name, Utf8CP alias)
    {
    bvector<NamedFormatSpecCP> const& fmtSet = Set()->m_formatSet;
    return fmtSet.end() != std::find_if(fmtSet.begin(), fmtSet.end(),
        [name, alias](NamedFormatSpecCP pNamedFmtSpec) -> bool
            {return pNamedFmtSpec->HasName(name) || pNamedFmtSpec->HasAlias(name) || pNamedFmtSpec->HasName(alias) || pNamedFmtSpec->HasAlias(alias);});
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
bvector<Utf8CP> StdFormatSet::StdFormatNames(bool useAlias)
    {
    bvector<Utf8CP> vec;
    NamedFormatSpecCP fmtP = *Set()->m_formatSet.begin();
    Utf8CP name;

    for (auto itr = Set()->m_formatSet.begin(); itr != Set()->m_formatSet.end(); ++itr)
        {
        fmtP = *itr;
        if (useAlias)
            name = fmtP->GetName();
        else
            name = fmtP->GetAlias();
        vec.push_back(name);
        }
    return vec;
    }

//bvector<Json::Value> StdFormatSet::ToJson()
//    {
//    bvector<Json::Value> vec;
//    NamedFormatSpecCP fmtP = *Set()->m_formatSet.begin();
//    for (auto itr = Set()->m_formatSet.begin(); itr != Set()->m_formatSet.end(); ++itr)
//        {
//        fmtP = *itr;
//        if (useAlias)
//            name = fmtP->GetName();
//        else
//            name = fmtP->GetAlias();
//        vec.push_back(name);
//        }
//    return vec;
//    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
Utf8String StdFormatSet::StdFormatNameList(bool useAlias)
    {
    Utf8String  txt;
    NamedFormatSpecCP fmtP = *Set()->m_formatSet.begin();
    Utf8CP name;
    int i = 0;
    for (auto itr = Set()->m_formatSet.begin(); itr != Set()->m_formatSet.end(); ++itr)
        {
        fmtP = *itr;
        if (useAlias)
            name = fmtP->GetName();
        else
            name = fmtP->GetAlias();
        if (i > 0)
            txt += " ";
        txt += name;
        i++;
        }
    return txt;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/18
//----------------------------------------------------------------------------------------
FormatUnitSetCP StdFormatSet::FindFUS(Utf8CP fusName) const
    {
    FormatUnitSetCP fusP;

    for (auto itr = Set()->m_fusSet.begin(); itr != Set()->m_fusSet.end(); ++itr)
        {
        fusP = *itr;
        if (BeStringUtilities::StricmpAscii(fusName, fusP->GetFusName()) == 0)
            return fusP;
        }
    return nullptr;
    }

bool StdFormatSet::HasDuplicate(Utf8CP fusName, FormatUnitSetCP * fusOut)
    {
    *fusOut = nullptr;

    if (Utils::IsNameNullOrEmpty(fusName))
        {
        m_problem.UpdateProblemCode(FormatProblemCode::SFS_InsertingNamelessFUS);
        return true;
        }
    FormatUnitSetCP fusP = FindFUS(fusName);
    if (nullptr == fusP) // the name is not used
        return false;

    *fusOut = fusP;
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/18
//----------------------------------------------------------------------------------------
// static
FormatUnitSetCP StdFormatSet::AddFUS(FormatUnitSetCR fusR, Utf8CP fusName)
    {
    FormatUnitSetCP fusP;
    if (Set()->HasDuplicate(fusName, &fusP))
        return nullptr;

    fusP = new FormatUnitSet(fusR); // make a clone
    fusP->SetFusName(fusName);
    Set()->m_fusSet.push_back(fusP);
    return Set()->m_fusSet.back();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/18
//----------------------------------------------------------------------------------------
// static
FormatUnitSetCP StdFormatSet::AddFUS(Utf8CP formatName, BEU::UnitCP unit, Utf8CP fusName)
    {
    FormatUnitSetCP fusP;
    if (Set()->HasDuplicate(fusName, &fusP))
        return nullptr;

    NamedFormatSpecCP format = StdFormatSet::FindFormatSpec(formatName);
    fusP = new FormatUnitSet(format, unit);
    if (fusP->HasProblem())
        {
        Set()->m_problem.UpdateProblemCode(FormatProblemCode::SFS_FailedToMakeFUS);
        return nullptr;
        }
    fusP->SetFusName(fusName);
    Set()->m_fusSet.push_back(fusP);
    return Set()->m_fusSet.back();
    }

FormatUnitSetCP StdFormatSet::AddFUS(Utf8CP descriptor, Utf8CP fusName)
    {
    FormatUnitSetCP fusP;
    if (Set()->HasDuplicate(fusName, &fusP))
        return nullptr;
    fusP = new FormatUnitSet(descriptor, &BEU::UnitRegistry::Get()); // make FUS
    if (fusP->HasProblem())
        {
        Set()->m_problem.UpdateProblemCode(FormatProblemCode::SFS_FailedToMakeFUS);
        return nullptr;
        }
    fusP->SetFusName(fusName);
    Set()->m_fusSet.push_back(fusP);
    return Set()->m_fusSet.back();
    }

FormatUnitSetCP StdFormatSet::LookupFUS(Utf8CP fusName)
    {
    FormatUnitSetCP fusP = Set()->FindFUS(fusName);
    return fusP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void FormattingToken::Init()
    {
    m_cursorStart = (nullptr == m_cursor) ? 0 : m_cursor->GetCurrentPosition();
    m_word.clear();
    m_delim.clear();
    m_tokenLength = 0;
    m_tokenBytes = 0;
    m_isASCII = true;
    }

END_BENTLEY_FORMATTING_NAMESPACE
