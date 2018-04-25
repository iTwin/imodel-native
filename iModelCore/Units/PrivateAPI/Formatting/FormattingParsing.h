/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateAPI/Formatting/FormattingParsing.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Formatting/FormattingDefinitions.h>
#include <Formatting/FormattingEnum.h>
#include <Units/Units.h>

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
    UNITS_EXPORT CursorScanPoint(Utf8CP input, size_t indx, bool revers);
    UNITS_EXPORT void ProcessNext(Utf8CP input);
    UNITS_EXPORT void ProcessPrevious(Utf8CP input);
    UNITS_EXPORT void Iterate(Utf8CP input, bool revers);
    bool IsAscii() { return m_len == 1; }
    bool IsDigit() { return (m_len == 1) && (m_patt == FormatConstant::NumberSymbol()); }
    bool IsSign() { return (m_len == 1) && (m_patt == FormatConstant::SignSymbol()); }
    bool IsSpace() { return (m_len == 1) && (m_patt == FormatConstant::SpaceSymbol()); }
    bool IsPoint() { return (m_len == 1) && (m_patt == '.'); }
    bool IsBar() { return (m_len == 1) && (m_patt == '/'); }
    bool IsExponent() {return (m_len == 1) && (m_patt == 'x'); }

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
    int m_ival;
    double m_dval;
    ParsingSegmentType m_type;

public:
    NumberGrabber() : NumberGrabber(nullptr) {}
    NumberGrabber(Utf8CP input, size_t start = 0) : m_input(input), m_start(start), m_next(start), m_type(ParsingSegmentType::NotNumber) {}
    UNITS_EXPORT  size_t Grab(Utf8CP input=nullptr, size_t start = 0);
    ParsingSegmentType GetType() const { return m_type; }
    size_t GetStartIndex() const { return m_start; }
    size_t GetNextIndex() const { return m_next; }
    int GetInteger() const { return m_ival; }
    double GetReal() const { return m_dval; }
    bool IsEndOfLine() const { return (m_type == ParsingSegmentType::EndOfLine); }
    size_t GetLength() const { return (m_type == ParsingSegmentType::NotNumber)? 0 : m_next - m_start; }
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct FormatParsingSegment
{
private:
    size_t m_start;         // start index to the input string
    size_t m_byteCount;     // byte length 
    bvector<CursorScanPoint> m_vect;
    Utf8String m_name;
    BEU::UnitCP m_unit;
    ParsingSegmentType m_type;
    int m_ival;
    double m_dval;

    UNITS_EXPORT void Init(size_t start);
public:
    FormatParsingSegment() { Init(0); }
    UNITS_EXPORT FormatParsingSegment(NumberGrabberCR ng);
    UNITS_EXPORT FormatParsingSegment(bvector<CursorScanPoint> vect, size_t s, BEU::UnitCP refUnit = nullptr);
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
    Utf8CP m_input;
    bvector<FormatParsingSegment> m_segs;
    BEU::UnitCP m_unit;     // optional reference to a "quantity" unit
    FormatProblemDetail m_problem;

    void Init(Utf8CP input, size_t start, BEU::UnitCP unit);
    //! Process's "colonized" expression using a Composite FUS
    //! Returns error codes when FUS does not match the expression.
    //! The input expression signature code mus be provided by the caller
    BEU::Quantity ComposeColonizedQuantity(Formatting::FormatSpecialCodes cod, FormatCP fusP = nullptr);

public:
    UNITS_EXPORT FormatParsingSet(Utf8CP input, BEU::UnitCP unit = nullptr);
    bool HasProblem() const {return m_problem.IsProblem();}
    FormatProblemCode GetProblemCode() {return m_problem.GetProblemCode();}
    Utf8String GetProblemDescription() {return m_problem.GetProblemDescription();}
    BEU::UnitCP GetUnit() {return m_unit;}
    UNITS_EXPORT Utf8String GetSignature(bool distinct = true);
    UNITS_EXPORT BEU::Quantity GetQuantity(FormatProblemCode* probCode = nullptr, FormatCP fusP = nullptr);
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
// @bsiclass                                                    David.Fox-Rabinovitz
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
// @bsistruct
//=======================================================================================
struct FormattingSignature
{
private:
    static const int m_maxNumSeg = 8;
    size_t m_size;
    size_t m_sigIndx;
    size_t m_patIndx;
    Utf8P  m_signature;
    Utf8P  m_pattern;
    size_t m_segCount;
    size_t m_segPos[m_maxNumSeg];

public:
    FormattingSignature() : m_size(0), m_signature(nullptr), m_pattern(nullptr),m_segCount(0){ memset(m_segPos, 0, sizeof(m_segPos)); }
    FormattingSignature(size_t reserve) {Reset(reserve);}
    UNITS_EXPORT bool Reset(size_t reserve); 
    ~FormattingSignature() { Reset(0); }
    Utf8CP GetSignature() const { return m_signature; }
    Utf8Char GetPatternChar(size_t indx) { return (indx < m_patIndx) ? m_pattern[indx] : FormatConstant::EndOfLine(); }
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
    size_t m_breakIndex;         // special position  dividing the string into two parts
    size_t m_uniCode;
    FormattingDividers m_dividers;
    bool m_isASCII;          // flag indicating that the last scanned byte is ASCII
    ScannerCursorStatus m_status;
    size_t m_effectiveBytes;
    FormattingSignature m_traits;

public:

    UNITS_EXPORT FormattingScannerCursor(Utf8CP utf8Text, int scanLength, Utf8CP div = nullptr);
    FormattingScannerCursor(FormattingScannerCursorCR other) : m_dividers(other.m_dividers),
        m_text(other.m_text), m_cursorPosition(other.m_cursorPosition), m_lastScannedCount(other.m_lastScannedCount),
        m_uniCode(other.m_uniCode), m_isASCII(other.m_isASCII), m_status(other.m_status)
        {}
    size_t GetTotalLength() { return m_totalScanLength; }
    UNITS_EXPORT void Rewind();
    //! This method attempts to extract the content of the last "enclosure" - that is a group of
    //! characters enclosed into one of brackets: parenthesis, curvy bracket or square brackets
    //! if brackets are not detected - the returned word wil be empty
    //! "vertical line" divider is marked by single boolean argument because the divider and its mate are same
    UNITS_EXPORT Utf8String ExtractLastEnclosure();
    UNITS_EXPORT Utf8String ExtractBeforeEnclosure();
    UNITS_EXPORT Utf8String ExtractSegment(size_t from, size_t to);
};

END_BENTLEY_FORMATTING_NAMESPACE
