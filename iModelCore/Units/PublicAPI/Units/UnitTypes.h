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

struct Unit
	{
friend struct UnitRegistry;

private:
	Utf8String m_system;
	Utf8String m_name;
	Utf8String m_displayLabel;
	bvector<Utf8String> m_numerator;
	bvector<Utf8String> m_denominator;

	Unit(Utf8CP system, Utf8CP phenomena, Utf8CP name, Utf8CP displayLabel, 
		   bvector<Utf8String>& numerator, bvector<Utf8String>& denominator);

	void SimplifySubTypes (bvector<Utf8String> &n, bvector<Utf8String> &d);

	static Unit * Create (Utf8CP sysName, Utf8CP phenomName, Utf8CP unitName, Utf8CP displayName, Utf8CP definition);

protected:

	bvector<Utf8String> Numerator() const { return m_numerator; }
	bvector<Utf8String> Denominator() const { return m_denominator; }

	virtual Utf8CP _GetName() const { return m_name.c_str(); }
	virtual Utf8CP _GetDisplayLabel() const { return m_displayLabel.c_str(); }
	
public:
	virtual ~Unit() { }

	bool IsRegistered();

	Utf8CP GetName() const { return _GetName(); }
	Utf8CP GetDisplayLabel() const { return _GetDisplayLabel(); }

	bool operator== (const Unit& rhs) const;
	bool operator!= (const Unit& rhs) const;

	// Overload operators.
	// TODO: Check that these are the right form.
	Unit operator*(const Unit& rhs) const;
	Unit operator/(const Unit& rhs) const;
	Unit operator+(const Unit& rhs) const;
	Unit operator-(const Unit& rhs) const;

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