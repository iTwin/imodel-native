/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/Formatting.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../BentleyInternal.h"
//#include <Bentley/ValueFormat.h>
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   David Fox-Rabinovitz 11/16
+---------------+---------------+---------------+---------------+---------------+------*/

    int NumericFormat::PrecisionValue() const { return (int)m_decPrecision; }

    double NumericFormat::PrecisionFactor() const
    {
    static double FactorSet[13] = {1.0, 10.0, 100.0, 1.0e3, 1.0e4, 1.0e5, 1.0e6, 1.0e7, 1.0e8, 1.0e9, 1.0e10, 1.0e11, 1.0e12};
    return FactorSet[PrecisionValue()];
    }

    DecimalPrecision NumericFormat::ConvertToPrecision(int num)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   David Fox-Rabinovitz 11/16
+---------------+---------------+---------------+---------------+---------------+------*/
    int NumericFormat::IntPartToText
    (
        double n,                  // given real number
        char * bufOut,
        int bufLen,
        bool useSeparator
    )
    {
        char sign = '+';
        char buf[64];
        int len;
        double n1;
        int ind = 0;
        //std::string* str;
        bool parent = false;

        if (m_useThousandsSeparator && m_thousandsSeparator == 0 && useSeparator)
            useSeparator = true;
        else
            useSeparator = false;

        if (NULL == bufOut || bufLen < 1)
            return 0;
        if (bufLen < 2)
        {
            *bufOut = 0;
            return 0;
        }

        if (n == 0)
        {
            *bufOut++ = '0';
            *bufOut = 0;
            return 1;
        }

        if (n < 0)
        {
            n = -n;
            if (m_signOption == ShowSignOption::NegativeParentheses)
            {
                parent = true;
                sign = '(';
            }
            else
                sign = '-';
        }

        int totLen = sizeof(buf);
        memset(buf, 0, totLen);
        ind = totLen - 1;
        len = 1;
        if (parent)
        {
            buf[--ind] = ')';
            len++;
        }
        int digs = 0;
        int rem;
        do {
            n1 = floor(n / 10.0);
            rem = (int)(n - 10.0 * n1);
            buf[--ind] = (char)rem + '0';
            len++;
            if (useSeparator)
                digs++;
            n = n1;
            if (n > 0 && digs > 2)
            {
                buf[--ind] = m_thousandsSeparator;
                digs = 0;
                len++;
            }
        } while (n > 0 && ind >= 0);

        if (m_signOption == ShowSignOption::SignAlways || (m_signOption == ShowSignOption::OnlyNegative && sign == '-' || parent))
        {
            buf[--ind] = sign;
            len++;
        }

        // now we just need to copy it out
        if (len > (--bufLen))
            len = bufLen;
        memcpy_s(bufOut, bufLen, &buf[ind], len);
        return len - 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   David Fox-Rabinovitz 11/16
+---------------+---------------+---------------+---------------+---------------+------*/
    int NumericFormat::FormatInteger
    (
        int n, 
        char* bufOut, 
        int bufLen
    )
    {
    char sign = '+';
    char buf[64];
    int len;
    int n1;
    int ind = 0;
    bool parent = false;

    if (NULL == bufOut || bufLen < 1)  // do nothing when output buffer is not provided
        return 0;

    if (bufLen < 2)  // if output buffer is too short make it empty and return
    {
        *bufOut = 0;
        return 0;
    }

    if (n == 0)  // the buffer is at least sufficient to take two bytes for the '0' value
    {
        *bufOut++ = '0';
        *bufOut = 0;
        return 1;
    }

    if (n < 0)
    {
        n = -n;

        if (m_signOption == ShowSignOption::NegativeParentheses)
        {
            parent = true;
            sign = '(';
        }
        else
            sign = '-';
    }

    ind = sizeof(buf);
    memset(buf, 0, ind--);
    len = 1;
    if (parent)
    {
        buf[--ind] = ')';
        len++;
    }
    int digs = 0;
    do {
        n1 = n / 10;
        buf[--ind] = (char)(n - 10 * n1) + '0';
        len++;
        if (m_useThousandsSeparator)
            digs++;
        n = n1;
        if (n > 0 && digs > 2)
        {
            digs = 0;
            buf[--ind] = m_thousandsSeparator;
            len++;
        }
    } while (n > 0 && ind >= 0);

    if (m_signOption == ShowSignOption::SignAlways || (m_signOption == ShowSignOption::OnlyNegative && sign == '-') || parent)
    {
        buf[--ind] = sign;
        len++;
    }

    if (len > (--bufLen))
        len = bufLen;
    memcpy_s(bufOut, bufLen, &buf[ind], len);
    return len - 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   David Fox-Rabinovitz 11/16
+---------------+---------------+---------------+---------------+---------------+------*/
    int NumericFormat::FormatDouble(double dval, char* buf, int bufLen)
    {
    double ival;
    bool parent = false;
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
        if (m_signOption == ShowSignOption::NegativeParentheses)
        {
            parent = true;
            sign = '(';
        }
        else
            sign = '-';
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
        if (m_signOption == ShowSignOption::SignAlways || (m_signOption == ShowSignOption::OnlyNegative && sign == '-') || parent)
        {
            locBuf[ind++] = sign;
        }
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
            while (fLen < totFractLen)
            {
                locBuf[ind++] = '0';
                fLen++;
            }
            memcpy(&locBuf[ind], fractBuf, fLen);
            ind += fLen;
        }
        if (sci && expInt != 0)
        {
            char expBuf[32];
            int expLen = FormatInteger((int)expInt, expBuf, sizeof(expBuf));
            locBuf[ind++] = 'e';
            memcpy(&locBuf[ind], expBuf, expLen);
            ind += expLen;
        }
    }

    // closing formatting
    if (parent)
        locBuf[ind++] = ')';
    locBuf[ind++] = '\0';

    if (ind > bufLen)
        ind = bufLen;
    memcpy_s(buf, bufLen, locBuf, ind);
    return ind;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   David Fox-Rabinovitz 11/16
+---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String NumericFormat::FormatDouble(double dval)
    {
    char buf[64];
    FormatDouble(dval, buf, sizeof(buf));
    return Utf8String(buf);
    }

    Utf8String NumericFormat::FormatInteger(int ival)
    {
        char buf[64];
        FormatInteger(ival, buf, sizeof(buf));
        return Utf8String(buf);
    }



