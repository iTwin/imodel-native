/*--------------------------------------------------------------------------------------+
|
|     $Source: src/UnitTypes.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include "SymbolicExpression.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include "../Localization/xliffs/Units.xliff.h"
#include <BeSQLite/L10N.h>
#include <Bentley/md5.h>

using namespace std;

USING_NAMESPACE_BENTLEY_UNITS

UnitsSymbol::UnitsSymbol(Utf8CP name, Utf8CP definition, Utf8Char baseSymbol, uint32_t id, double factor, double offset) :
    m_name(name), m_definition(definition), m_baseSymbol(baseSymbol), m_id(id), m_factor(factor), m_offset(offset), m_evaluated(false), 
    m_symbolExpression(new Expression())
    {
    m_dimensionless = strcmp("ONE", m_definition.c_str()) == 0;
    }

UnitsSymbol::UnitsSymbol() // creates a default - invalid - Symbol
    {
    m_name = nullptr;
    m_definition = nullptr;
    m_id = 0;
    m_baseSymbol = '\0';
    m_factor = 0.0;
    m_offset = 0.0;
    m_dimensionless = true;
    m_evaluated = false;
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
    Unit(parent.GetUnitSystem(), *(parent.GetPhenomenon()), unitName, id, parent.GetDefinition(), parent.GetBaseSymbol(), 0, 0, false)
    {
    m_system = parent.GetUnitSystem();
    m_phenomenon = parent.GetPhenomenon();
    SetName(unitName);
    m_parent = &parent;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Unit::Unit(Utf8CP system, PhenomenonCR phenomenon, Utf8CP name, uint32_t id, Utf8CP definition, Utf8Char dimensonSymbol, 
           double factor, double offset, bool isConstant) : UnitsSymbol(name, definition, dimensonSymbol, id, factor, offset),
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  11/2017
//---------------------------------------------------------------------------------------
Utf8CP Unit::GetLabel() const
    {
    if (m_displayLabel.empty())
        {
        MD5 md5;
        Utf8PrintfString fullUnitName("%s:%s", GetPhenomenon()->GetName(), GetName());
        Utf8PrintfString resname("LABEL_%s", md5(fullUnitName).c_str());
        m_displayLabel = BeSQLite::L10N::GetString(BeSQLite::L10N::NameSpace("unit_labels"), BeSQLite::L10N::StringId(resname.c_str()));

        if (m_displayLabel.empty())
            {
            LOG.errorv("Missing localized label for Unit %s", GetName());
            m_displayLabel = GetName();
            }
        }

    return m_displayLabel.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  11/2017
//---------------------------------------------------------------------------------------
Utf8CP Unit::GetDescription() const
    {
    if (m_displayDescription.empty())
        {
        MD5 md5;
        Utf8PrintfString fullUnitName("%s:%s", GetPhenomenon()->GetName(), GetName());
        Utf8PrintfString resname("DESCRIPTION_%s", md5(fullUnitName).c_str());
        m_displayDescription = BeSQLite::L10N::GetString(BeSQLite::L10N::NameSpace("unit_labels"), BeSQLite::L10N::StringId(resname.c_str()));

        if (m_displayLabel.empty())
            {
            LOG.errorv("Missing localized description for Unit %s", GetLabel());
            m_displayLabel = GetName();
            }
        }

    return m_displayDescription.c_str();
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

UnitsProblemCode Unit::Convert(double& converted, double value, UnitCP toUnit) const
    {
    if (nullptr == toUnit)
        {
        LOG.errorv("Cannot convert from %s to a null toUnit, returning a conversion factor of 0.0", this->GetName());
        return UnitsProblemCode::UncomparableUnits;
        }

    if (IsInverseUnit() && toUnit->IsInverseUnit() || !(IsInverseUnit() || toUnit->IsInverseUnit()))
        {
        return DoNumericConversion(converted, value, *toUnit);
        }
    // TODO: Do better check here
    //if (value == 0.0)
    //    return 0.0;
    double temp;
    UnitsProblemCode prob;
    if (IsInverseUnit())
        {
        if (IsNegligible(value))
            {
            converted = 0.0;
            prob =  UnitsProblemCode::InvertingZero;
            }
        else
            prob = DoNumericConversion(converted, 1.0 / value, *toUnit);
        }
    else
        {
        prob = DoNumericConversion(temp, value, *toUnit);
        if (UnitsProblemCode::NoProblem != prob || IsNegligible(temp))
            converted = 0.0;
        else
            converted = 1.0 / temp;
        }
    return prob;
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

UnitsProblemCode Unit::DoNumericConversion(double& converted, double value, UnitCR toUnit) const
    {
    uint64_t index = GetId();
    index = index << 32;
    index |= toUnit.GetId();

    converted = 0.0;
    Conversion conversion;
    if (!UnitRegistry::Instance().TryGetConversion(index, conversion))
        {
        GenerateConversion(toUnit, conversion);
        UnitRegistry::Instance().AddConversion(index, conversion);
        }

    if (conversion.Factor == 0.0)
        {
        LOG.errorv("Cannot convert from %s to %s, units incompatible, returning a conversion factor of 0.0", this->GetName(), toUnit.GetName());
        return UnitsProblemCode::UncomparableUnits;
        }

    LOG.infov("Conversion factor: %.17g, offset: %.17g", conversion.Factor, conversion.Offset);
    converted = value * conversion.Factor + conversion.Offset;
    return UnitsProblemCode::NoProblem;
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
    for (UnitCP const u : matchingPhenom->GetUnits())
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

bool Unit::AreCompatible(UnitCP unitA, UnitCP unitB)
    {
    return nullptr == unitA || nullptr == unitB ? false : Phenomenon::AreEqual(unitA->GetPhenomenon(), unitB->GetPhenomenon());
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

bool Phenomenon::IsLength() const { return m_name.Equals(LENGTH); }
bool Phenomenon::IsTime() const { return m_name.Equals(TIME); }
bool Phenomenon::IsAngle() const { return m_name.Equals(ANGLE); }

//===================================================
//
// UnitSynonymMap Methods
//
//===================================================

void UnitSynonymMap::Init(Utf8CP unitName, Utf8CP synonym)
    {
    m_unit = (nullptr == unitName)? nullptr : UnitRegistry::Instance().LookupUnitCI(unitName);
    if (nullptr == m_unit)
        m_synonym.clear();
    else
        m_synonym = synonym;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
UnitSynonymMap::UnitSynonymMap(Utf8CP unitName, Utf8CP synonym)
    {
    Init(nullptr, nullptr);
    if (!Utf8String::IsNullOrEmpty(unitName))  //if first argument is empty the map will be empty as well
        {
        if (Utf8String::IsNullOrEmpty(synonym)) // only first argument is passed
            {
            while (isspace(*unitName)) unitName++; // skip blanks
            if ('{' == *unitName) // indicator of the Json string
                {
                Json::Value jval (Json::objectValue);
                Json::Reader::Parse(unitName, jval);
                LoadJson(jval);
                }
            else
                {
                bvector<Utf8String> tokens;
                BeStringUtilities::Split(unitName, ", ", nullptr, tokens);
                if (tokens.size() == 2)
                    Init(tokens[0].c_str(), tokens[1].c_str());
                }
            }
        else
            Init(unitName, synonym);
        }
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
// The description of the map could be either a plain text string <UnitName>,<synonym>
//   or a Json text string that starts and ends by the "curvy brackets". If this is the case
//    this function attempts to parse the string into a Json-object that will be used for
//     populating the instance
//----------------------------------------------------------------------------------------
//UnitSynonymMap::UnitSynonymMap(Utf8CP descr)
//    {
//    Init(nullptr, nullptr);
//    if (nullptr != descr && *descr != '\0')
//        {
//        while (isspace(*descr)) descr++; // skip blanks
//        if ('{' == *descr) // indicator of the Json string
//            {
//            Json::Value jval (Json::objectValue);
//            Json::Reader::Parse(descr, jval);
//            LoadJson(jval);
//            }
//        else
//            {
//            bvector<Utf8String> tokens;
//            BeStringUtilities::Split(descr, ", ", nullptr, tokens);
//            if (tokens.size() == 2)
//                Init(tokens[0].c_str(), tokens[1].c_str());
//            }
//        }
//    }

void UnitSynonymMap::LoadJson(Json::Value jval)
    {
    m_unit = nullptr;
    m_synonym.clear();
    if (jval.empty())
        return;
    Utf8CP paramName;
    Utf8String formatName;
    Utf8String unitName;
    Utf8String synonym;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        paramName = iter.memberName();
        JsonValueCR val = *iter;
        if (BeStringUtilities::StricmpAscii(paramName, json_unitName()) == 0)
            unitName = val.asString();
        else if (BeStringUtilities::StricmpAscii(paramName, json_synonym()) == 0)
            synonym = val.asString();
        }
    Init(unitName.c_str(), synonym.c_str());
    }

UnitSynonymMap::UnitSynonymMap(Json::Value jval)
    {
    LoadJson(jval);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
Json::Value UnitSynonymMap::ToJson()
    {
    Json::Value jval;
    if (nullptr != m_unit)
        {
        jval[json_unitName()] = m_unit->GetName();
        jval[json_synonym()] = m_synonym.c_str();
        }
    return jval;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
bool UnitSynonymMap::IsIdentical(UnitSynonymMapCR other)
    {
    if (m_unit != other.m_unit) return false;
    if (BeStringUtilities::StricmpAscii(m_synonym.c_str(), other.m_synonym.c_str()) != 0) return false;
    return true;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
bool UnitSynonymMap::AreVectorsIdentical(bvector<UnitSynonymMap>& v1, bvector<UnitSynonymMap>& v2)
    {
    if (v1.size() != v2.size()) return false;
    for (size_t i = 0; i < v1.size(); i++)
        {
        if (!v1[i].IsIdentical(v2[i])) return false;
        }
    return true;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 09/17
//----------------------------------------------------------------------------------------
bvector<UnitSynonymMap> UnitSynonymMap::MakeUnitSynonymVector(Json::Value jval)
    {
    UnitSynonymMap map;
    Json::Value val;
    bvector<UnitSynonymMap> mapV;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        val = *iter;
        map = UnitSynonymMap(val);
        mapV.push_back(map);
        }
    return mapV;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 09/17
//----------------------------------------------------------------------------------------
size_t UnitSynonymMap::AugmentUnitSynonymVector(bvector<UnitSynonymMap>& mapV, Utf8CP unitName, Utf8CP synonym)
    {
    UnitSynonymMap map(unitName, synonym);
    mapV.push_back(map);
    return mapV.size();
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 09/17
//----------------------------------------------------------------------------------------
bool UnitSynonymMap::CompareSynonymMap(UnitSynonymMapCR map1, UnitSynonymMapCR map2)
    {
    PhenomenonCP p1 = map1.GetPhenomenon();
    PhenomenonCP p2 = map2.GetPhenomenon();
    if (nullptr == p1) // objects not to be moved if either one or both do not yield a Phenomenon
        return false;
    if (nullptr == p2)
        return true;
    int diff = BeStringUtilities::StricmpAscii(p1->GetName(), p2->GetName());
    if (diff < 0)
        return true;
    if (diff > 0)
        return false;
    UnitCP un1 = map1.GetUnit();
    UnitCP un2 = map2.GetUnit();
    diff = BeStringUtilities::StricmpAscii(un1->GetName(), un2->GetName());
    if (diff < 0)
        return true;
    if (diff > 0)
        return false;
    diff = BeStringUtilities::StricmpAscii(map1.GetSynonym(), map2.GetSynonym());
    if (diff <= 0)
        return true;
    return false;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
UnitCP Phenomenon::FindSynonym(Utf8CP synonym) const
    {
    if (m_altNames.size() > 0)  // there are some alternative names
        {
        for (const UnitSynonymMap* syn = m_altNames.begin(); syn != m_altNames.end(); syn++)
            {
            if (0 == BeStringUtilities::StricmpAscii(synonym, syn->GetSynonym()))
                return syn->GetUnit();
            }
        }
    return nullptr;
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
UnitCP Phenomenon::LookupUnit(Utf8CP unitName)
    {
    UnitCP un = FindSynonym(unitName);
    if (nullptr != un)
        return un;
    un = UnitRegistry::Instance().LookupUnitCI(unitName);
    PhenomenonCP ph = (nullptr == un)? nullptr : un->GetPhenomenon();
    if (this == ph)
        return un;

    return nullptr;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
void Phenomenon::AddSynonym(Utf8CP unitName, Utf8CP synonym)
    {
    UnitSynonymMap map = UnitSynonymMap(unitName, synonym);
    UnitCP un = FindSynonym(map.GetSynonym());

    if (nullptr != un) // synonym is found - we don't add duplicate
        return;
    m_altNames.push_back(map);
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 08/17
//----------------------------------------------------------------------------------------
void Phenomenon::AddSynonymMap(UnitSynonymMapCR map) const
    {
    UnitCP un = FindSynonym(map.GetSynonym());
    if (nullptr == un)
        m_altNames.push_back(map);
    }
//----------------------------------------------------------------------------------------
// @bsimethod                                                   David Fox-Rabinovitz 09/17
//----------------------------------------------------------------------------------------
void Phenomenon::AddSynonymMaps(Json::Value jval) const // this value could be an array
    {
    UnitSynonymMap map;
    Json::Value val;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        val = *iter;
        for (Json::Value::iterator iter = val.begin(); iter != val.end(); iter++)
            {
            map = UnitSynonymMap(val);
            AddSynonymMap(map);
            }
        }
    }

Json::Value Phenomenon::SynonymMapToJson() const
    {
    Json::Value jval;

    for (size_t i = 0; i < m_altNames.size(); i++)
        {
        jval.append(m_altNames[i].ToJson());
        }
    return jval;
    }

Json::Value Phenomenon::SynonymMapVectorToJson(bvector<UnitSynonymMap> mapV)
    {
    Json::Value jval;

    for (size_t i = 0; i < mapV.size(); i++)
        {
        jval.append(mapV[i].ToJson());
        }
    return jval;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bill.Steinbock                  11/2017
//---------------------------------------------------------------------------------------
Utf8CP Phenomenon::GetLabel() const
    {
    if (m_displayLabel.empty())
        {
        Utf8PrintfString stringId("PHENOMENON_%s", GetName());
        m_displayLabel = BeSQLite::L10N::GetString(UnitsL10N::GetNameSpace(), BeSQLite::L10N::StringId(stringId.c_str()));
        }

    return m_displayLabel.c_str();
    }


