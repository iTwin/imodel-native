/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/Quantity.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
#include <Units/Units.h>

UNITS_TYPEDEFS(Quantity);

BEGIN_BENTLEY_UNITS_NAMESPACE

//struct Quantity;
//typedef RefCountedPtr<Quantity> QuantityPtr;

//=======================================================================================
//! A base class for all quantities.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct Quantity
   // struct Quantity : RefCountedBase
    {
    friend struct UnitRegistry;
private:
    unsigned int m_tolerance;
    double       m_magnitude;
    UnitCP       m_unit;


    //BentleyStatus ConvertTo(Utf8CP unitName, double& value) const;
    BentleyStatus ConvertTo(UnitCP unit, double& value) const;
    UnitsProblemCode GetConvertedMagnitude(double& value, UnitCP unit) const;
public:

    Quantity() :m_tolerance(1000), m_magnitude(0.0), m_unit(nullptr) {}   // Invalid - empty - quantity
    UNITS_EXPORT  bool IsNullQuantity() const;// { return (0.0 == m_magnitude && nullptr == m_unit); }
    UNITS_EXPORT  Quantity(QuantityCR rhs);
 //   UNITS_EXPORT static QuantityCP Create (double magnitude, Utf8CP unitName);
    UNITS_EXPORT Quantity(double magnitude, UnitCR unit);

    //UNITS_EXPORT static QuantityPtr Create(double magnitude, Utf8CP unitName);
    //UNITS_EXPORT static QuantityPtr Create(double magnitude, UnitCP unit);
    bool ISValid() { return (nullptr != m_unit); }
    double GetMagnitude() const { return m_magnitude; }
    double Scale(double scale) { m_magnitude *= scale;  return m_magnitude; }
    UnitCP GetUnit () const { return m_unit; }
    Utf8CP GetUnitName() const { return (nullptr == m_unit)? nullptr : m_unit->GetName(); }
    UNITS_EXPORT Quantity ConvertTo(UnitCP unit) const;


    //UNITS_EXPORT QuantityCP ConvertTo(Utf8CP unitName) const;

    // Tolerance is factor used to scale the machine epsilon in order to determine
    // the acceptable level of error.  1000 is chosen as a default.
    UNITS_EXPORT void SetTolerance(unsigned int tolerance) { m_tolerance = tolerance; }
    UNITS_EXPORT unsigned int GetTolerance () const { return m_tolerance; }

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
    UNITS_EXPORT Quantity Add (QuantityCR rhs) const;
    UNITS_EXPORT Quantity Subtract (QuantityCR rhs) const;
    UNITS_EXPORT Quantity Multiply (QuantityCR rhs) const;
    UNITS_EXPORT Quantity Divide (QuantityCR rhs) const;
    };

END_BENTLEY_UNITS_NAMESPACE

/*__PUBLISH_SECTION_END__*/
