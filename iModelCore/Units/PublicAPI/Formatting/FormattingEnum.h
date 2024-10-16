/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Formatting/FormattingDefinitions.h>
#include <locale>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//===================================================
// Enumerations
//===================================================

//=======================================================================================
// @bsienum
//=======================================================================================
enum class SignOption
    { 
    NoSign,             //!< indicates that sign should not be used at all (like absolute value)
    OnlyNegative,       //!< indicates that only "-" will be used for negative numbers
    SignAlways,         //!< indicates that sign symbol should explicitly shown even for positive numbers
    NegativeParentheses //!< indicates that negative numbers shoul be enclosed in parenthesis instead of using a negative sign
    };

//=======================================================================================
// @bsienum
//=======================================================================================
enum class PresentationType
    {
    Decimal,// 
    Fractional,
    Scientific, // scientific with 1 digit presenting the integer part
    Station, // Civil Engineering Stationing (ie 1+00)
    Bearing, // Bearing angle e.g. N05:00:00E. Requires provided quantities to be of the angle phenomenon
    Azimuth, // Azimuth angle e.g. 45°30'00". Requires provided quantities to be of the angle phenomenon
    Ratio, // indicates the size compairson between two numbers. Eg. 1:2 or 0.3:1
    };

//=======================================================================================
// @bsienum
//=======================================================================================
enum class ScientificType
    {
    Normalized,
    ZeroNormalized,
    };

//=======================================================================================
// @bsienum
//=======================================================================================
enum class FormatTraits : int32_t
{
    None                    = 0,
    TrailingZeroes          = 1 << 0, //!< Indicates that one or more insignificant zeroes are to be added after the last digit of the fraction.
    KeepSingleZero          = 1 << 1, //!< Indicates that the fractional part of the number is required when the fraction is zero.
    ZeroEmpty               = 1 << 2, //!< Indicates that zero value should be presented by an empty string.
    KeepDecimalPoint        = 1 << 3, //!< Indicates that the decimal point is should be presented when the fraction is zero.
    ApplyRounding           = 1 << 4, //!< Use the rounding factor.
    FractionDash            = 1 << 5, //!< Use a dash between integer and fraction instead of a space: 3-1/4 rather than 3 1/4.
    ShowUnitLabel           = 1 << 6, //!< Indicates that the numeric expression should be followed by the unit name.
    PrependUnitLabel        = 1 << 7, //!< Indicates the position of the Unit name shifts from the right side of the value to the left.
    Use1000Separator        = 1 << 8, //!< Indicates that thousands in the integer part of the number should be separated by a special char (. or,).
    ExponenentOnlyNegative  = 1 << 9, //!< Indicates that if an exponent value is positive to not include a +. By default a sign, + or -, is always shown.
};

//=======================================================================================
//! Number of points after decimal point given significance in the scientific notation
//! representation of floating point numbers.
// @bsienum
//=======================================================================================
// TODO: The double 100101.125 with Precision3 will be formatted as 1.001e+5, in line
// with the current implimentation. @Caleb.Shafer mentioned that we may want to have that
// same double/DecimalPrecision should concider the three points of decimal precision to
// be the .125 before scientific normalization.
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
    Precision12 = 12,
    Max = Precision12
    };

//=======================================================================================
// @bsienum
//=======================================================================================
enum class FractionalPrecision
    {
    Whole = 0,       //!< Ex. 30
    Half = 1,        //!< Ex. 30 1/2
    Quarter = 2,     //!< Ex. 30 1/4
    Eighth = 3,      //!< Ex. 30 1/8
    Sixteenth = 4,   //!< Ex. 30 1/16
    Over_32 = 5,     //!< Ex. 30 1/32
    Over_64 = 6,     //!< Ex. 30 1/64
    Over_128 = 7,    //!< Ex. 30 1/128
    Over_256 = 8,    //!< Ex. 30 1/256
    };

//=======================================================================================
// @bsienum
//=======================================================================================
enum class ScannerCursorStatus
    {
    Success,
    InvalidSymbol,
    IncompleteSequence,
    NoEnclosure
    };

//=======================================================================================
//! Codes of problems might help in finding the source of the problem
// @bsienum
//=======================================================================================
enum class FormatProblemCode
    {
    NoProblems = 0,
    UnknownStdFormatName = 20011,
    UnknownUnitName = 20012,
    NotInitialized = 20013,
    TooManyDigits = 20014,              //!< All numeric format must have maximum of 15 digits in each of the integer part, decimal part, denominator part, and exponent part 
    CVS_InconsistentFactorSet = 20051,  //!< All ratio factors between units must be bigger than one
    CVS_InconsistentUnitSet = 20052,    //!< Each pair of UOM's for parts of combo-numbers should yeild a ratio > 1
    CVS_UncomparableUnits = 20053,      //!< Units provided on the argument list are not comparable
    CVS_InvalidUnitName = 20054,        //!< Not-recognizd unit name or unit is not associated with a Phenomenon
    CVS_InvalidMajorUnit = 20055,       //!< The MajorUnit in ComboNumbers is null or invalid
    CVS_ConstantAsUnit = 20056,
    QT_PhenomenonNotDefined = 20101,
    QT_PhenomenaNotSame = 20102,
    QT_InvalidTopMidUnits = 20103,
    QT_InvalidMidLowUnits = 20104,
    QT_InvalidUnitCombination = 20105,
    QT_InvalidSyntax = 20106,
    QT_ConversionFailed = 20107,
    QT_InvertingZero = 20108,
    QT_NoValueOrUnitFound = 20109,
    QT_InvalidRatioArgument = 20110,
    QT_ValueOutOfRange = 20111,
    QT_MathematicOperationFoundButIsNotAllowed = 20112,
    QT_BearingPrefixOrSuffixMissing = 20113,
    FUS_InvalidSyntax = 20151,
    NFS_Undefined = 20160,
    NFS_InvalidSpecName = 20161,
    NFS_DuplicateSpecName = 20162,
    NFS_DuplicateSpecNameOrAlias = 20163,
    NFS_InvalidJsonObject = 20164,
    NFS_MissingUnit = 20165,
    DIV_UnknownDivider = 25001,
    NA_InvalidSign = 25101,             // Numeric Accumulator problems
    NA_InvalidPoint = 25102,
    NA_InvalidExponent = 25103,
    NA_InvalidSyntax = 25104,
    PS_InvalidColonizedExpression = 26101,
    PS_MissingFUS = 2602,
    PS_MissingCompositeSpec = 2603,
    PS_MismatchingFUS = 2604,
    SFS_InsertingNamelessFUS = 2701,
    SFS_DuplicateFUSName = 2702,
    SFS_FailedToMakeFUS = 2703,
    NMQ_InvalidUnitName = 2801,
    NMQ_MissingName = 2802
    };

//=======================================================================================
//! these levels should be used for assigning the Problem code
// @bsienum
//=======================================================================================
enum class FormatProblemLevel
    {
    Undefined = 0,
    Notice = 1,
    Warning = 10000,
    Critical = 20000
    };

//=======================================================================================
// @bsienum
//=======================================================================================
enum class FormatSpecType
    {
    None,   //!< NumericFormatSpec only.
    Single, //!< NumericFormatSpec and CompositeValueSpec with 1 UOM : Major.
    Double, //!< NumericFormatSpec and CompositeValueSpec with 2 UOMs: Major and Middle.
    Triple, //!< NumericFormatSpec and CompositeValueSpec with 3 UOMs: Major, Middle, and Minor.
    Quad    //!< NumericFormatSpec and CompositeValueSpec with 4 UOMs: Major, Middle, Minor, and Sub.
    };

//=======================================================================================
// @bsienum
//=======================================================================================
enum class ParsingSegmentType
    {
    Integer = 1,
    Real = 2,
    Fraction = 3,
    NotNumber = 10,
    EndOfLine = 11
    };

//=======================================================================================
// @bsienum
//=======================================================================================
enum class FormatSpecialCodes
    {
    SignatureNull = 0,
    SignatureN = 1,
    SignatureF = 2,
    SignatureNF = 3,
    SignatureNU = 4,
    SignatureNFU = 5,
    SignatureNUNU = 6,
    SignatureNUNFU = 7,
    SignatureNUNUNU= 8,
    SignatureNUNUNFU = 9,
    SignatureNN = 10,
    SignatureNCNCN = 20,
    SignatureNCCN  = 21,
    SignatureNCC   = 22,
    SignatureCNCN  = 23,
    SignatureCNC   = 24,
    SignatureCCN   = 25,
    SignatureNCNC  = 26,
    SignatureNCN   = 27,
    SignatureNC    = 28,
    SignatureCN    = 29,
    SignatureMCNCN = 31,
    SignatureMCNC  = 32,
    SignatureMCCN  = 33,
    SignatureMCN = 34,
    SignatureInvalid = 100
    };

//=======================================================================================
// @bsienum
//=======================================================================================
enum class AlternativeFormatComparison
    {
    GreaterThan,
    LessThan
    };

enum class RatioType
    {
    OneToN, // 1 on the left side of the colon
    NToOne, // 1 on the right side of the colon
    ValueBased, // if abs value is greater than 1, 1 is on left. If abs value is less than 1, 1 is on right
    UseGreatestCommonDivisor, // scales to the greatest common divisor, integer ratio. e.g. 3:10
    };

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct FormatProblemDetail
{
private:
    FormatProblemCode m_code;

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
    UNITS_EXPORT void Reset() { m_code = FormatProblemCode::NoProblems; }
};

//=======================================================================================
//! @bsistruct
//=======================================================================================
struct Utils
{
    UNITS_EXPORT static Utf8String GetScientificTypeString(ScientificType type);
    //! Parses the provided string into a ScientificType.
    //! @return True if successfully parsed into a ScientificType. Otherwise, false.
    UNITS_EXPORT static bool ParseScientificType(ScientificType& sciType, Utf8StringCR sciTypeString);

    UNITS_EXPORT static Utf8String GetSignOptionString(SignOption opt);

    //! Parses the provided string into a SignOption.
    //! @return True if successfully parsed into a SignOption. Otherwise, false.
    UNITS_EXPORT static bool ParseSignOption(SignOption& signOpt, Utf8CP signOptString);

    static int32_t DecimalPrecisionToInt(DecimalPrecision decP) {return static_cast<int32_t>(decP);}
    UNITS_EXPORT static bool GetDecimalPrecisionByInt(DecimalPrecision& decP, int32_t num);

    //! Returns a factor, as a double, representing the provided precision.
    UNITS_EXPORT static double DecimalPrecisionFactor(DecimalPrecision decP);

    UNITS_EXPORT static Utf8String GetPresentationTypeString(PresentationType type);
    //! Parses the provided string into a SignOption.
    //! @return True if successfully parsed into a SignOption. Otherwise, false.
    UNITS_EXPORT static bool ParsePresentationType(PresentationType& type, Utf8CP typeString);

    UNITS_EXPORT static bool ParseRatioType(RatioType& mode, Utf8CP modeString);
    UNITS_EXPORT static Utf8String GetRatioTypeString(RatioType mode);
    
    UNITS_EXPORT static bool FractionalPrecisionByDenominator(FractionalPrecision& out, const int32_t prec);
    UNITS_EXPORT static int32_t FractionalPrecisionDenominator(FractionalPrecision prec);

    UNITS_EXPORT static Utf8String AppendUnitName(Utf8CP txtValue, Utf8CP unitName = nullptr, Utf8CP space = nullptr);
    UNITS_EXPORT static Utf8Char MatchingDivider(Utf8Char div);
};

// A collection of important global constants that are used across the whole formatting domain.
//=======================================================================================
// @bsistruct
//=======================================================================================
struct FormatConstant
{
private:
    // Set up user locale data
    static std::locale s_userLocale;
    static bool s_useLocale;

public:
    UNITS_EXPORT static void UseLocale(bool useLocale) { s_useLocale = useLocale; }
    static const double DefaultRoundingFactor() { return 0.0; }
    static PresentationType const DefaultPresentaitonType() { return PresentationType::Decimal; }
    static SignOption const DefaultSignOption() { return SignOption::OnlyNegative; }
    static FormatTraits const DefaultFormatTraits() { return FormatTraits::None; }
    static DecimalPrecision const DefaultDecimalPrecision() { return  DecimalPrecision::Precision6; }
    static FractionalPrecision const DefaultFractionalPrecision() { return  FractionalPrecision::Over_64; }
    static Utf8Char const DefaultDecimalSeparator() { return FormatConstant::FPV_DecimalSeparator(); }
    static Utf8Char const DefaultThousandSeparator() { return FormatConstant::FPV_ThousandSeparator(); }
    static Utf8String const DefaultUomSeparator() { return FormatConstant::BlankString(); };
    static Utf8Char const DefaultStationSeparator() {return '+';}
    static int const DefaultMinWidth() { return 0; }
    static Utf8String const DefaultSpacer() {return " ";}
    static Utf8String const DefaultSeparator() {return " ";}
    static RatioType const DefaultRatioType() {return RatioType::ValueBased;} 

    // FPN prefix stands for FormatParameterName
    static Utf8String FPN_NoSign() { return "NoSign"; }
    static Utf8String FPN_OnlyNegative() { return "OnlyNegative"; }
    static Utf8String FPN_SignAlways() { return "SignAlways"; }
    static Utf8String FPN_NegativeParenths() { return "NegativeParentheses"; }

    static Utf8String FPN_ScientificNormalized() {return "Normalized";}
    static Utf8String FPN_ScientificZeroNormalized() {return "ZeroNormalized";}

    static Utf8String FPN_Decimal() { return "Decimal"; }
    static Utf8String FPN_Fractional() { return "Fractional"; }
    static Utf8String FPN_Scientific() { return "Scientific"; }
    static Utf8String FPN_Station() {return "Station";}
    static Utf8String FPN_Ratio() {return "Ratio";}
    static Utf8String FPN_Azimuth() {return "Azimuth";}
    static Utf8String FPN_Bearing() {return "Bearing";}

    static Utf8String FPN_RatioOneToN() { return "OneToN"; }
    static Utf8String FPN_RatioNToOne() { return "NToOne"; }
    static Utf8String FPN_RatioValueBased() { return "ValueBased"; }
    static Utf8String FPN_RatioUseGreatestCommonDivisor() { return "UseGreatestCommonDivisor"; }

    static Utf8String FPN_TrailZeroes() {return "TrailZeroes";}
    static Utf8String FPN_LeadZeroes() {return "LeadZeroes";}
    static Utf8String FPN_KeepSingleZero() {return "KeepSingleZero";}
    static Utf8String FPN_ZeroEmpty() {return "ZeroEmpty";}
    static Utf8String FPN_KeepDecimalPoint() {return "KeepDecimalPoint";}
    static Utf8String FPN_ApplyRounding() {return "ApplyRounding";}
    static Utf8String FPN_FractionDash() {return "FractionDash";}
    static Utf8String FPN_ShowUnitLabel() {return "ShowUnitLabel";}
    static Utf8String FPN_PrependUnitLabel() {return "PrependUnitLabel";}
    static Utf8String FPN_Use1000Separator() {return "Use1000Separator";}
    static Utf8String FPN_ExponentOnlyNegative() {return "ExponentOnlyNegative";}
    static const double FPV_MinTreshold() { return 1.0e-14; }  // format parameter default values
    static const double FPV_RoundFactor() { return 0.50000000001; }  // rounding additive
    UNITS_EXPORT static const Utf8Char FPV_DecimalSeparator();
    UNITS_EXPORT static const Utf8Char FPV_ThousandSeparator();
    static const Utf8CP EmptyString() { return ""; }
    static const Utf8CP BlankString() { return " "; }
    static const Utf8Char EndOfLine() { return '\0'; }
    static const bool IsEndOfLine(Utf8Char c) { return ('\0' == c); }
    static const Utf8Char NumberSymbol() { return '0'; }
    static const Utf8Char SpaceSymbol() { return 's'; }
    static const Utf8Char SignSymbol() { return '-'; }
    static const unsigned char UTF_2ByteMask() { return  0xE0; }      // 11100000 - complement will select 5 upper bits
    static const unsigned char UTF_2ByteMark() { return  0xC0; }      // 11000000
    static const unsigned char UTF_3ByteMask() { return  0xF0; }    // 11110000  - complement will select 4 upper bits
    static const unsigned char UTF_3ByteMark() { return  0xE0; }    // 11100000
    static const unsigned char UTF_4ByteMask() { return   0xF8; }   // 11111000  - complement will select 3 upper bits 
    static const unsigned char UTF_4ByteMark() { return  0xF0; }    // 11110000
    static const size_t UTF_2ByteSelector() { return  0x1F; }  // 00011111
    static const size_t UTF_3ByteSelector() { return  0xF; }   // 00001111
    static const size_t UTF_4ByteSelector() { return  0x7; }   // 00000111
    static const unsigned char UTF_TrailingByteMask() { return  0xC0; } // 11000000 - complement will select trailing bits
    static const unsigned char UTF_TrailingByteMark() { return  0x80; } // 10000000 - indicator of the trailing bytes and also an ASCII char
    static const unsigned char UTF_TrailingBitsMask() { return  0x3F; }    // 00111111
    static const unsigned char UTF_NonASCIIMark() { return  0x80; }    // 10000000 - indicator of non ASCII symbol
    static const unsigned char UTF_UpperBitShift() { return  6; }
    static const unsigned char ASCIImask() { return  0x7F; }
    static const bool IsASCIIChar(Utf8Char c) { return (c & FormatConstant::UTF_NonASCIIMark()) == 0; }
    static const Utf8Char ASCIIcode(size_t symbol) { return (Utf8Char)(symbol & ASCIImask()); }
    static const  int GetTrailingShift() { return UTF_UpperBitShift(); }
    UNITS_EXPORT static const size_t GetSequenceLength(unsigned char c);
    static bool IsTrailingByteValid(unsigned char c) { return (UTF_TrailingByteMark() == (c & UTF_TrailingByteMask())); }     
    UNITS_EXPORT static bool GetTrailingBits(unsigned char c, Utf8P outBits);
    static bool IsNegligible(double dval) { return (fabs(dval) < FormatConstant::FPV_MinTreshold()); }
    static bool IsIgnored(double dval) { return (dval < 0.0 || fabs(dval) < FormatConstant::FPV_MinTreshold()); }
    UNITS_EXPORT static const unsigned char TriadBitMask(size_t threeBit);
    static const Utf8CP Dividers() { return "()[]{}|"; }
    static const Utf8CP DividerMatch() { return ")(][}{|"; }
    UNITS_EXPORT const static FormatSpecialCodes ParsingPatternCode(Utf8CP name);
};

END_BENTLEY_FORMATTING_NAMESPACE
