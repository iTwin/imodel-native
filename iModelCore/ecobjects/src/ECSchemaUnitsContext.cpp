/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
SchemaUnitContext::~SchemaUnitContext()
    {
    for (auto entry : m_unitSystemMap)
        {
        auto unitSystem = entry.second;
        delete unitSystem;
        }

    m_unitSystemMap.clear();
    BeAssert(m_unitSystemMap.empty());

    // Units have to be deconstructed before Phenomenon since the destructor of Unit remove itself from the phenomenon cache
    for (auto entry : m_unitMap)
        {
        auto unit = entry.second;
        delete unit;
        }

    m_unitMap.clear();
    BeAssert(m_unitMap.empty());

    for (auto entry : m_phenomenonMap)
        {
        auto phenomenon = entry.second;
        delete phenomenon;
        }

    m_phenomenonMap.clear();
    BeAssert(m_phenomenonMap.empty());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
template<typename T> ECObjectsStatus SchemaUnitContext::Add(T* toAdd, ECSchemaElementType unitType) {;}
template<> ECObjectsStatus SchemaUnitContext::Add<UnitSystem>(UnitSystemP toAdd, ECSchemaElementType unitType) {return AddToMap<UnitSystem, UnitSystemMap>(toAdd, &m_unitSystemMap, unitType);}
template<> ECObjectsStatus SchemaUnitContext::Add<Phenomenon>(PhenomenonP toAdd, ECSchemaElementType unitType) {return AddToMap<Phenomenon, PhenomenonMap>(toAdd, &m_phenomenonMap, unitType);}
template<> ECObjectsStatus SchemaUnitContext::Add<ECUnit>(ECUnitP toAdd, ECSchemaElementType unitType) {return AddToMap<ECUnit, UnitMap>(toAdd, &m_unitMap, unitType);}

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
template<typename T> ECObjectsStatus SchemaUnitContext::Delete(T& toDelete) {;}
template<> ECObjectsStatus SchemaUnitContext::Delete<UnitSystem>(UnitSystemR toDelete) {return DeleteFromMap<UnitSystem, UnitSystemMap>(toDelete, &m_unitSystemMap);}
template<> ECObjectsStatus SchemaUnitContext::Delete<Phenomenon>(PhenomenonR toDelete) {return DeleteFromMap<Phenomenon, PhenomenonMap>(toDelete, &m_phenomenonMap);}
template<> ECObjectsStatus SchemaUnitContext::Delete<ECUnit>(ECUnitR toDelete) {return DeleteFromMap<ECUnit, UnitMap>(toDelete, &m_unitMap); }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
template<typename T> T* SchemaUnitContext::Get(Utf8CP name) const {;}
template<> UnitSystemP SchemaUnitContext::Get<UnitSystem>(Utf8CP name) const {return GetFromMap<UnitSystem, UnitSystemMap>(name, &m_unitSystemMap);}
template<> PhenomenonP SchemaUnitContext::Get<Phenomenon>(Utf8CP name) const {return GetFromMap<Phenomenon, PhenomenonMap>(name, &m_phenomenonMap);}
template<> ECUnitP SchemaUnitContext::Get<ECUnit>(Utf8CP name) const {return GetFromMap<ECUnit, UnitMap>(name, &m_unitMap);}

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
ECUnitP SchemaUnitContext::_LookupUnitP(Utf8CP name, bool useFullName) const
    {
    Utf8String aliasOrSchemaName;
    Utf8String unqualifiedName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(aliasOrSchemaName, unqualifiedName, name))
        return nullptr;
    // Have to check !useFullName for checking alias because of the possibility of someone naming their schema the same as this schema's alias
    // Without this the lookup would occur on this schema instead of the referenced schema it was supposed to.
    if (aliasOrSchemaName.empty() || (!useFullName && aliasOrSchemaName.EqualsI(m_schema.GetAlias())) || (useFullName && aliasOrSchemaName.EqualsI(m_schema.GetName())))
        return m_schema.GetUnitP(unqualifiedName.c_str());

    if (useFullName)
        {
        for (const auto& s : m_schema.GetReferencedSchemas())
            {
            if (aliasOrSchemaName.EqualsI(s.second->GetName()))
                return s.second->GetUnitP(unqualifiedName.c_str());
            }
        return nullptr;
        }

    ECSchemaCP resolvedUnitSchema = m_schema.GetSchemaByAliasP(aliasOrSchemaName);
    return (nullptr != resolvedUnitSchema ? resolvedUnitSchema->GetUnitP(unqualifiedName.c_str()) : nullptr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
void SchemaUnitContext::_AllUnits(bvector<Units::UnitCP>& allUnits) const
    {
    for (auto unit : GetUnits())
        allUnits.push_back(unit);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
PhenomenonP SchemaUnitContext::_LookupPhenomenonP(Utf8CP name, bool useFullName) const
    {
    Utf8String aliasOrSchemaName;
    Utf8String unqualifiedName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(aliasOrSchemaName, unqualifiedName, name))
        return nullptr;
    // Have to check !useFullName for checking alias because of the possibility of someone naming their schema the same as this schema's alias
    // Without this the lookup would occur on this schema instead of the referenced schema it was supposed to.
    if (aliasOrSchemaName.empty() || (!useFullName && aliasOrSchemaName.EqualsI(m_schema.GetAlias())) || (useFullName && aliasOrSchemaName.EqualsI(m_schema.GetName())))
        return m_schema.GetPhenomenonP(unqualifiedName.c_str());

    if (useFullName)
        {
        for (const auto& s : m_schema.GetReferencedSchemas())
            {
            if (aliasOrSchemaName.EqualsI(s.second->GetName()))
                return s.second->GetPhenomenonP(unqualifiedName.c_str());
            }
        return nullptr;
        }

    ECSchemaCP resolvedUnitSchema = m_schema.GetSchemaByAliasP(aliasOrSchemaName);
    return (nullptr != resolvedUnitSchema ? resolvedUnitSchema->GetPhenomenonP(unqualifiedName.c_str()) : nullptr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
void SchemaUnitContext::_AllPhenomena(bvector<Units::PhenomenonCP>& allPhenomena) const
    {
    for (auto phen : GetPhenomena())
        allPhenomena.push_back(phen);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
UnitSystemP SchemaUnitContext::_LookupUnitSystemP(Utf8CP name, bool useFullName) const
    {
    Utf8String aliasOrSchemaName;
    Utf8String unqualifiedName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(aliasOrSchemaName, unqualifiedName, name))
        return nullptr;
    // Have to check !useFullName for checking alias because of the possibility of someone naming their schema the same as this schema's alias
    // Without this the lookup would occur on this schema instead of the referenced schema it was supposed to.
    if (aliasOrSchemaName.empty() || (!useFullName && aliasOrSchemaName.EqualsI(m_schema.GetAlias())) || (useFullName && aliasOrSchemaName.EqualsI(m_schema.GetName())))
        return m_schema.GetUnitSystemP(unqualifiedName.c_str());

    if (useFullName)
        {
        for (const auto& s : m_schema.GetReferencedSchemas())
            {
            if (aliasOrSchemaName.EqualsI(s.second->GetName()))
                return s.second->GetUnitSystemP(unqualifiedName.c_str());
            }
        return nullptr;
        }

    ECSchemaCP resolvedUnitSchema = m_schema.GetSchemaByAliasP(aliasOrSchemaName);
    return (nullptr != resolvedUnitSchema ? resolvedUnitSchema->GetUnitSystemP(unqualifiedName.c_str()) : nullptr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
void SchemaUnitContext::_AllSystems(bvector<Units::UnitSystemCP>& allUnitSystems) const
    {
    for (auto system : GetUnitSystems())
        allUnitSystems.push_back(system);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
