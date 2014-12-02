/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/charutil/regex.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/Tools/msstrgop.h>
#include <DgnPlatform/Tools/stringop.h>

#define CHAR         1
#define BOL          2
#define EOL          3
#define ANY          4
#define CLASS        5
#define NCLASS       6
#define STAR         7
#define PLUS         8
#define MINUS        9
#define ALPHABETIC  10
#define DIGIT       11
#define NALPHABETIC 12
#define PUNCT       13
#define RANGE       14
#define ENDPAT      15

#define REGEX_ERROR     -2

/*----------------------------------------------------------------------+
|                                                                       |
|   Local type definitions                                              |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Private Global variables                                            |
|                                                                       |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
|   Public Global variables                                             |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   External variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Local function declarations                                         |
|                                                                       |
+----------------------------------------------------------------------*/
/*======================================================================+
|                                                                       |
|   Private Utility Routines                                            |
|                                                                       |
+======================================================================*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          charClass                                               |
|                                                                       |
| author        kab                             2/90                    |
|                                                                       |
+----------------------------------------------------------------------*/
static char    *charClass
(
char            *patbuf,                /* destination pattern buffer */
char            **sourcePtr
)
    {
    /* Compile a class (within []) */

    char    *patptr,    /* destination pattern pointer */
            *cp;        /* Pattern start     */
    int         c;      /* Current character */
    int         o;      /* Temp              */

    patptr = patbuf;

    if ((c = *((*sourcePtr)++)) == 0)
        {
        return  NULL;   /* class terminates badly */
        }
    else if (c == '^')
        {
        /* Class exclusion, for example: [^abc]
            Swallow the "^" and set token type to class exclusion. */
        o = NCLASS;
        }
    else
        {
        /* Normal class, for example: [abc]
         * push back the character and set token type to class */
        (*sourcePtr)--;
        o = CLASS;
        }

    *patptr++ = o & 0xff;

    cp = patptr;        /* remember where byte count is */
    *patptr++ = 0;      /* and initialize byte count */

    while ((c = *((*sourcePtr)++)) && c!=']')
        {
        o = *((*sourcePtr)++);          /* peek at next char */
        if (c == '\\')                  /* Store quoted chars */
            {
            if (o == -1) /* Gotta get something */
                return NULL;
            *patptr++ = o & 0xff;
            }
        else if (c=='-' && (patptr-cp)>1 && o!=']' && o != -1)
            {
            c = patptr[-1];             /* Range start     */
            patptr[-1] = RANGE;         /* Range signal    */
            *patptr++ = c & 0xff;              /* Re-store start  */
            *patptr++ = o & 0xff;              /* Store end char  */
            }
        else
            {
            *patptr++ = c & 0xff;              /* Store normal char */
            (*sourcePtr)--;
            }
        }

    if (c != ']')
        return NULL;

    if ((c = (int)(patptr - cp)) >= 256)
        return NULL; /* class too large */
    if (c == 0)
        return NULL; /* empty class */
    *cp = c & 0xff;            /* fill in byte count */

    return  patptr;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          partialMatch                                            |
|                                                                       |
| author        kab                             2/90                    |
|                                                                       |
|                                                                       |
| Modified by Gray Yu                           10/95                   |
|                                                                       |
+----------------------------------------------------------------------*/
static char    *partialMatch
(
char    *linestart,     /* start of line to match */
char    *line,          /* (partial) line to match      */
char    *pattern        /* (partial) pattern to match   */
)
    {
    char        *l;     /* Current line pointer         */
    char        *p;     /* Current pattern pointer      */
    char        c;      /* Current character            */
    char        *e;     /* End for STAR and PLUS match  */
    int         op;     /* Pattern operation            */
    int         n;      /* Class counter                */
    char        *are;   /* Start of STAR match          */

    l = line;

    p = pattern;
    while ((op = *p++) != ENDPAT)
        {
        switch(op)
            {
            case CHAR:
                if (*l++ != *p++) return NULL;
                break;

            case BOL:
                if (l != linestart && *(l-1) != 0x0D) return NULL;
                break;

            case EOL:
                if (*l != '\0' && *l != 0x0D) return NULL;
                break;

            case ANY:
                if (*l == 0x0D || !*l)
                    {
                    l++;
                    return NULL;
                    }
                l++;
                break;

            case DIGIT:
                c = *l++;
                if (!isdigit(c)) return NULL;
                break;

            case ALPHABETIC:
                c = *l++;
                if (!isalpha(c)) return NULL;
                break;

            case NALPHABETIC:
                c = *l++;
                if (!isalnum(c)) return NULL;
                break;

            case PUNCT:
                if (!(c = *l++) || c > ' ') return NULL;
                break;

            case CLASS:
            case NCLASS:
                c = *l++;
                n = *p++ & 0377;
                do
                    {
                    if (*p == RANGE)
                        {
                        p += 3;
                        n -= 2;
                        if (c >= p[-2] && c <= p[-1])
                            break;
                        }
                    else if (c == *p++)
                        break;
                    } while (--n > 1);

                if ((op == CLASS) == (n <= 1))
                    return NULL;
                if (op == CLASS) p += n - 2;
                    break;

#if defined (supportingMinus)
            case MINUS:
                e = partialMatch (linestart,l,p); /* Look for a match    */
                while (*p++ != ENDPAT);     /* Skip over pattern   */

                if (e)                  /* Got a match?        */
                    l = e;              /* Yes, update string  */
                break;                  /* Always succeeds     */
#endif

            case PLUS:                  /* One or more ...     */
                if (!(l = partialMatch (linestart,l,p)))
                    return NULL;
                                        /* Gotta have a match  */
            case STAR:                  /* Zero or more ...    */
                for (are=l; *l && (e = partialMatch (linestart,l,p)); l=e);
                                            /* Get longest match   */
                while (*p++ != ENDPAT); /* Skip over pattern   */
                do
                    {                   /* Try to match rest   */
                    if (e = partialMatch (linestart,l,p))
                        return e;
                    } while (l-- > are);
                return NULL;            /* Nothing else worked */

            default:
                return  (char *) -1;
            }
        }

    return  l;
    }

/*ff Major Public Code Section */
/*----------------------------------------------------------------------+
|                                                                       |
|   Major Public Code Section                                           |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          regex_compile                                           |
|                                                                       |
| author        kab                             2/90                    |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      regex_compile
(
char    *patbuf,                /* where to put compiled pattern */
char    *regexBuf               /* source for regular expression */
)
    {
    /* Compile a regular expression from current input file
        into the given pattern buffer. */

    int     c,          /* Current character         */
            o;          /* Temp                      */

    char    *patptr,    /* destination string pntr   */
            *lp = NULL,        /* Last pattern pointer      */
            *spp;       /* Save beginning of pattern */
    char    *sourcePtr;

    patptr      = patbuf;
    sourcePtr   = regexBuf;

    while (c = *(sourcePtr++))
        {
#if defined (supportingMinus)
        /* STAR, PLUS and MINUS are special. */
        if (c == '*' || c == '+' || c == '-')
#else
        /*  I do not believe anyone will care about having all 3
            options.  I am removing the '-' option so I do
            not have to document it. Leaving it in but not
            documenting it can cause suprising results.

                                        JBG
        */
        if (c == '*' || c == '+')
#endif
            {
            if (patptr == patbuf ||
                      (o=patptr[-1]) == BOL ||
                      o == EOL ||
                      o == STAR ||
                      o == PLUS ||
                      o == MINUS)
                return REGEX_ERROR;

            *patptr++ = ENDPAT;
            *patptr++ = ENDPAT;

            spp = patptr;               /* Save pattern end     */
            while (--patptr > lp)       /* Move pattern down... */
                    *patptr = patptr[-1];       /* one byte     */

#if defined (supportingMinus)
            *patptr =   (c == '*') ? STAR :
                        (c == '-') ? MINUS : PLUS;
#else
            *patptr =   (c == '*') ? STAR : PLUS;
#endif
            patptr = spp;               /* Restore pattern end  */

            continue;
            }

        lp = patptr;                    /* Remember start */
        switch (c)
            {
            case '^':
                *patptr++ = BOL;
                break;

            case '$':
                *patptr++ = EOL;
                break;

            case '.':
                *patptr++ = ANY;
                break;

            case '[':
                if ((patptr = charClass (patptr, &sourcePtr)) == NULL)
                    return REGEX_ERROR;
                break;

            case ':':
                if (c = *(sourcePtr++))
                    {
                    switch (tolower(c))
                        {
                        case 'a':
                            *patptr++ = ALPHABETIC;
                            break;

                        case 'd':
                            *patptr++ = DIGIT;
                            break;

                        case 'n':
                            *patptr++ = NALPHABETIC;
                            break;

                        case ' ':
                            *patptr++ = PUNCT;
                            break;

                        default:
                            return  REGEX_ERROR;
                        }
                    }
                else
                    return REGEX_ERROR;
                break;

            case '\\':
                c = *(sourcePtr++);

            default:
                *patptr++ = CHAR;
                *patptr++ = c & 0xff;
            }
        }

    *patptr++ = ENDPAT;
    *patptr++ = 0;          /* Terminate string */

    return (int) (patptr - patbuf);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          regex_match                                             |
|                                                                       |
| author        kab                             2/90                    |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      regex_match
(
char    *line,          /* line to match */
char    *pattern,       /* pattern to match */
char    **start,        /* start of matched pattern */
char    **end           /* end of match */
)
    {
    /* Match the line return 1 if it does. */

    char        *l;             /* Line pointer       */
    char        *next;

    for (l = line; *l; l++)
        {
        if (next = partialMatch (line, l, pattern))
            {
            if (next == (char *) -1)
                return  REGEX_ERROR;

            if (start)
                *start = l;
            if (end)
                *end   = next;
            return  0;
            }
        }

    if (*l == '\0' && *pattern == EOL)
        {
        if (end)
            *end   = l;
        if (start)
            *start = l;
        return  0;
        }

    return  1;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          wcharClass                                              |
|                                                                       |
| author        PaulChater                             03/01            |
|                                                                       |
+----------------------------------------------------------------------*/
static WCharP wcharClass
(
WCharP     patbuf,                /* destination pattern buffer */
WCharCP*    sourcePtr
)
    {
    /* Compile a class (within []) */

    WCharP  cp;        /* Pattern start     */
    WCharP  patptr;    /* destination pattern pointer */
            
    int     c,          /* Current character */
            o;          /* Temp              */

    patptr = patbuf;

    if ((c = *((*sourcePtr)++)) == 0)
        {
        return  NULL;   /* class terminates badly */
        }
    else if (c == '^')
        {
        /* Class exclusion, for example: [^abc]
            Swallow the "^" and set token type to class exclusion. */
        o = NCLASS;
        }
    else
        {
        /* Normal class, for example: [abc]
         * push back the character and set token type to class */
        (*sourcePtr)--;
        o = CLASS;
        }

    *patptr++ = o & 0xff;

    cp = patptr;        /* remember where byte count is */
    *patptr++ = 0;      /* and initialize byte count */

    while ((c = *((*sourcePtr)++)) && c!=']')
        {
        o = *((*sourcePtr)++);          /* peek at next char */
        if (c == '\\')                  /* Store quoted chars */
            {
            if (o == -1) /* Gotta get something */
                return NULL;
            *patptr++ = o & 0xffff;
            }
        else if (c=='-' && (patptr-cp)>1 && o!=']' && o != -1)
            {
            c = patptr[-1];             /* Range start     */
            patptr[-1] = RANGE;         /* Range signal    */
            *patptr++ = c & 0xffff;              /* Re-store start  */
            *patptr++ = o & 0xffff;              /* Store end char  */
            }
        else
            {
            *patptr++ = c & 0xffff;              /* Store normal char */
            (*sourcePtr)--;
            }
        }

    if (c != ']')
        return NULL;

    if ((c = (int)(patptr - cp)) >= 256)
        return NULL; /* class too large */
    if (c == 0)
        return NULL; /* empty class */
    *cp = c & 0xffff;            /* fill in byte count */

    return  patptr;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          partialMatch                                            |
|                                                                       |
| author        PaulChater                             03/01            |
|                                                                       |
|                                                                       |
| Modified by Gray Yu                           10/95                   |
|                                                                       |
+----------------------------------------------------------------------*/
static WCharCP wpartialMatch
(
WCharCP linestart,     /* start of line to match */
WCharCP line,          /* (partial) line to match      */
WCharCP pattern        /* (partial) pattern to match   */
)
    {
    WCharCP     l;     /* Current line pointer         */
    WCharCP     p;     /* Current pattern pointer      */
    WChar     c;      /* Current MSWCharacter         */
    WCharCP     e;     /* End for STAR and PLUS match  */
    int         op;     /* Pattern operation            */
    int         n;      /* Class counter                */
    WCharCP     are;   /* Start of STAR match          */

    l = line;

    p = pattern;
    while ((op = *p++) != ENDPAT)
        {
        switch(op)
            {
            case CHAR:
                if (*l++ != *p++) return NULL;
                break;

            case BOL:
                if (l != linestart && *(l-1) != 0x0D) return NULL;
                break;

            case EOL:
                if (*l != '\0' && *l != 0x0D) return NULL;
                break;

            case ANY:
                if (*l == 0x0D || !*l)
                    {
                    l++;
                    return NULL;
                    }
                l++;
                break;

            case DIGIT:
                c = *l++;
                if (!iswdigit(c)) return NULL;
                break;

            case ALPHABETIC:
                c = *l++;
                if (!iswalpha(c)) return NULL;
                break;

            case NALPHABETIC:
                c = *l++;
                if (!iswalnum(c)) return NULL;
                break;

            case PUNCT:
                if (!(c = *l++) || c > ' ') return NULL;
                break;

            case CLASS:
            case NCLASS:
                c = *l++;
                n = *p++ & 0377;
                do
                    {
                    if (*p == RANGE)
                        {
                        p += 3;
                        n -= 2;
                        if (c >= p[-2] && c <= p[-1])
                            break;
                        }
                    else if (c == *p++)
                        break;
                    } while (--n > 1);

                if ((op == CLASS) == (n <= 1))
                    return NULL;
                if (op == CLASS) p += n - 2;
                    break;

#if defined (supportingMinus)
            case MINUS:
                e = wpartialMatch (linestart,l,p); /* Look for a match   */
                while (*p++ != ENDPAT);     /* Skip over pattern   */

                if (e)                  /* Got a match?        */
                    l = e;              /* Yes, update string  */
                break;                  /* Always succeeds     */
#endif

            case PLUS:                  /* One or more ...     */
                if (!(l = wpartialMatch (linestart,l,p)))
                    return NULL;
                                        /* Gotta have a match  */
            case STAR:                  /* Zero or more ...    */
                for (are=l; *l && (e = wpartialMatch (linestart,l,p)); l=e);
                                            /* Get longest match   */
                while (*p++ != ENDPAT); /* Skip over pattern   */
                do
                    {                   /* Try to match rest   */
                    if (e = wpartialMatch (linestart,l,p))
                        return e;
                    } while (l-- > are);
                return NULL;            /* Nothing else worked */

            default:
                return  (WChar *) -1;
            }
        }

    return  l;
    }

/*ff Major Public Code Section */
/*----------------------------------------------------------------------+
|                                                                       |
|   Major Public Code Section                                           |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          regex_compile                                           |
|                                                                       |
| author        PaulChater                             03/01            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      regexw_compile
(
WStringR    patbufStr,             /* where to put compiled pattern */
WCharCP     regexBuf               /* source for regular expression */
)
    {
    // BEIJING_DGNPLATFORM_WIP
    //  The loop below would not be trivial to make WString-friendly.
    //  Old code would simply pass a 512-character buffer in, so this is at least one step up from that.
    ScopedArray<WChar>  patbufBuff  (std::max<size_t> (512, 2 * wcslen (regexBuf)));
    WCharP              patbuf      = patbufBuff.GetData ();
    
    /* Compile a regular expression from current input file
        into the given pattern buffer. */

    int     c,          /* Current character         */
            o;          /* Temp                      */

    WChar    *patptr, /* destination string pntr   */
            *lp = NULL,        /* Last pattern pointer      */
            *spp;       /* Save beginning of pattern */
    WCharCP sourcePtr;

    patptr      = patbuf;
    sourcePtr   = regexBuf;

    while (c = *(sourcePtr++))
        {
#if defined (supportingMinus)
        /* STAR, PLUS and MINUS are special. */
        if (c == '*' || c == '+' || c == '-')
#else
        /*  I do not believe anyone will care about having all 3
            options.  I am removing the '-' option so I do
            not have to document it. Leaving it in but not
            documenting it can cause suprising results.

                                        JBG
        */
        if (c == '*' || c == '+')
#endif
            {
            if (patptr == patbuf ||
                      (o=patptr[-1]) == BOL ||
                      o == EOL ||
                      o == STAR ||
                      o == PLUS ||
                      o == MINUS)
                return REGEX_ERROR;

            *patptr++ = ENDPAT;
            *patptr++ = ENDPAT;

            spp = patptr;               /* Save pattern end     */
            while (--patptr > lp)       /* Move pattern down... */
                    *patptr = patptr[-1];       /* one byte     */

#if defined (supportingMinus)
            *patptr =   (c == '*') ? STAR :
                        (c == '-') ? MINUS : PLUS;
#else
            *patptr =   (c == '*') ? STAR : PLUS;
#endif
            patptr = spp;               /* Restore pattern end  */

            continue;
            }

        lp = patptr;                    /* Remember start */
        switch (c)
            {
            case '^':
                *patptr++ = BOL;
                break;

            case '$':
                *patptr++ = EOL;
                break;

            case '.':
                *patptr++ = ANY;
                break;

            case '[':
                if ((patptr = wcharClass (patptr, &sourcePtr)) == NULL)
                    return REGEX_ERROR;
                break;

            case ':':
                if (c = *(sourcePtr++))
                    {
                    switch (towlower((wint_t)c))
                        {
                        case 'a':
                            *patptr++ = ALPHABETIC;
                            break;

                        case 'd':
                            *patptr++ = DIGIT;
                            break;

                        case 'n':
                            *patptr++ = NALPHABETIC;
                            break;

                        case ' ':
                            *patptr++ = PUNCT;
                            break;

                        default:
                            return  REGEX_ERROR;
                        }
                    }
                else
                    return REGEX_ERROR;
                break;

            case '\\':
                c = *(sourcePtr++);

            default:
                *patptr++ = CHAR;
                *patptr++ = c & 0xffff;
            }
        }

    *patptr++ = ENDPAT;
    *patptr++ = 0;          /* Terminate string */

    patbufStr = patbufBuff.GetData ();

    return (int)(patptr - patbuf);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          regexw_match                                            |
|                                                                       |
| author        PaulChater                             03/01            |
|                                                                       |
+----------------------------------------------------------------------*/
Public int      regexw_match
(
WCharCP     lineStart,     /* start of line */
WCharCP     line,          /* line to match */
WCharCP     pattern,       /* pattern to match */
WCharCP*    start,        /* start of matched pattern */
WCharCP*    end           /* end of match */
)
    {
    /* Match the line return 1 if it does. */

    WCharCP   l;             /* Line pointer       */
    WCharCP   next;

    for (l = line; *l; l++)
        {
        if (next = wpartialMatch (lineStart, l, pattern))
            {
            if (next == (WChar *) -1)
                return  REGEX_ERROR;

            if (start)
                *start = l;
            if (end)
                *end   = next;
            return  0;
            }
        }

    if (*l == '\0' && *pattern == EOL)
        {
        if (end)
            *end   = l;
        if (start)
            *start = l;
        return  0;
        }

    return  1;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          matchString                                             |
|                                                                       |
| author        PaulChater                             03/01            |
|                                                                       |
+----------------------------------------------------------------------*/
Public StatusInt mdlString_matchRE
(
WCharCP     string,
WCharCP     regex,
WCharCP*    start,
WCharCP*    end
)
    {
    WString compiled;
    int     status          = regexw_compile (compiled, regex);
    
    if (!status || REGEX_ERROR == status)
        return (2);

    return (regexw_match (string, string, compiled.c_str (), start, end));
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          getLastWChar                                            |
|                                                                       |
| author        PaulChater                             03/01            |
|                                                                       |
+----------------------------------------------------------------------*/
static WChar   getLastWChar
(
WCharCP        string
)
    {
    WCharCP     ptr = string;

    while (*ptr != ENDPAT)
            ptr++;

    /*-------------------------------------------------------------------
    Since this examines a compiled string it should always be okay to look
    back 1 character.
    -------------------------------------------------------------------*/
    ptr--;
    return *ptr;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlString_matchREExtended                           |
|                                                                       |
| author        PaulChater                             03/01            |
|                                                                       |
+----------------------------------------------------------------------*/
Public StatusInt mdlString_matchREExtended
(
WCharCP     string,
WCharCP     regex,
WCharCP*    start,
WCharCP*    end,
int*        stopState
)
    {
    return mdlString_matchREExtended2 (string, string, regex, start, end, stopState);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlString_matchREExtended2                          |
|                                                                       |
| author        PaulChater                             03/01            |
|                                                                       |
+----------------------------------------------------------------------*/
Public StatusInt mdlString_matchREExtended2
(
WCharCP     lineStart,
WCharCP     line,
WCharCP     regex,
WCharCP*    start,
WCharCP*    end,
int*        stopState
)
    {
    WString  compiled;
    WChar   firstChar;
    WChar   lastChar;
    int     status;

    if (*stopState != 0)
        return 0;

    status = regexw_compile (compiled, regex);
    if (!status || REGEX_ERROR == status)
        return (2);

    firstChar = compiled[0];

    if (firstChar == BOL)
        *stopState |= 1;
    else
        {
        lastChar = getLastWChar (compiled.c_str ());
        if (lastChar == EOL)
            *stopState |= 2;
        }

    return (regexw_match (lineStart, line, compiled.c_str (), start, end));
    }

