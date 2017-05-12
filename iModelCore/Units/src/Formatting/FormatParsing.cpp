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

size_t FormattingScannerCursor::SetSections()
    {
    return 0;
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
size_t FormattingSignature::AppendPattern(char c)
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

        if(m_stat != AccumulatorState::RejectedSymbol)
            m_bytes++;
        return m_stat;
    }

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

    return m_stat = AccumulatorState::Complete; 
    }

Utf8String NumericAccumulator::ToText()
    {
    Utf8String str;
    str.Sprintf("Sign %d int %d(%d) fract  %d(%d) exp  %d(%d) point[%s] exp[%s] expSign %d real[%s] |%d| |%f|", m_sign, m_part[0], m_count[0], 
                m_part[1], m_count[1], m_part[2], m_count[2], FormatConstant::BoolText(m_point), FormatConstant::BoolText(m_expMark), 
                m_expSign, FormatConstant::BoolText(m_real), m_ival, m_dval);
    return str;
    }

//===================================================
//
// FormattingCursorSection Methods
//
//===================================================
//CursorSectionState FormattingCursorSection::AddSymbol(size_t symb, size_t byteLen)
//    {
//    if (byteLen == 0)
//        byteLen = Utils::NumberOfUtf8Bytes(symb);
//
//    AccumulatorState stat = m_numAcc.AddSymbol(symb);
//    switch (m_type)
//        {
//        case CursorSectionType::Numeric:
//            break;
//        case CursorSectionType::Word:
//            break;
//        case CursorSectionType::Undefined:
//        default:
//            if(stat != AccumulatorState::)
//            m_type = CursorSectionType::Numeric;
//            break;
//
//        }
//    if (AccumulatorState::RejectedSymbol == stat)  // process non-numeric symbol
//        {
//        if(0 == m_numAcc.GetByteCount())  //
//        }
//
//    return CursorSectionState::Success;
//    }

//===================================================
//
// CursorSectionSet Methods
//
//===================================================
//CursorSectionState CursorSectionSet::AddSymbol(size_t symb, size_t byteLen)
//    {
//    if (c == FormatConstant::EndOfLine())
//        return CursorSectionState::Failure;
//
//        if (byteLen == 1)
//            {
//            Utf8Char cc = FormatConstant::ASCIIcode(c);
//            if (isspace(cc))
//                m_traits.AppendSignature(FormatConstant::SpaceSymbol());
//            else if (isdigit(cc))
//                m_traits.AppendSignature(FormatConstant::NumberSymbol());
//            else if (cc == '+' || cc == '-')
//                m_traits.AppendSignature(FormatConstant::SignSymbol());
//            else if (cc == '.' || cc == ',' || cc == '/')
//                m_traits.AppendSignature(cc);
//            else
//                m_traits.AppendSignature(symb[1]); // symb[m_lastScannedCount & 0x7];
//            m_traits.AppendPattern();
//            }
//        else
//            {
//            m_traits.AppendSignature(symb[m_lastScannedCount & 0x7]);
//            if (FormatConstant::IsFractionSymbol(c))
//                m_traits.AppendPattern(FormatConstant::FractionSymbol());
//            else
//                m_traits.AppendPattern(FormatConstant::WordSymbol());
//            }
//        c = GetNextSymbol();
//
//    return CursorSectionState::Success;
//    }

//===================================================
//
// CodepointBuffer Methods
//
//===================================================
//
//CodepointSize CodepointBuffer::IntToSize(size_t cps)
//    {
//    if (cps == 4)
//        return CodepointSize::Quatro;
//    if (cps == 2)
//        return CodepointSize::Double;
//    if (cps == 1)
//        return CodepointSize::Single;
//    return CodepointSize::Zero;
//    }
//
//size_t CodepointBuffer::SizeToInt(CodepointSize cps)
//    {
//    switch (cps)
//        {
//        case CodepointSize::Quatro: return 4;
//        case CodepointSize::Double: return 2;
//        case CodepointSize::Single: return 1;
//        case CodepointSize::Zero:
//        default: return 0;
//        }
//    }
//
//void CodepointBuffer::Init(CodepointSize cps, size_t capacity)
//    {
//    m_size = cps;
//    m_lastIndex = -1;
//    if (CodepointSize::Zero == m_size)
//        {
//        m_capacity = 0;
//        m_bufSize = 0;
//        m_buff.ptr = nullptr;
//        }
//    else
//        {
//        m_capacity = capacity;
//        m_bufSize = (m_capacity + 1)* SizeToInt(m_size);  // extra bytes for terminating zero
//        m_buff.ptr = new char[m_bufSize];
//        }
//    }
//
//void CodepointBuffer::Release()
//    {
//    if (nullptr != m_buff.bytes)
//        delete m_buff.bytes;
//    m_lastIndex = -1;
//    m_size = CodepointSize::Zero;
//    m_capacity = 0;
//    m_bufSize = 0;
//    m_buff.ptr = nullptr;
//    }
//
//void CodepointBuffer::Reset(CodepointSize cps, size_t capacity)
//    {
//    if (nullptr != m_buff.bytes)
//        delete m_buff.bytes;
//    m_lastIndex = -1;
//    if (0 == capacity)
//        {
//        m_size = CodepointSize::Zero;
//        m_capacity = 0;
//        m_bufSize = 0;
//        m_buff.ptr = nullptr;
//        }
//    else
//        {
//        m_size = cps;
//        m_capacity = capacity;
//        m_bufSize = (m_capacity + 1)* SizeToInt(m_size);  // extra bytes for terminating zero
//        m_buff.ptr = new char[m_bufSize];
//        }
//    }
//
//Utf8P CodepointBuffer::AppendByte(Utf8Char symb)
//    {
//    if (m_lastIndex < m_capacity)
//        m_buff.bytes[++m_lastIndex] = symb;
//    return  GetByteBuffer();
//    }
//
//uint32_t* CodepointBuffer::AppendSymbol(uint32_t symb)
//    {
//    if (m_lastIndex < m_capacity)
//        m_buff.longs[++m_lastIndex] = symb;
//    return  GetLongBuffer();
//    }
//
//uint16_t* CodepointBuffer::AppendSymbol(uint16_t symb)
//    {
//    if (m_lastIndex < m_capacity)
//        m_buff.shorts[++m_lastIndex] = symb;
//    return GetShortBuffer();
//    }
//

END_BENTLEY_FORMATTING_NAMESPACE