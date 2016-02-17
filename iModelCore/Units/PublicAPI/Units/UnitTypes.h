/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/UnitTypes.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Units/Units.h>

UNITS_TYPEDEFS(Unit);

BEGIN_BENTLEY_UNITS_NAMESPACE

typedef bvector<Utf8String> Utf8Vector;

struct Unit;
typedef RefCountedPtr<Unit> UnitPtr;

struct SymbolicFraction
{
private:
    Utf8Vector m_numerator;
    Utf8Vector m_denominator;

protected:
    SymbolicFraction(Utf8Vector& numerator, Utf8Vector& denominator);
    static void SimplifySubTypes(Utf8Vector &n, Utf8Vector &d);
    void Simplify() { SimplifySubTypes(m_numerator, m_denominator); }
    Utf8Vector& GetNumerator() { return m_numerator; }
    Utf8Vector& GetDenominator() { return m_denominator; }

    Utf8Vector const& Numerator() const { return m_numerator; }
    Utf8Vector const& Denominator() const { return m_denominator; }

public:
    SymbolicFraction& operator*=(const SymbolicFraction& rhs);
    SymbolicFraction& operator/=(const SymbolicFraction& rhs);
    bool operator== (const SymbolicFraction& rhs) const;
    bool operator!= (const SymbolicFraction& rhs) const;
    };


//=======================================================================================
//! A base class for all units.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct Unit : RefCountedBase, SymbolicFraction
{
DEFINE_T_SUPER(SymbolicFraction);
friend struct UnitRegistry;

private:
    Utf8String  m_system;
    Utf8String  m_name;
    double      m_factor;
    double      m_offset;

    static BentleyStatus ParseDefinition(Utf8CP definition, Utf8Vector& numerator, Utf8Vector& denominator);
    static UnitPtr Create (Utf8CP sysName, Utf8CP phenomName, Utf8CP unitName, Utf8CP definition, double factor, double offset);

    Unit(Utf8CP system, Utf8CP phenomena, Utf8CP name, Utf8Vector& numerator, Utf8Vector& denominator, double factor, double offset);

protected:
    virtual Utf8CP _GetName() const { return m_name.c_str(); }
    //virtual Utf8CP _GetDisplayLabel() const { return m_displayLabel.c_str(); }
    
public:
    virtual ~Unit() { }

    bool   IsRegistered()    const;
    Utf8CP GetName()         const { return _GetName(); }
    //Utf8CP GetDisplayLabel() const { return _GetDisplayLabel(); }

    // Binary comparison operators.
    bool operator== (const UnitR rhs) const;
    bool operator!= (const UnitR rhs) const;

    // Arithmetic operators.
    Unit operator*(const UnitR rhs) const;
    Unit operator/(const UnitR rhs) const;
    Unit operator+(const UnitR rhs) const;
    Unit operator-(const UnitR rhs) const;

    // Compound assignment operators.
    UnitR operator*=(const UnitR rhs);
    UnitR operator/=(const UnitR rhs);
    UnitR operator+=(const UnitR rhs);
    UnitR operator-=(const UnitR rhs);
};

END_BENTLEY_UNITS_NAMESPACE