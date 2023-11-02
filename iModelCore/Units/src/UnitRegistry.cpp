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
PhenomenonP UnitRegistry::AddPhenomenonInternal(Utf8CP phenomenaName, Utf8CP definition)
        {
        if (Utf8String::IsNullOrEmpty(phenomenaName))
            {
            LOG.error("Failed to create Phenomenon because name is null");
            return nullptr;
            }

        if (Utf8String::IsNullOrEmpty(definition))
            {
            LOG.error("Failed to create Phenomenon because definition is null");
            return nullptr;
            }

        if (NamedItemExists(phenomenaName))
            {
            LOG.errorv("Cannot create Phenomenon '%s' because name is already in use", phenomenaName);
            return nullptr;
            }

        if (Utf8String::IsNullOrEmpty(definition))
            {
            LOG.errorv("Cannot create Phenomenon '%s' because the definition is empty.", phenomenaName);
            return nullptr;
            }

        auto phenomenon = new Phenomenon(phenomenaName, definition);

        phenomenon->m_unitsContext = this;

        m_phenomena.insert(bpair<Utf8CP, PhenomenonP>(phenomenon->GetName().c_str(), phenomenon));

        return phenomenon;
        }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
PhenomenonCP UnitRegistry::AddPhenomenon(Utf8CP name, Utf8CP definition)
    {
    return AddPhenomenonInternal(name, definition);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitP UnitRegistry::AddUnitInternal(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double numerator, double denominator, double offset, bool isConstant)
        {
        if (Utf8String::IsNullOrEmpty(unitName))
            {
            LOG.error("Cannot create base unit because the input name is null");
            return nullptr;
            }

        if (NamedItemExists(unitName))
            {
            LOG.errorv("Could not create unit '%s' because that name is already in use", unitName);
            return nullptr;
            }

        PhenomenonP phenomenon = _LookupPhenomenonP(phenomName);
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

        if (Utf8String::IsNullOrEmpty(definition))
            {
            LOG.errorv("Cannot create Unit '%s' because the definition is empty.", unitName);
            return nullptr;
            }

        if (0.0 == numerator || 0.0 == denominator)
            {
            LOG.errorv("Failed to create unit %s because numerator or denominator is 0.  Factor: %.17g / %.17g  Offset: %d", unitName, numerator, denominator, offset);
            return nullptr;
            }
        auto unit = new Unit(*system, *phenomenon, unitName, definition, numerator, denominator, offset, isConstant);

        unit->m_unitsContext = this;
        phenomenon->AddUnit(*unit);

        m_units.insert(bpair<Utf8CP, UnitP>(unit->GetName().c_str(), unit));

        return unit;
        }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitCP UnitRegistry::AddUnit(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double numerator, double denominator, double offset)
    {
    return AddUnitInternal(phenomName, systemName, unitName, definition, numerator, denominator, offset, false);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitP UnitRegistry::AddInvertedUnitInternal(Utf8CP parentUnitName, Utf8CP unitName, Utf8CP unitSystemName)
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

        if (Utf8String::IsNullOrEmpty(unitSystemName))
            {
            LOG.errorv("Cannot create unit %s because it's unit system name is null", unitName);
            return nullptr;
            }

        UnitCP parentUnit = LookupUnit(parentUnitName);
        if (nullptr == parentUnit)
            {
            LOG.errorv("Cannot create unit %s because it's parent unit %s cannot be found", unitName, parentUnitName);
            return nullptr;
            }

        UnitSystemCP unitSystem = LookupUnitSystem(unitSystemName);
        if (nullptr == unitSystem)
            {
            LOG.errorv("Cannot create unit %s because it's unit system %s cannot be found", unitName, unitSystemName);
            return nullptr;
            }

        if (NamedItemExists(unitName))
            {
            LOG.errorv("Could not create unit '%s' because that name is already in use", unitName);
            return nullptr;
            }

        if (parentUnit->HasOffset())
            {
             LOG.errorv("Cannot create inverted unit %s with parent %s because parent has an offset.", unitName, parentUnit->GetName().c_str());
            return nullptr;
            }
        if (parentUnit->IsInvertedUnit())
            {
             LOG.errorv("Cannot create inverted unit %s with parent %s because parent is an inverted unit", unitName, parentUnit->GetName().c_str());
            return nullptr;
            }

        auto unit = new Unit(*parentUnit, *unitSystem, unitName);

        unit->m_unitsContext = this;

        PhenomenonP phenomenon = _LookupPhenomenonP(parentUnit->GetPhenomenon()->GetName().c_str());
        phenomenon->AddUnit(*unit);

        m_units.insert(bpair<Utf8CP, UnitP>(unit->GetName().c_str(), unit));

        return unit;
        }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitCP UnitRegistry::AddInvertedUnit(Utf8CP parentUnitName, Utf8CP unitName, Utf8CP unitSystemName)
    {
    return AddInvertedUnitInternal(parentUnitName, unitName, unitSystemName);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitSystemP UnitRegistry::AddSystemInternal(Utf8CP name)
    {
    if (Utf8String::IsNullOrEmpty(name))
        {
        LOG.error("Cannot create UnitSystem because name is null");
        return nullptr;
        }
    if (NamedItemExists(name))
        {
        LOG.errorv("Cannot create UnitSystem '%s' because that name is already in use.", name);
        return nullptr;
        }

    auto unitSystem = new UnitSystem(name);
    unitSystem->m_unitsContext = this;
    m_systems.Insert(unitSystem->GetName().c_str(), unitSystem);

    return unitSystem;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitSystemCP UnitRegistry::AddSystem(Utf8CP name)
    {
    return AddSystemInternal(name);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitCP UnitRegistry::AddConstant(Utf8CP phenomName, Utf8CP systemName, Utf8CP constantName, Utf8CP definition, double numerator, double denominator)
    {
    return AddUnitInternal(phenomName, systemName, constantName, definition, numerator, denominator, 0, true);
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
