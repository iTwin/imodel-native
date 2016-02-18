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

struct SymbolicFraction
	{
private:
    Utf8Vector m_numerator;
    Utf8Vector m_denominator;

protected:
    static void SimplifySubTypes(Utf8Vector &n, Utf8Vector &d);

    SymbolicFraction(Utf8Vector& numerator, Utf8Vector& denominator);
    
    Utf8Vector& GetNumerator() { return m_numerator; }
    Utf8Vector& GetDenominator() { return m_denominator; }

public:
    SymbolicFraction operator*(const SymbolicFraction& rhs) const;
    SymbolicFraction operator/(const SymbolicFraction& rhs) const;
    
    Utf8Vector const& Numerator() const { return m_numerator; }
    Utf8Vector const& Denominator() const { return m_denominator; }

    bool operator== (const SymbolicFraction& rhs) const;
    bool operator!= (const SymbolicFraction& rhs) const;
    };


//=======================================================================================
//! A base class for all units.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct Unit : SymbolicFraction
	{
DEFINE_T_SUPER(SymbolicFraction);
friend struct UnitRegistry;

private:
    Utf8String  m_system;
    Utf8String  m_name;
    double      m_factor;
    double      m_offset;

    static BentleyStatus ParseDefinition(Utf8CP definition, Utf8Vector& numerator, Utf8Vector& denominator);
    static UnitP Create (Utf8CP sysName, Utf8CP phenomName, Utf8CP unitName, Utf8CP definition, double factor, double offset);

	// TODO: Create a better definition of an "unknown" unit
	Unit (Utf8Vector& numerator, Utf8Vector& denominator) : Unit("", "", "", numerator, denominator, 1.0, 0.0) { }
    Unit (Utf8CP system, Utf8CP phenomena, Utf8CP name, Utf8Vector& numerator, Utf8Vector& denominator, double factor, double offset);

	// Lifecycle is managed by the UnitRegistry so we don't allow copies or assignments.
	Unit (UnitCR unit) = delete;
	UnitR operator=(UnitCR unit) = delete;

protected:
    virtual Utf8CP _GetName() const { return m_name.c_str(); }
    //virtual Utf8CP _GetDisplayLabel() const { return m_displayLabel.c_str(); }
    
public:
	virtual ~Unit() { }

    bool   IsRegistered()    const;
    Utf8CP GetName()         const { return _GetName(); }
    //Utf8CP GetDisplayLabel() const { return _GetDisplayLabel(); }

    // Binary comparison operators.
    bool operator== (UnitR rhs) const;
    bool operator!= (UnitR rhs) const;

    // Arithmetic operators.
    UnitCR operator*(UnitR rhs) const;
    UnitCR operator/(UnitR rhs) const;
    UnitCR operator+(UnitR rhs) const;
    UnitCR operator-(UnitR rhs) const;
};

END_BENTLEY_UNITS_NAMESPACE