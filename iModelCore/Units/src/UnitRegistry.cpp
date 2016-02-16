/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Util.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <UnitsPCH.h>
#include <Parser.h>

UnitRegistry * UnitRegistry::s_instance = nullptr;

UnitRegistry& UnitRegistry::Instance()
	{
	if (nullptr == s_instance)
		s_instance = new UnitRegistry();

	return *s_instance;
	}

UnitRegistry::UnitRegistry()
	{
	this->AddGlobalUnits ();
	this->AddGlobalConstants ();
	}

void UnitRegistry::AddGlobalUnits ()
	{

	}

void UnitRegistry::AddGlobalConstants ()
	{

	}

BentleyStatus UnitRegistry::AddUnit (Utf8CP systemName, Utf8CP phenomName, Utf8CP unitName, Utf8CP displayName, Utf8CP definition, double factor, double offset)
	{
	if (!(HasSystem(systemName) && HasPhenomena(phenomName)))
		return ERROR;

	auto unit = Unit::Create (systemName, phenomName, unitName, displayName, definition);
	if (nullptr == unit)
		return ERROR;

	auto nameStr = Utf8String(unitName);
	m_units.insert (bpair<Utf8String, Unit *>(nameStr, unit));

	// TODO: Add conversions.

	return SUCCESS;
	}

BentleyStatus UnitRegistry::AddConstant (double magnitude, Utf8CP unitName)
	{
	auto unit = LookupUnit (unitName);
	if (nullptr == unit)
		return ERROR;

	// TODO: Insert Constant.	
	return SUCCESS;
	}

Unit * UnitRegistry::LookupUnit (Utf8CP name)
	{
	auto nameStr = Utf8String(name);
	auto val_iter = m_units.find (nameStr);
	if (val_iter == m_units.end())
		return nullptr;

	return (*val_iter).second;
	}

Constant * UnitRegistry::LookupConstant (Utf8CP name)
	{
	// TODO: Implement
	return nullptr;
	}

bool UnitRegistry::HasSystem (Utf8CP systemName)
	{
	auto iter = find (m_systems.begin(), m_systems.end(), Utf8String(systemName));
	return iter != m_systems.end();
	}

bool UnitRegistry::HasPhenomena (Utf8CP phenomenaName)
	{
	auto iter = find (m_phenomena.begin(), m_phenomena.end(), Utf8String(phenomenaName));
	return iter != m_phenomena.end();
	}