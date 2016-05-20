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
        s_instance->AddDefaultMappings();
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

Utf8CP GetBasePhenomenonName(Utf8Char baseSymbol)
    {
    switch (baseSymbol)
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
                return LUMINOSITY;
            case BasePhenomena::Mass:
                return MASS;
            case BasePhenomena::Mole:
                return MOLE;
            case BasePhenomena::PlaneAngle:
                return ANGLE;
            case BasePhenomena::Ratio:
                return ONE;
            case BasePhenomena::SolidAngle:
                return SOLIDANGLE;
            case BasePhenomena::Temperature:
                return TEMPERATURE;
            case BasePhenomena::TemperatureChange:
                return TEMPERATURE_CHANGE;
            case BasePhenomena::Time:
                return TIME;
            default:
                return "";
        }
    }
//-------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr     02/16
//+---------------+---------------+---------------+---------------+---------------+----//
void UnitRegistry::AddBasePhenomena(Utf8Char baseSymbol)
    {
    Utf8CP phenomenaName = GetBasePhenomenonName(baseSymbol);
    if (Utf8String::IsNullOrEmpty(phenomenaName))
        return;

    if (HasPhenomena(phenomenaName))
        {
        LOG.errorv("Cannot create Base Phenomenon '%s' because name is already in use", phenomenaName);
        return;
        }

    auto phenomena = new Phenomenon(phenomenaName, phenomenaName, baseSymbol, m_nextId);
    ++m_nextId;

    m_phenomena.insert(bpair<Utf8String, PhenomenonP>(phenomenaName, phenomena));
    }

//-------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr     02/16
//+---------------+---------------+---------------+---------------+---------------+----//
void UnitRegistry::AddPhenomena (Utf8CP phenomenaName, Utf8CP definition)
    {
    if (Utf8String::IsNullOrEmpty(phenomenaName))
        {
        LOG.error("Failed to create Phenomenon because name is null");
        return;
        }

    if (HasPhenomena(phenomenaName))
        {
        LOG.errorv("Cannot create Phenomenon '%s' because name is already in use", phenomenaName);
        return;
        }

    auto phenomena = new Phenomenon(phenomenaName, definition, ' ', m_nextId);
    ++m_nextId;

    m_phenomena.insert(bpair<Utf8String, PhenomenonP>(phenomenaName, phenomena));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultSystems ()
    {
    AddSystem (SI);
    AddSystem (CGS);
    //AddSystem (METRIC);
    AddSystem (IMPERIAL);
    //AddSystem (PHYSICS);
    AddSystem (CHEMISTRY);
    //AddSystem (THERMODYNAMICS);
    //AddSystem (ASTRONOMY);
    AddSystem (MARITIME);
    AddSystem (USSURVEY);
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
    //AddSystem (HISTORICAL);
    AddSystem (STATISTICS);
    //AddSystem (BENTLEY);
    AddSystem (CUSTOMARY);
    AddSystem (FINANCE);
    AddSystem (CONSTANT);
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr           02/16
//+---------------+---------------+---------------+---------------+---------------+------//
UnitP UnitRegistry::AddUnitInternal(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, Utf8Char baseSymbol, double factor, double offset, bool isConstant)
    {
    if (Utf8String::IsNullOrEmpty(unitName))
        {
        LOG.error("Cannot create base unit because the input name is null");
        return nullptr;
        }

    // TODO: Add back in check for system name

    if (NameConflicts(unitName))
        {
        LOG.errorv("Could not create unit '%s' because that name is already in use", unitName);
        return nullptr;
        }

    PhenomenonP phenomenon = LookupPhenomenonP(phenomName);
    if (nullptr == phenomenon)
        {
        LOG.errorv("Could not find phenomenon '%s'", phenomName);
        return nullptr;
        }

    auto unit = Unit::Create(systemName, *phenomenon, unitName, m_nextId, definition, baseSymbol, factor, offset, isConstant);
    if (nullptr == unit)
        return nullptr;

    phenomenon->AddUnit(*unit);

    ++m_nextId;

    m_units.insert(bpair<Utf8String, UnitP>(unitName, unit));

    return unit;
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr           02/16
//+---------------+---------------+---------------+---------------+---------------+------//
UnitCP UnitRegistry::AddUnitForBasePhenomenon(Utf8CP unitName, Utf8Char baseSymbol)
    {
    if (Utf8String::IsNullOrEmpty(unitName))
        {
        LOG.errorv("Cannot create base unit because the input name is null");
        return nullptr;
        }

    Utf8CP phenomenonName = GetBasePhenomenonName(baseSymbol);
    if (Utf8String::IsNullOrEmpty(phenomenonName))
        {
        LOG.errorv("Cannot create base unit '%s' because the input base symbol '%c' does not map to a base phenomenon", unitName, baseSymbol);
        return nullptr;
        }
    
    return AddUnitInternal(phenomenonName, SI, unitName, unitName, baseSymbol, 1, 0, false);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitP UnitRegistry::AddUnitP (Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double factor, double offset)
    {
    if (Utf8String::IsNullOrEmpty(unitName))
        {
        LOG.errorv("Could not create unit because unit name is null");
        return nullptr;
        }
    return AddUnitInternal(phenomName, systemName, unitName, definition, ' ', factor, offset, false);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCP UnitRegistry::AddUnit (Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double factor, double offset)
    {
    return AddUnitP(phenomName, systemName, unitName, definition, factor, offset);
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr           05/16
//+---------------+---------------+---------------+---------------+---------------+------//
UnitCP UnitRegistry::AddDummyUnit(Utf8CP unitName)
    {
    return AddUnit("ONE", "USCUSTOM", unitName, "ONE");
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr           03/16
//+---------------+---------------+---------------+---------------+---------------+------//
UnitCP UnitRegistry::AddInvertingUnit(Utf8CP parentUnitName, Utf8CP unitName)
    {
    if (Utf8String::IsNullOrEmpty(unitName))
        {
        LOG.error("Cannot create unit because the input name is null");
        return nullptr;
        }

    if (Utf8String::IsNullOrEmpty(parentUnitName))
        {
        LOG.errorv("Cannot create unit %s because it's parent name is null", unitName);
        return nullptr;
        }

    UnitCP parentUnit = LookupUnit(parentUnitName);
    if (nullptr == parentUnit)
        {
        LOG.errorv("Cannot create unit %s because it's parent unit %s cannot be found", unitName, parentUnitName);
        return nullptr;
        }
    
    if (NameConflicts(unitName))
        {
        LOG.errorv("Could not create unit '%s' because that name is already in use", unitName);
        return nullptr;
        }
    
    auto unit = Unit::Create(*parentUnit, unitName, m_nextId);
    if (nullptr == unit)
        return nullptr;

    PhenomenonP phenomenon = LookupPhenomenonP(parentUnit->GetPhenomenon()->GetName());
    phenomenon->AddUnit(*unit);

    ++m_nextId;

    m_units.insert(bpair<Utf8String, UnitP>(unitName, unit));

    return unit;
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr           02/16
//+---------------+---------------+---------------+---------------+---------------+------//
bool UnitRegistry::NameConflicts(Utf8CP name)
    {
    if (HasConstant(name))
        {
        LOG.errorv("Name '%s' conflicts with a constant that is already registered", name);
        return true;
        }

    if (HasUnit(name))
        {
        LOG.errorv("Name '%s' conflicts with a unit that is already registered", name);
        return true;
        }
    return false;
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr           03/16
//+---------------+---------------+---------------+---------------+---------------+------//
bool UnitRegistry::TryGetConversion(uint64_t index, Conversion& conversion)
    {
    auto it = m_conversions.find(index);
    if (it != m_conversions.end())
        {
        conversion = it->second;
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCP UnitRegistry::AddConstant(Utf8CP phenomName, Utf8CP constantName, Utf8CP definition, double factor)
    {
    if (Utf8String::IsNullOrEmpty(constantName))
        {
        LOG.error("Could not create constant because name is null or empty");
        return nullptr;
        }

    return AddUnitInternal(phenomName, CONSTANT, constantName, definition, ' ', factor, 0, true);
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr           02/16
//+---------------+---------------+---------------+---------------+---------------+------//
BentleyStatus UnitRegistry::AddSynonym(Utf8CP unitName, Utf8CP synonymName)
    {
    if (Utf8String::IsNullOrEmpty(synonymName))
        return BentleyStatus::ERROR;
    if (Utf8String::IsNullOrEmpty(unitName))
        return BentleyStatus::ERROR;

    if (NameConflicts(synonymName))
        {
        LOG.errorv("Could not create synonym with name '%s' becasue it conflicts with an existing name", synonymName);
        return BentleyStatus::ERROR;
        }

    UnitP unit = LookupUnitP(unitName);

    if (nullptr == unit)
        {
        LOG.errorv("Could not create synonym with name '%s' because the unit it is for is null", synonymName);
        return BentleyStatus::ERROR;
        }

    m_units.insert(bpair<Utf8String, UnitP>(synonymName, unit));

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr           02/16
//+---------------+---------------+---------------+---------------+---------------+------//
BentleyStatus UnitRegistry::AddSynonym(UnitCP unit, Utf8CP synonymName)
    {
    if (Utf8String::IsNullOrEmpty(synonymName))
        return BentleyStatus::ERROR;

    if (nullptr == unit)
        {
        LOG.errorv("Could not create synonym with name '%s' because the unit it is for is null", synonymName);
        return BentleyStatus::ERROR;
        }

    return AddSynonym(unit->GetName(), synonymName);
    }

UnitP UnitRegistry::LookupUnitP(Utf8CP name) const
    {
    auto nameStr = Utf8String(name);
    auto val_iter = m_units.find(nameStr);
    if (val_iter == m_units.end())
        return nullptr;

    return (*val_iter).second;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCP UnitRegistry::LookupUnit (Utf8CP name) const
    {
    return LookupUnitP(name);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCP UnitRegistry::LookupConstant (Utf8CP name) const
    {
    auto val_iter = m_units.find(name);
    if (val_iter == m_units.end())
        return nullptr;

    return (*val_iter).second;
    }

PhenomenonP UnitRegistry::LookupPhenomenonP(Utf8CP name) const
    {
    auto val_iter = m_phenomena.find(name);
    if (val_iter == m_phenomena.end())
        return nullptr;

    return (*val_iter).second;
    }

void UnitRegistry::AllUnitNames(bvector<Utf8String>& allUnitNames, bool includeSynonyms) const
    {
    for (auto const& unitAndName : m_units)
        {
        if (includeSynonyms || unitAndName.first.Equals(unitAndName.second->GetName()))
            allUnitNames.push_back(unitAndName.first);
        }
    }

void UnitRegistry::AllUnits(bvector<UnitCP>& allUnits) const
    {
    for (auto const& unitAndName : m_units)
        {
        if (unitAndName.first.Equals(unitAndName.second->GetName()))
            allUnits.push_back(unitAndName.second);
        }
    }

void UnitRegistry::AllPhenomena(bvector<PhenomenonCP>& allPhenomena) const
    {
    for (auto const& phenomenonAndName : m_phenomena)
        allPhenomena.push_back(phenomenonAndName.second);
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
* @bsimethod                                              Robert.Schili     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddMapping(Utf8CP oldName, Utf8CP newName)
    {
    // NOTE: New mappings overwrite previously added mappings
    m_oldNameNewNameMapping[oldName] = newName;
    m_newNameOldNameMapping[newName] = oldName;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Robert.Schili     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitRegistry::TryGetNewName(Utf8CP oldName, Utf8StringR newName) const
    {
    auto p = m_oldNameNewNameMapping.find(oldName);
    if (p != m_oldNameNewNameMapping.end())
        {
        newName = p->second;
        return true;
        }

    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Robert.Schili     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitRegistry::TryGetOldName(Utf8CP newName, Utf8StringR oldName) const
    {
    auto p = m_newNameOldNameMapping.find(newName);
    if (p != m_newNameOldNameMapping.end())
        {
        oldName = p->second;
        return true;
        }

    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Robert.Schili     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCP UnitRegistry::LookupUnitUsingOldName(Utf8CP oldName) const
    {
    Utf8String newName;
    if (TryGetNewName(oldName, newName))
        {
        return LookupUnitP(newName.c_str());
        }
    return nullptr;
    }

