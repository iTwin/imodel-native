/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/FormatUtils.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/Formatting.h>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
DecimalPrecision Utils::DecimalPrecisionByIndex(size_t num)
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
double Utils::DecimalPrecisionFactor(DecimalPrecision decP, int index = -1)
    {
    static double FactorSet[13] = { 1.0, 10.0, 100.0, 1.0e3, 1.0e4, 1.0e5, 1.0e6, 1.0e7, 1.0e8, 1.0e9, 1.0e10, 1.0e11, 1.0e12 };
    if (0 <= index && sizeof(FactorSet) / sizeof(double) > index)
        return FactorSet[index];
    return FactorSet[DecimalPrecisionToInt(decP)];
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
Utf8CP Utils::GetParameterCategoryName(ParameterCategory parcat)
    {
    static Utf8CP CategoryNames[] = { "DataType", "Sign", "Presentation", "Zeroes", "DecPrecision", "FractPrecision", "RoundType",
        "FractionBar", "AngleFormat", "Alignment", "Separator", "Padding", "Mapping" };
    return CategoryNames[static_cast<int>(parcat)];
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
Utf8String  Utils::PresentationTypeName(PresentationType type)
    {
    switch (type)
        {
        case PresentationType::Fractional: return FormatConstant::FPN_Fractional();
        case PresentationType::Scientific: return FormatConstant::FPN_Scientific();
        case PresentationType::ScientificNorm: return FormatConstant::FPN_Scientific();
        default:
        case PresentationType::Decimal: return FormatConstant::FPN_Decimal();
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
Utf8String Utils::SignOptionName(ShowSignOption opt)
    {
    switch (opt)
        {
        case ShowSignOption::NoSign: return FormatConstant::FPN_NoSign();
        case ShowSignOption::SignAlways: return FormatConstant::FPN_SignAlways();
        case ShowSignOption::NegativeParentheses: return FormatConstant::FPN_NegativeParenths();
        default:
        case ShowSignOption::OnlyNegative: return FormatConstant::FPN_OnlyNegative();
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
Utf8String Utils::DecimalPrecisionName(DecimalPrecision prec)
    {
    switch (prec)
        {
        case DecimalPrecision::Precision1: return FormatConstant::FPN_Precision1();
        case DecimalPrecision::Precision2: return FormatConstant::FPN_Precision2();
        case DecimalPrecision::Precision3: return FormatConstant::FPN_Precision3();
        case DecimalPrecision::Precision4: return FormatConstant::FPN_Precision4();
        case DecimalPrecision::Precision5: return FormatConstant::FPN_Precision5();
        case DecimalPrecision::Precision6: return FormatConstant::FPN_Precision6();
        case DecimalPrecision::Precision7: return FormatConstant::FPN_Precision7();
        case DecimalPrecision::Precision8: return FormatConstant::FPN_Precision8();
        case DecimalPrecision::Precision9: return FormatConstant::FPN_Precision9();
        case DecimalPrecision::Precision10: return FormatConstant::FPN_Precision10();
        case DecimalPrecision::Precision11: return FormatConstant::FPN_Precision11();
        case DecimalPrecision::Precision12: return FormatConstant::FPN_Precision12();
        default:
        case DecimalPrecision::Precision0: return FormatConstant::FPN_Precision0();
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
 Utf8String Utils::FractionallPrecisionName(FractionalPrecision prec)
    {
    switch (prec)
        {
        case FractionalPrecision::Half: return FormatConstant::FPN_FractPrec2();
        case FractionalPrecision::Quarter: return FormatConstant::FPN_FractPrec2();
        case FractionalPrecision::Eighth: return FormatConstant::FPN_FractPrec2();
        case FractionalPrecision::Sixteenth: return FormatConstant::FPN_FractPrec2();
        case FractionalPrecision::Over_32: return FormatConstant::FPN_FractPrec2();
        case FractionalPrecision::Over_64: return FormatConstant::FPN_FractPrec2();
        case FractionalPrecision::Over_128: return FormatConstant::FPN_FractPrec2();
        case FractionalPrecision::Over_256: return FormatConstant::FPN_FractPrec2();
        default:
        case FractionalPrecision::Whole: return FormatConstant::FPN_FractPrec1();
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
FractionalPrecision Utils::FractionalPrecisionByDenominator(size_t prec)
    {
    switch (prec)
        {
        case 2: return FractionalPrecision::Half;
        case 4: return FractionalPrecision::Quarter;
        case 8: return FractionalPrecision::Eighth;
        case 16: return FractionalPrecision::Sixteenth;
        case 32: return FractionalPrecision::Over_32;
        case 64: return FractionalPrecision::Over_64;
        case 128: return FractionalPrecision::Over_128;
        case 256: return FractionalPrecision::Over_256;
        case 1:
        default:return FractionalPrecision::Whole;
        }
    }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //----------------------------------------------------------------------------------------
const size_t Utils::FractionalPrecisionDenominator(FractionalPrecision prec)
    {
    switch (prec)
        {
        case FractionalPrecision::Half: return 2;
        case FractionalPrecision::Quarter: return 4;
        case FractionalPrecision::Eighth: return 8;
        case FractionalPrecision::Sixteenth: return 16;
        case FractionalPrecision::Over_32: return 32;
        case FractionalPrecision::Over_64: return 64;
        case FractionalPrecision::Over_128: return 128;
        case FractionalPrecision::Over_256: return 256;
        default:
        case FractionalPrecision::Whole: return 1;
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
size_t Utils::AppendText(Utf8P buf, size_t bufLen, size_t index, Utf8CP str)
    {
        int cap = static_cast<int>(bufLen) - static_cast<int>(index) - 1;
        size_t strL = (nullptr == str) ? 0 : strlen(str);
        if (strL < 1 || cap < 1)
            return index;
        if (static_cast<int>(strL) > cap)
            strL = static_cast<size_t>(cap);
        memcpy(static_cast<void*>(buf + index), str, strL);
        index += strL;
        buf[index] = '\0';     
        return index;
    }


 //===================================================
 //
 // FormatStopWatchMethods
 //
 //===================================================
 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 FormatStopWatch::FormatStopWatch()
     {
     m_start = std::chrono::steady_clock::now();
     m_lastInterval = 0.0;
     m_totalElapsed = 0.0;
     m_lastAmount = 0;
     m_totalAmount = 0;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 Utf8String FormatStopWatch::LastIntervalMetrics(size_t amount)
     {
     //m_lastInterval = GetElapsedSeconds();
     std::chrono::steady_clock::time_point moment = std::chrono::steady_clock::now();
     m_lastInterval = (double)std::chrono::duration_cast<std::chrono::microseconds>(moment - m_start).count();
     m_totalElapsed += m_lastInterval;
     m_lastAmount = amount;
     m_totalAmount += amount;
     NumericFormatSpec nfmt = NumericFormatSpec("LIM", 6);
     nfmt.SetUse1000Separator(true);
     Utf8String amTxt = nfmt.FormatInteger((int)amount);
     Utf8String duraTxt = (amount > 0) ? nfmt.FormatDouble(m_lastInterval / (double)amount) : "n/a";
     Utf8String perfTxt = (m_lastInterval > 0.0) ? nfmt.FormatRoundedDouble((double)amount * 1.0e6 / m_lastInterval, 0.5) : "n/a";

     char buf[256];
     sprintf(buf, "Completed %s op's average duration %s mksec performance: %s op/sec", amTxt.c_str(), duraTxt.c_str(), perfTxt.c_str());
     return buf;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
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

 //===================================================
 //
 // FractionalNumeric
 //
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
 FractionalNumeric::FractionalNumeric(double dval, int denominator)
     {
     Calculate(dval, denominator);
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 FractionalNumeric::FractionalNumeric(double dval, int denominatorBase, double precision)
     {
     double maxPrec = 1.0;
     denominatorBase = abs(denominatorBase); // it must be positive
     double absDenom = (double)denominatorBase;
     int denominator = denominatorBase;
     if (denominator > 0)
        maxPrec /= absDenom;

     if (!FormatConstant::IsNegligible(precision))
         {
         while (maxPrec > precision)
             {
             maxPrec /= absDenom;
             denominator *= denominatorBase;
             }
         }
     Calculate(dval, denominator);
     }


 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
  void FractionalNumeric::FormTextParts(bool reduce)
     {
     NumericFormatSpec fmt =  NumericFormatSpec("fract", PresentationType::Decimal, ShowSignOption::OnlyNegative, FormatTraits::DefaultZeroes, 0);
     size_t numer = m_numerator;
     size_t denom = m_denominator;
     if (reduce && m_gcf > 1)
         {
         numer /= m_gcf;
         denom /= m_gcf;
         }
     m_textParts.push_back(fmt.FormatInteger((int)m_integral));
     if (numer > 0)
         {
         m_textParts.push_back(fmt.FormatInteger((int)numer));
         m_textParts.push_back(fmt.FormatInteger((int)denom));
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

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //----------------------------------------------------------------------------------------
 Utf8CP FractionalNumeric::GetIntegralText()
     {
     Utf8CP p = GetIntegralString().c_str();
     return p;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //----------------------------------------------------------------------------------------
 Utf8CP FractionalNumeric::GetDenominatorText()
     {
     Utf8CP p = GetDenominatorString().c_str();
     return p;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //----------------------------------------------------------------------------------------
 Utf8CP FractionalNumeric::GetNumeratorText()
     {
     Utf8CP p = GetNumeratorString().c_str();
     return p;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //----------------------------------------------------------------------------------------
 Utf8String FractionalNumeric::ToTextDefault(bool reduce)
     {
     FormTextParts(reduce);
     if (m_numerator > 0)
        return GetIntegralString() + " " + GetNumeratorString() + "/" + GetDenominatorString();
     else
        return GetIntegralString();
     }

 //===================================================
 //
 // FactorPower
 //
 //===================================================
 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 const size_t FactorPower::GetFactor()
     {
     size_t res = 1;
     
     for(size_t i = 0; i < m_power; i++)
         {
         res *= m_divisor;
         }
     return res;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 11/16
 //----------------------------------------------------------------------------------------
 void FactorPower::CopyValues(FactorPowerP other)
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
 void FactorPower::Merge(FactorPowerP fp1, FactorPowerP fp2)
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
 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 Utf8String FactorPower::ToText(Utf8Char pref)
     {
     char buf[128];
     memset(buf, 0, sizeof(buf));
     int i = 0;
     if (0 != pref)
         buf[i++] = pref;
     for (size_t n = 0; n < m_power; n++)
         {
         if (n > 0)
             buf[i++] = 'x';
         i += NumericFormatSpec::FormatIntegerSimple (static_cast<int>(m_divisor), buf + i, static_cast<int>(sizeof(buf)) - i, false, false);
         }
     return Utf8String(buf);
     }
 //===================================================
 //
 // FactorizedNumber
 //
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
 size_t FactorizedNumber::RestoreNumber()
     {
     m_ival = 0;
     FactorPowerP fpp;
     if (m_factors.size() > 0)
         {
         m_ival = 1;
         for (auto curr = m_factors.begin(), end = m_factors.end(); curr != end; curr++)
            {
             fpp = curr;
             m_ival *= fpp->GetFactor();
             //div = fpp->GetDivisor();
             //for (int i = 0; i < fpp->GetPower(); i++)
             //    {
             //    m_ival *= div;
             //    }
            }
         }
     return m_ival;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 FactorPowerP FactorizedNumber::FindDivisor(int div)
     {
     FactorPowerP fpp;
     for (auto curr = m_factors.begin(), end = m_factors.end(); curr != end; curr++)
         {
         fpp = curr;
         if (div == fpp->GetDivisor())
             return fpp;
         }
     return nullptr;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 void FactorizedNumber::ResetFactors(bvector<FactorPower> fact)
     {
     m_factors.clear();
     if (fact.size() < 1)
         return;
     FactorPowerP fpp;
     for (auto curr = fact.begin(), end = fact.end(); curr != end; curr++)
         {
         fpp = curr;
         if (0 < fpp->GetPower())
             m_factors.push_back(*curr);
         }
     RestoreNumber();
     return;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 Utf8String FactorizedNumber::ToText()
     {
     Utf8String txt = "1";
     FactorPowerP fpp;
     if (m_factors.size() > 0)
         {
         for (auto curr = m_factors.begin(), end = m_factors.end(); curr != end; curr++)
             {
             fpp = curr;
             txt += fpp->ToText('x');
             }
         }
     return txt;
     }

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
PUSH_MSVC_IGNORE(6385) // static analysis thinks we exceed the bounds of fact3... I don't see how.
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
POP_MSVC_IGNORE

 //----------------------------------------------------------------------------------------
 // @bsimethod                                                   David Fox-Rabinovitz 01/17
 //----------------------------------------------------------------------------------------
 Utf8String FactorizedNumber::DebugText()
     {
     char buf[256];
     sprintf(buf, "Value %d  (factors) %s ", static_cast<int>(m_ival), ToText().c_str());
     return Utf8String(buf);
     }
 

END_BENTLEY_FORMATTING_NAMESPACE


