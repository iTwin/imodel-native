/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/ValueFormat.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "Bentley.h"
#include "WString.h"
#include "RefCounted.h"
#include "ValueFormat.r.h"

BENTLEY_NAMESPACE_TYPEDEFS(DoubleFormatter);

BEGIN_BENTLEY_NAMESPACE

typedef RefCountedPtr<DoubleFormatter>      DoubleFormatterPtr;

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DoubleFormatterBase
{
protected:  
    bool                m_leadingZero;
    bool                m_trailingZeros;
    bool                m_insertThousandsSeparator;
    PrecisionType       m_precisionType;
    Byte                m_precisionByte;
    Utf8Char            m_decimalSeparator;
    Utf8Char            m_thousandsSeparator;

    BENTLEYDLL_EXPORT void Init();
    BENTLEYDLL_EXPORT void InitFrom(DoubleFormatterBase const& other);

    //! Get the precision used by this formatter.
    BENTLEYDLL_EXPORT Utf8String ToStringFromElevatedValue (double inValue) const;
    BENTLEYDLL_EXPORT Utf8String ToStringBasic (double inValue) const;

    // Helper methods also used by other formatter types
    BENTLEYDLL_EXPORT static void InsertThousandsSeparator (Utf8StringR numericString, Utf8Char thousandsSeparator);
    BENTLEYDLL_EXPORT static void GetScientificString (Utf8StringR outString, double value, Byte precisionByte);
    BENTLEYDLL_EXPORT static void GetPrecisionString (Utf8StringR outString, double value, PrecisionType precisionType, Byte precisionByte, bool leaveLeadingZero, bool leaveTrailingZeros);

public:
    BENTLEYDLL_EXPORT static void ReplaceDecimalSeparator (Utf8StringR numericString, Utf8Char newDecimalSeparator);
    BENTLEYDLL_EXPORT static double MinimumResolutionForType (Byte precisionByte, PrecisionType precisionType);
    BENTLEYDLL_EXPORT static void StripTrailingZeros (Utf8StringR string, Utf8Char decimalChar = '.');

    BENTLEYDLL_EXPORT void SetPrecision (PrecisionType type, Byte precision);

protected:
    BENTLEYDLL_EXPORT DoubleFormatterBase ();

//! Get the precision used by this formatter.
public: 
    BENTLEYDLL_EXPORT PrecisionFormat GetPrecision () const;

    //! Get the decimal separator used by this formatter.
    BENTLEYDLL_EXPORT Utf8Char GetDecimalSeparator () const;

    //! Get the value indicating if the thousands separator is inserted by this formatter.
    BENTLEYDLL_EXPORT bool GetInsertThousandsSeparator () const;

    //! Get the thousands separator used by this formatter.
    BENTLEYDLL_EXPORT Utf8Char GetThousandsSeparator () const;

    //! Test if this formatter will include a leading zero.  A leading zero is only
    //! included for values less than 1.0.  Ex. "0.5" vs. ".5"
    BENTLEYDLL_EXPORT bool GetLeadingZero () const;

    //! Test if this formatter will include trailing zeros.  Trailing zeros are only included
    //! up to the requested precision.  Ex. "30.500" vs. "30.5"
    BENTLEYDLL_EXPORT bool GetTrailingZeros () const;

    //! Set the formatter's precision.
    BENTLEYDLL_EXPORT void SetPrecision (PrecisionFormat newVal);

    //! Set the formatter's decimal separator.
    BENTLEYDLL_EXPORT void SetDecimalSeparator (Utf8Char newVal);

    //! Set the formatter's thousands separator.
    BENTLEYDLL_EXPORT void SetThousandsSeparator (Utf8Char newVal);

    //! Specify if the thousands separator should be inserted or not.
    BENTLEYDLL_EXPORT void SetInsertThousandsSeparator (bool newVal);

    //! Set the formatter's leading zero behavior.  A leading zero is only
    //! included for values less than 1.0.  Ex. "0.5" vs. ".5"
    //! @param[in] newVal pass true to include a leading zero for values less than 1.0
    BENTLEYDLL_EXPORT void SetLeadingZero (bool newVal);

    //! Set the formatter's trailing zeros behavior.  Trailing zeros are only included
    //! up to the requested precision.  Ex. "30.500" vs. "30.5"
    //! @param[in] newVal pass true to zero pad the output string to the requested precision.
    BENTLEYDLL_EXPORT void SetTrailingZeros (bool newVal);
};

//=======================================================================================
//! Used to construct a string from a numerical value. 
//!
//! This class provides various distance formatting options including:
//!   - PrecisionFormat
//!       - Whole numbers ex. 10
//!       - Decimal precision ex. 10.25
//!       - Fractional precision ex. 10 1/4
//!       - Scientific precision ex. 1.025E2
//!   - Decimal Separator
//!       - Period ex. 10.5
//!       - Comma ex. 10,5
//!   - Many more
//!
// @bsiclass
//=======================================================================================
struct DoubleFormatter : DoubleFormatterBase, RefCountedBase
{
    DEFINE_T_SUPER(DoubleFormatterBase)
private:  
    DoubleFormatter ();
    DoubleFormatter (DoubleFormatterCR other);
    void Init();

public: 
    static BENTLEYDLL_EXPORT PrecisionFormat ToPrecisionEnum (PrecisionType type, Byte precisionByte);
    static BENTLEYDLL_EXPORT Byte GetByteFromPrecision (PrecisionFormat value);

    //! Categorize a precision value.
    static BENTLEYDLL_EXPORT PrecisionType GetTypeFromPrecision (PrecisionFormat in);

    //! Construct a formatter with default settings.
    static BENTLEYDLL_EXPORT DoubleFormatterPtr Create ();

    //! Construct a formatter which is a duplicate of an existing formatter.
    BENTLEYDLL_EXPORT DoubleFormatterPtr Clone () const;

    //! Use the settings defined in this formatter to convert a double value to a string.
    //! @param[in] value value to format.
    BENTLEYDLL_EXPORT Utf8String ToString (double value) const;
};

END_BENTLEY_NAMESPACE

