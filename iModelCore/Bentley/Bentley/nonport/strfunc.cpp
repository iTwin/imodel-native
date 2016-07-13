/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/strfunc.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <Bentley/Bentley.h>
#include    <Bentley/BeAssert.h>
#include    <Bentley/BeDebugLog.h>
#include    <Bentley/PTypesU.h>
#include    <Bentley/BeStringUtilities.h>
#include    <Bentley/WString.h>
#include    <Bentley/ScopedArray.h>
#include    <stdio.h>
#if !defined (__APPLE__)
#include    <malloc.h>
#endif
#include    <string.h>
#include    <stdlib.h>
#include    <ctype.h>
#include    <type_traits>
#include    "strfunc.h"

USING_NAMESPACE_BENTLEY

#ifndef BENTLEY_CPP_MISSING_WCHAR_SUPPORT
    #error strfunc should be used only when the CRT lacks wchar_t support
#endif

#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
extern void utf8ToWChar (wchar_t* outStr, size_t outStrCount, char const* inStr, size_t inStrCount);
#endif

#define     SCANF_ABORT             -1
#define     SCANF_FATAL             -2

/*  FLAGS */
#define     FLAGS_HAVE_MINUS        1
#define     FLAGS_HAVE_POUND        2
#define     FLAGS_HAVE_SHORT        4
#define     FLAGS_HAVE_LONG_LONG    8
#define     FLAGS_HAVE_LONG         16

/*
    As of 21-Mar-2014...
    
    Number one guiding principle: do NOT allow mixed encodings. If the format string type is char*, only char* format specifiers are allowed; vice versa for wchar_t* format strings.
    Ideally, char* strings should only include %s specifiers, and wchar_t* strings should only include %ls specifiers.
    %hs will continue to be accepted for the time being, but this is a Microsoft-only extension, and it causes overhead in our pre-processor to strip it off on non-Win32.
    Additionally, the C++ standard says %s always means char*, even in a whcar_t* format string; we now honor this interpretation vs. Microsoft's approach to swap the meaning depending on the function call.

    -----------------------------------------------------------------------------
    |Format String Type |%s                 |%ls                |%hs            |
    |-------------------|-------------------|-------------------|---------------|
    |char*              |-Accepted          |-Not accepted      |-Accepted      |
    |                   |-Means char*       |                   |-Means char*   |
    |-------------------|-------------------|-------------------|---------------|
    |wchar_t*           |-Not accepted      |-Accepted          |-Not accepted  |
    |                   |                   |-Means whcar_t*    |               |
    -----------------------------------------------------------------------------
*/

#if defined (_WIN32) && !defined (NDEBUG)
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Dan.East                        12/12
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void reportBadStringType (Utf8CP underlyingType, Utf8CP attemptedType)
        {
        Utf8String message = Utf8PrintfString("Cannot use %s with a %s format string; you cannot mix encodings. See top of strfunc.cpp for details.", attemptedType, underlyingType);
        BeDebugLogFunctions::PerformBeDebugLog(message.c_str(), __FILE__, __LINE__);
        BeAssert(false);
        }

    #define REPORT_BAD_STRING_TYPE(underlyingType,attemptedType) reportBadStringType(underlyingType,attemptedType)
#else
    #define REPORT_BAD_STRING_TYPE(underlyingType,attemptedType) ((void)0)
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct Formatter
{
size_t          iTypeArrSize;   /* the size of the memory pointed by pcArgType*/
size_t          iArrayMax;      /* the last argument specified by format string*/
void**          ppArguments;    /* the start addresses of each argument on the stack*/
CharP           pcArgType;      /* the type corresponse to each argument on the stack*/
CharCP          m_currFormat;
CharCP          m_fullFormat;
PrintSink&      m_sink;
bool            m_naturalStringIsWchar;

Formatter (PrintSink& sink, CharCP fmt, bool w) : m_sink(sink), m_currFormat(fmt), m_fullFormat(fmt), m_naturalStringIsWchar(w), iTypeArrSize(0), iArrayMax(0), ppArguments(NULL), pcArgType(NULL) {;}

void babort (char const* msg)
    {
    m_sink.OnError (-1);
    //BeAssert (false);
    BeDebugLog ("strfunc.cpp - Formatter error");
    BeDebugLog (msg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Min.Chu         05/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void *strGetMem
(
size_t iSize
)
    {
    return malloc (iSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    01/90
+---------------+---------------+---------------+---------------+---------------+------*/
void DoFormat (va_list args)
    {
    int         minWidth, c;
    int         flags = 0;
    int         haveNewline = 0;
    Ptypes_u    destP;
    char        build [1000];
    char        buf [100];  // used only for numbers and single characters
    // *** WIP replace build, pbuild with bastring
    char        *pbuild;
    int         firstTime = TRUE;
    int         n;

    while ((c = *m_currFormat++) != 0)
        {
        if (c != '%')
            {
            m_sink.PutChar((char)c);
            if (c == '\n')
                haveNewline = 1;
            }
        else
            {
            if (*m_currFormat == '%')
                {
                m_sink.PutChar(*m_currFormat++);
                continue;
                }
            build[ 0 ] = '%';
            pbuild = build + 1;

            flags = 0;
            /*  Collect the flags */
            for (; (c = *m_currFormat) != '\0'; *pbuild++ = *m_currFormat++)
                {
                if (c == '-')
                    flags |= FLAGS_HAVE_MINUS;
                else if (c == '#')
                    flags |= FLAGS_HAVE_POUND;
                else if ((c != '+') && (c != ' ') && (c != '0'))
                    break;
                }

            /*  Collect minimum field width */
            if (c == '*')
                {
                sprintf (pbuild, "%d", minWidth = va_arg(args,int));
                pbuild += strlen (pbuild);;
                m_currFormat++;
                firstTime = FALSE;
                }
            else
                {
                for (minWidth = 0, c = *m_currFormat; isdigit (c); c = *(++m_currFormat))
                    {
                    minWidth = 10 * minWidth + c - '0';
                    *pbuild++ = (char)c;
                    }
                }

            if (*m_currFormat == '.')
                {
                *pbuild++ = *m_currFormat++;
                if (*m_currFormat == '*')
                    {
                    sprintf (pbuild, "%d", va_arg(args,int));
                    pbuild += strlen (pbuild);
                    m_currFormat++;
                    firstTime = FALSE;
                    }
                else
                    {
                    while (isdigit (*m_currFormat))
                        *pbuild++ = *m_currFormat++;
                    }
                }

            /*  Get the conversion modifiers */
            if ((c = *m_currFormat) == 'h')
                {
                m_currFormat++;
                // Inlude the 'h' (short) modifier when formatting a number, not a string. That is because %hs is not supported on Android. When formatting a string, we always convert strings to UTF-8 anyway. So, %s is unambiguous.
                if (*m_currFormat != 's')
                    *pbuild++ = 'h';
                flags |= FLAGS_HAVE_SHORT;
                }
            else if (c == 'I' && *(m_currFormat + 1) == '6' && *(m_currFormat + 2) == '4')
                {
                REPORT_BAD_STRING_TYPE ("long long", "I64");
                *pbuild++ = 'l';
                *pbuild++ = 'l';
                m_currFormat += 3;
                flags |= FLAGS_HAVE_LONG_LONG;
                }
            else if ((c == 'l') || (c == 'L'))
                {
                m_currFormat++;
                flags |= FLAGS_HAVE_LONG;
                if (c == 'L')
                    flags |= FLAGS_HAVE_LONG_LONG;
                switch (*m_currFormat)
                    {
                    case 'd': case 'x': case 'X':
                    case 'o': case 'i': case 'u': // 'l' and 'L' do not add anything since we are working with long-sized integers.
                        static_assert (sizeof(int)==sizeof(long), "This code only works when ints and longs are the same size.");
                        break;
                    case 'e': case 'E':
                    case 'f':
                    case 'g': case 'G':
                        *pbuild++ = (char)c;
                        break;
                    case 's':
                        break;

                    case 'l': case 'L':
                        *pbuild++ = (char)c;        
                        *pbuild++ = *m_currFormat++;
                        flags |= FLAGS_HAVE_LONG_LONG;
                        break;
                    }
                }
            else if (tolower(c) == 'p' && sizeof(void*) > sizeof(long))
                { // *** NEEDS WORK: Maybe we should just handle %p separately, instead of trying to make it seem like an int[64]
                flags |= FLAGS_HAVE_LONG_LONG;
                }

            if (*m_currFormat == '\0')  // premature end of formatting string
                return;

            c = *m_currFormat++;

            if (c == '$')
                {
#if defined (NOT_IMPLEMENTED)
                if (minWidth > 0 && firstTime)
                    {
                    outByPos (build, args);
                    if (m_sink.GetError() != 0)
                        return;
                    }
                else
                    {
                    babort ("invalid field width");
                    return;
                    }
#else
                    babort ("$ not implemented");
#endif
                break;
                }
            else
                firstTime = FALSE;

            switch (c)
                {
                default:
                    m_sink.PutChar((char)c);
                    break;

                case 's':
                    {
                    char    *newStringP = va_arg(args,char*);
                    *pbuild++ = 's';
                    *pbuild = 0;
                    
                    // See top of file: char* fmt strings only accept %s and %hs, wchar_t* fmt strings only accept %ls
                    if (m_naturalStringIsWchar && ((0 == (flags & FLAGS_HAVE_LONG)) || (0 != (flags & FLAGS_HAVE_SHORT))))
                        {
                        REPORT_BAD_STRING_TYPE("wchar_t*", (0 != (flags & FLAGS_HAVE_SHORT)) ? "%hs" : "%s");
                        
                        // For now, preserve Microsoft behavior that %s in wchar_t* means wchar_t*.
                        // This should be removed in the future, and/or always forced to consider FLAGS_HAVE_LONG.
                        if (0 == (flags & FLAGS_HAVE_LONG | FLAGS_HAVE_SHORT))
                            flags |= FLAGS_HAVE_LONG;
                        
                        // Also for now, allow the conversions below to continue.
                        // This should be removed in the future, and/or always forced to consider FLAGS_HAVE_LONG.
                        }
                    else if (!m_naturalStringIsWchar)
                        {
                        if (0 != (flags & FLAGS_HAVE_LONG))
                            {
                            REPORT_BAD_STRING_TYPE("char*", "%ls");
                            }
                        else if (0 != (flags & FLAGS_HAVE_SHORT))
                            {
                            flags |= FLAGS_HAVE_SHORT;
                            }
                        
                        // For now, preserve Microsoft behavior that %hs means char*.
                        // This should be removed in the future, and/or always forced to consider FLAGS_HAVE_LONG.
                        
                        // Also for now, allow the conversions below to continue.
                        // This should be removed in the future, and/or always forced to consider FLAGS_HAVE_SHORT.
                        }
                    
                    if ((flags & FLAGS_HAVE_LONG) && (newStringP != NULL))
                        {
                        Utf8String u ((wchar_t*)newStringP);
                        size_t formattedLength = u.size() + strlen(build) + 1;
                        ScopedArray<char> strbuf (formattedLength);
                        sprintf (strbuf.GetData(), build, u.c_str());
                        m_sink.PutCharCP (strbuf.GetData());
                        }
                    else
                        {
                        size_t formattedLength = newStringP? strlen(newStringP) + strlen(build) + 1: strlen(build) + 100;
                        ScopedArray<char> strbuf (formattedLength);
                        sprintf (strbuf.GetData(), build, newStringP);
                        m_sink.PutCharCP (strbuf.GetData());
                        }
                    }
                    break;
                case 'p':
                case 'P':
                    c = 'X';
                case 'd': case 'x': case 'X':
                case 'o': case 'i': case 'u':
                    *pbuild++ = (char)c;
                    *pbuild = 0;
                    if  (flags & FLAGS_HAVE_LONG_LONG)
                        sprintf (buf, build, va_arg(args,int64_t));
                    else
                        sprintf (buf, build, va_arg(args,int32_t));

                    m_sink.PutCharCP (buf);
                    break;

                case 'c':
                    {
                    // See top of file: char* fmt strings only accept %c and %hc, wchar_t* fmt strings only accept %lc
                    if (m_naturalStringIsWchar && ((0 == (flags & FLAGS_HAVE_LONG)) || (0 != (flags & FLAGS_HAVE_SHORT))))
                        {
                        REPORT_BAD_STRING_TYPE("wchar_t*", (0 != (flags & FLAGS_HAVE_SHORT)) ? "%hc" : "%c");

                        // For now, preserve Microsoft behavior that %c in wchar_t* means wchar_t.
                        // This should be removed in the future, and/or always forced to consider FLAGS_HAVE_LONG.
                        if (0 == (flags & FLAGS_HAVE_LONG | FLAGS_HAVE_SHORT))
                            flags |= FLAGS_HAVE_LONG;

                        // Also for now, allow the conversions below to continue.
                        // This should be removed in the future, and/or always forced to consider FLAGS_HAVE_LONG.
                        }
                    else if (!m_naturalStringIsWchar)
                        {
                        if (0 != (flags & FLAGS_HAVE_LONG))
                            {
                            REPORT_BAD_STRING_TYPE("char*", "%lc");
                            }
                        else if (0 != (flags & FLAGS_HAVE_SHORT))
                            {
                            flags |= FLAGS_HAVE_SHORT;
                            }

                        // For now, preserve Microsoft behavior that %hc means char*.
                        // This should be removed in the future, and/or always forced to consider FLAGS_HAVE_LONG.

                        // Also for now, allow the conversions below to continue.
                        // This should be removed in the future, and/or always forced to consider FLAGS_HAVE_SHORT.
                        }
                    
                    *pbuild++ = 'c';
                    *pbuild = 0;
                    sprintf (buf, build, va_arg(args,int));
                    m_sink.PutCharCP (buf);
                    }
                    break;
                case 'C':
                    {
#if defined (_WIN32)
                    wchar_t     wc = va_arg(args,wchar_t);
#else
                    wchar_t     wc = va_arg(args,uint32_t);
#endif
                    *pbuild++ = 'c';
                    *pbuild = 0;
                    sprintf (buf, build, wc);
                    m_sink.PutCharCP (buf);
                    }
                    break;
                case 'e': case 'E':
                case 'f':
                case 'g': case 'G':
                    *pbuild++ = (char)c;
                    *pbuild = (char)0;
                    if (flags & FLAGS_HAVE_LONG_LONG)
                        n = sprintf (buf, build, va_arg(args,long double));
                    else                        
                        n = sprintf (buf, build, va_arg(args,double));
                    if (0 == n)
                        perror (build);
                    m_sink.PutCharCP (buf);
                    break;
                case 'n':
                    destP.pi = va_arg(args,int*);
                    if (flags & FLAGS_HAVE_SHORT)
                        *destP.ps = (short)m_sink.GetCount();
                    else
                        *destP.pi = (int)m_sink.GetCount();
                    break;
#if defined (WIP_BENTLEY_EXTENSION)
                case 'w':
                    *pbuild = (char)0;
                    if (flags & FLAGS_HAVE_POUND)
                        mdlString_fromUorsWithUnits (tmp, *argumentsP.pd++, 0);
                    else
                        generateSimpleCoord (tmp, *argumentsP.pd++);

                    i = strlen (tmp);
                    if ((minWidth > i) &&
                                    !(flags & FLAGS_HAVE_MINUS))
                        {/* Right justify */
                        memset (p, ' ', minWidth -= i);
                        p += minWidth;
                        minWidth = 0;
                        }
                    strcpy (p, tmp);
                    p += i;
                    if ((minWidth > i) &&
                                    (flags & FLAGS_HAVE_MINUS))
                        {/* Right justify */
                        memset (p, ' ', minWidth -= i);
                        p += minWidth;
                        minWidth = 0;
                        }
                    break;
                    /*  End of case 'w' */
#endif
                }
            }
        }
    }
};

/*--------------------------------------------------------------------
| following are functions for scanf
--------------------------------------------------------------------*/
struct Scanner
{
ScanSource& m_scanSource;
CharP       m_currFormat;
CharCP      m_fullFormat;
bool        m_naturalStringIsWchar;

Scanner (ScanSource& ss, CharP s, bool w) : m_scanSource(ss), m_currFormat(s), m_fullFormat(s), m_naturalStringIsWchar(w) {;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    jbg             04/90
+---------------+---------------+---------------+---------------+---------------+------*/
int getHexDigits  /* <= bytes stored in buffer */
(
char    *pBuffer,               /* => Buffer to receive the data. */
int     maxFieldWidth,          /* => Max characters to  store. */
int     currChar                /* => First character to scan. */
)
    {
    int     count = 0;

    while ((maxFieldWidth-- > 0) && (currChar != -1) && isxdigit (currChar))
        {
        count++;
        *pBuffer++ = (char)currChar;
        currChar = m_scanSource.Getc();
        }

    m_scanSource.PutBack ((char)currChar);
    if (count == 0 && currChar == -1)
        return -1;
    return count;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    jbg             04/90
+---------------+---------------+---------------+---------------+---------------+------*/
int getDecimalDigits     /* <= bytes stored in buffer */
(
char    *pBuffer,               /* => Buffer to receive the data. */
int     maxFieldWidth,          /* => Max characters to  store. */
int     currChar                /* => First character to scan. */
)
    {
    int     count = 0;

    while ((maxFieldWidth-- > 0) && (currChar != -1) && isdigit (currChar))
        {
        count++;
        *pBuffer++ = (char)currChar;
        currChar = m_scanSource.Getc();
        }

    m_scanSource.PutBack ((char)currChar);
    if (count == 0 && currChar == -1)
        return -1;
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    jbg             04/90
+---------------+---------------+---------------+---------------+---------------+------*/
int getOctalDigits       /* <= bytes stored in buffer */
(
char    *pBuffer,               /* => Buffer to receive the data. */
int     maxFieldWidth,          /* => Max characters to  store. */
int     currChar                /* => First character to scan. */
)
    {
    int     count = 0;

    while ((maxFieldWidth-- > 0) && (currChar != -1) &&
                            isdigit (currChar) && currChar < '8')
        {
        count++;
        *pBuffer++ = (char)currChar;
        currChar = m_scanSource.Getc();
        }

    if (count == 0 && currChar == -1)
        return -1;
    m_scanSource.PutBack ((char)currChar);
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* It assumes that m_currFormat points to the first character of the current field. m_currFormat must be left pointing to the first
* character after the field.
* @bsimethod                                                    jbg             04/90
+---------------+---------------+---------------+---------------+---------------+------*/
int getFormatFloat       /* <= SUCCESS, SCANF_ABORT, or SCANF_FATAL */
(
double  *pValue,                /* <= value resulting from parse. */
int     *minusSign,             /* <= Return if minus sign found. */
int     *pFieldWidth,           /* => Return count of bytes scanned. */
int     maxFieldWidth           /* => Do not scan any more than this. */
)
    {
    int     count = 0;
    char    buffer [100], *pBuffer = buffer;
    int     ch;
    int     signsAllowed = 1, haveDigit = 0, dotAllowed = 1, haveE = 0;
    int     errorDetected = 0;

    if ((maxFieldWidth < 0) || (maxFieldWidth > sizeof (buffer)-1))
        maxFieldWidth = sizeof (buffer) - 1;

    if (minusSign != NULL)
        *minusSign = FALSE;

    ch = m_scanSource.Getc();
    /*  Skip white space. */
    while (ch != -1 && isspace (ch))
        {
        ch = m_scanSource.Getc();
        count++;
        }

    while (maxFieldWidth-- > 0)
        {
        if (ch == -1)
            break;
        if (isdigit (ch))
            {
            haveDigit = 1;
            signsAllowed = 0;
            }
        else if (ch == 'E' || ch == 'e')
            {
            if (!haveDigit)
                {
                errorDetected = 1;
                break;
                }
            if (haveE)
                break;
            haveE = 1;
            dotAllowed = 0;
            signsAllowed = 1;
            }
        else if (ch == '.')
            {
            if (!dotAllowed)
                break;
            dotAllowed = 0;
            signsAllowed = 0;
            }
        else if (ch == '+' || ch == '-')
            {
            if (!signsAllowed)
                break;
            signsAllowed = 0;
            if (ch == '-' && minusSign != NULL)
                *minusSign = TRUE;
            }
        else
            break;

        *pBuffer++ = (char)ch;
        ch = m_scanSource.Getc();
        count++;
        }

    *pBuffer = 0;
    m_scanSource.PutBack ((char)ch);

    *pFieldWidth = count;

    if (!haveDigit || ch == 'e' || ch == 'E' || ch == '+'
        || ch == '-' || (errorDetected))
        return ((ch == -1) ? SCANF_FATAL : SCANF_ABORT);
    else
        *pValue = strtod (buffer, &pBuffer);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    jbg             09/90
+---------------+---------------+---------------+---------------+---------------+------*/
int getNextWuField       /* <= SUCCESS, SCANF_ABORT, or SCANF_FATAL*/
(
double  *pValue,                /* <= value resulting from parse. */
int     *minusSign,             /* <= Return if minus sign found. */
int     *pFieldWidth,           /* => Return count of bytes scanned. */
int     maxFieldWidth           /* => Do not scan any more than this. */
)
    {
    int     retval, ch;
    int    lastWidth;

    *pValue = 0.0;
    *pFieldWidth = 0;

    if (maxFieldWidth == 0)
        return SCANF_FATAL;

    /*  Look for ':'.  If it is followed by ':', just return 0.
        Otherwise, it must be followed by a digit.          */
    if ((ch = m_scanSource.Getc()) != ':')
        {
        /*  Put it back and return SCANF_FATAL */
        m_scanSource.PutBack ((char)ch);
        return SCANF_FATAL;
        }

    (*pFieldWidth)++;
    maxFieldWidth--;

    /*  If the next character is not a digit, use 0 as a value. */
    ch = m_scanSource.Getc();
    m_scanSource.PutBack ((char)ch);
    if (!isdigit (ch) && ch != '-')
        return SUCCESS;

    retval = getFormatFloat (pValue, minusSign, &lastWidth, maxFieldWidth);

    *pFieldWidth += lastWidth;

    return retval;
    }

/*---------------------------------------------------------------------------------**//**
* Everyplace this is called used to call ctran. I can't see any reason for it to call ctran.
* @bsimethod                                                    John.Gooding    06/93
+---------------+---------------+---------------+---------------+---------------+------*/
int localCTran
(
CharP&   p
)
    {
    return *p++;
    }

/*---------------------------------------------------------------------------------**//**
* It assumes that m_currFormat points to the first character of the current field. m_currFormat must be left pointing to the first
* character after the field.
* @bsimethod                                                    jbg             04/90
+---------------+---------------+---------------+---------------+---------------+------*/
int initializeScanSet    /* <= SUCCESS if no error. */
(
char        *pScanArray         /* <=> Array to define scan sets */
)
    {
    unsigned int    ch;
    int             mask = 0, firstChar = 1;
    unsigned int    prevChar = 0;

    if (*m_currFormat == '^')
        {
        mask = 1;
        (m_currFormat)++;
        }
    memset (pScanArray, mask, 256);
    mask ^= 1;
    ch = localCTran (m_currFormat);
    while (ch != '\0')
        {
        if (ch == ']' && !firstChar)
            break;
        firstChar = 0;
        if ((ch == '-')  && (prevChar != 0) &&
            (*m_currFormat) && (*m_currFormat != ']'))
            {
            unsigned int finalChar, swapChar;

            if ((finalChar = localCTran (m_currFormat)) < prevChar)
                {
                swapChar = prevChar;
                prevChar = finalChar;
                finalChar = swapChar;
                }

            memset ((pScanArray + prevChar), mask, finalChar-prevChar+1);
            /*  Do a range. */
            prevChar = 0;
            }
        else
            {
            *(pScanArray+ch) = (char)mask;
            prevChar = ch;
            }
        ch = localCTran (m_currFormat);
        }
    return ch != ']';
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    07/00
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t  strfunc_strtoi64
(
char            *pString,
int              base
)
    {
    int          ch;
    int64_t      retval = 0;

    while (ch = *pString++)
        {
        if  (ch >= '0' && ch <= '7')
            ch = ch - '0';
        else if (ch == '8' || ch == '9')
            {
            if  (base == 8)
                break;
            ch = ch - '0';
            }
        else if ((ch >= 'a' && ch <= 'f') || ch >= 'A' && ch <= 'F' )
            ch = tolower (ch) - 'a' + 10;
        else
            break;

        retval = retval * base + ch;
        }

    return  retval;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    jbg             04/90
+---------------+---------------+---------------+---------------+---------------+------*/
int getScanSet           /* <= returns SUCCESS unless the count is
                                      0 and the last character is EOF. */
(
int         *pCharsScanned,     /* <= Count of characters scanned. */
int         maxFieldWidth,      /* => Maximum number of characters to scan
                                      \t counts as 1 character.  */
char        *pTarget,           /* <=> Area to receive characters. */
char        *pScanArray         /* => For any character ch, *(pScanArray+ch)
                                      is 1 if ch is in the scan set. */
 )
   {
    int     ch, charCount = 0;

    if (maxFieldWidth < 0)
        maxFieldWidth = 30000;

    if (maxFieldWidth == 0)
        {
        *pCharsScanned = 0;
        return SUCCESS;
        }

    ch = m_scanSource.Getc();

    if (pTarget != NULL)
        while (maxFieldWidth > 0)
            {
            maxFieldWidth--;
            if ((ch == -1) || *(pScanArray+ch) == 0)
                break;
            charCount++;
            *pTarget++ = (char)ch;
            ch = m_scanSource.Getc();
            }
    else
        while (maxFieldWidth > 0)
            {
            maxFieldWidth--;
            if ((ch == -1) || *(pScanArray+ch) == 0)
                break;
            charCount++;
            ch = m_scanSource.Getc();
            }

    if (pTarget != NULL)
        *pTarget = '\0';
    m_scanSource.PutBack ((char)ch);
    *pCharsScanned = charCount;
    if (charCount == 0)
        {
        if (ch == -1)
            return SCANF_FATAL;
        else
            return SCANF_ABORT;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    07/00
+---------------+---------------+---------------+---------------+---------------+------*/
int     getFormatD64             /* <= SUCCESS, SCANF_ABORT, or SCANF_FATAL*/
(
int64_t         *pValue,                /* <=> value resulting from parse. */
int             *pFieldWidth,           /* => Return count of bytes scanned. */
int              maxFieldWidth          /* => Do not scan any more than this. */
)
    {
    int     retval, count = 0;
    char    buffer [100], *pBufferEnd = buffer;
    char    ch;

    if ((maxFieldWidth < 0) || (maxFieldWidth > sizeof (buffer)-1))
        maxFieldWidth = sizeof (buffer) - 1;

    /*  Skip white space. */
    ch = (char)m_scanSource.Getc();

    /*  Skip white space. */
    while (ch != -1 && isspace (ch))
        {
        ch = (char)m_scanSource.Getc();
        count++;
        }

    if (ch == '+' || ch == '-')
        {
        *pBufferEnd++ = ch;
        maxFieldWidth--;
        ch = (char)m_scanSource.Getc();
        count++;
        }

    if ((retval = getDecimalDigits (pBufferEnd, maxFieldWidth, ch)) == -1)
        return SCANF_FATAL;

    if (retval == 0)
        return SCANF_ABORT;

    pBufferEnd += retval;
    count += retval;

    *pBufferEnd = 0;
    sscanf (buffer, "%lld", pValue);
    *pFieldWidth = count;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    jbg             04/90
+---------------+---------------+---------------+---------------+---------------+------*/
int     getFormatD               /* <= SUCCESS, SCANF_ABORT, or SCANF_FATAL*/
(
long            *pValue,                /* <=> value resulting from parse. */
int             *pFieldWidth,           /* => Return count of bytes scanned. */
int              maxFieldWidth          /* => Do not scan any more than this. */
)
    {
    int     retval, count = 0;
    char    buffer [100], *pBufferEnd = buffer;
    char    ch;

    if ((maxFieldWidth < 0) || (maxFieldWidth > sizeof (buffer)-1))
        maxFieldWidth = sizeof (buffer) - 1;

    /*  Skip white space. */
    ch = (char)m_scanSource.Getc();

    /*  Skip white space. */
    while (ch != -1 && isspace (ch))
        {
        ch = (char)m_scanSource.Getc();
        count++;
        }

    if (ch == '+' || ch == '-')
        {
        *pBufferEnd++ = ch;
        maxFieldWidth--;
        ch = (char)m_scanSource.Getc();
        count++;
        }

    if ((retval = getDecimalDigits (pBufferEnd, maxFieldWidth, ch)) == -1)
        return SCANF_FATAL;

    if (retval == 0)
        return SCANF_ABORT;

    pBufferEnd += retval;
    count += retval;

    *pBufferEnd = 0;
    *pValue = strtol (buffer, &pBufferEnd, 10);
    *pFieldWidth = count;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    07/00
+---------------+---------------+---------------+---------------+---------------+------*/
int      getFormatI64
(
int64_t         *pValue,                /* <=> value resulting from parse. */
int             *pFieldWidth,           /* => Return count of bytes scanned. */
int              maxFieldWidth          /* => Do not scan any more than this. */
)
    {
    int     retval, count = 0;
    char    buffer [100], *pBufferEnd = buffer;
    char    ch;
    int     negative = 0;
    int     base = 10;

    if ((maxFieldWidth < 0) || (maxFieldWidth > sizeof (buffer)-1))
        maxFieldWidth = sizeof (buffer) - 1;

    /*  Skip white space. */
    ch = (char)m_scanSource.Getc();

    /*  Skip white space. */
    while (ch != -1 && isspace (ch))
        {
        ch = (char)m_scanSource.Getc();
        count++;
        }

    switch (ch)
        {
        case '0':
            base = 8;
            count++;
            ch = (char)m_scanSource.Getc();
            if (tolower (ch) == 'x')
                {
                base = 16;
                count++;
                ch = (char)m_scanSource.Getc();
                }
            break;
        case '-':
            negative = 1;
        case '+':
            count++;
            ch = (char)m_scanSource.Getc();
        }

    retval = (base == 10) ? getDecimalDigits (pBufferEnd, maxFieldWidth, ch) :
                (base == 16) ? getHexDigits (pBufferEnd, maxFieldWidth, ch) :
                               getOctalDigits (pBufferEnd, maxFieldWidth, ch);

    if (retval == -1)
        return SCANF_FATAL;
    else if (retval == 0)
        return SCANF_ABORT;

    pBufferEnd += retval;
    count += retval;

    *pBufferEnd = 0;
    switch (base)
        {
        case 8:
            *pValue = strfunc_strtoi64 (buffer, 8);
            break;
        case 10:
            *pValue = strfunc_strtoi64 (buffer, 10);
            if (negative)
                *pValue = -*pValue;
            break;
        case 16:
            *pValue = strfunc_strtoi64 (buffer, 16);
            break;
        }

    *pFieldWidth = count;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    jbg             04/90
+---------------+---------------+---------------+---------------+---------------+------*/
int getFormatI           /* <= SUCCESS, SCANF_ABORT, or SCANF_FATAL*/
(
long    *pValue,                /* <=> value resulting from parse. */
int     *pFieldWidth,           /* => Return count of bytes scanned. */
int     maxFieldWidth           /* => Do not scan any more than this. */
)
    {
    int     retval, count = 0;
    char    buffer [100], *pBufferEnd = buffer;
    char    ch;
    int     negative = 0;
    int     base = 10;
    int     return0 = FALSE;

    if ((maxFieldWidth < 0) || (maxFieldWidth > sizeof (buffer)-1))
        maxFieldWidth = sizeof (buffer) - 1;

    /*  Skip white space. */
    ch = (char)m_scanSource.Getc();

    /*  Skip white space. */
    while (ch != -1 && isspace (ch))
        {
        ch = (char)m_scanSource.Getc();
        count++;
        }

    switch (ch)
        {
        case '0':
            return0 = TRUE;  /*  standalone zero should be returned as a zero */
            base = 8;
            count++;
            ch = (char)m_scanSource.Getc();
            if (tolower (ch) == 'x')
                {
                base = 16;
                count++;
                ch = (char)m_scanSource.Getc();
                }
            break;
        case '-':
            negative = 1;
        case '+':
            count++;
            ch = (char)m_scanSource.Getc();
        }

    retval = (base == 10) ? getDecimalDigits (pBufferEnd, maxFieldWidth, ch) :
                (base == 16) ? getHexDigits (pBufferEnd, maxFieldWidth, ch) :
                               getOctalDigits (pBufferEnd, maxFieldWidth, ch);

    if  (retval <= 0 && return0)
        {
        *pValue = 0;
        *pFieldWidth = count;
        return  SUCCESS;
        }

    if (retval == -1)
        return SCANF_FATAL;
    else if (retval == 0)
        return SCANF_ABORT;

    pBufferEnd += retval;
    count += retval;

    *pBufferEnd = 0;
    switch (base)
        {
        case 8:
            *pValue = strtoul (buffer, &pBufferEnd, 8);
            break;
        case 10:
            *pValue = strtol (buffer, &pBufferEnd, 10);
            if (negative)
                *pValue = -*pValue;
            break;
        case 16:
            *pValue = strtoul (buffer, &pBufferEnd, 16);
            break;
        }

    *pFieldWidth = count;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    07/00
+---------------+---------------+---------------+---------------+---------------+------*/
int      getFormatX64
(
int64_t         *pValue,                /* <=> value resulting from parse. */
int             *pFieldWidth,           /* => Return count of bytes scanned. */
int              maxFieldWidth          /* => Do not scan any more than this. */
)
    {
    int     retval, count = 0;
    char    buffer [100], *pBufferEnd = buffer;
    char    ch;

    if ((maxFieldWidth < 0) || (maxFieldWidth > sizeof (buffer)-1))
        maxFieldWidth = sizeof (buffer) - 1;

    /*  Skip white space. */
    ch = (char)m_scanSource.Getc();

    /*  Skip white space. */
    while (ch != -1 && isspace (ch))
        {
        ch = (char)m_scanSource.Getc();
        count++;
        }

    /*  Strip leading 0 or 0x. */
    if (ch == '0')
        {
        count++;
        ch = (char)m_scanSource.Getc();
        if (tolower (ch) == 'x')
            {
            count++;
            ch = (char)m_scanSource.Getc();
            }
        else
            {
            m_scanSource.PutBack (ch);
            ch = '0';
            }
        }

    if ((retval = getHexDigits (pBufferEnd, maxFieldWidth, ch)) == -1)
        return SCANF_FATAL;

    if (retval == 0)
        return SCANF_ABORT;

    pBufferEnd += retval;
    count += retval;

    *pBufferEnd = 0;
    sscanf (buffer, "%llx", pValue);
    *pFieldWidth = count;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    jbg             04/90
+---------------+---------------+---------------+---------------+---------------+------*/
int getFormatX           /* <= SUCCESS, SCANF_ABORT, or SCANF_FATAL*/
(
unsigned long   *pValue,                /* <=> value resulting from parse. */
int             *pFieldWidth,           /* => Return count of bytes scanned. */
int             maxFieldWidth           /* => Do not scan any more than this. */
)
    {
    int     retval, count = 0;
    char    buffer [100], *pBufferEnd = buffer;
    char    ch;

    if ((maxFieldWidth < 0) || (maxFieldWidth > sizeof (buffer)-1))
        maxFieldWidth = sizeof (buffer) - 1;

    /*  Skip white space. */
    ch = (char)m_scanSource.Getc();

    /*  Skip white space. */
    while (ch != -1 && isspace (ch))
        {
        ch = (char)m_scanSource.Getc();
        count++;
        }

    /*  Strip leading 0 or 0x. */
    if (ch == '0')
        {
        count++;
        ch = (char)m_scanSource.Getc();
        if (tolower (ch) == 'x')
            {
            count++;
            ch = (char)m_scanSource.Getc();
            }
        else
            {
            m_scanSource.PutBack (ch);
            ch = '0';
            }
        }

    if ((retval = getHexDigits (pBufferEnd, maxFieldWidth, ch)) == -1)
        return SCANF_FATAL;

    if (retval == 0)
        return SCANF_ABORT;

    pBufferEnd += retval;
    count += retval;

    *pBufferEnd = 0;
    *pValue = strtoul (buffer, &pBufferEnd, 16);
    *pFieldWidth = count;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    07/00
+---------------+---------------+---------------+---------------+---------------+------*/
int      getFormatO64
(
int64_t         *pValue,                /* <=> value resulting from parse. */
int             *pFieldWidth,           /* => Return count of bytes scanned. */
int              maxFieldWidth          /* => Do not scan any more than this. */
)
    {
    int     retval, count = 0;
    char    buffer [100], *pBufferEnd = buffer;
    char    ch;

    if ((maxFieldWidth < 0) || (maxFieldWidth > sizeof (buffer)-1))
        maxFieldWidth = sizeof (buffer) - 1;

    /*  Skip white space. */
    ch = (char)m_scanSource.Getc();

    /*  Skip white space. */
    while (ch != -1 && isspace (ch))
        {
        ch = (char)m_scanSource.Getc();
        count++;
        }

    if ((retval = getOctalDigits (buffer, maxFieldWidth, ch)) == -1)
        return SCANF_FATAL;
    if (retval == 0)
        return SCANF_ABORT;
    pBufferEnd = buffer + retval;
    count += retval;

    *pBufferEnd = 0;
    sscanf (buffer, "%llo", pValue);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    jbg             04/90
+---------------+---------------+---------------+---------------+---------------+------*/
int getFormatO           /* <= SUCCESS, SCANF_ABORT, or SCANF_FATAL*/
(
unsigned long   *pValue,                /* <=> value resulting from parse. */
int             *pFieldWidth,           /* => Return count of bytes scanned. */
int             maxFieldWidth           /* => Do not scan any more than this. */
)
    {
    int     retval, count = 0;
    char    buffer [100], *pBufferEnd = buffer;
    char    ch;

    if ((maxFieldWidth < 0) || (maxFieldWidth > sizeof (buffer)-1))
        maxFieldWidth = sizeof (buffer) - 1;

    /*  Skip white space. */
    ch = (char)m_scanSource.Getc();

    /*  Skip white space. */
    while (ch != -1 && isspace (ch))
        {
        ch = (char)m_scanSource.Getc();
        count++;
        }

    if ((retval = getOctalDigits (buffer, maxFieldWidth, ch)) == -1)
        return SCANF_FATAL;
    if (retval == 0)
        return SCANF_ABORT;
    pBufferEnd = buffer + retval;
    count += retval;

    *pBufferEnd = 0;
    *pValue = strtoul (buffer, &pBufferEnd, 8);
    *pFieldWidth = count;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    07/00
+---------------+---------------+---------------+---------------+---------------+------*/
int      getFormatU64
(
int64_t         *pValue,                /* <=> value resulting from parse. */
int             *pFieldWidth,           /* => Return count of bytes scanned. */
int              maxFieldWidth          /* => Do not scan any more than this. */
)
    {
    int     count = 0, retval;
    char    buffer [100], *pBufferEnd;
    char    ch;

    if ((maxFieldWidth < 0) || (maxFieldWidth > sizeof (buffer)-1))
        maxFieldWidth = sizeof (buffer) - 1;

    /*  Skip white space. */
    ch = (char)m_scanSource.Getc();

    /*  Skip white space. */
    while (ch != -1 && isspace (ch))
        {
        ch = (char)m_scanSource.Getc();
        count++;
        }

    if ((retval = getDecimalDigits (buffer, maxFieldWidth, ch)) == -1)
        return SCANF_FATAL;
    if (retval == 0)
        return SCANF_ABORT;
    pBufferEnd = buffer + retval;
    count += retval;

    *pBufferEnd = 0;
    sscanf (buffer, "%llu", pValue);
    *pFieldWidth = count;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    jbg             04/90
+---------------+---------------+---------------+---------------+---------------+------*/
int getFormatU           /* <= SUCCESS, SCANF_ABORT, or SCANF_FATAL*/
(
unsigned long   *pValue,                /* <=> value resulting from parse. */
int             *pFieldWidth,           /* => Return count of bytes scanned. */
int             maxFieldWidth           /* => Do not scan any more than this. */
)
    {
    int     count = 0, retval;
    char    buffer [100], *pBufferEnd;
    char    ch;

    if ((maxFieldWidth < 0) || (maxFieldWidth > sizeof (buffer)-1))
        maxFieldWidth = sizeof (buffer) - 1;

    /*  Skip white space. */
    ch = (char)m_scanSource.Getc();

    /*  Skip white space. */
    while (ch != -1 && isspace (ch))
        {
        ch = (char)m_scanSource.Getc();
        count++;
        }

    if ((retval = getDecimalDigits (buffer, maxFieldWidth, ch)) == -1)
        return SCANF_FATAL;
    if (retval == 0)
        return SCANF_ABORT;
    pBufferEnd = buffer + retval;
    count += retval;

    *pBufferEnd = 0;
    *pValue = strtoul (buffer, &pBufferEnd, 10);
    *pFieldWidth = count;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* It assumes that m_currFormat points to the first character of the current field. m_currFormat must be left pointing to the first
* character after the field.
* Take character from the input array and put them into the array pTarget points to. If pTarget is NULL, just scan the characters.
* Continue taking the characters until either the end of the string is encountered, or until the field width is exhausted.
* @bsimethod                                                    jbg             04/90
+---------------+---------------+---------------+---------------+---------------+------*/
int getFormatC           /* <= returns SUCCESS only EOF occurred
                                      before getting the first char. */
(
int         *pCharsScanned,     /* <= Count of characters scanned. */
int         maxFieldWidth,      /* => Maximum number of characters to scan
                                      \t counts as 1 character.  */
char        *pTarget            /* <=> Area to receive characters. */
)
    {
    int     charCount = 0, ch;

    ch = m_scanSource.Getc();
    if (pTarget != NULL)
        while (maxFieldWidth > 0)
            {
            maxFieldWidth--;
            if (ch == -1)
                break;
            *pTarget++ = (char)ch;
            charCount++;
            ch = m_scanSource.Getc();
            }
    else
        while (maxFieldWidth > 0)
            {
            maxFieldWidth--;
            if (ch == -1)
                break;
            charCount++;
            ch = m_scanSource.Getc();
            }

    m_scanSource.PutBack ((char)ch);
    *pCharsScanned = charCount;
    if (charCount == 0)
        return SCANF_FATAL;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* It assumes that m_currFormat points to the first character of the current field. m_currFormat must be left pointing to the first
* character after the field.
* Scan over leading white space. This characters will be included in the "characters-spanned count".
* Transfer characters to the target buffer until one of the following occurs:
* o EOS occurs o fieldWidth-1 characters have been scanned. o whitespace is encountered.
* Store EOS in the target buffer.
* Return a count of characters scanned.
* @bsimethod                                                    jbg             04/90
+---------------+---------------+---------------+---------------+---------------+------*/
int getFormatS           /* <= returns SUCCESS if exiting with
                                      pCharsScanned == maxFieldWidth. */
(
int         *pCharsScanned,     /* <= Count of characters scanned. */
int         maxFieldWidth,      /* => Maximum number of characters to scan
                                      \t counts as 1 character.  */
char        *pTarget            /* <=> Area to receive characters. */
)
    {
    int     ch, skippedCount = 0, returnedCount = 0;
    int     blockPutBack = 0;

    if (maxFieldWidth < 0)
        maxFieldWidth = 30000;

    ch = m_scanSource.Getc();
    while (ch >= 0 && isspace (ch))
        {
        skippedCount++;
        ch = m_scanSource.Getc();
        }

    if (maxFieldWidth == 0)
        {
        *pCharsScanned = skippedCount;
        m_scanSource.PutBack ((char)ch);
        return 0;
        }

    if (pTarget != NULL)
        while (maxFieldWidth > 0)
            {
            maxFieldWidth--;
            if ((ch == -1) || (isspace (ch)))
                break;
            returnedCount++;
            if (ch == '\n')
                {
                blockPutBack = 1;
                break;
                }
            *pTarget++ = (char)ch;
            ch = m_scanSource.Getc();
            }
    else
        while (maxFieldWidth > 0)
            {
            maxFieldWidth--;
            if ((ch == -1) || (isspace (ch)))
                break;
            returnedCount++;
            if (ch == '\n')
                {
                blockPutBack = 1;
                break;
                }
            ch = m_scanSource.Getc();
            }

    if (pTarget != NULL)
        *pTarget = '\0';
    if (!blockPutBack)
        m_scanSource.PutBack ((char)ch);
    *pCharsScanned = returnedCount + skippedCount;
    if (returnedCount == 0)
        return SCANF_FATAL;
    return SUCCESS;
    }

 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    sam.wilson      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int getFormatLS           /* <= returns SUCCESS if exiting with pCharsScanned == maxFieldWidth. */
(
int         *pCharsScanned,     /* <= Count of characters scanned. */
int         maxFieldWidth,      /* => Maximum number of characters to scan
                                      \t counts as 1 character.  */
wchar_t     *pTarget            /* <=> Area to receive characters. */
)
    {
    ScopedArray<char> ubuf (maxFieldWidth > 0? maxFieldWidth*4 + 4: 4096);

    StatusInt status = getFormatS (pCharsScanned, maxFieldWidth, ubuf.GetData());   // read in UTF-8 format
    
    if (status != SUCCESS || pTarget == NULL)
        return status;

    WString wbuf (ubuf.GetData(), true);

    if (maxFieldWidth < 0)
        {
        wcscpy (pTarget, wbuf.c_str());
        }
    else if (maxFieldWidth>0)
        {
        wcsncpy (pTarget, wbuf.c_str(), maxFieldWidth);
        pTarget[maxFieldWidth-1] = 0;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* performs the scanning functions for all of the scanf functions.
* strfunc_scanf's return value is stored in mdlLoop_retval.i. The return value is set to EOF if input is terminated early by end of input.
* If strfunc_scanf encounters a character it is not prepared to deal with, it terminates early returning the count of of conversions.
* @bsimethod                                                    John.Gooding    01/90
+---------------+---------------+---------------+---------------+---------------+------*/
int DoScan       /* <= count of fields assigned, -1 if an error occurred */
(
va_list         args
)
    {
    int         formatChar, streamChar;
    int         typeModifier, suppressOutput;
    int         fieldWidth;
    int         lastFieldScanned, totalScanned = 0;
    int         fieldsScannedAndAssigned = 0;
    int         status = 0;     /* SUCCESS, SCANF_ABORT, SCANF_FATAL */
    long        value;
    unsigned long uValue;
    double      dValue;
    Ptypes_u    pData;
    char        scanSetArray [256];
    CharP       pcTmp;
    int         isLong, isLongLong;

    /*  Break out of the loop if the format string is completed, no more
        data is available, an error is encountered, or stack underflow
        occurs.
    */
    while (status == 0)
        {
        if (*m_currFormat == '\0')
            break;

        pcTmp = m_currFormat;
        formatChar = localCTran (m_currFormat);

        if (isspace (formatChar))
            {
            /* Skip the current block of white space in both the
               format and the input stream.  */
            while (isspace (*m_currFormat))
                m_currFormat++;

            /*  If the next character format character is text which must
                be matched, then verify that the input stream contains at
                least one space.
            */
            if (*m_currFormat && (*m_currFormat != '%'))
                {
                totalScanned++;
                streamChar = m_scanSource.Getc();
                if (!isspace (streamChar))
                    {
                    status = (streamChar == -1) ? SCANF_FATAL : SCANF_ABORT;
                    m_scanSource.PutBack((char)streamChar);
                    continue;
                    }
                }

            do
                {
                totalScanned++;
                streamChar = m_scanSource.Getc();
                } while (isspace (streamChar));

            totalScanned--;
            m_scanSource.PutBack((char)streamChar);
            continue;
            }

        if (formatChar == '%')
            {/*  Start assembling the format string.  */
            if (*m_currFormat == '%')
                {
                m_currFormat++;
                /*  Consume the next character in the input stream. */
                if (m_scanSource.Getc() != '%')
                    status = SCANF_ABORT;
                continue;
                }

            if (*m_currFormat == '*')
                {
                m_currFormat++;
                suppressOutput = 1;
                }
            else
                suppressOutput = 0;

            if (isdigit (*m_currFormat))
                {/* Have a digit so get a field width. */
                fieldWidth = (int)strtol (m_currFormat, &m_currFormat, 10);
                }
            else
                fieldWidth = -1;    /*  Use default for specific conversion */

            isLong = isLongLong = FALSE;

            typeModifier = *m_currFormat;
            if (typeModifier == 'h')
                {
                m_currFormat++;
                }
            else if (typeModifier == 'L')
                {
                isLong = TRUE;
                isLongLong = TRUE;
                typeModifier = 0;
                m_currFormat++;
                }
            else if (typeModifier == 'l')
                {
                isLong = TRUE;
                m_currFormat++;

                if (tolower(*m_currFormat) == 'l')
                    {
                    isLongLong = TRUE;
                    typeModifier = 0;
                    m_currFormat++;
                    }
                }
            else if (*m_currFormat == 'I' && *(m_currFormat + 1) == '6' && *(m_currFormat + 2) == '4')
                {
                REPORT_BAD_STRING_TYPE ("long long", "I64");
                m_currFormat += 3;
                typeModifier = 0;
                isLongLong = TRUE;
                }
            else if (tolower(*m_currFormat=='p') && sizeof(void*)>sizeof(long))
                {
                typeModifier = 0;
                isLongLong = TRUE;
                }
            else
                typeModifier = 0;

            if (fieldsScannedAndAssigned == 0)
                {
                if (*m_currFormat == '$')
                    {
                    BeAssert (false && "*** $ not implemented ***");
                    return SCANF_FATAL;
#if defined (NOT_IMPLEMENTED)
                    if (suppressOutput == 1)
                        {
                        status = SCANF_FATAL;
                        break;
                        }
                    fieldsScannedAndAssigned = getByPos (pcTmp, args);
                    if (fieldsScannedAndAssigned == -1)
                        {
                        fieldsScannedAndAssigned = 0;
                        status = SCANF_FATAL;
                        }
                    break;
#endif
                    }
                }
            else if (*m_currFormat == '$')
                {
                status = SCANF_FATAL;
                break;
                }

            if  (isLongLong)
                {
                int64_t      value64;

                switch (*m_currFormat++)
                    {
                    case 'd':
                        status = getFormatD64 (&value64, &lastFieldScanned, fieldWidth);
                        totalScanned += lastFieldScanned;
                        if (status != 0)
                            continue;
                        break;

                    case 'i':
                        status = getFormatI64 (&value64, &lastFieldScanned, fieldWidth);
                        totalScanned += lastFieldScanned;
                        if (status != 0)
                            continue;
                        break;

                    case 'o':
                        status = getFormatO64 (&value64, &lastFieldScanned, fieldWidth);
                        totalScanned += lastFieldScanned;
                        if (status != 0)
                            continue;
                        break;

                    case 'u':
                        status = getFormatU64 (&value64, &lastFieldScanned,
                                                            fieldWidth);
                        totalScanned += lastFieldScanned;
                        if (status != 0)
                            continue;
                        break;

                    case 'x':
                    case 'p':
                    case 'P':
                        status = getFormatX64 (&value64, &lastFieldScanned,
                                                        fieldWidth);
                        totalScanned += lastFieldScanned;
                        if (status != 0)
                            continue;
                        break;

                    case 'e':
                    case 'f':
                    case 'g':
                        BeAssert (false && "WIP_SCANF -- support for %Lg, %Le, %llg, %lle");

                    default:
                        status = SCANF_FATAL;
                        continue;
                    }

                if (!suppressOutput)
                    {
                    fieldsScannedAndAssigned++;
                    pData.pi = va_arg(args,int*);
                    *pData.pi64 = value64;
                    }
                continue;
                }
            char formatType = *m_currFormat;
            ++m_currFormat;
            switch (formatType)
                {
                case 'd':
                    status = getFormatD(&value, &lastFieldScanned,
                                                fieldWidth);
                    totalScanned += lastFieldScanned;
                    if (status != 0)
                        continue;

                    if (!suppressOutput)
                        {
                        fieldsScannedAndAssigned++;
                        pData.pi = va_arg(args,int*);
                        if (typeModifier == 'h')
                            *pData.ps = (short)value;
                        else
                            *pData.pi = (int)value;
                        }
                    continue;
                case 'i':
                    status = getFormatI (&value, &lastFieldScanned,
                                                fieldWidth);
                    totalScanned += lastFieldScanned;
                    if (status != 0)
                        continue;

                    if (!suppressOutput)
                        {
                        fieldsScannedAndAssigned++;
                        pData.pi = va_arg(args,int*);
                        if (typeModifier == 'h')
                            *pData.ps = (short)value;
                        else
                            *pData.pi = (int)value;
                        }
                    continue;
                case 'o':
                    status = getFormatO (&uValue, &lastFieldScanned,
                                                        fieldWidth);
                    totalScanned += lastFieldScanned;
                    if (status != 0)
                        continue;

                    if (!suppressOutput)
                        {
                        fieldsScannedAndAssigned++;
                        pData.pi = va_arg(args,int*);
                        if (typeModifier == 'h')
                            *pData.pus = (unsigned short)uValue;
                        else
                            *pData.pul = (unsigned long)uValue;
                        }
                    continue;
                case 'u':
                    status = getFormatU (&uValue, &lastFieldScanned,
                                                        fieldWidth);
                    totalScanned += lastFieldScanned;
                    if (status != 0)
                        continue;

                    if (!suppressOutput)
                        {
                        fieldsScannedAndAssigned++;
                        pData.pi = va_arg(args,int*);
                        if (typeModifier == 'h')
                            *pData.pus = (unsigned short)uValue;
                        else
                            *pData.pul = (unsigned long)uValue;
                        }
                    continue;
                case 'p':
                case 'x':
                case 'X':
                    status = getFormatX (&uValue, &lastFieldScanned,
                                                    fieldWidth);
                    totalScanned += lastFieldScanned;
                    if (status != 0)
                        continue;

                    if (!suppressOutput)
                        {
                        fieldsScannedAndAssigned++;
                        pData.pi = va_arg(args,int*);
                        if (typeModifier == 'h')
                            *pData.pus = (unsigned short)uValue;
                        else
                            *pData.pul = uValue;
                        }
                    continue;
                case 'c':
                    if (fieldWidth < 1)
                        fieldWidth = 1;

                    // *** WIP_UNICODE -- TBD getFormatLC

                    if (!suppressOutput)
                        {
                        status = getFormatC
                                (&lastFieldScanned, fieldWidth, va_arg(args,char*));
                        totalScanned += lastFieldScanned;
                        if (status == SUCCESS)
                            {
                            fieldsScannedAndAssigned++;
                            }
                        }
                    else
                        {
                        status = getFormatC
                                (&lastFieldScanned, fieldWidth, 0);
                        totalScanned += lastFieldScanned;
                        }
                    continue;
                case 's':
                    if (!suppressOutput)
                        {
                        CharP obuf = va_arg(args,char*);
                        
                        // See top of file: char* fmt strings only accept %s and %hs, wchar_t* fmt strings only accept %ls
                        if (m_naturalStringIsWchar && (typeModifier != 'l'))
                            {
                            REPORT_BAD_STRING_TYPE("wchar_t*", (typeModifier ? "%hs" : "%s"));
                            }
                        else if (!m_naturalStringIsWchar && (typeModifier == 'l'))
                            {
                            REPORT_BAD_STRING_TYPE("char*", "%ls");
                            }
                        
                        if (typeModifier=='l')
                            status = getFormatLS (&lastFieldScanned, fieldWidth, (WCharP)obuf);
                        else
                            status = getFormatS (&lastFieldScanned, fieldWidth, obuf);
                        
                        totalScanned += lastFieldScanned;
                        
                        if (status == SUCCESS)
                            fieldsScannedAndAssigned++;
                        }
                    else
                        {
                        status = getFormatS
                                (&lastFieldScanned, fieldWidth, 0);
                        totalScanned += lastFieldScanned;
                        }
                    continue;
                case 'n':
                    pData.pi = va_arg(args,int*);
                    *pData.pi = totalScanned;
                    continue;
                case 'e':
                case 'f':
                case 'g':
                    status = getFormatFloat
                        (&dValue, NULL, &lastFieldScanned, fieldWidth);
                    totalScanned += lastFieldScanned;
                    if (status != SUCCESS)
                        continue;

                    if (!suppressOutput)
                        {
                        fieldsScannedAndAssigned++;
                        pData.pv = va_arg(args,void*);
                        if (isLong)
                            *pData.pd = dValue;
                        else
                            *pData.pf = (float)dValue;
                        }
                    continue;
#if defined (WIP_BENTLEY_EXTENSION)
                case 'w':
                    status = getFormatWorkingUnits
                                    (&dValue, &lastFieldScanned, fieldWidth);
                    totalScanned += lastFieldScanned;
                    if (status != SUCCESS)
                        continue;

                    if (!suppressOutput)
                        {
                        if (nargs-- <= 0)
                            {
                            status = SCANF_FATAL;
                            continue;
                            }
                        fieldsScannedAndAssigned++;
                        pData.pd = *tos.ppd++;
                        *pData.pd = dValue;
                        }
                    continue;
#endif
                case '[':
                    if ((status = initializeScanSet (scanSetArray))
                        != SUCCESS)
                        continue;
                    if (!suppressOutput)
                        {
                        status = getScanSet (&lastFieldScanned,
                                        fieldWidth, va_arg(args,char*), scanSetArray);
                        totalScanned += lastFieldScanned;
                        if (status == SUCCESS)
                            {
                            fieldsScannedAndAssigned++;
                            }
                        }
                    else
                        {
                        status = getScanSet (&lastFieldScanned,
                                        fieldWidth, NULL, scanSetArray);
                        totalScanned += lastFieldScanned;
                        }
                    continue;
                default:
                    continue;
                }
            } /* End of processing '%' specification */

        /*  Consume the next character in the input stream. */
        totalScanned++;
        streamChar = m_scanSource.Getc();

        if (streamChar != formatChar)
            {
            if (streamChar == -1)
                {
                status = SCANF_FATAL;
                }
            else
                {
                m_scanSource.PutBack((char)streamChar);
                status = SCANF_ABORT;
                }
            }
        }

    if (fieldsScannedAndAssigned)
        return fieldsScannedAndAssigned;

    if (status == SCANF_FATAL)
        return -1;

    return 0;
    }

};

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sources and sinks

PrintSink::PrintSink() : m_error(0) {;}
void    PrintSink::OnError (int e)                  {m_error=e;}
int     PrintSink::GetError() const                 {return m_error;}

WStringPrintSink::WStringPrintSink (WString& s) : m_s(s) {;}
void    WStringPrintSink::PutChar (char c)          {m_s.append (1,c);}
void    WStringPrintSink::PutCharCP (CharCP s)      
    {
    // NB: #chars in UTF-8 string is always >= UTF-32/16 array 
    size_t nChars = strlen(s)+1;
    ScopedArray<wchar_t> wbuf (nChars); 
    BeStringUtilities::Utf8ToWChar (wbuf.GetData(), s, nChars);
    m_s.append (wbuf.GetData());
    }
/*
void    WStringPrintSink::PutWChar (wchar_t c)      {m_s.append (1,c);}
void    WStringPrintSink::PutWCharCP (WCharCP s)    {m_s.append (s);}
*/
size_t  WStringPrintSink::GetCount() const          {return m_s.size();}

/*
void    Utf8PrintSink::PutWCharCP (WCharCP s)   
    {
    Utf8String u (s);
    PutCharCP (u.c_str());
    }

void    Utf8PrintSink::PutWChar (wchar_t c)  
    {
    if (c < 127) 
        Putc((char)c); 
    else 
        {
        wchar_t buf[2]; 
        buf[0]=c; 
        buf[1]=0; 
        Utf8String u (buf);
        PutCharCP (u.c_str());
        }
    }
*/

Utf8StringPrintSink::Utf8StringPrintSink (Utf8String& s) : m_s(s) {;}
void    Utf8StringPrintSink::PutChar (char c)       {m_s.append (1,c);}
void    Utf8StringPrintSink::PutCharCP (CharCP s)   {m_s.append (s);}
size_t  Utf8StringPrintSink::GetCount() const       {return m_s.size();}

ScanSource::ScanSource () : m_error(0)              {;}
void    ScanSource::OnError (int e)                 {m_error=e;};
int     ScanSource::GetError() const                {return m_error;}
int     ScanSource::Getc () 
    {
    int c;
    if (!m_pushedBack.empty())
        {
        c = m_pushedBack.back();
        m_pushedBack.pop_back();
        }
    else
        {
        c = GetNextChar();
        }
    return c;
    }

void    ScanSource::PutBack (char c) 
    {
    m_pushedBack.push_back(c);
    }

StringScanSource::StringScanSource (CharCP p) : m_string(p) {;}
int     StringScanSource::GetNextChar ()    {return *m_string? *m_string++: EOF;}

WStringScanSource::WStringScanSource (WCharCP p) : m_string(p) {;}
int     WStringScanSource::GetNextChar ()   {return *m_string? *m_string++: EOF;}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// scanf cover functions
static int doscanf (ScanSource& ss, CharCP fmt, va_list args, bool isNaturalWchar)
    {
    Scanner scanner (ss, const_cast<CharP>(fmt), isNaturalWchar);
    return scanner.DoScan (args);
    }

int BeStringUtilities::Sscanf (CharCP stringSource, CharCP fmt, ...)
    {
    va_list args;
    va_start (args, fmt);
    StringScanSource sss (stringSource);
    return doscanf (sss, fmt, args, false);
    }

int BeStringUtilities::Swscanf (WCharCP stringSource, WCharCP fmt, ...)
    {
    va_list args;
    va_start (args, fmt);
    Utf8String usource (stringSource);
    Utf8String ufmt    (fmt);
    StringScanSource sss (usource.c_str());
    return doscanf (sss, ufmt.c_str(), args, true);
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////
// printf cover functions
static int dovprintf (PrintSink& ps, CharCP fmt, va_list args, bool isNaturalWchar)
    {
    Formatter f (ps, const_cast<CharP>(fmt), isNaturalWchar);
    f.DoFormat (args);
    return ps.GetError() != 0? EOF: (int)ps.GetCount();
    }

int BentleyApi::BeUtf8StringSprintf (Utf8String& buffer, CharCP fmt, va_list ap, int /*initialLengthGuess*/)
    {
    buffer.clear();
    Utf8StringPrintSink ups (buffer);
    dovprintf (ups,fmt, ap, false);
    return (ups.GetError() != 0 ? EOF : (int)ups.GetCount());
    }

int BentleyApi::BeWStringSprintf (WString& buffer, WCharCP fmt, va_list ap, int /*initialLengthGuess*/)
    {
    buffer.clear();
    WStringPrintSink ups (buffer);
    dovprintf (ups,Utf8String(fmt).c_str(), ap, true);
    return (ups.GetError() != 0 ? EOF : (int)ups.GetCount());
    }
