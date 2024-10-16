/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <UnitsPCH.h>
#include <Formatting/FormattingApi.h>
#include "../../PrivateAPI/Formatting/FormattingParsing.h"
#include "../../PrivateAPI/Formatting/NumericFormatUtils.h"
#include <regex>
#include <string>
#include <sstream>

#define MAX_PARSING_DIGITS 16

BEGIN_BENTLEY_FORMATTING_NAMESPACE

//===================================================
// FormattingScannerCursor
//===================================================

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
// the caller is responsible for keeping the index inside the allowable range
//----------------------------------------------------------------------------------------
CursorScanPoint::CursorScanPoint(Utf8CP input, size_t indx)
    {
    m_indx = indx;
    ProcessNext(input);
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
void CursorScanPoint::Iterate(Utf8CP input)
    {
    ProcessNext(input);
    }

//----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//----------------------------------------------------------------------------------------
// The following method makes a few assumptions about the ordering of the different pieces of the input string when extacting the number.
//    1.  Skips all whitespace at the beginning of the string
//    2.  Checks for a sign character.
//    3.  Iterates through any digits until a non-digit char is hit
//    4.  Checks for a "bar" char, signifies a fractional number.
//       a)
//    5.  Checks for decimal or thousands separator
//       a)  Iterates through any further digits after separator
//    6.  Checks for exponent
//  TODO: Finish This
size_t NumberGrabber::Grab(Utf8CP input, size_t start, Utf8Char const decimalSep, Utf8Char const thousandSep)
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

    m_problem.Reset();
    m_type = ParsingSegmentType::NotNumber; // Reset this as a non-number until determined otherwise.
    int64_t ival = 0; // integer value
    int64_t fval = 0; // floating point value
    int64_t eval = 0; // exponent value
    int64_t denom = 0; // denominator value
    double dval = 0.0;
    int iCount = 0; // integer digit count
    int fCount = 0; // floating point digit count - number of numbers after decimal separator
    int eCount = 0; // exponent digit count
    int dCount = 0;  // denominator digit count
    int sign = 0; // 0 or 1 means a positive number, -1 means the number is negative
    bool point = false; // decimal separator exists
    bool  expMark = false; // exponent exists

    int expSign = 0; // exponent sign
    bool num = false; // set to true if the input has any numbers - determined by checking if iCount and fCount are > 0.

    Utf8Char c;

    while (csp.IsSpace()) { m_next++; csp.Iterate(m_input); }
    if (csp.IsSign())  // the sign character can be expected at the start
        {
        c = csp.GetAscii();
        sign = (c == FormatConstant::SignSymbol()) ? -1 : 1; // Only checks for the negative sign because a positive sign doesn't change anything so it is skipped.
        ++m_next;
        csp.Iterate(m_input);
        }
    while (csp.IsDigit())    // if there any digits - they will be accumulated in the int part
        {
        if (iCount + 1 >= MAX_PARSING_DIGITS)
            {
            m_problem.UpdateProblemCode(FormatProblemCode::TooManyDigits);
            return m_next - m_start;
            }

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
                if (dCount + 1 >= MAX_PARSING_DIGITS)
                    {
                    m_problem.UpdateProblemCode(FormatProblemCode::TooManyDigits);
                    return m_next - m_start;
                    }

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
                m_ival = (int64_t)m_dval;    // regardless of what we collected there it should be refreshed
                m_type = ParsingSegmentType::Fraction;
                }
            else
                m_next = m_start;
            }
        else
           m_next = m_start;    // the sequence cannot be recognized
        return m_next - m_start;
        }


    while (csp.IsSeparator(decimalSep, thousandSep)) // we can expect a decimal point which can actually come by itself
        {                   // or can be followed by more digits
        if (csp.GetAscii() == decimalSep)
            {
            point = true;
            ++m_next;
            csp.Iterate(m_input);
            while (csp.IsDigit())    // if there any digits - they will be accumulated in the fraction
                {
                if (fCount + 1 >= MAX_PARSING_DIGITS)
                    {
                    m_problem.UpdateProblemCode(FormatProblemCode::TooManyDigits);
                    return m_next - m_start;
                    }

                int d = (int) (csp.GetAscii() - '0');
                fval = fval * 10 + d;
                fCount++;
                ++m_next;
                csp.Iterate(m_input);
                }
            if (csp.IsSeparator(decimalSep, thousandSep))
                {
                if (iCount + fCount >= MAX_PARSING_DIGITS)
                    {
                    m_problem.UpdateProblemCode(FormatProblemCode::TooManyDigits);
                    return m_next - m_start;
                    }

                ival = ival * (int64_t)(pow(10, fCount)) + fval;
                iCount = iCount + fCount;
                fval = 0;
                fCount = 0;
                }
            }
        else
            {
            ++m_next;
            csp.Iterate(m_input);
            while (csp.IsDigit())    // if there any digits - they will be accumulated in the int part
                {
                if (iCount + 1 >= MAX_PARSING_DIGITS)
                    {
                    m_problem.UpdateProblemCode(FormatProblemCode::TooManyDigits);
                    return m_next - m_start;
                    }

                int d = (int) (csp.GetAscii() - '0');
                ival = ival * 10 + d;
                iCount++;
                ++m_next;
                csp.Iterate(m_input);
                }
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
            if (eCount + 1 >= MAX_PARSING_DIGITS)
                {
                m_problem.UpdateProblemCode(FormatProblemCode::TooManyDigits);
                return m_next - m_start;
                }

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
            double f2 = 1.0;
            double mult = (expSign < 0) ? 0.1 : 10.0;

            for (int i = 0; i < eval; ++i) { f2 *= mult; }
            dval *= f2;
            }
        m_dval = (sign < 0)? -dval : dval;
        m_ival = (int64_t)dval;    // regardless of what we collected there it should be refreshed
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
// @bsimethod
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
// @bsimethod
//----------------------------------------------------------------------------------------
FormatParsingSegment::FormatParsingSegment(NumberGrabberCR ng) : FormatParsingSegment()
    {
    m_start = ng.GetStartIndex();

    if (ng.GetType() != ParsingSegmentType::NotNumber)
        {
        m_start = ng.GetStartIndex();
        m_vect.clear();
        m_type = ng.GetType();
        m_ival = ng.GetInteger();
        m_dval = ng.GetReal();
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
FormatParsingSegment::FormatParsingSegment(bvector<CursorScanPoint> vect, size_t s, BEU::UnitCP refUnit, FormatCP format, QuantityFormatting::UnitResolver* resolver)
    {
    Init(s);
    unsigned char* ptr;

    for (auto vp = vect.begin(); vp != vect.end(); ++vp)
        {
        m_vect.push_back(*vp);
        for (ptr = vp->GetBytes(); *ptr != '\0'; ++ptr)
            {
            m_name += *ptr;
            }
        }
    if (!m_name.empty()) // something in the buffer
        {
        if (m_name.StartsWith("_")) // special case of prpending the unit name by underscore
            m_name = m_name.substr(1);

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
                                if (comp->GetMiddleLabel().EqualsI(m_name))
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
// @bsimethod
//----------------------------------------------------------------------------------------
void FormatParsingSet::SegmentInput(Utf8CP input, size_t start)
    {
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

    Utf8Char decSep = '.';
    Utf8Char thousSep = ',';
    if (m_format != nullptr && m_format->GetNumericSpec() != nullptr)
        {
        decSep = m_format->GetNumericSpec()->GetDecimalSeparator();
        thousSep = m_format->GetNumericSpec()->GetThousandSeparator();
        }

    while (!ng.IsEndOfLine())
        {
        ng.Grab(input, ind, decSep, thousSep);
        if (ng.HasProblem())
            {
            m_problem.UpdateProblemCode(ng.GetProblemCode());
            break;
            }

        if (ng.GetLength() > 0)  // a number is detected
            {
            if (!m_symbs.empty())
                {
                fps = FormatParsingSegment(m_symbs, ind0, m_unit, m_format, m_resolver);
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
            csp = CursorScanPoint(input, ind);
            if (csp.IsSpace())
                {
                if (!m_symbs.empty())
                    {
                    fps = FormatParsingSegment(m_symbs, ind0, m_unit, m_format, m_resolver);
                    m_segs.push_back(fps);
                    m_symbs.clear();
                    ind = csp.GetIndex();
                    ind0 = ind;
                    }
                while (csp.IsSpace()) { csp.Iterate(input); }
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
        fps = FormatParsingSegment(m_symbs, ind0, m_unit, m_format, m_resolver);
        m_segs.push_back(fps);
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
FormatParsingSet::FormatParsingSet(Utf8CP input, BEU::UnitCP unit, FormatCP format, QuantityFormatting::UnitResolver* resolver)
    : m_input(input), m_unit(unit), m_format(format), m_resolver(resolver), m_problem(FormatProblemCode::NoProblems), m_segs()
    { }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
Utf8String FormatParsingSet::GetSignature(bool distinct) //, int* colonCount)
    {
    Utf8String signature("");
    if (m_segs.size() == 0)
        return signature;

    ParsingSegmentType type;

    for (auto fps = m_segs.begin(); fps != m_segs.end(); fps++)
        {
        type = fps->GetType();
        if (type == ParsingSegmentType::Real)
            signature += distinct ? 'R': 'N';
        else if (type == ParsingSegmentType::Integer)
            signature += distinct ? 'I' : 'N';
        else if (type == ParsingSegmentType::Fraction)
            signature += 'F';
        else
            {
            if (fps->IsColon())
                signature += 'C';
            else if (fps->IsDoubleColon())
                signature += "CC";
            else if (fps->IsTripleColon())
                signature += "CCC";
            else if (fps->IsMinusColon())
                signature += "-C";
            else if (fps->IsMinusDoubleColon())
                signature += "-CC";
            else if (fps->IsMinusTripleColon())
                signature += "-CCC";
            else if (nullptr == fps->GetUnit())
                signature += 'W';
            else
                signature += 'U';
            }
        }

    return signature;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
bool FormatParsingSet::ValidateParsingFUS(int reqUnitCount, FormatCP format)
    {
    if (0 == reqUnitCount)
        return false;
    if (nullptr == format)
        {
        m_problem.UpdateProblemCode(FormatProblemCode::PS_MissingFUS);
        return false;
        }
    if (!format->HasComposite())
        m_problem.UpdateProblemCode(FormatProblemCode::PS_MissingCompositeSpec);
    if(reqUnitCount != format->GetCompositeUnitCount())
        m_problem.UpdateProblemCode(FormatProblemCode::PS_MismatchingFUS);
    if (m_problem.IsProblem())
        return false;

    return true;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
BEU::Quantity FormatParsingSet::GetQuantity(FormatProblemCode* probCode, FormatCP format)
    {
    if (probCode != nullptr) {
        *probCode = FormatProblemCode::NoProblems; // Safely dereference and assign
    }
    m_problem.Reset();

    if (format == nullptr)
        {
        m_problem.UpdateProblemCode(FormatProblemCode::PS_MissingFUS);
        return BEU::Quantity(BEU::UnitsProblemCode::UnspecifiedProblem);
        }

    BEU::UnitCP inputUnit;
    if (nullptr != format->GetCompositeMajorUnit())
        inputUnit = format->GetCompositeMajorUnit();
    else
        inputUnit = m_unit;

    _Analysis_assume_(inputUnit != nullptr);
    
    BEU::Quantity qty;
    if (format->GetPresentationType() == PresentationType::Azimuth){
        SegmentInput(m_input.c_str(), 0);
        qty = ParseAzimuthFormat(probCode, format, inputUnit);
    }

    if (format->GetPresentationType() == PresentationType::Bearing){
        // special handling for bearing format - strip off the direction characters
        std::regex directionPattern(R"(^[NSEW]|[NSEW]$)"); // TODO - <Naron>: dont use regex here
        Utf8String input(std::regex_replace(m_input, directionPattern, ""));
        SegmentInput(input.c_str(), 0); 
        qty = ParseBearingFormat(m_input.c_str(), probCode, format, inputUnit);
    }

    if (format->GetPresentationType() == PresentationType::Ratio){
        SegmentInput(m_input.c_str(), 0);
        qty = ParseRatioFormat(m_input.c_str(), probCode, format, inputUnit);
    }

    if (probCode != nullptr && *probCode != FormatProblemCode::NoProblems){
        m_problem.UpdateProblemCode(*probCode);
        return qty;
    }

    if (!qty.IsNullQuantity())
        return qty;

    SegmentInput(m_input.c_str(), 0);

    Utf8String sig = GetSignature(false); 
    // only a limited number of signatures will be recognized in this particular context
    // reduced version: NN, NU, NFU, NUNU, NUNFU NUNUNU NUNUNFU
    //        3 FT - NU
    //      1/3 FT - FU
    //  50+00.1    - NN (station)

    Formatting::FormatSpecialCodes cod = Formatting::FormatConstant::ParsingPatternCode(sig.c_str());
    qty = ParseAndProcessTokens(cod, format, inputUnit);

    if (!m_problem.IsProblem() && nullptr != inputUnit)
        qty = qty.ConvertTo(inputUnit);
    if (nullptr != probCode)
        *probCode = m_problem.GetProblemCode();
    return qty;
    }

//----------------------------------------------------------------------------------------
// @bsimethod
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

BEU::Quantity FormatParsingSet::ParseAndProcessTokens(Formatting::FormatSpecialCodes cod, FormatCP format, BEU::UnitCP inputUnit)
    {
    BEU::Quantity qty = BEU::Quantity();
    BEU::Quantity tmp = BEU::Quantity();
    BEU::UnitCP majP, midP;
    double sign = 1.0;

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
            if (nullptr == format)
                {
                m_problem.UpdateProblemCode(FormatProblemCode::PS_MissingFUS);
                }
            else
                {
                if (format->GetPresentationType() == PresentationType::Station)
                    qty = BEU::Quantity(m_segs[0].GetReal() * std::pow(10, format->GetNumericSpec()->GetStationOffsetSize()) + m_segs[1].GetReal(), *inputUnit);
                else
                    m_problem.UpdateProblemCode(FormatProblemCode::QT_InvalidSyntax);
                }
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
    return qty;
    }

BEU::Quantity FormatParsingSet::ParseAzimuthFormat(FormatProblemCode* probCode, FormatCP format, BEU::UnitCP inputUnit)
    {
    BEU::Quantity converted = BEU::Quantity();
    
    Utf8String sig = GetSignature(false);
    Formatting::FormatSpecialCodes cod = Formatting::FormatConstant::ParsingPatternCode(sig.c_str());
    BEU::Quantity qty = ParseAndProcessTokens(cod, format, inputUnit);

    if (m_problem.IsProblem())
        {
        //TODO - this is redundant here as m_problem will get updated in GetQuantity(), its here now cuz we have 2 different ways tracking problem code
        if (probCode != nullptr) {
            *probCode = m_problem.GetProblemCode();
        }
        return converted;
        }

    double azimuthBase = 0.0;
    double magnitude = qty.GetMagnitude();

    double revolution;
    if (format->TryGetRevolution(*inputUnit, revolution) != BentleyStatus::SUCCESS)
        {
        if (probCode != nullptr) {
            *probCode = FormatProblemCode::NFS_MissingUnit;
        }
        return BEU::Quantity(BEU::UnitsProblemCode::UnspecifiedProblem);
        }

    if (Format::NormalizeAngle(qty, "azimuth", revolution) != BentleyStatus::SUCCESS)
        {
        if (probCode != nullptr) {
            *probCode = FormatProblemCode::CVS_InvalidMajorUnit;
        }
        return converted;
        }

    NumericFormatSpecCP fmtP = format->GetNumericSpec();
    if (fmtP->HasAzimuthBase())
        {
        azimuthBase = fmtP->GetAzimuthBase();
        auto azimuthBaseUnit = fmtP->GetAzimuthBaseUnit();
        if (azimuthBaseUnit != nullptr)
            {
            BEU::Quantity azimuthBaseQuantity(azimuthBase, *azimuthBaseUnit);
            BEU::Quantity converted = azimuthBaseQuantity.ConvertTo(qty.GetUnit());
            if (converted.IsValid())
                azimuthBase = converted.GetMagnitude();
            else
                {
                if (probCode != nullptr) {
                    *probCode = FormatProblemCode::QT_ConversionFailed;
                }
                return converted;
                }
            }
        else
            {
            if (probCode != nullptr) {
                *probCode = FormatProblemCode::QT_NoValueOrUnitFound;
            }
            return converted;
            }
        }

    if (std::fmod(azimuthBase, revolution) == 0.0 &&  !fmtP->GetAzimuthCounterClockwise())
        {
        converted = qty.ConvertTo(m_unit); 
        return converted;
        }

    if (fmtP->GetAzimuthCounterClockwise())
        magnitude = azimuthBase - magnitude;
    else
        magnitude = azimuthBase + magnitude;

    // normalize the angle
    magnitude = std::fmod(magnitude, revolution);
    while(magnitude < 0)
        magnitude += revolution;
    
    while(magnitude > revolution)
        magnitude -= revolution;
    
    qty = BEU::Quantity(magnitude, *inputUnit);
    converted = qty.ConvertTo(m_unit);

    if (!converted.IsValid())
        {
        if (probCode != nullptr) {
            *probCode = FormatProblemCode::QT_ConversionFailed;
        }
        }

    return converted;
    }

BEU::Quantity FormatParsingSet::ParseBearingFormat(Utf8CP input, FormatProblemCode* probCode, FormatCP format, BEU::UnitCP inputUnit)
    {
    BEU::Quantity converted = BEU::Quantity();

    const std::string North = "N";
    const std::string South = "S";
    const std::string East = "E";
    const std::string West = "W";

    std::string matchedPrefix;
    std::string matchedSuffix;
    std::string inString(input);

    if (inString.empty()){
        if (probCode != nullptr) {
            *probCode = FormatProblemCode::QT_NoValueOrUnitFound;
        }
        return converted;
    }

    // check if input stirng begins with North or South, and strip it off
    if(inString.compare(0, North.size(), North) == 0){
        inString = inString.substr(North.length());
        matchedPrefix = North;
    } else if(inString.compare(0, South.size(), South) == 0){
        inString = inString.substr(South.length());
        matchedPrefix = South;
    } 

    // check if input stirng ends with East or West, and strip it off
    if (inString.length() >= East.length() &&
        inString.compare(inString.length() - East.length(), East.length(), East) == 0){
        inString = inString.substr(0, inString.length() - East.length());
        matchedSuffix = East;
    } else if (inString.length() >= West.length() &&
        inString.compare(inString.length() - West.length(), West.length(), West) == 0){
        inString = inString.substr(0, inString.length() - West.length());
        matchedSuffix = West;
    }

    // if either prefix and suffix are missing, return error
    if (matchedPrefix.empty() || matchedSuffix.empty()){
        if (probCode != nullptr) {
            *probCode = FormatProblemCode::QT_BearingPrefixOrSuffixMissing;
        }
        return converted;
    }

    Utf8String sig = GetSignature(false);
    Formatting::FormatSpecialCodes cod = Formatting::FormatConstant::ParsingPatternCode(sig.c_str());
    BEU::Quantity qty = ParseAndProcessTokens(cod, format, inputUnit);

    if (m_problem.IsProblem()){
        //TODO - this is redundant here as m_problem will get updated in GetQuantity(), its here now cuz we have 2 different ways tracking problem code
        if (probCode != nullptr) {
            *probCode = m_problem.GetProblemCode();
        }
        return converted;
    }

    double revolution;
    if (format->TryGetRevolution(*inputUnit, revolution) != BentleyStatus::SUCCESS)
        {
        if (probCode != nullptr) {
            *probCode = FormatProblemCode::NFS_MissingUnit;
        }
        return BEU::Quantity(BEU::UnitsProblemCode::UnspecifiedProblem);
        }

    if (Format::NormalizeAngle(qty, "bearing", revolution) != BentleyStatus::SUCCESS){
        if (probCode != nullptr) {
            *probCode = FormatProblemCode::CVS_InvalidMajorUnit;
        }
        return converted;
    }
    double quarterRevolution = revolution / 4;
    double magnitude = qty.GetMagnitude();

    if (matchedPrefix == North){
        if (matchedSuffix == West){
            magnitude = revolution - magnitude;
        }
    } else if (matchedPrefix == South){
        if (matchedSuffix == West){
            magnitude = (2 * quarterRevolution) + magnitude;
        } else if (matchedSuffix == East){
            magnitude = (2 * quarterRevolution) - magnitude;
        }
    }

    qty = BEU::Quantity(magnitude, *inputUnit);
    
    converted = qty.ConvertTo(m_unit);
    if (!converted.IsValid()){
        if (probCode != nullptr) {
            *probCode = FormatProblemCode::QT_ConversionFailed;
        }
    }

    return converted;
    }

BEU::Quantity FormatParsingSet::ParseRatioFormat(Utf8CP input, FormatProblemCode* probCode, FormatCP format, BEU::UnitCP inputUnit)
    {
    BEU::Quantity converted = BEU::Quantity();

    std::string inString(input);
    std::istringstream iss(inString);
    std::string part;

    std::vector<std::string> parts;
    while (std::getline(iss, part, ':'))
        parts.push_back(part);

    if (parts.size() == 0)
        {
        if (probCode != nullptr) {
            *probCode = FormatProblemCode::QT_NoValueOrUnitFound;
        }
        return converted;
        }

    if (parts.size() > 2)
        {
        if (probCode != nullptr) {
            *probCode = FormatProblemCode::QT_InvalidRatioArgument;
        }
        return converted;
        }

    // for single number input. e.g. input of "30" will be converted to "30:1"
    if (parts.size() == 1)
        parts.push_back("1");

    double numerator;
    double denominator;

    try
        {
        numerator = std::stod(parts[0]);
        denominator = std::stod(parts[1]);
        }
    catch (std::invalid_argument)
        {
        if (probCode != nullptr) {
            *probCode = FormatProblemCode::QT_InvalidRatioArgument;
        }
        return converted;
        }
    catch (std::out_of_range)
        {
        if (probCode != nullptr) {
            *probCode = FormatProblemCode::QT_ValueOutOfRange;
        }
        return converted;
        }

    if (m_unit == nullptr)
        {
        if (probCode != nullptr) {
            *probCode = FormatProblemCode::QT_NoValueOrUnitFound;
        }
        return converted;
        }

    BEU::Quantity zeroQty = BEU::Quantity(0, *inputUnit); // temp qty to check invertingZero error on input "1:0" with invert units
    if (denominator == 0)
        {
        converted = zeroQty.ConvertTo(m_unit);
        if (converted.GetProblemCode() == BEU::UnitsProblemCode::InvertingZero)
            return BEU::Quantity(0, *m_unit);

        // for input of "N:0" without reversed unit
        if (probCode != nullptr) {
            *probCode = FormatProblemCode::QT_MathematicOperationFoundButIsNotAllowed;
        }
        return converted;
        }

    BEU::Quantity qty = BEU::Quantity(numerator / denominator, *inputUnit);
    converted = qty.ConvertTo(m_unit);

    // for input of "0:N" with reversed unit
    if (converted.GetProblemCode() == BEU::UnitsProblemCode::InvertingZero)
        {
        if (probCode != nullptr) {
            *probCode = FormatProblemCode::QT_MathematicOperationFoundButIsNotAllowed;
        }
        return converted;
        }

    return converted;
    }


END_BENTLEY_FORMATTING_NAMESPACE
