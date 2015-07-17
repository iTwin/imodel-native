/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/ValueFormat.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../BentleyInternal.h"
#include <Bentley/ValueFormat.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeStringUtilities.h>

USING_NAMESPACE_BENTLEY

#define  THOUSANDSSEPARATOR ','
#define  DECIMALSEPARATOR   '.'
#define  ROUNDOFF           (0.5 - std::numeric_limits<double>::epsilon())

// From DgnPlatform.h...
#define   RMINI4                  (-2147483648.0)
#define   RMAXUI4                 4294967295.0

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2005
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleFormatterBase::ReplaceDecimalSeparator (Utf8StringR numericString, Utf8Char newDecimalSeparator)
    {
    if (newDecimalSeparator == DECIMALSEPARATOR)
        return;

    Utf8String::size_type offset;
    if (Utf8String::npos == (offset = numericString.find(DECIMALSEPARATOR)))
        return;

    numericString[offset] = newDecimalSeparator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleFormatterBase::InsertThousandsSeparator (Utf8StringR numericString, Utf8Char thousandsSeparator)
    {
    // Walk forward looking for the decimal point or the end, whichever comes first
    size_t  currPos = 0;
    for ( ; currPos < numericString.size(); currPos++)
        {
        if ( ! isdigit(numericString[currPos]))
            break;
        }

    // Walk back to the beginning, doing an insert for every third character
    while (3 < currPos)
        {
        numericString.insert (currPos - 3, &thousandsSeparator, 1);
        currPos -= 3;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    05/2005
+---------------+---------------+---------------+---------------+---------------+------*/
double DoubleFormatterBase::MinimumResolutionForType (Byte precisionByte, PrecisionType precisionType)
    {
    double  minimumResolution = 1.0;

    switch (precisionType)
        {
        case PrecisionType::Scientific:
            {
            minimumResolution = 0.0;
            break;
            }
        case PrecisionType::Fractional:
            {
            minimumResolution /= (double) (precisionByte + 1);
            break;
            }
        default:
        case PrecisionType::Decimal:
            {
            int     i;
            for (i = 0; i < precisionByte; i++)
                minimumResolution *= 10.0;
            minimumResolution = 1.0 / minimumResolution;
            break;
            }
        }

    return minimumResolution;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/04
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleFormatterBase::GetScientificString (Utf8StringR outString, double value, Byte precisionByte)
    {
    Utf8String     formatString;

    formatString.Sprintf ("%%0.%dE", precisionByte);
    outString.Sprintf (formatString.c_str(), value);

    // Drop the most significant digit from the 3-digit exponent if its zero.
    size_t len = outString.size();
    if (len > 2 && '0' == outString[len-3])
        outString.erase (len-3, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             04/87
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetFractionalStr (Utf8StringR outString, int idenominator, double value)
    {
    outString.clear();

    int     inumerator = (int) (value * (double) idenominator);

    if (inumerator <= 0)
        return;

    while ((inumerator/2)*2 == inumerator && idenominator > 2)
        {
        inumerator      /= 2;
        idenominator    /= 2;
        }

    outString.Sprintf (" %d/%d", inumerator, idenominator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    05/2005
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleFormatterBase::GetPrecisionString (Utf8StringR outString, double value, PrecisionType precisionType, Byte precisionByte, bool leaveLeadingZero, bool leaveTrailingZeros)
    {
    if (precisionType == PrecisionType::Fractional)
        {
        int     iValue = (int) value;
        Utf8String fractionString;

        GetFractionalStr (fractionString, precisionByte + 1, value - iValue);
        outString.Sprintf ("%d%ls", iValue, fractionString.c_str());
        }
    else
        {
        int       i;
        Utf8String   formatString;
        double    truncateValue = ROUNDOFF;
        double    factor = 1.0;

        for (i=0; i<precisionByte; i++)
            factor *= 10.0;

        truncateValue = truncateValue / factor;

        if (value > truncateValue)
            value -= truncateValue;

        formatString.Sprintf ("%%0.%dlf", precisionByte);
        outString.Sprintf (formatString.c_str(), value);

        if (!leaveTrailingZeros && precisionByte > 0)
            StripTrailingZeros (outString);

        if (!leaveLeadingZero)
            {
            if ('0' == outString[0] && 2 <= outString.size())
                outString.erase (0, 1);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   JoshSchifter    07/11
//---------------------------------------------------------------------------------------
void DoubleFormatterBase::StripTrailingZeros (Utf8StringR string, Utf8Char decimalChar)
    {
    size_t newSize = string.length ();
    
    while ((0 < newSize) && (string[newSize - 1] == '0'))
        --newSize;

    if ((0 < newSize) && (string[newSize - 1] == decimalChar))
        --newSize;

    string.resize (newSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */ DoubleFormatterBase::DoubleFormatterBase () { Init(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleFormatterBase::Init ()
    {
    m_leadingZero                   = true;
    m_trailingZeros                 = false;
    m_insertThousandsSeparator      = false;
    m_precisionType                 = PrecisionType::Decimal;
    m_precisionByte                 = 4;
    m_decimalSeparator              = '.';
    m_thousandsSeparator            = ',';
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoeZbuchalski   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void  DoubleFormatterBase::SetPrecision (PrecisionFormat precision)
    {
    m_precisionType = DoubleFormatter::GetTypeFromPrecision (precision);
    m_precisionByte = DoubleFormatter::GetByteFromPrecision (precision);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleFormatterBase::SetPrecision (PrecisionType type, Byte precision)
    {
    m_precisionType = type;
    m_precisionByte = precision;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DoubleFormatterBase::InitFrom(DoubleFormatterBase const& source)
    {
    m_leadingZero                   = source.m_leadingZero;
    m_trailingZeros                 = source.m_trailingZeros;
    m_insertThousandsSeparator      = source.m_insertThousandsSeparator;
    m_precisionType                 = source.m_precisionType;
    m_precisionByte                 = source.m_precisionByte;
    m_decimalSeparator              = source.m_decimalSeparator;
    m_thousandsSeparator            = source.m_thousandsSeparator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DoubleFormatterBase::ToStringFromElevatedValue (double inValue) const
    {
    Utf8String     outString;

    if (PrecisionType::Scientific == m_precisionType || (inValue > RMAXUI4) || (inValue < RMINI4))
        {
        Byte precision = m_precisionByte;

        if (PrecisionType::Fractional == m_precisionType)
            precision = DoubleFormatter::GetByteFromPrecision (PrecisionFormat::Scientific4Places);

        GetScientificString (outString, inValue, precision);
        ReplaceDecimalSeparator (outString, m_decimalSeparator);
        }
    else
        {
        GetPrecisionString (outString, inValue, m_precisionType, m_precisionByte, m_leadingZero, m_trailingZeros);
        ReplaceDecimalSeparator (outString, m_decimalSeparator);

        if (m_insertThousandsSeparator)
            InsertThousandsSeparator (outString, m_thousandsSeparator);
        }

    return outString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DoubleFormatterBase::ToStringBasic (double inValue) const
    {
    bool        isNegative = inValue < 0;

    if (inValue < 0)
        inValue = -inValue;

    double  minimumResolution = MinimumResolutionForType (m_precisionByte, m_precisionType);
    inValue += minimumResolution * ROUNDOFF;

    Utf8String outString = ToStringFromElevatedValue (inValue);

    if (isNegative)
        outString.insert (0, "-");

    return outString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/12
+---------------+---------------+---------------+---------------+---------------+------*/
PrecisionType DoubleFormatter::GetTypeFromPrecision (PrecisionFormat value)
    {
    if (PrecisionFormat::DecimalWhole <= value && PrecisionFormat::Decimal8Places >= value)
        return PrecisionType::Decimal;

    if (PrecisionFormat::ScientificWhole <= value && PrecisionFormat::Scientific8Places >= value)
        return PrecisionType::Scientific;

    if (PrecisionFormat::FractionalWhole <= value && PrecisionFormat::Fractional1_Over_256 >= value)
        return PrecisionType::Fractional;

    BeAssert (false);
    return PrecisionType::Decimal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/12
+---------------+---------------+---------------+---------------+---------------+------*/
Byte DoubleFormatter::GetByteFromPrecision (PrecisionFormat value)
    {
    switch (GetTypeFromPrecision (value))
        {
        case PrecisionType::Decimal:    
            return (Byte) (static_cast<int>(value) - static_cast<int>(PrecisionFormat::DecimalWhole));

        case PrecisionType::Scientific: 
            return (Byte) (static_cast<int>(value) - static_cast<int>(PrecisionFormat::ScientificWhole));

        case PrecisionType::Fractional: 
            return (Byte)((1 << (static_cast<int>(value)-static_cast<int>(PrecisionFormat::FractionalWhole)))- 1);
        }

    BeAssert (false);
    return 4;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/12
+---------------+---------------+---------------+---------------+---------------+------*/
PrecisionFormat DoubleFormatter::ToPrecisionEnum (PrecisionType type, Byte precisionByte)
    {
    PrecisionFormat baseValue;

    switch (type)
        {
        default:
        case PrecisionType::Decimal:     baseValue = PrecisionFormat::DecimalWhole;    break;
        case PrecisionType::Scientific:  baseValue = PrecisionFormat::ScientificWhole; break;
        case PrecisionType::Fractional:  baseValue = PrecisionFormat::FractionalWhole; break;
        }

    if (PrecisionType::Fractional != type)
        {
        // Decimal or Scientific
        if (0 > precisionByte || 8 < precisionByte)
            return (PrecisionFormat) (static_cast<int>(baseValue) + 4);

        return (PrecisionFormat) (static_cast<int>(baseValue) + precisionByte);
        }

    // Fractional
    int offset;
    int denominator = precisionByte+1;

    // convert to sequential precision number, i.e. 0, 1, 2, ... 8
    for (offset = 0; denominator > 1; offset++)
        denominator /= 2;

    return (PrecisionFormat) (static_cast<int>(baseValue) + offset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DoubleFormatterPtr      DoubleFormatter::Create ()            { return new DoubleFormatter(); }
DoubleFormatterPtr      DoubleFormatter::Clone() const        { return new DoubleFormatter (*this); }
/* ctor */              DoubleFormatter::DoubleFormatter ()   { Init(); }
void                    DoubleFormatter::Init ()              { T_Super::Init(); }
/* ctor */              DoubleFormatter::DoubleFormatter(DoubleFormatterCR source)     { T_Super::InitFrom (source);  }

PrecisionFormat   DoubleFormatterBase::GetPrecision () const                      { return DoubleFormatter::ToPrecisionEnum (m_precisionType, m_precisionByte); }
Utf8Char          DoubleFormatterBase::GetDecimalSeparator () const               { return m_decimalSeparator; }
bool              DoubleFormatterBase::GetInsertThousandsSeparator () const       { return m_insertThousandsSeparator; }
Utf8Char          DoubleFormatterBase::GetThousandsSeparator () const             { return m_thousandsSeparator; }
bool              DoubleFormatterBase::GetLeadingZero () const                    { return m_leadingZero; }
bool              DoubleFormatterBase::GetTrailingZeros () const                  { return m_trailingZeros; }
void              DoubleFormatterBase::SetDecimalSeparator (Utf8Char newVal)      { m_decimalSeparator = newVal; }
void              DoubleFormatterBase::SetThousandsSeparator (Utf8Char newVal)    { m_thousandsSeparator = newVal; }
void              DoubleFormatterBase::SetInsertThousandsSeparator (bool newVal ) { m_insertThousandsSeparator = newVal; }
void              DoubleFormatterBase::SetLeadingZero (bool newVal)               { m_leadingZero = newVal; }
void              DoubleFormatterBase::SetTrailingZeros (bool newVal)             { m_trailingZeros = newVal; }

Utf8String        DoubleFormatter::ToString (double inVal) const                  { return T_Super::ToStringBasic (inVal); }

