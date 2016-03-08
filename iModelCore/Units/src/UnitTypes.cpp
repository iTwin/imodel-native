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

ExpressionCR Symbol::Evaluate(int depth, std::function<SymbolCP(Utf8CP)> getSymbolByName) const
    {
    if (!m_evaluated)
        {
        m_symbolExpression->Add(this, 1);
        Expression::ParseDefinition(*this, depth, m_definition.c_str(), *m_symbolExpression, 1, getSymbolByName);
        m_evaluated = true;
        }
    return *m_symbolExpression;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Unit::Unit(Utf8CP system, PhenomenonCR phenomenon, Utf8CP name, int id, Utf8CP definition, Utf8Char dimensonSymbol, double factor, double offset) :
    Symbol(name, definition, dimensonSymbol, id, factor, offset),
    m_system(system)
    {
    m_phenomenon = &phenomenon;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitP Unit::Create(Utf8CP sysName, PhenomenonCR phenomenon, Utf8CP unitName, int id, Utf8CP definition, Utf8Char dimensionSymbol, double factor, double offset)
    {
    LOG.debugv("Creating unit %s  Factor: %.17g  Offset: %d", unitName, factor, offset);
    return new Unit(sysName, phenomenon, unitName, id, definition, dimensionSymbol, factor, offset);
    }

int Unit::GetPhenomenonId() const { return GetPhenomenon()->GetId(); }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::IsRegistered() const
    {
    return UnitRegistry::Instance().HasUnit(GetName());
    }

ExpressionCR Unit::Evaluate() const
    {
    return T_Super::Evaluate(0, [] (Utf8CP unitName) { return UnitRegistry::Instance().LookupUnit(unitName); });
    }

double FastIntegerPower(double a, uint32_t n)
    {
    double q = a;
    double product = (n & 0x01) ? a : 1.0;
    n = n >> 1;
    while (n > 0)
        {
        q = q * q;
        if (n & 0x01)
            product *= q;
        n = n >> 1;
        }
    return product;
    }

double Unit::Convert(double value, UnitCP toUnit) const
    {
    if (nullptr == toUnit)
        {
        LOG.errorv("Cannot convert from %s to a null toUnit, returning a conversion factor of 0.0", this->GetName());
        return 0.0;
        }

    Expression conversionExpression;
    if (BentleyStatus::SUCCESS != Expression::GenerateConversionExpression(*this, *toUnit, conversionExpression))
        {
        LOG.errorv("Cannot convert from %s to %s, units incompatible, returning a conversion factor of 0.0", this->GetName(), toUnit->GetName());
        return 0.0;
        }

    double factor = 1;
    double offset = 0;
    for (auto const& toUnitExp : conversionExpression)
        {
        if (toUnitExp.GetExponent() == 0)
            continue;
        
        LOG.infov("Adding unit %s^%d to the conversion.  Factor: %.17g  Offset:%.17g", toUnitExp.GetSymbol()->GetName(), 
                  toUnitExp.GetExponent(), toUnitExp.GetSymbolFactor(), toUnitExp.GetSymbol()->GetOffset());
        double unitFactor = FastIntegerPower(toUnitExp.GetSymbolFactor(), abs(toUnitExp.GetExponent()));
        if (toUnitExp.GetExponent() > 0)
            {
            LOG.infov("Multiplying existing factor %.17g by %.17g", factor, unitFactor);
            factor *= unitFactor;
            LOG.infov("New factor %.17g", factor);
            if (toUnitExp.GetSymbol()->HasOffset())
                {
                double unitOffset = toUnitExp.GetSymbol()->GetOffset() * toUnitExp.GetSymbol()->GetFactor();
                LOG.infov("Adding %.17g to existing offset %.17g.", unitOffset, offset);
                offset += unitOffset;
                LOG.infov("New offset %.17g", offset);
                }
            else
                {
                LOG.infov("Multiplying offset %.17g by units conversion factor %.17g", offset, unitFactor);
                offset *= unitFactor;
                LOG.infov("New offset %.17g", offset);
                }
            }
        else
            {
            LOG.infov("Dividing existing factor %.17g by %.17g", factor, unitFactor);
            factor /= unitFactor;
            LOG.infov("New factor %.17g", factor);
            if (toUnitExp.GetSymbol()->HasOffset())
                {
                double unitOffset = toUnitExp.GetSymbol()->GetOffset() * toUnitExp.GetSymbol()->GetFactor();
                LOG.infov("Subtracting %.17g from existing offset %.17g.", unitOffset, offset);
                offset -= unitOffset;
                LOG.infov("New offset %l.17g", offset);
                }

            LOG.infov("Dividing offset %.17g by units conversion factor %.17g", offset, unitFactor);
            offset /= unitFactor;
            LOG.infov("New offset %.17g", offset);
            }
        }
    
    LOG.infov("Conversion factor: %.17g, offset: %.17g", factor, offset);
    value *= factor;

    value += offset;

    return value;
    }

Utf8String Unit::GetUnitDimension() const
    {
    Expression phenomenonExpression = Evaluate();
    Expression baseExpression;
    Expression::CreateExpressionWithOnlyBaseSymbols(phenomenonExpression, baseExpression);
    return baseExpression.ToString(false);
    }

UnitCP Unit::CombineWithUnit(UnitCR rhs, int factor) const
    {
    auto temp = Evaluate();
    auto expression2 = rhs.Evaluate();

    Expression expression;
    Expression::Copy(temp, expression);
    Expression::MergeExpressions(GetName(), expression, rhs.GetName(), expression2, factor);

    bvector<PhenomenonCP> phenomList;
    PhenomenonCP matchingPhenom = nullptr;
    UnitRegistry::Instance().AllPhenomena(phenomList);
    for (const auto p : phenomList)
        {
        if (!Expression::ShareDimensions(*p, expression))
            continue;

        matchingPhenom = p;
        break;
        }

    if (nullptr == matchingPhenom)
        return nullptr;

    UnitCP output = nullptr;
    for (const auto u : matchingPhenom->GetUnits())
        {
        temp = u->Evaluate();

        Expression unitExpr;
        Expression::Copy(temp, unitExpr);
        Expression::MergeExpressions(u->GetName(), unitExpr, GetName(), expression, -1);

        auto count = std::count_if(unitExpr.begin(), unitExpr.end(), 
            [](ExpressionSymbolCR e) { return e.GetExponent() != 0; });
        
        if (count > 1)
            continue;

        output = u;
        break;
        }

    return output;
    }

UnitCP Unit::MultiplyUnit(UnitCR rhs) const
    {
    return CombineWithUnit(rhs, 1);
    }

UnitCP Unit::DivideUnit(UnitCR rhs) const
    {
    return CombineWithUnit(rhs, -1);
    }

Utf8String Phenomenon::GetPhenomenonDimension() const
    {
    Expression phenomenonExpression = Evaluate();
    Expression baseExpression;
    Expression::CreateExpressionWithOnlyBaseSymbols(phenomenonExpression, baseExpression);
    return baseExpression.ToString(false);
    }

void Phenomenon::AddUnit(UnitCR unit)
    {
    auto it = find_if(m_units.begin(), m_units.end(), [&unit] (UnitCP existingUnit) { return existingUnit->GetId() == unit.GetId(); });
    if (it == m_units.end())
        m_units.push_back(&unit);
    }

ExpressionCR Phenomenon::Evaluate() const
    {
    return T_Super::Evaluate(0, [] (Utf8CP phenomenonName) { return UnitRegistry::Instance().LookupPhenomenon(phenomenonName); });
    }

bool Phenomenon::IsCompatible(UnitCR unit) const
    {
    return Expression::ShareDimensions(*this, unit);
    }
