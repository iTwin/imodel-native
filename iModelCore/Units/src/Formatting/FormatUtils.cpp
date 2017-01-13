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

double Utils::DecimalPrecisionFactor(DecimalPrecision decP, int index = -1)
    {
    static double FactorSet[13] = { 1.0, 10.0, 100.0, 1.0e3, 1.0e4, 1.0e5, 1.0e6, 1.0e7, 1.0e8, 1.0e9, 1.0e10, 1.0e11, 1.0e12 };
    if (0 <= index && sizeof(FactorSet) / sizeof(double) > index)
        return FactorSet[index];
    return FactorSet[DecimalPrecisionToInt(decP)];
    }

Utf8CP Utils::GetParameterCategoryName(ParameterCategory parcat)
    {
    static Utf8CP CategoryNames[] = { "DataType", "Sign", "Presentation", "Zeroes", "DecPrecision", "FractPrecision", "RoundType",
        "FractionBar", "AngleFormat", "Alignment", "Separator", "Padding", "Mapping" };
    return CategoryNames[static_cast<int>(parcat)];
    }

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

  const int Utils::FractionalPrecisionDenominator(FractionalPrecision prec)
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
     Utf8String perfTxt = (m_lastInterval > 0.0) ? nfmt.FormatRoundedDouble((double)amount * 1.0e6 / m_lastInterval, 0.5) : "n/a";

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

 FractionalNumeric::FractionalNumeric(double dval, FractionalPrecision fprec)
     {
      m_integral = dval;
      m_numerator = 0;
      m_denominator = Utils::FractionalPrecisionDenominator(fprec);
      
     }

END_BENTLEY_FORMATTING_NAMESPACE