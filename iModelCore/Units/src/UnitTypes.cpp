/*--------------------------------------------------------------------------------------+
|
|     $Source: src/UnitTypes.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include <Parser.h>

USING_NAMESPACE_BENTLEY_UNITS

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Unit::Unit(Utf8CP system, Utf8CP phenomena, Utf8CP name, Utf8Vector& numerator, Utf8Vector& denominator, double factor, double offset) : SymbolicFraction(numerator, denominator),
    m_name(name), m_system (system), m_factor(factor), m_offset(offset)
    {
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitPtr Unit::Create (Utf8CP sysName, Utf8CP phenomName, Utf8CP unitName, Utf8CP definition, double factor, double offset)
    {
    auto n = Utf8Vector();
    auto d = Utf8Vector();
    if (SUCCESS != ParseDefinition (definition, n, d))
        return nullptr;
    
    return new Unit (sysName, phenomName, unitName, n, d, factor, offset);
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
bool Unit::operator== (const UnitR rhs) const
    {
    return T_Super::operator==(rhs);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::operator!= (const UnitR rhs) const
    {
    return !(*this == rhs);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Unit Unit::operator* (const UnitR rhs) const
    {
    Unit result = *this;
    result *= rhs;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Unit Unit::operator/ (const UnitR rhs) const
    {
    Unit result = *this;
    result /= rhs;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Unit Unit::operator+ (const UnitR rhs) const
    {
    Unit result = *this;
    result += rhs;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Unit Unit::operator- (const UnitR rhs) const
    {
    Unit result = *this;
    result -= rhs;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitR Unit::operator+= (const UnitR rhs)
    {
    // TODO: This might not be right.
    return *this;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitR Unit::operator-= (const UnitR rhs)
    {
    // TODO: This might not be right.
    return *this;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitR Unit::operator*= (const UnitR rhs)
    {
    T_Super::operator*=(rhs);

    auto newUnit = UnitRegistry::Instance().LookupUnitBySubTypes(GetNumerator(), GetDenominator());
    if (newUnit != nullptr)
        return *newUnit;

    return *this;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitR Unit::operator/= (const UnitR rhs)
    {
    T_Super::operator/=(rhs);

    auto newUnit = UnitRegistry::Instance().LookupUnitBySubTypes(GetNumerator(), GetDenominator());
    if (newUnit != nullptr)
        return *newUnit;

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

SymbolicFraction::SymbolicFraction(Utf8Vector& numerator, Utf8Vector& denominator)
    {
    m_numerator = move(numerator);
    m_denominator = move(denominator);
    }

SymbolicFraction& SymbolicFraction::operator*=(const SymbolicFraction& rhs)
    {
    // Combine numerator and denominators.
    for_each(rhs.m_numerator.begin(), rhs.m_numerator.end(),
             [&] (Utf8String s) { m_numerator.push_back(s); });
    sort(m_numerator.begin(), m_numerator.end());

    for_each(rhs.m_denominator.begin(), rhs.m_denominator.end(),
             [&] (Utf8String s) { m_denominator.push_back(s); });
    sort(m_denominator.begin(), m_denominator.end());

    Simplify();
    return *this;
    }

SymbolicFraction& SymbolicFraction::operator/=(const SymbolicFraction& rhs)
    {
    // Combine numerator and denominators.
    for_each(rhs.m_numerator.begin(), rhs.m_numerator.end(),
             [&] (Utf8String s) { m_denominator.push_back(s); });
    sort(m_denominator.begin(), m_denominator.end());

    for_each(rhs.m_denominator.begin(), rhs.m_denominator.end(),
             [&] (Utf8String s) { m_numerator.push_back(s); });
    sort(m_numerator.begin(), m_numerator.end());

    Simplify();
    return *this;
    }

bool SymbolicFraction::operator==(const SymbolicFraction& rhs) const
    {
    if (m_numerator.size() != rhs.m_numerator.size())
        return false;

    if (m_denominator.size() != rhs.m_denominator.size())
        return false;

    return equal(m_numerator.begin(), m_numerator.end(), rhs.m_numerator.begin()) && equal(m_denominator.begin(), m_denominator.end(), rhs.m_denominator.begin());
    }

bool SymbolicFraction::operator!= (const SymbolicFraction& rhs) const
    {
    return !(*this == rhs);
    }
