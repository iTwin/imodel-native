/*--------------------------------------------------------------------------------------+
|
|     $Source: src/UnitRegistry.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include "StandardNames.h"

USING_NAMESPACE_BENTLEY_UNITS

UnitRegistry * UnitRegistry::s_instance = nullptr;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitRegistry& UnitRegistry::Instance()
    {
    if (nullptr == s_instance)
        {
        s_instance = new UnitRegistry();
        s_instance->AddDefaultSystems();
        s_instance->AddDefaultPhenomena();
        s_instance->AddDefaultUnits();
        s_instance->AddDefaultConstants();
        }

    return *s_instance;
    }

//-------------------------------------------------------------------------------------//
// @bsimethod                                              Chris.Tartamella     02/16
//---------------+---------------+---------------+---------------+---------------+------//
void UnitRegistry::Clear()
    {
    s_instance = nullptr;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitRegistry::UnitRegistry()
    {
    
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::InsertUnique (Utf8Vector &vec, Utf8String &str)
    {
    // Don't insert duplicates.
    auto iter = find (vec.begin(), vec.end(), str);
    if (iter != vec.end())
        return;

    vec.push_back(str);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddSystem (Utf8CP systemName)
    {
    auto str = Utf8String(systemName);
    InsertUnique (m_systems, str);
    }

Utf8CP GetBasePhenomenonName(Utf8Char dimensionSymbol)
    {
    switch (dimensionSymbol)
        {
            case BasePhenomena::Capita:
                return CAPITA;
            case BasePhenomena::ElectricCurrent:
                return CURRENT;
            case BasePhenomena::Finance:
                return FINANCE;
            case BasePhenomena::Length:
                return LENGTH;
            case BasePhenomena::Luminosity:
                return LUMINOUSINTENSITY;
            case BasePhenomena::Mass:
                return MASS;
            case BasePhenomena::Mole:
                return MOLE;
            case BasePhenomena::PlaneAngle:
                return ANGLE;
            case BasePhenomena::Ratio:
                return RATIO;
            case BasePhenomena::SolidAngle:
                return SOLIDANGLE;
            case BasePhenomena::Temperature:
                return TEMPERATURE;
            case BasePhenomena::Time:
                return TIME;
            default:
                return "";
        }
    }
//-------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr     02/16
//+---------------+---------------+---------------+---------------+---------------+----//
void UnitRegistry::AddBasePhenomena(Utf8Char dimensionalSymbol)
    {
    Utf8CP phenomenaName = GetBasePhenomenonName(dimensionalSymbol);
    if (Utf8String::IsNullOrEmpty(phenomenaName))
        return;

    auto phenomena = new Phenomenon(phenomenaName, phenomenaName, dimensionalSymbol);

    //auto nameStr = Utf8String(phenomenaName);
    m_phenomena.insert(bpair<Utf8String, PhenomenonCP>(phenomenaName, phenomena));
    }

//-------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr     02/16
//+---------------+---------------+---------------+---------------+---------------+----//
void UnitRegistry::AddPhenomena (Utf8CP phenomenaName, Utf8CP definition)
    {
    auto phenomena = new Phenomenon(phenomenaName, definition, ' ');

    //auto nameStr = Utf8String(phenomenaName);
    m_phenomena.insert(bpair<Utf8String, PhenomenonCP>(phenomenaName, phenomena));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultSystems ()
    {
    AddSystem (SI);
    AddSystem (CGS);
    AddSystem (METRIC);
    AddSystem (IMPERIAL);
    AddSystem (PHYSICS);
    AddSystem (CHEMISTRY);
    AddSystem (THERMODYNAMICS);
    AddSystem (ASTRONOMY);
    AddSystem (MARITIME);
    AddSystem (SURVEYOR);
    AddSystem (TYPOGRAPHY);
    AddSystem (POSTSCRIPT);
    AddSystem (TEXT);
    AddSystem (INDUSTRIAL);
    AddSystem (PHARMACEUTICAL);
    AddSystem (AGRICULTURE);
    AddSystem (INTERNATIONAL);
    AddSystem (USCUSTOM);
    AddSystem (BRITISH);
    AddSystem (JAPANESE);
    AddSystem (HISTORICAL);
    AddSystem (STATISTICS);
    AddSystem (BENTLEY);
    AddSystem (CUSTOMARY);
    AddSystem (FINANCE);
    AddSystem (CONSTANT);
    }

UnitCP UnitRegistry::AddSIBaseUnit(Utf8CP unitName, Utf8Char dimensionSymbol)
    {
    Utf8CP phenomenonName = GetBasePhenomenonName(dimensionSymbol);
    if (Utf8String::IsNullOrEmpty(phenomenonName))
        return nullptr;
    
    // TODO: Error checking
    auto unit = Unit::Create(SI, phenomenonName, unitName, unitName, dimensionSymbol, 1, 0);

    //auto nameStr = Utf8String(unitName);
    m_units.insert(bpair<Utf8String, UnitCP>(unitName, unit));

    return unit;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCP UnitRegistry::AddUnit (Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double factor, double offset)
    {
    if (!HasSystem(systemName) || !HasPhenomena(phenomName))
        return nullptr;

    auto unit = Unit::Create (systemName, phenomName, unitName, definition, ' ', factor, offset);
    if (nullptr == unit)
        return nullptr;

    //auto nameStr = Utf8String(unitName);
    m_units.insert (bpair<Utf8String, UnitCP>(unitName, unit));

    // TODO: Add conversions.  Do we really do this here or when we are asked to convert between units?

    return unit;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UnitRegistry::AddConstant(Utf8CP phenomName, Utf8CP constantName, Utf8CP definition, double factor)
    {
    // TODO: Error checking
    auto constant = Unit::Create(CONSTANT, phenomName, constantName, definition, ' ', factor, 0);

    //auto nameStr = Utf8String(constantName);
    m_constants.insert(bpair<Utf8String, UnitCP>(constantName, constant));

    return BentleyStatus::SUCCESS;
    }

BentleyStatus UnitRegistry::AddSynonym(UnitCP unit, Utf8CP synonymName)
    {
    if (Utf8String::IsNullOrEmpty(synonymName))
        return BentleyStatus::ERROR;

    if (nullptr == unit)
        return BentleyStatus::ERROR;

    // TODO: Check to make sure not constant
    auto iter = m_units.find(synonymName);
    if (iter != m_units.end())
        return BentleyStatus::ERROR;

    //auto nameStr = Utf8String();
    m_units.insert(bpair<Utf8String, UnitCP>(synonymName, unit));

    return BentleyStatus::SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCP UnitRegistry::LookupUnit (Utf8CP name) const
    {
    auto nameStr = Utf8String(name);
    auto val_iter = m_units.find (nameStr);
    if (val_iter == m_units.end())
        return nullptr;

    return (*val_iter).second;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCP UnitRegistry::LookupConstant (Utf8CP name) const
    {
    auto val_iter = m_constants.find(name);
    if (val_iter == m_constants.end())
        return nullptr;

    return (*val_iter).second;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitRegistry::HasSystem (Utf8CP systemName) const
    {
    auto iter = find (m_systems.begin(), m_systems.end(), Utf8String(systemName));
    return iter != m_systems.end();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitRegistry::HasPhenomena (Utf8CP phenomenaName) const
    {
    auto iter = m_phenomena.find(phenomenaName);
    return iter != m_phenomena.end();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitRegistry::HasUnit (Utf8CP unitName) const
    {
    auto nameStr = Utf8String(unitName);
    auto iter = m_units.find(nameStr);
    return iter != m_units.end();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitRegistry::HasConstant (Utf8CP constantName) const
    {
    auto constant = LookupConstant (constantName);
    return nullptr != constant;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCP UnitRegistry::LookupUnitBySubTypes (const Utf8Vector &numerator, const Utf8Vector &denominator) const
    {
    auto n = Utf8Vector(numerator), d = Utf8Vector(denominator);
    
    sort(n.begin(), n.end());
    sort(d.begin(), d.end());

    auto comparison = [&](bpair<Utf8String, UnitCP> pair)
        {
        auto unit = pair.second;

        if (n.size() != unit->Numerator().size())
            return false;

        if (d.size() != unit->Denominator().size())
            return false;

        // TODO: Is this necessary?  We sort when creating the unit
        //sort (unit->GetNumerator().begin(), unit->GetNumerator().end());
        //sort (unit->GetDenominator().begin(), unit->GetDenominator().end());

        auto nmatch = mismatch (n.begin(), n.end(), unit->Numerator().begin());
        if (nmatch.first != n.end())
            return false;

        auto dmatch = mismatch (d.begin(), d.end(), unit->Denominator().begin());
        if (dmatch.first != d.end())
            return false;

        return true;
        };

    auto iter = find_if (m_units.begin(), m_units.end(), comparison);
    if (iter != m_units.end())
        return nullptr;

    return ((*iter).second);
    }
