/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ValueParse.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

BEGIN_BENTLEY_DGN_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* NOTE: This code was brought up from MstnPlatform to preserve the old behavior until it is refactored correctly. 
*    I prefixed everything with vp_ so it is clear that it is coming from here. 
*    This stuff either needs refactored into an API that the MstnPlatform methods wrap or it needs to just be coded into the new API.
*
+---------------+---------------+---------------+---------------+---------------+------*/
const double    fc_90           = 90.0;
const double    fc_180          = 180.0;
const double    fc_270          = 270.0;

/*---------------------------------------------------------------------------------**//**
* adds the value calculated from the input string to the current value. The "units" parameter specifies a multiplier for the result.
* Returns ERROR if syntax error
* @bsimethod                                                    kab             06/86
+---------------+---------------+---------------+---------------+---------------+------*/
static int     vp_AddNumberStringScaled
(
double&         value,          // <= result that inputString is added to.
size_t&         szParsed,
bool&           negative,       // <= set if there is a minus sign in string */
WCharP          inputString,    // => input string */
double          scale           // => units */
)
    {
    double      integerPart = 0.0;
    double      numerator   = 0.0;
    double      denominator = 1.0;
    WCharP      denominatorString;
    WCharP      numeratorString;
    szParsed = 0;
    if (L'-' == inputString[0])         /* check for minus sign */
        {
        negative = true;
        inputString++;
        szParsed++;
        }

    denominatorString = wcsnxtchr (inputString, L'/');        /* check for fractions */
    numeratorString   = wcsnxtchr (inputString, L' ');        /* integer part */

    if (0 != denominatorString[0])
        {
        if (BE_STRING_UTILITIES_SWSCANF(denominatorString, L"%lf", &denominator) != 1)
            return (ERROR);
        }

    if (0 != inputString[0])
        {
        if (BE_STRING_UTILITIES_SWSCANF(inputString, L"%lf", &integerPart) != 1)
            return (ERROR);
        }

    if ( (0 != numeratorString[0]) && (0 != denominatorString[0]) )
        {
        if (BE_STRING_UTILITIES_SWSCANF(numeratorString, L"%lf", &numerator) != 1)
            return (ERROR);
        }
    else
        {
        numerator   = integerPart;
        integerPart = 0.0;
        }

    if (0.0 == denominator)   /* never divide by zero */
        return (ERROR);

    value += ((integerPart + (numerator/denominator)) * scale);

    return (SUCCESS);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
static int     vp_AddNumberStringScaledNoModify
(
double&         value,          // <= result that inputString is added to.
size_t&         szParsed,
bool&           negative,       // <= set if there is a minus sign in string */
WCharCP         input,    // => input string */
double          scale           // => units */
)
    {
    size_t l = wcslen (input) + 1;
    WCharP buffer = (WCharP) _alloca(l * sizeof (WChar));
    wcscpy (buffer, input);

    return vp_AddNumberStringScaled (value, szParsed, negative, buffer, scale);
    }

/*------------------------------------------------------------------------------------**/
/// <summary>Overrides ConvertFrom to return the right type</summary>
/// <author>Barry.Bentley</author>                              <date>09/2004</date>
/*--------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   vp_degreeAngleFromString
(
double&         angle,
WCharCP       inValue
)
    {
    // we want to handle the MicroStation style keyins:
    // 30^20'10", or 30:20:10, or 30d20m10s or 30(0xb0)20'10".
    // If there's a trailing W or S, the value is negative.
    // The trailing " or s is not necessary.
    // 30^10" is the same as 30^0'10"
    // Any of the degrees, minutes, seconds can have decimal within, although of course
    //  it is not recommended to put in something like 30.5^30'10" - that results in 31^0'10"

    // white space has no significance, remove it.
    angle             = 0.0;
    size_t szParsed = 0;
    WChar   value[1024];
    wcscpy (value, inValue);
    strutil_wstrpwspc (value);

    bool    negative    = false;
    size_t  strLength   = wcslen (value);

    // an empty string is interpreted as 0.0;
    if (strLength < 1)
        return SUCCESS;

    // if last character is W or S, it's negative, truncate it. If last character is N or E, it's positive, still truncate it.
    int     lastChar            = value[strLength-1];
    if ( (L'W' == lastChar) || (L'w' == lastChar) || (L'S' == lastChar) || (L's' == lastChar) )
        {
        negative = true;
        value[strLength-1] = 0;
        }
    else if ( (L'N' == lastChar) || (L'n' == lastChar) || (L'E' == lastChar) || (L'e' == lastChar) )
        {
        value[strLength-1] = 0;
        }

    WChar     degreeDelimiters[] = {L'^', L':', L'd', L'D', 0x00b0 /*degree*/, 0 };
    WChar     minuteDelimiters[] = {L'\'', L':', L'm', L'M', 0};
    WChar     secondDelimiters[] = {L'"', L':', L's', L'S', 0};
    WCharP    degreeString        = value;
    WCharP    minuteString        = NULL;
    WCharP    secondString        = NULL;

    size_t delimiterIndex = wcscspn (degreeString, degreeDelimiters);
    // is there really a degree delimiter?
    if (0 != degreeString[delimiterIndex])
        {
        degreeString[delimiterIndex] = 0;
        minuteString = &degreeString[delimiterIndex+1];
        }
    else
        {
        // no degrees delimiter
        minuteString = degreeString;
        }

    delimiterIndex = wcscspn (minuteString, minuteDelimiters);
    // is there really a minute delimiter?
    if (0 != minuteString[delimiterIndex])
        {
        // Found a minutes delimiter. If that means that what we thought might be degrees was actually minutes, correct that.
        if (degreeString == minuteString)
            degreeString = NULL;

        minuteString[delimiterIndex] = 0;
        secondString = &minuteString[delimiterIndex+1];
        }
    else
        {
        // no minutes delimiter.
        secondString = minuteString;
        }

    delimiterIndex = wcscspn (secondString, secondDelimiters);
    if (0 != secondString[delimiterIndex])
        {
        // Found a seconds delimiter. If that means that what we thought might be degrees was actually seconds, correct that.
        if (minuteString == secondString)
            minuteString = NULL;
        // Found a seconds delimiter. If that means that what we thought might be minutes was actually seconds, correct that.
        if (degreeString == secondString)
            degreeString = NULL;

        secondString[delimiterIndex] = 0;
        }
    else
        {
        // no seconds delimiter.
        if (secondString == minuteString)
            secondString = NULL;
        if (minuteString == degreeString)
            minuteString = NULL;
        }

    bool    numberNegative  = false;;
    double  degrees         = 0.0;

    WChar acceptableChars[] = L"0123456789.e+-/ ";

    if ( (NULL != degreeString) && (0 != *degreeString) )
        {
        if (0 != degreeString[wcsspn (degreeString, acceptableChars)])
            return ERROR;
        vp_AddNumberStringScaled (degrees, szParsed, numberNegative, degreeString, 1.0);
        }

    if ( (NULL != minuteString) && (0 != *minuteString) )
        {
        if (0 != minuteString[wcsspn (minuteString, acceptableChars)])
            return ERROR;
        vp_AddNumberStringScaled (degrees, szParsed, numberNegative, minuteString, 1.0/60.0);
        }

    if ( (NULL != secondString) && (0 != *secondString) )
        {
        if (0 != secondString[wcsspn (secondString, acceptableChars)])
            return ERROR;
        vp_AddNumberStringScaled (degrees, szParsed, numberNegative, secondString, 1.0/3600.0);
        }

    if (negative ^ numberNegative)
        degrees = -1.0 * degrees;

    angle = degrees;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/08
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    vp_parseAsDegreeString (AngleMode mode, WCharCP inputString)
    {
    if ( (AngleMode::Centesimal != mode) && (AngleMode::Radians != mode) )
        return true;

    WChar allDelimiters[] = {L'^', L':', L'd', L'D', 0x00b0 /*degree*/, L'\'', L'm', L'M', L'"', L's', L'S', 0};
    size_t  delimiterIndex  = wcscspn (inputString, allDelimiters);

    return (0 != inputString[delimiterIndex]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
static void  vp_wgetsubstr
(
WChar **outpntr,          /* <= set to beginning of substring */
WChar **inpntr,           /* <=> pointer to start of string, adjusted */
WChar brkchar             /* => break character */
)
    {
    WChar *p;

    *outpntr = *inpntr;
    if ((p = ::wcschr (*inpntr, brkchar)) == NULL)
        *inpntr += wcslen (*inpntr);
    else
        {
        *inpntr = p+1;
        *p = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* convert a string in MU:SU:PU format to a floating point value.
* This was changed from the original - does not modify the incoming string.
* @bsimethod                                                    kab             06/86
+---------------+---------------+---------------+---------------+---------------+------*/
static int      vp_mdlString_toUors2
(
double*         uor,
size_t&         szParsed,
WCharCP         inStr,
WCharCP         masterUnitsLabel,
WCharCP         subUnitsLabel,
double          uorPerMast,
double          uorPerSub
)
    {
    if (NULL == inStr)
        return ERROR;
    szParsed = 0;
    WChar buffer[1024];
    WChar *mu_str, *su_str, *pu_str, *p;

    bool            negflg;
    BeAssert (wcslen (inStr) < 1024);
    wcscpy (buffer, inStr);
    WCharP str = buffer;

    /* skip leading whitespace */
    while (*str == L' ')
        {
        str++;
        szParsed++;
        }
    WCharP start = str;

    if (0 == masterUnitsLabel[0])
        p = NULL;
    else
        p = ::wcsstr (str, masterUnitsLabel);
    bool doMuSu = true; 
    if (NULL != p)
        {
        /* allow units labels, for example 1'-2.0000" format */
        WChar *pNext = ::wcschr (p, L'-');
        if (NULL == pNext)
            {
            pNext = p + wcslen (masterUnitsLabel);
            szParsed += wcslen (masterUnitsLabel);
            }
        else
            {
            pNext++;
            szParsed++;
            }
        *p = L':';
        for (p++; *pNext; p++, pNext++)
            *p = *pNext;
        *p = 0;
        /* remove sub units label */
        if (0 != *subUnitsLabel)
            {
            p = ::wcsstr (str, subUnitsLabel);
            szParsed += wcslen (subUnitsLabel);

            if (NULL != p)
                *p = 0;
            }
        }
    else if (NULL != (p = ::wcschr(str, L'E')))
        {
        /* scientific format */
        double  value = BeStringUtilities::Wtof (str);
        doMuSu = false;
        WCharP space = ::wcschr (str, L' ');
        if (NULL != space)
            szParsed += space - str;
        else
            szParsed += wcslen (str);

        if (*(p+1) == L'-')
            {
            int     precision = 0;
            WChar format[48];

            p += 2;
            while ('0' == *p)
                p++;

            if (0 != *p)
                precision = BeStringUtilities::Wtoi (p);
            if (precision < 10)
                precision = 10;
            else if  (precision > 120)
                precision = 120;

            BeStringUtilities::Snwprintf (format, _countof(format), L"%%.%df", precision+5);
            BeStringUtilities::Snwprintf (str, _countof(buffer), format, value);
            }
        else
            {
            BeStringUtilities::Snwprintf (str, _countof(buffer), L"%f", value);
            }
        }
    else
        {
        /* allow ";" as synonym for ':' */
        for (p=str; *p; p++)
            if (*p==L';') *p=L':';

        /* allow ".." as synonym for ';' */
        for (p=str; *p; p++)
            if (p[0] == L'.' && p[1] == L'.')
                {
                WChar *p0, *p1;

                szParsed++;
                *p++ = L':';
                for (p0 = p, p1 = p + 1; *p0; *p0++ = *p1++)
                    ;
                }
        }
    vp_wgetsubstr (&mu_str, &str, L':');
    vp_wgetsubstr (&su_str, &str, L':');
    vp_wgetsubstr (&pu_str, &str, L':');

    if (doMuSu)
        szParsed += str - start;

    *uor = 0;
    negflg = false;

    size_t szNotUsed = 0;
    if (vp_AddNumberStringScaled (*uor, szNotUsed, negflg, mu_str, uorPerMast) ||
        vp_AddNumberStringScaled (*uor, szNotUsed, negflg, su_str, uorPerSub)  ||
        vp_AddNumberStringScaled (*uor, szNotUsed, negflg, pu_str, 1.0))
        return(ERROR);

    if (negflg)
        *uor = - *uor;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
static bool vp_cmdStartsWith (WCharCP cmd, WCharCP in)
    {
    WCharCP a = cmd;
    WCharCP b = in;
    for (;*a && *b; ++a, ++b)
        {
        if (*a != *b)
            return false;
        }
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
static WCharCP vp_removeWhiteSpaceAndCount (size_t& count, WCharCP str)
    {
    count = 0;
    while (str && L' ' == *str)
        {
        ++str;
        ++count;
        }
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DoubleParserPtr         DoubleParser::Create ()             { return new DoubleParser(); }
DoubleParserPtr         DoubleParser::Clone() const         { return new DoubleParser (*this); }
/* ctor */              DoubleParser::DoubleParser ()       { }
/* ctor */              DoubleParser::DoubleParser(DoubleParserCR source)     { }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DoubleParser::ToValue (double& outValue, WCharCP input) const
    {
    outValue = 0.0;
    if (NULL == input || 0 == *input)
        return SUCCESS;

    size_t numCharsParsed = 0;
    size_t whiteSpaceCount = 0;
    input = vp_removeWhiteSpaceAndCount (whiteSpaceCount, input);
    bool negflg = false;
    if (SUCCESS != vp_AddNumberStringScaledNoModify (outValue, numCharsParsed, negflg, input, 1.0))
        return ERROR;

    if (negflg)
        outValue = -outValue;
    
    numCharsParsed += whiteSpaceCount;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            AngleParser::Init ()
    {
    m_angleMode         = AngleMode::Degrees;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            AngleParser::InitModelSettings (DgnModelCR model)
    {
    DgnModel::Properties const& modelInfo = model.GetProperties();

    SetAngleMode (modelInfo.GetAngularMode ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/        AngleParser::AngleParser(AngleParserCR source)
    {
    m_angleMode         = source.m_angleMode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
AngleParserPtr  AngleParser::Create ()          { return new AngleParser(); }
/* ctor */      AngleParser::AngleParser ()     { Init(); }
AngleParserPtr  AngleParser::Clone() const      { return new AngleParser (*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
AngleParserPtr  AngleParser::Create (DgnModelCR model)
    {
    AngleParserPtr   formatter = Create();

    formatter->InitModelSettings (model);

    return formatter;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AngleParser::ToValue (double& outAngle, WCharCP input) const
    {
    outAngle = 0.0;
    // null and empty strings result in 0.0 angle
    if (NULL == input || 0 == *input)
        return SUCCESS;

    size_t whiteSpaceCount = 0;
    input = vp_removeWhiteSpaceAndCount (whiteSpaceCount, input);

    // assume degrees unless format is Gradians or Radians. Also assume degrees if any of the degree delimiters are present.
    if (vp_parseAsDegreeString (m_angleMode, input))
        {
        return SUCCESS == vp_degreeAngleFromString (outAngle, input) ? SUCCESS : ERROR;
        }

    double  scaleFactor = 1.0;
    bool    negflg      = false;
    switch (m_angleMode)
        {
        case AngleMode::Centesimal:
            scaleFactor = 90.0 / 100.0;
            break;
        case AngleMode::Radians:
            scaleFactor = msGeomConst_degreesPerRadian;
            break;
        default:
            scaleFactor = 1.0;
            break;
        }

    size_t numCharsParsed = 0;
    vp_AddNumberStringScaledNoModify (outAngle, numCharsParsed, negflg, input, scaleFactor);

    if (negflg)
        outAngle = -outAngle;

    numCharsParsed += whiteSpaceCount;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AngleParser::SetAngleModeFromLegacy (AngleFormatVals legacyValue)
    {
    BentleyStatus status = SUCCESS;

    switch (legacyValue)
        {
        default:
            {
            BeAssert(0);
            status = ERROR;
            // FALLTHROUGH
            }
        case AngleFormatVals::Degrees:
            m_angleMode = AngleMode::Degrees;
            break;
        case AngleFormatVals::DegMinSec:
            m_angleMode =  AngleMode::DegMinSec;
            break;
        case AngleFormatVals::Centesimal:
            m_angleMode =  AngleMode::Centesimal;
            break;
        case AngleFormatVals::Radians:
            m_angleMode =  AngleMode::Radians;
            break;
        case AngleFormatVals::DegMin:
            m_angleMode =  AngleMode::DegMin;
            break;
        }

    return status;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t        AngleParser::GetLegacyFormat () const
    {
    /* Used to call old format asyncs */

    AngleFormatVals angleFormat;
    StatusInt       status = SUCCESS;

    switch (m_angleMode)
        {
        default:
            {
            BeAssert(0);
            status = ERROR;
            // FALLTHROUGH
            }
        case AngleMode::Degrees:
            angleFormat = AngleFormatVals::Degrees;
            break;
        case AngleMode::DegMinSec:
            angleFormat = AngleFormatVals::DegMinSec;
            break;
        case AngleMode::Centesimal:
            angleFormat = AngleFormatVals::Centesimal;
            break;
        case AngleMode::Radians:
            angleFormat = AngleFormatVals::Radians;
            break;
        case AngleMode::DegMin:
            angleFormat =  AngleFormatVals::DegMin;
            break;
        }

    return static_cast<uint16_t>(angleFormat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            DirectionParser::Init ()
    {
    m_mode          = DirectionMode::Azimuth;
    m_isClockwise   = false;
    m_baseDirection = 0.0;
    m_trueNorth     = 0.0;
    m_angleParser   = AngleParser::Create();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            DirectionParser::InitModelSettings (DgnModelCR model)
    {
    m_angleParser->InitModelSettings (model);

    DgnModel::Properties const& modelInfo = model.GetProperties();

    SetDirectionMode (modelInfo.GetDirectionMode ());
    SetClockwise (modelInfo.GetDirectionClockwise ());
    SetBaseDirection (modelInfo.GetDirectionBaseDir ());
    SetTrueNorthValue (model.GetDgnDb().Units().GetAzimuth ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/        DirectionParser::DirectionParser(DirectionParserCR source)
    {
    m_mode          = source.m_mode;
    m_isClockwise   = source.m_isClockwise;
    m_baseDirection = source.m_baseDirection;
    m_trueNorth     = source.m_trueNorth;
    m_angleParser   = source.m_angleParser->Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DirectionParserPtr      DirectionParser::Create ()          { return new DirectionParser(); }
/* ctor */              DirectionParser::DirectionParser () { Init(); }
DirectionParserPtr      DirectionParser::Clone() const      { return new DirectionParser (*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/12
+---------------+---------------+---------------+---------------+---------------+------*/
DirectionParserPtr      DirectionParser::Create (DgnModelCR model)
    {
    DirectionParserPtr   parser = Create();

    parser->InitModelSettings (model);

    return parser;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             07/87
+---------------+---------------+---------------+---------------+---------------+------*/
int         DirectionParser::StringToDirection (double& dir, WCharCP inputString) const
    {
    WString str = inputString;

    double  tmpang;
    short   ns_command = 0, ew_command = 0;
    WChar   *leftover, *ew_str, *p;
    size_t  szInput = wcslen (inputString) + 1;
    WCharP  strng = (WCharP) _alloca(szInput * sizeof (WChar));
    WCharP  nw_str = (WCharP) _alloca(szInput * sizeof (WChar));
    wcscpy (strng, str.c_str());
    BeStringUtilities::Wcsupr (strng);

    dir = 0.0;         /* default value */
    /* first go through and find the first digit */
    for (leftover=strng, p=nw_str; *leftover && !isdigit (*leftover); *p++ = *leftover++);

    *p = L'\0';

    WChar default_delimiters[] = {L" =/"};
    WChar* context;
    
    WCharP res = BeStringUtilities::Wcstok (nw_str, default_delimiters, &context);
    if (res) 
        {
        if (vp_cmdStartsWith (L"NORTH", res))
            ns_command = 1;
        else if (vp_cmdStartsWith (L"SOUTH", res))
            ns_command = 3;
        }
    if (ns_command == 1 || ns_command == 3)
        {
        if (! *leftover)
            {
            dir = fc_90 * (double) ns_command;
            return SUCCESS;
            }
        if (0 != (ew_str = wcspbrk (leftover, L"EW")))
            {
            res = BeStringUtilities::Wcstok (ew_str, L" ", &context);
            if (res)
                {
                if (vp_cmdStartsWith (L"EAST", ew_str))
                    ew_command = 1;
                else if (vp_cmdStartsWith (L"WEST", ew_str))
                    ew_command = 2;
                }
            *ew_str = '\0';
            }
        else
            {
            ew_command = -1;
            }

        if (SUCCESS == m_angleParser->ToValue (tmpang, leftover))
            {
            if (ew_command == 1 || ew_command == 2)
                {
                switch  (ns_command + ew_command)
                    {
                    case 2:         /* NE */
                        dir = fc_90 - tmpang;
                        break;

                    case 3:         /* NW */
                        dir = tmpang + fc_90;
                        break;

                    case 5:         /* SW */
                        dir = fc_270 - tmpang;
                        break;

                    case 4:         /* SE */
                        dir = tmpang + fc_270;
                        break;
                    }
                }
            else
                {
                return -1;
                }
            }
        else
            {
            return -1;
            }
        }
    else
        {
        res = BeStringUtilities::Wcstok (strng, L" ", &context);
        if (res)
            {
            if (vp_cmdStartsWith (L"EAST", strng))
                ew_command = 1;
            else if (vp_cmdStartsWith (L"WEST", strng))
                ew_command = 2;
            }

        if (ew_command == 1 || ew_command == 2)
            {
            dir = (double) (ew_command - 1)*fc_180;
            }
        else
            {
            if (SUCCESS == m_angleParser->ToValue (dir, strng))
                {
                if (DirectionMode::Azimuth == m_mode)
                    {
                    if (m_isClockwise)
                        dir = m_baseDirection - dir;
                    else
                        dir = dir - m_baseDirection;
                    }
                }
            else
                {
                return ERROR;
                }
            }
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DirectionParser::ToValue (double& outDirection, WCharCP input)
    {
    outDirection = 0.0;
    if (NULL == input || 0 == *input)
        return SUCCESS;

    size_t whiteSpaceCount = 0;
    input = vp_removeWhiteSpaceAndCount (whiteSpaceCount, input);

    if (SUCCESS != StringToDirection (outDirection, input))
        return ERROR; 

    outDirection += m_trueNorth;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            DistanceParser::Init ()
    {
    m_masterUnitScale = 1.0;
    m_subUnitScale    = 1.0;
    m_scale           = 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            DistanceParser::InitModelSettings (DgnModelCR model)
    {
    DgnModel::Properties const& modelInfo = model.GetProperties();

    UnitDefinitionCR  subUnit       = modelInfo.GetSubUnits();
    UnitDefinitionCR  masterUnit    = modelInfo.GetMasterUnits();
    
    double  uorPerMast = masterUnit.ToMillimeters();
    double  uorPerSub  = subUnit.ToMillimeters();
    
    SetMasterUnitLabel (masterUnit.GetLabelCP());
    SetSubUnitLabel    (subUnit.GetLabelCP());
    SetMasterUnitScale (uorPerMast);
    SetSubUnitScale    (uorPerSub);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/        DistanceParser::DistanceParser (DistanceParserCR source)
    {
    m_masterUnitLabel   = source.m_masterUnitLabel;
    m_subUnitLabel      = source.m_subUnitLabel;
    m_masterUnitScale   = source.m_masterUnitScale;
    m_subUnitScale      = source.m_subUnitScale;
    m_scale             = source.m_scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceParserPtr       DistanceParser::Create ()           { return new DistanceParser(); }
/* ctor */              DistanceParser::DistanceParser ()   { Init(); }
DistanceParserPtr       DistanceParser::Clone() const       { return new DistanceParser (*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceParserPtr       DistanceParser::Create (DgnModelCR model)
    {
    DistanceParserPtr   parser = Create();

    parser->InitModelSettings (model);

    return parser;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceParserPtr       DistanceParser::Create (DgnViewportR viewport)
    {
    DgnModelP           targetModel = viewport.GetViewController ().GetTargetModel();
    DistanceParserPtr   parser = DistanceParser::Create (*targetModel);

#ifdef WIP_V10_MODEL_ACS
    IAuxCoordSysP   acs = NULL;
    if (targetModel->GetProperties().GetIsAcsLocked())
        acs = IACSManager::GetManager().GetActive (viewport);

    if (NULL == acs)
        return parser;

    parser->SetScale (1 / acs->GetScale ());
#endif

    return parser;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceParserPtr       DistanceParser::Create (DgnModelCR model, IAuxCoordSysCR acs)
    {
    DistanceParserPtr      parser = DistanceParser::Create (model);

    parser->SetScale (acs.GetScale ());

    return parser;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DistanceParser::ToValue (double& outValue, size_t& numCharsParsed, WCharCP input) 
    {
    outValue = 0.0;
    if (NULL == input || 0 == *input)
        return  SUCCESS;

    size_t whiteSpaceCount = 0;
    input = vp_removeWhiteSpaceAndCount (whiteSpaceCount, input);

    BentleyStatus result = SUCCESS == vp_mdlString_toUors2 (&outValue, 
            numCharsParsed,
            input, 
            m_masterUnitLabel.c_str(), 
            m_subUnitLabel.c_str(), 
            m_masterUnitScale, 
            m_subUnitScale) ? SUCCESS: ERROR;

    if (SUCCESS == result)
        outValue *= m_scale;

    numCharsParsed += whiteSpaceCount;

    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DistanceParser::ToValue (double& outValue, WCharCP input) 
    {
    size_t  numChars;

    return ToValue (outValue, numChars, input);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            PointParser::Init ()
    {
    m_is3d              = true;
    m_distanceParser    = DistanceParser::Create();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            PointParser::InitModelSettings (DgnModelCR model)
    {
    m_is3d              = model.Is3d();

    m_distanceParser->InitModelSettings(model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/        PointParser::PointParser (PointParserCR source)
    {
    m_is3d              = source.m_is3d;
    m_distanceParser    = source.m_distanceParser->Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
PointParserPtr          PointParser::Create ()           { return new PointParser(); }
/* ctor */              PointParser::PointParser ()      { Init(); }
PointParserPtr          PointParser::Clone() const       { return new PointParser (*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
PointParserPtr         PointParser::Create (DgnModelCR model)
    {
    PointParserPtr   parser = Create();

    parser->InitModelSettings (model);

    return parser;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
PointParserPtr          PointParser::Create (DgnViewportR viewport)
    {
    DgnModelP           targetModel = viewport.GetViewController ().GetTargetModel();
    PointParserPtr      parser = PointParser::Create (*targetModel);

#ifdef WIP_V10_MODEL_ACS
    IAuxCoordSysP   acs = NULL;
    if (targetModel->GetProperties().GetIsAcsLocked())
        acs = IACSManager::GetManager().GetActive (viewport);

    if (NULL == acs)
        return parser;

    // WIP_DGNPLATFORM_UNITS
    // Needswork: the parser needs to apply the actual ACS transform
    parser->m_distanceParser->SetScale (1 / acs->GetScale ());
#endif

    return parser;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
PointParserPtr          PointParser::Create (DgnModelCR model, IAuxCoordSysCR acs)
    {
    PointParserPtr      parser = PointParser::Create (model);

    // WIP_DGNPLATFORM_UNITS
    // Needswork: the parser needs to apply the actual ACS transform
    parser->m_distanceParser->SetScale (acs.GetScale ());

    return parser;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   PointParser::StringToPoint
(
DPoint3dR       point,              /* <= point returned */
Point3dP        relativeFlags,      /* <= bool for each coordinate */
WCharCP         inString            /* => character string */
)
    {
    int         numCoords = 0;
    WString     coordStr[3];
    WCharCP     currPos = inString;
    WCharCP     endPos = inString + wcslen (inString);

    for (int iCoord = 0; iCoord < 3; iCoord++)
        {
        currPos = wskipSpace (currPos);
        if (currPos == endPos)
            break;

        WCharCP commaPos = ::wcschr(currPos, L',');
        if (NULL == commaPos)
            commaPos = endPos;

        coordStr[iCoord].assign (currPos, commaPos - currPos);
        numCoords++;

        if (commaPos == endPos)
            break;

        currPos = commaPos + 1;
        }

    if (relativeFlags)
        {
        relativeFlags->x = relativeFlags->y = relativeFlags->z = 0;

        // look for '#' signs indicating a relative value.
        if (coordStr[0][0] == L'#')
            {
            relativeFlags->x = 1;
            coordStr[0].erase(0, 1);
            }
        if (coordStr[1][0] == L'#')
            {
            relativeFlags->y = 1;
            coordStr[1].erase(0, 1);
            }
        if (coordStr[2][0] == L'#')
            {
            relativeFlags->z = 1;
            coordStr[2].erase(0, 1);
            }
        }

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    if (SUCCESS != m_distanceParser->ToValue (x, coordStr[0].c_str()) ||
        SUCCESS != m_distanceParser->ToValue (y, coordStr[1].c_str()) )
        return ERROR;

    if (m_is3d)
        {
        if (SUCCESS != m_distanceParser->ToValue (z, coordStr[2].c_str()))
            return ERROR;
        }

    point.x = x;
    point.y = y;
    point.z = z;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PointParser::ToValue (DPoint3dR outPoint, WCharCP input)
    {
    outPoint.Init (0.0, 0.0, 0.0);
    if (NULL == input || 0 == *input)
        return SUCCESS;

    BentleyStatus result = StringToPoint (outPoint, NULL, input);

    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PointParser::ToValue (DPoint3dR outPoint, Point3dR relativeFlags, WCharCP input)
    {
    outPoint.Init (0.0, 0.0, 0.0);
    if (NULL == input || 0 == *input)
        return SUCCESS;

    BentleyStatus result = StringToPoint (outPoint, &relativeFlags, input);

    return result;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            AreaOrVolumeParser::Init ()
    {
    m_masterUnitScale = 1.0;
    m_scale           = 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            AreaOrVolumeParser::InitModelSettings (DgnModelCR model)
    {
    DgnModel::Properties const& modelInfo = model.GetProperties();

    UnitDefinition  masterUnit = modelInfo.GetMasterUnits();
    double          uorPerMast = masterUnit.ToMillimeters();
    
    SetMasterUnitScale (uorPerMast);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AreaOrVolumeParser::ToValue (double& outValue, size_t& numCharsParsed, WCharCP input) 
    {
    outValue = 0.0;
    if (NULL == input || 0 == *input)
        return  SUCCESS;

    size_t whiteSpaceCount = 0;
    input = vp_removeWhiteSpaceAndCount (whiteSpaceCount, input);

    bool    negative;
    int     result = vp_AddNumberStringScaledNoModify (outValue, numCharsParsed, negative, input, pow (m_masterUnitScale, m_dimension));

    if (SUCCESS == result)
        outValue *= pow (m_scale, m_dimension);

    numCharsParsed += whiteSpaceCount;

    return (SUCCESS == result) ? BSISUCCESS : BSIERROR;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     11/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AreaOrVolumeParser::ToValue (double& outValue, WCharCP input) 
    {
    size_t  numChars;

    return ToValue (outValue, numChars, input);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
AreaOrVolumeParser::AreaOrVolumeParser (AreaOrVolumeParserCR source)
    {
    m_masterUnitScale   = source.m_masterUnitScale;
    m_scale             = source.m_scale;
    m_dimension         = source.m_dimension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
AreaOrVolumeParser::AreaOrVolumeParser (uint8_t dimension)
    {
    m_dimension = dimension;
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/        AreaParser::AreaParser (AreaParserCR source)
    : AreaOrVolumeParser (source)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
AreaParserPtr       AreaParser::Create ()           { return new AreaParser(); }
/* ctor */          AreaParser::AreaParser ()       : AreaOrVolumeParser (2) { }
AreaParserPtr       AreaParser::Clone() const       { return new AreaParser (*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
AreaParserPtr       AreaParser::Create (DgnModelCR model)
    {
    AreaParserPtr   parser = Create();

    parser->InitModelSettings (model);

    return parser;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
AreaParserPtr       AreaParser::Create (DgnViewportR viewport)
    {
    DgnModelP       targetModel = viewport.GetViewController ().GetTargetModel();
    AreaParserPtr   parser = AreaParser::Create (*targetModel);

#ifdef WIP_V10_MODEL_ACS
    IAuxCoordSysP   acs = NULL;
    if (targetModel->GetProperties().GetIsAcsLocked())
        acs = IACSManager::GetManager().GetActive (viewport);

    if (NULL == acs)
        return parser;

    parser->SetScale (acs->GetScale ());
#endif

    return parser;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/        VolumeParser::VolumeParser (VolumeParserCR source)
    : AreaOrVolumeParser (source)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
VolumeParserPtr       VolumeParser::Create ()           { return new VolumeParser(); }
/* ctor */          VolumeParser::VolumeParser ()       : AreaOrVolumeParser (3) { }
VolumeParserPtr       VolumeParser::Clone() const       { return new VolumeParser (*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/12
+---------------+---------------+---------------+---------------+---------------+------*/
VolumeParserPtr       VolumeParser::Create (DgnModelCR model)
    {
    VolumeParserPtr   parser = Create();

    parser->InitModelSettings (model);

    return parser;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
VolumeParserPtr       VolumeParser::Create (DgnViewportR viewport)
    {
    DgnModelP       targetModel = viewport.GetViewController ().GetTargetModel();
    VolumeParserPtr   parser = VolumeParser::Create (*targetModel);

#ifdef WIP_V10_MODEL_ACS
    IAuxCoordSysP   acs = NULL;
    if (targetModel->GetRoot()->GetPropertiesCP()->GetIsAcsLocked())
        acs = IACSManager::GetManager().GetActive (viewport);

    if (NULL == acs)
        return parser;

    parser->SetScale (acs->GetScale ());
#endif

    return parser;
    }

END_BENTLEY_DGN_NAMESPACE
