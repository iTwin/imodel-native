/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Formatting/FormattingEnum.h $
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
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatProblemDetail)
//===================================================
//
// Enumerations
//
//===================================================

enum class ParameterCode
    {
    FormatName = 50,   
    NoSign = 101,            // no sign will prepend the number even if it is negative (like absolute value)
    OnlyNegative = 102,      // sign is added only for negative numbers
    SignAlways = 103,        // +23.0 - explicit sign even for positive numbers
    NegativeParenths = 104,  // present a negative number as (243.45)  rather than -243.45 
    Decimal = 151,
    Fractional = 152,
    Scientific = 153,
    ScientificNorm = 154,
    Binary = 155,
    DefaultZeroes = 201,     // a combination of default "zero settings"
    LeadingZeroes = 202,     // 00243.5  rather than 243.5 
    TrailingZeroes = 203,    // 243.000 rather than 243.0 if precision is 3
    KeepSingleZero = 204,    // 243.0 rather than 243 or 243.
    KeepDecimalPoint = 205,  // 243.  if an insignifcant zero after point is not preserved - rather than 243
    ExponentZero = 206,      // e05 instead of e5
    ZeroEmpty = 207,         // formatter will return the empy string if the result is 0
    FractionDash = 210,      // some people prefer to insert dash between integer and fraction: 3-1/4 rather than 3 1/4
    UseFractSymbol = 211,    // indicates that a limited set of fractional values can be presented by a single glyph
    DecPrec0 = 300,
    DecPrec1 = 301,
    DecPrec2 = 302,
    DecPrec3 = 303,
    DecPrec4 = 304,
    DecPrec5 = 305,
    DecPrec6 = 306,
    DecPrec7 = 307,
    DecPrec8 = 308,
    DecPrec9 = 309,
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
    ScientificNorm = 4,   // normalized scientific when Mantissa is < 1
    Stop100 = 5,       // special format for stations 100 feet
    Stop1000 = 6       // special format for stations 1000 meters
    };

enum class FormatTraits : int
    {
    DefaultZeroes = 0,
    LeadingZeroes = 0x1,
    TrailingZeroes = 0x2,
    KeepDecimalPoint = 0x4,
    KeepSingleZero = 0x8,
    ExponentZero = 0x10,
    ZeroEmpty = 0x20,
    Use1000Separator = 0x40,
    ApplyRounding = 0x80,
    FractionDash = 0x100,      // some people prefer to insert dash between integer and fraction: 3-1/4 rather than 3 1/4
    UseFractSymbol = 0x200,    // indicates that a limited set of fractional values can be presented by a single glyph
    AppendUnitName = 0x400     // indicates that the numeric expression can be followed by the unit name
    };

enum class NumSequenceTraits
    {
    None = 0,
    Signed = 0x1,
    DecPoint = 0x2,
    Exponent = 0x4,
    Uom = 0x8
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
    IncompleteSequence = 2,
    NoEnclosure = 3
    };

//! Codes of problems might help in finding the source of the problem
enum class FormatProblemCode
    {
    NoProblems = 0,
    UnknownStdFormatName = 20011,
    UnknownUnitName = 20012,
    CNS_InconsistentFactorSet = 20051,  //!< All ratio factors between units must be bigger than one
    CNS_InconsistentUnitSet = 20052,    //!< Each pair of UOM's for parts of combo-numbers should yeild a ratio > 1
    CNS_UncomparableUnits = 20053,      //!< Units provided on the argument list are not comparable
    CNS_InvalidUnitName = 20054,        //!< Not-recognizd unit name or unit is not associated with a Phenomenon
    CNS_InvalidMajorUnit = 20055,       //!< The MajorUnit in ComboNumbers is null or invalid
    QT_PhenomenonNotDefined = 20101,
    QT_PhenomenaNotSame = 20102,
    QT_InvalidTopMidUnits = 20103,
    QT_InvalidMidLowUnits = 20104,
    QT_InvalidUnitCombination = 20105,
    QT_InvalidSyntax = 20106,
    FUS_InvalidSyntax = 20151,
    NFS_InvalidSpecName = 20161,
    NFS_DuplicateSpecName = 20162,
    NFS_InvalidJsonObject = 20163,
    DIV_UnknownDivider = 25001,
    NA_InvalidSign = 25101,             // Numeric Accumulator problems
    NA_InvalidPoint = 25102,
    NA_InvalidExponent = 25103,
    NA_InvalidSyntax = 25104
    };

enum class FormatProblemLevel  // these levels should be used for assigning the Problem code
    {
    Undefined = 0,
    Notice = 1,
    Warning = 10000,
    Critical = 20000
    };

enum class CodepointSize
    {
    Zero = 0,
    Single = 1,
    Double = 2,
    Quatro = 4
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

enum class AccumulatorState
    {
    Init = 0,
    Integer = 1,
    Fraction = 2,
    Exponent = 3,
    Complete = 4,
    Real = 5,
    Failure = 10,
    RejectedSymbol = 11
    };

enum class ParsingSegmentType
    {
    Integer = 1,
    Real = 2,
    Fraction = 3,
    NotNumber = 10,
    EndOfLine = 11
    };

enum class CursorSectionState
    {
    Success = 0,
    Complete = 1,
    Failure = 2,
    RejectedSymbol = 10,
    Undefined = 20
    };

enum class CursorSectionType
    {
    Undefined = 0,
    Word = 1,
    Numeric = 2
    };

enum class FormatSpecialCodes
    {
    SignatureNull = 0,
    SignatureN = 1,
    SignatureF = 2,
    SignatureNF = 3,
    SignatureNU =4,
    SignatureNFU = 5,
    SignatureNUNU = 6,
    SignatureNUNFU = 7,
    SignatureNUNUNU= 8,
    SignatureNUNUNFU = 9,
    SignatureNCNCN = 10,
    SignatureNCCN = 11,
    SignatureNCC  = 12,
    SignatureCNCN = 13,
    SignatureCNC = 14,
    SignatureCCN = 13,
    SignatureInvalid = 100
    };


struct FormatProblemDetail
    {
    private:
        FormatProblemCode  m_code;

    public:

        FormatProblemDetail() { m_code = FormatProblemCode::NoProblems; }
        FormatProblemDetail(FormatProblemCode code) {  m_code = code;  }
        bool IsCritical() const { return (static_cast<int>(m_code) > static_cast<int>(FormatProblemLevel::Critical)); }
        bool IsWarning() const {
            return (static_cast<int>(m_code) < static_cast<int>(FormatProblemLevel::Critical) &&
                static_cast<int>(m_code) > static_cast<int>(FormatProblemLevel::Warning));
            }
        bool IsProblem() const { return m_code != FormatProblemCode::NoProblems; }
        bool NoProblem() const { return m_code == FormatProblemCode::NoProblems; }

        FormatProblemCode GetProblemCode() const { return m_code; }
        UNITS_EXPORT bool UpdateProblemCode(FormatProblemCode code);
        UNITS_EXPORT Utf8String GetProblemDescription() const;
    };

struct Utils
    {
    UNITS_EXPORT static Utf8String ShowSignOptionName(ShowSignOption opt);
    UNITS_EXPORT static ShowSignOption NameToSignOption(Utf8CP name);
    static int DecimalPrecisionToInt(DecimalPrecision decP) { return static_cast<int>(decP); }
    UNITS_EXPORT static DecimalPrecision DecimalPrecisionByIndex(size_t num);
    UNITS_EXPORT static double DecimalPrecisionFactor(DecimalPrecision decP, int index);
    UNITS_EXPORT static Utf8CP GetParameterCategoryName(ParameterCategory parcat);
    UNITS_EXPORT static Utf8String PresentationTypeName(PresentationType type);
    UNITS_EXPORT static PresentationType NameToPresentationType(Utf8CP name);
    UNITS_EXPORT static Utf8String SignOptionName(ShowSignOption opt);
    UNITS_EXPORT static Utf8String DecimalPrecisionName(DecimalPrecision prec);
    UNITS_EXPORT static Utf8String FractionallPrecisionName(FractionalPrecision prec);
    UNITS_EXPORT static Utf8String FractionBarName(FractionBarType bar);
    UNITS_EXPORT static FractionBarType NameToFractionBarType(Utf8CP name);
    UNITS_EXPORT static FractionalPrecision FractionalPrecisionByDenominator(size_t prec);
    static int FormatTraitsBit(FormatTraits zcValue) { return static_cast<int>(zcValue); }
    static size_t TextLength(Utf8CP text) { return (nullptr == text) ? 0 : strlen(text); }
    UNITS_EXPORT static const size_t FractionalPrecisionDenominator(FractionalPrecision prec);
    UNITS_EXPORT static size_t AppendText(Utf8P buf, size_t bufLen, size_t index, Utf8CP str);
    UNITS_EXPORT static bool IsNameNullOrEmpty(Utf8CP name);
    static Utf8CP SubstituteEmptyOrNull(Utf8CP name, Utf8CP subs) { return IsNameNullOrEmpty(name) ? subs : name; }
    //UNITS_EXPORT static Utf8CP GetFormatProblemDescription(FormatProblemCode code);
    UNITS_EXPORT static bool AreUnitsComparable(BEU::UnitCP un1, BEU::UnitCP u2);
    static size_t MinInt(size_t a, size_t b) { return(a <= b) ? a : b; }
    static size_t MaxInt(size_t a, size_t b) { return(a >= b) ? a : b; }
    // UNITS_EXPORT static Utf8String FormatProblemDescription(FormatProblemCode code);
    UNITS_EXPORT static Utf8String AppendUnitName(Utf8CP txtValue, Utf8CP unitName = nullptr, Utf8CP space = nullptr);
    //#if defined(FUNCTION_NOT_USED)
    //int StdFormatCodeValue(StdFormatCode code) { return static_cast<int>(code); }
    //static double DecimalPrecisionFactor(DecimalPrecision decP, int index = -1);
    //UNITS_EXPORT static Utf8String GetSignature(Utf8CP text);
    UNITS_EXPORT static Utf8String HexDump(Utf8CP txt, int len);
    UNITS_EXPORT static Utf8CP HexByte(Utf8Char c, Utf8P buf, size_t bufLen);
    UNITS_EXPORT static Utf8Char MatchingDivider(Utf8Char div);
    UNITS_EXPORT static int IndexOf(Utf8Char c, Utf8CP text);
    UNITS_EXPORT static size_t NumberOfUtf8Bytes(size_t code);
    UNITS_EXPORT static Utf8String AccumulatorStateName(AccumulatorState state);
    UNITS_EXPORT static Utf8String CharToString(Utf8Char c);
    UNITS_EXPORT static Utf8String FormatSpecTypeToName(FormatSpecType type);
    UNITS_EXPORT static FormatSpecType NameToFormatSpecType(Utf8CP name);
    static Utf8CP GetCharsOrNull(Utf8StringCR str) { return str.empty() ? nullptr : str.c_str(); }
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

        static Utf8String FPN_Composite() { return "composite"; }
        static Utf8String FPN_Numeric() { return "numeric"; }
        static Utf8String FPN_Undefined() { return "undefined"; }

        static Utf8String FPN_Decimal() { return "Decimal"; }
        static Utf8String FPN_Fractional() { return "Fractional"; }
        static Utf8String FPN_Scientific() { return "Scientific"; }
        static Utf8String FPN_ScientificNorm() { return "ScientificNorm"; }
        static Utf8String FPN_Stop100() { return "Stop100"; }
        static Utf8String FPN_Stop1000() { return "Stop1000"; }
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
        static Utf8String FPN_FractBarHoriz() { return "Horizontal"; }
        static Utf8String FPN_FractBarOblique() { return "Oblique"; }
        static Utf8String FPN_FractBarDiagonal() { return "Diagonal"; }
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
        static const Utf8Char FPV_singleSpace() { return ' '; }
        static const Utf8CP DefaultName() { return "*"; }
        static const Utf8CP EmptyString() { return ""; }
        static const Utf8CP BlankString() { return " "; }
        static const Utf8Char EndOfLine() { return '\0'; }
        static const bool IsEndOfLine(Utf8Char c) { return ('\0' == c); }
        static const Utf8Char NumberSymbol() { return '0'; }
        static const Utf8Char SpaceSymbol() { return 's'; }
        static const Utf8Char WordSymbol() { return 'a'; }
        static const Utf8Char ColonSymbol() { return ':'; }
        static const Utf8Char SignSymbol() { return '-'; }
        static const Utf8Char UOMLink() { return '_'; }
        static const Utf8Char FractionSymbol() { return 'r'; }
        static const Utf8CP FailedOperation() { return "Operation failed"; }
        static const PresentationType DefaultPresentaitonType() { return PresentationType::Decimal; }
        static const ShowSignOption DefaultSignOption() { return ShowSignOption::OnlyNegative; }
        static const DecimalPrecision DefaultDecimalPrecision() { return  DecimalPrecision::Precision6; }
        static const size_t DefaultDecimalPrecisionIndex() { return static_cast<int>(DefaultDecimalPrecision()); }
        static const size_t MaxDecimalPrecisionIndex() { return static_cast<int>(DecimalPrecision::Precision12); }
        static const FractionalPrecision DefaultFractionalPrecision() { return  FractionalPrecision::Over_64; }
        static const size_t DefaultFractionalDenominator() { return Utils::FractionalPrecisionDenominator(DefaultFractionalPrecision()); }
        static const FormatTraits DefaultFormatTraits() { return static_cast<FormatTraits>(static_cast<int>(FormatTraits::KeepDecimalPoint) | static_cast<int>(FormatTraits::KeepSingleZero)); }
        static const FormatTraits UnitizedFormatTraits() { return static_cast<FormatTraits>(static_cast<int>(FormatTraits::KeepDecimalPoint) | static_cast<int>(FormatTraits::KeepSingleZero) | static_cast<int>(FormatTraits::AppendUnitName)); }
        static const double FPV_MaxTokenLength() { return 256; }
        static const unsigned char UTF_2ByteMask() { return  0xE0; }      // 11100000 - complement will select 5 upper bits
        static const unsigned char UTF_2ByteMark() { return  0xC0; }      // 11000000
        static const unsigned char UTF_3ByteMask() { return  0xF0; }    // 11110000  - complement will select 4 upper bits
        static const unsigned char UTF_3ByteMark() { return  0xE0; }    // 11100000
        static const unsigned char UTF_4ByteMask() { return   0xF8; }   // 11111000  - complement will select 3 upper bits 
        static const unsigned char UTF_4ByteMark() { return  0xF0; }    // 11110000
        static const size_t UTF_2ByteSelector() { return  0x1F; }  // 00011111
        static const size_t UTF_3ByteSelector() { return  0xF; }   // 00001111
        static const size_t UTF_4ByteSelector() { return  0x7; }   // 00000111
        static const size_t UTF_5ByteSelector() { return  0x3; }   // 00000011
        static const size_t UTF_6ByteSelector() { return  0x1; }   // 00000001
        static const unsigned char UTF_TrailingByteMask() { return  0xC0; } // 11000000 - complement will select trailing bits
        static const unsigned char UTF_TrailingByteMark() { return  0x80; } // 10000000 - indicator of the trailing bytes and also an ASCII char
        static const unsigned char UTF_TrailingBitsMask() { return  0x3F; }    // 00111111
        static const unsigned char UTF_NonASCIIMark() { return  0x80; }    // 10000000 - indicator of non ASCII symbol
        static const unsigned char UTF_MultiByteMask() { return  0x40; }    // 01000000 - indicator of the first byte
        static const unsigned char UTF_UpperBitShift() { return  6; }
        static const unsigned char ASCIImask() { return  0x7F; }
        static const bool IsASCII(size_t symbol) { return (symbol <= ASCIImask()); }
        static const bool IsASCIIChar(Utf8Char c) { return (c & FormatConstant::UTF_NonASCIIMark()) == 0; }
        static const Utf8Char ASCIIcode(size_t symbol) { return (Utf8Char)(symbol & ASCIImask()); }
        static const int DigitValue(Utf8Char dig) { return (int)(dig - '0'); }
        static const  int GetTrailingShift() { return UTF_UpperBitShift(); }
        UNITS_EXPORT static const bool IsLittleEndian();
        UNITS_EXPORT static const size_t GetSequenceLength(unsigned char c);
        static bool IsTrailingByteValid(unsigned char c) { return (UTF_TrailingByteMark() == (c & UTF_TrailingByteMask())); }     
        
        UNITS_EXPORT static bool GetTrailingBits(unsigned char c, Utf8P outBits);
        UNITS_EXPORT static size_t ExtractTrailingBits(unsigned char c, size_t shift);
        UNITS_EXPORT static bool GetCodeBits(unsigned char c, size_t seqLength, size_t index, size_t* outBits);
        static bool IsNegligible(double dval) { return (fabs(dval) < FormatConstant::FPV_MinTreshold()); }
        static bool IsIgnored(double dval) { return (dval < 0.0 || fabs(dval) < FormatConstant::FPV_MinTreshold()); }
        static bool IsMagnitudeOne(double dval) { return (fabs(1.0 - fabs(dval)) < FormatConstant::FPV_MinTreshold()); }
        UNITS_EXPORT static const unsigned char TriadBitMask(size_t threeBit);
        static const Utf8CP BoolText(bool t) { return t ? "true" : "false"; }
        static const Utf8CP AllocError() { return "AllocError"; }
        static const Utf8CP HexSymbols() { return "0123456789ABCDEF"; }
        static const Utf8CP FUSDividers() { return "()[]{}|"; }
        static const Utf8CP FUSDividerMatch() { return ")(][}{|"; }
        static const Utf8CP ASCIIprintable() { return " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"; }
        static const Utf8CP ASCIImap() { return "b!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"; }
        UNITS_EXPORT static const size_t* FractionCodes();
        static const Utf8CP DefaultFormatName() { return "DefaultReal"; }
        static const Utf8CP DefaultFormatAlias() { return "real"; }
        static const Utf8CP FUSJsonName() { return "name"; }
        static const Utf8CP FUSJsonAlias() { return "alias"; }
        static const Utf8CP FUSJsonUnit() { return "unit"; }
        static const Utf8CP FUSJsonValue() { return "value"; }
        static const Utf8CP FUSJsonDispValue() { return "displayValue"; }
        static const Utf8CP FUSJsonPersist() { return "persistFUS"; }
        static const Utf8CP FUSJsonDispaly() { return "displayFUS"; }
        static const bool IsBoolEqual(bool val, bool ref) { return val == ref; }
        static const bool IsIntEqual(int val, int ref) { return val == ref; }
        static const bool IsRealEqual(double val, double ref, double resolution) { return fabs(val - ref) <= fabs(resolution); }


        static const size_t* SpecialUOM()
            {
            // the array of codes holds integer codes of several special unicode characters that could be used in a specific
            //   context for denoting units Of Measurement. two characters 'degC' and 'degF' are included for the future growth
            //     they represent a special case of degrees of Celsius and degrees of Fahrenheit that could be used with Chinese characters
            //                        "     $     %     '     ^    deg    degC    degF
            static size_t cod[9] = { 0x22, 0x24, 0x25, 0x27, 0x5e, 0xB0, 0x2103, 0x2109, 0x0};
            return cod;
            }

        static const bool IsFractionSymbol(size_t code)
            {
            const size_t* p = FractionCodes();
            while (*p != FormatConstant::EndOfLine())
                {
                if (*p == code)
                    return true;
                p++;
                }
            return false;
            }
        UNITS_EXPORT const static FormatSpecialCodes ParsingPatternCode(Utf8CP name);
        UNITS_EXPORT const static Utf8CP SpecialAngleSymbol(Utf8String name);
        UNITS_EXPORT const static Utf8CP SpecialLengthSymbol(Utf8String name);
    };

END_BENTLEY_FORMATTING_NAMESPACE