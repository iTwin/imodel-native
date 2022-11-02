/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <Units/Units.h>

UNITS_TYPEDEFS(Quantity);

BEGIN_BENTLEY_UNITS_NAMESPACE

//=======================================================================================
//! A base class for all quantities.
// @bsiclass
//=======================================================================================
struct Quantity
{
friend struct UnitRegistry;
private:
    unsigned int m_tolerance;
    double m_magnitude;
    UnitCP m_unit;

    BentleyStatus ConvertTo(UnitCP unit, double& value) const;
    UnitsProblemCode GetConvertedMagnitude(double& value, UnitCP unit) const;
public:
    Quantity() :m_tolerance(1000), m_magnitude(0.0), m_unit(nullptr) {}   // Invalid - empty - quantity
    Quantity(double magnitude, UnitCR unit) : m_unit(&unit), m_magnitude(magnitude), m_tolerance(1000) {}
    UNITS_EXPORT Quantity(QuantityCR rhs);

    bool IsNullQuantity() const {return (0.0 == m_magnitude && nullptr == m_unit);}

    bool IsValid() {return (nullptr != m_unit);}
    double GetMagnitude() const {return m_magnitude;}
    double Scale(double scale) {m_magnitude *= scale; return m_magnitude;}
    UnitCP GetUnit() const {return m_unit;}
    Utf8CP GetUnitName() const {return (nullptr == m_unit)? nullptr : m_unit->GetName().c_str();}
    Utf8CP GetUnitLabel() const {return (nullptr == m_unit)? nullptr : m_unit->GetDisplayLabel().c_str();}
    PhenomenonCP GetPhenomenon() const {return (nullptr == m_unit) ? nullptr : m_unit->GetPhenomenon();}
    UNITS_EXPORT Quantity ConvertTo(UnitCP unit) const;
    UNITS_EXPORT bool IsClose(QuantityCR rhs, double tolerance) const;

    // Tolerance is factor used to scale the machine epsilon in order to determine
    // the acceptable level of error.  1000 is chosen as a default.
    UNITS_EXPORT void SetTolerance(unsigned int tolerance) {m_tolerance = tolerance;}
    UNITS_EXPORT unsigned int GetTolerance() const {return m_tolerance;}

    // All comparison is done using the machine epsilon and scaling it by
    // the magnitude of the quantities in question and by the tolerance that
    // we have defined.  So, we don't have operators overloaded but instead
    // provide these methods to differentiate their behavior.
    UNITS_EXPORT bool AlmostEqual(QuantityCR rhs) const;
    UNITS_EXPORT bool AlmostGreaterThan(QuantityCR rhs) const;
    UNITS_EXPORT bool AlmostLessThan(QuantityCR rhs) const;
    UNITS_EXPORT bool AlmostGreaterThanOrEqual(QuantityCR rhs) const {return this->AlmostEqual(rhs) || this->AlmostGreaterThan(rhs);}
    UNITS_EXPORT bool AlmostLessThanOrEqual(QuantityCR rhs) const {return this->AlmostEqual(rhs) || this->AlmostLessThan(rhs);}

    // Arithmetic operators.
    UNITS_EXPORT Quantity Add(QuantityCR rhs) const;
    UNITS_EXPORT Quantity Subtract(QuantityCR rhs) const;
    UNITS_EXPORT Quantity Multiply(QuantityCR rhs) const;
    UNITS_EXPORT Quantity Divide(QuantityCR rhs) const;
};

END_BENTLEY_UNITS_NAMESPACE
