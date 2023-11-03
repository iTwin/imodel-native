/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include <Units/UnitRegistry.h>

using namespace std;

BEGIN_BENTLEY_UNITS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
UnitRegistry::UnitRegistry()
    {
    }

//---------------------------------------------------------------------------------------//
// @bsimethod
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
    auto dummy = AddUnitInternal(dummyPhenName.c_str(), DUMMY, unitName, "ONE", 1.0, 1.0, 0.0, false);
    dummy->m_dummyUnit = true;
    return dummy;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitCP UnitRegistry::LookupConstant(Utf8CP name) const
    {
    auto constant = LookupUnit(name);
    if (nullptr == constant)
        return constant;
    return constant->IsConstant() ? constant : nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//--------------------------------------------------------------------------------------
void UnitRegistry::_AllUnits(bvector<UnitCP>& allUnits) const
    {
    for (auto const& unitAndName : m_units)
        allUnits.push_back(unitAndName.second);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
void UnitRegistry::_AllPhenomena(bvector<PhenomenonCP>& allPhenomena) const
    {
    for (auto const& phenomenonAndName : m_phenomena)
        allPhenomena.push_back(phenomenonAndName.second);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//--------------------------------------------------------------------------------------
bool UnitRegistry::RemoveSystem(Utf8CP name)
    {
    auto iter = m_systems.find(name);
    if (iter != m_systems.end())
        {
        delete (iter->second);
        m_systems.erase(iter);
        return true;
        }

    return false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
bool UnitRegistry::RemovePhenomenon(Utf8CP name)
    {
    auto iter = m_phenomena.find(name);
    if (iter != m_phenomena.end())
        {
        delete (iter->second);
        m_phenomena.erase(iter);
        return true;
        }

    return false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
bool UnitRegistry::RemoveUnit(Utf8CP name)
    {
    auto iter = m_units.find(name);
    if (iter != m_units.end())
        {
        delete (iter->second);
        m_units.erase(iter);
        return true;
        }

    return false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
bool UnitRegistry::RemoveInvertedUnit(Utf8CP name)
    {
    auto iter = m_units.find(name);
    if (iter != m_units.end() && iter->second->IsInvertedUnit())
        {
        delete (iter->second);
        m_units.erase(iter);
        return true;
        }

    return false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
bool UnitRegistry::RemoveConstant(Utf8CP name)
    {
    auto iter = m_units.find(name);
    if (iter != m_units.end() && iter->second->IsConstant())
        {
        delete (iter->second);
        m_units.erase(iter);
        return true;
        }

    return false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
void UnitRegistry::_AllSystems(bvector<UnitSystemCP>& allUnitSystems) const
    {
    for (auto const& unitSystemAndName : m_systems)
        allUnitSystems.push_back(unitSystemAndName.second);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCP UnitRegistry::LookupUnitUsingOldName(Utf8CP oldName) const
    {
    auto newName = UnitNameMappings::TryGetNewNameFromOldName(oldName);
    if (nullptr == newName)
        return nullptr;
    return LookupUnit(newName);
    }

END_BENTLEY_UNITS_NAMESPACE
