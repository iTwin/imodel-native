/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Util.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <UnitsPCH.h>

USING_NAMESPACE_BENTLEY_UNITS

Unit::Unit(Utf8CP system, Utf8CP phenomena, Utf8CP name, Utf8CP displayLabel, bvector<Utf8String>& numerator, bvector<Utf8String>& denominator)
	: m_system (system), m_name(name), m_displayLabel(displayLabel)
	{
	m_numerator = move(numerator);
	m_denominator = move(denominator);
	}

Unit * Unit::Create (Utf8CP sysName, Utf8CP phenomName, Utf8CP unitName, Utf8CP displayName, Utf8CP definition)
	{
	auto n = bvector<Utf8String>();
	auto d = bvector<Utf8String>();
	//Parser::ParseDefinition (definition, n, d);
	
	return new Unit (sysName, phenomName, unitName, displayName, n, d);
	}

bool Unit::IsRegistered()
	{
	auto newUnit = UnitRegistry::Instance().LookupUnitBySubTypes(m_numerator, m_denominator);
	
	return newUnit != nullptr;
	}

void Unit::SimplifySubTypes(bvector<Utf8String> &n, bvector<Utf8String> &d)
	{
	auto temp = bvector<Utf8String>();
	
	// Remove the intersection between the two vectors.
	set_difference (n.begin(), n.end(), d.begin(), d.end(), temp.begin());
	n.erase(n.begin(), n.end());
	move(temp.begin(), temp.end(), n.begin());
	temp.erase(temp.begin(), temp.end());

	set_difference (d.begin(), d.end(), n.begin(), n.end(), temp.begin());
	d.erase(d.begin(), d.end());
	move(temp.begin(), temp.end(), d.begin());
	}

Unit& Unit::operator* (const Unit& rhs)
	{
	auto n = bvector<Utf8String>(this->m_numerator);
	auto d = bvector<Utf8String>(this->m_denominator);

	// Combine numerator and denominators.
	for_each (rhs.m_numerator.begin(), rhs.m_numerator.end(), 
							[&](Utf8String s) { n.push_back(s); });

	for_each (rhs.m_denominator.begin(), rhs.m_denominator.end(), 
							[&](Utf8String s) { d.push_back(s); });

	SimplifySubTypes (n, d);	

	auto newUnit = UnitRegistry::Instance().LookupUnitBySubTypes(n, d);
	if (newUnit != nullptr)
		return *newUnit;

	return *this;
	}

Unit& Unit::operator/ (const Unit& rhs)
	{
	auto n = bvector<Utf8String>(this->m_numerator);
	auto d = bvector<Utf8String>(this->m_denominator);

	// Combine numerator and denominators.
	for_each (rhs.m_numerator.begin(), rhs.m_numerator.end(), 
							[&](Utf8String s) { d.push_back(s); });

	for_each (rhs.m_denominator.begin(), rhs.m_denominator.end(), 
							[&](Utf8String s) { n.push_back(s); });

	SimplifySubTypes(n, d);

	auto newUnit = UnitRegistry::Instance().LookupUnitBySubTypes(n, d);
	if (newUnit != nullptr)
		return *newUnit;

	return *this;
	}
