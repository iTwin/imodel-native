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
#include "../../PrivateAPI/Formatting/NumericFormatUtils.h"
#include <regex>

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//===================================================
// FormattingScannerCursor
//===================================================

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
FormattingScannerCursor::FormattingScannerCursor(Utf8CP utf8Text, int scanLength, Utf8CP div) :m_dividers(div)
    {
    m_text = Utf8String(utf8Text);  // we make a copy of the original string
    m_status = ScannerCursorStatus::Success;
    m_breakIndex = 0; 
    m_totalScanLength = StringUtils::TextLength(utf8Text);
    if (scanLength > 0 && scanLength <= (int)m_totalScanLength)
        m_totalScanLength = scanLength;
    m_dividers = FormattingDividers(div);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 03/17
//---------------------------------------------------------------------------------------
Utf8String FormattingScannerCursor::ExtractLastEnclosure()
    {
    Utf8Char emptyBuf[2];
    emptyBuf[0] = 0;
    m_status = ScannerCursorStatus::Success;
    m_breakIndex = m_totalScanLength; // points to the terminating zero
    Utf8CP txt = m_text.c_str();
    
    while (isspace(txt[--m_breakIndex]) && 0 < m_breakIndex);  // skip blanks from the end

    if (!m_dividers.IsDivider(txt[m_breakIndex]))
        {
        m_status = ScannerCursorStatus::NoEnclosure;
        return emptyBuf;
        }
    Utf8Char div = txt[m_breakIndex];
    Utf8Char m = Utils::MatchingDivider(txt[m_breakIndex]);
    if(FormatConstant::EndOfLine() == m)
        {
        m_status = ScannerCursorStatus::NoEnclosure;
        return emptyBuf;
        }
    size_t endDivPosition = m_breakIndex;
    if (div)
        --m_breakIndex;
    while (0 < m_breakIndex && txt[m_breakIndex] != m) --m_breakIndex;

    if(txt[m_breakIndex] != m || endDivPosition <= m_breakIndex)
        {
        m_status = ScannerCursorStatus::NoEnclosure;
        return emptyBuf;
        }
    // ready to extract content

    size_t len = endDivPosition - m_breakIndex - 1;
    Utf8P buf = (Utf8P)alloca(len + 2);

    memcpy(buf, txt + m_breakIndex + 1, len);
    buf[len] = FormatConstant::EndOfLine();
    emptyBuf[0] = div;
    emptyBuf[1] = FormatConstant::EndOfLine();
    return Utf8String(buf);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
Utf8String FormattingScannerCursor::ExtractBeforeEnclosure()
    {
    Utf8Char emptyBuf[2];
    emptyBuf[0] = 0;
    m_status = ScannerCursorStatus::Success;
    Utf8CP txt = m_text.c_str();
    size_t indx = 0;
    while (isspace(txt[indx]) && indx <= m_totalScanLength) ++indx;

    if (m_breakIndex <= indx || !m_dividers.IsDivider(txt[m_breakIndex]))
        {
        m_status = ScannerCursorStatus::NoEnclosure;
        return emptyBuf;
        }

    size_t len = m_breakIndex - indx;
    
    Utf8P buf = (Utf8P)alloca(len + 2);

    memcpy(buf, txt + indx, len);
    buf[len] = FormatConstant::EndOfLine();
    emptyBuf[0] = txt[m_breakIndex];
    emptyBuf[1] = FormatConstant::EndOfLine();
    return Utf8String(buf);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 11/16
//---------------------------------------------------------------------------------------
Utf8String FormattingScannerCursor::ExtractSegment(size_t from, size_t to)
    {
    Utf8Char emptyBuf[2];
    emptyBuf[0] = 0;
    m_status = ScannerCursorStatus::Success;
    Utf8CP txt = m_text.c_str();

    if(from < to && to <= m_totalScanLength)
        {
        size_t len = to - from +1;
        Utf8P buf = (Utf8P)alloca(len + 2);
        memcpy(buf, txt + from, len);
        buf[len] = FormatConstant::EndOfLine();
        return Utf8String(buf);
        }
 
    m_status = ScannerCursorStatus::NoEnclosure;
    return emptyBuf;
    }

//===================================================
// FormatDividerInstance
//===================================================

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
    if (Utf8String::IsNullOrEmpty(divs))
        m_problem = FormatProblemDetail(FormatProblemCode::DIV_UnknownDivider);
    else
        {
        m_problem = FormatProblemDetail();
        if (nullptr != txt)
            {
            while (FormatConstant::EndOfLine() != *txt)
                {
                indx = StringUtils::IndexOf(*txt, divs);
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
    if (0 >= m_divCount)
        return false;

    int last = m_positions.back();
    return (m_div == m_mate) ? m_totLen == last : m_totLen == -last;
    }

//===================================================
// CursorScanPoint
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
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
CursorScanPoint::CursorScanPoint(Utf8CP input, size_t indx)
    {
    m_indx = indx;
    ProcessNext(input);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void CursorScanPoint::Iterate(Utf8CP input)
    {
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

//===================================================
// NumberGrabber
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
size_t NumberGrabber::Grab(Utf8CP input, size_t start)
    {
    if (!Utf8String::IsNullOrEmpty(input))
        {
        m_input = input;
        m_start = start;
        }
    m_next = m_start;
    m_ival = 0;
    m_dval = 0;  
    CursorScanPoint csp = CursorScanPoint(m_input, m_next);
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

    while (csp.IsSpace()) { m_next++; csp.Iterate(m_input); }
    if (csp.IsSign())  // the sign character can be expected at the start
        {
        c = csp.GetAscii();
        sign = (c == '-') ? -1 : 1;
        ++m_next;
        csp.Iterate(m_input);
        }
    while (csp.IsDigit())    // if there any digits - they will be accumulated in the int part
        {
        int d = (int)(csp.GetAscii() - '0');
        ival = ival * 10 + d;
        iCount++;
        ++m_next;
        csp.Iterate(m_input);
        }

    if (csp.IsBar())  // a very special case of a fraction 
        {
        if (iCount > 0)
            {
            ++m_next;
            csp.Iterate(m_input);
            while (csp.IsDigit())    // if there any digits - they will be accumulated in the int part
                {
                int d = (int)(csp.GetAscii() - '0');
                denom = denom * 10 + d;
                dCount++;
                ++m_next;
                csp.Iterate(m_input);
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
        csp.Iterate(m_input);
        while (csp.IsDigit())    // if there any digits - they will be accumulated in the fraction
            {
            int d = (int)(csp.GetAscii() - '0');
            fval = fval * 10 + d;
            fCount++;
            ++m_next;
            csp.Iterate(m_input);
            }
        }
    if (csp.IsExponent())
        {
        expMark = true;
        ++m_next;
        csp.Iterate(m_input);
        if (csp.IsSign())  // the sign character can be expected at the start
            {
            c = csp.GetAscii();
            expSign = (c == '-') ? -1 : 1;
            ++m_next;
            csp.Iterate(m_input);
            }
        while (csp.IsDigit())    // if there any digits - they will be accumulated in the fraction
            {
            int d = (int)(csp.GetAscii() - '0');
            eval = eval * 10 + d;
            eCount++;
            ++m_next;
            csp.Iterate(m_input);
            }
        }

    // it's time to validate the number
    if (iCount > 0 || fCount > 0)
        num = true; // at least some digits

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

//===================================================
// FormatParsingSegment
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
    void FormatParsingSegment::Init(size_t start)
    {
    m_start = start;
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
        m_vect.clear();
        m_type = ng.GetType();
        m_ival = ng.GetInteger();
        m_dval = ng.GetReal();
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
FormatParsingSegment::FormatParsingSegment(bvector<CursorScanPoint> vect, size_t s, BEU::UnitCP refUnit, FormatCP format, QuantityFormatting::UnitResolver* resolver)
    {
    Init(s);
    size_t bufL = vect.size() * 4 + 2;
    Utf8Char* buf = (Utf8Char*)alloca(bufL);
    memset(buf, 0, bufL);
    int i = 0;
    unsigned char* ptr;
    for (CursorScanPointP vp = vect.begin(); vp != vect.end(); vp++)
        {
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
            if (nullptr != resolver)
                m_unit = (*resolver)(m_name.c_str(), ph);
            if (nullptr == m_unit)
                {
                // special case to treat ^ as degree symbol
                if (ph->IsAngle() && m_name.Equals("^"))
                    m_name.AssignOrClear("ARC_DEG");

                auto const& units = ph->GetUnits();
                auto matchingUnit = std::find_if(units.begin(), units.end(), 
                    [&](BEU::UnitCP unit) {return ((unit->GetName().EqualsI(m_name)) || (unit->GetDisplayLabel() == m_name));});
                if (matchingUnit != units.end())
                    m_unit = *matchingUnit;
                else
                    {
                    if (nullptr != format && format->HasComposite())
                        {
                        CompositeValueSpecCP comp = format->GetCompositeSpec();
                        if (comp->HasMajorLabel())
                            {
                            if (comp->GetMajorLabel().EqualsI(m_name))
                                {
                                m_unit = comp->GetMajorUnit();
                                return;
                                }
                            if (comp->HasMiddleLabel())
                                {
                                if (comp->GetMajorLabel().EqualsI(m_name))
                                    {
                                m_unit = comp->GetMiddleUnit();
                                    return;
                                    }

                                if (comp->HasMinorLabel())
                                    {
                                    if (comp->GetMinorLabel().EqualsI(m_name))
                                        {
                                        m_unit = comp->GetMinorUnit();
                                        return;
                                        }

                                    if (comp->HasSubLabel())
                                        {
                                        if (comp->GetSubLabel().EqualsI(m_name))
                                            {
                                            m_unit = comp->GetSubUnit();
                                            return;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

//===================================================
// FormatParsingSet
//===================================================

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 06/17
//----------------------------------------------------------------------------------------
void FormatParsingSet::Init(Utf8CP input, size_t start, BEU::UnitCP unit, FormatCP format, QuantityFormatting::UnitResolver* resolver)
    {
    m_format = format;
    m_input = input;
    m_unit = unit;
    m_problem.Reset();
    FormatParsingSegment fps;
    NumberGrabber ng;
    bvector<CursorScanPoint> m_symbs;
    CursorScanPoint csp = CursorScanPoint();
    size_t ind = start;
    size_t ind0 = start;
    bool initVect = true;

    if (Utf8String::IsNullOrEmpty(input))
        return;

    //special case for station format in order not to brake functionality below
    if (nullptr != m_format && m_format->GetPresentationType() == PresentationType::Station)
        {
        Utf8String specialChars(R"*(([]()\.*+^?|{}$))*"); // if separator is one of these, we need to escape

        auto signWithSpacePattern(R"*(([+-]?)\s*)*");
        auto integerPattern(R"*((0|[1-9]\d*))*");
        auto realNumberPattern(R"*((\d*\.\d+|\d+))*");

        std::string separatorString(1, m_format->GetNumericSpec()->GetStationSeparator());
        auto separator = separatorString.c_str();

        Utf8String separatorPattern;
        if (-1 != specialChars.find(separator))
            separatorPattern.append("\\");
        separatorPattern.append(separator);

        Utf8String regexString(signWithSpacePattern);
        regexString.append(integerPattern).append(separatorPattern).
            append(realNumberPattern);

        std::regex regex { regexString.c_str() };
        std::smatch matches;

        Utf8String inputString(input);
        inputString.Trim();
        std::string const inputStringForRegex(inputString.c_str());

        if (std::regex_search(inputStringForRegex, matches, regex))
            {
            auto sign(matches[1].str());
            auto majorNumber(matches[2].str());
            auto minorNumber(matches[3].str());

            if ((Utf8String::IsNullOrEmpty(sign.c_str()) && inputString.StartsWith(majorNumber.c_str()) ||
                !Utf8String::IsNullOrEmpty(sign.c_str()) && inputString.StartsWith(sign.c_str())) &&
                inputString.EndsWith(minorNumber.c_str()))
                {
                Utf8String numberWithSign(sign.c_str());
                numberWithSign.append(majorNumber.c_str());
                ng.Grab(numberWithSign.c_str());
                auto firstSegment = FormatParsingSegment(ng);
                m_segs.push_back(firstSegment);

                ng.Grab(minorNumber.c_str());
                auto secondSegment = FormatParsingSegment(ng);
                m_segs.push_back(secondSegment);
                }
            else
                m_problem.UpdateProblemCode(FormatProblemCode::NA_InvalidSyntax);
            }
        else
            m_problem.UpdateProblemCode(FormatProblemCode::NA_InvalidSyntax);

        return;
        }

    while (!ng.IsEndOfLine())
        {
        ng.Grab(m_input, ind);
        if (ng.GetLength() > 0)  // a number is detected
            {
            if (!m_symbs.empty())
                {
                fps = FormatParsingSegment(m_symbs, ind0, m_unit, m_format, resolver);
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
            csp = CursorScanPoint(m_input, ind);
            if (csp.IsSpace())
                {
                if (!m_symbs.empty())
                    {
                    fps = FormatParsingSegment(m_symbs, ind0, m_unit, m_format, resolver);
                    m_segs.push_back(fps);
                    m_symbs.clear();
                    ind = csp.GetIndex();
                    ind0 = ind;
                    }
                while (csp.IsSpace()) { csp.Iterate(m_input); }
                ind = csp.GetIndex();
                ind0 = ind;
                }
            if (csp.IsEndOfLine())
                break;
            m_symbs.push_back(csp);
            ind = csp.GetIndex();
            }
        }

    if (!m_symbs.empty())
        {
        fps = FormatParsingSegment(m_symbs, ind0, m_unit, m_format, resolver);
        m_segs.push_back(fps);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 07/17
//----------------------------------------------------------------------------------------
FormatParsingSet::FormatParsingSet(Utf8CP input, BEU::UnitCP unit, FormatCP format, QuantityFormatting::UnitResolver* resolver)
    {
    Init(input, 0, unit, format, resolver);
    }

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

    return Utf8String(buf);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 07/17
//----------------------------------------------------------------------------------------
bool FormatParsingSet::ValidateParsingFUS(int reqUnitCount, FormatCP format)
    {
    if (0 == reqUnitCount)
        return false;
    if (nullptr == format)
        m_problem.UpdateProblemCode(FormatProblemCode::PS_MissingFUS);
    if (!format->HasComposite())
        m_problem.UpdateProblemCode(FormatProblemCode::PS_MissingCompositeSpec);
    if(reqUnitCount != format->GetCompositeUnitCount())
        m_problem.UpdateProblemCode(FormatProblemCode::PS_MismatchingFUS);
    if (m_problem.IsProblem())
        return false;

    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 07/17
//----------------------------------------------------------------------------------------
BEU::Quantity FormatParsingSet::GetQuantity(FormatProblemCode* probCode, FormatCP format)
    {
    BEU::Quantity qty = BEU::Quantity();
    BEU::Quantity tmp = BEU::Quantity();
    Utf8String sig = GetSignature(false);
    // only a limited number of signatures will be recognized in this particular context
    // reduced version: NN, NU, NFU, NUNU, NUNFU NUNUNU NUNUNFU
    //        3 FT - NU
    //      1/3 FT - FU
    //  50+00.1    - NN (station)
    BEU::UnitCP majP, midP;
    
    BEU::UnitCP inputUnit;
    
    if (nullptr != format && nullptr != format->GetCompositeMajorUnit())
        inputUnit = format->GetCompositeMajorUnit();
    else
        inputUnit = m_unit;

    double sign = 1.0;

    m_problem.Reset();
   
    Formatting::FormatSpecialCodes cod = Formatting::FormatConstant::ParsingPatternCode(sig.c_str());
    switch (cod)
        {
        case Formatting::FormatSpecialCodes::SignatureN:
        case Formatting::FormatSpecialCodes::SignatureF:
            if (nullptr == inputUnit)
                m_problem.UpdateProblemCode(FormatProblemCode::UnknownUnitName);
            qty = BEU::Quantity(m_segs[0].GetReal(), *inputUnit);
            break;
        case Formatting::FormatSpecialCodes::SignatureNF:
            if (nullptr == inputUnit)
                m_problem.UpdateProblemCode(FormatProblemCode::UnknownUnitName);
            sign = m_segs[0].GetSign();
            qty = BEU::Quantity(m_segs[0].GetAbsReal() + m_segs[1].GetAbsReal(), *inputUnit);
            qty.Scale(sign);
            break;
        case Formatting::FormatSpecialCodes::SignatureNN:
            if (format->GetPresentationType() == PresentationType::Station)
                qty = BEU::Quantity(m_segs[0].GetReal() * std::pow(10, format->GetNumericSpec()->GetStationOffsetSize()) + m_segs[1].GetReal(), *inputUnit);
            else
                m_problem.UpdateProblemCode(FormatProblemCode::QT_InvalidSyntax);
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
        default:
            qty = FormatParsingSet::ComposeColonizedQuantity(cod, format);
            break;
        }

    if (!m_problem.IsProblem() && nullptr != inputUnit)
        qty = qty.ConvertTo(inputUnit);
    if (nullptr != probCode)
        *probCode = m_problem.GetProblemCode();
    return qty;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
BEU::Quantity  FormatParsingSet::ComposeColonizedQuantity(Formatting::FormatSpecialCodes cod, FormatCP format)
    {
    BEU::Quantity qty = BEU::Quantity();
    if (m_problem.IsProblem())
        return qty;

    BEU::UnitCP majP, midP, minP;
    BEU::Quantity tmp = BEU::Quantity();
    double sign = 1.0;

    switch (cod)
        {
        case Formatting::FormatSpecialCodes::SignatureNCNCN:
            if (ValidateParsingFUS(3, format))
                {
                sign = m_segs[0].GetSign();
                majP = format->GetCompositeMajorUnit();
                midP = format->GetCompositeMiddleUnit();
                minP = format->GetCompositeMinorUnit();
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
            if (ValidateParsingFUS(3, format))
                {
                sign = m_segs[0].GetSign();
                majP = format->GetCompositeMajorUnit();
                minP = format->GetCompositeMinorUnit();
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
            if (ValidateParsingFUS(3, format))
                {
                sign = m_segs[0].GetSign();
                majP = format->GetCompositeMajorUnit();
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
            if (ValidateParsingFUS(3, format))
                {
                midP = format->GetCompositeMiddleUnit();
                minP = format->GetCompositeMinorUnit();
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
            if (ValidateParsingFUS(3, format))
                {
                midP = format->GetCompositeMiddleUnit();
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
            if (ValidateParsingFUS(3, format))
                {
                minP = format->GetCompositeMinorUnit();
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
            if (ValidateParsingFUS(3, format))
                {
                sign = m_segs[0].GetSign();
                majP = format->GetCompositeMajorUnit();
                midP = format->GetCompositeMiddleUnit();
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
            if (ValidateParsingFUS(2, format))
                {
                sign = m_segs[0].GetSign();
                majP = format->GetCompositeMajorUnit();
                midP = format->GetCompositeMiddleUnit();
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
            if (ValidateParsingFUS(2, format))
                {
                sign = m_segs[0].GetSign();
                majP = format->GetCompositeMajorUnit();
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
            if (ValidateParsingFUS(2, format))
                {
                midP = format->GetCompositeMiddleUnit();
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
