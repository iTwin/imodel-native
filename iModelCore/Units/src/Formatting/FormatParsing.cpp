/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/FormatParsing.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//===================================================
//
// FormattingScannerCursorMethods
//
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
void FormattingScannerCursor::Rewind(bool freeBuf)
    {
    m_cursorPosition = 0;
    m_lastScannedCount = 0;
    m_uniCode = 0;
    m_isASCII = false;
    m_status = ScannerCursorStatus::Success;
    m_effectiveBytes = 0;
    m_dividers = FormattingDividers(nullptr);
    if (freeBuf)
        m_unicodeBuff.Release();
    return;
    }

//FormattingScannerCursor::FormattingScannerCursor()
//    {
//    Rewind();
//    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
size_t FormattingScannerCursor::TrueIndex(size_t index, size_t wordSize)
    {
    const bool end = FormatConstant::IsLittleEndian();
    if (end || wordSize <= 1)
        return index;
    int i = (int)(--wordSize) - (int)index;
    return i; // (int)(--wordSize) - (int)index;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
FormattingScannerCursor::FormattingScannerCursor(CharCP utf8Text, int scanLength, CharCP div) :m_dividers(div)
    {
    m_text = utf8Text;
    Rewind(false);
    //m_unicodeConst = new UnicodeConstant();
    m_totalScanLength = (nullptr == utf8Text) ? 0 : strlen(utf8Text);
    if (scanLength > 0 && scanLength <= (int)m_totalScanLength)
        m_totalScanLength = scanLength;
    m_dividers = FormattingDividers(div);
    m_signature = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
FormattingScannerCursor::FormattingScannerCursor(FormattingScannerCursorCR other) :m_dividers(other.m_dividers)
    {
    m_text = other.m_text;
    m_cursorPosition = other.m_cursorPosition;
    m_lastScannedCount = other.m_lastScannedCount;
    m_uniCode = other.m_uniCode;
    m_isASCII = other.m_isASCII;
    m_status = other.m_status;
    m_dividers = FormattingDividers(other.m_dividers);
    m_signature = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
int FormattingScannerCursor::AddTrailingByte()
    {
    if (ScannerCursorStatus::Success == m_status)
        {
        char bits = 0;
        if (FormatConstant::GetTrailingBits(m_text.c_str()[++m_cursorPosition], &bits))
            {
            m_uniCode <<= FormatConstant::GetTrailingShift();
            m_uniCode |= bits;
            return 1;
            }
        else
            m_status = ScannerCursorStatus::IncompleteSequence;
        }
    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 03/17
//---------------------------------------------------------------------------------------
FormattingWord FormattingScannerCursor::ExtractWord()
    {
    static const size_t maxDelim = 4;
    Utf8P buff = (Utf8P)alloca(m_totalScanLength + 1);
    buff[0] = 0;
    m_status = ScannerCursorStatus::Success;
    m_lastScannedCount = 0;
    m_isASCII = true;
    if (m_cursorPosition > m_totalScanLength)
        return FormattingWord(this, buff, buff, true);

    Utf8CP c = &m_text.c_str()[m_cursorPosition];

    while (!m_dividers.IsDivider(*c))
        {
        buff[m_lastScannedCount++] = *c;
        buff[m_lastScannedCount] = 0;
        if (0 != (*c & 0x80))
            m_isASCII = false;
        c++;
        }

    Utf8P delim = (Utf8P)alloca(maxDelim);
    memset(delim, 0, maxDelim);
    delim[0] = *c;
    m_cursorPosition += ++m_lastScannedCount;
    FormattingWord word = FormattingWord(this, buff, delim, m_isASCII);
    return word;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
size_t FormattingScannerCursor::GetNextSymbol()
    {
    unsigned int temp = 0;
    m_status = ScannerCursorStatus::Success;
    m_lastScannedCount = 0;
    m_uniCode = 0;
    m_effectiveBytes = 1;
    m_isASCII = true;
    unsigned char c = m_text.c_str()[m_cursorPosition];
    m_temp = 0;
    size_t seqLen = FormatConstant::GetSequenceLength(c);
    if (0 == seqLen)
        m_status = ScannerCursorStatus::InvalidSymbol;
    if ('\0' == c || m_cursorPosition >= m_totalScanLength)
        return m_uniCode;
    m_lastScannedCount++;
    switch (seqLen)
        {
        case 1: // ASCII
            m_uniCode = (size_t)(c & ~FormatConstant::UTF_TrailingByteMark());
            break;
        case 2: // Two byte sequence
            m_uniCode = (size_t)(c &  ~FormatConstant::UTF_2ByteMask());
            m_lastScannedCount += AddTrailingByte();
            m_isASCII = false;
            temp = m_uniCode & 0xFFFFFF00;
            if (0 != (m_uniCode & 0xFFFFFF00))
                m_effectiveBytes = 2;
            break;
        case 3: // Three byte sequence
                // m_code.octet[TrueIndex(2, sizeof(m_code.octet))] = c & ~m_unicodeConst->Get3ByteMask();
            m_uniCode = (size_t)(c &  ~FormatConstant::UTF_3ByteMask());
            m_lastScannedCount += AddTrailingByte();
            m_lastScannedCount += AddTrailingByte();
            m_isASCII = false;
            temp = m_uniCode & 0xFFFF0000;
            if (0 != (m_uniCode & 0xFFFFFF00))
                m_effectiveBytes = 2;
            if (0 != (m_uniCode & 0xFFFF0000))
                m_effectiveBytes = 3;
            break;
        case 4: // Four byte sequence
                //m_code.octet[TrueIndex(3, sizeof(m_code.octet))] = c & ~m_unicodeConst->Get3ByteMask();
            m_uniCode = (size_t)(c &  ~FormatConstant::UTF_4ByteMask());
            m_lastScannedCount += AddTrailingByte();
            m_lastScannedCount += AddTrailingByte();
            m_lastScannedCount += AddTrailingByte();
            m_isASCII = false;
            if (0 != (m_uniCode & 0xFF000000))
                m_effectiveBytes = 4;
            break;
        }
    if (IsSuccess())
        m_cursorPosition++;
    return m_uniCode;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
size_t FormattingScannerCursor::SkipBlanks()
    {
    Utf8CP c = &m_text.c_str()[m_cursorPosition];

    size_t skipped = 0;
    while (isspace(*c))
        {
        skipped++;
        c++;
        }
    m_cursorPosition += skipped;
    return skipped;
    }

END_BENTLEY_FORMATTING_NAMESPACE