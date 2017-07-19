/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Formatting/FormattingParsing.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormattingCursorSection)
DEFINE_POINTER_SUFFIX_TYPEDEFS(CursorScanPoint)
DEFINE_POINTER_SUFFIX_TYPEDEFS(NumericAccumulator)
DEFINE_POINTER_SUFFIX_TYPEDEFS(NumberGrabber)
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatParsingSegment)

struct CursorScanPoint
    {
    private:
        size_t m_uniCode;             // this is a Unicode codePoint of the selected string position
        size_t m_indx;                // index in the input string pointing to the first byte of the sequence
        size_t m_len;                 // the actual length of the sequence (for ASCII = 1)
        unsigned char m_bytes[4];     // the actual sequence of bytes containing a Unicode Symbol un UTF8
        Utf8Char m_patt;              // a "pattern" symbol 
        ScannerCursorStatus m_status; // constructor status

        UNITS_EXPORT void Init();
        void ProcessASCII(unsigned char c);
        //UNITS_EXPORT size_t CursorScanPoint::DetectHeadByte(Utf8CP input);
    public:
        CursorScanPoint() { Init(); }
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
        bool IsUline() { return (m_len == 1) && (m_patt == '_'); }
        bool IsExponent() {return (m_len == 1) && (m_patt == 'x'); }

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
        UNITS_EXPORT size_t ExtractTriplet();

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

struct CursorScanTriplet
    {
    private:
        size_t m_strInd;      // index to the text location
        size_t m_tripInd;     // index to the triplet
        bool m_revers;        // indicator that symbols must be inserted in a reverse order
        size_t m_triple[3];  // Unicode code points of 3 consequtive symbols
        Utf8Char m_patt[3];  // A pattern of the triple

        UNITS_EXPORT void Init(bool rev);

    public:
        UNITS_EXPORT CursorScanTriplet();

        size_t SetTextIndex(size_t ind) { return m_strInd = ind; }
        UNITS_EXPORT void PushSymbol(size_t symb); // returns remaining capacity
        UNITS_EXPORT bool SetReverse(bool rev);   // resetting the direction destroys current state

    };


struct NumberGrabber
    {
private:
    Utf8CP m_input;   // the text to explore
    size_t m_start;   // the starting position in the input string
    size_t m_next;    // the offset to the character following the numeric sequence
    int m_ival;
    double m_dval;
    ParsingSegmentType m_type;

    void Init(Utf8CP input, size_t start)
        {
        m_input = input;
        m_start = start;
        m_next = start;
        m_type = ParsingSegmentType::NotNumber;
        }
public:
    NumberGrabber() { Init(nullptr, 0); }
    NumberGrabber(Utf8CP input, size_t start = 0) { Init(input, start); }
    UNITS_EXPORT  size_t Grab(Utf8CP input=nullptr, size_t start = 0);
    ParsingSegmentType GetType() const { return m_type; }
    size_t GetStartIndex() const { return m_start; }
    size_t GetNextIndex() const { return m_next; }
    UNITS_EXPORT Utf8CP GetTail();
    bool IsComplete() const { return (m_type == ParsingSegmentType::Integer) || (m_type == ParsingSegmentType::Real); }
    bool IsReal() { return (m_type == ParsingSegmentType::Real); }
    int GetInteger() const { return m_ival; }
    double GetReal() const { return m_dval; }
    bool IsEndOfLine() const { return (m_type == ParsingSegmentType::EndOfLine); }
    size_t GetLength() const { return (m_type == ParsingSegmentType::NotNumber)? 0 : m_next - m_start; }
    size_t AdvanceStart(size_t inc) { m_start += inc;  return m_start; }
    };

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
        int GetInteger() { return m_ival; }
        BEU::UnitCP GetUnit() { return m_unit; }
        size_t GetNextindex() { return m_start + m_byteCount; }
        void SetBoundary(size_t s, size_t l) { m_start = s; m_byteCount = l; }
        bool IsNumber() { return (m_type == ParsingSegmentType::Integer) || (m_type == ParsingSegmentType::Real); }
        UNITS_EXPORT Utf8PrintfString ToText(int n);
    };

struct FormatParsingSet
    {
    private:
        Utf8CP m_input;
        size_t m_start;         // start index to the input string
        bvector<FormatParsingSegment> m_segs;
        BEU::UnitCP m_unit;     // optional reference to a "quantity" unit

        UNITS_EXPORT void Init(Utf8CP input, size_t start, BEU::UnitCP unit);

    public:
        UNITS_EXPORT FormatParsingSet(Utf8CP input, size_t start, Utf8CP unitName = nullptr);// : m_input(input), m_start(start) { m_segs.clear(); }
        UNITS_EXPORT FormatParsingSet(Utf8CP input, size_t start, BEU::UnitCP unit = nullptr);// : m_input(input), m_start(start) { m_segs.clear(); }
        void AddSegment(FormatParsingSegmentCR seg) { m_segs.push_back(seg); }
        bvector<FormatParsingSegment> GetSegments() { return m_segs; }
        UNITS_EXPORT Utf8String GetSignature(bool distinct = true);
        UNITS_EXPORT BEU::Quantity GetQuantity();
    };

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

        UNITS_EXPORT AccumulatorState SetSign(int sig);
        UNITS_EXPORT AccumulatorState SetDecimalPoint();
        UNITS_EXPORT AccumulatorState SetExponentMark();

        AccumulatorState SetIntegerState() { return m_stat = AccumulatorState::Integer; }
        AccumulatorState SetfractionState() { return m_stat = AccumulatorState::Fraction; }
        AccumulatorState SetExponentState() { return m_stat = AccumulatorState::Exponent; }

        int IncrementPart(int v, int idx) { m_part[idx] = m_part[idx] * 10 + v; return ++m_count[idx]; }
        int IncrementIntPart(int v) { return IncrementPart(v, 0); }
        int IncrementFractPart(int v) { return IncrementPart(v, 1); }
        int IncrementExponentPart(int v) { return IncrementPart(v, 2); }

        UNITS_EXPORT int AddDigitValue(Utf8Char c);
         
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

    UNITS_EXPORT size_t DetectUOMPattern(size_t ind);
    UNITS_EXPORT size_t DetectFractPattern(size_t ind);
public:
    FormattingSignature():m_size(0), m_signature(nullptr), m_pattern(nullptr),m_segCount(0){ memset(m_segPos, 0, sizeof(m_segPos)); }
    UNITS_EXPORT FormattingSignature(size_t reserve);
        
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
// the cursor section describes a consistent part of the original text string where each character 
//   belongs to the specific semantic group of characters/symbols regardless of their code page
// Each of those groups is represented by an ASCII symbol: 0 - represents any digit, a - represents
//  any letter, s - space, n - number, f -fraction, u -uom. This approach allows to create a compact signature of the given
//    string that could be quickly investigated further by accessing a specific part that is already
//   categorized
// @bsiclass                                                    David.Fox-Rabinovitz
//=======================================================================================
struct FormattingCursorSection
    {
private:
        size_t m_start;      // start index
        size_t m_symbCount;  // logical length of the section
        size_t m_byteCount;  // the actual - byte - length of the section 
        NumericAccumulator m_numAcc;
        CursorSectionType m_type;
        Utf8Char m_symbol;

        CursorSectionState TrySymbol(Utf8Char symb)
            {
            if (m_symbol == FormatConstant::EndOfLine() && m_symbCount == 0 && symb != FormatConstant::EndOfLine())
                {
                m_symbol = symb;
                return CursorSectionState::Success;
                }
            if(m_symbol == symb)
                return CursorSectionState::Success;

            return CursorSectionState::RejectedSymbol;
            }
public:

    FormattingCursorSection(size_t start)
        {
        m_start = start;
        m_symbCount = 0;
        m_byteCount = 0;
        m_symbol = FormatConstant::EndOfLine();
        m_type = CursorSectionType::Undefined;
        }

    Utf8Char GetSymbol() { return m_symbol; }
    size_t GetStart() { return m_start; }
    size_t GetSymbCount() { return m_symbCount; }
    double GetReal() { return m_numAcc.GetReal(); }
    double GetInteger() { return m_numAcc.GetInteger(); }
    bool IsNumber() { return m_symbol == FormatConstant::NumberSymbol(); }
    bool IsSpace() { return m_symbol == FormatConstant::SpaceSymbol(); }
    bool IsWord() { return m_symbol == FormatConstant::WordSymbol(); }
    bool HasProblem() { return m_numAcc.HasProblem(); }

    UNITS_EXPORT CursorSectionState AddSymbol(size_t code, size_t byteLen);

    };

struct CursorSectionSet
    {
    private:
        bvector<FormattingCursorSection> m_set;
        FormattingCursorSectionCP m_current;
        CursorSectionState m_stat;

    public:
        UNITS_EXPORT CursorSectionSet(Utf8CP input, size_t start);
        CursorSectionSet():m_current(nullptr), m_stat(CursorSectionState::Success){}
        void Reset() { m_set.clear(); }
        CursorSectionState GetState() const { return m_stat; }
        size_t GetCount() { return m_set.size(); }
        UNITS_EXPORT CursorSectionState AddSymbol(size_t code, size_t byteLen);
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
    CursorSectionSet m_sections;

    // takes an logical index to an array of ordered bytes representing an integer entity in memory and 
    // returns the physical index of the same array adjusted by endianness. The little endian is default 
    //  and the index will be returned unchaged. This function does not check if supplied 
    UNITS_EXPORT size_t TrueIndex(size_t index, size_t wordSize);
    UNITS_EXPORT int AddTrailingByte();

    bool IsAscii(Utf8Char c) { return (c & FormatConstant::UTF_NonASCIIMark()) == 0; }
    bool IsTrailing(unsigned char c) { return (c & FormatConstant::UTF_TrailingByteMask()) == FormatConstant::UTF_TrailingByteMark(); }
    Utf8Char GetPrecedingByte() { return (m_cursorPosition > 0) ? m_text.c_str()[--m_cursorPosition] : 0; }
public:

    UNITS_EXPORT FormattingScannerCursor(Utf8CP utf8Text, int scanLength, Utf8CP div = nullptr);
    UNITS_EXPORT FormattingScannerCursor(FormattingScannerCursorCR other);

    UNITS_EXPORT size_t HeadBitCount(unsigned char c);
    size_t GetTotalLength() { return m_totalScanLength; }
    size_t GetCurrentPosition() { return m_cursorPosition; }
    size_t GetLastLength() { return m_lastScannedCount; }
    bool CursorInRange() { return m_cursorPosition <= m_totalScanLength; }
    //size_t IncrementCount(size_t delta) { return m_lastScannedCount + delta; }
    UNITS_EXPORT size_t GetNextSymbol();
    UNITS_EXPORT size_t GetNextReversed();
    bool IsError() { return (m_status != ScannerCursorStatus::Success); }
    bool IsSuccess() { return (m_status == ScannerCursorStatus::Success); }
    ScannerCursorStatus GetOperationStatus() { return m_status; }
    bool IsASCII() { return m_isASCII; }
    UNITS_EXPORT int CodePointCount();
    UNITS_EXPORT void Rewind();

    UNITS_EXPORT size_t SkipBlanks();
    UNITS_EXPORT Utf8String SelectKeyWord();
    void SetDividers(CharCP div) { m_dividers = FormattingDividers(div); }
    size_t GetEffectiveBytes() { return m_effectiveBytes; }
    UNITS_EXPORT FormattingWord ExtractWord();
    UNITS_EXPORT FormattingWord ExtractLastEnclosure();
    UNITS_EXPORT FormattingWord ExtractBeforeEnclosure();
    UNITS_EXPORT FormattingWord ExtractSegment(size_t from, size_t to);
    UNITS_EXPORT Utf8CP GetSignature(bool refresh, bool compress);
    UNITS_EXPORT Utf8CP GetReversedSignature(bool refresh, bool compress);
    UNITS_EXPORT size_t SetSections();
    size_t GetSectionCount() { return m_sections.GetCount();}
    UNITS_EXPORT  Utf8CP GetSectionPattern();
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
    FormattingWord() :m_cursor(nullptr), m_isASCII(false) {}
    Utf8String GetWord() { return m_word; }
    Utf8Char GetDelim() { return m_delim[0]; }
    Utf8CP GetText() { return m_word.c_str(); }
    bool IsDelimeterOnly() { return ((0 == m_word.length()) && (0 != m_delim[0])); }
    bool IsEndLine() { return ((0 == m_word.length()) && (0 == m_delim[0])); }
    bool IsSeparator() { return ((0 == m_word.length()) && (',' == m_delim[0] || ' ' == m_delim[0])); }
    };

//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz
//=======================================================================================
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


END_BENTLEY_FORMATTING_NAMESPACE