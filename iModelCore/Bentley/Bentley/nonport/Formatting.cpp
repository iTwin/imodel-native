/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/Formatting.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../BentleyInternal.h"
#include <Bentley/Formatting.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeStringUtilities.h>

USING_NAMESPACE_BENTLEY

#define  THOUSANDSSEPARATOR ','
#define  DECIMALSEPARATOR   '.'
#define  DECIMALSEPARATOR_W   L'.'
#define  ROUNDOFF           (0.5 - std::numeric_limits<double>::epsilon())

// From DgnPlatform.h...
#define   RMINI4                  (-2147483648.0)
#define   RMAXUI4                 4294967295.0

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
int NumericFormat::PrecisionValue() const { return (int)m_decPrecision; }

double NumericFormat::PrecisionFactor() const
    {
    static double FactorSet[13] = {1.0, 10.0, 100.0, 1.0e3, 1.0e4, 1.0e5, 1.0e6, 1.0e7, 1.0e8, 1.0e9, 1.0e10, 1.0e11, 1.0e12};
    return FactorSet[PrecisionValue()];
    }

DecimalPrecision NumericFormat::ConvertToPrecision(size_t num)
    {
        switch (num)
        {
        case 1: return DecimalPrecision::Precision1;
        case 2: return DecimalPrecision::Precision2;        
        case 3: return DecimalPrecision::Precision3;        
        case 4: return DecimalPrecision::Precision4;        
        case 5: return DecimalPrecision::Precision5;
        case 6: return DecimalPrecision::Precision6;
        case 7: return DecimalPrecision::Precision7;        
        case 8: return DecimalPrecision::Precision8;        
        case 9: return DecimalPrecision::Precision9;        
        case 10: return DecimalPrecision::Precision10;
        case 11: return DecimalPrecision::Precision11;
        case 12: return DecimalPrecision::Precision12;
        default: return DecimalPrecision::Precision0;
        }
    }

void NumericFormat::DefaultInit(size_t precision)
    {
    m_minTreshold = 1.0e-16;
    m_presentationType = PresentationType::Decimal;
    m_signOption = ShowSignOption::OnlyNegative;
    m_showDotZero = false;
    m_replace0Empty = false;
    m_decPrecision = ConvertToPrecision(precision);
    m_fractPrecision = FractionalPrecision::Sixteenth;
    m_useThousandsSeparator = false;
    m_decimalSeparator = '.';
    m_thousandsSeparator = ',';
    m_ZeroControl = ZeroControl::None;
    SetKeepSingleZero(true);
    SetKeepDecimalPoint(true);
    }

////////            Helpers for NumericFormat
//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericFormat::SetKeepTrailingZeroes(bool keep)
    {
    size_t temp = static_cast<int>(m_ZeroControl);
    if (keep)
        temp |= static_cast<int>(ZeroControl::TrailingZeroes);
    else
        temp &= ~static_cast<int>(ZeroControl::TrailingZeroes);
    m_ZeroControl = static_cast<ZeroControl>(temp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericFormat::SetUseLeadingZeroes(bool use)
    {
    size_t temp = static_cast<int>(m_ZeroControl);
    if (use)
        temp |= static_cast<int>(ZeroControl::LeadingZeroes);
    else
        temp &= ~static_cast<int>(ZeroControl::LeadingZeroes);
    m_ZeroControl = static_cast<ZeroControl>(temp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericFormat::SetKeepDecimalPoint(bool keep)
    {
    size_t temp = static_cast<int>(m_ZeroControl);
    if (keep)
        temp |= static_cast<int>(ZeroControl::KeepDecimalPoint);
    else
        temp &= ~static_cast<int>(ZeroControl::KeepDecimalPoint);
    m_ZeroControl = static_cast<ZeroControl>(temp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
void NumericFormat::SetKeepSingleZero(bool keep)
    {
    size_t temp = static_cast<int>(m_ZeroControl);
    if (keep)
        temp |= static_cast<int>(ZeroControl::KeepSingleZero);
    else
        temp &= ~static_cast<int>(ZeroControl::KeepSingleZero);
    m_ZeroControl = static_cast<ZeroControl>(temp);
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
int NumericFormat::IntPartToText (double n, char * bufOut, int bufLen, bool useSeparator)
    {
    char sign = '+';
    char buf[64];
    double n1;
    int ind = 0;

    useSeparator = (m_useThousandsSeparator && m_thousandsSeparator != 0 && useSeparator);

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
        sign = (m_signOption == ShowSignOption::NegativeParentheses) ? '(' : '-';
        }

    ind = sizeof(buf);
    memset(buf, 0, ind--);
    if (sign == '(')
        buf[--ind] = ')';
    int digs = 0;
    int rem;
    do {
        n1 = floor(n / 10.0);
        rem = (int)(n - 10.0 * n1);
        buf[--ind] = (char)rem + '0';
        if (useSeparator)
            digs++;
        n = n1;
        if (n > 0 && digs > 2)
            {
            buf[--ind] = m_thousandsSeparator;
            digs = 0;
            }
        } while (n > 0 && ind >= 0);

    if (m_signOption == ShowSignOption::SignAlways ||
        ((m_signOption == ShowSignOption::OnlyNegative || m_signOption == ShowSignOption::NegativeParentheses) && sign != '+'))
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
int NumericFormat::FormatInteger (int n, char* bufOut,  int bufLen)
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
        sign = (m_signOption == ShowSignOption::NegativeParentheses) ? '(' : '-';
        }

    ind = sizeof(buf);
    memset(buf, 0, ind--);
    if ('(' == sign)
        buf[--ind] = ')';

    int digs = 0;
    do {
        n1 = n / 10;
        buf[--ind] = (char)(n - 10 * n1) + '0';
        if (m_useThousandsSeparator)
            digs++;
        n = n1;
        if (n > 0 && digs > 2)
            {
            digs = 0;
            buf[--ind] = m_thousandsSeparator;
            }
        } while (n > 0 && ind >= 0);

    if (m_signOption == ShowSignOption::SignAlways ||
         ((m_signOption == ShowSignOption::OnlyNegative || m_signOption == ShowSignOption::NegativeParentheses) && sign != '+'))
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
int NumericFormat::TrimTrailingZeroes(CharP buf, int index)
    {
    if (nullptr == buf)
        return index;
    int i = (0 > index) ? (int)strlen(buf) - 1 : index;

    while (buf[i] == '0' && index > 0)
        {
        if (m_decimalSeparator == buf[i - 1])
         {
            if (IfKeepSingleZero()) // preserve decimal separator and a single zero after it
                break;
            i--;
            if (IfKeepDecimalPoint())
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
//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
int NumericFormat::FormatDouble(double dval, char* buf, int bufLen)
    {
    double ival;
    Utf8Char sign = '+';
    double precScale = NumericFormat::PrecisionFactor();
    int totFractLen = NumericFormat::PrecisionValue();
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
        double rounding = 0.501;
        memset(locBuf, 0, sizeof(locBuf));

        fract = modf(IsPrecisionZero()? dval + rounding: dval, &ival);
        int iLen = IntPartToText(ival, intBuf, sizeof(intBuf), true);
        if (m_signOption == ShowSignOption::SignAlways || 
            ((m_signOption == ShowSignOption::OnlyNegative || m_signOption == ShowSignOption::NegativeParentheses) && sign != '+'))
            locBuf[ind++] = sign;

        memcpy(&locBuf[ind], intBuf, iLen);
        ind += iLen;

        if (IsPrecisionZero())
            {
            if (m_showDotZero)
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
            if (!IfKeepTrailingZeroes() && locBuf[ind - 1] == '0')
                ind = TrimTrailingZeroes(locBuf, ind-1);
            }
        if (sci && expInt != 0)
            {
            char expBuf[32];
            int expLen = FormatInteger((int)expInt, expBuf, sizeof(expBuf));
            locBuf[ind++] = 'e';
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
Utf8String NumericFormat::FormatDouble(double dval)
    {
    char buf[64];
    FormatDouble(dval, buf, sizeof(buf));
    return Utf8String(buf);
    }

Utf8String NumericFormat::FormatRoundedDouble(double dval, double round)
    {
    return FormatDouble(RoundedValue(dval, round));
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
 int NumericFormat::FormatBinaryShort (short int n, char* bufOut, int bufLen, bool useSeparator)
 {
     char binBuf[64];

     unsigned char c = (n & 0xFF00) >> 8;
     FormatBinaryByte (c, binBuf, 9);
     int ind = 8;
     if (IfInsertSeparator(useSeparator))
         binBuf[ind++] = m_thousandsSeparator;
     c = n & 0xFF;
     FormatBinaryByte (c, &binBuf[ind], 9);
     return RightAlignedCopy(bufOut, bufLen, true, binBuf, -1);
 }

 //---------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //---------------------------------------------------------------------------------------
 int NumericFormat::FormatBinaryInt (int n, char* bufOut, int bufLen, bool useSeparator)
 {
     char binBuf[80];

     memset(binBuf, 0, sizeof(binBuf));
     unsigned int temp = (size_t)n;
     unsigned short int c = (temp & 0xFFFF0000) >> 16;
     FormatBinaryShort (c, binBuf, sizeof(binBuf), useSeparator);
     size_t ind = strlen(binBuf);
     if (IfInsertSeparator(useSeparator))
         binBuf[ind++] = m_thousandsSeparator;
     c = n & 0xFFFF;
     FormatBinaryShort (c, &binBuf[ind], (int)(sizeof(binBuf) - ind), useSeparator);
     return RightAlignedCopy(bufOut, bufLen, true, binBuf, -1);
 }

 int NumericFormat::FormatBinaryDouble (double x, char* bufOut, int bufLen, bool useSeparator)
 {
     char binBuf[80];
     union { unsigned int ival[2]; double x; }temp;
     temp.x = x;
     memset(binBuf, 0, sizeof(binBuf));
     FormatBinaryInt (temp.ival[1], binBuf, sizeof(binBuf), useSeparator);
     size_t ind = strlen(binBuf);
     if (IfInsertSeparator(useSeparator))
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


 //---------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //---------------------------------------------------------------------------------------
void FormatDictionary::InitLoad()
    {
    AddParameter(FormatParameter("NoSign", ParameterCategory::Sign, ParameterCode::NoSign, ParameterDataType::Flag));
    AddParameter(FormatParameter("OnlyNegative", ParameterCategory::Sign, ParameterCode::OnlyNegative, ParameterDataType::Flag));
    AddParameter(FormatParameter("SignAlways", ParameterCategory::Sign, ParameterCode::SignAlways, ParameterDataType::Flag));
    AddParameter(FormatParameter("NegativeParenths", ParameterCategory::Sign, ParameterCode::NegativeParenths, ParameterDataType::Flag));
    AddParameter(FormatParameter("Decimal", ParameterCategory::Presentation, ParameterCode::Decimal, ParameterDataType::Flag));
    AddParameter(FormatParameter("Fractional", ParameterCategory::Presentation, ParameterCode::Fractional, ParameterDataType::Flag));
    AddParameter(FormatParameter("Scientific", ParameterCategory::Presentation, ParameterCode::Sientific, ParameterDataType::Flag));
    AddParameter(FormatParameter("ScientificNorm", ParameterCategory::Presentation, ParameterCode::ScientificNorm, ParameterDataType::Flag));
    AddParameter(FormatParameter("Binary", ParameterCategory::Presentation, ParameterCode::Binary, ParameterDataType::Flag));
    AddParameter(FormatParameter("NoZeroes", ParameterCategory::Zeroes, ParameterCode::NoZeroControl, ParameterDataType::Flag));
    AddParameter(FormatParameter("LeadingZeroes", ParameterCategory::Zeroes, ParameterCode::LeadingZeroes, ParameterDataType::Flag));
    AddParameter(FormatParameter("TrailingZeroes", ParameterCategory::Zeroes, ParameterCode::TrailingZeroes, ParameterDataType::Flag));
    AddParameter(FormatParameter("KeepDecimalPoint", ParameterCategory::Zeroes, ParameterCode::KeepDecimalPoint, ParameterDataType::Flag));
    AddParameter(FormatParameter("Replace0Empty", ParameterCategory::Zeroes, ParameterCode::Replace0Empty, ParameterDataType::Flag));
    AddParameter(FormatParameter("KeepSingleZero", ParameterCategory::Zeroes, ParameterCode::KeepSingleZero, ParameterDataType::Flag));
    AddParameter(FormatParameter("Precision0", ParameterCategory::DecPrecision, ParameterCode::DecPrec0, ParameterDataType::Flag));
    AddParameter(FormatParameter("Precision1", ParameterCategory::DecPrecision, ParameterCode::DecPrec1, ParameterDataType::Flag));
    AddParameter(FormatParameter("Precision2", ParameterCategory::DecPrecision, ParameterCode::DecPrec2, ParameterDataType::Flag));
    AddParameter(FormatParameter("Precision3", ParameterCategory::DecPrecision, ParameterCode::DecPrec3, ParameterDataType::Flag));
    AddParameter(FormatParameter("Precision4", ParameterCategory::DecPrecision, ParameterCode::DecPrec4, ParameterDataType::Flag));
    AddParameter(FormatParameter("Precision5", ParameterCategory::DecPrecision, ParameterCode::DecPrec5, ParameterDataType::Flag));
    AddParameter(FormatParameter("Precision6", ParameterCategory::DecPrecision, ParameterCode::DecPrec6, ParameterDataType::Flag));
    AddParameter(FormatParameter("Precision7", ParameterCategory::DecPrecision, ParameterCode::DecPrec7, ParameterDataType::Flag));
    AddParameter(FormatParameter("Precision8", ParameterCategory::DecPrecision, ParameterCode::DecPrec8, ParameterDataType::Flag));
    AddParameter(FormatParameter("Precision9", ParameterCategory::DecPrecision, ParameterCode::DecPrec9, ParameterDataType::Flag));
    AddParameter(FormatParameter("Precision10", ParameterCategory::DecPrecision, ParameterCode::DecPrec10, ParameterDataType::Flag));
    AddParameter(FormatParameter("Precision11", ParameterCategory::DecPrecision, ParameterCode::DecPrec11, ParameterDataType::Flag));
    AddParameter(FormatParameter("Precision12", ParameterCategory::DecPrecision, ParameterCode::DecPrec12, ParameterDataType::Flag));
    AddParameter(FormatParameter("FractPrec1", ParameterCategory::FractPrecision, ParameterCode::FractPrec1, ParameterDataType::Flag));
    AddParameter(FormatParameter("FractPrec2", ParameterCategory::FractPrecision, ParameterCode::FractPrec2, ParameterDataType::Flag));
    AddParameter(FormatParameter("FractPrec4", ParameterCategory::FractPrecision, ParameterCode::FractPrec4, ParameterDataType::Flag));
    AddParameter(FormatParameter("FractPrec8", ParameterCategory::FractPrecision, ParameterCode::FractPrec8, ParameterDataType::Flag));
    AddParameter(FormatParameter("FractPrec16", ParameterCategory::FractPrecision, ParameterCode::FractPrec16, ParameterDataType::Flag));
    AddParameter(FormatParameter("FractPrec32", ParameterCategory::FractPrecision, ParameterCode::FractPrec32, ParameterDataType::Flag));
    AddParameter(FormatParameter("FractPrec64", ParameterCategory::FractPrecision, ParameterCode::FractPrec64, ParameterDataType::Flag));
    AddParameter(FormatParameter("FractPrec128", ParameterCategory::FractPrecision, ParameterCode::FractPrec128, ParameterDataType::Flag));
    AddParameter(FormatParameter("FractPrec256", ParameterCategory::FractPrecision, ParameterCode::FractPrec256, ParameterDataType::Flag));
    AddParameter(FormatParameter("DecimalComma", ParameterCategory::Separator, ParameterCode::DecimalComma, ParameterDataType::Flag));
    AddParameter(FormatParameter("DecimalPoint", ParameterCategory::Separator, ParameterCode::DecimalPoint, ParameterDataType::Flag));
    AddParameter(FormatParameter("DecimalSepar", ParameterCategory::Separator, ParameterCode::DecimalSepar, ParameterDataType::Symbol));
    AddParameter(FormatParameter("ThousandSepComma", ParameterCategory::Separator, ParameterCode::ThousandSepComma, ParameterDataType::Flag));
    AddParameter(FormatParameter("ThousandSepPoint", ParameterCategory::Separator, ParameterCode::ThousandSepPoint, ParameterDataType::Flag));
    AddParameter(FormatParameter("ThousandsSepar", ParameterCategory::Separator, ParameterCode::ThousandsSepar, ParameterDataType::Symbol));
    AddParameter(FormatParameter("RoundUp", ParameterCategory::RoundType, ParameterCode::RoundUp, ParameterDataType::Double));
    AddParameter(FormatParameter("RoundDown", ParameterCategory::RoundType, ParameterCode::RoundDown, ParameterDataType::Double));
    AddParameter(FormatParameter("RoundToward0", ParameterCategory::RoundType, ParameterCode::RoundToward0, ParameterDataType::Double));
    AddParameter(FormatParameter("RoundAwayFrom0", ParameterCategory::RoundType, ParameterCode::RoundAwayFrom0, ParameterDataType::Double));
    AddParameter(FormatParameter("FractBarHoriz", ParameterCategory::FractionBar, ParameterCode::FractBarHoriz, ParameterDataType::Flag));
    AddParameter(FormatParameter("FractBarOblique", ParameterCategory::FractionBar, ParameterCode::FractBarOblique, ParameterDataType::Flag));
    AddParameter(FormatParameter("FractBarDiagonal", ParameterCategory::FractionBar, ParameterCode::FractBarDiagonal, ParameterDataType::Flag));
    AddParameter(FormatParameter("AngleRegular", ParameterCategory::AngleFormat, ParameterCode::AngleRegular, ParameterDataType::Flag));
    AddParameter(FormatParameter("AngleDegMin", ParameterCategory::AngleFormat, ParameterCode::AngleDegMin, ParameterDataType::Flag));
    AddParameter(FormatParameter("AngleDegMinSec", ParameterCategory::AngleFormat, ParameterCode::AngleDegMinSec, ParameterDataType::Flag));
    AddParameter(FormatParameter("PaddingSymbol", ParameterCategory::Padding, ParameterCode::PaddingSymbol, ParameterDataType::Symbol));
    AddParameter(FormatParameter("CenterAlign", ParameterCategory::Alignment, ParameterCode::CenterAlign, ParameterDataType::Flag));
    AddParameter(FormatParameter("LeftAlign", ParameterCategory::Alignment, ParameterCode::LeftAlign, ParameterDataType::Flag));
    AddParameter(FormatParameter("RightAlign", ParameterCategory::Alignment, ParameterCode::RightAlign, ParameterDataType::Flag));
    AddParameter(FormatParameter("MapName", ParameterCategory::Mapping, ParameterCode::MapName, ParameterDataType::String));
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
    NumericFormat fmt;
    fmt.SetDecimalPrecision(prec);
    return fmt.FormatDouble(GetWhole());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
Utf8String NumericTriad::FormatTriad(Utf8StringCP topName, Utf8StringCP midName, Utf8StringCP lowName, bool includeZero)
    {
    NumericFormat fmt;
    Utf8String blank = Utf8String(" ");
    if (nullptr == topName || topName->size() < 1)
        topName = &blank;
    if (nullptr == midName || midName->size() < 1)
        midName = &blank;
    if (nullptr == lowName || lowName->size() < 1)
        lowName = &blank;

    if (!m_midAssigned)
        {
        fmt.SetDecimalPrecision(m_decPrecision);
        return fmt.FormatDouble(GetWhole());
        }

    fmt.SetDecimalPrecision(DecimalPrecision::Precision0);
    Utf8String top = fmt.FormatDouble(m_negative ? -m_topValue : m_topValue);
    top.append(blank);
    top.append(*topName);
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
        top.append(blank);
        top.append(*midName);
        }
    if ("" != low)
        {
        top.append(blank);
        top.append(low);
        top.append(blank);
        top.append(*lowName);
        }
    return top;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
const size_t UnicodeConstant::GetSequenceLength(unsigned char c)
    {
    if (0 == (c & m_trailingByteMark)) // ASCII - single byte
        return 1;
    if ((c & m_twoByteMask) == m_twoByteMark)
        return 2;
    if ((c & m_threeByteMask) == m_threeByteMark)
        return 3;
    if ((c & m_fourByteMask) == m_fourByteMark)
        return 4;
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
bool UnicodeConstant::IsTrailingByteValid(unsigned char c)
    {
    return (m_trailingByteMark == (c & m_trailingByteMask));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
bool UnicodeConstant::GetTrailingBits(unsigned char c, CharP outBits)
    {	
    if (nullptr != outBits)
        {
        *outBits = 0;
        if (m_trailingByteMark == (c & m_trailingByteMask))
            *outBits = c & ~m_trailingByteMask;
        return true;
        }
    return false;
    }

// the trailing byte should be properly marked for being processed. It always contains only
//  6 bits that should be shifted accordingly to the location of the byte in the sequence
//  there are only 3 possible numbers of bytes in sequences: 2, 3 and 4. Accordingly the 
//  the range of indexes is striclty governed by the sequence lenght. The minimum index
//  value is always 1 and the maximum is N-1 where N is the sequence length
//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
bool UnicodeConstant::GetCodeBits(unsigned char c, size_t seqLength, size_t index, size_t* outBits)
    {
    if (nullptr != outBits)
        {
        // calculate the shift 
        *outBits = 0;
        size_t shift = (seqLength - index - 1);
        if(0 > shift || 2 < shift)
            return false;      
        if (m_trailingByteMark == (c & m_trailingByteMask))
            {
            size_t temp = c & ~m_trailingByteMask;
            temp <<= shift * m_upperBitShift;
            *outBits = temp;
            }
        return true;
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
bool UnicodeConstant::IsLittleEndian()
    { 
    return m_isLittleEndian; 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
bool UnicodeConstant::ForceBigEndian()
    {
    m_isLittleEndian = false; 
    return m_isLittleEndian; 
    }

//bool UnicodeConstant::GetSymbolBits(unsigned char c, CharP outBits)


// FormattingScannerCursor implementation

size_t FormattingScannerCursor::TrueIndex(size_t index, size_t wordSize)
    {
    bool end = m_unicodeConst->IsLittleEndian();
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
    m_unicodeConst = new UnicodeConstant();
    m_totalScanLength = (nullptr == utf8Text)? 0 : strlen(utf8Text);
    if (scanLength > 0 && scanLength <= m_totalScanLength)
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
        if (m_unicodeConst->GetTrailingBits(m_text.c_str()[++m_cursorPosition], &bits))
            {
            m_uniCode <<= m_unicodeConst->GetTrailingShift();
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
    size_t seqLen = m_unicodeConst->GetSequenceLength(c);
    if(0 == seqLen)
        m_status = ScannerCursorStatus::InvalidSymbol;
    if ('\0' == c || m_cursorPosition >= m_totalScanLength)
        return m_uniCode;
    m_lastScannedCount++;
    switch (seqLen)
        {
        case 1: // ASCII
            m_uniCode = (size_t)(c & ~m_unicodeConst->GetTrailingByteMark());
            break;
        case 2: // Two byte sequence
            m_uniCode = (size_t)(c &  ~m_unicodeConst->Get2ByteMask());
            m_lastScannedCount += AddTrailingByte();
            m_isASCII = false;
            break;
        case 3: // Three byte sequence
           // m_code.octet[TrueIndex(2, sizeof(m_code.octet))] = c & ~m_unicodeConst->Get3ByteMask();
            m_uniCode = (size_t)(c &  ~m_unicodeConst->Get3ByteMask());
            m_lastScannedCount += AddTrailingByte();
            m_lastScannedCount += AddTrailingByte();
            m_isASCII = false;
            break;
        case 4: // Three byte sequence
            //m_code.octet[TrueIndex(3, sizeof(m_code.octet))] = c & ~m_unicodeConst->Get3ByteMask();
            m_uniCode = (size_t)(c &  ~m_unicodeConst->Get4ByteMask());
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

//size_t FormattingScannerCursor::GetNextCodePoint()
//{
//    ScannerCursorStatus status = ScannerCursorStatus::Success;
//    size_t lastScannedCount = 0;
//    size_t uniCode = 0;
//    bool isASCII = true;
//    size_t cursorPosition = m_cursorPosition;
//    char c = m_text.c_str()[cursorPosition];
//    char temp = 0;
//    size_t seqLen = m_unicodeConst->GetSequenceLength(c);
//    if (0 == seqLen)
//        m_status = ScannerCursorStatus::InvalidSymbol;
//    if ('\0' == c || m_cursorPosition >= m_totalScanLength)
//        return m_uniCode;
//    m_lastScannedCount++;
//    switch (seqLen)
//    {
//    case 1: // ASCII
//        m_uniCode = (size_t)(c & ~m_unicodeConst->GetTrailingByteMark());
//        break;
//    case 2: // Two byte sequence
//        m_uniCode = (size_t)(c &  ~m_unicodeConst->Get2ByteMask());
//        m_lastScannedCount += AddTrailingByte();
//        m_isASCII = false;
//        break;
//    case 3: // Three byte sequence
//            // m_code.octet[TrueIndex(2, sizeof(m_code.octet))] = c & ~m_unicodeConst->Get3ByteMask();
//        m_uniCode = (size_t)(c &  ~m_unicodeConst->Get3ByteMask());
//        m_lastScannedCount += AddTrailingByte();
//        m_lastScannedCount += AddTrailingByte();
//        m_isASCII = false;
//        break;
//    case 4: // Three byte sequence
//            //m_code.octet[TrueIndex(3, sizeof(m_code.octet))] = c & ~m_unicodeConst->Get3ByteMask();
//        m_uniCode = (size_t)(c &  ~m_unicodeConst->Get4ByteMask());
//        m_lastScannedCount += AddTrailingByte();
//        m_lastScannedCount += AddTrailingByte();
//        m_lastScannedCount += AddTrailingByte();
//        m_isASCII = false;
//        break;
//    }
//    if (IsSuccess())
//        m_cursorPosition++;
//    return m_uniCode;
//}
//


size_t FormattingScannerCursor::SkipBlanks() 
    {
    size_t code = GetNextSymbol();
    while (IsASCII() && isspace((int)code))
        {
        code = GetNextSymbol();
        }
    return m_lastScannedCount;
    }

//int FormattingScannerCursor::CodePointCount()
//    {
//    int n = 0;
//    int pos = 0;
//    char c;
//    c= m_text.c_str()[pos];
//    size_t seqLen = m_unicodeConst->GetSequenceLength(c);
//
//    return n;
//    }

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
    NumericFormat nfmt = NumericFormat(6);
    nfmt.SetUseSeparator(true);
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