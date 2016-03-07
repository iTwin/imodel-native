/*--------------------------------------------------------------------------------------+
|
|     $Source: src/UnitTypes.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include "SymbolicExpression.h"

USING_NAMESPACE_BENTLEY_UNITS

Symbol::Symbol(Utf8CP name, Utf8CP definition, Utf8Char dimensionSymbol, int id, double factor, double offset) : 
    m_name(name), m_definition(definition), m_dimensionSymbol(dimensionSymbol), m_id(id), m_factor(factor), m_offset(offset), m_evaluated(false), 
    m_symbolExpression(new Expression())
    {
    m_dimensionless = strcmp("ONE", m_definition.c_str()) == 0;
    }

Symbol::~Symbol()
    {
    if (nullptr != m_symbolExpression)
        delete m_symbolExpression;
    }

Utf8CP Symbol::GetName() const
    {
    return m_name.c_str(); 
    }

int Symbol::GetId()   const
    {
    return m_id; 
    }

Utf8CP Symbol::GetDefinition() const
    {
    return m_definition.c_str(); 
    }

double Symbol::GetFactor() const
    {
    return m_factor; 
    }

bool Symbol::IsBaseSymbol() const
    {
    return ' ' != m_dimensionSymbol; 
    }

bool Symbol::IsDimensionless() const
    {
    return m_dimensionless; 
    }    

Expression& Symbol::Evaluate(int depth, std::function<SymbolCP(Utf8CP)> getSymbolByName) const
    {
    if (!m_evaluated)
        {
        Expression::ParseDefinition(depth, m_definition.c_str(), *m_symbolExpression, 1, getSymbolByName);
        m_evaluated = true;
        }
    return *m_symbolExpression;
    }

bool Symbol::IsCompatibleWith(SymbolCR rhs) const
    {
    return Expression::DimensionallyCompatible(*(this->m_symbolExpression), *(rhs.m_symbolExpression));
    }

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

int Unit::GetPhenomenonId() const { return GetPhenomenon()->GetId(); }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::IsRegistered() const
    {
    return UnitRegistry::Instance().HasUnit(GetName());
    }

Expression& Unit::Evaluate() const
    {
    return T_Super::Evaluate(0, [] (Utf8CP unitName) { return UnitRegistry::Instance().LookupUnit(unitName); });
    }

double Unit::GetConversionTo(UnitCP unit) const
    {
    if (nullptr == unit)
        {
        LOG.errorv("Cannot convert from %s to a null unit, returning a conversion factor of 0.0", this->GetName());
        return 0.0;
        }

    // TODO: USING A MAX RECRUSION DEPTH TO CATCH CYCLES IS SUPER HACKY AND MAKES CHRIS T. MAD.  Replace with something that actually detects cycles.
    Expression fromExpression = Evaluate();
    Expression toExpression = unit->Evaluate();
    
    fromExpression.LogExpression(NativeLogging::SEVERITY::LOG_DEBUG, this->GetName());
    toExpression.LogExpression(NativeLogging::SEVERITY::LOG_DEBUG, unit->GetName());

    if (!Expression::DimensionallyCompatible(fromExpression, toExpression))
        {
        LOG.errorv("Cannot convert from %s: (%s) to %s: (%s) because they two units are not dimensionally compatible.  Returning a conversion factor of 0.0", 
                   this->GetName(), this->GetDefinition(), unit->GetName(), unit->GetDefinition());
        return 0.0;
        }

    Expression combinedExpression;
    Expression::Copy(fromExpression, combinedExpression);

    Expression::MergeExpressions(GetDefinition(), combinedExpression, unit->GetDefinition(), toExpression, -1);
    if (LOG.isSeverityEnabled(NativeLogging::SEVERITY::LOG_DEBUG))
        {
        Utf8PrintfString combinedName("%s/%s", this->GetName(), unit->GetName());
        combinedExpression.LogExpression(NativeLogging::SEVERITY::LOG_DEBUG, combinedName.c_str());
        }
    double factor = GetFactor() / unit->GetFactor();
    for (auto const& unitExp : combinedExpression)
        {
       // TODO: Consider always doing positive exponents and dividing if exponent is negative
       factor *= pow(unitExp->GetSymbolFactor(), unitExp->GetExponent());
        }

    return factor;
    }

void Unit::MultiplyUnit (UnitCR rhs) const
    {
    auto temp = Evaluate();
    auto expression2 = rhs.Evaluate();

    Expression expression;
    Expression::Copy(temp, expression);
    Expression::MergeExpressions(GetName(), expression, rhs.GetName(), expression2, 1);

    bvector<PhenomenonCP> phenomList;
    PhenomenonCP matchingPhenom;
    UnitRegistry::Instance().AllPhenomena(phenomList);
    for (const auto p : phenomList)
        {
        if (!Expression::DimensionallyCompatible(p->Evaluate(), expression))
            continue;

        matchingPhenom = p;
        break;
        }
    }

Utf8String Phenomenon::GetPhenomenonDimension() const
    {
    Expression phenomenonExpression = Evaluate();
    Expression baseExpression;
    Expression::CreateExpressionWithOnlyBaseSymbols(phenomenonExpression, baseExpression, false);
    return baseExpression.ToString();
    }

void Phenomenon::AddUnit(UnitCR unit)
    {
    auto it = find_if(m_units.begin(), m_units.end(), [&unit] (UnitCP existingUnit) { return existingUnit->GetId() == unit.GetId(); });
    if (it == m_units.end())
        m_units.push_back(&unit);
    }

Expression& Phenomenon::Evaluate() const
    {
    return T_Super::Evaluate(0, [] (Utf8CP phenomenonName) { return UnitRegistry::Instance().LookupPhenomenon(phenomenonName); });
    }

bool Phenomenon::IsCompatible(UnitCR unit) const
    {
    return Expression::ShareDimensions(*this, unit);
    }
