/*--------------------------------------------------------------------------------------+
|
|     $Source: src/UnitRegistry.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsPCH.h"
#include <Parser.h>

USING_NAMESPACE_BENTLEY_UNITS

UnitRegistry * UnitRegistry::s_instance = nullptr;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitRegistry& UnitRegistry::Instance()
    {
    if (nullptr == s_instance)
        s_instance = new UnitRegistry();

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
    AddDefaultSystems();
    AddDefaultPhenomena();
    AddDefaultUnits ();
    AddDefaultConstants ();
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

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddPhenomena (Utf8CP phenomenaName)
    {
    auto str = Utf8String(phenomenaName);
    InsertUnique (m_phenomena, str);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultSystems ()
    {
    AddSystem ("SI");
    AddSystem ("Metric");
    AddSystem ("CGS");
    AddSystem ("Imperial");
    AddSystem ("Physics");
    AddSystem ("Chemistry");
    // TODO: Add more.
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultPhenomena ()
    {
    AddPhenomena ("Length");
    AddPhenomena ("Mass");
    AddPhenomena ("Time");
    AddPhenomena ("Temperature");
    AddPhenomena ("Current");
    AddPhenomena ("Matter");
    AddPhenomena ("Luminosity");
    AddPhenomena ("Planar");
    AddPhenomena ("Solid");
    AddPhenomena ("Finance");
    AddPhenomena ("Capita");
    AddPhenomena ("Dimensionless");
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitRegistry::AddDefaultConstants ()
    {

    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UnitRegistry::AddUnit (Utf8CP systemName, Utf8CP phenomName, Utf8CP unitName, Utf8CP displayName, Utf8CP definition, double factor, double offset)
    {
    if (!(HasSystem(systemName) && HasPhenomena(phenomName)))
        return ERROR;

    auto unit = Unit::Create (systemName, phenomName, unitName, displayName, definition);
    if (!unit.IsValid())
        return ERROR;

    auto nameStr = Utf8String(unitName);
    m_units.insert (bpair<Utf8String, Unit *>(nameStr, unit.get()));

    // TODO: Add conversions.

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus UnitRegistry::AddConstant (double magnitude, Utf8CP unitName)
    {
    auto unit = LookupUnit (unitName);
    if (!unit.IsValid())
        return ERROR;

    // TODO: Insert Constant.    
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitPtr UnitRegistry::LookupUnit (Utf8CP name) const
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
ConstantPtr UnitRegistry::LookupConstant (Utf8CP name) const
    {
    auto nameStr = Utf8String(name);
    auto iter = find_if (m_constants.begin(), m_constants.end(), 
        [&](Constant *c) { return 0 == BeStringUtilities::Stricmp(c->GetConstantName(), name); });
    
    if (iter == m_constants.end())
        return nullptr;

    return *iter;
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
    auto iter = find (m_phenomena.begin(), m_phenomena.end(), Utf8String(phenomenaName));
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
    return constant.IsValid();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
UnitPtr UnitRegistry::LookupUnitBySubTypes (const Utf8Vector &numerator, const Utf8Vector &denominator) const
    {
    auto n = Utf8Vector(numerator), d = Utf8Vector(denominator);
    
    sort(n.begin(), n.end());
    sort(d.begin(), d.end());

    auto comparison = [&](bpair<Utf8String, Unit*> pair)
        {
        auto unit = pair.second;

        if (n.size() != unit->m_numerator.size())
            return false;

        if (d.size() != unit->m_denominator.size())
            return false;

        sort (unit->m_numerator.begin(), unit->m_denominator.end());
        sort (unit->m_denominator.begin(), unit->m_denominator.end());

        auto nmatch = mismatch (n.begin(), n.end(), unit->m_numerator.begin());
        if (nmatch.first != n.end())
            return false;

        auto dmatch = mismatch (d.begin(), d.end(), unit->m_denominator.begin());
        if (dmatch.first != d.end())
            return false;

        return true;
        };

    auto iter = find_if (m_units.begin(), m_units.end(), comparison);
    if (iter != m_units.end())
        return nullptr;

    return (*iter).second;
    }
