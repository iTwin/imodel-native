/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Formatting/FormattingEnum.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Formatting/FormattingDefinitions.h>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatProblemDetail)
//===================================================
// Enumerations
//===================================================

//=======================================================================================
// @bsienum
//=======================================================================================
enum class ShowSignOption
    { 
    NoSign = 0,              // indicates that sign should not be used at all (like absolute value)
    OnlyNegative = 1,        // indicates that only "-" will be used for negative numbers
    SignAlways = 2,          // indicates that sign symbol should explicitly shown even for positive numbers
    NegativeParentheses = 3  // indicates that negative numbers shoul be enclosed in parenthesis instead of using a negative sign
    };

//=======================================================================================
// @bsienum
//=======================================================================================
enum class PresentationType
    {
    Decimal = 1,
    Fractional = 2,
    Scientific = 3,      // scientific with 1 digit presenting the integer part
    Station = 4,
    };

//=======================================================================================
// @bsienum
//=======================================================================================
enum class ScientificType
    {
    Standard        = 1,
    Normal          = 2,
    Engineering     = 3,
    };

//=======================================================================================
// @bsienum
//=======================================================================================
enum class FormatTraits : int32_t
    {
    None             = 0x000,
    TrailingZeroes   = 0x001, // Indicates that one or more insignificant zeroes are to be added after the last digit of the fraction.
    KeepSingleZero   = 0x002, // Indicates that the fractional part of the number is required when the fraction is zero.
    ZeroEmpty        = 0x004, // Indicates that zero value should be presented by an empty string.
    KeepDecimalPoint = 0x008, // Indicates that the decimal point is should be presented when the fraction is zero.
    ApplyRounding    = 0x010, // Use the rounding factor.
    FractionDash     = 0x020, // Use a dash between integer and fraction instead of a space: 3-1/4 rather than 3 1/4.
    ShowUnitLabel    = 0x040, // Indicates that the numeric expression should be followed by the unit name.
    PrependUnitLabel = 0x080, // Indicates the position of the Unit name shifts from the right side of the value to the left.
    Use1000Separator = 0x100, // Indicates that thousands in the integer part of the number should be separated by a special char (. or,).
    ExponenentOnlyNegative   = 0x200, // Indicates that if an exponent value is positive to not include a +. By default a sign, + or -, is always shown.
    LeadingZeroes    = 0x400,
    ExponentZero     = 0x800,
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
    Success = 0,
    InvalidSymbol = 1,
    IncompleteSequence = 2,
    NoEnclosure = 3
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
    CVS_InconsistentFactorSet = 20051,  //!< All ratio factors between units must be bigger than one
    CVS_InconsistentUnitSet = 20052,    //!< Each pair of UOM's for parts of combo-numbers should yeild a ratio > 1
    CVS_UncomparableUnits = 20053,      //!< Units provided on the argument list are not comparable
    CVS_InvalidUnitName = 20054,        //!< Not-recognizd unit name or unit is not associated with a Phenomenon
    CVS_InvalidMajorUnit = 20055,       //!< The MajorUnit in ComboNumbers is null or invalid
    QT_PhenomenonNotDefined = 20101,
    QT_PhenomenaNotSame = 20102,
    QT_InvalidTopMidUnits = 20103,
    QT_InvalidMidLowUnits = 20104,
    QT_InvalidUnitCombination = 20105,
    QT_InvalidSyntax = 20106,
    FUS_InvalidSyntax = 20151,
    NFS_Undefined = 20160,
    NFS_InvalidSpecName = 20161,
    NFS_DuplicateSpecName = 20162,
    NFS_DuplicateSpecNameOrAlias = 20163,
    NFS_InvalidJsonObject = 20164,
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
enum class FormatSpecType : int32_t
    {
    None   = 0, //!< NumericFormatSpec only.
    Single = 1, //!< NumericFormatSpec and CompositeValueSpec with 1 UOM : Major.
    Double = 2, //!< NumericFormatSpec and CompositeValueSpec with 2 UOMs: Major and Middle.
    Triple = 3, //!< NumericFormatSpec and CompositeValueSpec with 3 UOMs: Major, Middle, and Minor.
    Quad   = 4  //!< NumericFormatSpec and CompositeValueSpec with 4 UOMs: Major, Middle, Minor, and Sub.
    };

//=======================================================================================
// @bsienum
//=======================================================================================
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
    SignatureNU =4,
    SignatureNFU = 5,
    SignatureNUNU = 6,
    SignatureNUNFU = 7,
    SignatureNUNUNU= 8,
    SignatureNUNUNFU = 9,
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
    UNITS_EXPORT static Utf8String ScientificTypeName(ScientificType type);
    UNITS_EXPORT static bool NameToScientificType(Utf8StringCR name, ScientificType& out);
    UNITS_EXPORT static bool NameToSignOption(Utf8CP name, ShowSignOption& out);
    static int DecimalPrecisionToInt(DecimalPrecision decP) { return static_cast<int>(decP); }
    UNITS_EXPORT static bool DecimalPrecisionByIndex(const size_t num, DecimalPrecision& out);
    UNITS_EXPORT static double DecimalPrecisionFactor(DecimalPrecision decP, int index);
    UNITS_EXPORT static Utf8String PresentationTypeName(PresentationType type);
    UNITS_EXPORT static bool NameToPresentationType(Utf8CP name, PresentationType& type);
    UNITS_EXPORT static Utf8String SignOptionName(ShowSignOption opt);
    UNITS_EXPORT static bool FractionalPrecisionByDenominator(const size_t prec, FractionalPrecision& out);
    static size_t TextLength(Utf8CP text) { return (nullptr == text) ? 0 : strlen(text); }
    UNITS_EXPORT static const size_t FractionalPrecisionDenominator(FractionalPrecision prec);
    UNITS_EXPORT static size_t AppendText(Utf8P buf, size_t bufLen, size_t index, Utf8CP str);
    static Utf8CP SubstituteEmptyOrNull(Utf8CP name, Utf8CP subs) { return Utf8String::IsNullOrEmpty(name) ? subs : name; }
    static Utf8CP SubstituteNull(Utf8CP name, Utf8CP subs) { return (nullptr == name) ? subs : name; }
    static size_t MinInt(size_t a, size_t b) { return(a <= b) ? a : b; }
    static size_t MaxInt(size_t a, size_t b) { return(a >= b) ? a : b; }
    UNITS_EXPORT static Utf8String AppendUnitName(Utf8CP txtValue, Utf8CP unitName = nullptr, Utf8CP space = nullptr);
    UNITS_EXPORT static Utf8Char MatchingDivider(Utf8Char div);
    UNITS_EXPORT static int IndexOf(Utf8Char c, Utf8CP text);
    UNITS_EXPORT static Utf8CP SkipBlanks(Utf8CP str);
    UNITS_EXPORT static Utf8Char GetFirstSignificantChar(Utf8CP str);
    UNITS_EXPORT static Utf8Char GetLastSignificantChar(Utf8CP str);
    static bool IsJsonCandidate(Utf8CP str) {return (GetFirstSignificantChar(str) == '{') && (GetLastSignificantChar(str) == '}');}
    UNITS_EXPORT static Utf8String GetCurrentThousandSeparator();
    UNITS_EXPORT static Utf8String GetCurrentDecimalSeparator();
    UNITS_EXPORT static Utf8String GetCurrentGrouping();
};

// A collection of important global constants that are used across the whole formatting domain.
//=======================================================================================
// @bsistruct                                                    David.Fox-Rabinovitz  11/2016
//=======================================================================================
struct FormatConstant
{
public:
    static const double DefaultRoundingFactor() { return 0.0; }
    static PresentationType const DefaultPresentaitonType() { return PresentationType::Decimal; }
    static ShowSignOption const DefaultSignOption() { return ShowSignOption::OnlyNegative; }
    static FormatTraits const DefaultFormatTraits() { return FormatTraits::None; }
    static DecimalPrecision const DefaultDecimalPrecision() { return  DecimalPrecision::Precision6; }
    static FractionalPrecision const DefaultFractionalPrecision() { return  FractionalPrecision::Over_64; }
    static Utf8Char const DefaultDecimalSeparator() { return FormatConstant::FPV_DecimalSeparator(); }
    static Utf8Char const DefaultThousandSeparator() { return FormatConstant::FPV_ThousandSeparator(); }
    static Utf8String const DefaultUomSeparator() { return FormatConstant::BlankString(); };
    static Utf8Char const DefaultStationSeparator() {return '+';}
    static int const DefaultMinWidth() { return 0; }

    static const size_t MinDecimalPrecisionIndex() { return static_cast<size_t>(DecimalPrecision::Precision0); }
    static const size_t MaxDecimalPrecisionIndex() { return static_cast<size_t>(DecimalPrecision::Precision12); }

    // FPN prefix stands for FormatParameterName
    static Utf8String FPN_NoSign() { return "NoSign"; }
    static Utf8String FPN_OnlyNegative() { return "OnlyNegative"; }
    static Utf8String FPN_SignAlways() { return "SignAlways"; }
    static Utf8String FPN_NegativeParenths() { return "NegativeParenths"; }

    static Utf8String FPN_ScientificStandard() {return "standard";}
    static Utf8String FPN_ScientificNormal() {return "normal";}
    static Utf8String FPN_ScientificEngineering() {return "engineering";}

    static Utf8String FPN_Decimal() { return "Decimal"; }
    static Utf8String FPN_Fractional() { return "Fractional"; }
    static Utf8String FPN_Scientific() { return "Scientific"; }
    static Utf8String FPN_ScientificNorm() { return "ScientificNorm"; }
    static Utf8String FPN_Station() {return "Station";}
    static Utf8String FPN_Station100() { return "Station100"; }
    static Utf8String FPN_Station1000() { return "Station1000"; }
    static Utf8String FPN_FractBarHoriz() { return "Horizontal"; }
    static Utf8String FPN_FractBarOblique() { return "Oblique"; }
    static Utf8String FPN_FractBarDiagonal() { return "Diagonal"; }

    static Utf8String FPN_TrailZeroes() {return "trailZeroes";}
    static Utf8String FPN_KeepSingleZero() {return "keepSingleZero";}
    static Utf8String FPN_ZeroEmpty() {return "zeroEmpty";}
    static Utf8String FPN_KeepDecimalPoint() {return "keepDecimalPoint";}
    static Utf8String FPN_ApplyRounding() {return "applyRounding";}
    static Utf8String FPN_FractionDash() {return "fractionDash";}
    static Utf8String FPN_ShowUnitName() {return "showUnitName";}
    static Utf8String FPN_PrependUnitName() {return "prependUnitName";}
    static Utf8String FPN_Use1000Separator() {return "use1000Separator";}
    static Utf8String FPN_ExponentOnlyNegative() {return "exponentOnlyNegative";}
    static const double FPV_MinTreshold() { return 1.0e-14; }  // format parameter default values
    static const double FPV_RoundFactor() { return 0.50000000001; }  // rounding additive
    static const Utf8Char FPV_DecimalSeparator() { return '.'; }
    static const Utf8Char FPV_ThousandSeparator() { return ','; }
    static const Utf8CP EmptyString() { return ""; }
    static const Utf8CP BlankString() { return " "; }
    static const Utf8Char EndOfLine() { return '\0'; }
    static const bool IsEndOfLine(Utf8Char c) { return ('\0' == c); }
    static const Utf8Char NumberSymbol() { return '0'; }
    static const Utf8Char SpaceSymbol() { return 's'; }
    static const Utf8Char WordSymbol() { return 'a'; }
    static const Utf8Char SignSymbol() { return '-'; }
    static const Utf8Char FractionSymbol() { return 'r'; }
    static const size_t DefaultDecimalPrecisionIndex() { return static_cast<int>(DefaultDecimalPrecision()); }
    static const size_t DefaultFractionalDenominator() { return Utils::FractionalPrecisionDenominator(DefaultFractionalPrecision()); }
    static const FormatTraits UnitizedFormatTraits() { return static_cast<FormatTraits>(static_cast<int>(FormatTraits::KeepDecimalPoint) | static_cast<int>(FormatTraits::KeepSingleZero) | static_cast<int>(FormatTraits::ShowUnitLabel)); }
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
    UNITS_EXPORT static const unsigned char TriadBitMask(size_t threeBit);
    static const Utf8CP BoolText(bool t) { return t ? "true" : "false"; }
    static const Utf8CP AllocError() { return "AllocError"; }
    static const Utf8CP HexSymbols() { return "0123456789ABCDEF"; }
    static const Utf8CP FUSDividers() { return "()[]{}|"; }
    static const Utf8CP FUSDividerMatch() { return ")(][}{|"; }
    UNITS_EXPORT static const size_t* FractionCodes();
    static const Utf8CP DefaultFormatName() { return "DefaultRealU"; }
    static const Utf8CP DefaultFormatAlias() { return "real"; }
    static const Utf8CP FUSJsonValue() { return "value"; }
    static const Utf8CP FUSJsonDispValue() { return "displayValue"; }
    static const bool IsBoolEqual(bool val, bool ref) { return val == ref; }


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
};

END_BENTLEY_FORMATTING_NAMESPACE
