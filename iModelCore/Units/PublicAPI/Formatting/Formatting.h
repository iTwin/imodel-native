/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Formatting/Formatting.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Formatting/FormattingDefinitions.h>
#include <Units/Units.h>
using namespace BentleyApi::Units;

BEGIN_BENTLEY_FORMATTING_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(NumericFormatSpec)
FORMATTING_TYPEDEFS(FormatParameterSet)
FORMATTING_TYPEDEFS(NumericTriad)
DEFINE_POINTER_SUFFIX_TYPEDEFS(QuantityTriadSpec)
DEFINE_POINTER_SUFFIX_TYPEDEFS(StdFormatName)
DEFINE_POINTER_SUFFIX_TYPEDEFS(StdFormatNameMap)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatParameter)
FORMATTING_TYPEDEFS(FormatDictionary)
DEFINE_POINTER_SUFFIX_TYPEDEFS(UnicodeConstant)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormattingScannerCursor)
FORMATTING_REFCOUNTED_TYPEDEFS(NumericFormatSpec)
FORMATTING_TYPEDEFS(StdFormatSet)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FactorPower)
//===================================================
//
// Enumerations
//
//===================================================

enum class ParameterCode
    {
    FormatName = 50,
    NoSign = 101,
    OnlyNegative = 102,
    SignAlways = 103,
    NegativeParenths = 104,
    Decimal = 151,
    Fractional = 152,
    Scientific = 153,
    ScientificNorm = 154,
    Binary = 155,
    DefaultZeroes = 201,
    LeadingZeroes = 202,
    TrailingZeroes = 203,
    KeepSingleZero = 204,
    KeepDecimalPoint = 205,
    ExponentZero = 206,
    ZeroEmpty = 207,    // formatter will return the empy string if the result is 0
    DecPrec0 =  300,
    DecPrec1 =  301,
    DecPrec2 =  302,
    DecPrec3 =  303,
    DecPrec4 =  304,
    DecPrec5 =  305,
    DecPrec6 =  306,
    DecPrec7 =  307,
    DecPrec8 =  308,
    DecPrec9 =  309,
    DecPrec10 = 310,
    DecPrec11 = 311,
    DecPrec12 = 312,
    FractPrec1 = 331,
    FractPrec2 = 332,
    FractPrec4 = 333,
    FractPrec8 = 334,
    FractPrec16 = 335,
    FractPrec32 = 336,
    FractPrec64 = 337,
    FractPrec128 = 338,
    FractPrec256 = 339,
    DecimalComma = 351,
    DecimalPoint = 352,
    DecimalSepar = 353,
    ThousandSepComma = 354,
    ThousandSepPoint = 355,
    ThousandsSepar = 356,
    RoundUp = 401,
    RoundDown = 402,
    RoundToward0 = 403,
    RoundAwayFrom0 = 404,
    FractBarHoriz = 451,
    FractBarOblique = 452,
    FractBarDiagonal = 453,
    AngleRegular = 501,
    AngleDegMin = 502,
    AngleDegMinSec = 503,
    PaddingSymbol = 504,
    BoudaryLen = 601,
    CenterAlign = 620,
    LeftAlign = 621,
    RightAlign = 622,
    MapName = 651,
    };

enum class ParameterDataType
    {
    Flag = 0,
    BitFlag = 1,
    Integer = 2,
    Double = 3,
    Symbol = 4,
    String = 5
    };

enum class RoundingType
    {
    RoundUp = 1,
    RoundDown = 2,
    RoundToward0 = 3,
    RoundAwayFrom0 = 4
    };

enum class FractionBarType
    {
    Oblique = 0,
    Horizontal = 1,
    FractBarDiagonal = 2
    };

enum class AngleFormatType
    {
    AngleRegular = 0,
    AngleDegMin = 1,
    AngleDegMinSec = 2
    };

enum class FieldAlignment
    {
    Center = 0,
    Left = 1,
    Right = 2
    };

enum class ShowSignOption
    {
    NoSign = 0,
    OnlyNegative = 1,
    SignAlways = 2,
    NegativeParentheses = 3
    };

enum class PresentationType
    {
    Decimal = 1,
    Fractional = 2,
    Scientific = 3,      // scientific with 1 digit before the decimal point
    ScientificNorm = 4   // normalized scientific when Mantissa is < 1
    };

enum class FormatTraits: int
    {
    DefaultZeroes = 0,
    LeadingZeroes = 0x1,
    TrailingZeroes = 0x2,
    KeepDecimalPoint = 0x4,
    KeepSingleZero = 0x8,
    ExponentZero = 0x10,
    ZeroEmpty = 0x20,
    Use1000Separator = 0x40,
    ApplyRounding = 0x80
    };

enum class DecimalPrecision
    {
    Precision0 = 0,
    Precision1 = 1,
    Precision2 = 2,
    Precision3 = 3,
    Precision4 = 4,
    Precision5 = 5,
    Precision6 = 6,
    Precision7 = 7,
    Precision8 = 8,
    Precision9 = 9,
    Precision10 = 10,
    Precision11 = 11,
    Precision12 = 12
    };

enum class FractionalPrecision
    {
    Whole = 0,       //!< Ex. 30
    Half = 1,        //!< Ex. 30 1/2
    Quarter = 2,      //!< Ex. 30 1/4
    Eighth = 3,      //!< Ex. 30 1/8
    Sixteenth = 4,      //!< Ex. 30 1/16
    Over_32 = 5,      //!< Ex. 30 1/32
    Over_64 = 6,      //!< Ex. 30 1/64
    Over_128 = 7,      //!< Ex. 30 1/128
    Over_256 = 8,      //!< Ex. 30 1/256
    };

enum class ParameterCategory
    {
    DataType = 1,
    Sign = 2,
    Presentation = 3,
    Zeroes = 4,
    DecPrecision = 6,
    FractPrecision = 7,
    RoundType = 8,
    FractionBar = 9,
    AngleFormat = 10,
    Alignment = 11,
    Separator = 12,
    Padding = 13,
    Mapping = 14
    };

enum class ScannerCursorStatus
    {
    Success = 0,
    InvalidSymbol = 1,
    IncompleteSequence = 2
    };

enum class StdFormatCode
    {
    DefaultReal = 100,
    SignedReal = 150,
    ParenthsReal = 200,
    DefaultFractional = 300,
    SignedFractional = 350,
    DefaultExp = 400,
    NormalExp  = 450,
    SignedExp = 500,
    DefaultInt = 600
    };

enum class FormatProblemCode
    {
    NoProblems = 0,
    QT_PhenomenonNotDefined = 101,
    QT_PhenomenaNotSame = 102,
    QT_InvalidTopMidUnits = 103,
    QT_InvalidMidLowUnits = 104,
    QT_InvalidUnitCombination = 105
    };

struct Utils
    {
    UNITS_EXPORT static Utf8String ShowSignOptionName(ShowSignOption opt);
    static int DecimalPrecisionToInt(DecimalPrecision decP) { return static_cast<int>(decP); }
    UNITS_EXPORT static DecimalPrecision DecimalPrecisionByIndex(size_t num);
    UNITS_EXPORT static double DecimalPrecisionFactor(DecimalPrecision decP, int index);
    UNITS_EXPORT static Utf8CP GetParameterCategoryName(ParameterCategory parcat);
    UNITS_EXPORT static Utf8String PresentationTypeName(PresentationType type);
    UNITS_EXPORT static Utf8String SignOptionName(ShowSignOption opt);
    UNITS_EXPORT static Utf8String DecimalPrecisionName(DecimalPrecision prec);
    UNITS_EXPORT static Utf8String FractionallPrecisionName(FractionalPrecision prec);
    UNITS_EXPORT static FractionalPrecision FractionalPrecisionByDenominator(size_t prec);
    static int FormatTraitsBit(FormatTraits zcValue) { return static_cast<int>(zcValue); }
    UNITS_EXPORT static const size_t FractionalPrecisionDenominator(FractionalPrecision prec);
    UNITS_EXPORT static size_t AppendText(Utf8P buf, size_t bufLen, size_t index, Utf8CP str);
    //#if defined(FUNCTION_NOT_USED)
    //int StdFormatCodeValue(StdFormatCode code) { return static_cast<int>(code); }
    //static double DecimalPrecisionFactor(DecimalPrecision decP, int index = -1);

    //#endif
    };

// A collection of important "global" constants that are used across the whole formatting domain
//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz  11/2016                                                
//=======================================================================================
struct FormatConstant
    {
public:
    // FPN prefix stands for FormatParameterName
    static Utf8String FPN_FormatName() { return "FormatName"; }
    static Utf8String FPN_NoSign() { return "NoSign"; }
    static Utf8String FPN_OnlyNegative() { return "OnlyNegative"; }
    static Utf8String FPN_SignAlways() { return "SignAlways"; }
    static Utf8String FPN_NegativeParenths() { return "NegativeParenths"; }
    static Utf8String FPN_Decimal() { return "Decimal"; }
    static Utf8String FPN_Fractional() { return "Fractional"; }
    static Utf8String FPN_Scientific() { return "Scientific"; }
    static Utf8String FPN_ScientificNorm() { return "ScientificNorm"; }
    static Utf8String FPN_Binary() { return "Binary"; }
    static Utf8String FPN_DefaultZeroes() { return "DefaultZeroes"; }
    static Utf8String FPN_LeadingZeroes() { return "LeadingZeroes"; }
    static Utf8String FPN_TrailingZeroes() { return "TrailingZeroes"; }
    static Utf8String FPN_KeepDecimalPoint() { return "KeepDecimalPoint"; }
    static Utf8String FPN_ZeroEmpty() { return "ZeroEmpty"; }
    static Utf8String FPN_KeepSingleZero() { return "KeepSingleZero"; }
    static Utf8String FPN_ExponentZero() { return "ExponentZero"; }
    static Utf8String FPN_Precision0() { return "Precision0"; }
    static Utf8String FPN_Precision1() { return "Precision1"; }
    static Utf8String FPN_Precision2() { return "Precision2"; }
    static Utf8String FPN_Precision3() { return "Precision3"; }
    static Utf8String FPN_Precision4() { return "Precision4"; }
    static Utf8String FPN_Precision5() { return "Precision5"; }
    static Utf8String FPN_Precision6() { return "Precision6"; }
    static Utf8String FPN_Precision7() { return "Precision7"; }
    static Utf8String FPN_Precision8() { return "Precision8"; }
    static Utf8String FPN_Precision9() { return "Precision9"; }
    static Utf8String FPN_Precision10() { return "Precision10"; }
    static Utf8String FPN_Precision11() { return "Precision11"; }
    static Utf8String FPN_Precision12() { return "Precision12"; }
    static Utf8String FPN_FractPrec1() { return "FractPrec1"; }
    static Utf8String FPN_FractPrec2() { return "FractPrec2"; }
    static Utf8String FPN_FractPrec4() { return "FractPrec4"; }
    static Utf8String FPN_FractPrec8() { return "FractPrec8"; }
    static Utf8String FPN_FractPrec16() { return "FractPrec16"; }
    static Utf8String FPN_FractPrec32() { return "FractPrec32"; }
    static Utf8String FPN_FractPrec64() { return "FractPrec64"; }
    static Utf8String FPN_FractPrec128() { return "FractPrec128"; }
    static Utf8String FPN_FractPrec256() { return "FractPrec256"; }
    static Utf8String FPN_DecimalComma() { return "DecimalComma"; }
    static Utf8String FPN_DecimalPoint() { return "DecimalPoint"; }
    static Utf8String FPN_DecimalSepar() { return "DecimalSepar"; }
    static Utf8String FPN_ThousandSepComma() { return "ThousandSepComma"; }
    static Utf8String FPN_ThousandSepPoint() { return "ThousandSepPoint"; }
    static Utf8String FPN_ThousandsSepar() { return "ThousandsSepar"; }
    static Utf8String FPN_Use1000Separ() { return "Use1000Separator"; }
    static Utf8String FPN_RoundUp() { return "RoundUp"; }
    static Utf8String FPN_RoundDown() { return "RoundDown"; }
    static Utf8String FPN_RoundToward0() { return "RoundToward0"; }
    static Utf8String FPN_RoundAwayFrom0() { return "RoundFrom0"; }
    static Utf8String FPN_FractBarHoriz() { return "FractBarHoriz"; }
    static Utf8String FPN_FractBarOblique() { return "FractBarOblique"; }
    static Utf8String FPN_FractBarDiagonal() { return "FractBarDiagonal"; }
    static Utf8String FPN_AngleRegular() { return "AngleRegular"; }
    static Utf8String FPN_AngleDegMin() { return "AngleDegMin"; }
    static Utf8String FPN_AngleDegMinSec() { return "AngleDegMinSec"; }
    static Utf8String FPN_PaddingSymbol() { return "PaddingSymbol"; }
    static Utf8String FPN_CenterAlign() { return "CenterAlign"; }
    static Utf8String FPN_LeftAlign() { return "LeftAlign"; }
    static Utf8String FPN_RightAlign() { return "RightAlign"; }
    static Utf8String FPN_MapName() { return "MapName"; }
    static const double FPV_MinTreshold() { return 1.0e-15; }  // format parameter default values
    static const double FPV_RoundFactor() { return 0.50000000001; }  // rounding additive
    static const Utf8Char FPV_DecimalSeparator() { return '.'; }
    static const Utf8Char FPV_ThousandSeparator() { return ','; }
    static const Utf8CP DefaultName() { return "*"; }
    static const Utf8CP EmptyString() { return ""; }
    static const Utf8CP BlankString() { return " "; }
    static const Utf8CP FailedOperation() { return "Operation failed"; }
    static const PresentationType DefaultPresentaitonType() { return PresentationType::Decimal; }
    static const ShowSignOption DefaultSignOption() { return ShowSignOption::OnlyNegative; }
    static const DecimalPrecision DefaultDecimalPrecision() { return  DecimalPrecision::Precision6; }
    static const size_t DefaultDecimalPrecisionIndex() { return static_cast<int>(DefaultDecimalPrecision()); }
    static const size_t MaxDecimalPrecisionIndex() { return static_cast<int>(DecimalPrecision::Precision12); }
    static const FractionalPrecision DefaultFractionalPrecision() { return  FractionalPrecision::Over_64; }
    static const size_t DefaultFractionalDenominator() { return Utils::FractionalPrecisionDenominator(DefaultFractionalPrecision()); }
    static const FormatTraits DefaultFormatTraits() { return static_cast<FormatTraits>(static_cast<int>(FormatTraits::KeepDecimalPoint) | static_cast<int>(FormatTraits::KeepSingleZero)); }
    static const double FPV_MaxTokenLength() { return 256; }
    static const unsigned char UTF_2ByteMask() { return  0xE0; }      // 11100000 - complement will select 5 upper bits
    static const unsigned char UTF_2ByteMark() { return  0xC0; }      // 11000000
    static const unsigned char UTF_3ByteMask() { return  0xF0; }    // 11110000  - complement will select 4 upper bits
    static const unsigned char UTF_3ByteMark() { return  0xE0; }    // 11100000
    static const unsigned char UTF_4ByteMask() { return   0xF8; }   // 11111000  - complement will select 3 upper bits
    static const unsigned char UTF_4ByteMark() { return  0xF0; }    // 11110000
    static const unsigned char UTF_TrailingByteMask() { return  0xC0; } // 11000000 - complement will select trailing bits
    static const unsigned char UTF_TrailingByteMark() { return  0x80; } // 10000000 - indicator of the trailing bytes and also an ASCII char
    static const unsigned char UTF_TrailingBitsMask() { return  0x3F; }    // 00111111
    static const unsigned char UTF_UpperBitShift() { return  6; }
    static const  int GetTrailingShift() { return UTF_UpperBitShift(); }
    UNITS_EXPORT static const bool IsLittleEndian();
    UNITS_EXPORT static const size_t GetSequenceLength(unsigned char c);
    static bool IsTrailingByteValid(unsigned char c) { return (UTF_TrailingByteMark() == (c & UTF_TrailingByteMask())); }
    UNITS_EXPORT static bool GetTrailingBits(unsigned char c, CharP outBits);
    UNITS_EXPORT static bool GetCodeBits(unsigned char c, size_t seqLength, size_t index, size_t* outBits);
    static bool IsNegligible(double dval) { return (fabs(dval) < FormatConstant::FPV_MinTreshold()); }
    static bool IsIgnored(double dval) { return (dval < 0.0 || fabs(dval) < FormatConstant::FPV_MinTreshold()); }
    static bool IsMagnitudeOne(double dval) { return (fabs(1.0 - fabs(dval)) < FormatConstant::FPV_MinTreshold()); }
};



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
    UNITS_EXPORT void CopyValues(FactorPowerP other);
    UNITS_EXPORT void Merge(FactorPowerP fp1, FactorPowerP fp2);
    const int GetDivisor() { return static_cast<int>(m_divisor); }
    const size_t GetPower() { return m_power; }
    const int GetIndex() { return m_index; }
    UNITS_EXPORT const size_t GetFactor();
    UNITS_EXPORT Utf8String ToText(Utf8Char pref);
    };

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
    bvector<FactorPower> GetFactors() { return m_factors; }
    void ResetFactors(bvector<FactorPower> fact);
    UNITS_EXPORT FactorPowerP FindDivisor(int div);
    UNITS_EXPORT size_t GetGreatestCommonFactor(FactorizedNumber other);
    size_t GetValue() { return m_ival; }
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
    size_t GCF(size_t numer, size_t denom); 
public:
    size_t GetDenominator() { return m_denominator; }
    size_t GetNumerator() { return m_numerator; }
    int64_t GetIntegral() { return m_integral; }
    UNITS_EXPORT FractionalNumeric(double dval, FractionalPrecision fprec);
    UNITS_EXPORT FractionalNumeric(double dval, int denominator);
    UNITS_EXPORT FractionalNumeric(double dval, int denominatorBase, double precision);
    UNITS_EXPORT Utf8String ToTextDefault(bool reduce);
    UNITS_EXPORT Utf8String GetIntegralString();
    UNITS_EXPORT Utf8String GetDenominatorString();
    UNITS_EXPORT Utf8String GetNumeratorString();
    UNITS_EXPORT Utf8CP GetIntegralText();
    UNITS_EXPORT Utf8CP GetDenominatorText();
    UNITS_EXPORT Utf8CP GetNumeratorText();
    UNITS_EXPORT void FormTextParts(bool reduce);
    bool HasFractionPart() { return 1 < m_textParts.size(); }
    };

//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz  10/2016
//=======================================================================================
struct NumericFormatSpec
    {
private:
    Utf8String          m_name;                  // name or ID of the format
    Utf8String          m_alias;                 // short alternative name (alias)
    double              m_minTreshold;
    double              m_roundFactor;
    PresentationType    m_presentationType;      // Decimal, Fractional, Sientific, ScientificNorm
    ShowSignOption      m_signOption;            // NoSign, OnlyNegative, SignAlways, NegativeParenths
    FormatTraits        m_formatTraits;          // NoZeroes, LeadingZeroes, TrailingZeroes, BothZeroes
    DecimalPrecision    m_decPrecision;          // Precision0...12
    FractionalPrecision m_fractPrecision;
    Utf8Char            m_decimalSeparator;      // DecimalComma, DecimalPoint, DecimalSeparator
    Utf8Char            m_thousandsSeparator;    // ThousandSepComma, ThousandSepPoint, ThousandsSeparartor

    double EffectiveRoundFactor(double rnd) { return FormatConstant::IsIgnored(rnd) ? m_roundFactor : rnd; }

    UNITS_EXPORT void DefaultInit(Utf8CP name, size_t precision);
    UNITS_EXPORT void Init(Utf8CP name, PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, size_t precision);
    UNITS_EXPORT double RoundedValue(double dval, double round);
    UNITS_EXPORT int TrimTrailingZeroes(CharP buf, int index);
    UNITS_EXPORT size_t InsertChar(CharP buf, size_t index, char c, int num);
    NumericFormatSpec() { DefaultInit("*", FormatConstant::DefaultDecimalPrecisionIndex()); }

public:
    NumericFormatSpec(Utf8CP name) { DefaultInit(name, FormatConstant::DefaultDecimalPrecisionIndex()); }
    NumericFormatSpec(Utf8CP name, size_t precision) { DefaultInit(name, precision); }
    //UNITS_EXPORT NumericFormat(StdFormatNameR fmtType, size_t precision, double round = -1.0);
    UNITS_EXPORT NumericFormatSpec(Utf8CP name, PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, const size_t precision);
    UNITS_EXPORT NumericFormatSpec(NumericFormatSpecCR other);
    //UNITS_EXPORT static NumericFormat StdNumericFormat(Utf8P stdName, int prec, double round);
    void SetFormatTraits(FormatTraits opt) { m_formatTraits = opt; }
    FormatTraits GetFormatTraits() const { return m_formatTraits; }
    UNITS_EXPORT void SetKeepTrailingZeroes(bool keep);
    bool IsKeepTrailingZeroes() { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::TrailingZeroes)) != 0); }
    UNITS_EXPORT void SetUseLeadingZeroes(bool use);
    bool IsUseLeadingZeroes() { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::LeadingZeroes)) != 0); }
    UNITS_EXPORT void SetKeepDecimalPoint(bool keep);
    bool IsKeepDecimalPoint() { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::KeepDecimalPoint)) != 0); }
    UNITS_EXPORT void SetKeepSingleZero(bool keep);
    bool IsKeepSingleZero() { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::KeepSingleZero)) != 0); }
    UNITS_EXPORT void SetExponentZero(bool keep);
    bool IsExponentZero() { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::ExponentZero)) != 0); }
    UNITS_EXPORT void SetZeroEmpty(bool empty);
    bool IsZeroEmpty() { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::ZeroEmpty)) != 0); }
    UNITS_EXPORT void SetUse1000Separator(bool use);
    bool IsUse1000Separator() { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::Use1000Separator)) != 0); }
    UNITS_EXPORT void SetApplyRounding(bool use);
    bool IsApplyRounding() { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::ApplyRounding)) != 0); }
    bool IsInsertSeparator(bool confirm) { return (IsUse1000Separator() && (m_thousandsSeparator != 0) && confirm); }
    void SetNegativeParentheses() { m_signOption = ShowSignOption::NegativeParentheses; }
    bool IsNegativeParentheses() { return (m_signOption == ShowSignOption::NegativeParentheses); }
    bool IsOnlyNegative() { return (m_signOption == ShowSignOption::OnlyNegative); }
    bool IsSignAlways() { return (m_signOption == ShowSignOption::SignAlways); }
    bool IsFractional() { return m_presentationType == PresentationType::Fractional; }
    UNITS_EXPORT void SetAlias(Utf8CP alias);
    Utf8String GetAlias() { return m_alias; }
    void SetDecimalPrecision(DecimalPrecision prec) { m_decPrecision = prec; }
    DecimalPrecision GetDecimalPrecision() { return m_decPrecision; }
    UNITS_EXPORT int GetDecimalPrecisionIndex(int prec);
    UNITS_EXPORT double GetDecimalPrecisionFactor(int prec);
    FractionalPrecision SetFractionaPrecision(FractionalPrecision precision) { return m_fractPrecision = precision; }
    FractionalPrecision GetFractionalPrecision() const { return m_fractPrecision; }
    double SetRoundingFactor(double round) { return m_roundFactor = round; }
    double GetRoundingFactor() const { return m_roundFactor; }
    UNITS_EXPORT void SetPrecisionByValue(int prec);
    void SetPresentationType(PresentationType type) { m_presentationType = type; }
    PresentationType GetPresentationType() const { return m_presentationType; }
    void SetSignOption(ShowSignOption opt) { m_signOption = opt; }
    ShowSignOption GetSignOption() const { return m_signOption; }
    Utf8Char SetDecimalSeparator(Utf8Char sep) { return m_decimalSeparator = sep; }
    Utf8Char GetDecimalSeparator() const { return m_decimalSeparator; }
    Utf8Char SetThousandSeparator(char sep) { return m_thousandsSeparator = sep; }
    Utf8Char GetThousandSeparator() const { return m_thousandsSeparator; }
    UNITS_EXPORT static double RoundDouble(double dval, double roundTo);
    UNITS_EXPORT static bool AcceptableDifference(double dval1, double dval2, double maxDiff); 
    bool IsPrecisionZero() {    return (m_decPrecision == DecimalPrecision::Precision0);}
    UNITS_EXPORT int IntPartToText(double n, CharP bufOut, int bufLen, bool useSeparator);
    
    
    UNITS_EXPORT int FormatInteger (int n, CharP bufOut, int bufLen);
    UNITS_EXPORT int static FormatIntegerSimple (int n, CharP bufOut, int bufLen, bool showSign, bool extraZero);
    UNITS_EXPORT size_t FormatDouble(double dval, CharP buf, size_t bufLen, int prec = -1, double round = -1.0);
    //Utf8P
    
    UNITS_EXPORT static Utf8String StdFormatDouble(double dval, Utf8P stdName, int prec = -1, double round = -1.0);
    UNITS_EXPORT static Utf8String StdFormatQuantity(QuantityCR qty, UnitCP useUnit, Utf8P stdName, int prec = -1, double round = -1.0);
    UNITS_EXPORT static Utf8String StdFormatQuantityTriad(QuantityTriadSpecP qtr, Utf8CP stdName, Utf8CP space, int prec = -1, double round = -1.0);
    UNITS_EXPORT Utf8String FormatDouble(double dval, int prec = -1, double round = -1.0);
    UNITS_EXPORT Utf8String FormatQuantity(QuantityCR qty, UnitCP useUnit, int prec = -1, double round = -1.0);
    UNITS_EXPORT static Utf8String StdFormatPhysValue(double dval, Utf8CP fromUOM, Utf8CP toUOM, Utf8CP toLabel, Utf8P stdName, int prec = -1, double round = -1.0);
    //FormatDoubleStd


    UNITS_EXPORT int FormatBinaryByte (unsigned char n, CharP bufOut, int bufLen);
    UNITS_EXPORT int FormatBinaryShort (short int n, CharP bufOut, int bufLen, bool useSeparator);
    UNITS_EXPORT int FormatBinaryInt (int n, CharP bufOut, int bufLen, bool useSeparator);
    UNITS_EXPORT int FormatBinaryDouble (double x, CharP bufOut, int bufLen, bool useSeparator);
    UNITS_EXPORT static int RightAlignedCopy(CharP dest, int destLen, bool termZero, CharCP src, int srcLen);
    UNITS_EXPORT Utf8String FormatRoundedDouble(double dval, double round);
    UNITS_EXPORT Utf8String FormatInteger(int ival);
    UNITS_EXPORT Utf8String ByteToBinaryText(unsigned char n);
    UNITS_EXPORT Utf8String ShortToBinaryText(short int n, bool useSeparator);
    UNITS_EXPORT Utf8String IntToBinaryText(int n, bool useSeparator);
    UNITS_EXPORT Utf8String DoubleToBinaryText(double x, bool useSeparator);
    UNITS_EXPORT Utf8String GetName() { return m_name; };
    };

    //Spec
//=======================================================================================
//! A class for breaking a given double precision number into 2 or 3 sub-parts defined by their ratios
//! Can be used for presenting angular measurement in the form of Degrees, Minutes and Seconds or 
//!   length in a form of yards, feet and inches. The parts are called top, middle and low and two ratios between
//!     top and middle and middle and low define the breakup details. The top is always supposed to be an integer
//!  represented as double precision number. Both ratios are assumed to be positive integer numbers for avoiding incorrect
//!   results. If a negative number is provided for either of two ratios its sign will be dropped. 
//!  
// @bsiclass                                                    David.Fox-Rabinovitz  10/2016
//=======================================================================================
struct NumericTriad
    {
protected:
    double m_dval;            // we keep the originally submitted value for its sign and possible reverse operations
    double m_topValue;        
    double m_midValue;
    double m_lowValue;
    //DecimalPrecision m_decPrecision;
    size_t    m_topToMid;
    size_t    m_midToLow;
    bool   m_init;
    bool   m_midAssigned;
    bool   m_lowAssigned;
    bool   m_negative;
    UNITS_EXPORT void Convert();
    UNITS_EXPORT void SetValue(double dval);
    UNITS_EXPORT NumericTriad();
    UNITS_EXPORT static bool IsNameNullOrEmpty(Utf8CP name) { return (nullptr == name || strlen(name) == 0); } 
    size_t SetTopToMid(size_t value) { return  m_topToMid = value; }
    size_t SetMidToLow(size_t value) { return  m_midToLow = value; }
    void SetInit() { m_init = true; }
public:
    UNITS_EXPORT NumericTriad(double dval, size_t topMid, size_t midLow);
    
    double GetWhole() { return m_negative ? -m_dval: m_dval; }
    void ProcessValue(double dval)  {  SetValue(dval);  Convert(); }
    void SetRatio(int topToMid, int midToLow) { m_topToMid = topToMid; m_midToLow = midToLow; }
    //void SetPrecision(DecimalPrecision prec) { m_decPrecision = prec; }
    double GetTopValue() { return m_topValue; }
    double GetMidValue() { return m_midValue; }
    double GetlowValue() { return m_lowValue; }
    UNITS_EXPORT Utf8String FormatWhole(DecimalPrecision prec);
    UNITS_EXPORT Utf8String FormatTriad(Utf8CP topName, Utf8CP midName, Utf8CP lowName, Utf8CP space, int prec, bool fract = false, bool includeZero=false);
    };

struct QuantityTriadSpec : NumericTriad
    {
private:
    QuantityCP m_quant;
    UnitCP m_topUnit;
    UnitCP m_midUnit;
    UnitCP m_lowUnit;
    Utf8CP m_topUnitLabel;
    Utf8CP m_midUnitLabel;
    Utf8CP m_lowUnitLabel;
    bool m_includeZero;
    FormatProblemCode m_problemCode;

    void Init(bool incl0);
    QuantityTriadSpec();
    bool ValidatePhenomenaPair(PhenomenonCP srcPhen, PhenomenonCP targPhen);
public:

    Utf8CP SetTopUnitLabel(Utf8CP symbol) { return m_topUnitLabel = symbol; }
    Utf8CP GetTopUnitLabel() { return m_topUnitLabel; }
    Utf8CP SetMidUnitLabel(Utf8CP symbol) { return m_midUnitLabel = symbol; }
    Utf8CP GetMidUnitLabel() { return m_midUnitLabel; }
    Utf8CP SetLowUnitLabel(Utf8CP symbol) { return m_lowUnitLabel = symbol; }
    Utf8CP GetLowUnitLabel() { return m_lowUnitLabel; }
    bool SetIncludeZero(bool val) { return m_includeZero = val; }
    bool GetIncludeZero() const { return m_includeZero; }
    UNITS_EXPORT static size_t UnitRatio(UnitCP un1, UnitCP un2);
    UNITS_EXPORT QuantityTriadSpec(QuantityCR qty, UnitCP topUnit, UnitCP midUnit = nullptr, UnitCP lowUnit = nullptr, bool incl0 = false);

    bool IsProblem() { return m_problemCode != FormatProblemCode::NoProblems; }
    FormatProblemCode GetProblemCode() { return m_problemCode; }
    UNITS_EXPORT bool UpdateProblemCode(FormatProblemCode code);
    UNITS_EXPORT Utf8String FormatQuantTriad(Utf8CP space, int prec, bool fract=false, bool includeZero = false);
    Utf8CP GetTopUOM() { return (nullptr == m_topUnit) ? FormatConstant::EmptyString() : m_topUnit->GetName(); }
    Utf8CP GetMidUOM() { return (nullptr == m_topUnit) ? FormatConstant::EmptyString() : m_midUnit->GetName(); }
    Utf8CP GetLowUOM() { return (nullptr == m_topUnit) ? FormatConstant::EmptyString() : m_lowUnit->GetName(); }
    };

struct StdFormatSet
    {
private:
    bvector<NumericFormatSpecP> m_formatSet;

    NumericFormatSpecP AddFormat(NumericFormatSpecP fmtP);
    void StdInit();
    StdFormatSet() { StdInit(); }
    static StdFormatSet& Set() { static StdFormatSet set; return set; }
    
public:

    UNITS_EXPORT static NumericFormatSpecP DefaultDecimal();
    static size_t GetFormatSetSize() { return Set().m_formatSet.size(); }
    UNITS_EXPORT static NumericFormatSpecP FindFormat(Utf8CP name);
    };

// Format parameter traits
//=======================================================================================
// @bsiclass
//=======================================================================================
struct FormatParameter
{
private:
    Utf8String m_paramName;
    ParameterCategory m_category;
    ParameterCode m_paramCode;
    ParameterDataType m_paramType;
    int m_intValue;            // for binary flags and other integer values

public:
    FormatParameter(Utf8StringCR name, ParameterCategory cat, ParameterCode code, ParameterDataType type) :
        m_paramName(name), m_category(cat), m_paramCode(code), m_paramType(type), m_intValue(0) {}

    FormatParameter(Utf8StringCR name, ParameterCategory cat, ParameterCode code, int bitFlag):
        m_paramName(name), m_category(cat), m_paramCode(code), m_paramType(ParameterDataType::BitFlag), m_intValue(bitFlag) {}

    Utf8StringCR GetName() { return m_paramName; }
    int CompareName(Utf8StringCR other) { return strcmp(m_paramName.c_str(), other.c_str()); }
    ParameterCategory GetCategory() { return m_category; }
    CharCP GetCategoryName() { return Utils::GetParameterCategoryName(m_category); }
    ParameterCode GetParameterCode() { return m_paramCode; }
    size_t GetParameterCodeValue() { return (size_t)m_paramCode; }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct FormatDictionary //: FormatConstant
    {
private:
    bvector<FormatParameter> m_paramList;

    UNITS_EXPORT void InitLoad();
    Utf8StringP ParameterValuePair(Utf8StringCR name, Utf8StringCR value, char quote, Utf8StringCR prefix);

public:

    FormatDictionary() { InitLoad(); }
    int GetCount() { return (int)m_paramList.size(); }
    void AddParameter(FormatParameterCR par) { m_paramList.push_back(par); return; }
    UNITS_EXPORT FormatParameterP FindParameterByName(Utf8StringCR paramName);
    UNITS_EXPORT FormatParameterP FindParameterByCode(ParameterCode paramCode);
    UNITS_EXPORT FormatParameterP GetParameterByIndex(int index);
    UNITS_EXPORT Utf8StringCR CodeToName(ParameterCode paramCode);
    UNITS_EXPORT Utf8StringP SerializeFormatDefinition(NumericFormatSpec format);
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct FormatCursorDetail
    {
private:
    size_t m_totalScanLength; // this is the total length of the byte sequence that ought to be scanned/parsed
    size_t m_cursorPosition;  // the index of the next byte to be scanned
    size_t m_lastScannedCount;   // the number of bytes processed in the last step
    size_t m_uniCode;

    void Init() { m_totalScanLength = 0; m_cursorPosition = 0; m_lastScannedCount = 0; m_uniCode = 0; }
public:

    };

//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz
//=======================================================================================
struct FormattingScannerCursor
    {
private:
    Utf8String m_text;           // pointer to the head of the string
    size_t m_totalScanLength;    // this is the total length of the byte sequence that ought to be scanned/parsed
    size_t m_cursorPosition;     // the index of the next byte to be scanned
    size_t m_lastScannedCount;   // the number of bytes processed in the last step
    size_t m_uniCode;
    //union { uint8_t octet[4];  unsigned int word; } m_code; // container for the scanned bytes
    bool m_isASCII;          // flag indicating that the last scanned byte is ASCII
    UnicodeConstantP m_unicodeConst; // reference to constants and character processors
    ScannerCursorStatus m_status;
    char m_temp;

    // takes an logical index to an array of ordered bytes representing an integer entity in memory and 
    // returns the physical index of the same array adjusted by endianness. The little endian is default 
    //  and the index will be returned unchaged. This function does not check if supplied 
    size_t TrueIndex(size_t index, size_t wordSize);
    int AddTrailingByte();
    size_t SetCurrentPosition(size_t position) { return m_cursorPosition = position; }
    //UNITS_EXPORT int ProcessTrailingByte(char c, int* bits);

public:
    //! Construct a cursor attached to the given Utf8 string 

    UNITS_EXPORT FormattingScannerCursor(CharCP utf8Text, int scanLength);
    UNITS_EXPORT FormattingScannerCursor(FormattingScannerCursorCR other);
    UnicodeConstant* GetConstants() { return m_unicodeConst; }
    size_t GetCurrentPosition() { return m_cursorPosition; }
    UNITS_EXPORT size_t GetNextSymbol();
    UNITS_EXPORT size_t GetNextCodePoint();
    bool IsError() { return (m_status != ScannerCursorStatus::Success); }
    bool IsSuccess() { return (m_status == ScannerCursorStatus::Success); }
    ScannerCursorStatus GetOperationStatus() { return m_status;}
    bool IsEndOfLine() { return (m_text[m_cursorPosition] == '\0'); }
    bool IsASCII() { return m_isASCII; }
    UNITS_EXPORT int CodePointCount();
    UNITS_EXPORT void Rewind();
    size_t GetUnicode() { return m_uniCode; }
    size_t GetLastScanned() { return m_lastScannedCount; }
    UNITS_EXPORT size_t SkipBlanks();
    UNITS_EXPORT Utf8String SelectKeyWord();
    };

struct FormattingToken
    {
private:
    FormattingScannerCursorP m_cursor;
    size_t m_tokenLength;    // this is the number of logical symbols
    size_t m_tokenBytes;     // this is the number of bytes containing the token
    size_t m_cursorStart;    // the index of the first byte in token
    bvector<size_t> m_word;
    bvector<size_t> m_delim;
    bool m_isASCII;

public:
    UNITS_EXPORT FormattingToken(FormattingScannerCursorP cursor);

    UNITS_EXPORT WString GetNextToken();
    };

struct FormatStopWatch
    {
private:
    std::chrono::steady_clock::time_point m_start;
    double m_lastInterval;
    double m_totalElapsed;
    size_t m_lastAmount;
    size_t m_totalAmount;
   
public:
    UNITS_EXPORT FormatStopWatch();
    UNITS_EXPORT Utf8String LastIntervalMetrics(size_t amount);
    UNITS_EXPORT Utf8String LastInterval(double factor);
    };
END_BENTLEY_FORMATTING_NAMESPACE
