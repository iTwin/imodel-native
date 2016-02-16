/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Util.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <UnitsPCH.h>

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

BentleyStatus UnitRegistry::AddUnit (Utf8CP name, Utf8CP phenomena, Utf8CP system, Utf8CP expression, double factor, double offset)
	{
	if (!(HasSystem(system) && HasPhenomena(phenomena)))
		return ERROR;
	
	return SUCCESS;
	}

BentleyStatus UnitRegistry::AddConstant (double magnitude, Utf8CP unitName)
	{
	return SUCCESS;
	}

Unit * UnitRegistry::LookupUnit (Utf8CP name)
	{
	return nullptr;
	}

Constant * UnitRegistry::LookupConstant (Utf8CP name)
	{
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