/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/Quantity.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
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

    unsigned int m_tolerance;
    double       m_magnitude;
    UnitCP       m_unit;

    BentleyStatus ConvertTo(Utf8CP unitName, double& value) const;
public:
    UNITS_EXPORT static QuantityPtr Create(double magnitude, Utf8CP unitName);

    double GetMagnitude() { return m_magnitude; }
    UnitCP GetUnit () { return m_unit; }

    UNITS_EXPORT QuantityPtr ConvertTo(Utf8CP unitName) const;

    // Tolerance is factor used to scale the machine epsilon in order to determine
    // the acceptable level of error.  1000 is chosen as a default.
    UNITS_EXPORT void SetTolerance(unsigned int tolerance) { m_tolerance = tolerance; }
    UNITS_EXPORT unsigned int GetTolerance () { return m_tolerance; }

    // All comparison is done using the machine epsilon and scaling it by
    // the magnitude of the quantities in question and by the tolerance that
    // we have defined.  So, we don't have operators overloaded but instead
    // provide these methods to differentiate their behavior.
    UNITS_EXPORT bool AlmostEqual(QuantityCR rhs) const;
    UNITS_EXPORT bool AlmostGreaterThan(QuantityCR rhs) const;
    UNITS_EXPORT bool AlmostLessThan(QuantityCR rhs) const;
    UNITS_EXPORT bool AlmostGreaterThanOrEqual(QuantityCR rhs) const;
    UNITS_EXPORT bool AlmostLessThanOrEqual(QuantityCR rhs) const;

    // Arithmetic operators.
    UNITS_EXPORT QuantityPtr Add (QuantityCR rhs) const;
    UNITS_EXPORT QuantityPtr Subtract (QuantityCR rhs) const;
    UNITS_EXPORT QuantityPtr Multiply (QuantityCR rhs) const;
    UNITS_EXPORT QuantityPtr Divide (QuantityCR rhs) const;
    };

END_BENTLEY_UNITS_NAMESPACE

/*__PUBLISH_SECTION_END__*/
