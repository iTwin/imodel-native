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
DEFINE_POINTER_SUFFIX_TYPEDEFS(ScanBuffer)
DEFINE_POINTER_SUFFIX_TYPEDEFS(ScanSegment)
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
// @bsiclass                                                    David.Fox-Rabinovitz
//=======================================================================================
//struct CodepointBuffer
//    {
//private:
//    CodepointSize m_size;    // the size of the codepoint
//    size_t    m_capacity;    // capacity in number of codepoints
//    size_t    m_bufSize;     // the actual buffer size in bytes
//    int       m_lastIndex;   // index of the last inserted symbol
//    union {
//        void*     ptr;     // pointer to the storage
//        Utf8P     bytes;
//        uint16_t* shorts;
//        uint32_t* longs;
//        } m_buff;
//
//    UNITS_EXPORT CodepointSize IntToSize(size_t cps);
// 
//    UNITS_EXPORT size_t SizeToInt(CodepointSize cps);
//         
//    UNITS_EXPORT void Init(CodepointSize cps, size_t capacity);
//
//
//public:
//
//    CodepointBuffer() { Init(CodepointSize::Zero, 0); }
//    CodepointBuffer(size_t codepointSize, size_t capacity) { Init(IntToSize(codepointSize), capacity); }
//    CodepointBuffer(CodepointSize cps, size_t capacity) { Init(cps, capacity); }
//    ~CodepointBuffer() { if (nullptr != m_buff.bytes) delete m_buff.bytes; }
//
//    size_t GetCapacity() { return m_capacity; }
//    size_t GetCodepointSize() { return SizeToInt(m_size); }
//    uint32_t* GetLongBuffer() { return  m_buff.longs; }
//    uint16_t* GetShortBuffer() { return m_buff.shorts; }
//    Utf8P     GetByteBuffer() { return m_buff.bytes; }
//    UNITS_EXPORT void Reset(CodepointSize cps, size_t capacity);
//    UNITS_EXPORT void Release();
//    UNITS_EXPORT  Utf8P     AppendByte(Utf8Char c);
//    UNITS_EXPORT  uint32_t* AppendSymbol(uint32_t symb);
//    UNITS_EXPORT  uint16_t* AppendSymbol(uint16_t symb);
//    };

//= == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == ==
// @bsiclass
//=======================================================================================
struct FormatCursorDetail
    {
private:
    size_t m_totalLength; // this is the total length of the byte sequence that ought to be scanned/parsed
    size_t m_position;   // the index of the next byte to be scanned
    size_t m_scanCount;   // the number of bytes processed in the last step
    size_t m_uniCode;

    void Init() { m_totalLength = 0; m_position = 0; m_scanCount = 0; m_uniCode = 0; }
public:
    FormatCursorDetail(): m_totalLength(0), m_position(0), m_scanCount(0), m_uniCode(0){}
    FormatCursorDetail(FormatCursorDetailCR src):m_totalLength(src.m_totalLength), m_position(src.m_position),
                       m_scanCount(src.m_scanCount), m_uniCode(src.m_uniCode) { }
    FormatCursorDetail(size_t totLen, size_t posit, size_t count, size_t code):m_totalLength(totLen), 
                       m_position(posit), m_scanCount(count), m_uniCode(code) {}
        
    size_t GetPosition() { return m_position; }
    size_t SetPosition(size_t position) { return m_position = position; }
    size_t GetTotalLength() { return m_totalLength; }
    size_t SetTotalLength(size_t len) { return m_totalLength = len; }
    size_t GetScanCount() { return m_scanCount; }
    size_t SetScanCount(size_t count) { return m_scanCount = count; }
    size_t IncrementCount(size_t delta) { m_scanCount += delta; return m_scanCount; }
    size_t GetUnicode() { return m_uniCode; }
    char   GetASCII() { return (char)(m_uniCode & 0x7F); }
    size_t SetUnicode(size_t code) { return m_uniCode = code; }
    bool CursorInRange() { return m_position >= m_totalLength; }
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
    UNITS_EXPORT FormatDividerInstance():m_div('\0'), m_mate('\0'), m_divCount(0), m_mateCount(0), m_totLen(0){}
    int GetDivCount() { return m_divCount; }
    int GetMateCount() { return m_mateCount; }
    bool BracketsMatched() { return (0 < m_divCount && m_divCount == m_mateCount); }
    bool IsDivLast() { return (0 < m_divCount && m_totLen == -(m_positions.back())); }
    int  GetFirstLocation() { return (m_positions.size() > 0) ? m_positions.front()-1 : -1; }
    UNITS_EXPORT Utf8String ToText();
    };

//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz
//=======================================================================================
struct FormattingScannerCursor
    {
private:
    Utf8String m_text;           // pointer to the head of the string
    FormatCursorDetail m_detail;
    size_t m_totalScanLength;    // this is the total length of the byte sequence that ought to be scanned/parsed
    size_t m_cursorPosition;     // the index of the next byte to be scanned
    size_t m_lastScannedCount;   // the number of bytes processed in the last step
    size_t m_breakIndex;         // special position  dividing the string into two parts
    size_t m_uniCode;
    FormattingDividers m_dividers;
    //union { uint8_t octet[4];  unsigned int word; } m_code; // container for the scanned bytes
    bool m_isASCII;          // flag indicating that the last scanned byte is ASCII
    //UnicodeConstantP m_unicodeConst; // reference to constants and character processors
    ScannerCursorStatus m_status;
    size_t m_effectiveBytes;
    char m_temp;
    //CodepointBuffer m_unicodeBuff;
    Utf8P  m_signature;

    // takes an logical index to an array of ordered bytes representing an integer entity in memory and 
    // returns the physical index of the same array adjusted by endianness. The little endian is default 
    //  and the index will be returned unchaged. This function does not check if supplied 
    size_t TrueIndex(size_t index, size_t wordSize);
    int AddTrailingByte();
    size_t SetCurrentPosition(size_t position) { return m_detail.SetPosition(position); }
    //UNITS_EXPORT int ProcessTrailingByte(char c, int* bits);
    void ReleaseSignature() { if (nullptr != m_signature) delete m_signature; m_signature = nullptr; }

public:
    //! Construct a cursor attached to the given Utf8 string 
    // FormattingScannerCursor();
    UNITS_EXPORT FormattingScannerCursor(CharCP utf8Text, int scanLength, CharCP div = nullptr);
    UNITS_EXPORT FormattingScannerCursor(FormattingScannerCursorCR other);
    ~FormattingScannerCursor() { ReleaseSignature(); }

    size_t GetTotalLength() { return m_totalScanLength; }
    //UnicodeConstant* GetConstants() { return m_unicodeConst; }
   // void ResetScanCount() { m_detail.SetScanCount(0); }
   // void ResetUnicode() { m_detail.SetUnicode(0); }
    //void SetUnicode(size_t code) { m_detail.SetUnicode(code); }
    size_t GetCurrentPosition() { return m_detail.GetPosition(); }
    bool CursorInRange() { return m_cursorPosition >= m_totalScanLength; }
    //size_t IncrementCount(size_t delta) { return m_lastScannedCount + delta; }
    UNITS_EXPORT size_t GetNextSymbol();
    UNITS_EXPORT size_t GetNextCodePoint();
    bool IsError() { return (m_status != ScannerCursorStatus::Success); }
    bool IsSuccess() { return (m_status == ScannerCursorStatus::Success); }
    ScannerCursorStatus GetOperationStatus() { return m_status; }
    bool IsEndOfLine() { return (m_text[m_detail.GetPosition()] == '\0'); }
    bool IsASCII() { return m_isASCII; }
    UNITS_EXPORT int CodePointCount();
    UNITS_EXPORT void Rewind(bool freeBuf);
    size_t GetUnicode() { return m_detail.GetUnicode(); }
    size_t GetLastScanned() { return m_detail.GetScanCount(); }
    UNITS_EXPORT size_t SkipBlanks();
    UNITS_EXPORT Utf8String SelectKeyWord();
    void SetDividers(CharCP div) { m_dividers = FormattingDividers(div); }
    bool IsDivider() { return m_isASCII ? m_dividers.IsDivider(m_detail.GetASCII()) : false; }
    size_t GetEffectiveBytes() { return m_effectiveBytes; }
    UNITS_EXPORT FormattingWord ExtractWord();
    UNITS_EXPORT FormattingWord ExtractLastEnclosure();
    UNITS_EXPORT FormattingWord ExtractBeforeEnclosure();
    UNITS_EXPORT FormattingWord ExtractSegment(size_t from, size_t to);
    //UNITS_EXPORT uint32_t* GetLongUcode() { return m_unicodeBuff.GetLongBuffer(); }
    //UNITS_EXPORT uint16_t* GetShortUcode() { return m_unicodeBuff.GetShortBuffer(); }
    UNITS_EXPORT Utf8CP GetSignature();
    UNITS_EXPORT Utf8String CollapseSpaces();
    UNITS_EXPORT int DetectEnclosures(Utf8Char bracket);
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

//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz
//=======================================================================================
struct ScanBuffer       // this class can be used only on stack and cannot be passed outside of the scope of the function
    {
private:
    Utf8P  m_buffer;
    size_t m_capacity;
    size_t m_actLen;
public:
    ScanBuffer(Utf8P buff, size_t size)
        {   
            m_buffer = buff;
            memset(m_buffer, 0, size + 2);
            m_capacity = size;   
        }
    //UNITS_EXPORT size_t LoadTextSegment(Utf8CP text, int index, int len);
    Utf8P GetBuffer() { return m_buffer; }
    size_t GetCapacity() {return m_capacity; }
    size_t SetActualLength(size_t len) { return m_actLen = len; }
    size_t GetActualLength() { return m_actLen; }
    UNITS_EXPORT size_t ExtractSegment(Utf8CP text, ScanSegmentR seg);
    };

//=======================================================================================
// @bsiclass                                                    David.Fox-Rabinovitz
//=======================================================================================
struct ScanSegment
    {
private:
    ScanSegmentType m_type;
    int m_indx;
    int m_len;
public: 
    ScanSegment() { m_type = ScanSegmentType::Undefined;  m_indx = -1; m_len = -1; }
    ScanSegment(ScanSegmentType type, int indx=-1, int len=-1) { m_type = type;  m_indx = indx; m_len = len; }
    bool IsPresent() { return (0 <= m_indx && 0 < m_len); }  // a true segment must have index >= 0 and some length > 0
    int GetIndex() { return m_indx; }
    int SetIndex(int indx) { return m_indx = indx; }
    int GetLength() { return m_len; }
    int SetLength(int len) { return m_len = len; }
    int IncrementLength(int delta) { m_len += delta;  return m_len; }
    UNITS_EXPORT Utf8String ExtractSegment(Utf8CP text);
    UNITS_EXPORT static Utf8String TypeToName(ScanSegmentType type);
    UNITS_EXPORT static ScanSegmentType IndexToType(int indx);
    UNITS_EXPORT Utf8String SegmentInfo(Utf8CP prefix);
    };

struct NumeriChunk
    {
private:
    ScanSegment m_parts[(int)ScanSegmentType::Undefined];
    double m_dval;
    size_t m_ival;
    void Init();
public:
    UNITS_EXPORT NumeriChunk() { Init();  m_dval = 0.0; m_ival = 0; }
    UNITS_EXPORT NumeriChunk(Utf8CP text);
    UNITS_EXPORT Utf8String ChunkInfo(Utf8CP mess);
    };


END_BENTLEY_FORMATTING_NAMESPACE