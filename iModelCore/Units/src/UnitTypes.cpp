/*--------------------------------------------------------------------------------------+
|
|     $Source: src/UnitTypes.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include <queue>

USING_NAMESPACE_BENTLEY_UNITS
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Unit::Unit(Utf8CP system, Utf8CP phenomena, Utf8CP name, Utf8CP definition, Utf8Char dimensonSymbol, double factor, double offset, bool isConstant) :
    m_name(name), m_system(system), m_phenomenon(phenomena), m_definition(definition), m_factor(factor), m_offset(offset), m_dimensionSymbol(dimensonSymbol),
    m_isConstant(isConstant), m_evaluated(false)
    {
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitP Unit::Create (Utf8CP sysName, Utf8CP phenomName, Utf8CP unitName, Utf8CP definition, Utf8Char dimensionSymbol, double factor, double offset, bool isConstant)
    {
    LOG.debugv("Creating unit %s  Factor: %lf  Offset: %d", unitName, factor, offset);
    return new Unit (sysName, phenomName, unitName, definition, dimensionSymbol, factor, offset, isConstant);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::IsRegistered() const
    {
    return UnitRegistry::Instance().HasUnit(GetName());
    }

void Unit::PrintForumula(UnitCP unit, bvector<UnitExponent*> expression) const
    {
    LOG.debugv("Formula for: %s", unit->GetName());
    Utf8String output;
    for (auto const& uWE : expression)
        {
        Utf8PrintfString uWEString("Unit: %s   UnitFactor: %lf   UnitExponent: %d \n", uWE->m_unit->GetName(), uWE->m_unit->GetFactor(), uWE->m_exponent);
        output.append(uWEString.c_str());
        }
    LOG.debug(output.c_str());
    }

double Unit::GetConversionTo(UnitCP unit) const
    {
    if (!m_evaluated)
        Evaluate();
    if (!unit->m_evaluated)
        unit->Evaluate();
    
    if (LOG.isSeverityEnabled(NativeLogging::SEVERITY::LOG_DEBUG))
        {
        PrintForumula(this, m_unitFormula);
        PrintForumula(unit, unit->m_unitFormula);
        }

    // TODO: Minify forumlas
    // TODO: Check that formulas are compatible
    
    double fromFactor = m_factor;
    LOG.debugv("Starting from factor for %s: %lf", GetName(), fromFactor);
    for (auto const& uWE : m_unitFormula)
        {
        fromFactor *= pow(uWE->m_unit->GetFactor(), uWE->m_exponent);
        }
    LOG.debugv("Final From factor for %s: %lf", GetName(), fromFactor);
    double toFactor = unit->m_factor;
    LOG.debugv("Starting to factor for %s: %lf", unit->GetName(), toFactor);
    for (auto const& uWE : unit->m_unitFormula)
        {
        toFactor *= pow(uWE->m_unit->GetFactor(), uWE->m_exponent);
        }
    LOG.debugv("Final To factor for %s: %lf", unit->GetName(), toFactor);
    return fromFactor / toFactor;
    }

bvector<Unit::UnitExponent*>& Unit::Evaluate() const
    {
    if (!m_evaluated)
        {
        ParseDefinition(m_definition.c_str(), m_unitFormula, 1);
        m_evaluated = true;
        }
    return m_unitFormula;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::operator== (UnitCR rhs) const
    {
    return false;//TODO: Replace with comparison of unit formula T_Super::operator==(rhs);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::operator!= (UnitCR rhs) const
    {
    return !(*this == rhs);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCR Unit::operator* (UnitCR rhs) const
    {
    auto result = SymbolicFraction(m_definition.c_str());//T_Super::operator/(rhs);

    UnitCP unit = UnitRegistry::Instance().LookupUnitBySubTypes(result.Numerator(), result.Denominator());
    if (nullptr != unit)
        return *unit;

    Utf8Vector n = Utf8Vector();
    copy (result.Numerator().begin(), result.Numerator().end(), n.begin());

    Utf8Vector d = Utf8Vector();
    copy (result.Denominator().begin(), result.Denominator().end(), d.begin());

    return rhs;// move(Unit(n, d));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCR Unit::operator/ (UnitCR rhs) const
    {
    auto result = SymbolicFraction(m_definition.c_str());//T_Super::operator/(rhs);

    UnitCP unit = UnitRegistry::Instance().LookupUnitBySubTypes(result.Numerator(), result.Denominator());
    if (nullptr != unit)
        return *unit;

    Utf8Vector n = Utf8Vector();
    copy (result.Numerator().begin(), result.Numerator().end(), n.begin());

    Utf8Vector d = Utf8Vector();
    copy (result.Denominator().begin(), result.Denominator().end(), d.begin());

    return rhs; // move(Unit(n, d));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SymbolicFraction::SimplifySubTypes(Utf8Vector &n, Utf8Vector &d)
    {
    // No intersection if one vector is empty
    if (n.size() == 0 || d.size() == 0)
        return;
    
    auto temp = Utf8Vector();
    
    // Remove the intersection between the two vectors.
    set_difference (n.begin(), n.end(), d.begin(), d.end(), temp.begin());
    n.erase(n.begin(), n.end());
    move(temp.begin(), temp.end(), n.begin());
    temp.erase(temp.begin(), temp.end());

    // We can stop if either one is empty
    if (n.size() == 0 || d.size() == 0)
        return;

    set_difference(d.begin(), d.end(), n.begin(), n.end(), temp.begin());
    d.erase(d.begin(), d.end());
    move(temp.begin(), temp.end(), d.begin());
    }

SymbolicFraction::SymbolicFraction(Utf8CP definition)
    {
    //ParseDefinition(definition, m_numerator, m_denominator);
    }
SymbolicFraction::SymbolicFraction(Utf8Vector& numerator, Utf8Vector& denominator)
    {
    m_numerator = move(numerator);
    m_denominator = move(denominator);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
SymbolicFraction SymbolicFraction::operator*(SymbolicFractionCR rhs) const
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
SymbolicFraction SymbolicFraction::operator/(SymbolicFractionCR rhs) const
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
bool SymbolicFraction::operator==(SymbolicFractionCR rhs) const
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
bool SymbolicFraction::operator!= (SymbolicFractionCR rhs) const
    {
    return !(*this == rhs);
    }
