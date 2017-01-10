/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/Formatting.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/Formatting.h>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//static int FractionalPrecisionDenominator(FractionalPrecision prec)
//    {
//    switch (prec)
//        {
//        case FractionalPrecision::Half: return 2;
//        case FractionalPrecision::Quarter: return 4;
//        case FractionalPrecision::Eighth: return 8;
//        case FractionalPrecision::Sixteenth: 16;
//        case FractionalPrecision::Over_32: 32;
//        case FractionalPrecision::Over_64: 64;
//        case FractionalPrecision::Over_128: 128;
//        case FractionalPrecision::Over_256: 256;
//        default:
//        case FractionalPrecision::Whole: return 1;
//        }
//    }
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
void NumericFormat::DefaultInit(Utf8CP name, size_t precision)
    {
    m_name = name;
    m_decPrecision = Utils::DecimalPrecisionByIndex(precision);
    m_minTreshold = FormatConstant::FPV_MinTreshold();
    m_presentationType = FormatConstant::DefaultPresentaitonType();
    m_signOption = FormatConstant::DefaultSignOption();
    m_fractPrecision = FormatConstant::DefaultFractionalPrecision();
    m_decimalSeparator = FormatConstant::FPV_DecimalSeparator();
    m_thousandsSeparator = FormatConstant::FPV_ThousandSeparator();
    m_formatTraits = FormatConstant::DefaultFormatTraits();
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//----------------------------------------------------------------------------------------
void NumericFormat::Init(Utf8CP name, PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, size_t precision)
    {
    m_name = name;
    m_presentationType = presType;
    m_signOption = signOpt;
    m_formatTraits = formatTraits;
    if (PresentationType::Fractional == m_presentationType)
        {
        m_fractPrecision = Utils::FractionalPrecisionByDenominator(precision);
        m_decPrecision = FormatConstant::DefaultDecimalPrecision();
        }
    else
        {
        m_decPrecision = Utils::DecimalPrecisionByIndex(precision);
        m_fractPrecision = FormatConstant::DefaultFractionalPrecision();
        }
    m_minTreshold = FormatConstant::FPV_MinTreshold();
    m_decimalSeparator = FormatConstant::FPV_DecimalSeparator();
    m_thousandsSeparator = FormatConstant::FPV_ThousandSeparator();
    m_roundFactor = 0.0;
    }

void NumericFormat::SetAlias(Utf8CP alias)
    { 
    m_alias = alias;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//----------------------------------------------------------------------------------------
double NumericFormat::RoundDouble(double dval, double roundTo)
    {
    if (IsNegligible(roundTo))
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
bool NumericFormat::AcceptableDifference(double dval1, double dval2, double maxDiff) 
    { 
    return (fabs(maxDiff) > fabs(dval1 - dval2));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//----------------------------------------------------------------------------------------
NumericFormat::NumericFormat(NumericFormatCR other)
    {
    m_name = other.m_name;
    m_alias = other.m_alias;
    m_decPrecision = other.m_decPrecision;
    m_minTreshold = FormatConstant::FPV_MinTreshold();
    m_presentationType = other.m_presentationType;
    m_signOption = other.m_signOption;
    m_fractPrecision = other.m_fractPrecision;
    m_decimalSeparator = other.m_decimalSeparator;
    m_thousandsSeparator = other.m_thousandsSeparator;
    m_formatTraits = other.m_formatTraits;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//----------------------------------------------------------------------------------------
NumericFormat::NumericFormat(Utf8CP name, PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, const size_t precision)
    {
    Init(name, presType, signOpt, formatTraits, precision);   
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//----------------------------------------------------------------------------------------
// Note: if a positive value of the argument "round" is provided on the argument list it signals 
//   that the additional rounding of the double value will be applied at the time of formatting
NumericFormat::NumericFormat(StdFormatNameR fmtType, size_t precision, double round)
    {
    ShowSignOption signOpt = FormatConstant::DefaultSignOption();
    FormatTraits traits = FormatConstant::DefaultFormatTraits();
    switch (fmtType.GetCode())
        {
        case static_cast<int>(StdFormatCode::SignedReal) :
            Init(fmtType.GetShortName(), PresentationType::Decimal, ShowSignOption::SignAlways, traits, precision);
            break;
        case static_cast<int>(StdFormatCode::ParenthsReal) :
            Init(fmtType.GetShortName(), PresentationType::Decimal, ShowSignOption::NegativeParentheses, traits, precision);
            break;
        case static_cast<int>(StdFormatCode::DefaultFractional) :
            Init(fmtType.GetShortName(), PresentationType::Fractional, signOpt, traits, precision);
            break;
        case static_cast<int>(StdFormatCode::SignedFractional) :
           Init(fmtType.GetShortName(), PresentationType::Decimal, ShowSignOption::SignAlways, traits, precision);
            break;
        case static_cast<int>(StdFormatCode::DefaultExp) :
            Init(fmtType.GetShortName(), PresentationType::Scientific, signOpt, traits, precision);
            break;
        case static_cast<int>(StdFormatCode::NormalExp) :
            Init(fmtType.GetShortName(), PresentationType::ScientificNorm, signOpt, traits, precision);
            break;
        case static_cast<int>(StdFormatCode::SignedExp) :
            Init(fmtType.GetShortName(), PresentationType::Scientific, ShowSignOption::SignAlways, traits, precision);
            break;

        case static_cast<int>(StdFormatCode::DefaultReal) :
        case static_cast<int>(StdFormatCode::DefaultInt) :
        default:
            Init(fmtType.GetShortName(), PresentationType::Decimal, signOpt, traits, precision);
            break;
        }
    if (round >= 0.0)
        {
        m_roundFactor = round;
        SetApplyRounding(true);
        }
    }

//NumericFormat NumericFormat::StdNumericFormat(Utf8P stdName, int prec)
//    {
//
//    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//----------------------------------------------------------------------------------------
void NumericFormat::SetPrecisionByValue(int prec)
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

//NumericFormat NumericFormat::StdNumericFormat(Utf8P stdName, int prec)
//    {
//
//    }


////////            Helpers for NumericFormat
//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericFormat::SetKeepTrailingZeroes(bool keep)
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
void NumericFormat::SetUseLeadingZeroes(bool use)
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
void NumericFormat::SetKeepDecimalPoint(bool keep)
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
void NumericFormat::SetKeepSingleZero(bool keep)
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
void NumericFormat::SetZeroEmpty(bool empty)
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
void NumericFormat::SetUse1000Separator(bool use)
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
void NumericFormat::SetApplyRounding(bool use)
    {
    size_t temp = static_cast<int>(m_formatTraits);
    if (use)
        temp |= static_cast<int>(FormatTraits::ApplyRounding);
    else
        temp &= ~static_cast<int>(FormatTraits::ApplyRounding);
    m_formatTraits = static_cast<FormatTraits>(temp);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericFormat::SetExponentZero(bool empty)
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
double NumericFormat::RoundedValue(double dval, double round)
    {
    round = fabs(round);
    if (round < m_minTreshold)
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
int NumericFormat::RightAlignedCopy(CharP dest, int destLen, bool termZero, CharCP src, int srcLen)
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
int NumericFormat::IntPartToText (double n, CharP bufOut, int bufLen, bool useSeparator)
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
int NumericFormat::FormatInteger (int n, CharP bufOut,  int bufLen)
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
int NumericFormat::FormatIntegerSimple (int n, CharP bufOut, int bufLen, bool showSign, bool extraZero)
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
int NumericFormat::TrimTrailingZeroes(CharP buf, int index)
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

int NumericFormat::InsertChar(CharP buf, int index, char c, int num)
    {
    if (nullptr != buf && 0 < num)
        {
        for(int i = 0; i < num; buf[index++] = c, i++){}
        }
    return index;
    }


double NumericFormat::GetDecimalPrecisionFactor(int prec = -1) 
    { 
    return Utils::DecimalPrecisionFactor(m_decPrecision, prec); 
    }

int  NumericFormat::GetDecimalPrecisionIndex(int prec = -1) 
    { 
    if (0 <= prec && prec < Utils::DecimalPrecisionToInt(DecimalPrecision::Precision12))
        return prec;
    return Utils::DecimalPrecisionToInt(m_decPrecision); 
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
int NumericFormat::FormatDouble(double dval, CharP buf, int bufLen, int prec, double round)
    {
    double ival;
    Utf8Char sign = '+';
    double precScale = NumericFormat::GetDecimalPrecisionFactor(prec);
    int totFractLen = NumericFormat::GetDecimalPrecisionIndex(prec);
    double expInt = 0.0;
    double fract;
    char intBuf[64];
    char fractBuf[64];
    char locBuf[128];
    int ind = 0;

    if (dval < 0.0)
        {
        dval = -dval;
        sign = (m_signOption == ShowSignOption::NegativeParentheses) ? '(' : '-';
        }
    bool sci = (m_presentationType == PresentationType::Scientific || m_presentationType == PresentationType::ScientificNorm);
    bool decimal = (sci || m_presentationType == PresentationType::Decimal);

    if (IsApplyRounding() || !IsIgnored(round))
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
        memset(locBuf, 0, sizeof(locBuf));

        fract = modf(IsPrecisionZero()? dval + FormatConstant::FPV_RoundFactor(): dval, &ival);
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
            if (fract < 0.0)
                fract = -fract;
            fract = fract * precScale + 0.501;
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
                ind = TrimTrailingZeroes(locBuf, ind-1);
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
        } // decimal
    // closing formatting
    if ('(' == sign)
        locBuf[ind++] = ')';
    locBuf[ind++] = '\0';

    if (ind > bufLen)
        ind = bufLen;
    memcpy(buf, locBuf, ind);
    return ind;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
Utf8String NumericFormat::FormatDouble(double dval, int prec, double round)
    {
    char buf[64];
    FormatDouble(dval, buf, sizeof(buf), prec, round);
    return Utf8String(buf);
    }

Utf8String NumericFormat::FormatRoundedDouble(double dval, double round)
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

Utf8String NumericFormat::RefFormatDouble(double dval, Utf8P stdName, int prec, double round)
    {
    NumericFormatP fmtP = StdFormatSet::FindFormat(stdName);
    if (nullptr == fmtP)  // invalid name
        fmtP = StdFormatSet::DefaultDecimal();
    if (nullptr == fmtP)
        return "";
    return fmtP->FormatDouble(dval, prec, round);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
 Utf8String NumericFormat::FormatInteger(int ival)
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
 int NumericFormat::FormatBinaryByte (unsigned char n, CharP bufOut, int bufLen)
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
 int NumericFormat::FormatBinaryShort (short int n, CharP bufOut, int bufLen, bool useSeparator)
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
 int NumericFormat::FormatBinaryInt (int n, CharP bufOut, int bufLen, bool useSeparator)
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

 int NumericFormat::FormatBinaryDouble (double x, CharP bufOut, int bufLen, bool useSeparator)
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

 Utf8String NumericFormat::ByteToBinaryText(unsigned char n)
 {
     char buf[64];
     FormatBinaryByte(n, buf, sizeof(buf));
     return Utf8String(buf);
 }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //---------------------------------------------------------------------------------------
 Utf8String NumericFormat::ShortToBinaryText(short int n, bool useSeparator)
 {
     char buf[64];
     FormatBinaryShort(n, buf, sizeof(buf), useSeparator);
     return Utf8String(buf);
 }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //---------------------------------------------------------------------------------------
 Utf8String NumericFormat::IntToBinaryText(int n, bool useSeparator)
 {
     char buf[80];
     FormatBinaryInt(n, buf, sizeof(buf), useSeparator);
     return Utf8String(buf);
 }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //---------------------------------------------------------------------------------------
 Utf8String NumericFormat::DoubleToBinaryText(double x, bool useSeparator)
 {
     char buf[80];
     FormatBinaryDouble(x, buf, sizeof(buf), useSeparator);
     return Utf8String(buf);
 }


//===================================================
//
// FormatDictionaryMethods
//
//===================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
 const bool FormatConstant::IsLittleEndian()
     {
     union { short int s; char b[4]; } un;
     un.s = 1;
     return (un.b[0] == (char)1);
     }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 12/16
 //---------------------------------------------------------------------------------------
const size_t FormatConstant::GetSequenceLength(unsigned char c)
     {
     if (0 == (c & UTF_TrailingByteMark())) // ASCII - single byte
         return 1;
     if ((c & UTF_2ByteMask()) == UTF_2ByteMark())
         return 2;
     if ((c & UTF_3ByteMask()) == UTF_3ByteMark())
         return 3;
     if ((c & UTF_4ByteMask()) == UTF_4ByteMark())
         return 4;
     return 0;
     }

// the trailing byte should be properly marked for being processed. It always contains only
//  6 bits that should be shifted accordingly to the location of the byte in the sequence
//  there are only 3 possible numbers of bytes in sequences: 2, 3 and 4. Accordingly the 
//  the range of indexes is striclty governed by the sequence lenght. The minimum index
//  value is always 1 and the maximum is N-1 where N is the sequence length
//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
bool FormatConstant::GetCodeBits(unsigned char c, size_t seqLength, size_t index, size_t* outBits)
    {
    if (nullptr != outBits)
        {
        // calculate the shift 
        *outBits = 0;
        int shift = ((int)seqLength - (int)index - 1);
        if (0 > shift || 2 < shift)
            return false;
        if (UTF_TrailingByteMark() == (c & UTF_TrailingByteMask()))
            {
            size_t temp = c & ~UTF_TrailingByteMask();
            temp <<= shift * UTF_UpperBitShift();
            *outBits = temp;
            }
        return true;
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
bool FormatConstant::GetTrailingBits(unsigned char c, CharP outBits)
    {
    if (nullptr != outBits)
        {
        *outBits = 0;
        if (UTF_TrailingByteMark() == (c & UTF_TrailingByteMask()))
            *outBits = c & ~UTF_TrailingByteMask();
        return true;
        }
    return false;
    }

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
    FormatParameterP par;
    for (auto curr = m_paramList.begin(), end = m_paramList.end(); curr != end; curr++)
        {
        par = curr;
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
    FormatParameterP par;
        for (auto curr = m_paramList.begin(), end = m_paramList.end(); curr != end; curr++)
        {
        par = curr;
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

Utf8StringP FormatDictionary::SerializeFormatDefinition(NumericFormat format)
    {
    Utf8StringP str = new Utf8String();

    str->append(*ParameterValuePair(FormatConstant::FPN_FormatName(), format.GetName(), '\"', ""));
    str->append(" " + Utils::PresentationTypeName(format.GetPresentationType())); // Decimal, Fractional, Sientific, ScientificNorm
    str->append(" " + Utils::SignOptionName(format.GetSignOption()));             // NoSign, OnlyNegative, SignAlways, NegativeParenths
    str->append(" " + Utils::DecimalPrecisionName(format.GetDecimalPrecision()));
    str->append(" " + Utils::FractionallPrecisionName(format.GetFractionalPrecision()));
    if (format.IsKeepTrailingZeroes())
        str->append(" " + FormatConstant::FPN_TrailingZeroes());
    if (format.IsUseLeadingZeroes())
        str->append(" " + FormatConstant::FPN_LeadingZeroes());
    if (format.IsKeepDecimalPoint())
        str->append(" " + FormatConstant::FPN_KeepDecimalPoint());
    if (format.IsKeepSingleZero())
        str->append(" " + FormatConstant::FPN_KeepSingleZero());
    if(format.IsUse1000Separator())
        str->append(" " + FormatConstant::FPN_Use1000Separ());
    Utf8Char symb = format.GetThousandSeparator();
    if(symb != FormatConstant::FPV_ThousandSeparator()) 
        str->append(*ParameterValuePair(FormatConstant::FPN_ThousandsSepar(), Utf8String(symb, 1), '\'', ""));
    symb = format.GetDecimalSeparator();
    if (symb != FormatConstant::FPV_DecimalSeparator())
        str->append(*ParameterValuePair(FormatConstant::FPN_DecimalSepar(), Utf8String(symb, 1), '\'', ""));
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
NumericFormatP StdFormatSet::AddFormat(NumericFormatP fmtP)
    {
    m_formatSet.push_back(fmtP);
    return fmtP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
void StdFormatSet::StdInit()
    {
    FormatTraits traits = FormatConstant::DefaultFormatTraits();
    AddFormat(new NumericFormat("DefaultReal", PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()))->SetAlias("real");
    AddFormat(new NumericFormat("SignedReal", PresentationType::Decimal, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()))->SetAlias("realSign");
    AddFormat(new NumericFormat("ParenthsReal", PresentationType::Decimal, ShowSignOption::NegativeParentheses, traits, FormatConstant::DefaultDecimalPrecisionIndex()))->SetAlias("realPth");
    AddFormat(new NumericFormat("DefaultFractional", PresentationType::Decimal, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()))->SetAlias("fract");
    AddFormat(new NumericFormat("SignedFractional", PresentationType::Decimal, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()))->SetAlias("fractSign");
    AddFormat(new NumericFormat("DefaultExp", PresentationType::Scientific, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()))->SetAlias("sci");
    AddFormat(new NumericFormat("SignedExp", PresentationType::Scientific, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()))->SetAlias("sciSign");
    AddFormat(new NumericFormat("NormalizedExp", PresentationType::ScientificNorm, ShowSignOption::OnlyNegative, traits, FormatConstant::DefaultDecimalPrecisionIndex()))->SetAlias("sciN");
    AddFormat(new NumericFormat("DefaultInt", PresentationType::Decimal, ShowSignOption::SignAlways, traits, FormatConstant::DefaultDecimalPrecisionIndex()))->SetAlias("int");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 12/16
//---------------------------------------------------------------------------------------
NumericFormatP StdFormatSet::DefaultDecimal()
    {
    NumericFormatP fmtP;

    for (auto itr = Set().m_formatSet.begin(); itr != Set().m_formatSet.end(); itr++)
        {
        fmtP = *itr;
        if (PresentationType::Decimal == fmtP->GetPresentationType())
            return fmtP;
        }
    return nullptr;
    }

//===================================================
//
// NumericTriadMethods
//
//===================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericTriad::Convert()
    {
    if (!m_init)
        return;
    m_midAssigned = false;
    m_lowAssigned = false;

    double topmid = (double)m_topToMid;
    double midlow = (double)m_midToLow;
    double toplow = topmid * midlow;
    double rem = 0.0;
    if (m_decPrecision == DecimalPrecision::Precision0)
        m_dval = floor(m_dval + 0.501);
    m_topValue = 0.0;
    m_midValue = 0.0;
    m_lowValue = 0.0;
    int convType = 0;
    if (m_topToMid > 1)
        convType |= 0x1;
    if (m_midToLow > 1)
        convType |= 0x2;
    // there are only three allowed combinations of the factors:
    //  0 - when topMid < 1  top value is set to the initial value regardless of midToLow factor value
    //  1 - when topMid > 1 and midlow < 1 only top and middle values will be calculated
    //  3 - when both factors are > 1

    switch (convType)
        {
        case 1:
            m_topValue = floor(m_dval / topmid);
            m_midValue = m_dval - m_topValue * topmid;
            m_midAssigned = true;
            break;

        case 3:
            m_topValue = floor(m_dval / toplow);
            rem = m_dval - m_topValue * toplow;
            m_midValue = floor(rem / midlow);
            m_lowValue = rem - m_midValue * midlow;
            m_midAssigned = true;
            m_lowAssigned = true;
            break;

        default:
            m_topValue = GetWhole();
            break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericTriad::SetValue(double dval, DecimalPrecision prec)
    {
    m_dval = dval;
    m_negative = false;
    if (m_dval < 0.0)
        {
        m_negative = true;
        m_dval = -m_dval;
        }
    m_decPrecision = prec;
    }

//---------------------------------------------------------------------------------------
// Constructor
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
NumericTriad::NumericTriad()
    {
    m_dval = 0.0;
    m_topValue = 0.0;
    m_midValue = 0.0;
    m_lowValue = 0.0;
    m_topToMid = 0;
    m_midToLow = 0;
    m_init = false;
    m_midAssigned = false;
    m_lowAssigned = false;
    m_negative = false;
    m_decPrecision = DecimalPrecision::Precision0;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
Utf8String NumericTriad::FormatWhole(DecimalPrecision prec)
    {
    NumericFormat fmt("FW");
    fmt.SetDecimalPrecision(prec);
    return fmt.FormatDouble(GetWhole());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
Utf8String NumericTriad::FormatTriad(Utf8CP topName, Utf8CP midName, Utf8CP lowName, Utf8CP space, bool includeZero)
    {
    NumericFormat fmt("FT");
    Utf8CP blank = " ";
    if (IsNameNullOrEmpty(topName))
        topName = blank;
    if (IsNameNullOrEmpty(midName))
        midName = blank;
    if (IsNameNullOrEmpty(lowName))
        lowName = blank;

    if (!m_midAssigned)
        {
        fmt.SetDecimalPrecision(m_decPrecision);
        return fmt.FormatDouble(GetWhole());
        }

    fmt.SetDecimalPrecision(DecimalPrecision::Precision0);
    fmt.SetFormatTraits(FormatTraits::DefaultZeroes);
    Utf8String top = fmt.FormatDouble(m_negative ? -m_topValue : m_topValue);
    top.append(space);
    top.append(topName);
    Utf8String mid = "";
    Utf8String low = "";
    if (m_lowAssigned)
        {
        if (m_midValue > 0.0 || includeZero)
            mid = fmt.FormatDouble(m_midValue);
        if (m_lowValue > 0.0 || includeZero)
            {
            fmt.SetDecimalPrecision(m_decPrecision);
            low = fmt.FormatDouble(m_lowValue);
            }
        }
    else if (m_midValue > 0.0 || includeZero)
        {
        fmt.SetDecimalPrecision(m_decPrecision);
        mid = fmt.FormatDouble(m_midValue);
        }

    if ("" != mid)
        {
        top.append(blank);
        top.append(mid);
        top.append(space);
        top.append(midName);
        }
    if ("" != low)
        {
        top.append(blank);
        top.append(low);
        top.append(space);
        top.append(lowName);
        }
    return top;
    }

//===================================================
//
// FormattingScannerCursorMethods
//
//===================================================

size_t FormattingScannerCursor::TrueIndex(size_t index, size_t wordSize)
    {
    const bool end = FormatConstant::IsLittleEndian();
    if (end || wordSize <= 1)
        return index;
    int i = (int)(--wordSize) - (int)index;
    return i; // (int)(--wordSize) - (int)index;
    }

//---------------------------------------------------------------------------------------
// Constructor
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
FormattingScannerCursor::FormattingScannerCursor(CharCP utf8Text, int scanLength) 
    { 
    m_text = utf8Text;
    /*m_cursorPosition = 0;
    m_lastScannedCount = 0;
    m_code.word = 0;
    m_isASCII = false;
    m_status = ScannerCursorStatus::Success;*/
    Rewind();
    //m_unicodeConst = new UnicodeConstant();
    m_totalScanLength = (nullptr == utf8Text)? 0 : strlen(utf8Text);
    if (scanLength > 0 && scanLength <= (int)m_totalScanLength)
        m_totalScanLength = scanLength;
    }

FormattingScannerCursor::FormattingScannerCursor(FormattingScannerCursorCR other)
    {
    }

void FormattingScannerCursor::Rewind()
{
    m_cursorPosition = 0;
    m_lastScannedCount = 0;
    m_uniCode = 0;
    m_isASCII = false;
    m_status = ScannerCursorStatus::Success;
    return;
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
int FormattingScannerCursor::AddTrailingByte()
    {
    if (ScannerCursorStatus::Success == m_status)
        {
        char bits = 0;
        if (FormatConstant::GetTrailingBits(m_text.c_str()[++m_cursorPosition], &bits))
            {
            m_uniCode <<= FormatConstant::GetTrailingShift();
            m_uniCode |= bits;
            return 1;
            }
        else
            m_status = ScannerCursorStatus::IncompleteSequence;
        }
    return 0;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
size_t FormattingScannerCursor::GetNextSymbol()
    {
    m_status = ScannerCursorStatus::Success;
    m_lastScannedCount = 0;
    m_uniCode = 0;
    m_isASCII = true;
    char c = m_text.c_str()[m_cursorPosition];
    m_temp = 0;
    size_t seqLen = FormatConstant::GetSequenceLength(c);
    if(0 == seqLen)
        m_status = ScannerCursorStatus::InvalidSymbol;
    if ('\0' == c || m_cursorPosition >= m_totalScanLength)
        return m_uniCode;
    m_lastScannedCount++;
    switch (seqLen)
        {
        case 1: // ASCII
            m_uniCode = (size_t)(c & ~FormatConstant::UTF_TrailingByteMark());
            break;
        case 2: // Two byte sequence
            m_uniCode = (size_t)(c &  ~FormatConstant::UTF_2ByteMask());
            m_lastScannedCount += AddTrailingByte();
            m_isASCII = false;
            break;
        case 3: // Three byte sequence
           // m_code.octet[TrueIndex(2, sizeof(m_code.octet))] = c & ~m_unicodeConst->Get3ByteMask();
            m_uniCode = (size_t)(c &  ~FormatConstant::UTF_3ByteMask());
            m_lastScannedCount += AddTrailingByte();
            m_lastScannedCount += AddTrailingByte();
            m_isASCII = false;
            break;
        case 4: // Three byte sequence
            //m_code.octet[TrueIndex(3, sizeof(m_code.octet))] = c & ~m_unicodeConst->Get3ByteMask();
            m_uniCode = (size_t)(c &  ~FormatConstant::UTF_4ByteMask());
            m_lastScannedCount += AddTrailingByte();
            m_lastScannedCount += AddTrailingByte();
            m_lastScannedCount += AddTrailingByte();
            m_isASCII = false;
            break;
        }
    if (IsSuccess())
        m_cursorPosition++;
    return m_uniCode;
    }

size_t FormattingScannerCursor::SkipBlanks()
    {
    size_t code = GetNextSymbol();
    while (IsASCII() && isspace((int)code))
        {
        code = GetNextSymbol();
        }
    return m_lastScannedCount;
    }

//===================================================
//
// FormatStopWatchMethods
//
//===================================================

FormatStopWatch::FormatStopWatch()
{
    m_start = std::chrono::steady_clock::now();
    m_lastInterval = 0.0;
    m_totalElapsed = 0.0;
    m_lastAmount = 0;
    m_totalAmount = 0;
}

Utf8String FormatStopWatch::LastIntervalMetrics(size_t amount)
{
    //m_lastInterval = GetElapsedSeconds();
    std::chrono::steady_clock::time_point moment = std::chrono::steady_clock::now();
    m_lastInterval = (double)std::chrono::duration_cast<std::chrono::microseconds>(moment - m_start).count();
    m_totalElapsed += m_lastInterval;
    m_lastAmount = amount;
    m_totalAmount += amount;
    NumericFormat nfmt = NumericFormat("LIM", 6);
    nfmt.SetUse1000Separator(true);
    Utf8String amTxt = nfmt.FormatInteger((int)amount);
    Utf8String duraTxt = (amount > 0) ? nfmt.FormatDouble(m_lastInterval / (double)amount) : "n/a";
    Utf8String perfTxt = (m_lastInterval > 0.0) ? nfmt.FormatRoundedDouble((double)amount * 1.0e6/ m_lastInterval, 0.5) : "n/a";

    char buf[256];
    sprintf(buf, "Completed %s op's average duration %s mksec performance: %s op/sec", amTxt.c_str(), duraTxt.c_str(), perfTxt.c_str());
    return buf;
}

Utf8String FormatStopWatch::LastInterval(double factor)
    {
    std::chrono::steady_clock::time_point moment = std::chrono::steady_clock::now();
    //std::chrono::duration<double> diff = moment - m_start;
    m_lastInterval = (double)std::chrono::duration_cast<std::chrono::microseconds>(moment - m_start).count();
    char buf[256];
    //m_lastInterval = GetElapsedSeconds();
    sprintf(buf, "%.4f UOR", m_lastInterval * factor);
    return Utf8String(buf);
    }


Utf8String Utils::ShowSignOptionName(ShowSignOption opt)
{
switch (opt)
    {
    case ShowSignOption::OnlyNegative: return "OnlyNegative";
    case ShowSignOption::SignAlways: return "SignAlways";
    case ShowSignOption::NegativeParentheses: return "NegativeParentheses";
    default: return "NoSign";
    }
}

END_BENTLEY_FORMATTING_NAMESPACE
