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
UNITS_TYPEDEFS(Symbol)

BEGIN_BENTLEY_UNITS_NAMESPACE

typedef bvector<Utf8String> Utf8Vector;

struct UnitRegistry;

struct Symbol
    {
friend struct SymbolicExpression;
friend struct Unit;
friend struct Phenomenon;
private:
    Utf8String  m_name;
    Utf8String  m_definition;
    int         m_id;
    Utf8Char    m_dimensionSymbol;
    double      m_factor;
    double      m_offset;
    bool        m_dimensionless;
    mutable bool m_evaluated;
    mutable SymbolicExpression m_symbolExpression;


    SymbolicExpression& Evaluate(std::function<SymbolCP(Utf8CP)> getSymbolByName) const;

protected:
    Symbol(Utf8CP name, Utf8CP definition, Utf8Char dimensionSymbol, int id, double factor, double offset) : 
        m_name(name), m_definition(definition), m_dimensionSymbol(dimensionSymbol), m_id(id), m_factor(factor), m_offset(offset), m_evaluated(false)
        {
        m_dimensionless = strcmp("ONE", m_definition.c_str()) == 0;
        }

public:
    Utf8CP GetName() const { return m_name.c_str(); }
    int     GetId() const { return m_id; }
    Utf8CP GetDefinition() const { return m_definition.c_str(); }
    double GetFactor() const { return m_factor; }
    bool IsBaseSymbol() const { return ' ' != m_dimensionSymbol; }
    bool IsDimensionless() const { return m_dimensionless; }

    // Binary comparison operators.
    bool operator== (SymbolCR rhs) const { return m_id == rhs.m_id; }
    bool operator!= (SymbolCR rhs) const { return m_id != rhs.m_id; }
    };


//=======================================================================================
//! A base class for all units.
// @bsiclass                                                    Chris.Tartamella   02/16
//=======================================================================================
struct Unit : Symbol
    {
DEFINE_T_SUPER(Symbol)
friend struct UnitRegistry;

private:
    Utf8String  m_system;
    Utf8String  m_phenomenon;
    bool        m_isConstant;

    static UnitP Create (Utf8CP sysName, Utf8CP phenomName, Utf8CP unitName, int id, Utf8CP definition, Utf8Char baseDimensionSymbol, double factor, double offset, bool isConstant);

    Unit (Utf8CP system, Utf8CP phenomena, Utf8CP name, int id, Utf8CP definition, Utf8Char dimensionSymbol, double factor, double offset, bool isConstant);

    // Lifecycle is managed by the UnitRegistry so we don't allow copies or assignments.
    Unit (UnitCR unit) = delete;
    UnitR operator=(UnitCR unit) = delete;

public:
    virtual ~Unit() { }

    UNITS_EXPORT double GetConversionTo(UnitCP unit) const;

    bool   IsRegistered()    const;
    Utf8CP GetPhenomenon()   const { return m_phenomenon.c_str(); }
    bool IsConstant() const { return m_isConstant; }

};

struct Phenomenon : Symbol
{
DEFINE_T_SUPER(Symbol)
friend struct UnitRegistry;
private:
    Phenomenon(Utf8CP name, Utf8CP definition, Utf8Char dimensionSymbol, int id) : Symbol(name, definition, dimensionSymbol, id, 0.0, 0) {}

};
END_BENTLEY_UNITS_NAMESPACE