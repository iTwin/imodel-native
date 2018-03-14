/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Formatting/FormatParsing.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>
#include "../../PrivateAPI/Formatting/FormattingParsing.h"

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//===================================================
//
// FormattingScannerCursorMethods
//
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
void FormattingScannerCursor::Rewind()
    {
    m_cursorPosition = 0;
    m_lastScannedCount = 0;
    m_uniCode = 0;
    m_isASCII = false;
    m_status = ScannerCursorStatus::Success;
    m_effectiveBytes = 0;
    m_breakIndex = 0; 
    return;
    }

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
FormattingScannerCursor::FormattingScannerCursor(Utf8CP utf8Text, int scanLength, Utf8CP div) :m_dividers(div)
    {
    m_text = Utf8String(utf8Text);  // we make a copy of the original string
    Rewind();
    //m_unicodeConst = new UnicodeConstant();
    m_totalScanLength = Utils::TextLength(utf8Text);
    if (scanLength > 0 && scanLength <= (int)m_totalScanLength)
        m_totalScanLength = scanLength;
    m_dividers = FormattingDividers(div);
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
PUSH_MSVC_IGNORE(6385 6386) // Static analysis thinks that iArg can exceed the array bounds, but the if statement above ensures it will not.
FormattingWord FormattingScannerCursor::ExtractWord()
    {
    static const size_t maxDelim = 4;
    Utf8P buff = (Utf8P)alloca(m_totalScanLength + 2);
    buff[0] = 0;
    m_status = ScannerCursorStatus::Success;
    m_lastScannedCount = 0;
    m_isASCII = true;
    if (m_cursorPosition > m_totalScanLength)
        return FormattingWord(this, buff, buff, true);

    Utf8CP c = &m_text.c_str()[m_cursorPosition];

    while (!m_dividers.IsDivider(*c) && m_lastScannedCount < m_totalScanLength)
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
//  This method attempts to extract the content of the last "enclosure" - that is a group of
//    characters enclosed into one of brackets: parenthesis, curvy bracket or square brackets
//     if brackets are not detected - the returned word wil be empty
//  "vertical line" divider is marked by single boolean argument because the divider and its mate are same
// @bsimethod                                                   David Fox-Rabinovitz 03/17
//---------------------------------------------------------------------------------------
FormattingWord FormattingScannerCursor::ExtractLastEnclosure()
    {
    Utf8Char emptyBuf[2];
    emptyBuf[0] = 0;
    m_status = ScannerCursorStatus::Success;
    m_lastScannedCount = 0;
    m_isASCII = true;
    m_breakIndex = m_totalScanLength; // points to the terminating zero
    Utf8CP txt = m_text.c_str();
    
    while (isspace(txt[--m_breakIndex]) && 0 < m_breakIndex);  // skip blanks from the end

    if (!m_dividers.IsDivider(txt[m_breakIndex]))
        {
        m_status = ScannerCursorStatus::NoEnclosure;
        return FormattingWord(this, emptyBuf, emptyBuf, true);
        }
    Utf8Char div = txt[m_breakIndex];
    Utf8Char m = Utils::MatchingDivider(txt[m_breakIndex]);
    if(FormatConstant::EndOfLine() == m)
        {
        m_status = ScannerCursorStatus::NoEnclosure;
        return FormattingWord(this, emptyBuf, emptyBuf, true);
        }
    size_t endDivPosition = m_breakIndex;
    if (div)
        --m_breakIndex;
    while (0 < m_breakIndex && txt[m_breakIndex] != m) --m_breakIndex;

    if(txt[m_breakIndex] != m || endDivPosition <= m_breakIndex)
        {
        m_status = ScannerCursorStatus::NoEnclosure;
        return FormattingWord(this, emptyBuf, emptyBuf, true);
        }
    // ready to extract content

    size_t len = endDivPosition - m_breakIndex - 1;
    Utf8P buf = (Utf8P)alloca(len + 2);

    memcpy(buf, txt + m_breakIndex + 1, len);
    buf[len] = FormatConstant::EndOfLine();
    emptyBuf[0] = div;
    emptyBuf[1] = FormatConstant::EndOfLine();
    FormattingWord word = FormattingWord(this, buf, emptyBuf, m_isASCII);
    return word;
    }

FormattingWord FormattingScannerCursor::ExtractBeforeEnclosure()
    {
    Utf8Char emptyBuf[2];
    emptyBuf[0] = 0;
    m_status = ScannerCursorStatus::Success;
    m_lastScannedCount = 0;
    m_isASCII = true;
    Utf8CP txt = m_text.c_str();
    size_t indx = 0;
    while (isspace(txt[indx]) && indx <= m_totalScanLength) ++indx;

    if (m_breakIndex <= indx || !m_dividers.IsDivider(txt[m_breakIndex]))
        {
        m_status = ScannerCursorStatus::NoEnclosure;
        return FormattingWord(this, emptyBuf, emptyBuf, true);
        }

    size_t len = m_breakIndex - indx;
    
    Utf8P buf = (Utf8P)alloca(len + 2);

    memcpy(buf, txt + indx, len);
    buf[len] = FormatConstant::EndOfLine();
    emptyBuf[0] = txt[m_breakIndex];
    emptyBuf[1] = FormatConstant::EndOfLine();
    FormattingWord word = FormattingWord(this, buf, emptyBuf, m_isASCII);
    return word;
    }

FormattingWord FormattingScannerCursor::ExtractSegment(size_t from, size_t to)
{
    Utf8Char emptyBuf[2];
    emptyBuf[0] = 0;
    m_status = ScannerCursorStatus::Success;
    m_lastScannedCount = 0;
    m_isASCII = true;
    Utf8CP txt = m_text.c_str();

    if(from < to && to <= m_totalScanLength)
        {
        size_t len = to - from +1;
        Utf8P buf = (Utf8P)alloca(len + 2);
        memcpy(buf, txt + from, len);
        buf[len] = FormatConstant::EndOfLine();
        return FormattingWord(this, buf, emptyBuf, m_isASCII);
        }
 
    m_status = ScannerCursorStatus::NoEnclosure;
    return FormattingWord(this, emptyBuf, emptyBuf, true);
}

POP_MSVC_IGNORE

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
    if (FormatConstant::EndOfLine() == c || !CursorInRange())
        return 0; // GetUnicode();
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
size_t FormattingScannerCursor::HeadBitCount(unsigned char c)
    {
    size_t count = 0;
    unsigned char  mask = FormatConstant::UTF_NonASCIIMark();
    if ((c & mask) == 0)
      return 1;
    while ((c & mask) != 0 && count < 7)
        {
        count++;
        mask >>= 1;
        }

    return count;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
size_t FormattingScannerCursor::GetNextReversed()
    {
    m_status = ScannerCursorStatus::Success;
    m_lastScannedCount = 0;
    m_uniCode = 0;
    m_effectiveBytes = 1;
    m_isASCII = true;
    Utf8Char c = GetPrecedingByte();
    m_lastScannedCount = 1;

    if (IsAscii(c))
        {
        m_lastScannedCount = 1;
        return m_uniCode = (size_t)c;
        }

    size_t shift = 0;
    while (IsTrailing(c) && shift < 31)
        {
        m_uniCode |= FormatConstant::ExtractTrailingBits(c, shift);
        shift += FormatConstant::UTF_UpperBitShift();
        c = GetPrecedingByte();
        m_lastScannedCount++;
        }
    // now byte must be a header of the Utf8 sequence
    size_t bitCount = HeadBitCount(c);
    if (m_lastScannedCount < 2 || bitCount != m_lastScannedCount) // corrupted Utf8 sequence
        m_status = ScannerCursorStatus::IncompleteSequence;
    else  // adding the header byte
        {
        size_t hBits = 0;
        switch (bitCount)
            {
            case 2:
                hBits = c & FormatConstant::UTF_2ByteSelector();
                break;
            case 3:
                hBits = c & ~FormatConstant::UTF_3ByteSelector();
                break;
            case 4:
                hBits = c & ~FormatConstant::UTF_4ByteSelector();
                break;
            case 5:
                hBits = c & ~FormatConstant::UTF_5ByteSelector();
                break;
            case 6:
                hBits = c & ~FormatConstant::UTF_6ByteSelector();
                break;
            }
        hBits <<= shift;
        m_uniCode |= hBits;
        }
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
Utf8CP FormattingScannerCursor::GetSignature(bool refresh, bool compress)
    {
    if (!refresh)
        return m_traits.GetSignature();

    if(!m_traits.Reset(m_totalScanLength))
        return FormatConstant::AllocError();
    Utf8CP symb = "xabcdefg";

    size_t c = GetNextSymbol();

    while (c != 0)
        {
        if (m_lastScannedCount == 1)
            {
            Utf8Char cc = FormatConstant::ASCIIcode(c);
            if (isspace(cc))
                m_traits.AppendSignature(FormatConstant::SpaceSymbol());
            else if (isdigit(cc))
                m_traits.AppendSignature(FormatConstant::NumberSymbol());
            else if (cc == '+' || cc == '-')
                m_traits.AppendSignature(FormatConstant::SignSymbol());
            else if (cc == '.' || cc == ',' || cc == '/' || cc == '_')
                m_traits.AppendSignature(cc);
            else
                m_traits.AppendSignature(symb[1]); // symb[m_lastScannedCount & 0x7];
            m_traits.AppendPattern();
            }
        else
            {
            m_traits.AppendSignature(symb[m_lastScannedCount & 0x7]);
            if(FormatConstant::IsFractionSymbol(c))
                m_traits.AppendPattern(FormatConstant::FractionSymbol());
            else
                m_traits.AppendPattern(FormatConstant::WordSymbol());
            }
        c = GetNextSymbol();
        }
    if (compress)
        {
        m_traits.CompressPattern();
        }
    return m_traits.GetSignature();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
Utf8CP FormattingScannerCursor::GetReversedSignature(bool refresh, bool compress)
    {
    if (!refresh)
        return m_traits.GetSignature();

    if (!m_traits.Reset(m_totalScanLength))
        return FormatConstant::AllocError();

    size_t savePos = m_cursorPosition;
    Utf8CP symb = "xabcdefg";
    m_cursorPosition = m_totalScanLength;
    size_t c = GetNextReversed();
    if (m_status != ScannerCursorStatus::Success)
        return "!!Error!!";
    while (c != 0)
        {
        if (m_lastScannedCount == 1)
            {
            Utf8Char cc = FormatConstant::ASCIIcode(c);
            if (isspace(cc))
                m_traits.AppendSignature(FormatConstant::SpaceSymbol());
            else if (isdigit(cc))
                m_traits.AppendSignature(FormatConstant::NumberSymbol());
            else if (cc == '+' || cc == '-')
                m_traits.AppendSignature(FormatConstant::SignSymbol());
            else if (cc == '.' || cc == ',' || cc == '/' || cc == '_')
                m_traits.AppendSignature(cc);
            else
                m_traits.AppendSignature(symb[1]); // symb[m_lastScannedCount & 0x7];
            m_traits.AppendPattern();
            }
        else
            {
            m_traits.AppendSignature(symb[m_lastScannedCount & 0x7]);
            if (FormatConstant::IsFractionSymbol(c))
                m_traits.AppendPattern(FormatConstant::FractionSymbol());
            else
                m_traits.AppendPattern(FormatConstant::WordSymbol());
            }
        c = GetNextReversed();
        }
    if (compress)
        {
        m_traits.CompressPattern();
        }
    m_cursorPosition = savePos;
    return m_traits.GetSignature();
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 02/17
//----------------------------------------------------------------------------------------
PUSH_MSVC_IGNORE(6385 6386)
Utf8String FormattingScannerCursor::CollapseSpaces(bool replace)
    {
    Utf8CP sig = GetSignature(true, true);
    size_t sigLen = Utils::TextLength(sig);
    Utf8String str;

    if (sigLen > 0)
        {
        size_t start = 0;
        size_t end = sigLen - 1;
        for (Utf8CP p = sig; *p == 's'; ++p, ++start);
        if (start < sigLen)
            {
            while (end > start && sig[end] == 's') --end;
            if (end >= start)
                {
                Utf8P buf = (Utf8P)alloca(m_totalScanLength + 2);
                bool notSpace = true;
                for (size_t i = start, j = start, k = 0; i <= end && j < m_totalScanLength && k < m_totalScanLength; i++)
                    {
                    switch (sig[i])
                        {
                        case 's':
                            if (notSpace)
                                buf[k++] = m_text[j++];
                            else
                                j++;
                            notSpace = false;
                            break;
                        case 'f':
                            buf[k++] = m_text[j++];
                        case 'e':
                            buf[k++] = m_text[j++];
                        case 'd':
                            buf[k++] = m_text[j++];
                        case 'c':
                            buf[k++] = m_text[j++];
                        case 'b':
                            buf[k++] = m_text[j++];
                        case 'a':
                        default:
                            buf[k++] = m_text[j++];
                            notSpace = true;
                            break;
                        }
                    buf[k] = FormatConstant::EndOfLine();
                    }
                str = Utf8String(buf);
                }
            if (replace)
                {
                Rewind();
                m_text = str;
                m_totalScanLength = strlen(m_text.c_str());
                }
            }//start < sigLen

        } // siglen > 0

    return str;
    }
POP_MSVC_IGNORE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
FormatDividerInstance::FormatDividerInstance(Utf8CP  txt, Utf8Char div)
    {
    m_div = div;
    m_divCount = 0;
    m_mateCount = 0;
    m_totLen = 0;
    m_mate = Utils::MatchingDivider(div);
    if (FormatConstant::EndOfLine() == m_mate)
        m_problem = FormatProblemDetail(FormatProblemCode::DIV_UnknownDivider);
    else
        {
        m_problem = FormatProblemDetail();
        if (nullptr != txt)
            {
            while (FormatConstant::EndOfLine() != *txt)
                {
                if (*txt == m_div)
                    {
                    m_positions.push_back(m_totLen+1);
                    m_divCount++;
                    }
                else if (*txt == m_mate)
                    {
                    m_positions.push_back(-m_totLen-1);
                    m_mateCount++;
                    }
                ++txt;
                ++m_totLen;
                }
            }
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
FormatDividerInstance::FormatDividerInstance(Utf8CP  txt, Utf8CP divs)
    {
    m_div = FormatConstant::EndOfLine();
    m_divCount = 0;
    m_mateCount = 0;
    m_totLen = 0;
    m_mate = FormatConstant::EndOfLine();
    int indx;
    if (Utils::IsNameNullOrEmpty(divs))
        m_problem = FormatProblemDetail(FormatProblemCode::DIV_UnknownDivider);
    else
        {
        m_problem = FormatProblemDetail();
        if (nullptr != txt)
            {
            while (FormatConstant::EndOfLine() != *txt)
                {
                indx = Utils::IndexOf(*txt, divs);
                if (indx >= 0)
                    {
                    m_positions.push_back((indx << 8) + (int)*txt);
                    m_divCount++;
                    }
                ++txt;
                ++m_totLen;
                }
            }
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
bool FormatDividerInstance::IsDivLast() 
    {
    if (0 < m_divCount)
        {
        int last = m_positions.back();
        bool check = (m_div == m_mate) ? m_totLen == last : m_totLen == -last;
        return check;
        }
    return false;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
Utf8String FormatDividerInstance::ToText()
    {
    if (m_problem.IsProblem())
     return "Unknown Divider";
    Utf8String str;
    str.Sprintf("Dividers: %d  mates %d", m_divCount, m_mateCount);
    if (m_positions.size() > 0 )
        {
        Utf8String tmp;
        for(int* pos = m_positions.begin(), *end = m_positions.end(); pos != end; ++pos)
            {
            if (*pos >= 0)
                tmp.Sprintf(" %c[%d]", m_div, *pos - 1);
            else
                tmp.Sprintf(" %c[%d]", m_mate, -(*pos) - 1);
            str += tmp;
            }
        }
    return str;
    }


//===================================================
//
// FormattingSignature Methods
//
//===================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
bool FormattingSignature::Reset(size_t reserve)
    { 
    if (0 == reserve || reserve > m_size) // special case for releasing allocated memory
        {
        m_size = reserve;
        if (nullptr != m_signature)
            delete m_signature;
        if (nullptr != m_pattern)
            delete m_pattern;
        if (reserve > 0)
            {
            m_pattern = new char[m_size + 2];
            m_signature = new char[m_size + 2];
            if (nullptr == m_signature || nullptr == m_pattern)
                return false;
            m_signature[0] = FormatConstant::EndOfLine();
            m_pattern[0] = FormatConstant::EndOfLine();
            }
        else
            {
            m_signature = nullptr;
            m_pattern = nullptr;
            }    
        }
    else 
        {
        if(nullptr != m_signature)
            m_signature[0] = FormatConstant::EndOfLine();
        if(nullptr != m_pattern)
            m_pattern[0] = FormatConstant::EndOfLine();
        }
    m_segCount = 0;
    m_sigIndx = 0;
    m_patIndx = 0;
    memset(m_segPos, 0, sizeof(m_segPos));
    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod      Constructor                                  David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
FormattingSignature::FormattingSignature(size_t reserve)
    {
    Reset(reserve);
    }

size_t FormattingSignature::AppendSignature(Utf8Char c)
    { 
    if (m_sigIndx < m_size) 
        m_signature[m_sigIndx++] = c; 
    m_signature[m_sigIndx] = FormatConstant::EndOfLine();
    return m_sigIndx; 
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
size_t FormattingSignature::AppendPattern(Utf8Char c)
    { 
    if((m_patIndx < m_size) && (m_patIndx == 0 || m_pattern[m_patIndx-1] != c))
        m_pattern[m_patIndx++] = c;
    m_pattern[m_patIndx] = FormatConstant::EndOfLine();
    return m_patIndx;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
size_t FormattingSignature::AppendPattern()
    {
    if (m_sigIndx > 0)
        {
        if ((m_patIndx < m_size) && (m_patIndx == 0 || m_pattern[m_patIndx - 1] != m_signature[m_sigIndx - 1]))
            m_pattern[m_patIndx++] = m_signature[m_sigIndx - 1];
        m_pattern[m_patIndx] = FormatConstant::EndOfLine();
        }
    return m_patIndx;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
size_t FormattingSignature::DetectUOMPattern(size_t ind)
    {
    Utf8Char buf[12];
    size_t i = 0;
    memset(buf, 0, sizeof(buf));
    Utf8Char c = GetPatternChar(ind);
    if (c == FormatConstant::NumberSymbol())
        {
        while (i < 10)
            {
            buf[i++] = c;
            c = GetPatternChar(ind + i);
            if (c == FormatConstant::NumberSymbol() || c == FormatConstant::EndOfLine())
                break;
            }
        if (strcmp(buf, "0sas") == 0 || strcmp(buf, "0as") == 0 || strcmp(buf, "0sa") == 0)
            return i;
        }
    return 0;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 04/17
//----------------------------------------------------------------------------------------
size_t FormattingSignature::DetectFractPattern(size_t ind)
    {
    Utf8Char buf[12];
    size_t i = 0;
    memset(buf, 0, sizeof(buf));
    Utf8Char c = GetPatternChar(ind);
    if (c == FormatConstant::NumberSymbol())
        {
        while (i < 10)
            {
            buf[i++] = c;
            c = GetPatternChar(ind + i);
            if (c == FormatConstant::NumberSymbol() || c == FormatConstant::EndOfLine())
                break;
            }
        if (strcmp(buf, "0sas") == 0 || strcmp(buf, "0as") == 0 || strcmp(buf, "0sa") == 0)
            return i;
        }
    return 0;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Utf8CP FormattingSignature::ReverseString(Utf8CP str, Utf8P revStr, size_t bufSize)
    {
    size_t len = Utils::TextLength(str);
    if (nullptr == revStr || bufSize == 0)
        return nullptr;
    if (bufSize < len + 1)
        {
        *revStr = 0;
        return revStr;
        }
    while (len > 0)
        {
        *revStr = str[--len];
        revStr++;
        }
    *revStr = 0;
    return revStr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Utf8String FormattingSignature::ReversedSignature()
    {
    size_t sigL = Utils::TextLength(m_signature);
    if (sigL == 0)
        return nullptr;
    Utf8P temp = (Utf8P)alloca(sigL + 2);
    ReverseString(m_signature, temp, sigL + 2);

    return Utf8String(temp);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Utf8String FormattingSignature::ReversedPattern()
    {
    size_t sigL = Utils::TextLength(m_pattern);
    if (sigL == 0)
        return nullptr;
    Utf8P temp = (Utf8P)alloca(sigL + 2);
    ReverseString(m_pattern, temp, sigL + 2);

    return Utf8String(temp);
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
PUSH_MSVC_IGNORE(6385 6386)
size_t FormattingSignature::CompressPattern()
    {
    if (FormatConstant::EndOfLine() == m_pattern[0])
        return 0;
    Utf8P temp = (Utf8P)alloca(m_size + 2);
    size_t i = 0, j = 0, n;
    while (i < m_patIndx && j <= m_size)
        {
        n = DetectUOMPattern(i);
        if (n > 0)  // pattern is detected
            {
            temp[j++] = FormatConstant::NumberSymbol();
            temp[j++] = FormatConstant::WordSymbol();
            i += n;
            }
        else
            {
            temp[j++] = GetPatternChar(i++);
            }
        temp[j] = FormatConstant::EndOfLine();
        }
    memcpy(m_pattern, temp, i);
    m_pattern[i] = FormatConstant::EndOfLine();
    return i;
    }
POP_MSVC_IGNORE
//===================================================
//
// NumericAccumulator Methods
//
//===================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
int  NumericAccumulator::AddDigitValue(Utf8Char c)
    {
    switch (m_stat)
        {
        case AccumulatorState::Init:
            SetIntegerState();
        case AccumulatorState::Integer:
            return IncrementIntPart(FormatConstant::DigitValue(c));
        case AccumulatorState::Fraction:
            return IncrementFractPart(FormatConstant::DigitValue(c));
        case AccumulatorState::Exponent:
            return IncrementExponentPart(FormatConstant::DigitValue(c));
        default:
            break;
        }
    return 0;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
AccumulatorState NumericAccumulator::SetSign(int sig)
    {
    if (m_stat == AccumulatorState::Init && m_sign == 0)
        {
        m_sign = sig;
        return m_stat = AccumulatorState::Integer;
        }
    else if(m_stat == AccumulatorState::Exponent && m_expSign == 0)
        {
        m_expSign = sig;
        return m_stat;
        }
    else
        m_problem.UpdateProblemCode(FormatProblemCode::NA_InvalidSign);
    return m_stat = AccumulatorState::Failure;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
AccumulatorState NumericAccumulator::SetDecimalPoint()
    {
    if((m_stat == AccumulatorState::Init || m_stat == AccumulatorState::Integer) && m_point == false)
        {
        m_point = true;
        return m_stat = AccumulatorState::Fraction;
        }
    m_problem.UpdateProblemCode(FormatProblemCode::NA_InvalidPoint);
    return m_stat = AccumulatorState::Failure;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
AccumulatorState NumericAccumulator::SetExponentMark()
    {
    if((m_stat == AccumulatorState::Integer|| m_stat == AccumulatorState::Fraction) && (m_count[0] + m_count[1] > 0) && m_expMark == false)
        {
        m_expMark = true;
        return m_stat = AccumulatorState::Exponent;
        }      
    m_problem.UpdateProblemCode(FormatProblemCode::NA_InvalidExponent);
    return m_stat = AccumulatorState::Failure;
    }

AccumulatorState NumericAccumulator::AddSymbol(size_t symb)
    {
    if (FormatConstant::IsASCII(symb)) // symbol must be ASCII
        {
        Utf8Char c = FormatConstant::ASCIIcode(symb);
        if (isdigit(c))
            {
            AddDigitValue(c);
            m_bytes++;
            return m_stat;
            }

        switch (c)
            {
            case '+':
                SetSign(1);
                break;
            case '-':
                SetSign(-1);
                break;
            case '.':
                SetDecimalPoint();
                break;
            case 'E':
            case 'e':
                SetExponentMark();
                break;
            default:
                m_stat = AccumulatorState::RejectedSymbol;
            }
        }
        else
            m_stat = AccumulatorState::RejectedSymbol;

        if (m_stat != AccumulatorState::RejectedSymbol)
            m_bytes++;
        return m_stat;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
AccumulatorState NumericAccumulator::SetComplete() 
    { 
    if (HasFailed() || IsComplete())
        return m_stat;
    if (m_expMark && m_count[2] == 0)
        {
        m_problem.UpdateProblemCode(FormatProblemCode::NA_InvalidExponent);
        return m_stat = AccumulatorState::Failure;
        }
    if (m_point && (m_count[0] + m_count[1]) == 0)
        {
        m_problem.UpdateProblemCode(FormatProblemCode::NA_InvalidSyntax);
        return m_stat = AccumulatorState::Failure;
        }
    if (0 != m_sign && (m_count[0] + m_count[1]) == 0)
        {
        m_problem.UpdateProblemCode(FormatProblemCode::NA_InvalidSyntax);
        return m_stat = AccumulatorState::Failure;
        }
    m_ival = (m_sign >=0)?  m_part[0] : -m_part[0];
    m_dval = (double)m_part[0];
    if (m_point)
        {
        if (m_count[1] > 0) // there is a fractional part
            {
            double f = (double)m_part[1];
            for (int i = 0; i < m_count[1]; ++i) { f *= 0.1; }
            m_dval += f;
            }
        m_real = true;
        }
    if (m_expMark && m_count[2] > 0)
        {
        double f = 1.0;
        double mult = (m_expSign < 0) ? 0.1 : 10.0;
  
        for (int i = 0; i < m_count[1]; ++i) { f *= mult; }
        m_dval *= f;
        m_real = true;
        }
    if (m_sign < 0)
        m_dval = -m_dval;
    if (m_real)
        m_ival = (int)m_dval;

    return m_stat = AccumulatorState::Complete; 
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Utf8String NumericAccumulator::ToText()
    {
    Utf8String str;
    str.Sprintf("Sign %d int %d(%d) fract  %d(%d) exp  %d(%d) point[%s] exp[%s] expSign %d real[%s] |%d| |%f|", m_sign, m_part[0], m_count[0], 
                m_part[1], m_count[1], m_part[2], m_count[2], FormatConstant::BoolText(m_point), FormatConstant::BoolText(m_expMark), 
                m_expSign, FormatConstant::BoolText(m_real), m_ival, m_dval);
    return str;
    }

//===================================================
// UnitProxy Methods
//===================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
UnitProxy::UnitProxy(Units::UnitCP unit, Utf8CP label)
    {
    m_unitLabel = nullptr;
    m_unit = unit;
    if (nullptr != m_unit)
        {
        if (!Utf8String::IsNullOrEmpty(label))
            m_unitLabel = Utf8String(label);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
Json::Value UnitProxy::ToJson() const
    {
    Json::Value jUP;

    if(nullptr != m_unit)
        jUP[json_unitName()] = m_unit->GetName().c_str();
    if (!m_unitLabel.empty())
        jUP[json_unitLabel()] = m_unitLabel.c_str();
    return jUP;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//----------------------------------------------------------------------------------------
void UnitProxy::LoadJson(Json::Value jval, BEU::IUnitsContextCP context)
    {
    m_unitLabel.clear();
    m_unit = nullptr;
    if (jval.empty())
        return;

    Utf8CP paramName;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (BeStringUtilities::StricmpAscii(paramName, json_unitName()) == 0)
            {
            Utf8CP str = val.asCString();
            if (nullptr != str)
                m_unit = context->LookupUnit(str);
            }
        else if (BeStringUtilities::StricmpAscii(paramName, json_unitLabel()) == 0)
            m_unitLabel = val.asString().c_str();
        }
    }

//===================================================
//
// CursorScanPoint Methods
//
//============================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
// the caller is responsible for keeping the index inside the allowable range
//----------------------------------------------------------------------------------------
ScannerCursorStatus CursorScanPoint::AppendTrailingByte(Utf8CP txt)
    {
    if (ScannerCursorStatus::Success == m_status && m_len < sizeof(m_bytes))
        {
        char bits = 0;
        unsigned char c = txt[m_indx];
        if (FormatConstant::GetTrailingBits(c, &bits)) // this validates the byte and extracts trailing bits
            {
            m_uniCode <<= FormatConstant::GetTrailingShift();
            m_uniCode |= bits;
            m_bytes[m_len++] = c;
            m_indx++;
            return ScannerCursorStatus::Success;
            }
        else
            return ScannerCursorStatus::IncompleteSequence;
        }
    return m_status;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
// the caller is responsible for keeping the index inside the allowable range
//----------------------------------------------------------------------------------------
CursorScanPoint::CursorScanPoint(Utf8CP input, size_t indx, bool revers)
    {
    m_indx = indx;
    if (revers)
      ProcessPrevious(input);
    else
      ProcessNext(input);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void CursorScanPoint::Iterate(Utf8CP input, bool revers)
    {
    if (revers)
        ProcessPrevious(input);
    else
        ProcessNext(input);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void CursorScanPoint::ProcessASCII(unsigned char c)
    {
    m_bytes[m_len++] = c;
    m_uniCode = FormatConstant::ASCIIcode(c);
    m_patt = 'a';
    if (isspace(c))
        m_patt = FormatConstant::SpaceSymbol();
    else if (isdigit(c))
        m_patt = FormatConstant::NumberSymbol();
    else if (c == '+' || c == '-')
        m_patt = FormatConstant::SignSymbol();
    else if (c == 'e' || c == 'E')
        m_patt = 'x';     // a suspicion of exponent - TBD later
    else if (c == '.' || c == ',' || c == '/' || c == '_' || c == ':')
        m_patt = c;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void CursorScanPoint::ProcessNext(Utf8CP input)
    {
    if (nullptr == input)
        {
        input = FormatConstant::EmptyString();
        m_indx = 0;
        }
    m_uniCode = 0;
    m_len = 0;
    memset(m_bytes, '\0', sizeof(m_bytes));
    m_patt = FormatConstant::EndOfLine();
    m_status = ScannerCursorStatus::Success;

    unsigned char c = input[m_indx];
    size_t seqLen = FormatConstant::GetSequenceLength(c);
    while (ScannerCursorStatus::Success == m_status)
        {
        if (FormatConstant::IsEndOfLine(c))    // detected end of line
            break;
        if (seqLen == 1)
            {
            ProcessASCII(c);
            //m_bytes[m_len++] = c;
            //m_uniCode = FormatConstant::ASCIIcode(c);
            //m_patt = 'a';
            //if (isspace(c))
            //    m_patt = FormatConstant::SpaceSymbol();
            //else if (isdigit(c))
            //    m_patt = FormatConstant::NumberSymbol();
            //else if (c == '+' || c == '-')
            //    m_patt = FormatConstant::SignSymbol();
            //else if (c == 'e' || c == 'E')
            //    m_patt = 'x';     // a suspicion of exponent - TBD later
            //else if (c == '.' || c == ',' || c == '/' || c == '_')
            //   m_patt = c;
            m_indx++;
            break;
            }
        if (seqLen == 2)
            {
            m_bytes[m_len++] = c;
            m_uniCode = (size_t)(c &  ~FormatConstant::UTF_2ByteMask());
            m_indx++;
            AppendTrailingByte(input);
            m_patt = 'b';
            break;
            }
        if (seqLen == 3)
            {
            m_bytes[m_len++] = c;
            m_uniCode = (size_t)(c &  ~FormatConstant::UTF_3ByteMask());
            m_indx++;
            AppendTrailingByte(input);
            AppendTrailingByte(input);
            m_patt = 'c';
            break;
            }
        if (seqLen == 4)
            {
            m_bytes[m_len++] = c;
            m_uniCode = (size_t)(c &  ~FormatConstant::UTF_4ByteMask());
            m_indx++;
            AppendTrailingByte(input);
            AppendTrailingByte(input);
            AppendTrailingByte(input);
            m_patt = 'd';
            break;
            }

        m_status = ScannerCursorStatus::InvalidSymbol;
        break;
        }
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void CursorScanPoint::Init()
    {
    m_uniCode = 0;
    m_indx = 0;
    m_len = 0;
    memset(m_bytes, 0, sizeof(m_bytes));
    m_patt = '\0';
    m_status = ScannerCursorStatus::Success;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void CursorScanPoint::ProcessPrevious(Utf8CP input)
    {
    if (nullptr == input)
        {
        input = FormatConstant::EmptyString();
        m_indx = 0;
        }
    m_uniCode = 0;
    m_len = 0;
    memset(m_bytes, '\0', sizeof(m_bytes));
    m_patt = FormatConstant::EndOfLine();
    m_status = ScannerCursorStatus::Success;
    unsigned char c = GetPrecedingByte(input);
    while (ScannerCursorStatus::Success == m_status)
        {
        if (FormatConstant::EndOfLine() == c)    // detected end of line
            break;
        if (FormatConstant::IsASCIIChar(c))  // it's the simplest case - we done
            {
            ProcessASCII(c);
            //m_bytes[m_len++] = c;
            //m_uniCode = FormatConstant::ASCIIcode(c);
            //m_patt = 'a';
            //if (isspace(c))
            //    m_patt = FormatConstant::SpaceSymbol();
            //else if (isdigit(c))
            //    m_patt = FormatConstant::NumberSymbol();
            //else if (c == '+' || c == '-')
            //    m_patt = FormatConstant::SignSymbol();
            //else if (c == 'e' || c == 'E')
            //    m_patt = 'x';     // a suspicion of exponent - TBD later
            //else if (c == '.' || c == ',' || c == '/'|| c== '_')
            //    m_patt = c;
            break;
            }
        // need to find the head byte by moving backward
        int n = 0;
        while (FormatConstant::IsTrailingByteValid(c) && m_indx > 0 && n < 3)
            {
            c = GetPrecedingByte(input);
            ++n;
            }
        size_t seqLen = FormatConstant::GetSequenceLength(c); // it must be 2, 3 or 4
       
        if (seqLen == 2)
            {
            m_bytes[m_len++] = c;
            m_uniCode = (size_t)(c &  FormatConstant::UTF_2ByteSelector());
            m_bytes[m_len] = input[m_indx + 1];
            m_uniCode <<= FormatConstant::UTF_UpperBitShift();
            m_uniCode |= (size_t)(m_bytes[m_len++] & FormatConstant::UTF_TrailingBitsMask());
            m_patt = 'b';
            break;
            }
         if (seqLen == 3)
            {
            m_bytes[m_len] = c;
            m_uniCode = (size_t)(m_bytes[m_len++] & FormatConstant::UTF_3ByteSelector());

            m_bytes[m_len] = input[m_indx + 1];
            m_uniCode <<= FormatConstant::UTF_UpperBitShift();
            m_uniCode |= (size_t)(m_bytes[m_len++] & FormatConstant::UTF_TrailingBitsMask());
            m_bytes[m_len] = input[m_indx + 2];
            m_uniCode <<= FormatConstant::UTF_UpperBitShift();
            m_uniCode |= (size_t)(m_bytes[m_len++] & FormatConstant::UTF_TrailingBitsMask());
            m_patt = 'c';
            break;
            }
          if (seqLen == 4)
            {
            m_bytes[m_len] = c;
            m_uniCode = (size_t)(m_bytes[m_len++] & FormatConstant::UTF_4ByteSelector());
            m_bytes[m_len] = input[m_indx + 1];
            m_uniCode <<= FormatConstant::UTF_UpperBitShift();
            m_uniCode |= (size_t)(m_bytes[m_len++] & FormatConstant::UTF_TrailingBitsMask());
            m_bytes[m_len] = input[m_indx + 2];
            m_uniCode <<= FormatConstant::UTF_UpperBitShift();
            m_uniCode |= (size_t)(m_bytes[m_len++] & FormatConstant::UTF_TrailingBitsMask());
            m_bytes[m_len] = input[m_indx + 3];
            m_uniCode <<= FormatConstant::UTF_UpperBitShift();
            m_uniCode |= (size_t)(m_bytes[m_len++] & FormatConstant::UTF_TrailingBitsMask());
            m_patt = 'd';
            break;
            }

            m_status = ScannerCursorStatus::InvalidSymbol;
            break;
        }// end of While
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
Utf8PrintfString CursorScanPoint::ToText()
    {
    Utf8CP asc = IsAscii() ? Utf8PrintfString("(ASCII %c)", m_bytes[0]).c_str() : FormatConstant::EmptyString();

    Utf8PrintfString txt("Ucode: %d[0x%x] %s indx %d byteLen %d bytes(0x%x 0x%x 0x%x 0x%x) patt %c",
        m_uniCode, m_uniCode, asc, m_indx, m_len, m_bytes[0], m_bytes[1], m_bytes[2], m_bytes[3],
        (IsEndOfLine() ? 'z' : m_patt));
    return txt;
    }

//===================================================
//
// FormatParseVector Methods
//
//===================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
size_t FormatParseVector::ExtractTriplet()
    {
    m_patt.word = 0;
    m_bufIndex = 0; // it is also the actual number of bytes in the triplet
    for (size_t i = m_tripletIndx; m_bufIndex < 3 && i < m_vect.size(); m_bufIndex++, i++)
        {
        m_patt.byte[m_bufIndex] = m_vect[i].GetPattern();
        }
    return m_bufIndex;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
bool FormatParseVector::MoveFrame(bool forw)
    {
    bool mov = false;
    if (forw)
    {
        if (m_tripletIndx + 3 < m_vect.size())
            {
            ++m_tripletIndx;
            mov = true;
            }
    }
    else
    {
        if (m_tripletIndx > 0)
            {
            --m_tripletIndx;
            mov = true;
            }
    }
    if(mov)
       ExtractTriplet();
    return mov;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
FormatParseVector::FormatParseVector(Utf8CP input, bool revers)
    {
    m_patt.word = 0;
    m_tripletIndx = 0;
    size_t n = strlen(input);
    CursorScanPoint csp(input, revers ? n : 0, revers);
    while (!csp.IsEndOfLine() && n > 0)
        {
        m_vect.push_back(csp);
        csp.Iterate(input, revers);
        }
    ExtractTriplet();
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
size_t FormatParseVector::AddPoint(CursorScanPointCR pnt)
    {
     //CursorScanPointCP last = m_vect.empty() ? nullptr : &m_vect.back();
    m_vect.push_back(pnt);
    return m_vect.size();
    }

PUSH_MSVC_IGNORE(6385 6386)

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
Utf8String FormatParseVector::GetPattern()
    {
    Utf8Char last, foll;
    size_t blen = m_vect.size() + 2;
    Utf8P buf = (Utf8P)alloca(blen);
    memset((void*)buf, 0, blen);
    size_t i = 0;
    last = '\0';
    for (CursorScanPointP csp = m_vect.begin(); csp != m_vect.end(); csp++)
        {
        foll = csp->GetPattern();
        if (foll != last)
            {
            buf[i++] = foll;
            last = foll;
            }
        }
    return Utf8String(buf);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
Utf8String FormatParseVector::GetSignature()
    {
    size_t blen = m_vect.size() + 2;
    Utf8P buf = (Utf8P)alloca(blen);
    memset((void*)buf, 0, blen);
    size_t i = 0;
    for (CursorScanPointP csp = m_vect.begin(); csp != m_vect.end(); csp++)
        {
        buf[i++] = csp->GetPattern();
        }
    return Utf8String(buf);
    }
POP_MSVC_IGNORE

//===================================================
//
// CursorScanTriplet Methods
//
//============================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void CursorScanTriplet::Init(bool rev)
    {
    m_strInd = 0;
    m_revers = rev;
    m_tripInd = m_revers? 3 : 0;
    memset(m_triple, 0, sizeof(m_triple));
    memset(m_patt, '\0', sizeof(m_patt));
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
CursorScanTriplet::CursorScanTriplet()
    {
    Init(false);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
bool CursorScanTriplet::SetReverse(bool rev) 
    {
    Init(rev);
    return m_revers;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 05/17
//   this method creates a "sliding" cursor that exposes three consequtive symbols from
//   the input string the "reading head" is always at the current position in the string
//     
//----------------------------------------------------------------------------------------
void CursorScanTriplet::PushSymbol(size_t symb) 
    {
    if (m_revers)
        {
        if (m_tripInd == 0)
            {
            m_triple[2] = m_triple[1];
            m_triple[1] = m_triple[0];
            m_triple[0] = symb;
            }
        else
            m_triple[--m_tripInd] = symb;
        }
    else
        {
        if(m_tripInd < 2)
          m_triple[m_tripInd++] = symb;
        else
            {
            m_triple[0] = m_triple[1];
            m_triple[1] = m_triple[2];
            m_triple[2] = symb;
            }
        }
    }
//===================================================
//
// NumberGrabber Methods
//
//===================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
size_t NumberGrabber::Grab(Utf8CP input, size_t start)
    {
    if (!Utils::IsNameNullOrEmpty(input))
        {
        m_input = input;
        m_start = start;
        }
    m_next = m_start;
    m_ival = 0;
    m_dval = 0;  
    bool revs = false;
    CursorScanPoint csp = CursorScanPoint(m_input, m_next, revs);
    if (csp.IsEndOfLine())
        {
        m_type = ParsingSegmentType::EndOfLine;
        return 0;
        }

    m_type = ParsingSegmentType::NotNumber;
    int ival = 0;
    int fval = 0;
    int eval = 0;
    int denom = 0;
    double dval = 0.0;
    int iCount = 0;
    int fCount = 0;
    int eCount = 0;
    int dCount = 0;  // denominator
    int sign = 0;
    bool point = false;
    bool  expMark = false;

    int expSign = 0;
    bool num = false;

    Utf8Char c;

    while (csp.IsSpace()) { m_next++; csp.Iterate(m_input, revs); }
    if (csp.IsSign())  // the sign character can be expected at the start
        {
        c = csp.GetAscii();
        sign = (c == '-') ? -1 : 1;
        ++m_next;
        csp.Iterate(m_input, revs);
        }
    while (csp.IsDigit())    // if there any digits - they will be accumulated in the int part
        {
        int d = (int)(csp.GetAscii() - '0');
        ival = ival * 10 + d;
        iCount++;
        ++m_next;
        csp.Iterate(m_input, revs);
        }

    if (csp.IsBar())  // a very special case of a fraction 
        {
        if (iCount > 0)
            {
            ++m_next;
            csp.Iterate(m_input, revs);
            while (csp.IsDigit())    // if there any digits - they will be accumulated in the int part
                {
                int d = (int)(csp.GetAscii() - '0');
                denom = denom * 10 + d;
                dCount++;
                ++m_next;
                csp.Iterate(m_input, revs);
                }
            if (dCount > 0 && denom > 0)
                {
                dval = (double)ival / (double)denom;
                m_dval = (sign < 0) ? -dval : dval;
                m_ival = (int)m_dval;    // regardless of what we collected there it should be refreshed
                m_type = ParsingSegmentType::Fraction;
                }
            else
                m_next = m_start;
            }
        else
           m_next = m_start;    // the sequence cannot be recognized 
        return m_next - m_start;
        }


    if (csp.IsPoint()) // we can expect a decimal point which can actually come by itself
        {              // or can be followed by more digits
        point = true;
        ++m_next;
        csp.Iterate(m_input, revs);
        while (csp.IsDigit())    // if there any digits - they will be accumulated in the fraction
            {
            int d = (int)(csp.GetAscii() - '0');
            fval = fval * 10 + d;
            fCount++;
            ++m_next;
            csp.Iterate(m_input, revs);
            }
        }
    if (csp.IsExponent())
        {
        expMark = true;
        ++m_next;
        csp.Iterate(m_input, revs);
        if (csp.IsSign())  // the sign character can be expected at the start
            {
            c = csp.GetAscii();
            expSign = (c == '-') ? -1 : 1;
            ++m_next;
            csp.Iterate(m_input, revs);
            }
        while (csp.IsDigit())    // if there any digits - they will be accumulated in the fraction
            {
            int d = (int)(csp.GetAscii() - '0');
            eval = eval * 10 + d;
            eCount++;
            ++m_next;
            csp.Iterate(m_input, revs);
            }
        }
    // it's time to validate the number
    if (iCount > 0 || fCount > 0) num = true; // at least some digits
 
   /* if ((sign != 0 && !num) || (!num && expMark) || (expMark && eCount == 0))
        {
        m_next = m_start;
        }*/
    if(num && (point || expMark))
        {
        m_type = ParsingSegmentType::Real;
        double f = (double)fval;
        for (int i = 0; i < fCount; ++i) { f *= 0.1; }
        dval = (double)ival + f;
             
        if (expMark && eCount > 0)
            {
            double f = 1.0;
            double mult = (expSign < 0) ? 0.1 : 10.0;

            for (int i = 0; i < eval; ++i) { f *= mult; }
            dval *= f;
            }
        m_dval = (sign < 0)? -dval : dval;
        m_ival = (int)dval;    // regardless of what we collected there it should be refreshed
        }
    else if(iCount > 0)
        {
        m_type = ParsingSegmentType::Integer;
        m_ival = (sign < 0) ? -ival : ival;
        m_dval = (double)m_ival;
        }
    else
        m_next = m_start;

    return m_next - m_start;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
Utf8CP NumberGrabber::GetTail() 
    { 
    return &m_input[m_next]; 
    }

//===================================================
//
// FormatParsingSegment Methods
//
//===================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void FormatParsingSegment::Init(size_t start)
    {
    m_start = start;
    m_byteCount = 0;
    m_vect.clear();
    m_type = ParsingSegmentType::NotNumber;
    m_ival = 0;
    m_dval = 0.0;
    m_unit = nullptr;
    m_name = "";
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
FormatParsingSegment::FormatParsingSegment(NumberGrabberCR ng)
    {
    if (ng.GetType() == ParsingSegmentType::NotNumber)
        Init(ng.GetStartIndex());
    else
        {
        m_start = ng.GetStartIndex();
        m_byteCount = ng.GetLength();
        m_vect.clear();
        m_type = ng.GetType();
        m_ival = ng.GetInteger();
        m_dval = ng.GetReal();
        }
    }
PUSH_MSVC_IGNORE(6385 6386)

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
FormatParsingSegment::FormatParsingSegment(bvector<CursorScanPoint> vect, size_t s, BEU::UnitCP refUnit)
    {
    Init(s);
    size_t bufL = vect.size() * 4 + 2;
    Utf8Char* buf = (Utf8Char*)alloca(bufL);
    memset(buf, 0, bufL);
    int i = 0;
    unsigned char* ptr;
    for (CursorScanPointP vp = vect.begin(); vp != vect.end(); vp++)
        {
        m_byteCount += vp->GetLength();
        m_vect.push_back(*vp);
        for (ptr = vp->GetBytes(); *ptr != '\0'; ptr++)
            {
            buf[i++] = *ptr;
            }
        }
    if (i > 0) // something in the buffer
        {
        if (buf[0] == '_') // special case of prpending the unit name by underscore
            m_name = Utf8String(buf + 1);
        else
            m_name = Utf8String(buf);

        m_unit = nullptr;
        if(nullptr != refUnit)  // before looking into the registry we can try to find Unit in the Phenomenon
            {
            BEU::PhenomenonCP ph = refUnit->GetPhenomenon();
            m_unit = ph->LookupUnit(m_name.c_str());
            }
         }
    }
POP_MSVC_IGNORE

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
BEU::UnitCP FormatParsingSegment::GetUnit(Formatting::FormatUnitSetCP fusP)
    { 
    return (nullptr == fusP) ? m_unit : fusP->GetUnit(); 
    }

Utf8PrintfString FormatParsingSegment::ToText(int n)
    {
     Utf8PrintfString head("start %d bytes %d ", m_start, m_byteCount);
     if (m_type == ParsingSegmentType::Real)
         {
         return Utf8PrintfString("Segment[%d] %s Real %.5f Int %d", n, head.c_str(), m_dval, m_ival);
         }
     else if (m_type == ParsingSegmentType::Integer)
         {
         return Utf8PrintfString("Segment[%d] %s Int %d Real %.5f", n, head.c_str(), m_ival, m_dval);
         }
     else if (m_type == ParsingSegmentType::Fraction)
         {
         return Utf8PrintfString("Segment[%d] %s Fraction %.5f Int %d ", n, head.c_str(), m_dval, m_ival);
         }
     else
         {
         //size_t bufL = 4 * m_vect.size() + 2;
         //Utf8Char* buf = (Utf8Char*)alloca(bufL);
         //memset(buf, 0, bufL);
         //int i = 0;
         //unsigned char* ptr;

        /* for (CursorScanPointP vp = m_vect.begin(); vp != m_vect.end(); vp++)
             {
             for (ptr = vp->GetBytes(); *ptr != '\0'; ptr++)
                 {
                 buf[i++] = *ptr;
                 }
             }*/

         return Utf8PrintfString("Segment[%d] %s Text %s %s", n, head.c_str(), m_name.c_str(),
                        ((nullptr == m_unit)? "(not a Unit)" : "(Unit)"));

         }
    }

//===================================================
//
// FormatParsingSet Methods
//
//===================================================
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
BEU::UnitCP FormatParsingSet::GetUnit(Formatting::FormatUnitSetCP fusP)
    {
    return (nullptr == fusP) ? m_unit : fusP->GetUnit();
    }

void FormatParsingSet::Init(Utf8CP input, size_t start, BEU::UnitCP unit)
    {
    m_input = input;
    m_unit = unit;
    m_problem.Reset();
    FormatParsingSegment fps;
    NumberGrabber ng;
    bvector<CursorScanPoint> m_symbs;
    bool revs = false;
    CursorScanPoint csp = CursorScanPoint();
    size_t ind = start;
    size_t ind0 = start;
    bool initVect = true;

    if (!Utf8String::IsNullOrEmpty(input))
        {
        ng = NumberGrabber();
        while (!ng.IsEndOfLine())
            {
            ng.Grab(m_input, ind);
            if (ng.GetLength() > 0)  // a number is detected
                {
                if (m_symbs.size() > 0)
                    {
                    fps = FormatParsingSegment(m_symbs, ind0, m_unit);
                    m_segs.push_back(fps);
                    m_symbs.clear();
                    }

                fps = FormatParsingSegment(ng);
                m_segs.push_back(fps);
                ind = ng.GetNextIndex();
                initVect = true;
                }
            else
                {
                if (initVect) ind0 = ind;
                initVect = false;
                csp = CursorScanPoint(m_input, ind, revs);
                if (csp.IsSpace())
                    {
                    if (m_symbs.size() > 0)
                        {
                        fps = FormatParsingSegment(m_symbs, ind0, m_unit);
                        m_segs.push_back(fps);
                        m_symbs.clear();
                        ind = csp.GetIndex();
                        ind0 = ind;
                        }
                    while (csp.IsSpace()) { csp.Iterate(m_input, revs); }
                    ind = csp.GetIndex();
                    ind0 = ind;
                    }
                if (csp.IsEndOfLine())
                    break;
                m_symbs.push_back(csp);
                ind = csp.GetIndex();
                }
            }
        if (m_symbs.size() > 0)
            {
            fps = FormatParsingSegment(m_symbs, ind0, m_unit);
            m_segs.push_back(fps);
            }
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 07/17
//----------------------------------------------------------------------------------------
FormatParsingSet::FormatParsingSet(Utf8CP input, BEU::UnitCP unit)
    {
    Init(input, 0, unit);
    }

PUSH_MSVC_IGNORE(6385 6386)

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 07/17
//----------------------------------------------------------------------------------------
Utf8String FormatParsingSet::GetSignature(bool distinct) //, int* colonCount)
    {
    Utf8String txt = "";
    if (m_segs.size() == 0)
        return txt; 
    ParsingSegmentType type;
    size_t bufL = m_segs.size() * 4 + 2;
    Utf8Char* buf = (Utf8Char*)alloca(bufL);
    memset(buf, 0, bufL);
    int i = 0;
    int colNum = 0;

    for (FormatParsingSegmentP fps = m_segs.begin(); fps != m_segs.end(); fps++)
        {
        type = fps->GetType();
        if (type == ParsingSegmentType::Real)
            buf[i++] = distinct? 'R': 'N';
        else if (type == ParsingSegmentType::Integer)
            buf[i++] = distinct ? 'I' : 'N';
        else if (type == ParsingSegmentType::Fraction)
            buf[i++] = 'F';
        else
            {
            if (fps->IsColon())
                {
                buf[i++] = 'C';
                colNum++;
                }
            else if (fps->IsDoubleColon())
                {
                buf[i++] = 'C';
                buf[i++] = 'C';
                colNum = 2;
                }
            else if (fps->IsTripleColon())
                {
                buf[i++] = 'C';
                buf[i++] = 'C';
                buf[i++] = 'C';
                colNum = 3;
                }
            else if (fps->IsMinusColon())
                {
                buf[i++] = '-';
                buf[i++] = 'C';
                colNum++;
                }
            else if (fps->IsMinusDoubleColon())
                {
                buf[i++] = '-';
                buf[i++] = 'C';
                buf[i++] = 'C';
                colNum = 2;
                }
            else if (fps->IsMinusTripleColon())
                {
                buf[i++] = '-';
                buf[i++] = 'C';
                buf[i++] = 'C';
                buf[i++] = 'C';
                colNum = 3;
                }
            else if (nullptr == fps->GetUnit())
                buf[i++] = 'W';
            else
                buf[i++] = 'U';
            }
        }
    //if (nullptr != colonCount)
    //    *colonCount = colNum;
    return Utf8String(buf);
    }
POP_MSVC_IGNORE
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 07/17
//----------------------------------------------------------------------------------------
bool FormatParsingSet::ValidateParsingFUS(int reqUnitCount, FormatUnitSetCP fusP)
    {
    if (0 == reqUnitCount)
        return false;
   if (nullptr == fusP)
        m_problem.UpdateProblemCode(FormatProblemCode::PS_MissingFUS);
    if (!fusP->HasComposite())
        m_problem.UpdateProblemCode(FormatProblemCode::PS_MissingCompositeSpec);
    if(reqUnitCount != fusP->GetCompositeUnitCount())
        m_problem.UpdateProblemCode(FormatProblemCode::PS_MismatchingFUS);
    if (m_problem.IsProblem())
        return false;

    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 07/17
//----------------------------------------------------------------------------------------
BEU::Quantity  FormatParsingSet::GetQuantity(FormatProblemCode* probCode, FormatUnitSetCP fusP)
    {
    BEU::Quantity qty = BEU::Quantity();
    BEU::Quantity tmp = BEU::Quantity();
    //int sigT = 0;
    Utf8String sig = GetSignature(false);
    // only a limited number of signatures will be recognized in this particular context
    // reduced version: NU, NFU, NUNU, NUNFU NUNUNU NUNUNFU
    //   3 FT - NU
    //  1/3 FT  FU
    BEU::UnitCP majP, midP;
    //FormatProblemCode locCode;
    double sign = 1.0;
    //if (nullptr == probCode)
   //     probCode = &locCode;

    m_problem.Reset();
   
    //double mu, su;
    Formatting::FormatSpecialCodes cod = Formatting::FormatConstant::ParsingPatternCode(sig.c_str());
    switch (cod)
        {
        case Formatting::FormatSpecialCodes::SignatureN:
        case Formatting::FormatSpecialCodes::SignatureF:
            qty = BEU::Quantity(m_segs[0].GetReal(), *GetUnit(fusP));    // *m_unit);
            break;
        case Formatting::FormatSpecialCodes::SignatureNF:
            sign = m_segs[0].GetSign();
            qty = BEU::Quantity(m_segs[0].GetAbsReal() + m_segs[1].GetAbsReal(), *GetUnit(fusP));
            qty.Scale(sign);
            break;
        case Formatting::FormatSpecialCodes::SignatureNU:
            majP = m_segs[1].GetUnit();
            qty = BEU::Quantity(m_segs[0].GetReal(), *majP);
            break;
        case Formatting::FormatSpecialCodes::SignatureNFU:
            sign = m_segs[0].GetSign();
            majP = m_segs[2].GetUnit();
            qty = BEU::Quantity(m_segs[0].GetAbsReal() + m_segs[1].GetAbsReal(), *majP);
            break;
        case Formatting::FormatSpecialCodes::SignatureNUNU:
            sign = m_segs[0].GetSign();
            majP = m_segs[1].GetUnit();
            qty = BEU::Quantity(m_segs[0].GetAbsReal(), *majP);
            midP = m_segs[3].GetUnit();
            tmp = BEU::Quantity(m_segs[2].GetAbsReal(), *midP);
            qty = qty.Add(tmp);
            qty.Scale(sign);
            break;
        case Formatting::FormatSpecialCodes::SignatureNUNFU:
            sign = m_segs[0].GetSign();
            majP = m_segs[1].GetUnit();
            qty = BEU::Quantity(m_segs[0].GetAbsReal(), *majP);
            midP = m_segs[4].GetUnit();
            tmp = BEU::Quantity(m_segs[2].GetAbsReal() + m_segs[3].GetAbsReal(), *midP);
            qty = qty.Add(tmp);
            qty.Scale(sign);
            break;
        case Formatting::FormatSpecialCodes::SignatureNUNUNU:
            sign = m_segs[0].GetSign();
            majP = m_segs[1].GetUnit();
            qty = BEU::Quantity(m_segs[0].GetAbsReal(), *majP);
            midP = m_segs[3].GetUnit();
            tmp = BEU::Quantity(m_segs[2].GetAbsReal(), *midP);
            qty = qty.Add(tmp);
            midP = m_segs[5].GetUnit();
            tmp = BEU::Quantity(m_segs[4].GetAbsReal(), *midP);
            qty = qty.Add(tmp);
            qty.Scale(sign);
            break;
        case Formatting::FormatSpecialCodes::SignatureNUNUNFU:
            sign = m_segs[0].GetSign();
            majP = m_segs[1].GetUnit();
            qty = BEU::Quantity(m_segs[0].GetAbsReal(), *majP);
            midP = m_segs[3].GetUnit();
            tmp = BEU::Quantity(m_segs[2].GetAbsReal(), *midP);
            qty = qty.Add(tmp);
            midP = m_segs[6].GetUnit();
            tmp = BEU::Quantity(m_segs[4].GetAbsReal() + m_segs[5].GetAbsReal(), *midP);
            qty = qty.Add(tmp);
            qty.Scale(sign);
            break;

    /*    case Formatting::FormatSpecialCodes::SignatureNCNCN:
            if(ValidateParsingFUS(3, fusP))
                {
                sign = m_segs[0].GetSign();
                majP = fusP->GetCompositeMajorUnit();
                if(nullptr == majP)
                    m_problem.UpdateProblemCode(FormatProblemCode::PS_MismatchingFUS);
                else
                   qty = BEU::Quantity(m_segs[0].GetAbsReal(), *majP);
                }*/
        //case Formatting::FormatSpecialCodes::SignatureNCCN:
        //case Formatting::FormatSpecialCodes::SignatureNCC:
        //case Formatting::FormatSpecialCodes::SignatureCNCN:
        //case Formatting::FormatSpecialCodes::SignatureCNC:
        //case Formatting::FormatSpecialCodes::SignatureCCN:
        //case Formatting::FormatSpecialCodes::SignatureNCNC:
        //    break;

        default:
            qty = FormatParsingSet::ComposeColonizedQuantity(cod, fusP);
            //m_problem.UpdateProblemCode(FormatProblemCode::QT_InvalidSyntax);
            break;
        }

    if (!m_problem.IsProblem() && nullptr != GetUnit(fusP))
        qty = qty.ConvertTo(GetUnit(fusP));
    if (nullptr != probCode)
        *probCode = m_problem.GetProblemCode();
    return qty;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
BEU::Quantity  FormatParsingSet::ComposeColonizedQuantity(Formatting::FormatSpecialCodes cod, FormatUnitSetCP fusP)
    {
    BEU::Quantity qty = BEU::Quantity();
    BEU::Quantity tmp = BEU::Quantity();
    if (m_problem.IsProblem())
        return qty;
    BEU::UnitCP majP, midP, minP;
    double sign = 1.0;

    switch (cod)
        {
        case Formatting::FormatSpecialCodes::SignatureNCNCN:
            if (ValidateParsingFUS(3, fusP))
                {
                sign = m_segs[0].GetSign();
                majP = fusP->GetCompositeMajorUnit();
                midP = fusP->GetCompositeMiddleUnit();
                minP = fusP->GetCompositeMinorUnit();
                if (nullptr == majP || nullptr == midP || nullptr == minP)
                    m_problem.UpdateProblemCode(FormatProblemCode::PS_MismatchingFUS);
                else
                    {
                    qty = BEU::Quantity(m_segs[0].GetAbsReal(), *majP);
                    tmp = BEU::Quantity(m_segs[2].GetAbsReal(), *midP);
                    qty = qty.Add(tmp);
                    tmp = BEU::Quantity(m_segs[4].GetAbsReal(), *minP);
                    qty = qty.Add(tmp);
                    qty.Scale(sign);
                    }
                }
            break;
        case Formatting::FormatSpecialCodes::SignatureNCCN:
            if (ValidateParsingFUS(3, fusP))
                {
                sign = m_segs[0].GetSign();
                majP = fusP->GetCompositeMajorUnit();
                minP = fusP->GetCompositeMinorUnit();
                if (nullptr == majP || nullptr == minP)
                    m_problem.UpdateProblemCode(FormatProblemCode::PS_MismatchingFUS);
                else
                    {
                    qty = BEU::Quantity(m_segs[0].GetAbsReal(), *majP);
                    tmp = BEU::Quantity(m_segs[2].GetAbsReal(), *minP);
                    qty = qty.Add(tmp);
                    qty.Scale(sign);
                    }
                }
            break;
        case Formatting::FormatSpecialCodes::SignatureNCC:
            if (ValidateParsingFUS(3, fusP))
                {
                sign = m_segs[0].GetSign();
                majP = fusP->GetCompositeMajorUnit();
                if (nullptr == majP)
                    m_problem.UpdateProblemCode(FormatProblemCode::PS_MismatchingFUS);
                else
                    {
                    qty = BEU::Quantity(m_segs[0].GetAbsReal(), *majP);
                    qty.Scale(sign);
                    }
                }
            break;

        case Formatting::FormatSpecialCodes::SignatureMCNCN:
            sign = -1.0;
        case Formatting::FormatSpecialCodes::SignatureCNCN:
            if (ValidateParsingFUS(3, fusP))
                {
                midP = fusP->GetCompositeMiddleUnit();
                minP = fusP->GetCompositeMinorUnit();
                if (nullptr == midP || nullptr == minP)
                    m_problem.UpdateProblemCode(FormatProblemCode::PS_MismatchingFUS);
                else
                    {
                    qty = BEU::Quantity(m_segs[0].GetAbsReal(), *midP);
                    tmp = BEU::Quantity(m_segs[2].GetAbsReal(), *minP);
                    qty = qty.Add(tmp);
                    qty.Scale(sign);
                    }
                }
            break;
        case Formatting::FormatSpecialCodes::SignatureMCNC:
            sign = -1.0;
        case Formatting::FormatSpecialCodes::SignatureCNC:
            if (ValidateParsingFUS(3, fusP))
                {
                midP = fusP->GetCompositeMiddleUnit();
                if (nullptr == midP)
                    m_problem.UpdateProblemCode(FormatProblemCode::PS_MismatchingFUS);
                else
                    {
                    qty = BEU::Quantity(m_segs[0].GetAbsReal(), *midP);
                    qty.Scale(sign);
                    }
                }
            break;
        case Formatting::FormatSpecialCodes::SignatureMCCN:
            sign = -1.0;
        case Formatting::FormatSpecialCodes::SignatureCCN:
            if (ValidateParsingFUS(3, fusP))
                {
                minP = fusP->GetCompositeMinorUnit();
                if (nullptr == minP)
                    m_problem.UpdateProblemCode(FormatProblemCode::PS_MismatchingFUS);
                else
                    {
                    qty = BEU::Quantity(m_segs[0].GetAbsReal(), *minP);
                    qty.Scale(sign);
                    }
                }
            break;
        case Formatting::FormatSpecialCodes::SignatureNCNC:
            if (ValidateParsingFUS(3, fusP))
                {
                sign = m_segs[0].GetSign();
                majP = fusP->GetCompositeMajorUnit();
                midP = fusP->GetCompositeMiddleUnit();
                if (nullptr == majP || nullptr == midP)
                    m_problem.UpdateProblemCode(FormatProblemCode::PS_MismatchingFUS);
                else
                    {
                    qty = BEU::Quantity(m_segs[0].GetAbsReal(), *majP);
                    tmp = BEU::Quantity(m_segs[2].GetAbsReal(), *midP);
                    qty = qty.Add(tmp);
                    qty.Scale(sign);
                    }
                }
            break;
        case Formatting::FormatSpecialCodes::SignatureNCN:
            if (ValidateParsingFUS(2, fusP))
                {
                sign = m_segs[0].GetSign();
                majP = fusP->GetCompositeMajorUnit();
                midP = fusP->GetCompositeMiddleUnit();
                if (nullptr == majP || nullptr == midP)
                    m_problem.UpdateProblemCode(FormatProblemCode::PS_MismatchingFUS);
                else
                    {
                    qty = BEU::Quantity(m_segs[0].GetAbsReal(), *majP);
                    tmp = BEU::Quantity(m_segs[2].GetAbsReal(), *midP);
                    qty = qty.Add(tmp);
                    qty.Scale(sign);
                    }
                }
            break;

        case Formatting::FormatSpecialCodes::SignatureNC:
            if (ValidateParsingFUS(2, fusP))
                {
                sign = m_segs[0].GetSign();
                majP = fusP->GetCompositeMajorUnit();
                if (nullptr == majP)
                    m_problem.UpdateProblemCode(FormatProblemCode::PS_MismatchingFUS);
                else
                    {
                    qty = BEU::Quantity(m_segs[0].GetAbsReal(), *majP);
                    qty.Scale(sign);
                    }
                }
            break;

        case Formatting::FormatSpecialCodes::SignatureMCN:
            sign = -1.0;
        case Formatting::FormatSpecialCodes::SignatureCN:
            if (ValidateParsingFUS(2, fusP))
                {
                midP = fusP->GetCompositeMiddleUnit();
                if (nullptr == midP)
                    m_problem.UpdateProblemCode(FormatProblemCode::PS_MismatchingFUS);
                else
                    {
                    qty = BEU::Quantity(m_segs[1].GetAbsReal(), *midP);
                    qty.Scale(sign);
                    }
                }
            break;

        default:
            m_problem.UpdateProblemCode(FormatProblemCode::QT_InvalidSyntax);
            break;
        }
    return qty;
    }

END_BENTLEY_FORMATTING_NAMESPACE