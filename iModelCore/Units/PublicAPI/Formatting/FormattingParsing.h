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
DEFINE_POINTER_SUFFIX_TYPEDEFS(FormatCursorDetail)


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
    };

//struct FormattingSignatureChunk
//    {
//    private:
//        size_t m_start;
//        size_t m_len;
//        int m_ival;
//        double m_dval;
//        Utf8Char m_symbol;
//    public:
//        FormattingSignatureChunk(FormatCursorDetailCR curd)
//            {
//            m_start = curd.GetPosition();
//
//            }
//
//    };

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
    FormattingSignature m_traits;

    // takes an logical index to an array of ordered bytes representing an integer entity in memory and 
    // returns the physical index of the same array adjusted by endianness. The little endian is default 
    //  and the index will be returned unchaged. This function does not check if supplied 
    size_t TrueIndex(size_t index, size_t wordSize);
    int AddTrailingByte();

public:

    UNITS_EXPORT FormattingScannerCursor(CharCP utf8Text, int scanLength, CharCP div = nullptr);
    UNITS_EXPORT FormattingScannerCursor(FormattingScannerCursorCR other);

    size_t GetTotalLength() { return m_totalScanLength; }
    size_t GetCurrentPosition() { return m_cursorPosition; }
    bool CursorInRange() { return m_cursorPosition <= m_totalScanLength; }
    //size_t IncrementCount(size_t delta) { return m_lastScannedCount + delta; }
    UNITS_EXPORT size_t GetNextSymbol();
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
    Utf8CP GetPattern(bool refresh, bool compress) { GetSignature(refresh, compress); return m_traits.GetPattern(); }
    UNITS_EXPORT Utf8String CollapseSpaces(bool replace);
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