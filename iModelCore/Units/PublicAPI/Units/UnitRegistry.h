/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Util.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Units/Units.h>

BEGIN_BENTLEY_UNITS_NAMESPACE

//=======================================================================================
//! A central place to store registered units with the system.  Users interact
//! with the units system here.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct UnitRegistry
	{
private:
	static UnitRegistry * s_instance;

	bvector<Utf8String> m_systems;
	bvector<Utf8String> m_phenomena;
	bmap<Utf8String, Unit *> m_units;
	bvector<Constant> m_constants;
	bmap<bpair<Utf8String, Utf8String>, double> m_conversions;

	UnitRegistry();
	UnitRegistry(const UnitRegistry& rhs) = delete;
	UnitRegistry & operator= (const UnitRegistry& rhs) = delete;

    void AddDefaultSystems ();
    void AddDefaultPhenomena ();
	void AddDefaultUnits ();
	void AddDefaultConstants();

    void InsertUnique (bvector<Utf8String> &vec, Utf8String &str);
	void AddSystem(Utf8CP systemName);
	void AddPhenomena(Utf8CP phenomenaName);

public:
	static UnitRegistry & Instance();

	// Register methods.
	UNITS_EXPORT BentleyStatus AddUnit(Utf8CP systemName, Utf8CP phenomName, Utf8CP unitName, Utf8CP displayName, Utf8CP definition, double factor, double offset);
	UNITS_EXPORT BentleyStatus AddConstant(double magnitude, Utf8CP unitName);

	// Lookup methods
	UNITS_EXPORT UnitPtr    LookupUnit(Utf8CP name);
	UNITS_EXPORT Constant * LookupConstant(Utf8CP name);
		
	Unit * LookupUnitBySubTypes (const bvector<Utf8String> &numerator, const bvector<Utf8String> &denominator) const;

	// bool Exists methods.
	UNITS_EXPORT bool HasSystem (Utf8CP systemName);
	UNITS_EXPORT bool HasPhenomena (Utf8CP phenomenaName);

	// Probably some query methods. (Find base for phenomena and system probably).
	};

END_BENTLEY_UNITS_NAMESPACE