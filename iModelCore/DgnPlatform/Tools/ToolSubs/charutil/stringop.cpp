/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/charutil/stringop.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <algorithm>
#include    <stdlib.h>
#include    <ctype.h>
#include    <string.h>
#include    <stdio.h>
#include    <wchar.h>
#include    <DgnPlatform/Tools/stringop.h>


/*---------------------------------------------------------------------------------**//**
* different from isspace since it does not return true for '\n' or '\r'.
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool          isSpaceOrTab
(
int     c
)
    {
    return (c==' ' || c=='\t');
    }

/*---------------------------------------------------------------------------------**//**
* compares two strings and returns index where they are first unequal (or one of them ends)
* @bsimethod                                                    BJB             12/85
+---------------+---------------+---------------+---------------+---------------+------*/
Public int   wstrcomp
(
WCharCP     str1,
WCharCP     str2
)
    {
    size_t  i, len = wcslen (str1);

    if (wcslen(str2) < len)
        len = wcslen(str2);
    for (i=0; i<len; i++)
        {
        if (*str1++ != *str2++)
            break;
        }
    return (int) i;
    }

/*---------------------------------------------------------------------------------**//**
* steps past white space in string
* @bsimethod                                                    BJB             02/86
+---------------+---------------+---------------+---------------+---------------+------*/
Public WChar *wstpblk
(
WCharCP p
)
    {
    while (*p == ' ' || *p == '\t') p++;
    return const_cast<WCharP>(p);
    }

/*---------------------------------------------------------------------------------**//**
* returns pointer to end of string
* @bsimethod                                                    BJB             04/88
+---------------+---------------+---------------+---------------+---------------+------*/
Public WCharP    wend_string
(
WCharCP input
)
    {
    return const_cast<WCharP>(input + wcslen(input));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public WCharCP  findSubstring
(
WCharCP         masterStringP,
WCharCP         searchStringP
)
    {
    size_t   masterLen, searchLen;

    for (masterLen = wcslen (masterStringP), searchLen = wcslen (searchStringP); masterLen >= searchLen; masterLen--, masterStringP++)
        {
        if (0 == wcsncmp (masterStringP, searchStringP, searchLen))
            return masterStringP;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* string pointer copy (copies between two pointers)
* @bsimethod                                                    Keith.Bentley   10/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  wstrpcpy
(
WChar *result,
WChar **p,
WChar *end
)
    {
    WChar    *s;

    /* if second pointer is NULL, null terminate output string */
    if (end<*p)
        {
        *result = '\0';
        return;
        }

    /* copy between two pointers */
    for (s = *p; s<end; *(result++) = *(s++));

    /* null terminate string */
    *result = '\0';

    /* update start pointer */
    *p= (WChar *) (end+1);
    }

/*---------------------------------------------------------------------------------**//**
* synopsis trim (field)
* @bsimethod                                                    KAB             10/84
+---------------+---------------+---------------+---------------+---------------+------*/
//Public void  wtrim removed in graphite - unused
//(
//WChar *field
//)
//    {
//    int    i;
//    for (i=(int)wcslen(field)-1; ((i>=0) && *(field+i)==' '); i--);
//    *(field+i+1) = '\0';
//    }

/*---------------------------------------------------------------------------------**//**
* author kab .
* Use skipBlankTab if you do not want to skip newlines. isspace returns true to newlines.
* @bsimethod                                                                         
+---------------+---------------+---------------+---------------+---------------+------*/
Public WCharP       wskipSpace
(
WCharCP string
)
    {
    WCharCP p;

    for (p=string; *p && iswspace(*p); p++)
        {}
    return  const_cast<WCharP>(p);
    }


/*---------------------------------------------------------------------------------**//**
* skipSpace also skips newlines. This does not
* @bsimethod                                                    John.Gooding    10/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public WChar *wskipBlankTab
(
WChar *stringP
)
    {
    while (*stringP == '\t' || *stringP == ' ')
        stringP++;
    return stringP;
    }

/*---------------------------------------------------------------------------------**//**
* author kab .
* Strip leading and trailing spaces.
* @bsimethod                                                                         
+---------------+---------------+---------------+---------------+---------------+------*/
Public void  wstripSpace
(
WChar *string
)
    {
    WChar    *p;

    wcscpy (string, wskipSpace(string));
    for (p=string+wcslen(string)-1; p>string && iswspace(*p); *(p--) = '\0')
        {}
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Chater     11/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public void strutil_wstrpwspc
(
WChar     *chrstr
)
    {
    WChar    *p;

    if  (chrstr == NULL)
        return;

    for (p=chrstr; iswspace(*p); p++)
        ;

    if (p != chrstr)
        wcscpy (chrstr, p);

    for (p=wend_string(chrstr)-1; iswspace(*p) && p>chrstr; *p--=0)
        ;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Chater     11/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public int   mdlwcscmpi
(
const WChar     *p1,
const WChar     *p2
)
    {
    /* BeStringUtilities::Wcsicmp doesn't like null wide char strings */
    if (!p1 && !p2)
        return 0;
    else if (!p1)
        return -1;
    else if (!p2)
        return 1;
    else
        return BeStringUtilities::Wcsicmp (p1, p2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kiran.Hebbar    03/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public WChar* mdlwcsupr
(
WChar     *wideStringP
)
    {
    return BeStringUtilities::Wcsupr (wideStringP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kiran.Hebbar    03/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public WChar* mdlwcslwr
(
WChar     *wideStringP
)
    {
    return BeStringUtilities::Wcslwr (wideStringP);
    }


#if !defined (DLM_INSTANCE)
/*---------------------------------------------------------------------------------**//**
* returns pointer to character just after the first occurrance of specified char. Zeros character found character. Returns pointer to NULL if
* char not found
* @bsimethod                                                    kab             06/86
+---------------+---------------+---------------+---------------+---------------+------*/
Public WCharP  wcsnxtchr
(
WCharP   str,
WChar    chr
)
    {
    WCharP    p;

    if ((p=::wcschr (str, chr)) == NULL)
        return (wend_string(str));

    *p = 0;
    return (p+1);
    }

#endif

/*---------------------------------------------------------------------------------**//**
* author John.Gooding 06/05
* @bsimethod                                                                         
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     wstripLeadingZero
(
WChar     *wstr
)
    {
    if (*wstr != '0' || 0 == *(wstr+1))
        return;

    wstr++;
    while (0 != *wstr)
        {
        *(wstr-1) = *wstr;
        wstr++;
        }

    *(wstr-1) = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             09/88
+---------------+---------------+---------------+---------------+---------------+------*/
Public void wstripTrailingZeros
(
WChar     *wstr
)
    {
    WChar    *p;

    for (p=wend_string(wstr)-1; *p=='0' && p>wstr; *p--=0);
    if (*p == '.') *p = '\0';
    }

/*---------------------------------------------------------------------------------**//**
* convert string to radix 50
* @bsimethod                                                    BJB             02/86
+---------------+---------------+---------------+---------------+---------------+------*/
void  asciiToR50
(
size_t           length,  /* => length of input string to be converted */
CharCP           input,   /* => string to be converted                 */
void*            outputP  /* <= output array                           */
)
    {
    size_t       i;
    short        c;
    short       *output = (short *)outputP;

    for (i=0, *output=0; i<length; i++)
        {
        *output *= 40;
        c = *input++;

        if (c >= 'A' && c <= 'Z')       *output += c - 'A' + 1;
        else if (c >= '0' && c <= '9')  *output += c - '0' + 30;
        else if (c == '$')              *output += 27;
        else if (c == '.')              *output += 28;
        else if (c == '_')              *output += 29;

        if (((i+1)%3 == 0) && ((i+1) < length))
            {
            output++;
            *output = 0;
            }
        }

    for (; i%3; i++)
        *output *= 40;
    }

/*---------------------------------------------------------------------------------**//**
* converts radix 50 input to WChar string output
* @bsimethod                                                    Paul.Chater     11/00
+---------------+---------------+---------------+---------------+---------------+------*/
void  R50ToWide
(
size_t          length,         /* => maximum length of output string
                                    (excluding terminator). Should be multiple of 3 */
const void*     inputP,         /* => radix 50 to be converted */
WCharP          output          /* <= output string */
)
    {
    size_t          i, j;
    unsigned short  temp1, temp2;
    WCharP          out;
    WCharP          origOutput = output;
    short           *input = (short *)inputP;

    for (i=0; i<(length+2)/3; i++)
        {
        temp1 = *input++;

        for (j=0, out=output+2; j<3; j++)
            {
            temp2 = temp1 % 40;
            temp1 /= 40;

            if (temp2 == 0) *out-- = ' ';
            else if (temp2 <= 26) *out-- = 'A' - 1 + temp2;
            else if (temp2 == 27) *out-- = '$';
            else if (temp2 == 28) *out-- = '.';
            else if (temp2 == 29) *out-- = '_';
            else *out-- = temp2 + '0' - 30;
            }
        output += 3;
        }
    *output = '\0';

    // strip white space from beginning and end of wide output string.
    strutil_wstrpwspc (origOutput);
    }


