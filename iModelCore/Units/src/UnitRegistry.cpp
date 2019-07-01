/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include <Units/UnitRegistry.h>

using namespace std;

BEGIN_BENTLEY_UNITS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                              Caleb.Shafer            01/18
//---------------------------------------------------------------------------------------
UnitRegistry::UnitRegistry()
    {
    AddBaseSystems();
    AddBasePhenomena();
    AddBaseUnits();
    AddDefaultPhenomena();
    AddDefaultUnits();
    AddDefaultConstants();
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
    auto dummy = AddUnitInternal(dummyPhenName.c_str(), DUMMY, unitName, "ONE", 1.0, 1.0, 0.0, false);
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

    return AddUnitInternal(basePhenomenonName, SI, unitName, unitName, 1, 1, 0, false);
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
UnitCP UnitRegistry::LookupUnitUsingOldName(Utf8CP oldName) const
    {
    auto newName = UnitNameMappings::TryGetNewNameFromOldName(oldName);
    if (nullptr == newName)
        return nullptr;
    return LookupUnit(newName);
    }

END_BENTLEY_UNITS_NAMESPACE
