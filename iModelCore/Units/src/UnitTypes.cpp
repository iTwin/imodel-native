/*--------------------------------------------------------------------------------------+
|
|     $Source: src/UnitTypes.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"

USING_NAMESPACE_BENTLEY_UNITS

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Unit::Unit(Utf8CP system, Utf8CP phenomena, Utf8CP name, Utf8CP definition, Utf8Char dimensonSymbol, double factor, double offset) : SymbolicFraction(definition),
    m_name(name), m_system (system), m_factor(factor), m_offset(offset), m_dimensionSymbol(dimensonSymbol)
    {
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitPtr Unit::Create (Utf8CP sysName, Utf8CP phenomName, Utf8CP unitName, Utf8CP definition, Utf8Char dimensionSymbol, double factor, double offset)
    {
    return new Unit (sysName, phenomName, unitName, definition, dimensionSymbol, factor, offset);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::IsRegistered() const
    {
    auto newUnit = UnitRegistry::Instance().LookupUnitBySubTypes(Numerator(), Denominator());
    
    return newUnit != nullptr;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::operator== (UnitR rhs) const
    {
    return T_Super::operator==(rhs);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::operator!= (UnitR rhs) const
    {
    return !(*this == rhs);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCR Unit::operator* (UnitR rhs) const
    {
    auto result = T_Super::operator/(rhs);

    UnitCP unit = UnitRegistry::Instance().LookupUnitBySubTypes(result.Numerator(), result.Denominator());
	if (nullptr != unit)
		return *unit;

	Utf8Vector n = Utf8Vector();
    copy (result.Numerator().begin(), result.Numerator().end(), n.begin());

    Utf8Vector d = Utf8Vector();
    copy (result.Denominator().begin(), result.Denominator().end(), d.begin());

    return move(Unit(n, d));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCR Unit::operator/ (UnitR rhs) const
    {
    auto result = T_Super::operator/(rhs);

    UnitCP unit = UnitRegistry::Instance().LookupUnitBySubTypes(result.Numerator(), result.Denominator());
	if (nullptr != unit)
		return *unit;

	Utf8Vector n = Utf8Vector();
    copy (result.Numerator().begin(), result.Numerator().end(), n.begin());

    Utf8Vector d = Utf8Vector();
    copy (result.Denominator().begin(), result.Denominator().end(), d.begin());

    return move(Unit(n, d));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCR Unit::operator+ (UnitR rhs) const
    {
	//TODO: Is this right?
    return *this;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCR Unit::operator- (UnitR rhs) const
    {
	//TODO: Is this right?
    return *this;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SymbolicFraction::SimplifySubTypes(Utf8Vector &n, Utf8Vector &d)
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

SymbolicFraction::SymbolicFraction(Utf8CP definition)
    {
    ParseDefinition(definition, m_numerator, m_denominator);
    }
SymbolicFraction::SymbolicFraction(Utf8Vector& numerator, Utf8Vector& denominator)
    {
    m_numerator = move(numerator);
    m_denominator = move(denominator);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
SymbolicFraction SymbolicFraction::operator*(const SymbolicFraction& rhs) const
    {
    auto n = Utf8Vector(m_numerator);
    auto d = Utf8Vector(m_denominator);

    // Combine numerator and denominators.
    for_each(rhs.m_numerator.begin(), rhs.m_numerator.end(),
             [&] (Utf8String s) { n.push_back(s); });
    sort(n.begin(), n.end());

    for_each(rhs.m_denominator.begin(), rhs.m_denominator.end(),
             [&] (Utf8String s) { d.push_back(s); });
    sort(d.begin(), d.end());

    SimplifySubTypes(n, d);
    return SymbolicFraction(n,d);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
SymbolicFraction SymbolicFraction::operator/(const SymbolicFraction& rhs) const
    {
	auto n = Utf8Vector(m_numerator);
    auto d = Utf8Vector(m_denominator);

    // Combine numerator and denominators.
    for_each(rhs.m_numerator.begin(), rhs.m_numerator.end(),
             [&] (Utf8String s) { d.push_back(s); });
    sort(d.begin(), d.end());

    for_each(rhs.m_denominator.begin(), rhs.m_denominator.end(),
             [&] (Utf8String s) { n.push_back(s); });
    sort(n.begin(), n.end());

    SimplifySubTypes(n, d);
    return SymbolicFraction (n, d);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool SymbolicFraction::operator==(const SymbolicFraction& rhs) const
    {
    if (m_numerator.size() != rhs.m_numerator.size())
        return false;

    if (m_denominator.size() != rhs.m_denominator.size())
        return false;

    return equal(m_numerator.begin(), m_numerator.end(), rhs.m_numerator.begin()) && equal(m_denominator.begin(), m_denominator.end(), rhs.m_denominator.begin());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool SymbolicFraction::operator!= (const SymbolicFraction& rhs) const
    {
    return !(*this == rhs);
    }
