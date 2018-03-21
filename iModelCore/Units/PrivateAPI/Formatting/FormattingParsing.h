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
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormattingWord)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormattingScannerCursor)
DEFINE_POINTER_SUFFIX_TYPEDEFS(CursorScanPoint)
DEFINE_POINTER_SUFFIX_TYPEDEFS(NumericAccumulator)
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
    bool IsColon() { return (m_len == 1) && (m_bytes[0] == ':'); }
    bool IsDoubleColon() { return (m_len == 2) && (m_bytes[0] == ':') && (m_bytes[1] == ':'); }
    bool IsTripleColon() { return (m_len == 3) && (m_bytes[0] == ':') && (m_bytes[1] == ':') && (m_bytes[2] == ':'); }
    bool IsUline() { return (m_len == 1) && (m_patt == '_'); }
    bool IsExponent() {return (m_len == 1) && (m_patt == 'x'); }

    //! The caller is responsible for keeping the index inside the allowable range
    UNITS_EXPORT ScannerCursorStatus AppendTrailingByte(Utf8CP txt);
    Utf8Char GetPrecedingByte(Utf8CP txt) { return (m_indx > 0) ? txt[--m_indx] : FormatConstant::EndOfLine(); }

    size_t GetIndex() const { return m_indx; }
    size_t GetCode() const { return m_uniCode; }
    size_t GetLength() const { return m_len; }
    Utf8Char GetAscii(){ return FormatConstant::ASCIIcode(m_uniCode); }
    Utf8Char GetPattern() { return m_patt; }
    unsigned char* GetBytes() { return m_bytes; }
    bool IsEndOfLine() { return m_len == 0; }
    UNITS_EXPORT Utf8PrintfString ToText();
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct FormatParseVector
{
private:
    bvector<CursorScanPoint> m_vect;
    size_t m_tripletIndx;
    size_t m_bufIndex;
    union {
        size_t word;
        Utf8Char byte[4];
        } m_patt;
    size_t ExtractTriplet();

public:
    UNITS_EXPORT FormatParseVector(Utf8CP input, bool revers);
    UNITS_EXPORT size_t AddPoint(CursorScanPointCR pnt);
    bvector<CursorScanPoint> GetArray() { return m_vect; }
    UNITS_EXPORT Utf8String GetPattern();
    UNITS_EXPORT Utf8String GetSignature();
    UNITS_EXPORT bool MoveFrame(bool forw = true);
    Utf8String GetTriplet() { return Utf8String(m_patt.byte); }
    size_t GetTripletIndex() { return m_tripletIndx; }
    Utf8Char GetFrameChar() { return m_patt.byte[0]; }
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct CursorScanTriplet
{
private:
    size_t m_strInd;      // index to the text location
    size_t m_tripInd;     // index to the triplet
    bool m_revers;        // indicator that symbols must be inserted in a reverse order
    size_t m_triple[3];  // Unicode code points of 3 consequtive symbols
    Utf8Char m_patt[3];  // A pattern of the triple

    void Init(bool rev);

public:
    UNITS_EXPORT CursorScanTriplet();

    size_t SetTextIndex(size_t ind) { return m_strInd = ind; }

    //! This method creates a "sliding" cursor that exposes three consequtive symbols from
    //! the input string the "reading head" is always at the current position in the string
    UNITS_EXPORT void PushSymbol(size_t symb); // returns remaining capacity
    UNITS_EXPORT bool SetReverse(bool rev);   // resetting the direction destroys current state
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
    Utf8CP GetTail() {return &m_input[m_next];}
    bool IsComplete() const { return (m_type == ParsingSegmentType::Integer) || (m_type == ParsingSegmentType::Real); }
    bool IsReal() { return (m_type == ParsingSegmentType::Real); }
    int GetInteger() const { return m_ival; }
    double GetReal() const { return m_dval; }
    bool IsEndOfLine() const { return (m_type == ParsingSegmentType::EndOfLine); }
    size_t GetLength() const { return (m_type == ParsingSegmentType::NotNumber)? 0 : m_next - m_start; }
    size_t AdvanceStart(size_t inc) { m_start += inc;  return m_start; }
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
    size_t SetStart(size_t s) { return m_start = s; }
    size_t SetLength(size_t s) { return m_byteCount = s; }
    ParsingSegmentType GetType() { return m_type; }
    double GetReal() { return m_dval; }
    double GetAbsReal() { return fabs(m_dval); }
    double GetSign() { return (m_dval < 0) ? -1.0 : 1.0; }
    int GetInteger() { return m_ival; }
    BEU::UnitCP GetUnit(Formatting::FormatUnitSetCP fusP = nullptr) {return (nullptr == fusP) ? m_unit : fusP->GetUnit();}
    size_t GetNextindex() { return m_start + m_byteCount; }
    void SetBoundary(size_t s, size_t l) { m_start = s; m_byteCount = l; }
    bool IsNumber() { return (m_type == ParsingSegmentType::Integer) || (m_type == ParsingSegmentType::Real); }
    bool IsColon() { return (m_type == ParsingSegmentType::NotNumber) && BeStringUtilities::StricmpAscii(m_name.c_str(), ":") == 0; }
    bool IsDoubleColon() { return (m_type == ParsingSegmentType::NotNumber) && BeStringUtilities::StricmpAscii(m_name.c_str(), "::") == 0;}
    bool IsTripleColon() { return (m_type == ParsingSegmentType::NotNumber) && BeStringUtilities::StricmpAscii(m_name.c_str(), ":::") == 0; }
        
    bool IsMinusColon() { return (m_type == ParsingSegmentType::NotNumber) && BeStringUtilities::StricmpAscii(m_name.c_str(), "-:") == 0; }
    bool IsMinusDoubleColon() { return (m_type == ParsingSegmentType::NotNumber) && BeStringUtilities::StricmpAscii(m_name.c_str(), "-::") == 0; }
    bool IsMinusTripleColon() { return (m_type == ParsingSegmentType::NotNumber) && BeStringUtilities::StricmpAscii(m_name.c_str(), "-:::") == 0; }
    UNITS_EXPORT Utf8PrintfString ToText(int n);
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
    BEU::Quantity ComposeColonizedQuantity(Formatting::FormatSpecialCodes cod, FormatUnitSetCP fusP = nullptr);

public:
    UNITS_EXPORT FormatParsingSet(Utf8CP input, BEU::UnitCP unit = nullptr);

    void AddSegment(FormatParsingSegmentCR seg) {m_segs.push_back(seg);}

    bool HasProblem() const {return m_problem.IsProblem();}
    FormatProblemCode GetProblemCode() {return m_problem.GetProblemCode();}
    Utf8String GetProblemDescription() {return m_problem.GetProblemDescription();}

    bvector<FormatParsingSegment> GetSegments() {return m_segs;}

    BEU::UnitCP GetUnit(FormatUnitSetCP fusP = nullptr) {return (nullptr == fusP) ? m_unit : fusP->GetUnit();}

    size_t GetSegmentNumber() { return m_segs.size(); }

    UNITS_EXPORT Utf8String GetSignature(bool distinct = true);

    UNITS_EXPORT BEU::Quantity GetQuantity(FormatProblemCode* probCode = nullptr, FormatUnitSetCP fusP = nullptr);

    UNITS_EXPORT bool ValidateParsingFUS(int reqUnitCount, FormatUnitSetCP fusP);
};

//=======================================================================================
// @bsistruct
//=======================================================================================
struct NumericAccumulator
{
private:
    int m_sign;
    int m_part[3];    // integer, fraction, exponent
    int m_count[3];   // counters for the same parts
    bool m_point;
    bool m_expMark;
    int m_expSign;
    int m_ival;
    double m_dval;
    bool m_real;
    size_t m_bytes;
    AccumulatorState m_stat;
    FormatProblemDetail m_problem;

    void Init()
        {
        m_sign = 0;
        memset(m_part, 0, sizeof(m_part));
        memset(m_count, 0, sizeof(m_count));
        m_point = false;
        m_expMark = false;
        m_expSign = 0;
        m_ival = 0;
        m_dval = 0.0;
        m_real = false;
        m_bytes = 0;
        m_stat = AccumulatorState::Init;
        }

    AccumulatorState SetSign(int sig);
    AccumulatorState SetDecimalPoint();
    AccumulatorState SetExponentMark();

    AccumulatorState SetIntegerState() {return m_stat = AccumulatorState::Integer;}
    AccumulatorState SetfractionState() {return m_stat = AccumulatorState::Fraction;}
    AccumulatorState SetExponentState() {return m_stat = AccumulatorState::Exponent;}

    int IncrementPart(int v, int idx) {m_part[idx] = m_part[idx] * 10 + v; return ++m_count[idx];}
    int IncrementIntPart(int v) {return IncrementPart(v, 0);}
    int IncrementFractPart(int v) {return IncrementPart(v, 1);}
    int IncrementExponentPart(int v) {return IncrementPart(v, 2);}

    int AddDigitValue(Utf8Char c);

public:
    NumericAccumulator() { Init(); }
    int GetInteger() { return m_ival; }
    double GetReal() { return m_dval; }
    UNITS_EXPORT AccumulatorState AddSymbol(size_t symb);
    bool HasProblem() { return m_problem.IsProblem(); }
    bool HasFailed() { return AccumulatorState::Failure == m_stat; }
    bool IsComplete() { return AccumulatorState::Complete == m_stat; }
    bool HasRejected() { return AccumulatorState::RejectedSymbol == m_stat; }
    bool IsNumeric() { return (AccumulatorState::Init == m_stat || AccumulatorState::Integer == m_stat || 
                                AccumulatorState::Fraction == m_stat || AccumulatorState::Exponent == m_stat); }
    size_t GetByteCount() { return m_bytes; }
    FormatProblemCode const GetProblemCode() { return m_problem.GetProblemCode(); }
    Utf8String GetProblemDescription() const { return m_problem.GetProblemDescription(); }
    UNITS_EXPORT  AccumulatorState SetComplete();
    UNITS_EXPORT Utf8String ToText();
    bool CanTakeNext(Utf8CP txt) { return (!HasRejected() && '\0' != *txt); }
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
    CharCP GetMarkers() const { return m_markers; }
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
    int GetMateCount() { return m_mateCount; }
    bool BracketsMatched() { return (0 < m_divCount && m_divCount == m_mateCount); }
    UNITS_EXPORT bool IsDivLast();
    int  GetFirstLocation() { return (m_positions.size() > 0) ? m_positions.front()-1 : -1; }
    UNITS_EXPORT Utf8String ToText();
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

    size_t DetectUOMPattern(size_t ind);
    size_t DetectFractPattern(size_t ind);
public:
    FormattingSignature() : m_size(0), m_signature(nullptr), m_pattern(nullptr),m_segCount(0){ memset(m_segPos, 0, sizeof(m_segPos)); }
    FormattingSignature(size_t reserve) {Reset(reserve);}
        
    UNITS_EXPORT bool Reset(size_t reserve); 
    ~FormattingSignature() { Reset(0); }

    Utf8CP GetSignature() const { return m_signature; }
    Utf8CP GetPattern() const { return m_pattern; }
    size_t GetNumSegCount() { return m_segCount; }
    UNITS_EXPORT size_t AppendSignature(Utf8Char c);
    UNITS_EXPORT size_t AppendPattern();
    UNITS_EXPORT size_t AppendPattern(Utf8Char c);
    Utf8Char GetSignatureChar(size_t indx) { return (indx < m_sigIndx) ? m_signature[indx] : FormatConstant::EndOfLine(); }
    Utf8Char GetPatternChar(size_t indx) { return (indx < m_patIndx) ? m_pattern[indx] : FormatConstant::EndOfLine(); }
    UNITS_EXPORT size_t CompressPattern();
    UNITS_EXPORT  Utf8CP ReverseString(Utf8CP str, Utf8P revStr, size_t bufSize);
    UNITS_EXPORT  Utf8String ReversedSignature();
    UNITS_EXPORT  Utf8String ReversedPattern();
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
    char m_temp;
    CursorScanTriplet m_triplet;
    FormattingSignature m_traits;

    // takes an logical index to an array of ordered bytes representing an integer entity in memory and 
    // returns the physical index of the same array adjusted by endianness. The little endian is default 
    //  and the index will be returned unchaged. This function does not check if supplied 
    size_t TrueIndex(size_t index, size_t wordSize);
    int AddTrailingByte();

    bool IsAscii(Utf8Char c) { return (c & FormatConstant::UTF_NonASCIIMark()) == 0; }
    bool IsTrailing(unsigned char c) { return (c & FormatConstant::UTF_TrailingByteMask()) == FormatConstant::UTF_TrailingByteMark(); }
    Utf8Char GetPrecedingByte() { return (m_cursorPosition > 0) ? m_text.c_str()[--m_cursorPosition] : 0; }
public:

    UNITS_EXPORT FormattingScannerCursor(Utf8CP utf8Text, int scanLength, Utf8CP div = nullptr);
    FormattingScannerCursor(FormattingScannerCursorCR other) : m_dividers(other.m_dividers),
        m_text(other.m_text), m_cursorPosition(other.m_cursorPosition), m_lastScannedCount(other.m_lastScannedCount),
        m_uniCode(other.m_uniCode), m_isASCII(other.m_isASCII), m_status(other.m_status)
        {}

    UNITS_EXPORT size_t HeadBitCount(unsigned char c);
    size_t GetTotalLength() { return m_totalScanLength; }
    size_t GetCurrentPosition() { return m_cursorPosition; }
    size_t GetLastLength() { return m_lastScannedCount; }
    bool CursorInRange() { return m_cursorPosition <= m_totalScanLength; }
    UNITS_EXPORT size_t GetNextSymbol();
    UNITS_EXPORT size_t GetNextReversed();
    bool IsError() { return (m_status != ScannerCursorStatus::Success); }
    bool IsSuccess() { return (m_status == ScannerCursorStatus::Success); }
    ScannerCursorStatus GetOperationStatus() { return m_status; }
    bool IsASCII() { return m_isASCII; }
    UNITS_EXPORT void Rewind();

    UNITS_EXPORT size_t SkipBlanks();
    void SetDividers(CharCP div) { m_dividers = FormattingDividers(div); }
    size_t GetEffectiveBytes() { return m_effectiveBytes; }
    UNITS_EXPORT FormattingWord ExtractWord();

    //! This method attempts to extract the content of the last "enclosure" - that is a group of
    //! characters enclosed into one of brackets: parenthesis, curvy bracket or square brackets
    //! if brackets are not detected - the returned word wil be empty
    //! "vertical line" divider is marked by single boolean argument because the divider and its mate are same
    UNITS_EXPORT FormattingWord ExtractLastEnclosure();
    UNITS_EXPORT FormattingWord ExtractBeforeEnclosure();
    UNITS_EXPORT FormattingWord ExtractSegment(size_t from, size_t to);
    UNITS_EXPORT Utf8CP GetSignature(bool refresh, bool compress);
    UNITS_EXPORT Utf8CP GetReversedSignature(bool refresh, bool compress);
    Utf8CP GetPattern(bool refresh, bool compress) { GetSignature(refresh, compress); return m_traits.GetPattern(); }
    UNITS_EXPORT Utf8String CollapseSpaces(bool replace);
    Utf8String ReversedSignature() { return m_traits.ReversedSignature(); }
    Utf8String ReversedPattern() { return m_traits.ReversedPattern(); }
};

//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz
//=======================================================================================
struct FormattingWord
{
private:
    static const int maxDelim = 4;   // the maximum number of ASCII characters in the delimiting group/clause
    FormattingScannerCursorP m_cursor;  // just a reference to the cursor that has been used
    Utf8String m_word;
    Utf8Char m_delim[maxDelim + 2];
    bool m_isASCII;
public:
    UNITS_EXPORT FormattingWord(FormattingScannerCursorP cursor, Utf8CP buffer, Utf8CP delim, bool isAscii);
    FormattingWord() : m_cursor(nullptr), m_isASCII(false) {}
    Utf8String GetWord() { return m_word; }
    Utf8Char GetDelim() { return m_delim[0]; }
    Utf8CP GetText() { return m_word.c_str(); }
    bool IsDelimeterOnly() { return ((0 == m_word.length()) && (0 != m_delim[0])); }
    bool IsEndLine() { return ((0 == m_word.length()) && (0 == m_delim[0])); }
    bool IsSeparator() { return ((0 == m_word.length()) && (',' == m_delim[0] || ' ' == m_delim[0])); }
};

END_BENTLEY_FORMATTING_NAMESPACE
