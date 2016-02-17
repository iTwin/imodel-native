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
    Utf8Vector m_phenomena;
    bmap<Utf8String, Unit *> m_units;
    bvector<Constant *> m_constants;
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
    void AddPhenomena(Utf8CP phenomenaName);

public:
    UNITS_EXPORT static UnitRegistry & Instance();
    UNITS_EXPORT static void Clear();

    UnitPtr LookupUnitBySubTypes (const Utf8Vector &numerator, const Utf8Vector &denominator) const;

    // Register methods.
    UNITS_EXPORT BentleyStatus AddUnit(Utf8CP systemName, Utf8CP phenomName, Utf8CP unitName, Utf8CP definition, double factor, double offset = 0);
    UNITS_EXPORT BentleyStatus AddConstant(double magnitude, Utf8CP unitName);

    // Lookup methods
    UNITS_EXPORT UnitPtr     LookupUnit(Utf8CP name) const;
    UNITS_EXPORT ConstantPtr LookupConstant(Utf8CP name) const;
    bmap<Utf8String, UnitP> const & AllUnits() const { return m_units; }
        
    // bool Exists methods.
    UNITS_EXPORT bool HasSystem (Utf8CP systemName) const;
    UNITS_EXPORT bool HasPhenomena (Utf8CP phenomenaName) const;
    UNITS_EXPORT bool HasUnit (Utf8CP unitName) const;
    UNITS_EXPORT bool HasConstant (Utf8CP constantName) const;

    // Probably some query methods. (Find base for phenomena and system probably).
    };

END_BENTLEY_UNITS_NAMESPACE