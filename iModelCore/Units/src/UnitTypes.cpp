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
Unit::Unit(Utf8CP system, PhenomenonCR phenomenon, Utf8CP name, int id, Utf8CP definition, Utf8Char dimensonSymbol, double factor, double offset, bool isConstant) :
    Symbol(name, definition, dimensonSymbol, id, factor, offset),
    m_system(system), m_isConstant(isConstant)
    {
    m_phenomenon = &phenomenon;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitP Unit::Create (Utf8CP sysName, PhenomenonCR phenomenon, Utf8CP unitName, int id, Utf8CP definition, Utf8Char dimensionSymbol, double factor, double offset, bool isConstant)
    {
    LOG.debugv("Creating unit %s  Factor: %lf  Offset: %d", unitName, factor, offset);
    return new Unit (sysName, phenomenon, unitName, id, definition, dimensionSymbol, factor, offset, isConstant);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::IsRegistered() const
    {
    return UnitRegistry::Instance().HasUnit(GetName());
    }

double Unit::GetConversionTo(UnitCP unit) const
    {
    if (nullptr == unit)
        {
        LOG.errorv("Cannot convert from %s to a null unit, returning a conversion factor of 0.0", this->GetName());
        return 0.0;
        }

    SymbolicExpression fromExpression = Evaluate([] (Utf8CP unitName) { return UnitRegistry::Instance().LookupUnit(unitName); });
    SymbolicExpression toExpression = unit->Evaluate([] (Utf8CP unitName) { return UnitRegistry::Instance().LookupUnit(unitName); });
    
    fromExpression.LogExpression(NativeLogging::SEVERITY::LOG_DEBUG, this->GetName());
    toExpression.LogExpression(NativeLogging::SEVERITY::LOG_DEBUG, unit->GetName());

    if (!SymbolicExpression::DimensionallyCompatible(fromExpression, toExpression))
        {
        LOG.errorv("Cannot convert from %s: (%s) to %s: (%s) because they two units are not dimensionally compatible.  Returning a conversion factor of 0.0", 
                   this->GetName(), this->GetDefinition(), unit->GetName(), unit->GetDefinition());
        return 0.0;
        }

    SymbolicExpression combinedExpression;
    SymbolicExpression::Copy(fromExpression, combinedExpression);

    SymbolicExpression::MergeExpressions(GetDefinition(), combinedExpression, unit->GetDefinition(), toExpression, -1);
    if (LOG.isSeverityEnabled(NativeLogging::SEVERITY::LOG_DEBUG))
        {
        Utf8PrintfString combinedName("%s/%s", this->GetName(), unit->GetName());
        combinedExpression.LogExpression(NativeLogging::SEVERITY::LOG_DEBUG, combinedName.c_str());
        }
    double factor = m_factor / unit->m_factor;
    for (auto const& unitExp : combinedExpression)
        {
        // TODO: Decide if this is really necessary ...  if exponent is zero then the factor is 1 so the question is it better to pow and multiply by 1 or have a branch ... suspect pow and mult
        if (unitExp->GetExponent() == 0 || unitExp->GetSymbol()->GetFactor() == 1.0)
            continue;
        
        factor *= pow(unitExp->GetSymbol()->GetFactor(), unitExp->GetExponent());
        }

    return factor;
    }

SymbolicExpression& Symbol::Evaluate(std::function<SymbolCP(Utf8CP)> getSymbolByName) const
    {
    if (!m_evaluated)
        {
        SymbolicExpression::ParseDefinition(m_definition.c_str(), m_symbolExpression, 1, getSymbolByName);
        m_evaluated = true;
        }
    return m_symbolExpression;
    }

Utf8String Phenomenon::GetPhenomenonDimension() const
    {
    SymbolicExpression phenomenonExpression = Evaluate([] (Utf8CP phenomenonName) { return UnitRegistry::Instance().LookupPhenomenon(phenomenonName); });
    SymbolicExpression baseExpression;
    SymbolicExpression::CreateExpressionWithOnlyBaseSymbols(phenomenonExpression, baseExpression, false);
    return baseExpression.ToString();
    }
