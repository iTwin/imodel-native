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
	AddGlobalSystems();
	AddGlobalPhenomena();
	AddGlobalConstants();
	}

void UnitRegistry::AddGlobalSystems()
	{

	}

void UnitRegistry::AddGlobalPhenomena()
	{

	}

void UnitRegistry::AddGlobalConstants()
	{

	}

void UnitRegistry::AddSystem(Utf8CP systemName)
	{

	}

void UnitRegistry::AddPhenomena (Utf8CP phenomenaName)
	{

	}

void UnitRegistry::AddUnit (Utf8CP name, int phenomena, int system, Utf8CP expression, double factor, double offset)
	{

	}

void UnitRegistry::AddConstant (double magnitude, Utf8CP unitName)
	{

	}

Unit * UnitRegistry::LookupUnit (Utf8CP name)
	{
	return nullptr;
	}

Constant * UnitRegistry::LookupConstant (Utf8CP name)
	{
	return nullptr;
	}