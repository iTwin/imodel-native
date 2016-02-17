/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Util.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Units/Units.h>

BEGIN_BENTLEY_UNITS_NAMESPACE

typedef bvector<Utf8String> Utf8Vector;

struct Unit;
typedef RefCountedPtr<Unit> UnitPtr;

//=======================================================================================
//! A base class for all units.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct Unit : RefCountedBase
	{
friend struct UnitRegistry;

private:
	Utf8String m_system;
	Utf8String m_name;
	Utf8String m_displayLabel;
	Utf8Vector m_numerator;
	Utf8Vector m_denominator;

	static BentleyStatus ParseDefinition(Utf8CP definition, Utf8Vector& numerator, Utf8Vector& denominator);
	static UnitPtr Create (Utf8CP sysName, Utf8CP phenomName, Utf8CP unitName, 
		                                    Utf8CP displayName, Utf8CP definition);

	Unit(Utf8CP system, Utf8CP phenomena, Utf8CP name, Utf8CP displayLabel, 
		   Utf8Vector& numerator, Utf8Vector& denominator);

	void SimplifySubTypes (Utf8Vector &n, Utf8Vector &d);
	
protected:
	Utf8Vector Numerator() const { return m_numerator; }
	Utf8Vector Denominator() const { return m_denominator; }

	virtual Utf8CP _GetName() const { return m_name.c_str(); }
	virtual Utf8CP _GetDisplayLabel() const { return m_displayLabel.c_str(); }
	
public:
	virtual ~Unit() { }

	bool   IsRegistered()    const;
	Utf8CP GetName()         const { return _GetName(); }
	Utf8CP GetDisplayLabel() const { return _GetDisplayLabel(); }

	// Binary comparison operators.
	bool operator== (const Unit& rhs) const;
	bool operator!= (const Unit& rhs) const;

	// Arithmetic operators.
	Unit operator*(const Unit& rhs) const;
	Unit operator/(const Unit& rhs) const;
	Unit operator+(const Unit& rhs) const;
	Unit operator-(const Unit& rhs) const;

	// Compound assignment operators.
	Unit& operator*=(const Unit& rhs);
	Unit& operator/=(const Unit& rhs);
	Unit& operator+=(const Unit& rhs);
	Unit& operator-=(const Unit& rhs);
	};

struct QuantityBase
	{
private:
	double   m_quantity;
	Unit  *  m_unit;

	QuantityBase() { }

public:
	// Lookup the unit type using a string (from UnitRegistry) and
	// return the unit or null.
	static QuantityBase& Create(double value, Utf8CP unit);

	/// Overloaded Operators
	};

struct Quantity : QuantityBase
	{
	
	};

struct Constant : QuantityBase
	{

	};

END_BENTLEY_UNITS_NAMESPACE