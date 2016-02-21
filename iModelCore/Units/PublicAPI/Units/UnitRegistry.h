/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/UnitRegistry.h $
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

    Utf8Vector m_systems;
    bmap<Utf8String, PhenomenonCP> m_phenomena;
    bmap<Utf8String, UnitCP> m_units;
    int m_nextId = 0;

    bmap<bpair<Utf8String, Utf8String>, double> m_conversions;

    UnitRegistry();
    UnitRegistry(const UnitRegistry& rhs) = delete;
    UnitRegistry & operator= (const UnitRegistry& rhs) = delete;

    void AddDefaultSystems ();
    void AddDefaultPhenomena ();
    void AddDefaultUnits ();
    void AddDefaultConstants();

    void InsertUnique (Utf8Vector &vec, Utf8String &str);
    void AddSystem(Utf8CP systemName);
    void AddPhenomena(Utf8CP phenomenaName, Utf8CP definition);
    void AddBasePhenomena(Utf8Char dimensionalSymbol);
    UnitCP AddSIBaseUnit(Utf8CP unitName, Utf8Char dimensionSymbol);

    bool NameConflicts(Utf8CP name);

public:
    UNITS_EXPORT static UnitRegistry & Instance();
    UNITS_EXPORT static void Clear();

    UnitCP LookupUnitBySubTypes (const Utf8Vector &numerator, const Utf8Vector &denominator) const;
    // TODO: Wrap the return to be cleaner?
    bmap<Utf8String, UnitCP> const & AllUnits() const { return m_units; }
    
    // Register methods.
    UNITS_EXPORT UnitCP AddUnit(Utf8CP phenomName, Utf8CP systemName, Utf8CP unitName, Utf8CP definition, double factor = 1, double offset = 0);
    UNITS_EXPORT BentleyStatus AddConstant(Utf8CP phenomName, Utf8CP constantName, Utf8CP definition, double factor);
    
    UNITS_EXPORT BentleyStatus AddSynonym(UnitCP unit, Utf8CP synonymName);

    // Lookup methods
    UNITS_EXPORT UnitCP LookupUnit(Utf8CP name) const;
    UNITS_EXPORT UnitCP LookupConstant(Utf8CP name) const;
    UNITS_EXPORT PhenomenonCP LookupPhenomenon(Utf8CP name) const;
        
    // bool Exists methods.
    UNITS_EXPORT bool HasSystem (Utf8CP systemName) const;
    UNITS_EXPORT bool HasPhenomena (Utf8CP phenomenaName) const;
    UNITS_EXPORT bool HasUnit (Utf8CP unitName) const;
    UNITS_EXPORT bool HasConstant (Utf8CP constantName) const;

    // Probably some query methods. (Find base for phenomena and system probably).
    };

END_BENTLEY_UNITS_NAMESPACE