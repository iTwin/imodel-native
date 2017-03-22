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

namespace BEU = BentleyApi::Units;

BEGIN_BENTLEY_FORMATTING_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(NumericFormatSpec)
FORMATTING_TYPEDEFS(FormatParameterSet)
FORMATTING_TYPEDEFS(NumericTriad)
DEFINE_POINTER_SUFFIX_TYPEDEFS(QuantityTriadSpec)
DEFINE_POINTER_SUFFIX_TYPEDEFS(StdFormatName)
DEFINE_POINTER_SUFFIX_TYPEDEFS(StdFormatNameMap)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatParameter)
DEFINE_POINTER_SUFFIX_TYPEDEFS(CompositeValue)
DEFINE_POINTER_SUFFIX_TYPEDEFS(CompositeValueSpec)
FORMATTING_TYPEDEFS(FormatDictionary)
DEFINE_POINTER_SUFFIX_TYPEDEFS(UnicodeConstant)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormattingScannerCursor)
FORMATTING_REFCOUNTED_TYPEDEFS(NumericFormatSpec)
FORMATTING_TYPEDEFS(StdFormatSet)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FactorPower)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatUnitSet)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormattingDividers)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormattingWord)
DEFINE_POINTER_SUFFIX_TYPEDEFS(NamedFormatSpec)
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
    None = 0,
    Oblique = 1,
    Horizontal = 2,
    Diagonal = 3
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

//! Codes of problems might help in finding the source of the problem
enum class FormatProblemCode
    {
    NoProblems = 0,
    UnknownStdFormatName = 11,
    UnknownUnitName = 12,
    CNS_InconsistentFactorSet = 51,  //!< All ratio factors between units must be bigger than one
    CNS_InconsistentUnitSet = 52,    //!< Each pair of UOM's for parts of combo-numbers should yeild a ratio > 1
    CNS_UncomparableUnits = 53,      //!< Units provided on the argument list are not comparable
    CNS_InvalidUnitName = 54,        //!< Not-recognizd unit name or unit is not associated with a Phenomenon
    CNS_InvalidMajorUnit = 55,       //!< The MajorUnit in ComboNumbers is null or invalid
    QT_PhenomenonNotDefined = 101,
    QT_PhenomenaNotSame = 102,
    QT_InvalidTopMidUnits = 103,
    QT_InvalidMidLowUnits = 104,
    QT_InvalidUnitCombination = 105,
    FUS_InvalidSyntax = 151,
    NFS_InvalidSpecName = 161,
    NFS_DuplicateSpecName = 162
    };

//! Type of the ComboSpec describes one of allowable value transformations
enum class CompositeSpecType
    {
    Undefined = 0, //!< program failes to infer the type (default)
    Single = 1,    //!< trivial case when Combo effectively is not used - not prohibited though
    Double = 2,    //!< indicates of using 2 levels: Major and Middle
    Triple = 3,    //!< indicates of using 3 levels: Major, Middle and Minor UOM's
    Quatro = 4     //!< indicates of using 4 levels: Major, Middle, Minor and SubUnit UOM's
    };

enum class FormatSpecType
    {
    Undefined = 0,
    Numeric = 1,   // a pure numeric Spec
    Composite = 2      // a composite spec is also defined (numeric spec is implied)
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
    UNITS_EXPORT static Utf8String FractionBarName(FractionBarType bar);
    UNITS_EXPORT static FractionalPrecision FractionalPrecisionByDenominator(size_t prec);
    static int FormatTraitsBit(FormatTraits zcValue) { return static_cast<int>(zcValue); }
    UNITS_EXPORT static const size_t FractionalPrecisionDenominator(FractionalPrecision prec);
    UNITS_EXPORT static size_t AppendText(Utf8P buf, size_t bufLen, size_t index, Utf8CP str);
    static bool IsNameNullOrEmpty(Utf8CP name) { return (nullptr == name || strlen(name) == 0); }
    static Utf8CP SubstituteEmptyOrNull(Utf8CP name, Utf8CP subs) { return IsNameNullOrEmpty(name) ? subs : name; }
    //UNITS_EXPORT static Utf8CP GetFormatProblemDescription(FormatProblemCode code);
    UNITS_EXPORT static bool AreUnitsComparable(BEU::UnitCP un1, BEU::UnitCP u2);
    static size_t MinInt(size_t a, size_t b) { return(a <= b) ? a : b; }
    static size_t MaxInt(size_t a, size_t b) { return(a >= b) ? a : b; }
    UNITS_EXPORT static Utf8String FormatProblemDescription(FormatProblemCode code);
    UNITS_EXPORT static Utf8String AppendUnitName(Utf8CP txtValue, Utf8CP unitName= nullptr, Utf8CP space = nullptr);
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
    static Utf8String FPN_Name() { return "name"; }
    static Utf8String FPN_Alias() { return "alias"; }
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
    static Utf8String FPN_Precision0() { return "DecPrecision0"; }
    static Utf8String FPN_Precision1() { return "DecPrecision1"; }
    static Utf8String FPN_Precision2() { return "DecPrecision2"; }
    static Utf8String FPN_Precision3() { return "DecPrecision3"; }
    static Utf8String FPN_Precision4() { return "DecPrecision4"; }
    static Utf8String FPN_Precision5() { return "DecPrecision5"; }
    static Utf8String FPN_Precision6() { return "DecPrecision6"; }
    static Utf8String FPN_Precision7() { return "DecPrecision7"; }
    static Utf8String FPN_Precision8() { return "DecPrecision8"; }
    static Utf8String FPN_Precision9() { return "DecPrecision9"; }
    static Utf8String FPN_Precision10() { return "DecPrecision10"; }
    static Utf8String FPN_Precision11() { return "DecPrecision11"; }
    static Utf8String FPN_Precision12() { return "DecPrecision12"; }
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
    static const double FPV_MinTreshold() { return 1.0e-14; }  // format parameter default values
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
    UNITS_EXPORT static bool GetTrailingBits(unsigned char c, Utf8P outBits);
    UNITS_EXPORT static bool GetCodeBits(unsigned char c, size_t seqLength, size_t index, size_t* outBits);
    static bool IsNegligible(double dval) { return (fabs(dval) < FormatConstant::FPV_MinTreshold()); }
    static bool IsIgnored(double dval) { return (dval < 0.0 || fabs(dval) < FormatConstant::FPV_MinTreshold()); }
    static bool IsMagnitudeOne(double dval) { return (fabs(1.0 - fabs(dval)) < FormatConstant::FPV_MinTreshold()); }
    UNITS_EXPORT static const unsigned char TriadBitMask(size_t threeBit);
    static const Utf8CP BoolText(bool t) { return t ? "true" : "false"; }
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
    double              m_roundFactor;
    PresentationType    m_presentationType;      // Decimal, Fractional, Sientific, ScientificNorm
    ShowSignOption      m_signOption;            // NoSign, OnlyNegative, SignAlways, NegativeParenths
    FormatTraits        m_formatTraits;          // NoZeroes, LeadingZeroes, TrailingZeroes, BothZeroes
    DecimalPrecision    m_decPrecision;          // Precision0...12
    FractionalPrecision m_fractPrecision;
    Utf8Char            m_decimalSeparator;      // DecimalComma, DecimalPoint, DecimalSeparator
    Utf8Char            m_thousandsSeparator;    // ThousandSepComma, ThousandSepPoint, ThousandsSeparartor
    FractionBarType     m_barType;

    double EffectiveRoundFactor(double rnd) { return FormatConstant::IsIgnored(rnd) ? m_roundFactor : rnd; }

    //UNITS_EXPORT void DefaultInit(Utf8CP name, size_t precision);
    //UNITS_EXPORT void Init(Utf8CP name, PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, size_t precision);
    
    UNITS_EXPORT void DefaultInit(size_t precision);
    UNITS_EXPORT void Init(PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, size_t precision);
    UNITS_EXPORT double RoundedValue(double dval, double round);
    UNITS_EXPORT int TrimTrailingZeroes(Utf8P buf, int index);
    UNITS_EXPORT size_t InsertChar(Utf8P buf, size_t index, char c, int num);
    //NumericFormatSpec() { DefaultInit(FormatConstant::DefaultDecimalPrecisionIndex()); }

public:
    NumericFormatSpec() { DefaultInit( FormatConstant::DefaultDecimalPrecisionIndex()); }
    NumericFormatSpec(size_t precision) { DefaultInit(precision); }
    //UNITS_EXPORT NumericFormat(StdFormatNameR fmtType, size_t precision, double round = -1.0);
    UNITS_EXPORT NumericFormatSpec(PresentationType presType, ShowSignOption signOpt, FormatTraits formatTraits, const size_t precision);
    UNITS_EXPORT NumericFormatSpec(NumericFormatSpecCR other);
    //UNITS_EXPORT static NumericFormat StdNumericFormat(Utf8P stdName, int prec, double round);
    void SetFormatTraits(FormatTraits opt) { m_formatTraits = opt; }
    FormatTraits GetFormatTraits() const { return m_formatTraits; }
    UNITS_EXPORT void SetKeepTrailingZeroes(bool keep);
    bool IsKeepTrailingZeroes() const { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::TrailingZeroes)) != 0); }
    UNITS_EXPORT void SetUseLeadingZeroes(bool use);
    bool IsUseLeadingZeroes() const { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::LeadingZeroes)) != 0); }
    UNITS_EXPORT void SetKeepDecimalPoint(bool keep);
    bool IsKeepDecimalPoint() const { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::KeepDecimalPoint)) != 0); }
    UNITS_EXPORT void SetKeepSingleZero(bool keep);
    bool IsKeepSingleZero() const { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::KeepSingleZero)) != 0); }
    UNITS_EXPORT void SetExponentZero(bool keep);
    bool IsExponentZero() const { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::ExponentZero)) != 0); }
    UNITS_EXPORT void SetZeroEmpty(bool empty);
    bool IsZeroEmpty() const { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::ZeroEmpty)) != 0); }
    UNITS_EXPORT void SetUse1000Separator(bool use);
    bool IsUse1000Separator() const { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::Use1000Separator)) != 0); }
    UNITS_EXPORT void SetApplyRounding(bool use);
    bool IsApplyRounding() { return ((static_cast<int>(m_formatTraits) & static_cast<int>(FormatTraits::ApplyRounding)) != 0); }
    bool IsInsertSeparator(bool confirm) { return (IsUse1000Separator() && (m_thousandsSeparator != 0) && confirm); }
    void SetNegativeParentheses() { m_signOption = ShowSignOption::NegativeParentheses; }
    bool IsNegativeParentheses() { return (m_signOption == ShowSignOption::NegativeParentheses); }
    bool IsOnlyNegative() { return (m_signOption == ShowSignOption::OnlyNegative); }
    bool IsSignAlways() { return (m_signOption == ShowSignOption::SignAlways); }
    bool IsFractional() const { return m_presentationType == PresentationType::Fractional; }
    bool IsScientific() const { return (m_presentationType == PresentationType::Scientific || m_presentationType == PresentationType::ScientificNorm); }
    UNITS_EXPORT void SetAlias(Utf8CP alias);
    //Utf8String GetAlias() const { return m_alias; }
    void SetDecimalPrecision(DecimalPrecision prec) { m_decPrecision = prec; }
    DecimalPrecision GetDecimalPrecision() const { return m_decPrecision; }
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
    FractionBarType  GetFractionalBarType() const { return m_barType; }
    UNITS_EXPORT static double RoundDouble(double dval, double roundTo);
    UNITS_EXPORT static bool AcceptableDifference(double dval1, double dval2, double maxDiff); 
    bool IsPrecisionZero() {    return (m_decPrecision == DecimalPrecision::Precision0);}
    UNITS_EXPORT int IntPartToText(double n, Utf8P bufOut, int bufLen, bool useSeparator);
    
    UNITS_EXPORT int FormatInteger (int n, Utf8P bufOut, int bufLen);
    UNITS_EXPORT int static FormatIntegerSimple (int n, Utf8P bufOut, int bufLen, bool showSign, bool extraZero);
    UNITS_EXPORT size_t FormatDouble(double dval, Utf8P buf, size_t bufLen, int prec = -1, double round = -1.0);
    
    UNITS_EXPORT static Utf8String StdFormatDouble(Utf8P stdName, double dval, int prec = -1, double round = -1.0);
    UNITS_EXPORT static Utf8String StdFormatQuantity(Utf8P stdName, BEU::QuantityCR qty, BEU::UnitCP useUnit=nullptr, Utf8CP space = "", Utf8CP useLabel = nullptr, int prec = -1, double round = -1.0);
    UNITS_EXPORT static Utf8String StdFormatQuantityTriad(Utf8CP stdName, QuantityTriadSpecP qtr,Utf8CP space, int prec = -1, double round = -1.0);
    UNITS_EXPORT Utf8String FormatDouble(double dval, int prec = -1, double round = -1.0);
    UNITS_EXPORT Utf8String FormatQuantity(BEU::QuantityCR qty, BEU::UnitCP useUnit, Utf8CP space="", int prec = -1, double round = -1.0);
    UNITS_EXPORT static Utf8String StdFormatPhysValue(Utf8P stdName, double dval, Utf8CP fromUOM, Utf8CP toUOM, Utf8CP toLabel, Utf8CP space, int prec = -1, double round = -1.0);
    //UNITS_EXPORT static Utf8String StdFormatComboValue(Utf8P stdName, double dval, Utf8CP fromUOM, Utf8CP toUOM, Utf8CP toLabel, Utf8CP space, int prec = -1, double round = -1.0);

    //FormatDoubleStd

    UNITS_EXPORT int FormatBinaryByte (unsigned char n, Utf8P bufOut, int bufLen);
    UNITS_EXPORT int FormatBinaryShort (short int n, Utf8P bufOut, int bufLen, bool useSeparator);
    UNITS_EXPORT int FormatBinaryInt (int n, Utf8P bufOut, int bufLen, bool useSeparator);
    UNITS_EXPORT int FormatBinaryDouble (double x, Utf8P bufOut, int bufLen, bool useSeparator);
    UNITS_EXPORT static int RightAlignedCopy(Utf8P dest, int destLen, bool termZero, CharCP src, int srcLen);
    UNITS_EXPORT Utf8String FormatRoundedDouble(double dval, double round);
    UNITS_EXPORT Utf8String FormatInteger(int ival);
    UNITS_EXPORT Utf8String ByteToBinaryText(unsigned char n);
    UNITS_EXPORT Utf8String ShortToBinaryText(short int n, bool useSeparator);
    UNITS_EXPORT Utf8String IntToBinaryText(int n, bool useSeparator);
    UNITS_EXPORT Utf8String DoubleToBinaryText(double x, bool useSeparator);
    //Utf8String GetName() const { return m_name; } ;
    //Utf8String GetNameAndAlias() const { return m_name +"(" + m_alias + ")"; };
    };


//=======================================================================================
// We recognize combined numbers (combo-numbers) that represent some quantity as a sum of 
//   subquantities expressed in lesser UOM's. For example, a given length could be representes
//    as a sum of M + Y + F + I where M- is a number of miles, Y - a number of yards, 
//   F - a number of feet and I a number of inches. The operation of expressing the given length 
//  in this form will require 3 ratios: M/Y, Y/F, F/I where M/Y is a number of Yards in a Mile,
//   Y/F - is a number of feet in one yard, F/I is a number of inches in one foot.
//  Obviously, the combo-presentation of length is just one example and in some special cases 
//   this system of ratios can be anything the application needs. Ratios could be set explicitly
// or automatically via names of the UOM's. The only condition is that all integer ratios must be > 1.
//   let's define terms for UOM of different levels: 0 - majorUOM, 1 - midlleUOM, 2 - minorUOM, 3 - subUOM
//    accordingly there are 3 ratios: major/middle, middle/minor. minor/sub and their indexes
// in the ratio array are associated with the upper UOM
// @bsiclass                                                    David.Fox-Rabinovitz  01/2017
//=======================================================================================
struct CompositeValueSpec
    {
    friend struct CompositeValue;
protected:
    static const size_t  indxMajor  = 0;
    static const size_t  indxMiddle = 1;
    static const size_t  indxMinor  = 2;
    static const size_t  indxSub    = 3;
    static const size_t  indxInput  = 4;
    static const size_t  indxLimit  = 5;   
    size_t m_ratio[indxSub];
    BEU::UnitCP m_units[indxLimit];
    Utf8CP m_unitLabel[indxLimit];
    FormatProblemCode m_problemCode;
    CompositeSpecType m_type;
    bool m_includeZero;
    Utf8String m_spacer;

    void SetUnitLabel(int index, Utf8CP label);
    size_t UnitRatio(BEU::UnitCP upper, BEU::UnitCP lower);
    void ResetType() { m_type = CompositeSpecType::Undefined; }
    void Init();
    BEU::UnitCP SetInputUnit(BEU::UnitCP inputUnit) {return m_units[indxInput] = inputUnit; }
    void SetUnitRatios();
    bool SetUnitNames(Utf8CP MajorUnit, Utf8CP MiddleUnit, Utf8CP MinorUnit, Utf8CP SubUnit);
    Utf8CP GetUnitName(size_t indx, Utf8CP substitute) { return (nullptr == m_units[indx]) ? substitute : m_units[indx]->GetName(); }
    Utf8String GetEffectiveLabel(size_t indx, Utf8CP substitute) { return Utils::IsNameNullOrEmpty(m_unitLabel[indx]) ? GetUnitName(indx, substitute) : m_unitLabel[indx]; }
    //size_t GetRightmostRatioIndex();
    BEU::UnitCP GetSmallestUnit();
public:

   // UNITS_EXPORT CompositeValueSpec(size_t MajorToMiddle, size_t MiddleToMinor=0, size_t MinorToSub=0);
    UNITS_EXPORT CompositeValueSpec(BEU::UnitCP MajorUnit, BEU::UnitCP MiddleUnit=nullptr, BEU::UnitCP MinorUnit=nullptr, BEU::UnitCP subUnit = nullptr);
    UNITS_EXPORT CompositeValueSpec(Utf8CP MajorUnit, Utf8CP MiddleUnit = nullptr, Utf8CP MinorUni = nullptr, Utf8CP subUnit = nullptr);
    UNITS_EXPORT void SetUnitLabels(Utf8CP MajorLab, Utf8CP MiddleLab = nullptr, Utf8CP MinorLab = nullptr, Utf8CP SubLab = nullptr);
    UNITS_EXPORT Utf8String GetMajorLabel(Utf8CP substitute) { return GetEffectiveLabel(indxMajor, substitute); }
    UNITS_EXPORT Utf8String GetMiddleLabel(Utf8CP substitute) { return GetEffectiveLabel(indxMiddle, substitute); }
    UNITS_EXPORT Utf8String GetMinorLabel(Utf8CP substitute) { return GetEffectiveLabel(indxMinor, substitute); }
    UNITS_EXPORT Utf8String GetSubLabel(Utf8CP substitute) { return GetEffectiveLabel(indxSub, substitute); }
    UNITS_EXPORT bool UpdateProblemCode(FormatProblemCode code);
    bool IsProblem() { return m_problemCode != FormatProblemCode::NoProblems; }
    bool NoProblem() { return m_problemCode == FormatProblemCode::NoProblems; }
    size_t GetMajorToMiddleRatio() { return m_ratio[indxMajor]; }
    size_t GetMiddleToMinorRatio() { return m_ratio[indxMiddle]; }
    size_t GetMinorToSubRatio() { return m_ratio[indxMinor]; }
    UNITS_EXPORT Utf8CP GetProblemDescription();
    //UNITS_EXPORT NumericFormatSpecCP AssignFormatSpec(NumericFormatSpecCP spec) { return (m_formatSpec = spec); }
    UNITS_EXPORT CompositeValue DecomposeValue(double dval, BEU::UnitCP uom = nullptr);
    UNITS_EXPORT Utf8String FormatValue(double dval, NumericFormatSpecP fmtP, Utf8CP uomName = nullptr);
    CompositeSpecType GetType() { return m_type; }
    Utf8String GetSpacer() { return m_spacer; }
    Utf8String SetSpacer(Utf8CP spacer) { return m_spacer = spacer; }
    bool IsIncludeZero() { return m_includeZero; }
    bool SetIncludeZero(bool incl) { return m_includeZero = incl; }
    };

struct CompositeValue
    {
private:
    double m_parts[CompositeValueSpec::indxLimit];
    FormatProblemCode m_problemCode;

    void Init();
public:
    UNITS_EXPORT CompositeValue();
    double SetMajor(double dval)  { return m_parts[CompositeValueSpec::indxMajor] = dval; }
    double SetMiddle(double dval) { return m_parts[CompositeValueSpec::indxMiddle] = dval; }
    double SetMinor(double dval)  { return m_parts[CompositeValueSpec::indxMinor] = dval; }
    double SetSub(double dval)    { return m_parts[CompositeValueSpec::indxSub] = dval; }
    double SetInput(double dval)  { return m_parts[CompositeValueSpec::indxInput] = dval; }

    double GetMajor()  { return m_parts[CompositeValueSpec::indxMajor]; }
    double GetMiddle() { return m_parts[CompositeValueSpec::indxMiddle]; }
    double GetMinor()  { return m_parts[CompositeValueSpec::indxMinor]; }
    double GetSub()    { return m_parts[CompositeValueSpec::indxSub]; }
    double GetInput()  { return m_parts[CompositeValueSpec::indxInput]; }
    UNITS_EXPORT bool UpdateProblemCode(FormatProblemCode code);
    bool IsProblem() { return m_problemCode != FormatProblemCode::NoProblems; }
    };


//=======================================================================================
// Container for keeping together primary numeric, composite and other types of specs
//  and referrring them by the unique name. Name and at the valid numeric spec are required
//   for creating a valid instance of this class. Alias and composite spec are optional at the
// moment of creation but can be added later
// @bsiclass                                                    David.Fox-Rabinovitz  03/2017
//=======================================================================================
struct NamedFormatSpec
    {
    private:
        Utf8CP          m_name;                  // name or ID of the format
        Utf8CP          m_alias;                 // short alternative name (alias)
        NumericFormatSpecP   m_numericSpec;
        CompositeValueSpecP  m_compositeSpec;
        FormatSpecType  m_specType;
        FormatProblemCode m_problemCode;

    public:
        UNITS_EXPORT NamedFormatSpec(Utf8CP name, NumericFormatSpecP numSpec, Utf8CP alias = nullptr, CompositeValueSpecP compSpec = nullptr);
        Utf8CP SetAlias(Utf8CP alias) { return m_alias = alias; }
        Utf8CP GetAlias() const { return m_alias; }
        bool HasName(Utf8CP name) { return 0 == BeStringUtilities::StricmpAscii(name, m_name); }
        bool HasAlias(Utf8CP name) { return 0 == BeStringUtilities::StricmpAscii(name, m_alias); }
        Utf8CP GetName() const { return m_name; };
        FormatSpecType  GetSpecType(){return m_specType;}
        bool HasComposite() { return FormatSpecType::Composite == m_specType; }
        NumericFormatSpecP   GetNumericSpec() const { return m_numericSpec; }
        CompositeValueSpecP  GetCompositeSpec() { return  (HasComposite() ? m_compositeSpec : nullptr); }
        bool IsProblem() { return m_problemCode == FormatProblemCode::NoProblems; }
        Utf8String GetNameAndAlias() const { return Utf8String(m_name) + Utf8String("(") + Utf8String(m_alias) + Utf8String(")"); };
        PresentationType GetPresentationType() { return m_numericSpec->GetPresentationType(); }
    };

//=======================================================================================
// A pair of the unit reference and the format spec 
// @bsiclass                                                    David.Fox-Rabinovitz  03/2017
//=======================================================================================
struct FormatUnitSet
    {
    private:
        //NumericFormatSpecCP m_format;
        NamedFormatSpecCP m_formatSpec;
        BEU::UnitCP m_unit;
        FormatProblemCode m_problemCode;

    public:
        UNITS_EXPORT FormatUnitSet(NamedFormatSpecCP format, BEU::UnitCP unit);
        UNITS_EXPORT FormatUnitSet(Utf8CP formatName, Utf8CP unitName);
        UNITS_EXPORT Utf8String FormatQuantity(BEU::QuantityCR qty);
        UNITS_EXPORT FormatUnitSet(Utf8CP description);
        bool HasProblem() { return m_problemCode != FormatProblemCode::NoProblems; }
        FormatProblemCode GetProblemCode() { return m_problemCode; }
        UNITS_EXPORT Utf8String ToText(bool useAlias);
        //UNITS_EXPORT static bvector<FormatUnitSet> VectorFUS(Utf8CP description);
    };

struct FormatUnitGroup
    {
    private:
        bvector<FormatUnitSet> m_group;
        FormatProblemCode m_problemCode;
    public:
        UNITS_EXPORT FormatUnitGroup(Utf8CP description);
        UNITS_EXPORT Utf8String ToText(bool useAlias);
        bool HasProblem() { return m_problemCode != FormatProblemCode::NoProblems; }
        FormatProblemCode GetProblemCode() { return m_problemCode; }
    };


struct StdFormatSet
    {
private:
    bvector<NamedFormatSpecP> m_formatSet;
 
    NumericFormatSpecP AddFormat(Utf8CP name, NumericFormatSpecP fmtP, Utf8CP alias = nullptr, CompositeValueSpecP compS = nullptr);
    void StdInit();
    StdFormatSet() { StdInit(); }
    static StdFormatSet& Set() { static StdFormatSet set; return set; }
    
public:

    UNITS_EXPORT static NumericFormatSpecP DefaultDecimal();
    static size_t GetFormatSetSize() { return Set().m_formatSet.size(); }
    UNITS_EXPORT static NumericFormatSpecP GetNumericFormat(Utf8CP name);
    UNITS_EXPORT static NamedFormatSpecP FindFormatSpec(Utf8CP name);
    UNITS_EXPORT static bvector<Utf8CP> StdFormatNames(bool useAlias);
    UNITS_EXPORT static Utf8String StdFormatNameList(bool useAlias);
    };

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

        double GetWhole() { return m_negative ? -m_dval : m_dval; }
        void ProcessValue(double dval) { SetValue(dval);  Convert(); }
        void SetRatio(int topToMid, int midToLow) { m_topToMid = topToMid; m_midToLow = midToLow; }
        //void SetPrecision(DecimalPrecision prec) { m_decPrecision = prec; }
        double GetTopValue() { return m_topValue; }
        double GetMidValue() { return m_midValue; }
        double GetlowValue() { return m_lowValue; }
        UNITS_EXPORT Utf8String FormatWhole(DecimalPrecision prec);
        UNITS_EXPORT Utf8String FormatTriad(Utf8CP topName, Utf8CP midName, Utf8CP lowName, Utf8CP space, int prec, bool fract = false, bool includeZero = false);
    };


struct QuantityTriadSpec : NumericTriad
    {
    private:
        BEU::QuantityCP m_quant;
        BEU::UnitCP m_topUnit;
        BEU::UnitCP m_midUnit;
        BEU::UnitCP m_lowUnit;
        Utf8CP m_topUnitLabel;
        Utf8CP m_midUnitLabel;
        Utf8CP m_lowUnitLabel;
        bool m_includeZero;
        FormatProblemCode m_problemCode;

        void Init(bool incl0);
        QuantityTriadSpec();
        bool ValidatePhenomenaPair(BEU::PhenomenonCP srcPhen, BEU::PhenomenonCP targPhen);
        // UNITS_EXPORT static size_t UnitRatio(BEU::UnitCP un1, BEU::UnitCP un2);
    public:

        Utf8CP SetTopUnitLabel(Utf8CP symbol) { return m_topUnitLabel = symbol; }
        Utf8CP GetTopUnitLabel() { return m_topUnitLabel; }
        Utf8CP SetMidUnitLabel(Utf8CP symbol) { return m_midUnitLabel = symbol; }
        Utf8CP GetMidUnitLabel() { return m_midUnitLabel; }
        Utf8CP SetLowUnitLabel(Utf8CP symbol) { return m_lowUnitLabel = symbol; }
        Utf8CP GetLowUnitLabel() { return m_lowUnitLabel; }
        bool SetIncludeZero(bool val) { return m_includeZero = val; }
        bool GetIncludeZero() const { return m_includeZero; }
        UNITS_EXPORT static size_t UnitRatio(BEU::UnitCP un1, BEU::UnitCP un2);
        UNITS_EXPORT QuantityTriadSpec(BEU::QuantityCR qty, BEU::UnitCP topUnit, BEU::UnitCP midUnit = nullptr, BEU::UnitCP lowUnit = nullptr, bool incl0 = false);

        bool IsProblem() { return m_problemCode != FormatProblemCode::NoProblems; }
        FormatProblemCode GetProblemCode() { return m_problemCode; }
        UNITS_EXPORT bool UpdateProblemCode(FormatProblemCode code);
        UNITS_EXPORT Utf8String FormatQuantTriad(Utf8CP space, int prec, bool fract = false, bool includeZero = false);
        Utf8CP GetTopUOM() { return (nullptr == m_topUnit) ? FormatConstant::EmptyString() : m_topUnit->GetName(); }
        Utf8CP GetMidUOM() { return (nullptr == m_midUnit) ? FormatConstant::EmptyString() : m_midUnit->GetName(); }
        Utf8CP GetLowUOM() { return (nullptr == m_lowUnit) ? FormatConstant::EmptyString() : m_lowUnit->GetName(); }
        BEU::UnitCP GetTopUnit() { return m_topUnit; }
        BEU::UnitCP GetMidUnit() { return m_midUnit; }
        BEU::UnitCP GetLowUnit() { return m_lowUnit; }
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
    UNITS_EXPORT Utf8String SerializeFormatDefinition(NamedFormatSpecCP format);
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

struct FormattingDividers
    {
    // only 32 ASCII characters are regarded as allowable dividers in all formatting related text strings and expressions
    //  Their ASCII codes are quite stable and not going to change for the foreseeable future. The simplest and fastest 
    //   approach for marking dividers/stopperswill be creating an array of 128 bit flags occupying 128/8=16 bytes (or two doubles)
    //    Each bit in this array marks the code point that is considered as a astopper. This approach is also very flexible 
    //   since it will allow to directly control what is and what is not included into the set of stoppers.
    private:
        char m_markers[16];
    public:
        //UNITS_EXPORT FormattingDividers() { memset(m_markers, 0, sizeof(m_markers)); };
        UNITS_EXPORT FormattingDividers(Utf8CP div);
        UNITS_EXPORT FormattingDividers(FormattingDividersCR other);
        UNITS_EXPORT bool IsDivider(char c);
        CharCP GetMarkers() const { return m_markers; }

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
    FormattingDividers m_dividers;
    //union { uint8_t octet[4];  unsigned int word; } m_code; // container for the scanned bytes
    bool m_isASCII;          // flag indicating that the last scanned byte is ASCII
    UnicodeConstantP m_unicodeConst; // reference to constants and character processors
    ScannerCursorStatus m_status;
    size_t m_effectiveBytes;
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
   // FormattingScannerCursor();
    UNITS_EXPORT FormattingScannerCursor(CharCP utf8Text, int scanLength, CharCP div=nullptr);
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
    void SetDividers(CharCP div) { m_dividers = FormattingDividers(div); }
    bool IsDivider() { return m_isASCII ? m_dividers.IsDivider((char)(m_uniCode & 0x7F)) : false; }
    size_t GetEffectiveBytes() { return m_effectiveBytes; }
    UNITS_EXPORT FormattingWord ExtractWord();
    };

struct FormattingWord
    {
private:
    static const int maxDelim = 4;   // the maximum number of ASCII characters in the delimiting group/clause
    FormattingScannerCursorP m_cursor;  // just a reference to the cursor that has been used
    Utf8String m_word;
    Utf8Char m_delim[maxDelim+2];
    bool m_isASCII;
public:
    UNITS_EXPORT FormattingWord(FormattingScannerCursorP cursor, Utf8CP buffer, Utf8CP delim, bool isAscii);
    Utf8String GetWord() { return m_word; }
    Utf8Char GetDelim() { return m_delim[0]; }
    Utf8CP GetText() { return m_word.c_str(); }
    bool IsDelimeterOnly() { return ((0 == m_word.length()) && (0 != m_delim[0])); }
    bool IsEndLine() { return ((0 == m_word.length()) && (0 == m_delim[0])); }
    bool IsSeparator() { return ((0 == m_word.length()) && (',' == m_delim[0] || ' ' == m_delim[0])); }
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
    UNITS_EXPORT void Init();

public:
    UNITS_EXPORT FormattingToken(FormattingScannerCursorP cursor);
    UNITS_EXPORT WCharCP GetNextTokenW();
    UNITS_EXPORT CharCP GetASCII();
    UNITS_EXPORT Utf8Char GetDelimeter();
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

/*
struct KindOfQuantity
    {
    friend struct ECSchema;
    friend struct SchemaXmlWriter;
    friend struct SchemaXmlReaderImpl;

    private:
        ECSchemaCR m_schema;
        Utf8String m_fullName; //cached nsprefix:name representation
        ECValidatedName m_validatedName; //wraps name and displaylabel
        Utf8String m_description;

        //! Unit used for persisting the information
        Utf8String m_persistenceUnit;
        //! Precision
        uint32_t m_persistencePrecision;
        //! Unit used for presenting the information
        Utf8String m_defaultPresentationUnit;
        bvector<Utf8String> m_alternativePresentationUnitList;

        
        ////Quantity m_persistenceResolution;     // 1.0e-6 M   100.000003 1.100001
        double m_relativeError;   // a parameter related to the accuracy of the measurement methods.
        // Certainty, Accuracy, MarginOfError, Error, Tolerance, 
        FormatUnitSet m_persistenceFUS;
        bvector<FormatUnitSet> m_presentationFUS;  
        mutable KindOfQuantityId m_kindOfQuantityId;
*/

END_BENTLEY_FORMATTING_NAMESPACE
