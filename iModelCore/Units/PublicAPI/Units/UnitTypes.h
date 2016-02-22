/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Units/UnitTypes.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Units/Units.h>

UNITS_TYPEDEFS(Unit)
UNITS_TYPEDEFS(Phenomenon)

BEGIN_BENTLEY_UNITS_NAMESPACE

typedef bvector<Utf8String> Utf8Vector;

struct UnitRegistry;

//=======================================================================================
//! A base class for all units.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct Unit : Symbol
    {
DEFINE_T_SUPER(Symbol)
friend struct UnitRegistry;

private:
    Utf8String      m_system;
    PhenomenonCP    m_phenomenon;
    bool            m_isConstant;

    static UnitP Create (Utf8CP sysName, PhenomenonCR phenomenon, Utf8CP unitName, int id, Utf8CP definition, Utf8Char baseDimensionSymbol, double factor, double offset, bool isConstant);

    Unit (Utf8CP system, PhenomenonCR phenomenon, Utf8CP name, int id, Utf8CP definition, Utf8Char dimensionSymbol, double factor, double offset, bool isConstant);

    // Lifecycle is managed by the UnitRegistry so we don't allow copies or assignments.
    Unit() = delete;
    Unit (UnitCR unit) = delete;
    UnitR operator=(UnitCR unit) = delete;

public:
    UNITS_EXPORT double GetConversionTo(UnitCP unit) const;

    bool IsRegistered() const;
    bool IsConstant() const { return m_isConstant; }

    PhenomenonCP GetPhenomenon()   const { return m_phenomenon; }
};

struct Phenomenon : Symbol
{
DEFINE_T_SUPER(Symbol)
friend struct UnitRegistry;
private:
    bvector<UnitCP> m_units;

    void AddUnit(UnitCR unit);

    Phenomenon(Utf8CP name, Utf8CP definition, Utf8Char dimensionSymbol, int id) : Symbol(name, definition, dimensionSymbol, id, 0.0, 0) {}

    Phenomenon() = delete;
    Phenomenon(PhenomenonCR phenomenon) = delete;
    PhenomenonR operator=(PhenomenonCR phenomenon) = delete;

public:
    UNITS_EXPORT Utf8String GetPhenomenonDimension() const;
    
    bool HasUnits() const { return m_units.size() > 0; }
    bvector<UnitCP> const GetUnits() const { return m_units; }
};
END_BENTLEY_UNITS_NAMESPACE