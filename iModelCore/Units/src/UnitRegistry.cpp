/*--------------------------------------------------------------------------------------+
|
|     $Source: src/UnitRegistry.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"

#include <Bentley/RefCounted.h>

using namespace std;

BEGIN_BENTLEY_UNITS_NAMESPACE

UnitRegistry * UnitRegistry::s_instance = nullptr;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
// static
UnitRegistry& UnitRegistry::Instance()
    {
    if (nullptr == s_instance)
        s_instance = new UnitRegistry();
    return *s_instance;
    }

//-------------------------------------------------------------------------------------//
// @bsimethod                                              Chris.Tartamella     02/16
//---------------+---------------+---------------+---------------+---------------+------//
// static
void UnitRegistry::Clear()
    {
    s_instance = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Caleb.Shafer            01/18
//---------------------------------------------------------------------------------------
UnitRegistry::UnitRegistry()
    {
    // This is going to change... There shouldn't be as much to initialize.
    AddDefaultSystems();
    AddDefaultPhenomena();
    AddDefaultUnits();
    AddDefaultConstants();
    AddDefaultMappings();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Caleb.Shafer            01/18
//---------------------------------------------------------------------------------------
// static
UnitRegistryPtr UnitRegistry::Create()
    {
    return new UnitRegistry();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Caleb.Shafer            01/18
//---------------------------------------------------------------------------------------
void UnitRegistry::RemoveUnitLocater(IUnitLocaterR locater)
    {
    for (auto iter = m_locaters.begin(); iter != m_locaters.end();)
        {
        if (*iter == &locater)
            iter = m_locaters.erase(iter);
        else
            ++iter;
        }
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
void UnitRegistry::AddSystem (Utf8CP name)
    {
    // TODO: use error return enum
    if (Utf8String::IsNullOrEmpty(name))
        {
        LOG.error("Cannot create UnitSystem because name is null");
        return;
        }
    if (NameConflicts(name))
        {
        LOG.errorv("Cannot create UnitSystem '%s' because that name is already in use.", name);
        return;
        }

    auto unitSystem = UnitSystem::Create(name);
    m_systems.Insert(unitSystem->GetName(), unitSystem);
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
void UnitRegistry::AddBasePhenomenon(Utf8Char baseSymbol)
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
void UnitRegistry::AddPhenomenon (Utf8CP phenomenaName, Utf8CP definition)
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
    AddSystem (METRIC);
    AddSystem (IMPERIAL);
    AddSystem (MARITIME);
    AddSystem (USSURVEY);
    AddSystem (INDUSTRIAL);
    AddSystem (INTERNATIONAL);
    AddSystem (USCUSTOM);
    AddSystem (STATISTICS);
    AddSystem (FINANCE);
    AddSystem(CONSTANT);
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

    UnitSystemCP system = LookupUnitSystem(systemName);
    if (nullptr == system)
        {
        LOG.errorv("Could not find system '%s'", systemName);
        return nullptr;
        }

    auto unit = Unit::Create(*system, *phenomenon, unitName, m_nextId, definition, baseSymbol, factor, offset, isConstant);
    if (nullptr == unit)
        return nullptr;

    // Add the unit label as a synonym as long as it is not the same as the actual unit name
    if (!Utf8String::IsNullOrEmpty(unit->GetLabel()) && (0 != strcmp(unit->GetLabel(), unitName)))
        unit->AddSynonym(unit->GetLabel());

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCP UnitRegistry::AddDummyUnit(Utf8CP unitName)
    {
    auto dummy = LookupUnit(unitName);
    if (nullptr == dummy)
        dummy = AddUnit("ONE", "USCUSTOM", unitName, "ONE");

    BeAssert(nullptr != dummy);
    return dummy;
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

//--------------------------------------------------------------------------------------
// @bsimethod                                              Chris.Tartamella       02/16
//--------------------------------------------------------------------------------------
UnitP UnitRegistry::LookupUnitP(Utf8CP name) const
    {
    auto val_iter = m_units.find(name);
    if (val_iter != m_units.end())
        return (*val_iter).second;

    UnitP foundUnit;
    for (auto locater : m_locaters)
        {
        foundUnit = locater->LocateUnitP(name);
        if (nullptr != foundUnit)
            return foundUnit;
        }

    return nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Chris.Tartamella       02/16
//--------------------------------------------------------------------------------------
UnitCP UnitRegistry::LookupUnit(Utf8CP name) const
    {
    return LookupUnitP(name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              David.Fox-Rabinovitz    02/17
//---------------------------------------------------------------------------------------
PUSH_MSVC_IGNORE(6385 6386)
UnitCP UnitRegistry::LookupUnitCI (Utf8CP name) const
    {
    size_t len = (nullptr == name) ? 0 : strlen(name);
    if (len == 0)
        return nullptr;
    Utf8P temp = (Utf8P)alloca(len + 2);
    memset(temp, 0, len + 2);
    memcpy(temp, name, len);
    Utf8CP uppName = BeStringUtilities::Strupr(temp);
    return LookupUnitP(uppName);
    }
POP_MSVC_IGNORE

//--------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                      01/2018
//--------------------------------------------------------------------------------------
UnitSystemCP UnitRegistry::LookupUnitSystem(Utf8CP name) const
    {
    auto usIt = m_systems.find(name);
    if (usIt == m_systems.end())
        return nullptr;

    return usIt->second;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Chris.Tartamella       02/16
//--------------------------------------------------------------------------------------
UnitCP UnitRegistry::LookupConstant(Utf8CP name) const
    {
    auto constant = LookupUnit(name);
    if (nullptr == constant)
        return constant;
    return constant->IsConstant() ? constant : nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                      02/2016
//--------------------------------------------------------------------------------------
PhenomenonP UnitRegistry::LookupPhenomenonP(Utf8CP name) const
    {
    auto val_iter = m_phenomena.find(name);
    if (val_iter != m_phenomena.end())
        return (*val_iter).second;

    PhenomenonP foundPhen;
    for (auto locater : m_locaters)
        {
        foundPhen = locater->LocatePhenomenonP(name);
        if (nullptr != foundPhen)
            return foundPhen;
        }

    return nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                      02/2016
//--------------------------------------------------------------------------------------
PhenomenonCP UnitRegistry::LookupPhenomenon(Utf8CP name) const
    {
    return LookupPhenomenonP(name);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                      02/2016
//--------------------------------------------------------------------------------------
void UnitRegistry::AllUnitNames(bvector<Utf8String>& allUnitNames, bool includeSynonyms) const
    {
    for (auto const& unitAndName : m_units)
        {
        if (includeSynonyms || unitAndName.first.Equals(unitAndName.second->GetName()))
            allUnitNames.push_back(unitAndName.first);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                      02/2016
//--------------------------------------------------------------------------------------
void UnitRegistry::AllUnits(bvector<UnitCP>& allUnits) const
    {
    for (auto const& unitAndName : m_units)
        {
        if (unitAndName.first.Equals(unitAndName.second->GetName()))
            allUnits.push_back(unitAndName.second);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                      02/2016
//--------------------------------------------------------------------------------------
void UnitRegistry::AllPhenomena(bvector<PhenomenonCP>& allPhenomena) const
    {
    for (auto const& phenomenonAndName : m_phenomena)
        allPhenomena.push_back(phenomenonAndName.second);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                              Robert.Schili     01/18
//--------------------------------------------------------------------------------------
void UnitRegistry::AddECMapping(Utf8CP name, Utf8CP ecName)
    {
    // NOTE: New mappings overwrite previously added mappings
    m_nameECNameMapping[name] = ecName;
    m_ecNameNameMapping[ecName] = name;
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

//--------------------------------------------------------------------------------------
// @bsimethod                                              Robert.Schili     01/18
//--------------------------------------------------------------------------------------
bool UnitRegistry::TryGetECName(Utf8CP name, Utf8StringR ecName) const
    {
    auto p = m_nameECNameMapping.find(name);
    if (p != m_oldNameNewNameMapping.end())
        {
        ecName = p->second;
        return true;
        }

    return false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Robert.Schili     01/18
//--------------------------------------------------------------------------------------
bool UnitRegistry::TryGetNameFromECName(Utf8CP ecName, Utf8StringR name) const
    {
    auto p = m_ecNameNameMapping.find(ecName);
    if (p != m_ecNameNameMapping.end())
        {
        name = p->second;
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

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              David Fox-Rabinovitz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  UnitRegistry::LoadSynonyms(Json::Value jval) const
    {
    size_t num = 0;
    UnitSynonymMap map;
    //UnitSynonymMapCR mapP = map;
    PhenomenonCP phP;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        JsonValueCR val = *iter;
        map = UnitSynonymMap(val);
        if (!map.IsMapEmpty())
            {
            phP = map.GetPhenomenon();
            if (phP != nullptr)
                {
                phP->AddSynonymMap(map);
                ++num;
                }
            }
        }
    return num;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              David.Fox-Rabinovitz    09/17
//---------------------------------------------------------------------------------------
PhenomenonCP UnitRegistry::LoadSynonym(Utf8CP unitName, Utf8CP synonym) const
    {
    if (Utf8String::IsNullOrEmpty(unitName) || Utf8String::IsNullOrEmpty(synonym))
        return nullptr;
    UnitCP unit = UnitRegistry::Instance().LookupUnitCI(unitName);
    PhenomenonCP ph = (nullptr == unit)? nullptr : unit->GetPhenomenon();
    if (nullptr != ph)
        {
        UnitCP un = ph->FindSynonym(synonym);
        if (un == nullptr)  // a new synonym
            {

            }
        }

    return ph;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              David.Fox-Rabinovitz    09/17
//---------------------------------------------------------------------------------------
Json::Value UnitRegistry::SynonymsToJson() const
    {
    Json::Value jMap;
    for (auto iter = m_phenomena.begin(); iter != m_phenomena.end(); iter++)
        {
        PhenomenonP pp = iter->second;
        if(!pp->m_altNames.empty())
          jMap.append(pp->SynonymMapToJson());
        }
    return jMap;
    }

END_BENTLEY_UNITS_NAMESPACE
