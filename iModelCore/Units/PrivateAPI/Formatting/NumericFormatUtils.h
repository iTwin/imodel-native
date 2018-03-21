/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateAPI/Formatting/NumericFormatUtils.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Formatting/FormattingEnum.h>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(FactorPower)

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct FactorPower
{
private:
    size_t m_divisor;  // the value of divisor
    size_t m_power;    // the degree of the divisor
    int m_index;    // the index of the divisor in the prime set
    static size_t GetMin(size_t i1, size_t i2) { return (i1 <= i2) ? i1 : i2; }
public:
    FactorPower() { m_divisor = 0; m_power = 0; m_index = -1; }
    FactorPower(size_t div, size_t pow, int ind) : m_divisor(div), m_power(pow), m_index(ind) {}
    UNITS_EXPORT void CopyValues(FactorPowerCP other);
    UNITS_EXPORT void Merge(FactorPowerCP fp1, FactorPowerCP fp2);
    const int GetDivisor() {return static_cast<int>(m_divisor);}
    const size_t GetPower() {return m_power;}
    const int GetIndex() {return m_index;}
    UNITS_EXPORT const size_t GetFactor();
    UNITS_EXPORT Utf8String ToText(Utf8Char pref);
};

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct FactorizedNumber
{
private:
    size_t m_ival;
    bvector<FactorPower> m_factors;
    static const size_t* GetPrimes(int* length);
    static size_t PowerOfPrime(size_t ival, size_t prim, size_t* result);
    size_t RestoreNumber();

public:
    UNITS_EXPORT static size_t GetPrimeCount();
    UNITS_EXPORT FactorizedNumber(size_t ival);
    bvector<FactorPower> GetFactors() {return m_factors;}
    void ResetFactors(bvector<FactorPower> fact);
    UNITS_EXPORT FactorPowerP FindDivisor(int div);
    UNITS_EXPORT size_t GetGreatestCommonFactor(FactorizedNumber other);
    size_t GetValue() {return m_ival;}
    UNITS_EXPORT Utf8String ToText();
    UNITS_EXPORT Utf8String DebugText();
};

//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz  10/2016
//=======================================================================================
struct FractionalNumeric
{
private:
    int64_t m_integral;
    size_t m_numerator;
    size_t m_denominator;
    size_t m_gcf;
    bvector<Utf8String> m_textParts;

    void Calculate(double dval, size_t denominator);
    static size_t GCF(size_t numer, size_t denom);
public:
    size_t GetDenominator() {return m_denominator;}
    size_t GetNumerator() {return m_numerator;}
    int64_t GetIntegral() {return m_integral;}
    UNITS_EXPORT FractionalNumeric(double dval, FractionalPrecision fprec);
    UNITS_EXPORT FractionalNumeric(double dval, int denominator);
    UNITS_EXPORT FractionalNumeric(double dval, int denominatorBase, double precision);
    UNITS_EXPORT Utf8String ToTextDefault(bool reduce);
    UNITS_EXPORT Utf8String GetIntegralString();
    UNITS_EXPORT Utf8String GetDenominatorString();
    UNITS_EXPORT Utf8String GetNumeratorString();
    UNITS_EXPORT void FormTextParts(bool reduce);
    bool HasFractionPart() {return 1 < m_textParts.size();}
    bool IsZero() {return (0 == m_numerator);}
};

END_BENTLEY_FORMATTING_NAMESPACE
