/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/Formatting.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "Bentley.h"
#include "WString.h"
#include "RefCounted.h"
//#include <Bentley/BeTimeUtilities.h>
#include <ctype.h>
#include <ctime>
#include <chrono>
#include <math.h>

BENTLEY_NAMESPACE_TYPEDEFS(Formatting);

BEGIN_BENTLEY_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(NumericFormat)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatParameterSet)
DEFINE_POINTER_SUFFIX_TYPEDEFS(NumericTriad)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatParameter)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatDictionary)
DEFINE_POINTER_SUFFIX_TYPEDEFS(UnicodeConstant)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormattingScannerCursor)
typedef RefCountedPtr<NumericFormat>   NumericFormatBPtr;

enum class ParameterCode
    {
    NoSign = 101,
    OnlyNegative = 102,
    SignAlways = 103,
    NegativeParenths = 104,
    Decimal = 151,
    Fractional = 152,
    Sientific = 153,
    ScientificNorm = 154,
    Binary = 155,
    NoZeroControl = 201,
    LeadingZeroes = 202,
    TrailingZeroes = 203,
    KeepSingleZero = 204,
    KeepDecimalPoint = 205, 
    Replace0Empty = 206,    // formatter will return the empy string if the result is 0
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
    Integer = 1,
    Double = 2,
    Symbol = 3,
    String = 4
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

enum class ZeroControl: int
{
    None = 0,
    LeadingZeroes = 0x1,
    TrailingZeroes = 0x2,
    KeepDecimalPoint = 0x4,
    KeepSingleZero = 0x8
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

static const char * GetParameterCategoryName(ParameterCategory parcat)
    {
    static const char * CategoryNames[] = {"DataType", "Sign", "Presentation", "Zeroes", "DecPrecision", "FractPrecision", "RoundType",
        "FractionBar", "AngleFormat", "Alignment", "Separator", "Padding", "Mapping"};
    return CategoryNames[(int)parcat];
    }


//=======================================================================================
// @bsiclass
//=======================================================================================
struct NumericFormat
{
private:
    double              m_minTreshold;
    PresentationType    m_presentationType;      // Decimal, Fractional, Sientific, ScientificNorm
    ShowSignOption      m_signOption;            // NoSign, OnlyNegative, SignAlways, NegativeParenths
    ZeroControl         m_ZeroControl;           // NoZeroes, LeadingZeroes, TrailingZeroes, BothZeroes
    bool                m_showDotZero;
    bool                m_replace0Empty;
    DecimalPrecision    m_decPrecision;          // Precision0...12
    FractionalPrecision m_fractPrecision;
    bool                m_useThousandsSeparator; // UseThousandSeparator
    Utf8Char            m_decimalSeparator;      // DecimalComma, DecimalPoint, DecimalSeparator
    Utf8Char            m_thousandsSeparator;    // ThousandSepComma, ThousandSepPoint, ThousandsSeparartor

    BENTLEYDLL_EXPORT void DefaultInit(size_t precision);
    BENTLEYDLL_EXPORT double RoundedValue(double dval, double round);
    BENTLEYDLL_EXPORT int TrimTrailingZeroes(CharP buf, int index);
    BENTLEYDLL_EXPORT int InsertChar(CharP buf, int index, char c, int num);
public:

    BENTLEYDLL_EXPORT NumericFormat() { DefaultInit(6); }
    BENTLEYDLL_EXPORT NumericFormat(size_t precision) { DefaultInit(precision); }

    BENTLEYDLL_EXPORT bool IfKeepTrailingZeroes() { return ((static_cast<int>(m_ZeroControl) & static_cast<int>(ZeroControl::TrailingZeroes)) != 0); }
    BENTLEYDLL_EXPORT bool IfUseLeadingZeroes() { return ((static_cast<int>(m_ZeroControl) & static_cast<int>(ZeroControl::LeadingZeroes)) != 0); }
    BENTLEYDLL_EXPORT bool IfKeepDecimalPoint() { return ((static_cast<int>(m_ZeroControl) & static_cast<int>(ZeroControl::KeepDecimalPoint)) != 0); }
    BENTLEYDLL_EXPORT bool IfKeepSingleZero() { return ((static_cast<int>(m_ZeroControl) & static_cast<int>(ZeroControl::KeepSingleZero)) != 0); }
    BENTLEYDLL_EXPORT void SetKeepTrailingZeroes(bool keep);
    BENTLEYDLL_EXPORT void SetUseLeadingZeroes(bool use);
    BENTLEYDLL_EXPORT void SetKeepDecimalPoint(bool keep);
    BENTLEYDLL_EXPORT void SetKeepSingleZero(bool keep);

    BENTLEYDLL_EXPORT int PrecisionValue() const;
    BENTLEYDLL_EXPORT double PrecisionFactor() const;
    BENTLEYDLL_EXPORT DecimalPrecision ConvertToPrecision(size_t num);
    BENTLEYDLL_EXPORT void SetPresentationType(PresentationType type) { m_presentationType = type; }
    BENTLEYDLL_EXPORT PresentationType GetPresentationType() const { return m_presentationType; }
    BENTLEYDLL_EXPORT void SetSignOption(ShowSignOption opt) { m_signOption = opt; }
    BENTLEYDLL_EXPORT ShowSignOption GetSignOption() const { return m_signOption; }
    BENTLEYDLL_EXPORT void setZeroControl(ZeroControl opt) { m_ZeroControl = opt; }
    BENTLEYDLL_EXPORT ZeroControl getZeroControl() const { return m_ZeroControl; }
    BENTLEYDLL_EXPORT bool getShowDotZero() { return m_showDotZero; }
    BENTLEYDLL_EXPORT bool getReplace0Empty() const { return m_replace0Empty; }
    BENTLEYDLL_EXPORT bool setShowDotZero(bool set) { return m_showDotZero = set; }
    BENTLEYDLL_EXPORT bool setReplace0Empty(bool set) { return m_replace0Empty = set; }
    BENTLEYDLL_EXPORT FractionalPrecision SetfractionaPrecision(FractionalPrecision precision) { return m_fractPrecision = precision; }
    BENTLEYDLL_EXPORT FractionalPrecision GetFractionalPrecision() const { return m_fractPrecision; }
    BENTLEYDLL_EXPORT Utf8Char SetDecimalSeparator(Utf8Char sep) { return m_decimalSeparator = sep; }
    BENTLEYDLL_EXPORT Utf8Char GetDecimalSeparator() const { return m_decimalSeparator; }
    BENTLEYDLL_EXPORT Utf8Char SetThousandSeparator(char sep) { return m_thousandsSeparator = sep; }
    BENTLEYDLL_EXPORT Utf8Char GetThousandSeparator() const { return m_thousandsSeparator; }
    BENTLEYDLL_EXPORT bool IfInsertSeparator(bool useSeparator) { return (m_useThousandsSeparator && m_thousandsSeparator != 0 && useSeparator);}
    BENTLEYDLL_EXPORT bool SetUseSeparator(bool set) { return m_useThousandsSeparator = set; }
    BENTLEYDLL_EXPORT int GetDecimalPrecision()  { return PrecisionValue(); }
    BENTLEYDLL_EXPORT void SetDecimalPrecision(DecimalPrecision prec) { m_decPrecision = prec; }
    BENTLEYDLL_EXPORT bool IsPrecisionZero() {    return (m_decPrecision == DecimalPrecision::Precision0);}
    BENTLEYDLL_EXPORT int IntPartToText (double n, char * bufOut, int bufLen, bool useSeparator);
    BENTLEYDLL_EXPORT int FormatInteger (int n, char* bufOut, int bufLen);
    BENTLEYDLL_EXPORT int FormatDouble(double dval, char* buf, int bufLen);
    BENTLEYDLL_EXPORT int FormatBinaryByte (unsigned char n, CharP bufOut, int bufLen);
    BENTLEYDLL_EXPORT int FormatBinaryShort (short int n, char* bufOut, int bufLen, bool useSeparator);
    BENTLEYDLL_EXPORT int FormatBinaryInt (int n, char* bufOut, int bufLen, bool useSeparator);
    BENTLEYDLL_EXPORT int FormatBinaryDouble (double x, char* bufOut, int bufLen, bool useSeparator);
    BENTLEYDLL_EXPORT static int RightAlignedCopy(CharP dest, int destLen, bool termZero, CharCP src, int srcLen);
    BENTLEYDLL_EXPORT Utf8String FormatDouble(double dval);
    BENTLEYDLL_EXPORT Utf8String FormatRoundedDouble(double dval, double round);
    BENTLEYDLL_EXPORT Utf8String FormatInteger(int ival);
    BENTLEYDLL_EXPORT Utf8String ByteToBinaryText(unsigned char n);
    BENTLEYDLL_EXPORT Utf8String ShortToBinaryText(short int n, bool useSeparator);
    BENTLEYDLL_EXPORT Utf8String IntToBinaryText(int n, bool useSeparator);
    BENTLEYDLL_EXPORT Utf8String DoubleToBinaryText(double x, bool useSeparator);
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
private:
    double m_dval;            // we keep the originally submitted value for its sign and possible reverse operations
    double m_topValue;        
    double m_midValue;
    double m_lowValue;
    DecimalPrecision m_decPrecision;
    int    m_topToMid;
    int    m_midToLow;
    bool   m_init;
    bool   m_midAssigned;
    bool   m_lowAssigned;
    bool   m_negative;

    void Convert();
    void SetValue(double dval, DecimalPrecision prec);
    NumericTriad();

public:
    
    NumericTriad(double dval, int topMid, int midLow, DecimalPrecision prec)
        {
        SetValue(dval, prec);
        m_topToMid = topMid;
        m_midToLow = midLow;
        m_init = true;
        Convert();
        }

    double GetWhole() { return m_negative ? -m_dval: m_dval; }
    void ProcessValue(double dval, DecimalPrecision prec)  {  SetValue(dval, prec);  Convert(); }
    void SetRatio(int topToMid, int midToLow) { m_topToMid = topToMid; m_midToLow = midToLow; }
    void SetPrecision(DecimalPrecision prec) { m_decPrecision = prec; }
    double GetTopValue() { return m_topValue; }
    double GetMidValue() { return m_midValue; }
    double GetlowValue() { return m_lowValue; }
    Utf8String FormatWhole(DecimalPrecision prec);
    Utf8String FormatTriad(Utf8StringCP topName, Utf8StringCP midName, Utf8StringCP lowName, bool includeZero);

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

public:

    BENTLEYDLL_EXPORT FormatParameter(Utf8CP name, ParameterCategory cat, ParameterCode code, ParameterDataType type)
        {
        m_paramName = name;
        m_category = cat;
        m_paramCode = code;
        m_paramType = type;
        }

    BENTLEYDLL_EXPORT Utf8StringCR GetName() { return m_paramName; }
    BENTLEYDLL_EXPORT int CompareName(Utf8StringCR other) { return strcmp(m_paramName.c_str(), other.c_str()); }
    BENTLEYDLL_EXPORT ParameterCategory GetCategory() { return m_category; }
    BENTLEYDLL_EXPORT CharCP GetCategoryName() { return GetParameterCategoryName(m_category); }
    BENTLEYDLL_EXPORT ParameterCode GetParameterCode() { return m_paramCode; }
    BENTLEYDLL_EXPORT size_t GetParameterCodeValue() { return (size_t)m_paramCode; }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct FormatDictionary
{
private:
    bvector<FormatParameter> m_paramList;

    BENTLEYDLL_EXPORT void InitLoad();

public:

    BENTLEYDLL_EXPORT FormatDictionary() { InitLoad(); }
    BENTLEYDLL_EXPORT int GetCount() { return (int)m_paramList.size(); }
    BENTLEYDLL_EXPORT void AddParameter(FormatParameterCR par) { m_paramList.push_back(par); return; }
    BENTLEYDLL_EXPORT FormatParameterP FindParameterByName(Utf8StringCR paramName);
    BENTLEYDLL_EXPORT FormatParameterP FindParameterByCode(ParameterCode paramCode);
    BENTLEYDLL_EXPORT FormatParameterP GetParameterByIndex(int index);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct UnicodeConstant
{
private:
    static const unsigned char m_twoByteMask = 0xE0;      // 11100000 - complement will select 5 upper bits
    static const unsigned char m_twoByteMark = 0xC0;      // 11000000
    static const unsigned char m_threeByteMask = 0xF0;    // 11110000  - complement will select 4 upper bits
    static const unsigned char m_threeByteMark = 0xE0;    // 11100000
    static const unsigned char m_fourByteMask = 0xF8;     // 11111000  - complement will select 3 upper bits
    static const unsigned char m_fourByteMark = 0xF0;     // 11110000
    static const unsigned char m_trailingByteMask = 0xC0; // 11000000 - complement will select trailing bits
    static const unsigned char m_trailingByteMark = 0x80; // 10000000 - indicator of the trailing bytes and also an ASCII char
    static const unsigned char m_trailingBits = 0x3F;     // 00111111
    static const size_t m_upperBitShift = 6;
    bool m_isLittleEndian;

    static bool CheckEndian()
        {
        union { short int s; char b[4]; } un;
        un.s = 1;
        return (un.b[0] == (char)1);
        }


public:
    BENTLEYDLL_EXPORT UnicodeConstant() { m_isLittleEndian = CheckEndian(); }
    BENTLEYDLL_EXPORT const char Get2ByteMask() { return m_twoByteMask; }                 // 11100000 - complement will select 5 upper bits
    BENTLEYDLL_EXPORT const char Get3ByteMask() { return m_threeByteMask; }				  // 11000000
    BENTLEYDLL_EXPORT const char Get4ByteMask() { return m_fourByteMask; }				  // 11110000  - complement will select 4 upper bits
    BENTLEYDLL_EXPORT const char Get2ByteMark() { return m_twoByteMark; }				  // 11100000
    BENTLEYDLL_EXPORT const char Get3ByteMark() { return m_threeByteMark; }				  // 11111000  - complement will select 3 upper bits
    BENTLEYDLL_EXPORT const char Get4ByteMark() { return m_fourByteMark; }				  // 11110000
    BENTLEYDLL_EXPORT const char GetTrailingByteMask() { return m_trailingByteMask; }	  // 11000000 - complement will select trailing bits
    BENTLEYDLL_EXPORT const char GetTrailingByteMark() { return m_trailingByteMark; }	  // 10000000 - indicator of the trailing bytes and also an ASCII char
    BENTLEYDLL_EXPORT const char GetTrailingBitsMask() { return m_trailingBits; }		  // 00111111
    BENTLEYDLL_EXPORT const size_t GetSequenceLength(unsigned char c);
    BENTLEYDLL_EXPORT bool IsTrailingByteValid(unsigned char c);
    BENTLEYDLL_EXPORT bool GetTrailingBits(unsigned char c, CharP outBits);
    BENTLEYDLL_EXPORT bool GetCodeBits(unsigned char c, size_t seqLength, size_t index, size_t* outBits);
    BENTLEYDLL_EXPORT int GetTrailingShift() { return m_upperBitShift; }
    BENTLEYDLL_EXPORT bool IsLittleEndian(); 
    BENTLEYDLL_EXPORT bool ForceBigEndian();
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
    Utf8String m_text;            // pointer to the head of the string
    size_t m_totalScanLength; // this is the total length of the byte sequence that ought to be scanned/parsed
    size_t m_cursorPosition;  // the index of the next byte to be scanned
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
    BENTLEYDLL_EXPORT size_t TrueIndex(size_t index, size_t wordSize);
    BENTLEYDLL_EXPORT int AddTrailingByte();
    BENTLEYDLL_EXPORT size_t GetCurrentPosition() { return m_cursorPosition; }
    BENTLEYDLL_EXPORT size_t SetCurrentPosition(size_t position) { return m_cursorPosition = position; }
    BENTLEYDLL_EXPORT int FormattingScannerCursor::ProcessTrailingByte(char c, int* bits);
public:
    //! Construct a cursor attached to the given Utf8 string 
    BENTLEYDLL_EXPORT FormattingScannerCursor(CharCP utf8Text, int scanLength);
    BENTLEYDLL_EXPORT FormattingScannerCursor(FormattingScannerCursorCR other);
    BENTLEYDLL_EXPORT UnicodeConstant* GetConstants() { return m_unicodeConst; }
    BENTLEYDLL_EXPORT size_t GetNextSymbol();
    BENTLEYDLL_EXPORT size_t GetNextCodePoint();
    BENTLEYDLL_EXPORT bool IsError() { return (m_status != ScannerCursorStatus::Success); }
    BENTLEYDLL_EXPORT bool IsSuccess() { return (m_status == ScannerCursorStatus::Success); }
    BENTLEYDLL_EXPORT ScannerCursorStatus GetOperationStatus() { return m_status;}
    BENTLEYDLL_EXPORT bool IsEndOfLine() { return (m_text[m_cursorPosition] == '\0'); }
    BENTLEYDLL_EXPORT bool IsASCII() { return m_isASCII; }
    BENTLEYDLL_EXPORT int CodePointCount();
    BENTLEYDLL_EXPORT void Rewind();
    BENTLEYDLL_EXPORT size_t GetUnicode() { return m_uniCode; }
    BENTLEYDLL_EXPORT size_t GetLastScanned() { return m_lastScannedCount; }
    BENTLEYDLL_EXPORT size_t SkipBlanks();
    BENTLEYDLL_EXPORT Utf8String SelectKeyWord();
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
    BENTLEYDLL_EXPORT FormatStopWatch();
    BENTLEYDLL_EXPORT Utf8String LastIntervalMetrics(size_t amount);
    BENTLEYDLL_EXPORT Utf8String LastInterval(double factor);
    

};
END_BENTLEY_NAMESPACE
