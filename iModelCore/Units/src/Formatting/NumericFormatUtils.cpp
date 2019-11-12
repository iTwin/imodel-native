/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>
#include "../../PrivateAPI/Formatting/NumericFormatUtils.h"

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//===================================================
// FractionalNumeric
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//----------------------------------------------------------------------------------------
size_t FractionalNumeric::GCF(size_t numer, size_t denom)
    {
    FactorizedNumber numF = FactorizedNumber(numer);
    FactorizedNumber denF = FactorizedNumber(denom);
    return numF.GetGreatestCommonFactor(denF);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//----------------------------------------------------------------------------------------
void FractionalNumeric::Calculate(double dval, size_t denominator)
    {
    m_denominator = denominator;
    double integral = 0.0;
    double fract = modf(fabs(dval), &integral);
    double numer = floor(fract * m_denominator + FormatConstant::FPV_RoundFactor());
    m_numerator = static_cast<int>(numer);
    m_integral = static_cast<size_t>(integral);
    m_gcf = 1;
    if (0 != denominator && (m_numerator / m_denominator) == 1)
        {
        m_numerator = 0;
        m_integral += 1;
        }
    else
        m_gcf = GCF(m_numerator, m_denominator);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//----------------------------------------------------------------------------------------
FractionalNumeric::FractionalNumeric(double dval, FractionalPrecision fprec)
    {
    Calculate(dval, Utils::FractionalPrecisionDenominator(fprec));
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//----------------------------------------------------------------------------------------
void FractionalNumeric::FormTextParts(bool reduce)
    {
    NumericFormatSpec fmt;
    fmt.SetPresentationType(PresentationType::Decimal);
    fmt.SetSignOption(SignOption::OnlyNegative);
    fmt.SetFormatTraits(static_cast<FormatTraits>(0x000));
    fmt.SetPrecision(DecimalPrecision::Precision0);
    size_t numer = m_numerator;
    size_t denom = m_denominator;
    if (reduce && m_gcf > 1)
        {
        numer /= m_gcf;
        denom /= m_gcf;
        }
    m_textParts.push_back(fmt.Format((int)m_integral));
    if (numer > 0)
        {
        m_textParts.push_back(fmt.Format((int)numer));
        m_textParts.push_back(fmt.Format((int)denom));
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
Utf8String FractionalNumeric::GetIntegralString()
    {
    Utf8String strP;
    if (0 < m_textParts.size())
        strP = m_textParts.at(0);
    return strP ;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
Utf8String FractionalNumeric::GetNumeratorString()
    {
    Utf8String strP;
    if (3 <= m_textParts.size())
        strP = m_textParts.at(1);
    return strP;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
Utf8String FractionalNumeric::GetDenominatorString()
    {
    Utf8String strP;
    if (3 <= m_textParts.size())
        strP = m_textParts.at(2);
    return strP;
    }


//===================================================
// FactorPower
//===================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//----------------------------------------------------------------------------------------
const size_t FactorPower::GetFactor()
    {
    size_t res = 1;
    for(size_t i = 0; i < m_power; i++)
        res *= m_divisor;
    return res;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
void FactorPower::CopyValues(FactorPowerCP other)
    {
    if (nullptr == other)
        {
        m_divisor = 0;
        m_power = 0;
        m_index = 0;
        }
    else
        {
        m_divisor = other->m_divisor;
        m_power = other->m_power;
        m_index = other->m_index;
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//----------------------------------------------------------------------------------------
void FactorPower::Merge(FactorPowerCP fp1, FactorPowerCP fp2)
    {
    m_divisor = 0;
    m_power = 0;
    m_index = -1;
    if (nullptr == fp1 || nullptr == fp2)
        return;
    if (fp1->m_index == fp2->m_index )
        m_index = fp1->m_index;
    if (fp1->m_divisor == fp2->m_divisor)
        {
        m_divisor = fp1->m_divisor;
        m_power = GetMin(fp1->m_power, fp2->m_power);
        }
    }

//===================================================
// FactorizedNumber
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//----------------------------------------------------------------------------------------
const size_t* FactorizedNumber::GetPrimes(int* length)
    {
    static const size_t prim[]{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73,
        79, 83, 89, 97, 101, 0}; // NOTE: the last number in  this sequence always must be 0
    if (nullptr != length)
        *length = static_cast<int>((sizeof(prim) -1)/ sizeof(size_t));
    return prim;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//----------------------------------------------------------------------------------------
size_t FactorizedNumber::GetPrimeCount()
    {
    int primN;
    GetPrimes(&primN);
    return static_cast<size_t>(primN);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//----------------------------------------------------------------------------------------
size_t FactorizedNumber::PowerOfPrime(size_t ival, size_t prim, size_t* result)
    {
    size_t rem = ival % prim;
    size_t pwr = 0;
    while (0 == rem && ival > 1)
        {
        pwr++;
        ival = ival / prim;
        rem = ival % prim;
        }
    if (nullptr != result)
        *result = ival;
    return pwr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//----------------------------------------------------------------------------------------
FactorizedNumber::FactorizedNumber(size_t ival)
    {
    m_ival = ival;
    int primM;
    const size_t* primN = GetPrimes(&primM);
    size_t pwr = 0;
    size_t num = ival;
    FactorPower fp;
    for (int i = 0; primN[i] > 0; i++)
        {
        pwr = PowerOfPrime(num, primN[i], &num);
        if (pwr > 0)
            {
            fp = FactorPower(primN[i], pwr, i);
            m_factors.push_back(fp);
            }
        }
    if(num > 1)
        {
        fp = FactorPower(num, 1, primM+1); // the last factor has index exceeding the number of primes in the base set
        m_factors.push_back(fp);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//----------------------------------------------------------------------------------------
size_t FactorizedNumber::GetGreatestCommonFactor(FactorizedNumber other)
    {
    size_t primN = GetPrimeCount();
    size_t primS = (primN + 2)* sizeof(FactorPower);
    FactorPowerP fact1 = (FactorPowerP)alloca(primS);
    FactorPowerP fact2 = (FactorPowerP)alloca(primS);
    memset(fact1, 0, primS);
    memset(fact2, 0, primS);
    FactorPower temp = FactorPower();

    FactorPowerP fpp;
    for (auto curr = m_factors.begin(), end = m_factors.end(); curr != end; curr++)
        {
        fpp = curr;
        fact1[fpp->GetIndex()].CopyValues(fpp);
        }
    bvector<FactorPower> otherPwr = other.GetFactors();
    for (auto curr = otherPwr.begin(), end = otherPwr.end(); curr != end; curr++)
        {
        fpp = curr;
        fact2[fpp->GetIndex()].CopyValues(fpp);
        }
    size_t fact = 1;
    for (size_t i = 0; i < primN; i++)
        {
        temp.Merge(&fact1[i], &fact2[i]);
        fact *= temp.GetFactor();
        }

    return fact;
    }

//===================================================
// StringUtils
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
// static
size_t StringUtils::AppendText(Utf8P buf, size_t bufLen, size_t index, Utf8CP str)
    {
    int cap = static_cast<int>(bufLen) - static_cast<int>(index) - 1;
    size_t strL = (nullptr == str) ? 0 : strlen(str);
    if (strL < 1 || cap < 1)
        return index;
    if (static_cast<int>(strL) > cap)
        strL = static_cast<size_t>(cap);
    memcpy(static_cast<void*>(buf + index), str, strL);
    index += strL;
    buf[index] = FormatConstant::EndOfLine();     
    return index;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
// static
int StringUtils::IndexOf(Utf8Char c, Utf8CP text)
    {
    int indx = -1;
    if (Utf8String::IsNullOrEmpty(text))
        return indx;
    while (*text != FormatConstant::EndOfLine())
        {
        ++indx;
        if (c == *text)
            return indx;
        text++;
        }
    return -1;
    }

END_BENTLEY_FORMATTING_NAMESPACE
