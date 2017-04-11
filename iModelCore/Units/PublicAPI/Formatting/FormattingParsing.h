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
struct CodepointBuffer
    {
    private:
        CodepointSize m_size;    // the size of the codepoint
        size_t    m_capacity;    // capacity in number of codepoints
        size_t    m_bufSize;     // the actual buffer size in bytes
        int       m_lastIndex;   // index of the last inserted symbol
        union {
            void*     ptr;     // pointer to the storage
            Utf8P     bytes;
            uint16_t* shorts;
            uint32_t* longs;
            } m_buff;

        CodepointSize IntToSize(size_t cps)
            {
            if (cps == 4)
                return CodepointSize::Quatro;
            if (cps == 2)
                return CodepointSize::Double;
            if (cps == 1)
                return CodepointSize::Single;
            return CodepointSize::Zero;
            }

        size_t SizeToInt(CodepointSize cps)
            {
            switch (cps)
                {
                case CodepointSize::Quatro: return 4;
                case CodepointSize::Double: return 2;
                case CodepointSize::Single: return 1;
                case CodepointSize::Zero:
                default: return 0;
                }
            }
        void Init(CodepointSize cps, size_t capacity)
            {
            m_size = cps;
            m_lastIndex = -1;
            if (CodepointSize::Zero == m_size)
                {
                m_capacity = 0;
                m_bufSize = 0;
                m_buff.ptr = nullptr;
                }
            else
                {
                m_capacity = capacity;
                m_bufSize = (m_capacity + 1)* SizeToInt(m_size);  // extra bytes for terminating zero
                m_buff.ptr = new char[m_bufSize];
                }
            }

    public:

        CodepointBuffer() { Init(CodepointSize::Zero, 0); }
        CodepointBuffer(size_t codepointSize, size_t capacity) { Init(IntToSize(codepointSize), capacity); }
        CodepointBuffer(CodepointSize cps, size_t capacity) { Init(cps, capacity); }
        ~CodepointBuffer() { if (nullptr != m_buff.bytes) delete m_buff.bytes; }

        size_t GetCapacity() { return m_capacity; }
        size_t GetCodepointSize() { return SizeToInt(m_size); }
        uint32_t* GetLongBuffer() { return  m_buff.longs; }
        uint16_t* GetShortBuffer() { return m_buff.shorts; }
        Utf8P     GetByteBuffer() { return m_buff.bytes; }
        UNITS_EXPORT void Reset(CodepointSize cps, size_t capacity);
        UNITS_EXPORT void Release();
        UNITS_EXPORT  Utf8P     AppendByte(Utf8Char c);
        UNITS_EXPORT  uint32_t* AppendSymbol(uint32_t symb);
        UNITS_EXPORT  uint16_t* AppendSymbol(uint16_t symb);
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
        size_t m_uniCode;
        FormattingDividers m_dividers;
        //union { uint8_t octet[4];  unsigned int word; } m_code; // container for the scanned bytes
        bool m_isASCII;          // flag indicating that the last scanned byte is ASCII
        //UnicodeConstantP m_unicodeConst; // reference to constants and character processors
        ScannerCursorStatus m_status;
        size_t m_effectiveBytes;
        char m_temp;
        CodepointBuffer m_unicodeBuff;
        Utf8P     m_signature;

        // takes an logical index to an array of ordered bytes representing an integer entity in memory and 
        // returns the physical index of the same array adjusted by endianness. The little endian is default 
        //  and the index will be returned unchaged. This function does not check if supplied 
        size_t TrueIndex(size_t index, size_t wordSize);
        int AddTrailingByte();
        size_t SetCurrentPosition(size_t position) { return m_cursorPosition = position; }
        //UNITS_EXPORT int ProcessTrailingByte(char c, int* bits);
        void ReleaseSignature() { if (nullptr != m_signature) delete m_signature; m_signature = nullptr; }

    public:
        //! Construct a cursor attached to the given Utf8 string 
        // FormattingScannerCursor();
        UNITS_EXPORT FormattingScannerCursor(CharCP utf8Text, int scanLength, CharCP div = nullptr);
        UNITS_EXPORT FormattingScannerCursor(FormattingScannerCursorCR other);
        ~FormattingScannerCursor() { ReleaseSignature(); }

        //UnicodeConstant* GetConstants() { return m_unicodeConst; }
        size_t GetCurrentPosition() { return m_cursorPosition; }
        UNITS_EXPORT size_t GetNextSymbol();
        UNITS_EXPORT size_t GetNextCodePoint();
        bool IsError() { return (m_status != ScannerCursorStatus::Success); }
        bool IsSuccess() { return (m_status == ScannerCursorStatus::Success); }
        ScannerCursorStatus GetOperationStatus() { return m_status; }
        bool IsEndOfLine() { return (m_text[m_cursorPosition] == '\0'); }
        bool IsASCII() { return m_isASCII; }
        UNITS_EXPORT int CodePointCount();
        UNITS_EXPORT void Rewind(bool freeBuf);
        size_t GetUnicode() { return m_uniCode; }
        size_t GetLastScanned() { return m_lastScannedCount; }
        UNITS_EXPORT size_t SkipBlanks();
        UNITS_EXPORT Utf8String SelectKeyWord();
        void SetDividers(CharCP div) { m_dividers = FormattingDividers(div); }
        bool IsDivider() { return m_isASCII ? m_dividers.IsDivider((char)(m_uniCode & 0x7F)) : false; }
        size_t GetEffectiveBytes() { return m_effectiveBytes; }
        UNITS_EXPORT FormattingWord ExtractWord();
        UNITS_EXPORT uint32_t* GetLongUcode() { return m_unicodeBuff.GetLongBuffer(); }
        UNITS_EXPORT uint16_t* GetShortUcode() { return m_unicodeBuff.GetShortBuffer(); }
        UNITS_EXPORT Utf8CP GetSignature();
    };

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
        Utf8String GetWord() { return m_word; }
        Utf8Char GetDelim() { return m_delim[0]; }
        Utf8CP GetText() { return m_word.c_str(); }
        bool IsDelimeterOnly() { return ((0 == m_word.length()) && (0 != m_delim[0])); }
        bool IsEndLine() { return ((0 == m_word.length()) && (0 == m_delim[0])); }
        bool IsSeparator() { return ((0 == m_word.length()) && (',' == m_delim[0] || ' ' == m_delim[0])); }
    };

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