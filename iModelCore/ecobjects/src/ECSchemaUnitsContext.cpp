/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECSchemaUnitsContext.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

    for (auto entry : m_phenomenonMap)
        {
        auto phenomenon = entry.second;
        delete phenomenon;
        }

    m_phenomenonMap.clear();
    BeAssert(m_phenomenonMap.empty());

    for (auto entry : m_unitMap)
        {
        auto unit = entry.second;
        delete unit;
        }

    m_unitMap.clear();
    BeAssert(m_unitMap.empty());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
ECUnitP SchemaUnitContext::_LookupUnitP(Utf8CP name) const
    {
    Utf8String unitAlias;
    Utf8String unitName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(unitAlias, unitName, name))
        return nullptr;

    ECUnitP unit = nullptr;
    if (unitAlias.empty())
        unit = m_schema.GetUnitP(unitName.c_str());
    else
        {
        ECSchemaCP resolvedUnitSchema = m_schema.GetSchemaByAliasP(unitAlias);
        if (nullptr == resolvedUnitSchema)
            return nullptr;

        unit = resolvedUnitSchema->GetUnitP(unitName.c_str());
        }

    return unit;
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
PhenomenonP SchemaUnitContext::_LookupPhenomenonP(Utf8CP name) const
    {
    Utf8String phenomAlias;
    Utf8String phenomName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(phenomAlias, phenomName, name))
        return nullptr;

    PhenomenonP phenom = nullptr;
    if (phenomAlias.empty())
        phenom = m_schema.GetPhenomenonP(phenomName.c_str());
    else
        {
        ECSchemaCP resolvedPhenomSchema = m_schema.GetSchemaByAliasP(phenomAlias);
        if (nullptr == resolvedPhenomSchema)
            return nullptr;

        phenom = resolvedPhenomSchema->GetPhenomenonP(phenomName.c_str());
        }

    return phenom;
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
UnitSystemP SchemaUnitContext::_LookupUnitSystemP(Utf8CP name) const
    {
    Utf8String systemAlias;
    Utf8String systemName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(systemAlias, systemName, name))
        return nullptr;

    UnitSystemP system = nullptr;
    if (systemAlias.empty())
        system = m_schema.GetUnitSystemP(systemName.c_str());
    else
        {
        ECSchemaCP resolvedSchema = m_schema.GetSchemaByAliasP(systemAlias);
        if (nullptr == resolvedSchema)
            return nullptr;

        system = resolvedSchema->GetUnitSystemP(systemName.c_str());
        }

    return system;
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
