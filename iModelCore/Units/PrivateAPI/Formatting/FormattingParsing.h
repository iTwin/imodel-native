/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Formatting/FormattingDefinitions.h>
#include <Formatting/FormattingEnum.h>
#include <Units/Units.h>
#include <BeRapidJson/BeJsValue.h>

#include <cmath>
#include <cstdlib>
#include <cstdint>

namespace BEU = BentleyApi::Units;

BEGIN_BENTLEY_FORMATTING_NAMESPACE
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormattingDividers)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormattingScannerCursor)
DEFINE_POINTER_SUFFIX_TYPEDEFS(CursorScanPoint)
DEFINE_POINTER_SUFFIX_TYPEDEFS(NumberGrabber)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatParsingSegment)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatParsingSet)

//=======================================================================================
// @bsistruct
//=======================================================================================
struct CursorScanPoint
{
private:
    size_t m_uniCode;             // this is a Unicode codePoint of the selected string position
    size_t m_indx;                // index in the input string pointing to the first byte of the sequence
    size_t m_len;                 // the actual length of the sequence (for ASCII = 1)
    unsigned char m_bytes[4];     // the actual sequence of bytes containing a Unicode Symbol un UTF8
    Utf8Char m_patt;              // a "pattern" symbol 
    ScannerCursorStatus m_status; // constructor status

    void ProcessASCII(unsigned char c);

public:
    CursorScanPoint() : m_uniCode(0), m_indx(0), m_len(0), m_patt('\0'), m_status(ScannerCursorStatus::Success)
        {memset(m_bytes, 0, sizeof(m_bytes));}
    UNITS_EXPORT CursorScanPoint(Utf8CP input, size_t indx);
    UNITS_EXPORT void ProcessNext(Utf8CP input);
    UNITS_EXPORT void Iterate(Utf8CP input);
    bool IsDigit() { return (m_len == 1) && (m_patt == FormatConstant::NumberSymbol()); }
    bool IsSign() { return (m_len == 1) && (m_patt == FormatConstant::SignSymbol()); }
    bool IsSpace() { return (m_len == 1) && (m_patt == FormatConstant::SpaceSymbol()); }
    bool IsSeparator(Utf8Char const dec = '.', Utf8Char const thous = ',') { return (m_len == 1) && (m_patt == dec || m_patt == thous); }
    bool IsBar() { return (m_len == 1) && (m_patt == '/'); }
    bool IsExponent() {return (m_len == 1) && (m_patt == 'x'); }
    bool IsUnderscore() { return (m_len == 1) && (m_patt == '_'); }

    //! The caller is responsible for keeping the index inside the allowable range
    UNITS_EXPORT ScannerCursorStatus AppendTrailingByte(Utf8CP txt);
    Utf8Char GetPrecedingByte(Utf8CP txt) { return (m_indx > 0) ? txt[--m_indx] : FormatConstant::EndOfLine(); }

    size_t GetIndex() const { return m_indx; }
    size_t GetLength() const { return m_len; }
    Utf8Char GetAscii(){ return FormatConstant::ASCIIcode(m_uniCode); }
    unsigned char* GetBytes() { return m_bytes; }
    bool IsEndOfLine() { return m_len == 0; }
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct NumberGrabber
{
private:
    Utf8CP m_input;   // the text to explore
    size_t m_start;   // the starting position in the input string
    size_t m_next;    // the offset to the character following the numeric sequence
    int64_t m_ival;
    double m_dval;
    ParsingSegmentType m_type;
    FormatProblemDetail m_problem;

public:
    NumberGrabber() : NumberGrabber(nullptr) {}
    NumberGrabber(Utf8CP input, size_t start = 0) : m_input(input), m_start(start), m_next(start), m_type(ParsingSegmentType::NotNumber) {}
    ParsingSegmentType GetType() const { return m_type; }
    size_t GetStartIndex() const { return m_start; }
    size_t GetNextIndex() const { return m_next; }
    int64_t GetInteger() const { return m_ival; }
    double GetReal() const { return m_dval; }
    bool IsEndOfLine() const { return (m_type == ParsingSegmentType::EndOfLine); }
    size_t GetLength() const { return (m_type == ParsingSegmentType::NotNumber)? 0 : m_next - m_start; }
    bool HasProblem() const {return m_problem.IsProblem();}
    FormatProblemCode GetProblemCode() {return m_problem.GetProblemCode();}
    Utf8String GetProblemDescription() {return m_problem.GetProblemDescription();}

    //! Given an input string and a starting index, this Number Grabber will contain the resulting value.
    //! @param[in] input  The string to extract a value from.
    //! @param[in] start  The index to start parsing the string.
    //! @param[in] decimalSep   The character expected to represent decimal separation
    //! @param[in] thousandSep  The character expected to represent thousands separation
    //! @return The length of the value extracted.
    UNITS_EXPORT  size_t Grab(Utf8CP input = nullptr, size_t start = 0, Utf8Char const decimalSep = '.', Utf8Char const thousandSep = ',');
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct FormatParsingSegment
{
private:
    size_t m_start;         // start index to the input string
    bvector<CursorScanPoint> m_vect;
    Utf8String m_name;
    BEU::UnitCP m_unit;
    ParsingSegmentType m_type;
    int64_t m_ival;
    double m_dval;

    void Init(size_t start);
public:
    FormatParsingSegment() : m_start(0), m_type(ParsingSegmentType::NotNumber), m_ival(0), m_dval(0.0), m_unit(nullptr) {}
    UNITS_EXPORT FormatParsingSegment(NumberGrabberCR ng);
    UNITS_EXPORT FormatParsingSegment(bvector<CursorScanPoint> vect, size_t s, BEU::UnitCP refUnit = nullptr, FormatCP format=nullptr, QuantityFormatting::UnitResolver* resolver = nullptr);
    ParsingSegmentType GetType() { return m_type; }
    double GetReal() { return m_dval; }
    double GetAbsReal() { return fabs(m_dval); }
    double GetSign() { return (m_dval < 0) ? -1.0 : 1.0; }
    BEU::UnitCP GetUnit() {return m_unit;}
    bool IsColon() { return (m_type == ParsingSegmentType::NotNumber) && BeStringUtilities::StricmpAscii(m_name.c_str(), ":") == 0; }
    bool IsDoubleColon() { return (m_type == ParsingSegmentType::NotNumber) && BeStringUtilities::StricmpAscii(m_name.c_str(), "::") == 0;}
    bool IsTripleColon() { return (m_type == ParsingSegmentType::NotNumber) && BeStringUtilities::StricmpAscii(m_name.c_str(), ":::") == 0; }
        
    bool IsMinusColon() { return (m_type == ParsingSegmentType::NotNumber) && BeStringUtilities::StricmpAscii(m_name.c_str(), "-:") == 0; }
    bool IsMinusDoubleColon() { return (m_type == ParsingSegmentType::NotNumber) && BeStringUtilities::StricmpAscii(m_name.c_str(), "-::") == 0; }
    bool IsMinusTripleColon() { return (m_type == ParsingSegmentType::NotNumber) && BeStringUtilities::StricmpAscii(m_name.c_str(), "-:::") == 0; }
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct FormatParsingSet
{
private:
    Utf8String m_input;
    bvector<FormatParsingSegment> m_segs;
    BEU::UnitCP m_unit;     // optional reference to a "quantity" unit
    FormatProblemDetail m_problem;
    FormatCP m_format;
    QuantityFormatting::UnitResolver* m_resolver;

    //! Process's "colonized" expression using a Composite FUS
    //! Returns error codes when FUS does not match the expression.
    //! The input expression signature code mus be provided by the caller
    BEU::Quantity ComposeColonizedQuantity(Formatting::FormatSpecialCodes cod, FormatCP format = nullptr);
    BEU::Quantity ParseAndProcessTokens(Formatting::FormatSpecialCodes cod, FormatCP format, BEU::UnitCP inputUnit);
    // parsing helper methods
    BEU::Quantity ParseAzimuthFormat(FormatProblemCode* probCode, FormatCP format, BEU::UnitCP inputUnit);
    BEU::Quantity ParseBearingFormat(FormatProblemCode* probCode, FormatCP format, BEU::UnitCP inputUnit);
    BEU::Quantity ParseRatioFormat(FormatProblemCode* probCode,FormatCP format, BEU::UnitCP inputUnit);

    void SegmentInput(Utf8CP input, size_t start);

public:
    UNITS_EXPORT FormatParsingSet(Utf8CP input, BEU::UnitCP unit = nullptr, FormatCP format = nullptr, QuantityFormatting::UnitResolver* resolver = nullptr);
    bool HasProblem() const {return m_problem.IsProblem();}
    FormatProblemCode GetProblemCode() {return m_problem.GetProblemCode();}
    Utf8String GetProblemDescription() {return m_problem.GetProblemDescription();}
    BEU::UnitCP GetUnit() {return m_unit;}
    UNITS_EXPORT Utf8String GetSignature(bool distinct = true);
    UNITS_EXPORT BEU::Quantity GetQuantity(FormatProblemCode* probCode = nullptr, FormatCP format = nullptr);
    UNITS_EXPORT bool ValidateParsingFUS(int reqUnitCount, FormatCP format);
};

//=======================================================================================
// @bsistruct
//=======================================================================================
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
    UNITS_EXPORT FormattingDividers(Utf8CP div);
    UNITS_EXPORT FormattingDividers(FormattingDividersCR other);
    UNITS_EXPORT bool IsDivider(char c);
};

//=======================================================================================
// can be used for detecting occurances of dividers and their "mates" in text strings
//  
// @bsiclass
//=======================================================================================
struct FormatDividerInstance
{
private:
    Utf8Char m_div;
    Utf8Char m_mate;
    bvector<int> m_positions;
    int m_divCount;
    int m_mateCount;
    int m_totLen;
    FormatProblemDetail m_problem;  //DIV_UnknownDivider

public:
    UNITS_EXPORT FormatDividerInstance(Utf8CP  txt, Utf8Char div);
    UNITS_EXPORT FormatDividerInstance(Utf8CP  txt, Utf8CP divs);
    UNITS_EXPORT FormatDividerInstance():m_div(FormatConstant::EndOfLine()), m_mate(FormatConstant::EndOfLine()), m_divCount(0), m_mateCount(0), m_totLen(0){}
    int GetDivCount() { return m_divCount; }
    bool BracketsMatched() { return (0 < m_divCount && m_divCount == m_mateCount); }
    UNITS_EXPORT bool IsDivLast();
    int  GetFirstLocation() { return (m_positions.size() > 0) ? m_positions.front()-1 : -1; }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct FormattingScannerCursor
{
private:
    Utf8String m_text;           // pointer to the head of the string
    size_t m_totalScanLength;    // this is the total length of the byte sequence that ought to be scanned/parsed
    size_t m_breakIndex;         // special position  dividing the string into two parts
    FormattingDividers m_dividers;
    ScannerCursorStatus m_status;

public:

    UNITS_EXPORT FormattingScannerCursor(Utf8CP utf8Text, int scanLength, Utf8CP div = nullptr);
    FormattingScannerCursor(FormattingScannerCursorCR other) : m_dividers(other.m_dividers),
        m_text(other.m_text), m_status(other.m_status)
        {}
    size_t GetTotalLength() { return m_totalScanLength; }
    //! This method attempts to extract the content of the last "enclosure" - that is a group of
    //! characters enclosed into one of brackets: parenthesis, curvy bracket or square brackets
    //! if brackets are not detected - the returned word wil be empty
    //! "vertical line" divider is marked by single boolean argument because the divider and its mate are same
    UNITS_EXPORT Utf8String ExtractLastEnclosure();
    UNITS_EXPORT Utf8String ExtractBeforeEnclosure();
    UNITS_EXPORT Utf8String ExtractSegment(size_t from, size_t to);
};

//=======================================================================================
// Format JSON is externally authored (hand-written, and persisted inside KindOfQuantity
// definitions in ECSchemas), and some of it encodes numbers as JSON *strings* -
// e.g. "stationOffsetSize": "3". The Bentley fork of JsonCpp silently accepted that: its
// Value::asInt/asUInt/asInt64/asDouble/asFloat each had a `case stringValue:` that ran sscanf.
// BeJsConst does NOT - its accessors return the supplied default (0) for anything non-numeric,
// so "3" would silently become 0 instead of failing loudly.
// These helpers preserve the historical tolerance so existing schemas keep deserializing.
// @bsimethod
//=======================================================================================
inline bool TryCoerceJsonStringToDouble(BeJsConst val, double& out)
    {
    if (!val.isString())
        return false;
    Utf8String str = val.asString();
    if (str.empty())
        return false;
    // JsonCpp used sscanf("%lf"/"%d"/"%u"); strtod matches its locale-dependent behavior while
    // also letting us detect the "no digits consumed" case that sscanf reported via its return.
    char* end = nullptr;
    double parsed = strtod(str.c_str(), &end);
    if (end == str.c_str())
        return false;
    out = parsed;
    return true;
    }

inline double JsonToDouble(BeJsConst val, double defaultVal = 0.0)
    {
    double coerced;
    return TryCoerceJsonStringToDouble(val, coerced) ? coerced : val.asDouble(defaultVal);
    }

inline uint32_t JsonToUInt(BeJsConst val, uint32_t defaultVal = 0)
    {
    double coerced;
    if (!TryCoerceJsonStringToDouble(val, coerced))
        return val.asUInt(defaultVal);
    // strtod accepts "nan"/"inf"/"-infinity"; converting a non-finite double to an integer type is
    // undefined behavior and the range clamps below would not catch NaN, so reject it up front.
    if (!std::isfinite(coerced))
        return defaultVal;
    // Casting an out-of-range double to uint32_t is undefined behavior, so clamp first.
    if (coerced <= 0.0)
        return 0;
    if (coerced >= (double) UINT32_MAX)
        return UINT32_MAX;
    return (uint32_t) coerced;
    }

inline int64_t JsonToInt64(BeJsConst val, int64_t defaultVal = 0)
    {
    double coerced;
    if (!TryCoerceJsonStringToDouble(val, coerced))
        return val.asInt64(defaultVal);
    // See JsonToUInt: non-finite values must not reach the integer conversion below.
    if (!std::isfinite(coerced))
        return defaultVal;
    if (coerced <= (double) INT64_MIN)
        return INT64_MIN;
    if (coerced >= (double) INT64_MAX)
        return INT64_MAX;
    return (int64_t) coerced;
    }

//! NOTE: there is deliberately no JsonToBool here. BeJsConst::asBool ALREADY matches JsonCpp -
//! both treat any non-empty string as true and any non-zero number as true (BeJsValue.h
//! GetBoolean). Only the numeric accessors lost the string coercion.

END_BENTLEY_FORMATTING_NAMESPACE
