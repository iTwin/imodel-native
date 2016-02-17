/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Util.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <UnitsPCH.h>
#include <Parser.h>

USING_NAMESPACE_BENTLEY_UNITS

Unit::Unit(Utf8CP system, Utf8CP phenomena, Utf8CP name, Utf8CP displayLabel, Utf8Vector& numerator, Utf8Vector& denominator)
	: m_system (system), m_name(name), m_displayLabel(displayLabel)
	{
	m_numerator = move(numerator);
	m_denominator = move(denominator);
	}

UnitPtr Unit::Create (Utf8CP sysName, Utf8CP phenomName, Utf8CP unitName, Utf8CP displayName, Utf8CP definition)
	{
	auto n = Utf8Vector();
	auto d = Utf8Vector();
	Parser::ParseDefinition (definition, n, d);
	
	return new Unit (sysName, phenomName, unitName, displayName, n, d);
	}

bool Unit::IsRegistered() const
	{
	auto newUnit = UnitRegistry::Instance().LookupUnitBySubTypes(m_numerator, m_denominator);
	
	return newUnit != nullptr;
	}

void Unit::SimplifySubTypes(Utf8Vector &n, Utf8Vector &d)
	{
	auto temp = Utf8Vector();
	
	// Remove the intersection between the two vectors.
	set_difference (n.begin(), n.end(), d.begin(), d.end(), temp.begin());
	n.erase(n.begin(), n.end());
	move(temp.begin(), temp.end(), n.begin());
	temp.erase(temp.begin(), temp.end());

	set_difference (d.begin(), d.end(), n.begin(), n.end(), temp.begin());
	d.erase(d.begin(), d.end());
	move(temp.begin(), temp.end(), d.begin());
	}

bool Unit::operator== (const Unit& rhs) const
	{
	if (m_numerator.size() != rhs.m_numerator.size())
		return false;

	if (m_denominator.size() != rhs.m_denominator.size())
		return false;

	// TODO: Compare the two vectors.
	return true;
	}

bool Unit::operator!= (const Unit& rhs) const
	{
	return !(*this == rhs);
	}

Unit Unit::operator* (const Unit& rhs) const
	{
	Unit result = *this;
	result *= rhs;
	return result;
	}

Unit Unit::operator/ (const Unit& rhs) const
	{
	Unit result = *this;
	result /= rhs;
	return result;
	}

Unit Unit::operator+ (const Unit& rhs) const
	{
	Unit result = *this;
	result += rhs;
	return result;
	}

Unit Unit::operator- (const Unit& rhs) const
	{
	Unit result = *this;
	result -= rhs;
	return result;
	}

Unit& Unit::operator+= (const Unit& rhs)
	{
	// TODO: This might not be right.
	return *this;
	}

Unit& Unit::operator-= (const Unit& rhs)
	{
	// TODO: This might not be right.
	return *this;
	}

Unit& Unit::operator*= (const Unit& rhs)
	{
	// Combine numerator and denominators.
	for_each (rhs.m_numerator.begin(), rhs.m_numerator.end(), 
							[&](Utf8String s) { m_numerator.push_back(s); });

	for_each (rhs.m_denominator.begin(), rhs.m_denominator.end(), 
							[&](Utf8String s) { m_denominator.push_back(s); });

	SimplifySubTypes(m_numerator, m_denominator);

	auto newUnit = UnitRegistry::Instance().LookupUnitBySubTypes(m_numerator, m_denominator);
	if (newUnit != nullptr)
		return *newUnit;

	return *this;
	}

Unit& Unit::operator/= (const Unit& rhs)
	{
	// Combine numerator and denominators.
	for_each (rhs.m_numerator.begin(), rhs.m_numerator.end(), 
							[&](Utf8String s) { m_denominator.push_back(s); });

	for_each (rhs.m_denominator.begin(), rhs.m_denominator.end(), 
							[&](Utf8String s) { m_numerator.push_back(s); });

	SimplifySubTypes(m_numerator, m_denominator);

	auto newUnit = UnitRegistry::Instance().LookupUnitBySubTypes(m_numerator, m_denominator);
	if (newUnit != nullptr)
		return *newUnit;

	return *this;
	}