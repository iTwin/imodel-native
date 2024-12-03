/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include <BeJsonCpp/BeJsonUtilities.h>

using namespace std;

BEGIN_BENTLEY_UNITS_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitsSymbol::UnitsSymbol(Utf8CP name) : m_name(name), m_symbolExpression(new Expression()) {}

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitsSymbol::UnitsSymbol(Utf8CP name, Utf8CP definition, double numerator, double denominator, double offset) :
    m_name(name), m_definition(definition), m_numerator(numerator), m_denominator(denominator),
    m_offset(offset), m_symbolExpression(new Expression())
    {
    m_isBaseSymbol = m_name.EqualsI(m_definition.c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitsSymbol::UnitsSymbol(Utf8CP name, Utf8CP definition) : m_name(name), m_definition(definition), m_symbolExpression(new Expression())
    {
    m_isBaseSymbol = m_name.EqualsI(m_definition.c_str());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitsSymbol::~UnitsSymbol()
    {
    if (nullptr != m_symbolExpression)
        delete m_symbolExpression;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void Unit::SetDisplayLabel(Utf8CP label) 
    {
    if (nullptr == label || '\0' == *label)
        {
        m_explicitlyDefinedDisplayLabel = false;
        m_displayLabel.clear();
        }
    else
        {
        m_explicitlyDefinedDisplayLabel = true;
        m_displayLabel = label;
        }
    } 

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
BentleyStatus UnitsSymbol::SetDefinition(Utf8CP definition)
    {
    if (!m_definition.empty() || nullptr == definition || Utf8String::IsNullOrEmpty(definition))
        return ERROR;
    m_definition = definition;

    m_isBaseSymbol = m_name.EqualsI(m_definition.c_str());
    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
BentleyStatus UnitsSymbol::SetNumerator(double numerator)
    {
    if (1.0 != m_numerator || 0.0 == numerator)
        return ERROR;
    m_numerator = numerator;
    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
BentleyStatus UnitsSymbol::SetDenominator(double denominator)
    {
    if (1.0 != m_denominator || 0.0 == denominator)
        return ERROR;
    m_denominator = denominator;
    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
BentleyStatus UnitsSymbol::SetOffset(double offset)
    {
    m_offset = offset;
    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ExpressionCR UnitsSymbol::Evaluate(int depth, std::function<UnitsSymbolCP(Utf8CP, IUnitsContextCP)> getSymbolByName) const
    {
    if (!m_evaluated)
        {
        BeMutexHolder lock(GetMutex());
        if (!m_evaluated)
            {
            m_symbolExpression->Add(this, 1);
            Expression::ParseDefinition(*this, depth, m_definition.c_str(), *m_symbolExpression, 1, getSymbolByName);
            m_evaluated = true;
            }
        }
    return *m_symbolExpression;
    }

//=======================================================================================
//! Unit
//=======================================================================================

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Unit::Unit(UnitSystemCR system, PhenomenonCR phenomenon, Utf8CP name, Utf8CP definition, double nominator, double denominator, double offset, bool isConstant)
    : UnitsSymbol(name, definition, nominator, denominator, offset), m_system(&system), m_isConstant(isConstant)
    {
    SetPhenomenon(phenomenon);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Unit::Unit(UnitSystemCR system, PhenomenonCR phenomenon, Utf8CP name, Utf8CP definition)
    : UnitsSymbol(name, definition), m_system(&system)
    {
    SetPhenomenon(phenomenon);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Unit::~Unit()
    {
    if (nullptr != m_phenomenon)
        {
        m_phenomenon->RemoveUnit(*this);
        m_phenomenon = nullptr;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
BentleyStatus Unit::SetPhenomenon(PhenomenonCR phenom)
    {
    if (nullptr != m_phenomenon)
        return ERROR;

    phenom.AddUnit(*this);
    m_phenomenon = &phenom;
    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ExpressionCR Unit::Evaluate() const
    {
    if (IsInvertedUnit())
        return m_parent->Evaluate();

    return T_Super::Evaluate(0, [=](Utf8CP unitName, IUnitsContextCP context)
        {
        return context->LookupUnit(unitName);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8StringCR Unit::GetInvariantDisplayLabel() const
    {
    if (!m_explicitlyDefinedDisplayLabel)
        return m_name;
    return m_displayLabel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool Unit::IsNumber() const
    {
    return (nullptr == m_phenomenon) ? false : m_phenomenon->IsNumber();
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

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitsProblemCode Unit::Convert(double& converted, double value, UnitCP toUnit) const
    {
    if (nullptr == toUnit)
        {
        LOG.errorv("Cannot convert from %s to a null toUnit, returning a conversion factor of 0.0", this->GetName().c_str());
        return UnitsProblemCode::UncomparableUnits;
        }

    if (std::isnan(value))
        {
        converted = std::numeric_limits<double>::quiet_NaN();
        return UnitsProblemCode::NaN;
        }
    
    if (IsInvertedUnit() == toUnit->IsInvertedUnit())
        return DoNumericConversion(converted, value, *toUnit);

    // at this point we know that we need to invert the value on one side
    double temp;
    UnitsProblemCode prob;
    if (IsInvertedUnit()) // from unit is the inverted one, so invert before conversion
        {
        if (IsNegligible(value))
            {
            converted = 0.0;
            return UnitsProblemCode::InvertingZero;
            }

        return DoNumericConversion(converted, 1.0 / value, *toUnit);
        }

    //the toUnit is the inverted one, so invert after conversion
    prob = DoNumericConversion(temp, value, *toUnit);
    if (UnitsProblemCode::NoProblem != prob)
        {
        converted = 0.0;
        return prob;
        }

    if (IsNegligible(temp))
        {
        converted = 0.0;
        return UnitsProblemCode::InvertingZero;
        }
    
    converted = 1.0 / temp;
    return UnitsProblemCode::NoProblem;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
bool Unit::GenerateConversion(UnitCR toUnit, Conversion& conversion) const
    {
    Expression conversionExpression;
    if (BentleyStatus::SUCCESS != Expression::GenerateConversionExpression(*this, toUnit, conversionExpression))
        {
        LOG.errorv("Cannot convert from %s to %s, units incompatible", this->GetName().c_str(), toUnit.GetName().c_str());
        return false;
        }

    conversion.Factor = 1.0;
    for (auto const& toUnitExp : conversionExpression)
        {
        if (toUnitExp.GetExponent() == 0)
            continue;

        LOG.tracev("Adding unit %s^%d to the conversion.  Factor: %.17g / %.17g  Offset:%.17g", toUnitExp.GetSymbol()->GetName().c_str(),
                  toUnitExp.GetExponent(), toUnitExp.GetSymbol()->GetNumerator(), toUnitExp.GetSymbol()->GetDenominator(), toUnitExp.GetSymbol()->GetOffset());
        double unitFactor = FastIntegerPower(toUnitExp.GetSymbolFactor(), abs(toUnitExp.GetExponent()));
        if (toUnitExp.GetExponent() > 0)
            {
            LOG.tracev("Multiplying existing factor %.17g by %.17g", conversion.Factor, unitFactor);
            conversion.Factor *= unitFactor;
            LOG.tracev("New factor %.17g", conversion.Factor);
            if (toUnitExp.GetSymbol()->HasOffset())
                {
                double unitOffset = toUnitExp.GetSymbol()->GetOffset() * toUnitExp.GetSymbolFactor();
                LOG.tracev("Adding %.17g to existing offset %.17g.", unitOffset, conversion.Offset);
                conversion.Offset += unitOffset;
                LOG.tracev("New offset %.17g", conversion.Offset);
                }
            else
                {
                LOG.tracev("Multiplying offset %.17g by units conversion factor %.17g", conversion.Offset, unitFactor);
                conversion.Offset *= unitFactor;
                LOG.tracev("New offset %.17g", conversion.Offset);
                }
            }
        else
            {
            LOG.tracev("Dividing existing factor %.17g by %.17g", conversion.Factor, unitFactor);
            conversion.Factor /= unitFactor;
            LOG.tracev("New factor %.17g", conversion.Factor);
            if (toUnitExp.GetSymbol()->HasOffset())
                {
                double unitOffset = toUnitExp.GetSymbol()->GetOffset() * toUnitExp.GetSymbolFactor();
                LOG.tracev("Subtracting %.17g from existing offset %.17g.", unitOffset, conversion.Offset);
                conversion.Offset -= unitOffset;
                LOG.tracev("New offset %.17g", conversion.Offset);
                }

            LOG.tracev("Dividing offset %.17g by units conversion factor %.17g", conversion.Offset, unitFactor);
            conversion.Offset /= unitFactor;
            LOG.tracev("New offset %.17g", conversion.Offset);
            }
        }

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitsProblemCode Unit::DoNumericConversion(double& converted, double value, UnitCR toUnit) const
    {
    converted = 0.0;
    Conversion conversion;
    GenerateConversion(toUnit, conversion);

    // Conversion caching not supported
    // if (!m_phenomenon->TryGetConversion(conversion, *this, toUnit))
    //    m_phenomenon->AddConversion(index, conversion);

    if (conversion.Factor == 0.0)
        {
        LOG.errorv("Cannot convert from %s to %s, units incompatible, returning a conversion factor of 0.0", this->GetName().c_str(), toUnit.GetName().c_str());
        return UnitsProblemCode::UncomparableUnits;
        }

    LOG.tracev("Conversion factor: %.17g, offset: %.17g", conversion.Factor, conversion.Offset);
    converted = value * conversion.Factor + conversion.Offset;
    return UnitsProblemCode::NoProblem;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Utf8String Unit::GetUnitSignature() const
    {
    Expression phenomenonExpression = Evaluate();
    Expression baseExpression;
    Expression::CreateExpressionWithOnlyBaseSymbols(phenomenonExpression, baseExpression);
    return baseExpression.ToString(false);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Utf8String Unit::GetParsedUnitExpression() const
    {
    return Evaluate().ToString(true);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitCP Unit::CombineWithUnit(UnitCR rhs, int factor) const
    {
    auto temp = Evaluate();
    auto expression2 = rhs.Evaluate();

    Expression expression;
    Expression::Copy(temp, expression);
    Expression::MergeExpressions(GetName().c_str(), expression, rhs.GetName().c_str(), expression2, factor);

    bvector<PhenomenonCP> phenomList;
    PhenomenonCP matchingPhenom = nullptr;
    m_unitsContext->AllPhenomena(phenomList);
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
    for (UnitCP const u : matchingPhenom->GetUnits())
        {
        temp = u->Evaluate();
        Expression unitExpr;
        Expression::Copy(temp, unitExpr);
        Expression::MergeExpressions(u->GetName().c_str(), unitExpr, GetName().c_str(), expression, -1);

        auto count = std::count_if(unitExpr.begin(), unitExpr.end(),
            [](ExpressionSymbolCR e) { return e.GetExponent() != 0; });

        if (count > 1)
            continue;

        output = u;
        break;
        }

    return output;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitCP Unit::MultiplyUnit(UnitCR rhs) const
    {
    return CombineWithUnit(rhs, 1);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitCP Unit::DivideUnit(UnitCR rhs) const
    {
    return CombineWithUnit(rhs, -1);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
bool Unit::AreCompatible(UnitCP unitA, UnitCP unitB)
    {
    return nullptr == unitA || nullptr == unitB ? false : Phenomenon::AreEqual(unitA->GetPhenomenon(), unitB->GetPhenomenon());
    }

//===================================================
// Phenomenon
//===================================================

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Phenomenon::Phenomenon(Utf8CP name, Utf8CP definition) : UnitsSymbol(name, definition, 0.0, 0.0, 0)
    {
    m_isNumber = m_definition.Equals(NUMBER);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Phenomenon::~Phenomenon()
    {
    for (const auto u : m_units)
        {
        if (nullptr != u)
            const_cast<UnitP>(u)->m_phenomenon = nullptr; // Const cast is okay here. Units are invalid without a phen. If this gets deleted we don't want them having a garbage ptr
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
UnitCP Phenomenon::LookupUnit(Utf8CP unitName) const
    {
    auto it = std::find_if(m_units.begin(), m_units.end(), [&unitName](UnitCP unit) {return unit->GetName().EqualsI(unitName);});
    if (m_units.end() == it)
        return nullptr;
    return *it;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Utf8String Phenomenon::GetPhenomenonSignature() const
    {
    Expression phenomenonExpression = Evaluate();
    Expression baseExpression;
    Expression::CreateExpressionWithOnlyBaseSymbols(phenomenonExpression, baseExpression);
    return baseExpression.ToString(false);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ExpressionCR Phenomenon::Evaluate() const
    {
    return T_Super::Evaluate(0, [](Utf8CP unitName, IUnitsContextCP context) {return context->LookupPhenomenon(unitName);});
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
bool Phenomenon::IsCompatible(UnitCR unit) const
    {
    return Expression::ShareSignatures(*this, unit);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
BentleyStatus Phenomenon::SetDefinition(Utf8CP definition)
    {
    if (SUCCESS != T_Super::SetDefinition(definition))
        return ERROR;
    if (m_definition.Equals(NUMBER))
        m_isNumber = true;
    return SUCCESS;
    }

bool Phenomenon::IsLength() const { return m_name.Equals(LENGTH); }
bool Phenomenon::IsTime() const { return m_name.Equals(TIME); }
bool Phenomenon::IsAngle() const { return m_name.Equals(ANGLE); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8StringCR Phenomenon::GetDisplayLabel() const
    {
    if (m_displayLabel.empty())
        {
        Utf8PrintfString stringId("PHENOMENON_%s", GetName().c_str());
        if(m_displayLabel.empty())
            return m_name;
        }

    return m_displayLabel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8StringCR Phenomenon::GetInvariantDisplayLabel() const
    {
    if (m_displayLabel.empty())
        return m_name;
    return m_displayLabel;
    }

END_BENTLEY_UNITS_NAMESPACE
