/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/Formatting.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>

BEGIN_BENTLEY_FORMATTING_NAMESPACE


//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------


//DecimalPrecision NumericFormat::DecimalPrecisionByIndex(size_t num)
//    {
//    switch (num)
//        {
//        case 1: return DecimalPrecision::Precision1;
//        case 2: return DecimalPrecision::Precision2;        
//        case 3: return DecimalPrecision::Precision3;        
//        case 4: return DecimalPrecision::Precision4;        
//        case 5: return DecimalPrecision::Precision5;
//        case 6: return DecimalPrecision::Precision6;
//        case 7: return DecimalPrecision::Precision7;        
//        case 8: return DecimalPrecision::Precision8;        
//        case 9: return DecimalPrecision::Precision9;        
//        case 10: return DecimalPrecision::Precision10;
//        case 11: return DecimalPrecision::Precision11;
//        case 12: return DecimalPrecision::Precision12;
//        default: return DecimalPrecision::Precision0;
//        }
//    }


//FractionalNumeric::FractionalNumeric(double dval, FractionalPrecision fprec)
//    {
//    m_denominator = FractionalPrecisionDenominator(fprec);
//    double fract = modf(dval, &m_integral);
//    
//    }


//===================================================
//
// NumericFormatMethods
//
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//----------------------------------------------------------------------------------------
//void NumericFormatSpec::DefaultInit(Utf8CP name, size_t precision)
void NumericFormatSpec::DefaultInit(size_t precision)
    {
    //m_name = name;
    m_decPrecision = Utils::DecimalPrecisionByIndex(precision);
    //m_minTreshold = FormatConstant::FPV_MinTreshold();
    m_presentationType = FormatConstant::DefaultPresentaitonType();
    m_signOption = FormatConstant::DefaultSignOption();
    m_fractPrecision = FormatConstant::DefaultFractionalPrecision();
    m_decimalSeparator = FormatConstant::FPV_DecimalSeparator();
    m_thousandsSeparator = FormatConstant::FPV_ThousandSeparator();
    m_formatTraits = FormatConstant::DefaultFormatTraits();
    m_barType = FractionBarType::None;
    m_uomSeparator = FormatConstant::BlankString();
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//----------------------------------------------------------------------------------------
//void NumericFormatSpec::Init(Utf8CP name, PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, size_t precision)
void NumericFormatSpec::Init(PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, size_t precision)
    {
    //m_name = name;
    m_presentationType = presType;
    m_signOption = signOpt;
    m_formatTraits = formatTraits;
    if (PresentationType::Fractional == m_presentationType)
        {
        m_fractPrecision = Utils::FractionalPrecisionByDenominator(precision);
        m_decPrecision = FormatConstant::DefaultDecimalPrecision();
        m_barType = FractionBarType::Diagonal;
        }
    else
        {
        m_decPrecision = Utils::DecimalPrecisionByIndex(precision);
        m_fractPrecision = FormatConstant::DefaultFractionalPrecision();
        m_barType = FractionBarType::None;
        }
    //m_minTreshold = FormatConstant::FPV_MinTreshold();
    m_decimalSeparator = FormatConstant::FPV_DecimalSeparator();
    m_thousandsSeparator = FormatConstant::FPV_ThousandSeparator();
    m_roundFactor = 0.0;
    }

//void NumericFormatSpec::SetAlias(Utf8CP alias)
//    { 
//    m_alias = alias;
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
NumericFormatSpec::NumericFormatSpec(PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, size_t precision)
    {
    Init(presType, signOpt, formatTraits, precision);   
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 03/17
//----------------------------------------------------------------------------------------


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


////////            Helpers for NumericFormat
//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericFormatSpec::SetKeepTrailingZeroes(bool keep) 
    {
    size_t temp = static_cast<int>(m_formatTraits);

    if (keep)
        temp |= static_cast<int>(FormatTraits::TrailingZeroes);
    else
        temp &= ~static_cast<int>(FormatTraits::TrailingZeroes);
    m_formatTraits = static_cast<FormatTraits>(temp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericFormatSpec::SetUseLeadingZeroes(bool use)
    {
    size_t temp = static_cast<int>(m_formatTraits);
    if (use)
        temp |= static_cast<int>(FormatTraits::LeadingZeroes);
    else
        temp &= ~static_cast<int>(FormatTraits::LeadingZeroes);
    m_formatTraits = static_cast<FormatTraits>(temp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericFormatSpec::SetKeepDecimalPoint(bool keep)
    {
    size_t temp = static_cast<int>(m_formatTraits);
    if (keep)
        temp |= static_cast<int>(FormatTraits::KeepDecimalPoint);
    else
        temp &= ~static_cast<int>(FormatTraits::KeepDecimalPoint);
    m_formatTraits = static_cast<FormatTraits>(temp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericFormatSpec::SetKeepSingleZero(bool keep)
    {
    size_t temp = static_cast<int>(m_formatTraits);
    if (keep)
        temp |= static_cast<int>(FormatTraits::KeepSingleZero);
    else
        temp &= ~static_cast<int>(FormatTraits::KeepSingleZero);
    m_formatTraits = static_cast<FormatTraits>(temp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericFormatSpec::SetZeroEmpty(bool empty)
    {
    size_t temp = static_cast<int>(m_formatTraits);
    if (empty)
        temp |= static_cast<int>(FormatTraits::ZeroEmpty);
    else
        temp &= ~static_cast<int>(FormatTraits::ZeroEmpty);
    m_formatTraits = static_cast<FormatTraits>(temp);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
void NumericFormatSpec::SetUse1000Separator(bool use)
    {
    size_t temp = static_cast<int>(m_formatTraits);
    if (use)
        temp |= static_cast<int>(FormatTraits::Use1000Separator);
    else
        temp &= ~static_cast<int>(FormatTraits::Use1000Separator);
    m_formatTraits = static_cast<FormatTraits>(temp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
void NumericFormatSpec::SetApplyRounding(bool use)
    {
    size_t temp = static_cast<int>(m_formatTraits);
    if (use)
        temp |= static_cast<int>(FormatTraits::ApplyRounding);
    else
        temp &= ~static_cast<int>(FormatTraits::ApplyRounding);
    m_formatTraits = static_cast<FormatTraits>(temp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
void NumericFormatSpec::SetAppendUnit(bool use)
    {
    size_t temp = static_cast<int>(m_formatTraits);
    if (use)
        temp |= static_cast<int>(FormatTraits::AppendUnitName);
    else
        temp &= ~static_cast<int>(FormatTraits::AppendUnitName);
    m_formatTraits = static_cast<FormatTraits>(temp);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericFormatSpec::SetExponentZero(bool empty)
    {
    size_t temp = static_cast<int>(m_formatTraits);
    if (empty)
        temp |= static_cast<int>(FormatTraits::ExponentZero);
    else
        temp &= ~static_cast<int>(FormatTraits::ExponentZero);
    m_formatTraits = static_cast<FormatTraits>(temp);
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
int NumericFormatSpec::IntPartToText (double n, Utf8P bufOut, int bufLen, bool useSeparator) const
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
int NumericFormatSpec::FormatInteger (int n, Utf8P bufOut,  int bufLen)
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
    bool sci = (m_presentationType == PresentationType::Scientific || m_presentationType == PresentationType::ScientificNorm);
    bool decimal = (sci || m_presentationType == PresentationType::Decimal);
    bool fractional = (!decimal && m_presentationType == PresentationType::Fractional);

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
    if(nullptr == useUnit)
        return Utf8String(buf);
    Utf8String txt = Utils::AppendUnitName(buf, useUnit->GetName(), space);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 03/17
//---------------------------------------------------------------------------------------
//Utf8String NumericFormatSpec::StdFormatQuantity(Utf8CP stdName, BEU::QuantityCR qty, BEU::UnitCP useUnit, Utf8CP space, Utf8CP useLabel, int prec, double round)
//    {
//    NamedFormatSpecP namF = StdFormatSet::FindFormatSpec(stdName);
//    // there are two major options here: the format is a pure Numeric or it has a composite specification
//    NumericFormatSpecP fmtP = (nullptr == namF)? nullptr: namF->GetNumericSpec();
//    bool composite = (nullptr == namF) ? false : namF->HasComposite();
//    BEU::Quantity temp = qty.ConvertTo(useUnit); 
//    Utf8CP uomName = Utils::IsNameNullOrEmpty(useLabel)? ((nullptr == useUnit) ? qty.GetUnitName() : useUnit->GetName()): useLabel;
//    Utf8String majT, midT, minT, subT;
//
//
//    if (composite)  // procesing composite parts
//        {
//        CompositeValueSpecP compS = namF->GetCompositeSpec();
//        CompositeValue dval = compS->DecomposeValue(temp.GetMagnitude(),temp.GetUnit());
//        Utf8CP spacer = Utils::IsNameNullOrEmpty(space)? compS->GetSpacer().c_str() : space;
//        // for all parts but the last one we need to format an integer 
//        NumericFormatSpec fmtI = NumericFormatSpec(PresentationType::Decimal, FormatConstant::DefaultSignOption(), 
//                                                  FormatConstant::DefaultFormatTraits(), 0);
//        fmtI.SetKeepSingleZero(false);
//
//        switch (compS->GetType())
//            {
//            case CompositeSpecType::Single: // there is only one value to report
//                majT = fmtP->FormatDouble(dval.GetMajor(), prec, round);
//                majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel(nullptr).c_str(), spacer);
//                break;
//
//            case CompositeSpecType::Double: 
//                majT = fmtI.FormatDouble(dval.GetMajor(), prec, round);
//                majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel(nullptr).c_str(), spacer);
//                midT = fmtP->FormatDouble(dval.GetMiddle(), prec, round);
//                midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel(nullptr).c_str(), spacer);
//                majT += " " + midT;
//                break;
//
//            case CompositeSpecType::Triple:
//                majT = fmtI.FormatDouble(dval.GetMajor(), prec, round);
//                majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel(nullptr).c_str(), spacer);
//                midT = fmtI.FormatDouble(dval.GetMiddle(), prec, round);
//                midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel(nullptr).c_str(), spacer);
//                minT = fmtP->FormatDouble(dval.GetMinor(), prec, round);
//                minT = Utils::AppendUnitName(minT.c_str(), compS->GetMinorLabel(nullptr).c_str(), spacer);
//                majT += " " + midT + " " + minT;
//                break;
//
//            case CompositeSpecType::Quatro:
//                majT = fmtI.FormatDouble(dval.GetMajor(), prec, round);
//                majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel(nullptr).c_str(), spacer);
//                midT = fmtI.FormatDouble(dval.GetMiddle(), prec, round);
//                midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel(nullptr).c_str(), spacer);
//                minT = fmtI.FormatDouble(dval.GetMinor(), prec, round);
//                minT = Utils::AppendUnitName(minT.c_str(), compS->GetMinorLabel(nullptr).c_str(), spacer);
//                subT = fmtP->FormatDouble(dval.GetSub(), prec, round);
//                subT = Utils::AppendUnitName(subT.c_str(), compS->GetSubLabel(nullptr).c_str(), spacer);
//                majT += midT + " " + minT + " " + subT;
//                break;
//            }
//        }
//    else
//        {
//        if (nullptr == fmtP)  // invalid name
//            fmtP = StdFormatSet::DefaultDecimal();
//        if (nullptr == fmtP)
//            return "";
//        majT = fmtP->FormatDouble(temp.GetMagnitude(), prec, round);
//        majT = Utils::AppendUnitName(majT.c_str(), uomName, space);
//        /*if (nullptr != uomName)
//            {
//            if (!Utils::IsNameNullOrEmpty(space))
//                majT += Utf8String(space);
//            majT += Utf8String(uomName);
//            }*/
//        }
//    return majT;
//    }

Utf8String NumericFormatSpec::StdFormatQuantity(Utf8CP stdName, BEU::QuantityCR qty, BEU::UnitCP useUnit, Utf8CP space, Utf8CP useLabel, int prec, double round)
    {
    NamedFormatSpecCP namF = StdFormatSet::FindFormatSpec(stdName);
    return StdFormatQuantity(*namF, qty, useUnit, space, useLabel, prec, round);
    }

Utf8String NumericFormatSpec::StdFormatQuantity(NamedFormatSpecCR nfs, BEU::QuantityCR qty, BEU::UnitCP useUnit, Utf8CP space, Utf8CP useLabel, int prec, double round)
    {
    //NamedFormatSpecP namF = StdFormatSet::FindFormatSpec(stdName);
    // there are two major options here: the format is a pure Numeric or it has a composite specification
    NumericFormatSpecCP fmtP = nfs.GetNumericSpec();
    bool composite = nfs.HasComposite();
    BEU::Quantity temp = qty.ConvertTo(useUnit);
    Utf8CP uomName = Utils::IsNameNullOrEmpty(useLabel) ? ((nullptr == useUnit) ? qty.GetUnitName() : useUnit->GetName()) : useLabel;
    Utf8String majT, midT, minT, subT;


    if (composite)  // procesing composite parts
        {
        CompositeValueSpecP compS = (CompositeValueSpecP)nfs.GetCompositeSpec();
        CompositeValue dval = compS->DecomposeValue(temp.GetMagnitude(), temp.GetUnit());
        Utf8CP spacer = Utils::IsNameNullOrEmpty(space) ? compS->GetSpacer().c_str() : space;
        // for all parts but the last one we need to format an integer 
        NumericFormatSpec fmtI = NumericFormatSpec(PresentationType::Decimal, FormatConstant::DefaultSignOption(),
            FormatConstant::DefaultFormatTraits(), 0);
        fmtI.SetKeepSingleZero(false);

        switch (compS->GetType())
            {
            case CompositeSpecType::Single: // there is only one value to report
                majT = fmtP->FormatDouble(dval.GetMajor(), prec, round);
                majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel(nullptr).c_str(), spacer);
                break;

            case CompositeSpecType::Double:
                majT = fmtI.FormatDouble(dval.GetMajor(), prec, round);
                majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel(nullptr).c_str(), spacer);
                midT = fmtP->FormatDouble(dval.GetMiddle(), prec, round);
                midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel(nullptr).c_str(), spacer);
                majT += " " + midT;
                break;

            case CompositeSpecType::Triple:
                majT = fmtI.FormatDouble(dval.GetMajor(), prec, round);
                majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel(nullptr).c_str(), spacer);
                midT = fmtI.FormatDouble(dval.GetMiddle(), prec, round);
                midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel(nullptr).c_str(), spacer);
                minT = fmtP->FormatDouble(dval.GetMinor(), prec, round);
                minT = Utils::AppendUnitName(minT.c_str(), compS->GetMinorLabel(nullptr).c_str(), spacer);
                majT += " " + midT + " " + minT;
                break;

            case CompositeSpecType::Quatro:
                majT = fmtI.FormatDouble(dval.GetMajor(), prec, round);
                majT = Utils::AppendUnitName(majT.c_str(), compS->GetMajorLabel(nullptr).c_str(), spacer);
                midT = fmtI.FormatDouble(dval.GetMiddle(), prec, round);
                midT = Utils::AppendUnitName(midT.c_str(), compS->GetMiddleLabel(nullptr).c_str(), spacer);
                minT = fmtI.FormatDouble(dval.GetMinor(), prec, round);
                minT = Utils::AppendUnitName(minT.c_str(), compS->GetMinorLabel(nullptr).c_str(), spacer);
                subT = fmtP->FormatDouble(dval.GetSub(), prec, round);
                subT = Utils::AppendUnitName(subT.c_str(), compS->GetSubLabel(nullptr).c_str(), spacer);
                majT += midT + " " + minT + " " + subT;
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
           majT = Utils::AppendUnitName(majT.c_str(), uomName, space);
        /*if (nullptr != uomName)
        {
        if (!Utils::IsNameNullOrEmpty(space))
        majT += Utf8String(space);
        majT += Utf8String(uomName);
        }*/
        }
    return majT;
    }


//---------------------------------------------------------------------------------------
//  arg 'space' contains a separator between value and the unit name It also is an indicator
//   that the caller needs to append the unit name to the value
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
Utf8String NumericFormatSpec::StdFormatPhysValue(Utf8CP stdName, double dval, Utf8CP fromUOM, Utf8CP toUOM, Utf8CP toLabel, Utf8CP space, int prec, double round)
    {
    BEU::UnitCP fromUnit = BEU::UnitRegistry::Instance().LookupUnit(fromUOM);
    BEU::Quantity qty = BEU::Quantity(dval, *fromUnit);
    BEU::UnitCP toUnit = BEU::UnitRegistry::Instance().LookupUnit(toUOM);
     // UnitCP fromUnit = qty.GetUnit();
    BEU::PhenomenonCP phTo = toUnit->GetPhenomenon();
    BEU::PhenomenonCP phFrom = fromUnit->GetPhenomenon();
    if (phTo != phFrom)
        {
        Utf8String txt = "Impossible conversion from ";
        txt += fromUnit->GetName();
        txt += " to ";
        txt +=toUnit->GetName();
        return txt;
        }
    Utf8String str = StdFormatQuantity(stdName, qty, toUnit, space, toLabel, prec, round);
    /*if (nullptr != space)
        {
        str += space;
        if (nullptr == toLabel)
            str += toUnit->GetName();
        }
    if (nullptr != toLabel)
        str += toLabel;*/
    return str;
    }

const NumericFormatSpecCP NumericFormatSpec::DefaultFormat()
    {
    static NumericFormatSpec nfs = NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, FormatConstant::DefaultFormatTraits(), FormatConstant::DefaultDecimalPrecisionIndex());
    return &nfs;
    }




Utf8String NumericFormatSpec::StdFormatQuantityTriad(Utf8CP stdName, QuantityTriadSpecP qtr, Utf8CP space, int prec, double round)
    {
    NumericFormatSpecCP fmtP = StdFormatSet::GetNumericFormat(stdName);
    if (nullptr == fmtP)  // invalid name
        fmtP = StdFormatSet::DefaultDecimal();
    if (nullptr == fmtP)
        return "";
    return qtr->FormatQuantTriad(space, prec, fmtP->IsFractional(), qtr->GetIncludeZero());
    }
//ormatQuantTriad(Utf8CP space, int prec, bool fract=false, bool includeZero = false);

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
 int NumericFormatSpec::FormatBinaryShort (short int n, Utf8P bufOut, int bufLen, bool useSeparator)
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
 int NumericFormatSpec::FormatBinaryInt (int n, Utf8P bufOut, int bufLen, bool useSeparator)
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

 int NumericFormatSpec::FormatBinaryDouble (double x, Utf8P bufOut, int bufLen, bool useSeparator)
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
FormatParameterP FormatDictionary::GetParameterByIndex(int index)
    {
    FormatParameterP par = nullptr;
    if (0 <= index && (int)GetCount() > index)
        {
        par = &m_paramList[index];
        }
    return par;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
FormatParameterP FormatDictionary::FindParameterByName(Utf8StringCR paramName)
    {
    for (FormatParameterP par = m_paramList.begin(), end = m_paramList.end(); par != end; ++par)
        {
        if (0 == par->CompareName(paramName))
            return par;
        }
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
FormatParameterP FormatDictionary::FindParameterByCode(ParameterCode paramCode)
    {
    for (FormatParameterP par = m_paramList.begin(), end = m_paramList.end(); par != end; ++par)
        {
        if (paramCode == par->GetParameterCode())
            return par;
        }
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
Utf8StringCR FormatDictionary::CodeToName(ParameterCode paramCode)
    {
    FormatParameterP par = FindParameterByCode(paramCode);
    return (nullptr == par)? static_cast<Utf8StringCR>(nullptr) : par->GetName();
    }

Utf8StringP FormatDictionary::ParameterValuePair(Utf8StringCR name, Utf8StringCR value, char quote, Utf8StringCR prefix)
    {
    Utf8StringP str = new Utf8String();
    if (!name.empty())
        {
        str->append( prefix + name);
        if (!value.empty())
            {
            str->push_back('=');
            if (0 != quote)
                str->push_back(quote);
            str->append(value);
            if (0 != quote)
                str->push_back(quote);
            }
        }
    return str;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
Utf8String FormatDictionary::SerializeFormatDefinition(NamedFormatSpecCP namedFormat)
    {
    Utf8String str;

    //str.append(*ParameterValuePair(FormatConstant::FPN_FormatName(), namedFormat.GetNameAndAlias(), '\"', ""));
    // Names section
    str.append(*ParameterValuePair(FormatConstant::FPN_Name(), namedFormat->GetName(), '\"', ""));
    str.append(" " + *ParameterValuePair(FormatConstant::FPN_Alias(), namedFormat->GetAlias(), '\"', ""));
    NumericFormatSpecCP format = namedFormat->GetNumericSpec();
    // formating type/mode
    str.append(" " + Utils::PresentationTypeName(format->GetPresentationType())); // Decimal, Fractional, Sientific, ScientificNorm
    // precision
    if (format->IsFractional())
        {
        str.append(" " + Utils::FractionallPrecisionName(format->GetFractionalPrecision()));
        if (FractionBarType::None != format->GetFractionalBarType())
            str.append(" " + Utils::FractionBarName(format->GetFractionalBarType()));
        }
    else
        str.append(" " + Utils::DecimalPrecisionName(format->GetDecimalPrecision()));
    // sign options
    str.append(" " + Utils::SignOptionName(format->GetSignOption()));  // NoSign, OnlyNegative, SignAlways, NegativeParenths
    // zero options
    if (format->IsKeepTrailingZeroes()) str.append(" " + FormatConstant::FPN_TrailingZeroes());
    if (format->IsUseLeadingZeroes()) str.append(" " + FormatConstant::FPN_LeadingZeroes());
    if (format->IsKeepSingleZero()) str.append(" " + FormatConstant::FPN_KeepSingleZero());
    if (format->IsKeepDecimalPoint()) str.append(" " + FormatConstant::FPN_KeepDecimalPoint());
    if (format->IsZeroEmpty()) str.append(" " + FormatConstant::FPN_ZeroEmpty());
    if (format->IsScientific())
        {
        if (format->IsExponentZero()) str.append(" " + FormatConstant::FPN_ExponentZero());
        }
    //  separators section
    if(format->IsUse1000Separator()) str.append(" " + FormatConstant::FPN_Use1000Separ());
    Utf8Char symb = format->GetThousandSeparator();
    if(symb != FormatConstant::FPV_ThousandSeparator()) 
        str.append(*ParameterValuePair(FormatConstant::FPN_ThousandsSepar(), Utf8String(symb, 1), '\'', ""));
    symb = format->GetDecimalSeparator();
    if (symb != FormatConstant::FPV_DecimalSeparator())
        str.append(*ParameterValuePair(FormatConstant::FPN_DecimalSepar(), Utf8String(symb, 1), '\'', ""));
    return str;
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
    NamedFormatSpecCP nfs = new NamedFormatSpec(name, fmtP, compS, alias);
    m_formatSet.push_back(nfs);
    return nfs->GetNumericSpec();
    }

NumericFormatSpecCP StdFormatSet::AddFormat(Utf8CP name, NumericFormatSpecCR fmtP, Utf8CP alias)
    {
    NamedFormatSpecP nfs = new NamedFormatSpec(name, fmtP, alias);
    m_formatSet.push_back(nfs);
    return nfs->GetNumericSpec();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
void StdFormatSet::StdInit()
    {
    FormatTraits traits = FormatConstant::DefaultFormatTraits();
    FormatTraits traitsU = FormatConstant::UnitizedFormatTraits();
    //AddFormat("DefaultReal", new NumericFormatSpec( PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()), "real");
    AddFormat(FormatConstant::DefaultFormatName(), new NumericFormatSpec(NumericFormatSpec::DefaultFormat()), FormatConstant::DefaultFormatAlias());
    AddFormat("DefaultRealU", new NumericFormatSpec( PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, FormatConstant::DefaultDecimalPrecisionIndex()), "realu");
    AddFormat("Real2",       new NumericFormatSpec( PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, 2),"real2");
    AddFormat("Real3",       new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, 3),"real3");
    AddFormat("Real4",       new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, 4),"real4");

    AddFormat("Real2U", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 2), "real2u");
    AddFormat("Real3U", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 3), "real3u");
    AddFormat("Real4U", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 4), "real4u");
    AddFormat("Real6U", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, traitsU, 6), "real6u");

    AddFormat("SignedReal",  new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()),"realSign");
    AddFormat("ParenthsReal", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::NegativeParentheses, traits, FormatConstant::DefaultDecimalPrecisionIndex()),"realPth");
    AddFormat("DefaultFractional", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultFractionalDenominator()), "fract"); 
    AddFormat("DefaultFractionalU", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traitsU, FormatConstant::DefaultFractionalDenominator()),"fractu");
    AddFormat("SignedFractional", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::SignAlways, traits, FormatConstant::DefaultFractionalDenominator()),"fractSign");
    AddFormat("DefaultExp", new NumericFormatSpec(PresentationType::Scientific, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()),"sci");
    AddFormat("SignedExp", new NumericFormatSpec(PresentationType::Scientific, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()),"sciSign");
    AddFormat("NormalizedExp", new NumericFormatSpec(PresentationType::ScientificNorm, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()),"sciN");
    AddFormat("DefaultInt", new NumericFormatSpec(PresentationType::Decimal, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()),"int");
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
    AddFormat("AngleDMS", new NumericFormatSpec(PresentationType::Fractional, ShowSignOption::OnlyNegative, traits,0), cvs, "dms");
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
NumericFormatSpecCP StdFormatSet::DefaultDecimal()
    {
    NamedFormatSpecCP fmtP;

    for (auto itr = Set().m_formatSet.begin(); itr != Set().m_formatSet.end(); ++itr)
        {
        fmtP = *itr;
        if (PresentationType::Decimal == fmtP->GetPresentationType())
            return fmtP->GetNumericSpec();
        }
    return nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
NumericFormatSpecCP StdFormatSet::GetNumericFormat(Utf8CP name)
    {
    NamedFormatSpecCP fmtP = *Set().m_formatSet.begin();
    for (auto itr = Set().m_formatSet.begin(); itr != Set().m_formatSet.end(); ++itr)
        {
        fmtP = *itr;
        if (fmtP->HasName(name) || fmtP->HasAlias(name))
            {
            return fmtP->GetNumericSpec();
            }
        }
    return nullptr;
    }

NamedFormatSpecCP StdFormatSet::FindFormatSpec(Utf8CP name)
    {
    NamedFormatSpecCP fmtP = *Set().m_formatSet.begin();

    for (auto itr = Set().m_formatSet.begin(); itr != Set().m_formatSet.end(); ++itr)
        {
        fmtP = *itr;
        if (fmtP->HasName(name) || fmtP->HasAlias(name))
            {
            return fmtP;
            }
        }
    return nullptr;
    }

bvector<Utf8CP> StdFormatSet::StdFormatNames(bool useAlias)
    {
    bvector<Utf8CP> vec;
    NamedFormatSpecCP fmtP = *Set().m_formatSet.begin();
    Utf8CP name;

    for (auto itr = Set().m_formatSet.begin(); itr != Set().m_formatSet.end(); ++itr)
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

Utf8String StdFormatSet::StdFormatNameList(bool useAlias)
    {
    Utf8String  txt;
    NamedFormatSpecCP fmtP = *Set().m_formatSet.begin();
    Utf8CP name;
    int i = 0;
    for (auto itr = Set().m_formatSet.begin(); itr != Set().m_formatSet.end(); ++itr)
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



void FormattingToken::Init()
    {
    m_cursorStart = (nullptr == m_cursor) ? 0 : m_cursor->GetCurrentPosition();
    m_word.clear();
    m_delim.clear();
    m_tokenLength = 0;
    m_tokenBytes = 0;
    m_isASCII = true;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
//FormattingToken::FormattingToken(FormattingScannerCursorP cursor)
//    {
//    m_cursor = cursor;
//    Init();
//    }
//
//WString FormattingToken::GetNextToken()
//    {
//    /*bvector<size_t> m_word;
//    bvector<size_t> m_delim;*/
//    
//    Init();
//    //size_t skip = m_cursor->SkipBlanks();
//    size_t code = m_cursor->GetNextSymbol();
//    while (!m_cursor->IsDivider())
//        {
//        if (!m_cursor->IsASCII())
//            m_isASCII = false;
//        m_word.push_back(code);
//        code = m_cursor->GetNextSymbol();
//        }
//    m_delim.push_back(code);
//    WString ws;
//    return ws;
//    }

//CharCP FormattingToken::GetASCII()
//    {
//    if()
//    }

END_BENTLEY_FORMATTING_NAMESPACE
