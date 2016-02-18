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
UNITS_TYPEDEFS(Constant);

BEGIN_BENTLEY_UNITS_NAMESPACE

//=======================================================================================
//! A base class for all quantities.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct QuantityBase : RefCountedBase
    {
protected:
    QuantityBase(double magnitude, UnitCR unit);

    double      m_magnitude;
    UnitCP      m_unit;
    bool        m_error;
    Utf8String  m_errorMessage;

public:
    double GetMagnitude() { return m_magnitude; }
    UnitCR GetUnit () { return *m_unit; }

    bool IsValid() { return !m_error; }
    Utf8StringCR GetErrorMessage() { return m_errorMessage; }

    // Binary comparison operators.
    virtual bool operator== (const QuantityBase& rhs) const;
    virtual bool operator!= (const QuantityBase& rhs) const;

    // Arithmetic operators.
    virtual QuantityBase operator*(const QuantityBase& rhs) const;
    virtual QuantityBase operator/(const QuantityBase& rhs) const;
    virtual QuantityBase operator+(const QuantityBase& rhs) const;
    virtual QuantityBase operator-(const QuantityBase& rhs) const;

    // Compound assignment operators.
    virtual QuantityBase& operator*=(const QuantityBase& rhs);
    virtual QuantityBase& operator/=(const QuantityBase& rhs);
    virtual QuantityBase& operator+=(const QuantityBase& rhs);
    virtual QuantityBase& operator-=(const QuantityBase& rhs);
    };

struct Quantity;
typedef RefCountedPtr<Quantity> QuantityPtr;

//=======================================================================================
//! A class to represent a quantity which consists of a unit and magnitude.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct Quantity : QuantityBase
    {
private:
    Quantity(double magnitude, UnitCR unit);

public:
    static QuantityPtr Create (double magnitude, Utf8CP unitName);

    void SetMagnitude (double magnitude) { m_magnitude = magnitude; }
    void SetUnit (UnitCP unit) { m_unit = unit; }
    };

//struct Constant;
//typedef RefCountedPtr<Constant> ConstantPtr;

//////=======================================================================================
//////! A class to represent a defined constant.
////// @bsiclass                                                    Chris.Tartamella   02/16
//////=======================================================================================
////struct Constant : QuantityBase
////    {
////private:
////    Utf8String m_name;
////
////    Constant(Utf8CP constantName, double quantity, UnitCP unit);
////
////public:
////    static ConstantPtr Create (Utf8CP constantName, double magnitude, Utf8CP unitName);
////
////    Utf8CP GetConstantName() { return m_name.c_str(); }
////    };

END_BENTLEY_UNITS_NAMESPACE

