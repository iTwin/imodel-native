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

UnitsSymbol::UnitsSymbol(Utf8CP name, Utf8CP definition, Utf8Char baseSymbol, uint32_t id, double factor, double offset) :
    m_name(name), m_definition(definition), m_baseSymbol(baseSymbol), m_id(id), m_factor(factor), m_offset(offset), m_evaluated(false), 
    m_symbolExpression(new Expression())
    {
    m_dimensionless = strcmp("ONE", m_definition.c_str()) == 0;
    }

UnitsSymbol::~UnitsSymbol()
    {
    if (nullptr != m_symbolExpression)
        delete m_symbolExpression;
    }

ExpressionCR UnitsSymbol::Evaluate(int depth, std::function<UnitsSymbolCP(Utf8CP)> getSymbolByName) const
    {
    if (!m_evaluated)
        {
        m_symbolExpression->Add(this, 1);
        Expression::ParseDefinition(*this, depth, m_definition.c_str(), *m_symbolExpression, 1, getSymbolByName);
        m_evaluated = true;
        }
    return *m_symbolExpression;
    }

// TODO: Is the definition correct here?
// TODO: We should probably restrict inverting units to units which are unitless.  If we do not then the inverted unit would have a different signature.
Unit::Unit(UnitCR parent, Utf8CP unitName, uint32_t id) :
    Unit(parent.GetUnitSystem(), *parent.GetPhenomenon(), unitName, id, parent.GetDefinition(), parent.GetBaseSymbol(), 0, 0, false)
    {
    m_parent = &parent;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Unit::Unit(Utf8CP system, PhenomenonCR phenomenon, Utf8CP name, uint32_t id, Utf8CP definition, Utf8Char dimensonSymbol, double factor, double offset, bool isConstant) :
    UnitsSymbol(name, definition, dimensonSymbol, id, factor, offset),
    m_system(system), m_parent(nullptr), m_isConstant(isConstant)
    {
    m_phenomenon = &phenomenon;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitP Unit::Create(Utf8CP sysName, PhenomenonCR phenomenon, Utf8CP unitName, uint32_t id, Utf8CP definition, Utf8Char baseSymbol, double factor, double offset, bool isConstant)
    {
    LOG.debugv("Creating unit %s  Factor: %.17g  Offset: %d", unitName, factor, offset);
    return new Unit(sysName, phenomenon, unitName, id, definition, baseSymbol, factor, offset, isConstant);
    }

uint32_t Unit::GetPhenomenonId() const { return GetPhenomenon()->GetId(); }

UnitP Unit::Create(UnitCR parentUnit, Utf8CP unitName, uint32_t id)
    {
    if (parentUnit.HasOffset())
        {
        LOG.errorv("Cannot create inverting unit %s with parent %s because parent has an offset.", unitName, parentUnit.GetName());
        return nullptr;
        }
    if (parentUnit.IsInverseUnit())
        {
        LOG.errorv("Cannot create inverting unit %s with parent %s because parent is an inverting unit", unitName, parentUnit.GetName());
        return nullptr;
        }
    
    LOG.debugv("Creating inverting unit %s with parent unit %s", unitName, parentUnit.GetName());
    return new Unit(parentUnit, unitName, id);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Unit::IsRegistered() const
    {
    return UnitRegistry::Instance().HasUnit(GetName());
    }

ExpressionCR Unit::Evaluate() const
    {
    if (IsInverseUnit())
        return m_parent->Evaluate();

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

    if (IsInverseUnit() && toUnit->IsInverseUnit() || !(IsInverseUnit() || toUnit->IsInverseUnit()))
        return DoNumericConversion(value, *toUnit);

    // TODO: Do better check here
    if (value == 0.0)
        return 0.0;

    if (IsInverseUnit())
        return DoNumericConversion(1.0 / value, *toUnit);
    
    return 1.0 / DoNumericConversion(value, *toUnit);
    }

bool Unit::GenerateConversion(UnitCR toUnit, Conversion& conversion) const
    {
    Expression conversionExpression;
    if (BentleyStatus::SUCCESS != Expression::GenerateConversionExpression(*this, toUnit, conversionExpression))
        {
        LOG.errorv("Cannot convert from %s to %s, units incompatible", this->GetName(), toUnit.GetName());
        return false;
        }

    conversion.Factor = 1.0;
    for (auto const& toUnitExp : conversionExpression)
        {
        if (toUnitExp.GetExponent() == 0)
            continue;

        LOG.infov("Adding unit %s^%d to the conversion.  Factor: %.17g  Offset:%.17g", toUnitExp.GetSymbol()->GetName(),
                  toUnitExp.GetExponent(), toUnitExp.GetSymbolFactor(), toUnitExp.GetSymbol()->GetOffset());
        double unitFactor = FastIntegerPower(toUnitExp.GetSymbolFactor(), abs(toUnitExp.GetExponent()));
        if (toUnitExp.GetExponent() > 0)
            {
            LOG.infov("Multiplying existing factor %.17g by %.17g", conversion.Factor, unitFactor);
            conversion.Factor *= unitFactor;
            LOG.infov("New factor %.17g", conversion.Factor);
            if (toUnitExp.GetSymbol()->HasOffset())
                {
                double unitOffset = toUnitExp.GetSymbol()->GetOffset() * toUnitExp.GetSymbol()->GetFactor();
                LOG.infov("Adding %.17g to existing offset %.17g.", unitOffset, conversion.Offset);
                conversion.Offset += unitOffset;
                LOG.infov("New offset %.17g", conversion.Offset);
                }
            else
                {
                LOG.infov("Multiplying offset %.17g by units conversion factor %.17g", conversion.Offset, unitFactor);
                conversion.Offset *= unitFactor;
                LOG.infov("New offset %.17g", conversion.Offset);
                }
            }
        else
            {
            LOG.infov("Dividing existing factor %.17g by %.17g", conversion.Factor, unitFactor);
            conversion.Factor /= unitFactor;
            LOG.infov("New factor %.17g", conversion.Factor);
            if (toUnitExp.GetSymbol()->HasOffset())
                {
                double unitOffset = toUnitExp.GetSymbol()->GetOffset() * toUnitExp.GetSymbol()->GetFactor();
                LOG.infov("Subtracting %.17g from existing offset %.17g.", unitOffset, conversion.Offset);
                conversion.Offset -= unitOffset;
                LOG.infov("New offset %l.17g", conversion.Offset);
                }

            LOG.infov("Dividing offset %.17g by units conversion factor %.17g", conversion.Offset, unitFactor);
            conversion.Offset /= unitFactor;
            LOG.infov("New offset %.17g", conversion.Offset);
            }
        }
    
    return true;
    }

double Unit::DoNumericConversion(double value, UnitCR toUnit) const
    {
    uint64_t index = GetId();
    index = index << 32;
    index |= toUnit.GetId();

    Conversion conversion;
    if (!UnitRegistry::Instance().TryGetConversion(index, conversion))
        {
        GenerateConversion(toUnit, conversion);
        UnitRegistry::Instance().AddConversion(index, conversion);
        }

    if (conversion.Factor == 0.0)
        {
        LOG.errorv("Cannot convert from %s to %s, units incompatible, returning a conversion factor of 0.0", this->GetName(), toUnit.GetName());
        return 0.0;
        }


    LOG.infov("Conversion factor: %.17g, offset: %.17g", conversion.Factor, conversion.Offset);
    value *= conversion.Factor;

    value += conversion.Offset;

    return value;
    }

Utf8String Unit::GetUnitSignature() const
    {
    Expression phenomenonExpression = Evaluate();
    Expression baseExpression;
    Expression::CreateExpressionWithOnlyBaseSymbols(phenomenonExpression, baseExpression);
    return baseExpression.ToString(false);
    }

Utf8String Unit::GetParsedUnitExpression() const
    {
    return Evaluate().ToString(true);
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
        if (!Expression::ShareSignatures(*p, expression))
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

Utf8String Phenomenon::GetPhenomenonSignature() const
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
    return Expression::ShareSignatures(*this, unit);
    }
