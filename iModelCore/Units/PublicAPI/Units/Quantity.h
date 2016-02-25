/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/Quantity.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Units/Units.h>

UNITS_TYPEDEFS(Quantity);

BEGIN_BENTLEY_UNITS_NAMESPACE

struct Quantity;
typedef RefCountedPtr<Quantity> QuantityPtr;

//=======================================================================================
//! A base class for all quantities.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct Quantity : RefCountedBase
    {
private:
    Quantity (double magnitude, UnitCP unit);
    Quantity(const Quantity& rhs);

    double      m_magnitude;
    UnitCP      m_unit;
    bool        m_error;
    Utf8String  m_errorMessage;

public:
    static QuantityPtr Create(double magnitude, Utf8CP unitName);

    double GetMagnitude() { return m_magnitude; }
    UnitCP GetUnit () { return m_unit; }

    bool IsValid() { return !m_error; }
    Utf8StringCR GetErrorMessage() { return m_errorMessage; }

    BentleyStatus ConvertTo(Utf8CP unitName, double& value) const;

    // Binary comparison operators.
    bool operator== (QuantityCR rhs) const;
    bool operator!= (QuantityCR rhs) const;
    bool operator> (QuantityCR rhs) const;
    bool operator< (QuantityCR rhs) const;
    bool operator>= (QuantityCR rhs) const;
    bool operator<= (QuantityCR rhs) const;

    // Arithmetic operators.
    QuantityPtr Add (QuantityCR rhs) const;
    QuantityPtr Subtract (QuantityCR rhs) const;
    QuantityPtr Multiply (QuantityCR rhs) const;
    QuantityPtr Divide (QuantityCR rhs) const;
    };

END_BENTLEY_UNITS_NAMESPACE

