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

BENTLEY_NAMESPACE_TYPEDEFS(Formatting);

BEGIN_BENTLEY_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS(NumericFormat)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatParameterSet)
DEFINE_POINTER_SUFFIX_TYPEDEFS(NumericTriad)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatParameter)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatDictionary)

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
	NoZeroes = 201,
	LeadingZeroes = 202,
	TrailingZeroes = 203,
	BothZeroes = 204,
	ShowDotZero = 205, // with precision 0 by default only integer part will be shows. This option will force .0 after the int part 
	Replace0Empty = 206,  // formatter will return the empy string if the result is 0
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

enum class ZeroControl
{
	NoZeroes = 0,
	LeadingZeroes = 1,
	TrailingZeroes = 2,
	BothZeroes = 3
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


static const char * GetParameterCategoryName(ParameterCategory parcat)
{
	static const char * CategoryNames[] = { "DataType", "Sign", "Presentation", "Zeroes", "DecPrecision", "FractPrecision", "RoundType",
		"FractionBar", "AngleFormat", "Alignment", "Separator", "Padding", "Mapping" };
	return CategoryNames[(int)parcat];
}


//=======================================================================================
// @bsiclass
//=======================================================================================
struct NumericFormat
{
private:
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

public:

	int PrecisionValue() const;
	double PrecisionFactor() const;
	DecimalPrecision ConvertToPrecision(int num);

	NumericFormat()
	{
		m_presentationType = PresentationType::Decimal;
		m_signOption = ShowSignOption::OnlyNegative;
		m_ZeroControl = ZeroControl::NoZeroes;
		m_showDotZero = false;
		m_replace0Empty = false;
		m_decPrecision = DecimalPrecision::Precision6;
		m_fractPrecision = FractionalPrecision::Sixteenth;
		m_useThousandsSeparator = false;
		m_decimalSeparator = '.';
		m_thousandsSeparator = ',';
	}

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
	BENTLEYDLL_EXPORT bool IfInsertSeparator(bool useSeparator) { return (m_useThousandsSeparator && m_thousandsSeparator != 0 && useSeparator); }
	BENTLEYDLL_EXPORT bool SetUseSeparator(bool set) { return m_useThousandsSeparator = set; }
	BENTLEYDLL_EXPORT int GetDecimalPrecision() { return PrecisionValue(); }
	BENTLEYDLL_EXPORT void SetDecimalPrecision(DecimalPrecision prec) { m_decPrecision = prec; }
	BENTLEYDLL_EXPORT bool IsPrecisionZero() { return (m_decPrecision == DecimalPrecision::Precision0); }
	BENTLEYDLL_EXPORT int IntPartToText (double n, char * bufOut, int bufLen, bool useSeparator);
	BENTLEYDLL_EXPORT int FormatInteger (int n, char* bufOut, int bufLen);
	BENTLEYDLL_EXPORT int FormatDouble(double dval, char* buf, int bufLen);
	BENTLEYDLL_EXPORT int FormatBinaryByte (unsigned char n, CharP bufOut, int bufLen);
	BENTLEYDLL_EXPORT int FormatBinaryShort (short int n, char* bufOut, int bufLen, bool useSeparator);
	BENTLEYDLL_EXPORT int FormatBinaryInt (int n, char* bufOut, int bufLen, bool useSeparator);
	BENTLEYDLL_EXPORT int FormatBinaryDouble (double x, char* bufOut, int bufLen, bool useSeparator);
	BENTLEYDLL_EXPORT static int RightAlignedCopy(CharP dest, int destLen, bool termZero, CharCP src, int srcLen);
	BENTLEYDLL_EXPORT Utf8String FormatDouble(double dval);
	BENTLEYDLL_EXPORT Utf8String FormatInteger(int ival);
	BENTLEYDLL_EXPORT Utf8String ByteToBinaryText(unsigned char n);
	BENTLEYDLL_EXPORT Utf8String ShortToBinaryText(short int n, bool useSeparator);
	BENTLEYDLL_EXPORT Utf8String IntToBinaryText(int n, bool useSeparator);
	BENTLEYDLL_EXPORT Utf8String DoubleToBinaryText(double x, bool useSeparator);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct FormatParameterSet : NumericFormat
{
private:
	RoundingType        m_roundType;
	double              m_roundFactor;
	// unused - Utf8Char            m_leftPadding;
	// unused - Utf8Char            m_rightPadding;
	// unused - int                 m_boundaryLength;
	// unused - FieldAlignment      m_alignment;

public:

	FormatParameterSet()
	{
		m_roundType = RoundingType::RoundAwayFrom0;
		m_roundFactor = 0.5;
		// unused - m_leftPadding;
		// unused - m_rightPadding;
		// unused - m_boundaryLength;
		// unused - m_alignment;
	}
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

	double GetWhole() { return m_negative ? -m_dval : m_dval; }
	void ProcessValue(double dval, DecimalPrecision prec) { SetValue(dval, prec);  Convert(); }
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
	BENTLEYDLL_EXPORT size_t GetCount() { return m_paramList.size(); }
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

	static const unsigned char m_twoByteMask = 0xE0;   // 11100000 - complement will select 5 upper bits
	static const unsigned char m_twoByteMark = 0xC0;   // 11000000
	static const unsigned char m_threeByteMask = 0xF0; // 11110000  - complement will select 4 upper bits
	static const unsigned char m_threeByteMark = 0xE0; // 11100000
	static const unsigned char m_fourByteMask = 0xF8;  // 11111000  - complement will select 3 upper bits
	static const unsigned char m_fourByteMark = 0xF0;  // 11110000
	static const unsigned char m_trailingByteMask = 0xC0; // 11000000 - complement will select trailing bits
	static const unsigned char m_trailingByteMark = 0x80; // 10000000 - indicator of the trailing bytes and also an ASCII char
	static const unsigned char m_trailingBits = 0x3F; // 00111111
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
	BENTLEYDLL_EXPORT const char Get2ByteMask() { return m_twoByteMask; }
	BENTLEYDLL_EXPORT const char Get3ByteMask() { return m_threeByteMask; }
	BENTLEYDLL_EXPORT const char Get4ByteMask() { return m_fourByteMask; }
	BENTLEYDLL_EXPORT const char Get2ByteMark() { return m_twoByteMark; }
	BENTLEYDLL_EXPORT const char Get3ByteMark() { return m_threeByteMark; }
	BENTLEYDLL_EXPORT const char Get4ByteMark() { return m_fourByteMark; }
	BENTLEYDLL_EXPORT const char GetTrailingByteMask() { return m_trailingByteMask; }
	BENTLEYDLL_EXPORT const char GetTrailingByteMark() { return m_trailingByteMark; }
	BENTLEYDLL_EXPORT const char GetTrailingBitsMask() { return m_trailingBits; }
	BENTLEYDLL_EXPORT const size_t GetSequenceLength(unsigned char c);
	BENTLEYDLL_EXPORT bool IsTrailingByteValid(unsigned char c);
	BENTLEYDLL_EXPORT bool GetTrailingBits(unsigned char c, CharP outBits);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct FormatingScannerCursor
{
private:
	CharCP m_text;            // pointer to the head of the string
	size_t m_totalScanLength; // this is the total length of the byte sequence that ought to be scanned/parsed
	size_t m_cursorPosition;  // the index of the next byte to be scanned
	int m_lastScannedCount;   // the number of bytes processed in the last step
	union { uint8_t octet[4];  unsigned int word; } m_code; // container for the scanned bytes
	bool m_isASCII;          // flag indicating that the last scanned byte is ASCII
	UnicodeConstant m_unicodeConst; // reference to constants and character processors

public:
	//! Construct a cursor attached to the given Utf8 string 
	BENTLEYDLL_EXPORT FormatingScannerCursor(CharCP utf8Text, int scanLength);
	BENTLEYDLL_EXPORT UnicodeConstant GetConstants() { return m_unicodeConst; }
};

END_BENTLEY_NAMESPACE
