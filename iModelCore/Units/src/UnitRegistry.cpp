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

//-------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr     02/16
//+---------------+---------------+---------------+---------------+---------------+----//
void UnitRegistry::AddBasePhenomenon(Utf8Char baseSymbol)
    {
    Utf8CP phenomName = BasePhenomena::GetBasePhenomenonName(baseSymbol);
    if (Utf8String::IsNullOrEmpty(phenomName))
        return;

    if (HasPhenomenon(phenomName))
        {
        LOG.errorv("Cannot create Base Phenomenon '%s' because name is already in use", phenomName);
        return;
        }

    auto phenomena = new Phenomenon(phenomName, phenomName, baseSymbol, m_nextId);
    ++m_nextId;

    m_phenomena.insert(bpair<Utf8String, PhenomenonP>(phenomName, phenomena));
    }

//-------------------------------------------------------------------------------------//
// @bsimethod                                              Colin.Kerr     02/16
//+---------------+---------------+---------------+---------------+---------------+----//
PhenomenonCP UnitRegistry::AddPhenomenon (Utf8CP phenomenaName, Utf8CP definition)
    {
    if (Utf8String::IsNullOrEmpty(phenomenaName))
        {
        LOG.error("Failed to create Phenomenon because name is null");
        return nullptr;
        }

    if (HasPhenomenon(phenomenaName))
        {
        LOG.errorv("Cannot create Phenomenon '%s' because name is already in use", phenomenaName);
        return nullptr;
        }

    auto phenomena = new Phenomenon(phenomenaName, definition, ' ', m_nextId);
    ++m_nextId;

    m_phenomena.insert(bpair<Utf8String, PhenomenonP>(phenomenaName, phenomena));

    return phenomena;
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

    Utf8CP phenomenonName = BasePhenomena::GetBasePhenomenonName(baseSymbol);
    if (Utf8String::IsNullOrEmpty(phenomenonName))
        {
        LOG.errorv("Cannot create base unit '%s' because the input base symbol '%c' does not map to a base phenomenon", unitName, baseSymbol);
        return nullptr;
        }

    return AddUnitInternal<Unit>(phenomenonName, SI, unitName, unitName, baseSymbol, 1, 0, false);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                      02/2016
//--------------------------------------------------------------------------------------
void UnitRegistry::AllPhenomena(bvector<PhenomenonCP>& allPhenomena) const
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
// @bsimethod                                   Colin.Kerr                      02/2016
//--------------------------------------------------------------------------------------
void UnitRegistry::AllSystems(bvector<UnitSystemCP>& allUnitSystems) const
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

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Robert.Schili     03/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCP UnitRegistry::LookupUnitUsingOldName(Utf8CP oldName) const
    {
    auto newName = UnitRegistry::TryGetNewName(oldName);
    if (nullptr == newName)
        return nullptr;    
    return LookupUnitP(newName);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              David Fox-Rabinovitz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
size_t UnitRegistry::LoadSynonyms(Json::Value jval) const
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
