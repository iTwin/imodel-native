/*--------------------------------------------------------------------------------------+
|
|     $Source: src/UnitRegistry.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"

using namespace std;

BEGIN_BENTLEY_UNITS_NAMESPACE

UnitRegistry * UnitRegistry::s_instance = nullptr;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
// static
UnitRegistry& UnitRegistry::Get()
    {
    if (nullptr == s_instance)
        {
        s_instance = new UnitRegistry();
        s_instance->AddDefaultPhenomena();
        s_instance->AddDefaultUnits();
        s_instance->AddDefaultConstants();
        }

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
    AddBaseSystems();
    AddBasePhenomena();
    AddBaseUnits();
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

//---------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr           11/17
//+---------------+---------------+---------------+---------------+---------------+------//
UnitCP UnitRegistry::CreateDummyUnit(Utf8CP unitName)
    {
    if (Utf8String::IsNullOrEmpty(unitName))
        return nullptr;

    if (NamedItemExists(unitName))
        {
        LOG.errorv("Could not create dummy unit '%s' because that name is already in use", unitName);
        return _LookupUnitP(unitName);
        }

    LOG.warningv("Creating Dummy unit with name '%s'", unitName);
    Utf8PrintfString dummyPhenName("%s_%s", "DUMMY", unitName);
    AddPhenomenon(dummyPhenName.c_str(), "ONE");
    auto dummy = AddUnit<Unit>(dummyPhenName.c_str(), DUMMY, unitName, "ONE", 1.0, 0.0);
    dummy->m_dummyUnit = true;
    return dummy;
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
void UnitRegistry::AllUnitNames(bvector<Utf8String>& allUnitNames, bool includeSynonyms) const
    {
    for (auto const& unitAndName : m_units)
        {
        if (includeSynonyms || unitAndName.second->GetName().Equals(unitAndName.first))
            allUnitNames.push_back(unitAndName.first);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                      02/2016
//--------------------------------------------------------------------------------------
void UnitRegistry::_AllUnits(bvector<UnitCP>& allUnits) const
    {
    for (auto const& unitAndName : m_units)
        allUnits.push_back(unitAndName.second);
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr           02/16
//+---------------+---------------+---------------+---------------+---------------+------//
UnitCP UnitRegistry::AddUnitForBasePhenomenon(Utf8CP unitName, Utf8CP basePhenomenonName)
    {
    if (Utf8String::IsNullOrEmpty(unitName))
        {
        LOG.errorv("Cannot create base unit because the input name is null");
        return nullptr;
        }

    if (Utf8String::IsNullOrEmpty(basePhenomenonName))
        {
        LOG.errorv("Cannot create base unit '%s' because the input base phenomenon name is null", unitName);
        return nullptr;
        }

    PhenomenonCP basePhen = LookupPhenomenon(basePhenomenonName);

    if (nullptr == basePhen || !basePhen->IsBase())
        {
        LOG.errorv("Cannot create base unit '%s' because the input base phenomenon name '%s' does not map to a base phenomenon", unitName, basePhenomenonName);
        return nullptr;
        }

    return AddUnitInternal<Unit>(basePhenomenonName, SI, unitName, unitName, 1, 1, 0, false);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                      02/2016
//--------------------------------------------------------------------------------------
void UnitRegistry::_AllPhenomena(bvector<PhenomenonCP>& allPhenomena) const
    {
    for (auto const& phenomenonAndName : m_phenomena)
        allPhenomena.push_back(phenomenonAndName.second);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
void UnitRegistry::AddSystem(UnitSystemR unitSystem)
    {
    if (HasSystem(unitSystem.GetName().c_str()))
        {
        LOG.errorv("Cannot create UnitSystem '%s' because that name is already in use.", unitSystem.GetName().c_str());
        return;
        }

    unitSystem.m_unitsContext = this;

    m_systems.Insert(unitSystem.GetName().c_str(), &unitSystem);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
UnitSystemP UnitRegistry::RemoveSystem(Utf8CP name)
    {
    UnitSystemP ptrReturn = nullptr;

    auto iter = m_systems.find(name);
    if (iter != m_systems.end())
        {
        ptrReturn = iter->second;
        m_systems.erase(iter);
        }

    return ptrReturn;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
PhenomenonP UnitRegistry::RemovePhenomenon(Utf8CP name)
    {
    PhenomenonP ptrReturn = nullptr;

    auto iter = m_phenomena.find(name);
    if (iter != m_phenomena.end())
        {
        ptrReturn = iter->second;
        m_phenomena.erase(iter);
        }

    return ptrReturn;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
UnitP UnitRegistry::RemoveUnit(Utf8CP name)
    {
    UnitP ptrReturn = nullptr;

    auto iter = m_units.find(name);
    if (iter != m_units.end())
        {
        ptrReturn = iter->second;
        m_units.erase(iter);
        }

    return ptrReturn;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
UnitP UnitRegistry::RemoveInvertedUnit(Utf8CP name)
    {
    UnitP ptrReturn = nullptr;

    auto iter = m_units.find(name);
    if (iter != m_units.end() && iter->second->IsInvertedUnit())
        {
        ptrReturn = iter->second;
        m_units.erase(iter);
        }

    return ptrReturn;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
UnitP UnitRegistry::RemoveConstant(Utf8CP name)
    {
    UnitP ptrReturn = nullptr;

    auto iter = m_units.find(name);
    if (iter != m_units.end() && iter->second->IsConstant())
        {
        ptrReturn = iter->second;
        m_units.erase(iter);
        }

    return ptrReturn;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                      02/2016
//--------------------------------------------------------------------------------------
void UnitRegistry::_AllSystems(bvector<UnitSystemCP>& allUnitSystems) const
    {
    for (auto const& unitSystemAndName : m_systems)
        allUnitSystems.push_back(unitSystemAndName.second);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Robert.Schili     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8CP UnitRegistry::TryGetNewName(Utf8CP oldName)
    {
    return UnitNameMappings::TryGetNewNameFromOldName(oldName);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Robert.Schili     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8CP UnitRegistry::TryGetOldName(Utf8CP newName)
    {
    return UnitNameMappings::TryGetOldNameFromNewName(newName);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Robert.Schili     01/18
//--------------------------------------------------------------------------------------
// static
Utf8CP UnitRegistry::TryGetECName(Utf8CP name)
    {
    return UnitNameMappings::TryGetECNameFromNewName(name);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                              Robert.Schili     01/18
//--------------------------------------------------------------------------------------
// static
Utf8CP UnitRegistry::TryGetNameFromECName(Utf8CP ecName)
    {
    return UnitNameMappings::TryGetNewNameFromECName(ecName);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                        Kyle.Abramowitz        03/2018
//--------------------------------------------------------------------------------------
// static
Utf8CP UnitRegistry::TryGetOldNameFromECName(Utf8CP ecName)
    {
    return UnitNameMappings::TryGetOldNameFromECName(ecName);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Robert.Schili     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCP UnitRegistry::LookupUnitUsingOldName(Utf8CP oldName) const
    {
    auto newName = UnitRegistry::TryGetNewName(oldName);
    if (nullptr == newName)
        return nullptr;
    return LookupUnit(newName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              David.Fox-Rabinovitz    09/17
//---------------------------------------------------------------------------------------
size_t UnitRegistry::LoadSynonyms(Json::Value jval) const
    {
    size_t num = 0;
    UnitSynonymMap map;
    PhenomenonCP phP;
    for (Json::Value::iterator iter = jval.begin(); iter != jval.end(); iter++)
        {
        JsonValueCR val = *iter;
        map = UnitSynonymMap(this, val);
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
    UnitCP unit = LookupUnit(unitName);
    return (nullptr == unit) ? nullptr : unit->GetPhenomenon();
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
