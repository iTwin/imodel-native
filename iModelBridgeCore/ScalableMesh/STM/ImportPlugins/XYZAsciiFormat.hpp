/*----------------------------------------------------------------------+
|
|   $Source: STM/ImportPlugins/XYZAsciiFormat.hpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool XYZFormat::IsWS (WChar c)
    {
    // TDORAY: Wouldn't it be faster if we used an index table?
    return L' ' == c || L'\t' == c || L'\r' == c;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool XYZFormat::IsNum (WChar c)
    {
    // TDORAY: Wouldn't it be faster if we used an index table?
    return (c >= L'0' && c <= L'9') || c == L'-' || c == L'+' || c == L'.' || c == L'E' || c == L'e';
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct XYZFormat::IsWhiteSpace : unary_function<WChar, bool>
    {
    bool operator() (WChar c) const { return IsWS(c); }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct XYZFormat::IsWhiteSpaceOr : unary_function<WChar, bool>
    {
    const WChar m_other;
    explicit IsWhiteSpaceOr (WChar other) : m_other(other) {}

    // TDORAY: Wouldn't it be faster if we used an index table?
    bool operator() (WChar c) { return L' ' == c || L'\t' == c || m_other == c || L'\r' == c; }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct XYZFormat::IsNumeric : unary_function<WChar, bool>
    {
    bool operator() (WChar c) const { return IsNum(c); }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct XYZFormat::IsNewLineOrComment : unary_function<WChar, bool>
    {
    bool operator() (WChar c) const { return c == L'\n' || c == L'"'; }
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline WChar XYZFormat::FindDelimiterIn (FileRange::const_iterator delimZoneBegin, FileRange::const_iterator delimZoneEnd)
    {
    const FileRange::const_iterator found = std::find_if(delimZoneBegin, delimZoneEnd, not1(IsWhiteSpace()));
    return (delimZoneBegin == delimZoneEnd) ? INVALID_DELIMITER :
                (delimZoneEnd == found) ? *delimZoneBegin :
                    (1 < std::distance(found, delimZoneEnd) && 1 < std::count_if(found, delimZoneEnd, not1(IsWhiteSpace()))) ?
                        INVALID_DELIMITER :
                        *found;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline uint32_t XYZFormat::SplitLine (FileRange fieldsRanges[], const FileRange lineRange)
    {
    uint32_t fieldCount = 0;
    FileRange fieldRange(find_if(lineRange.begin, lineRange.end, IsNumeric()), lineRange.end);

    fieldRange.end = find_if(fieldRange.begin, lineRange.end, not1(IsNumeric()));

    while (lineRange.end != fieldRange.begin && fieldCount < MAX_FIELD_COUNT)
        {
        fieldsRanges[fieldCount] = fieldRange;
        ++fieldCount;

        fieldRange.begin = find_if(fieldRange.end, lineRange.end, IsNumeric());
        fieldRange.end = find_if(fieldRange.begin, lineRange.end, not1(IsNumeric()));
        };

    return fieldCount;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool XYZFormat::IsCommentLine (const FileRange& lineRange)
    {
    return (L'"' == *lineRange.end) && (lineRange.end == find_if(lineRange.begin, lineRange.end, not1(IsWhiteSpace())));
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool XYZFormat::IsCommentOrEmptyLine (const FileRange& lineRange)
    {
    return lineRange.begin == lineRange.end ||
           (IsWS(*lineRange.begin) && (lineRange.end == find_if(lineRange.begin + 1, lineRange.end, not1(IsWhiteSpace()))));
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool XYZFormat::GetNextLineRange (FileRange& nextLineRange, FileRange::const_iterator rangeBegin, FileRange::const_iterator rangeEnd)
    {
    FileRange::const_iterator nextNewLine(find(rangeBegin, rangeEnd, L'\n'));

    nextLineRange.begin = (rangeEnd == nextNewLine) ? nextNewLine : ++nextNewLine;
    nextLineRange.end = std::find_if(nextLineRange.begin, rangeEnd, IsNewLineOrComment());
    return rangeEnd != nextLineRange.end;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool XYZFormat::GetFirstLineRange (FileRange& firstLineRange, FileRange::const_iterator rangeBegin, FileRange::const_iterator rangeEnd)
    {
    firstLineRange.begin = rangeBegin;
    firstLineRange.end = std::find_if(firstLineRange.begin, rangeEnd, IsNewLineOrComment());
    return rangeEnd != firstLineRange.end;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline WChar XYZFormat::GetDelimiterFor (FileRange::const_iterator delimZoneBegin, FileRange::const_iterator delimZoneEnd)
    {
    const WChar delimBetween =
        (1 == std::distance(delimZoneBegin, delimZoneEnd)) ?
            (*delimZoneBegin) :
            FindDelimiterIn(delimZoneBegin, delimZoneEnd);
    return delimBetween;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline WChar XYZFormat::GetDelimiterFor (const FileRange fields[], uint32_t fieldsCount)
    {
    struct DelimiterBetweenEqualTo : std::binary_function<FileRange, FileRange, bool>
        {
        const WChar m_delim;
        explicit DelimiterBetweenEqualTo (WChar delim) : m_delim(delim) {}

        bool operator () (const FileRange& lhs, const FileRange& rhs) const
            {
            return m_delim == GetDelimiterFor(lhs.end, rhs.begin);
            }
        };
    if (1 >= fieldsCount)
        return INVALID_DELIMITER;

    const WChar firstDelimiter = GetDelimiterFor(fields[0].end, fields[1].begin);

    const FileRange* const fieldsEnd = fields + fieldsCount;
    return (fieldsEnd == std::adjacent_find(fields + 1, fieldsEnd, not2(DelimiterBetweenEqualTo(firstDelimiter)))) ?
        firstDelimiter : INVALID_DELIMITER;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename OutIt, typename IsWSPred>
uint32_t XYZFormat::ReadLine   (FILE*       file,
                            OutIt&      outBeginIt,
                            OutIt       outEndIt,
                            IsWSPred    isWSPred,
                            WChar        newDelimiter)
    {
    bool inWS = true;
    uint32_t fieldCount = 0;    
    OutIt outIt(outBeginIt);

    // Copy until new line or comment
    WChar c = NEW_LINE;
    while (outEndIt != outIt && feof (file) == 0)
        {
        char tempC = getc(file);
        
        mbstowcs(&c, &tempC, 1);            
        
        if (c == NEW_LINE || c == COMMENT || tempC == EOF)
            break;

        if (isWSPred(c))
            {
            if (!inWS)
                {
                inWS = true;
                *outIt++ = newDelimiter;
                }
            }
        else
            {
            *outIt++ = c;
            if (inWS)
                {
                inWS = false;
                ++fieldCount;
                }
            }
        }


    // Make sure there is at least one delimiter after last field
    if (!inWS && outEndIt != outIt)
        *outIt++ = newDelimiter;

    // Skip commented section if required
    while (c != NEW_LINE && feof (file) == 0)
        {
        char tempC = getc(file);        
        mbstowcs(&c, &tempC, 1);             
        }
    
    outBeginIt = outIt;
    return fieldCount;
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                               Raymond.Gauthier    02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename OutIt>
bool XYZFormat::ReadFields (const WChar*     lineBegin,
                            const WChar*     lineEnd,
                            OutIt           fieldOutIt,
                            OutIt           fieldOutEndIt)
    {
    const WChar* fieldStrIt = lineBegin;
    WChar* nextFieldStrIt; // Intentionally left uninitialized as initialized by wcstod

    while (fieldOutEndIt != fieldOutIt)
        {
        *fieldOutIt = wcstod(fieldStrIt, &nextFieldStrIt);
        if (nextFieldStrIt == fieldStrIt ||
            lineEnd <= nextFieldStrIt)
            break; // Incorrect field format

        fieldStrIt = nextFieldStrIt;
        ++fieldStrIt;
        ++fieldOutIt;
        }

    return fieldOutEndIt == fieldOutIt;
    }