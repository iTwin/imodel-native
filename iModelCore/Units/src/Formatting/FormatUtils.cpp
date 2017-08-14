/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/FormatUtils.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <map>
#include <Formatting/FormattingApi.h>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//===================================================
//
// FormatConstant Methods
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
bool FormatConstant::GetTrailingBits(unsigned char c, Utf8P outBits)
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

size_t  FormatConstant::ExtractTrailingBits(unsigned char c, size_t shift) 
    { 
    size_t cod = c & UTF_TrailingBitsMask();
    if (shift > 0)
        cod <<= shift;
    return cod; 
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
const unsigned char FormatConstant::TriadBitMask(size_t threeBit)
    {
    static unsigned char mask[8] = { 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80 };
    threeBit &= 0x7;
    return mask[threeBit];
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
const size_t* FormatConstant::FractionCodes()
    {
                          // 1/4  1/2  3/4  1/3   2/3   1/5   2/5   3/5   4/5   1/6   5/6   1/8   3/8   5/8   7/8
    static size_t cod[16] = {188, 189, 190, 8531, 8532, 8533, 8534, 8535, 8536, 8537, 8538, 8539, 8540, 8541, 8542 };
    return cod;
    }

const FormatSpecialCodes FormatConstant::SpecialtyMap(Utf8CP name)
    {
    static bmap<Utf8String, FormatSpecialCodes> map;
    if (map.empty())
        {
        map[Utf8String("NU")] = FormatSpecialCodes::SignatureNU;
        map[Utf8String("NFU")] = FormatSpecialCodes::SignatureNFU;
        map[Utf8String("NUNU")] = FormatSpecialCodes::SignatureNUNU;
        map[Utf8String("NUNFU")] = FormatSpecialCodes::SignatureNUNFU;
        map[Utf8String("NUNUNU")] = FormatSpecialCodes::SignatureNUNUNU;
        map[Utf8String("NUNUNFU")] = FormatSpecialCodes::SignatureNUNUNFU;
        }    
    return Utf8String::IsNullOrEmpty(name) ? FormatSpecialCodes::SignatureNull : map[name];
    }

const FormatSpecialCodes FormatConstant::ParsingPatternCode(Utf8CP name)
    {
    if(Utf8String::IsNullOrEmpty(name)) return FormatSpecialCodes::SignatureNull;
    if (strcmp("N", name) == 0) return FormatSpecialCodes::SignatureN;
    if (strcmp("F", name) == 0) return FormatSpecialCodes::SignatureF;
    if (strcmp("NF", name) == 0) return FormatSpecialCodes::SignatureNF;
    if (strcmp("NU", name) == 0) return FormatSpecialCodes::SignatureNU;
    if (strcmp("FU", name) == 0) return FormatSpecialCodes::SignatureNU;
    if (strcmp("NFU", name) == 0) return FormatSpecialCodes::SignatureNFU;
    if (strcmp("NUNU", name) == 0) return FormatSpecialCodes::SignatureNUNU;
    if (strcmp("FUNU", name) == 0) return FormatSpecialCodes::SignatureNUNU;
    if (strcmp("NUFU", name) == 0) return FormatSpecialCodes::SignatureNUNU;
    if (strcmp("NUNFU", name) == 0) return FormatSpecialCodes::SignatureNUNFU;
    if (strcmp("NUNUNU", name) == 0) return FormatSpecialCodes::SignatureNUNUNU;
    if (strcmp("NUNUNFU", name) == 0) return FormatSpecialCodes::SignatureNUNUNFU;
    if (strcmp("NUNUFU", name) == 0) return FormatSpecialCodes::SignatureNUNUNU;
    return FormatSpecialCodes::SignatureInvalid;
    }

const Utf8CP FormatConstant::SpecialAngleSymbol(Utf8String name)
    {
    if (name.Equals("^")) return "ARC_DEG";
    //if (name.Equals(u8"°")) return "ARC_DEG";   
    if (name.Equals(u8"\xC2\xB0"))return "ARC_DEG";
    if (name.Equals(u8"\xB0"))return "ARC_DEG";
    if (name.Equals("'")) return "ARC_MINUTE";
    if (name.Equals("\"")) return "ARC_SECOND";     
    return nullptr;
    }

const Utf8CP FormatConstant::SpecialLengthSymbol(Utf8String name)
    {
    if (name.EqualsI("'")) return "FT";
    if (name.EqualsI("\"")) return "IN";
    return nullptr;
    }


//===================================================
//
// Utils Methods
//
//===================================================
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

Utf8String Utils::AccumulatorStateName(AccumulatorState state)
    {
    switch (state)
        {
        case AccumulatorState::Init: return "init";
        case AccumulatorState::Complete: return "complete";
        case AccumulatorState::RejectedSymbol: return "rejectSymbol";
        case AccumulatorState::Exponent: return "exponent";
        case AccumulatorState::Fraction: return "fraction";
        case AccumulatorState::Integer: return "integer";
        case AccumulatorState::Failure: return "failure";
        default: return "???";
        }
    }

Utf8String Utils::CharToString(Utf8Char c)
    {
    Utf8Char buf[2];
    buf[0] = c;
    buf[1] = '\0';
    return Utf8String(buf);
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

Utf8String  Utils::FractionBarName(FractionBarType bar)
    {
    switch (bar)
        {
        case FractionBarType::Diagonal: return FormatConstant::FPN_FractBarDiagonal();
        case FractionBarType::Oblique: return FormatConstant::FPN_FractBarOblique();
        case FractionBarType::Horizontal: return FormatConstant::FPN_FractBarHoriz();
        case FractionBarType::None:
        default:  return "";
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
        case FractionalPrecision::Quarter: return FormatConstant::FPN_FractPrec4();
        case FractionalPrecision::Eighth: return FormatConstant::FPN_FractPrec8();
        case FractionalPrecision::Sixteenth: return FormatConstant::FPN_FractPrec16();
        case FractionalPrecision::Over_32: return FormatConstant::FPN_FractPrec32();
        case FractionalPrecision::Over_64: return FormatConstant::FPN_FractPrec64();
        case FractionalPrecision::Over_128: return FormatConstant::FPN_FractPrec128();
        case FractionalPrecision::Over_256: return FormatConstant::FPN_FractPrec256();
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




//Utf8String Utils::FormatProblemDescription(FormatProblemCode code)
//    {
//    switch (code)
//        {
//        case FormatProblemCode::UnknownStdFormatName: return "Unknown name of the standard format";
//        case FormatProblemCode::UnknownUnitName: return "Unknown name of the unit";
//        case FormatProblemCode::CNS_InconsistentFactorSet: return "Inconsistent set of factors";
//        case FormatProblemCode::CNS_InconsistentUnitSet: return "Inconsistent set of units";
//        case FormatProblemCode::CNS_UncomparableUnits: return "Units are not comparable";
//        case FormatProblemCode::CNS_InvalidUnitName: return "Unknown name of the Unit";
//        case FormatProblemCode::CNS_InvalidMajorUnit: return "Unknown name of the Major Unit";
//        case FormatProblemCode::QT_PhenomenonNotDefined: return "Unknown name of the Phenomenon";
//        case FormatProblemCode::QT_PhenomenaNotSame: return "Different Phenomena";
//        case FormatProblemCode::QT_InvalidTopMidUnits: return "Top and Middle units are not comparable";
//        case FormatProblemCode::QT_InvalidMidLowUnits: return "Middle and Low units are not comparable";
//        case FormatProblemCode::QT_InvalidUnitCombination: return "Invalid Unit combination ";
//        case FormatProblemCode::FUS_InvalidSyntax: return "Invalid syntax of FUS";
//        case FormatProblemCode::NoProblems:
//        default: return "No problems";
//        }
//    };


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
        buf[index] = FormatConstant::EndOfLine();     
        return index;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
//----------------------------------------------------------------------------------------
bool Utils::AreUnitsComparable(BEU::UnitCP un1, BEU::UnitCP un2)
    {
    BEU::PhenomenonCP ph1 = (nullptr == un1) ? nullptr : un1->GetPhenomenon();
    BEU::PhenomenonCP ph2 = (nullptr == un2) ? nullptr : un2->GetPhenomenon();
    return (ph1 == ph2);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 03/17
//----------------------------------------------------------------------------------------
Utf8String Utils::AppendUnitName(Utf8CP txtValue, Utf8CP unitName, Utf8CP space)
    {
    size_t txtL = Utils::IsNameNullOrEmpty(txtValue) ? 0 : strlen(txtValue);
    size_t uomL = Utils::IsNameNullOrEmpty(unitName) ? 0 : strlen(unitName);
    size_t spcL = Utils::IsNameNullOrEmpty(space) ? 0 : strlen(space);
    if (0 == txtL)
        txtValue = "";
    if (0 == uomL)
        return txtValue;
    size_t totL = txtL + uomL + spcL + 2;
    Utf8P buf = (char*)alloca(totL);
    BeStringUtilities::Snprintf(buf, totL, "%s%s%s", txtValue, ((0 == spcL)? "":space), unitName);
    return buf;
    }

Utf8CP Utils::HexByte(Utf8Char c, Utf8P buf, size_t bufLen)
    {
    static Utf8CP s = FormatConstant::HexSymbols();
    if (bufLen > 2)
        {
        size_t h = (size_t)(c & 0xF);
        buf[1] = s[h];
        h = (size_t)((c >> 4) & 0xF);
        buf[0] = s[h];
        buf[2] = FormatConstant::EndOfLine();
        }
    else if (nullptr != buf)
        buf[0] = FormatConstant::EndOfLine();
    return buf;
    }

size_t Utils::NumberOfUtf8Bytes(size_t code)
    {
    if (code < 0x80) return 1;
    if (code <  0x800) return 2;
    if (code < 0x10000) return 3;
    if (code < 0x200000) return 4;
    if (code < 0x4000000) return 5;
    return 6;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
Utf8String Utils::HexDump(Utf8CP txt, int len)
    {
    Utf8String str;
    int totL = 16;
    Utf8P buf = (char*)alloca(totL);
    buf[0] = ' ';
    buf[1] = '0';
    buf[2] = 'x';
    if (!Utils::IsNameNullOrEmpty(txt))
        {
        size_t actLen = strlen(txt);
        if (len < actLen)
            actLen = len;
        for (int i = 0; i < actLen; i++)
            {
            HexByte(txt[i], buf + 3, 10);
            str += Utf8String(buf);
            }
        }
    return str;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
bool Utils::IsNameNullOrEmpty(Utf8CP name) 
    { 
    size_t len = (nullptr == name) ? 0 : strlen(name);
     return (len == 0); 
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
Utf8Char Utils::MatchingDivider(Utf8Char div)
    {
    Utf8CP fd= FormatConstant::FUSDividers();
    Utf8CP df = FormatConstant::FUSDividerMatch();
    for (int i = 0; fd[i] != FormatConstant::EndOfLine(); i++)
        {
        if (div == fd[i])
            return df[i];
        }
    return FormatConstant::EndOfLine();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
int Utils::IndexOf(Utf8Char c, Utf8CP text)
    {
    int indx = -1;
    if (Utils::IsNameNullOrEmpty(text))
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
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 01/17
// this function scans through the given text string and returns a specially coded text string
//  representing types of the detected charactes using ASCII letters:
//    a - ASCII symbol occupying 1 byte
//    d - ASCII digit occupying 1 byte
//    . - ASCII point 
//    , - ASCII comma
//    e - 
//    l - Latin Char occupying 2 bytes
//    u - Unicode occupying 3 bytes
//    g - Unicode glyph occupying 4 bytes
//    v - Unicode char occupying 5 bytes  (reserved)
//    w - Unicode char occupying 6 bytes  (reserved)
//----------------------------------------------------------------------------------------
//Utf8String Utils::GetSignature(Utf8CP text)
//    {
//
//    }

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
     NumericFormatSpec nfmt = NumericFormatSpec(6);
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
     NumericFormatSpec fmt =  NumericFormatSpec(PresentationType::Decimal, ShowSignOption::OnlyNegative, FormatTraits::DefaultZeroes, 0);
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
 
//===================================================
//
// FormatUnitSet
//
//===================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
FormatUnitSet::FormatUnitSet(NamedFormatSpecCP format, BEU::UnitCP unit)
    {
    m_formatSpec = format;
    m_unitName = Utf8String(unit->GetName());
    m_unit = unit;
    if (nullptr == m_formatSpec)
        m_problem.UpdateProblemCode(FormatProblemCode::UnknownStdFormatName);
    else if (nullptr == m_unit)
        m_problem.UpdateProblemCode(FormatProblemCode::UnknownUnitName);
    else
        m_problem.UpdateProblemCode(FormatProblemCode::NoProblems);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
FormatUnitSet::FormatUnitSet(Utf8CP formatName, Utf8CP unitName)
    {
    m_problem = FormatProblemDetail();
    m_unit = nullptr;
    m_formatSpec = StdFormatSet::FindFormatSpec(formatName);
    if (nullptr == m_formatSpec)
        m_problem.UpdateProblemCode(FormatProblemCode::UnknownStdFormatName);
    else
        {
        m_unit = BEU::UnitRegistry::Instance().LookupUnit(unitName);
        if (nullptr == m_unit)
            m_problem.UpdateProblemCode(FormatProblemCode::UnknownUnitName);
        else
            m_unitName = unitName;
        }
    }

//----------------------------------------------------------------------------------------
//  The text string has format <unitName>(<formatName>)
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
FormatUnitSet::FormatUnitSet(Utf8CP description)
    {
    m_problem = FormatProblemDetail();
    FormattingScannerCursor curs = FormattingScannerCursor(description, -1, FormatConstant::FUSDividers());
    FormattingWord fnam;
    FormattingWord unit;
    FormatDividerInstance fdt;
    FormatDividerInstance fdi = FormatDividerInstance(description, '|'); // check if this is a new format
    int n = fdi.GetDivCount();
    if (n == 2 && fdi.IsDivLast())
        {
        fnam = curs.ExtractLastEnclosure();
        unit = curs.ExtractBeforeEnclosure();
        }
    else if (n == 1 && !fdi.IsDivLast())
        {
        int loc = fdi.GetFirstLocation();
        if (loc > 0)
            {
            unit = curs.ExtractSegment(0, (size_t)loc-1);
            fnam = curs.ExtractSegment((size_t)loc + 1, curs.GetTotalLength());
            }
        }
    else
        {
        fdi = FormatDividerInstance(description, '(');
        n = fdi.GetDivCount();
        if(n ==0)
            unit = curs.ExtractSegment(0, curs.GetTotalLength());
        if (fdi.BracketsMatched() && fdi.IsDivLast()) // there is a candidate for the format in parethesis
            {
            unit = curs.ExtractLastEnclosure(); 
            fdt = FormatDividerInstance(unit.GetText(), "/*");
            if (fdt.GetDivCount() == 0) // it can be a format name
                {
                 fnam = unit;
                 unit = curs.ExtractBeforeEnclosure();
                }
            else
                {
                unit = curs.ExtractSegment(0, curs.GetTotalLength());
                }
            }
         // dividers are not found - we assume a Unit name only
        }

    if (Utf8String::IsNullOrEmpty(fnam.GetText()))
        m_formatSpec = StdFormatSet::FindFormatSpec("DefaultReal");
    else
        m_formatSpec = StdFormatSet::FindFormatSpec(fnam.GetText());
    m_unit = BEU::UnitRegistry::Instance().LookupUnit(unit.GetText());
    if (nullptr == m_formatSpec)
        m_problem.UpdateProblemCode(FormatProblemCode::UnknownStdFormatName);
    else
        {
        if (nullptr == m_unit)
            m_problem.UpdateProblemCode(FormatProblemCode::UnknownUnitName);
        else
            m_unitName = Utf8String(m_unit->GetName());
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
Utf8String FormatUnitSet::ToText(bool useAlias) const
    {
    if (HasProblem())
        return "";
    char buf[256];
    sprintf(buf, "%s(%s)", m_unit->GetName(), (useAlias? m_formatSpec->GetAlias() : m_formatSpec->GetName()));
    return buf;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
bool FormatUnitSet::IsComparable(BEU::QuantityCR qty)
    {
    return Utils::AreUnitsComparable(qty.GetUnit(), m_unit);
    }

bool FormatUnitSet::IsUnitComparable(Utf8CP unitName)
    {
     BEU::UnitCP unit =  BEU::UnitRegistry::Instance().LookupUnit(unitName);
     return Utils::AreUnitsComparable(unit, m_unit);
    }


BEU::UnitCP FormatUnitSet::ResetUnit()
    {
    m_unit = BEU::UnitRegistry::Instance().LookupUnit(m_unitName.c_str());
    return m_unit;
    }



//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
Utf8String FormatUnitSet::FormatQuantity(BEU::QuantityCR qty, Utf8CP space) const
    {
    Utf8String txt = NumericFormatSpec::StdFormatQuantity(*m_formatSpec, qty.ConvertTo(m_unit), nullptr, space);
    return txt;
    }

Json::Value FormatUnitSet::FormatQuantityJson(BEU::QuantityCR qty, bool useAlias) const
    {
    Utf8String str;
    Json::Value jval = ToJson(useAlias);
    BEU::Quantity conv = qty.ConvertTo(m_unit);
    Utf8String txt = NumericFormatSpec::StdFormatQuantity(*m_formatSpec, conv, nullptr, " ");
    jval[FormatConstant::FUSJsonValue()] = conv.GetMagnitude();
    jval[FormatConstant::FUSJsonDispValue()] = txt.c_str();
    return jval;
    }


//===================================================
//
// FormatUnitGroup
//
//===================================================
//----------------------------------------------------------------------------------------
//  The text string has format <unitName>(<formatName>)
// @bsimethod                                                   David Fox-Rabinovitz 03/17
//----------------------------------------------------------------------------------------
FormatUnitGroup::FormatUnitGroup(Utf8CP description)
    {
    FormattingScannerCursor curs = FormattingScannerCursor(description, -1);
    m_problem = FormatProblemDetail();
    curs.SkipBlanks();
    FormattingWord unit = curs.ExtractWord();
    FormattingWord fnam = curs.ExtractWord();
    Utf8Char unitDelim = unit.GetDelim();
    Utf8Char fnamDelim = fnam.GetDelim();
    while (m_problem.NoProblem())
        {
        if ('(' == unitDelim && ')' == fnamDelim)
            {
            FormatUnitSet fus = FormatUnitSet(fnam.GetText(), unit.GetText());
            if (fus.HasProblem())
                m_problem.UpdateProblemCode(fus.GetProblemCode());
            else
                {
                m_group.push_back(fus);
                unit = curs.ExtractWord();
                if (unit.IsEndLine())
                    break;
                if (unit.IsSeparator())
                    {
                    curs.SkipBlanks();
                    unit = curs.ExtractWord();
                    fnam = curs.ExtractWord();
                    unitDelim = unit.GetDelim();
                    fnamDelim = fnam.GetDelim();
                    }
                }
            }
        else
            m_problem.UpdateProblemCode(FormatProblemCode::FUS_InvalidSyntax);
        }
    }


Utf8String FormatUnitGroup::ToText(bool useAlias)
    {
    if (HasProblem())
        return "";
    Utf8String txt;
    int i = 0;

    for (FormatUnitSetP fus = m_group.begin(); fus != m_group.end(); ++fus)
        {
        if (0 < i++)
            txt += ",";
        txt += fus->ToText(useAlias);
        }

    return txt;
    }


FormattingDividers::FormattingDividers(Utf8CP div)
    {
    if (nullptr == div)
        div = " !\"#$%&\'()*+,-./:;<=>? [\\]^{|}";
    memset(m_markers, 0, sizeof(m_markers));
    Utf8CP p = div;
    while (*p != 0)
        {
        m_markers[(*p & 0x78) >> 3] |= FormatConstant::TriadBitMask(*p);
        ++p;
        }
    }

FormattingDividers::FormattingDividers(FormattingDividersCR other)
    {
    memcpy(m_markers, other.m_markers, sizeof(m_markers));
    }

bool FormattingDividers::IsDivider(char c)
    {
    if (0 == c)
        return true;
    return (0 != ((m_markers[(c & 0x78) >> 3]) & (FormatConstant::TriadBitMask(c))));
    }


//===================================================
//
// FormattingWord
//
//===================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
FormattingWord::FormattingWord(FormattingScannerCursorP cursor, Utf8CP buffer, Utf8CP delim, bool isAscii)
    {
    m_cursor = cursor;
    m_word = "";
    size_t len = (nullptr == buffer) ? 0 : strlen(buffer);
    if(0 < len)
        m_word = buffer;
    len = (nullptr == delim) ? 0 : strlen(delim);
    memset(m_delim, 0, sizeof(m_delim));
    if (0 < len)
        memcpy(m_delim, delim, Utils::MinInt(maxDelim, len));
    m_isASCII = isAscii;
    }

//===================================================
//
// NamedFormatSpec
//
//===================================================

 //              NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, CompositeValueSpecCR compSpec, Utf8CP alias = nullptr);
 
 //              NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, CompositeValueSpecCR compSpec, Utf8CP alias = nullptr);
 //              NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, Utf8CP alias = nullptr);
NamedFormatSpec::NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, CompositeValueSpecCR compSpec, Utf8CP alias)
    {
    m_specType = FormatSpecType::Undefined;
    m_alias = alias;
    m_name = name;
    m_numericSpec = NumericFormatSpec(numSpec);
    m_compositeSpec = CompositeValueSpec(compSpec);
    m_specType = FormatSpecType::Composite;
    m_problem = FormatProblemDetail();
    if (Utils::IsNameNullOrEmpty(name))
        m_problem.UpdateProblemCode(FormatProblemCode::NFS_InvalidSpecName);
    }

NamedFormatSpec::NamedFormatSpec(Utf8CP name, NumericFormatSpecCR numSpec, Utf8CP alias)
    {
    m_specType = FormatSpecType::Undefined;
    m_alias = alias;
    m_name = name;
    m_numericSpec = NumericFormatSpec(numSpec);
    m_specType = FormatSpecType::Numeric;
    m_compositeSpec = CompositeValueSpec();
    m_problem = FormatProblemDetail();
    if (Utils::IsNameNullOrEmpty(name))
        m_problem.UpdateProblemCode(FormatProblemCode::NFS_InvalidSpecName);
    }




//===================================================
//
// FormatProblemDetail
//
//===================================================
Utf8String FormatProblemDetail::GetProblemDescription() const
    {
    switch (m_code)
        {
        case FormatProblemCode::UnknownStdFormatName: return "Unknown name of the standard format";
        case FormatProblemCode::UnknownUnitName: return "Unknown name of the unit";
        case FormatProblemCode::CNS_InconsistentFactorSet: return "Inconsistent set of factors";
        case FormatProblemCode::CNS_InconsistentUnitSet: return "Inconsistent set of units";
        case FormatProblemCode::CNS_UncomparableUnits: return "Units are not comparable";
        case FormatProblemCode::CNS_InvalidUnitName: return "Unknown name of the Unit";
        case FormatProblemCode::CNS_InvalidMajorUnit: return "Unknown name of the Major Unit";
        case FormatProblemCode::QT_PhenomenonNotDefined: return "Unknown name of the Phenomenon";
        case FormatProblemCode::QT_PhenomenaNotSame: return "Different Phenomena";
        case FormatProblemCode::QT_InvalidTopMidUnits: return "Top and Middle units are not comparable";
        case FormatProblemCode::QT_InvalidMidLowUnits: return "Middle and Low units are not comparable";
        case FormatProblemCode::QT_InvalidUnitCombination: return "Invalid Unit combination ";
        case FormatProblemCode::FUS_InvalidSyntax: return "Invalid syntax of FUS";
        case FormatProblemCode::NFS_InvalidSpecName: return "Invalid Numeric Format name";
        case FormatProblemCode::NFS_DuplicateSpecName: return "Duplicate Numeric Format name";
        case FormatProblemCode::DIV_UnknownDivider: return "Unknown Divider";
        case FormatProblemCode::NA_InvalidSign: return "Invalid or duplicate sign in numeric definition";         // Numeric Accumulator problems
        case FormatProblemCode::NA_InvalidPoint: return "Invalid or duplicate decimal point in numeric definition";
        case FormatProblemCode::NA_InvalidExponent: return "Invalid or duplicate exponent in numeric definition";
        case FormatProblemCode::NA_InvalidSyntax: return "Invalid symtax of numeric expression";
        case FormatProblemCode::NoProblems:
        default: return "No problems";
        }
    }
/*

FUS_InvalidSyntax = 20151,
NFS_InvalidSpecName = 20161,
NFS_DuplicateSpecName = 20162,
DIV_UnknownDivider = 25001,
NA_InvalidSign = 25101,             // Numeric Accumulator problems
NA_InvalidPoint = 25102,
NA_InvalidExponent = 25103
};

*/

bool FormatProblemDetail::UpdateProblemCode(FormatProblemCode code)
    {
    if (m_code == FormatProblemCode::NoProblems)
        m_code = code;
    return IsProblem();
    }

//===================================================
//
// QuantityFraction
//
//===================================================
//void QuantityFraction::SetSigned(bool set)
//    {
//    size_t temp = static_cast<int>(m_traits);
//    if (set)
//        temp |= static_cast<int>(NumSequenceTraits::Signed);
//    else
//        temp &= ~static_cast<int>(NumSequenceTraits::Signed);
//    m_traits = static_cast<NumSequenceTraits>(temp);
//    }
//
//void QuantityFraction::SetDecPoint(bool set)
//    {
//    size_t temp = static_cast<int>(m_traits);
//    if (set)
//        temp |= static_cast<int>(NumSequenceTraits::DecPoint);
//    else
//        temp &= ~static_cast<int>(NumSequenceTraits::DecPoint);
//    m_traits = static_cast<NumSequenceTraits>(temp);
//    }
//void QuantityFraction::SetExponent(bool set)
//    {
//    size_t temp = static_cast<int>(m_traits);
//    if (set)
//        temp |= static_cast<int>(NumSequenceTraits::Exponent);
//    else
//        temp &= ~static_cast<int>(NumSequenceTraits::Exponent);
//    m_traits = static_cast<NumSequenceTraits>(temp);
//    }
//
//void QuantityFraction::SetUom(bool set)
//    {
//    size_t temp = static_cast<int>(m_traits);
//    if (set)
//        temp |= static_cast<int>(NumSequenceTraits::Uom);
//    else
//        temp &= ~static_cast<int>(NumSequenceTraits::Uom);
//    m_traits = static_cast<NumSequenceTraits>(temp);
//    }
//
//void QuantityFraction::Detect(Utf8CP txt)
//    {
//    Init();
//    int phase = 0;
//    while (m_problem.NoProblem())
//        {
//        switch (phase)
//            {
//            case 0:
//                break;
//            case 1:
//                break;
//            case 2:
//                break;
//            case 3:
//                break;
//            case 4:
//                break;
//            default:
//                break;
//            }
//        }
//    }


END_BENTLEY_FORMATTING_NAMESPACE


