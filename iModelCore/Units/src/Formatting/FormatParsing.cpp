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
// ScanBuffer Methods
//
//===================================================
size_t ScanBuffer::ExtractSegment(Utf8CP text, ScanSegmentR ss)
    {
    size_t totLen = Utils::TextLength(text); // actual length of the text
    int indx = ss.GetIndex();
    int len = ss.GetLength();
    if (0 <= ss.GetIndex())
        {
        size_t maxLen = totLen - indx;  // how much text we have
        size_t subLen = (ss.GetLength() < 0) ? maxLen : len;  // how much text we will try to extract
        size_t actLen = Utils::MinInt(m_capacity, subLen); // how much text we can actually extract
        memcpy(m_buffer, text + indx, actLen);
        m_buffer[actLen] = 0;
        return actLen;
        }
    return 0;
    }

//===================================================
//
// ScanSegment Methods
//
//===================================================

Utf8String ScanSegment::ExtractSegment(Utf8CP text)
    {
    if (0 > m_indx || m_len == 0)
        return Utf8String();
    size_t actLen = Utils::MinInt(Utils::TextLength(text) - m_indx, m_len);
    Utf8String str(text + m_indx, text + m_indx + actLen);
    return str;
    }

Utf8String ScanSegment::SegmentInfo(Utf8CP prefix)
    {
    Utf8String str;
    Utf8String type = TypeToName(m_type);
    if (nullptr == prefix)
        prefix = "";
    str.reserve(80);
    if(IsPresent())
        str.Sprintf("%s%s(%d, %d)",prefix, type.c_str(), m_indx, m_len);
    else
        str.Sprintf("%s%s(absent)", prefix, type.c_str());
    return str;
    }

ScanSegmentType ScanSegment::IndexToType(int indx)
    {
    if (indx < 0 || indx >= (int)ScanSegmentType::Undefined)
        return ScanSegmentType::Undefined;
    return (ScanSegmentType)indx;
    }

Utf8String ScanSegment::TypeToName(ScanSegmentType type)
    {
    switch (type)
        {
        case ScanSegmentType::Prefix: return "Prefix";
        case ScanSegmentType::Sign: return "Sign";
        case ScanSegmentType::IntPart : return "IntegerPart";
        case ScanSegmentType::DecPoint: return "DecimalPoint";
        case ScanSegmentType::FractPart: return "FractionalPart";
        case ScanSegmentType::ExponentSymbol: return "ExponentSymbol";
        case ScanSegmentType::ExponentSign: return "ExponentSign";
        case ScanSegmentType::ExponentValue: return "ExponentPower";
        case ScanSegmentType::Delimiter: return "Delimiter";
        case ScanSegmentType::UnitName: return "UnitName";
        case ScanSegmentType::UnitDelim: return "UnitDelim";
        case ScanSegmentType::Suffix: return "Suffix";
        case ScanSegmentType::Total: return "Total";
        default: return "Undefined";
        }
    }

//===================================================
//
// NumeriChunk Methods
//
//===================================================

void NumeriChunk::Init()
    {
    for (int i = 0; i < (int)ScanSegmentType::Undefined; i++)
        {
        m_parts[i] = ScanSegment((ScanSegmentType)i);
        }
    }

NumeriChunk::NumeriChunk(Utf8CP text)
    {
    Utf8CP ptr = text;
    int phase = 0;
    while (0 != *ptr && phase < (int)ScanSegmentType::Undefined)
        {
        switch (phase)
            {
            case (int)ScanSegmentType::Prefix:
                break;
            case (int)ScanSegmentType::Sign:
                break;
            case (int)ScanSegmentType::IntPart:
                break;
            case (int)ScanSegmentType::DecPoint:
                break;
            case (int)ScanSegmentType::FractPart:
                break;
            case (int)ScanSegmentType::ExponentSymbol:
                break;
            case (int)ScanSegmentType::ExponentSign:
                break;
            case (int)ScanSegmentType::ExponentValue:
                break;
            case (int)ScanSegmentType::Delimiter:
                break;
            case (int)ScanSegmentType::UnitName:
                break;
            case (int)ScanSegmentType::UnitDelim:
                break;
            case (int)ScanSegmentType::Suffix:
                break;
            case (int)ScanSegmentType::Total:
            default:
                break;
            }
        }

    }


Utf8String NumeriChunk::ChunkInfo(Utf8CP mess)
    {
    Utf8String str;
    for (int i = 0; i < (int)ScanSegmentType::Total; i++)
        {
        str += m_parts[i].SegmentInfo(" ");
        }
    return str;
    }

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
    m_breakIndex = 0;
   /* if (freeBuf)
        m_unicodeBuff.Release();*/
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
PUSH_MSVC_IGNORE(6385 6386) // Static analysis thinks that iArg can exceed the array bounds, but the if statement above ensures it will not.
//POP_MSVC_IGNORE
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
    
    while (isspace(txt[--m_breakIndex]) && 0 < m_breakIndex);
    if (!m_dividers.IsDivider(txt[m_breakIndex]))
        {
        m_status = ScannerCursorStatus::NoEnclosure;
        return FormattingWord(this, emptyBuf, emptyBuf, true);
        }
    Utf8Char div = txt[m_breakIndex];
    Utf8Char m = Utils::MatchingDivider(txt[m_breakIndex]);
    if('\0' == m)
        {
        m_status = ScannerCursorStatus::NoEnclosure;
        return FormattingWord(this, emptyBuf, emptyBuf, true);
        }
    size_t endDivPosition = m_breakIndex;
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
    buf[len] = '\0';
    emptyBuf[0] = div;
    emptyBuf[1] = '\0';
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
    buf[len] = '\0';
    emptyBuf[0] = txt[m_breakIndex];
    emptyBuf[1] = '\0';
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
        buf[len] = '\0';
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
    if ('\0' == c || CursorInRange())
        return GetUnicode();
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
Utf8CP FormattingScannerCursor::GetSignature()
    {
    Rewind(true);
    Utf8CP symb = "0abcdefg";

    m_signature = new char[m_totalScanLength + 1];
    if (nullptr == m_signature)
        return FormatConstant::AllocError();
    else
        {
        size_t c = GetNextSymbol();
        int i = 0;
        while(c != 0 && i < m_totalScanLength)
            {
            if (m_lastScannedCount == 1 && isspace(0x7F & c))
                m_signature[i] = 's';
            else
                m_signature[i] = symb[m_lastScannedCount & 0x7];
            m_signature[++i] = 0;
            c = GetNextSymbol();
            }
        }      
    return m_signature;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
Utf8String FormattingScannerCursor::CollapseSpaces()
    {
    Utf8CP sig = GetSignature();
    size_t sigLen = Utils::IsNameNullOrEmpty(sig) ? 0 : strlen(sig);
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
                for (size_t i = start, j = start, k = 0; i <= end && j < m_totalScanLength; i++)
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
                    buf[k] = '\0';    
                    }
                str = Utf8String(buf);
                }
            }
        }

    return str;
    }
//----------------------------------------------------------------------------------------
// detects a number of groups surrounded by brackes of a special kind (), [], {}  and ||
//   the caller provides only the opening bracket and the function will infer what is a closing bracket
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//----------------------------------------------------------------------------------------
int FormattingScannerCursor::DetectEnclosures(Utf8Char bracket)
    {
    int openN = 0;
    /*int closN = 0;
    Utf8Char closBrk = Utils::MatchingDivider(bracket);
    if ('\0' == closBrk)
        return -1;

    for (size_t ind = 0; ind < m_totalScanLength; ++ind)
        {
 
        }
*/
    return openN;
    }

FormatDividerInstance::FormatDividerInstance(Utf8CP  txt, Utf8Char div)
    {
    m_div = div;
    m_divCount = 0;
    m_mateCount = 0;
    m_totLen = 0;
    m_mate = Utils::MatchingDivider(div);
    if ('\0' == m_mate)
        m_problem = FormatProblemDetail(FormatProblemCode::DIV_UnknownDivider);
    else
        {
        m_problem = FormatProblemDetail();
        if (nullptr != txt)
            {
            while ('\0' != *txt)
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

FormatDividerInstance::FormatDividerInstance(Utf8CP  txt, Utf8CP divs)
    {
    m_div = '\0';
    m_divCount = 0;
    m_mateCount = 0;
    m_totLen = 0;
    m_mate = '\0';
    int indx;
    if (Utils::IsNameNullOrEmpty(divs))
        m_problem = FormatProblemDetail(FormatProblemCode::DIV_UnknownDivider);
    else
        {
        m_problem = FormatProblemDetail();
        if (nullptr != txt)
            {
            while ('\0' != *txt)
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